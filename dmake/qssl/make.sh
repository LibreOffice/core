mkdir objects
cc -c -I. -Iqssl -3 -O infer.c
mv infer.o objects
cc -c -I. -Iqssl -3 -O make.c
mv make.o objects
cc -c -I. -Iqssl -3 -O stat.c
mv stat.o objects
cc -c -I. -Iqssl -3 -O expand.c
mv expand.o objects
cc -c -I. -Iqssl -3 -O dmstring.c
mv dmstring.o objects
cc -c -I. -Iqssl -3 -O hash.c
mv hash.o objects
cc -c -I. -Iqssl -3 -O dag.c
mv dag.o objects
cc -c -I. -Iqssl -3 -O dmake.c
mv dmake.o objects
cc -c -I. -Iqssl -3 -O path.c
mv path.o objects
cc -c -I. -Iqssl -3 -O imacs.c
mv imacs.o objects
cc -c -I. -Iqssl -3 -O sysintf.c
mv sysintf.o objects
cc -c -I. -Iqssl -3 -O parse.c
mv parse.o objects
cc -c -I. -Iqssl -3 -O getinp.c
mv getinp.o objects
cc -c -I. -Iqssl -3 -O quit.c
mv quit.o objects
cc -c -I. -Iqssl -3 -O state.c
mv state.o objects
cc -c -I. -Iqssl -3 -O dmdump.c
mv dmdump.o objects
cc -c -I. -Iqssl -3 -O macparse.c
mv macparse.o objects
cc -c -I. -Iqssl -3 -O rulparse.c
mv rulparse.o objects
cc -c -I. -Iqssl -3 -O percent.c
mv percent.o objects
cc -c -I. -Iqssl -3 -O function.c
mv function.o objects
cc -c -I. -Iqssl -3 -O qssl/ruletab.c
mv ruletab.o objects
cc -c -I. -Iqssl -3 -O qssl/runargv.c
mv runargv.o objects
cc -c -I. -Iqssl -3 -O qssl/tempnam.c
mv tempnam.o objects
cc -c -I. -Iqssl -3 -O unix/dcache.c
mv dcache.o objects
cc -c -I. -Iqssl -3 -O unix/rmprq.c
mv rmprq.o objects
cc -c -I. -Iqssl -3 -O unix/dirbrk.c
mv dirbrk.o objects
cc -c -I. -Iqssl -3 -O msdos/arlib.c
mv arlib.o objects
cc -N 8192 -o dmake  objects/infer.o objects/make.o objects/stat.o \
objects/expand.o objects/dmstring.o objects/hash.o objects/dag.o objects/dmake.o \
objects/path.o objects/imacs.o objects/sysintf.o objects/parse.o \
objects/getinp.o objects/quit.o objects/state.o objects/dmdump.o \
objects/macparse.o objects/rulparse.o objects/percent.o objects/function.o \
objects/ruletab.o objects/runargv.o objects/tempnam.o objects/dcache.o objects/rmprq.o objects/dirbrk.o objects/arlib.o
cp qssl/template.mk startup/config.mk
