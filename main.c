#include <stdio.h>
#include <string.h>
#include "nsf.h"

void Nsf_printChipExtensions(const NsfChipExtensions* ext,FILE* output){
	if(ext->VRCVI)       fputs("VRCVI, ",output);
	if(ext->VRCVII)      fputs("VRCVII, ",output);
	if(ext->FDS)         fputs("FDS sound, ",output);
	if(ext->MMC5)        fputs("MMC5 audio, ",output);
	if(ext->namco106)    fputs("Namco 106, ",output);
	if(ext->sunsoftFME07)fputs("Sunsoft FME-07, ",output);
}

int main(int argc,const char* argv[]){
	if(argc<2)
		goto Help;

	struct NsfFile nsf;
	memset(&nsf,'\0',sizeof(struct NsfFile));

	if(argc==2){
		FILE* file = fopen(argv[1],"rb");
		int fileLoadingReturnCode=NsfFile_load(&nsf,file,false,false);

		if(fileLoadingReturnCode!=0)
			fprintf(stderr,"Error in file loading: %i\n",fileLoadingReturnCode);
		else{
			printf("Title: %s\nRegion: %s\nArtist: %s\nCopyright: %s\n\nTracks: %i\nInitial track: %i\nExtension Chips: ",
				nsf.gameTitle,
				nsf.region?"PAL":"NTSC",
				nsf.artist,
				nsf.copyright,
				nsf.trackCount,
				nsf.initialTrack
			);

			Nsf_printChipExtensions(&nsf.chipExtensions,stdout);
			printf("\nRipper: %s\n\n",nsf.ripper);

			puts("Tracks:");
			for(int i=0;i<nsf.trackCount;++i){
				printf(" %03u:\n  Title:  %s\n  Length: %i ms (%u:%02u min)\n  Fade:   %i ms\n",
					i,
					nsf.trackLabels?nsf.trackLabels[i]:"",
					nsf.trackTimes?nsf.trackTimes[i]:-1,
					nsf.trackTimes?(nsf.trackTimes[i]/1000/60):0,
					nsf.trackTimes?(nsf.trackTimes[i]/1000%60):0,
					nsf.trackFades?nsf.trackFades[i]:-1
				);
			}

			if(nsf.playlist){
				puts("\nPlaylist Order:");
				for(int i=0;i<nsf.playlistSize;++i){
					printf(" %03u\n",nsf.playlist[i]);
				}
			}
		}

		if(file){
			NsfFile_free(&nsf);
			fclose(file);
		}
		return fileLoadingReturnCode;
	}else{
		argv+=argc-1;

		FILE* file = fopen(*argv,"rb");
		int fileLoadingReturnCode=NsfFile_load(&nsf,file,true,false);

		if(fileLoadingReturnCode!=0){
			fprintf(stderr,"Error when loading file `%s`: %i\n",*argv,fileLoadingReturnCode);
		}else{
			do{
				--argv;
				if((*argv)[0]=='-' && (*argv)[1]=='-'){
					if(strcmp((*argv)+2,"title")==0)
						puts(nsf.gameTitle);
					else if(strcmp((*argv)+2,"region")==0)
						puts(nsf.region==1?"PAL":"NTSC");
					else if(strcmp((*argv)+2,"artist")==0)
						puts(nsf.artist);
					else if(strcmp((*argv)+2,"copyright")==0)
						puts(nsf.copyright);
					else if(strcmp((*argv)+2,"trackcount")==0)
						printf("%i\n",nsf.trackCount);
					else if(strcmp((*argv)+2,"initialtrack")==0)
						printf("%i\n",nsf.initialTrack);
					else if(strcmp((*argv)+2,"ripper")==0)
						puts(nsf.ripper);
					else if(strcmp((*argv)+2,"chips")==0){
						Nsf_printChipExtensions(&nsf.chipExtensions,stdout);
						putchar('\n');
					}else if(memcmp((*argv)+2,"output=",7)==0){
						const char* path=(*argv)+2+7;
						if(path[0]=='\0')
							goto Help;
						FILE* file=fopen(path,"wb");
						NsfFile_saveAsNESM(&nsf,file);
						fclose(file);
					}else
						fprintf(stderr,"Unknown parameter: `%s`\n",*argv);
				}else
					goto Help;
			}while(--argc>2);
		}

		if(file){
			NsfFile_free(&nsf);
			fclose(file);
		}
		return fileLoadingReturnCode;
	}

	Help:
	fputs("Usage: nsfinfo [options ...] <filepath>\nOptions:\n  --title\n     Get game title\n  --region\n     Get region of nsf (e.g. NTSC or PAL)\n  --artist\n     Get artist\n  --copyright\n     Get copyright text\n  --trackcount\n     Get the total of tracks\n  --initialtrack\n     Get the starting track index\n  --chips\n     Get the extension chips required to play the file\n  --ripper\n     Get ripper name\n  --output=<file path>\n     Output to file with specified path\n",stderr);
	return 1;
}
