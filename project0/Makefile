
all: findminmax_seq findminmax_pipes findminmax_files

findminmax_seq : findminmax_seq.c measure.h
	gcc -o findminmax_seq findminmax_seq.c

findminmax_pipes : findminmax_pipes.c measure.h
	gcc -o findminmax_pipes findminmax_pipes.c

findminmax_files : findminmax_files.c measure.h
	gcc -o findminmax_files findminmax_files.c

clean:
	/bin/rm -f findminmax_seq findminmax_pipes findminmax_files

