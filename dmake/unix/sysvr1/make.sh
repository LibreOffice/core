mkdir objects
cc -c -I. -Iunix -Iunix/sysvr1 -O infer.c
mv infer.o objects
cc -c -I. -Iunix -Iunix/sysvr1 -O make.c
mv make.o objects
cc -c -I. -Iunix -Iunix/sysvr1 -O stat.c
mv stat.o objects
cc -c -I. -Iunix -Iunix/sysvr1 -O expand.c
mv expand.o objects
cc -c -I. -Iunix -Iunix/sysvr1 -O dmstring.c
mv dmstring.o objects
cc -c -I. -Iunix -Iunix/sysvr1 -O hash.c
mv hash.o objects
cc -c -I. -Iunix -Iunix/sysvr1 -O dag.c
mv dag.o objects
cc -c -I. -Iunix -Iunix/sysvr1 -O dmake.c
mv dmake.o objects
cc -c -I. -Iunix -Iunix/sysvr1 -O path.c
mv path.o objects
cc -c -I. -Iunix -Iunix/sysvr1 -O imacs.c
mv imacs.o objects
cc -c -I. -Iunix -Iunix/sysvr1 -O sysintf.c
mv sysintf.o objects
cc -c -I. -Iunix -Iunix/sysvr1 -O parse.c
mv parse.o objects
cc -c -I. -Iunix -Iunix/sysvr1 -O getinp.c
mv getinp.o objects
cc -c -I. -Iunix -Iunix/sysvr1 -O quit.c
mv quit.o objects
cc -c -I. -Iunix -Iunix/sysvr1 -O state.c
mv state.o objects
cc -c -I. -Iunix -Iunix/sysvr1 -O dmdump.c
mv dmdump.o objects
cc -c -I. -Iunix -Iunix/sysvr1 -O macparse.c
mv macparse.o objects
cc -c -I. -Iunix -Iunix/sysvr1 -O rulparse.c
mv rulparse.o objects
cc -c -I. -Iunix -Iunix/sysvr1 -O percent.c
mv percent.o objects
cc -c -I. -Iunix -Iunix/sysvr1 -O function.c
mv function.o objects
cc -c -I. -Iunix -Iunix/sysvr1 -O unix/arlib.c
mv arlib.o objects
cc -c -I. -Iunix -Iunix/sysvr1 -O unix/dirbrk.c
mv dirbrk.o objects
cc -c -I. -Iunix -Iunix/sysvr1 -O unix/rmprq.c
mv rmprq.o objects
cc -c -I. -Iunix -Iunix/sysvr1 -O unix/ruletab.c
mv ruletab.o objects
cc -c -I. -Iunix -Iunix/sysvr1 -O unix/runargv.c
mv runargv.o objects
cc -c -I. -Iunix -Iunix/sysvr1 -O unix/dcache.c
mv dcache.o objects
cc -c -I. -Iunix -Iunix/sysvr1 -O unix/sysvr1/vfprintf.c
mv vfprintf.o objects
cc -O -o dmake  objects/infer.o objects/make.o objects/stat.o objects/expand.o \
objects/dmstring.o objects/hash.o objects/dag.o objects/dmake.o objects/path.o \
objects/imacs.o objects/sysintf.o objects/parse.o objects/getinp.o \
objects/quit.o objects/state.o objects/dmdump.o objects/macparse.o \
objects/rulparse.o objects/percent.o objects/function.o objects/arlib.o \
objects/dirbrk.o objects/rmprq.o objects/ruletab.o objects/runargv.o objects/dcache.o objects/vfprintf.o
cp unix/sysvr1/template.mk startup/config.mk
