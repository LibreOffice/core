mkdir objects
cc -c -I. -Iunix -Iunix/bsd43 -Iunix/bsd43/vf -O infer.c
mv infer.o objects
cc -c -I. -Iunix -Iunix/bsd43 -Iunix/bsd43/vf -O make.c
mv make.o objects
cc -c -I. -Iunix -Iunix/bsd43 -Iunix/bsd43/vf -O stat.c
mv stat.o objects
cc -c -I. -Iunix -Iunix/bsd43 -Iunix/bsd43/vf -O expand.c
mv expand.o objects
cc -c -I. -Iunix -Iunix/bsd43 -Iunix/bsd43/vf -O dmstring.c
mv dmstring.o objects
cc -c -I. -Iunix -Iunix/bsd43 -Iunix/bsd43/vf -O hash.c
mv hash.o objects
cc -c -I. -Iunix -Iunix/bsd43 -Iunix/bsd43/vf -O dag.c
mv dag.o objects
cc -c -I. -Iunix -Iunix/bsd43 -Iunix/bsd43/vf -O dmake.c
mv dmake.o objects
cc -c -I. -Iunix -Iunix/bsd43 -Iunix/bsd43/vf -O path.c
mv path.o objects
cc -c -I. -Iunix -Iunix/bsd43 -Iunix/bsd43/vf -O imacs.c
mv imacs.o objects
cc -c -I. -Iunix -Iunix/bsd43 -Iunix/bsd43/vf -O sysintf.c
mv sysintf.o objects
cc -c -I. -Iunix -Iunix/bsd43 -Iunix/bsd43/vf -O parse.c
mv parse.o objects
cc -c -I. -Iunix -Iunix/bsd43 -Iunix/bsd43/vf -O getinp.c
mv getinp.o objects
cc -c -I. -Iunix -Iunix/bsd43 -Iunix/bsd43/vf -O quit.c
mv quit.o objects
cc -c -I. -Iunix -Iunix/bsd43 -Iunix/bsd43/vf -O state.c
mv state.o objects
cc -c -I. -Iunix -Iunix/bsd43 -Iunix/bsd43/vf -O dmdump.c
mv dmdump.o objects
cc -c -I. -Iunix -Iunix/bsd43 -Iunix/bsd43/vf -O macparse.c
mv macparse.o objects
cc -c -I. -Iunix -Iunix/bsd43 -Iunix/bsd43/vf -O rulparse.c
mv rulparse.o objects
cc -c -I. -Iunix -Iunix/bsd43 -Iunix/bsd43/vf -O percent.c
mv percent.o objects
cc -c -I. -Iunix -Iunix/bsd43 -Iunix/bsd43/vf -O function.c
mv function.o objects
cc -c -I. -Iunix -Iunix/bsd43 -Iunix/bsd43/vf -O unix/arlib.c
mv arlib.o objects
cc -c -I. -Iunix -Iunix/bsd43 -Iunix/bsd43/vf -O unix/dirbrk.c
mv dirbrk.o objects
cc -c -I. -Iunix -Iunix/bsd43 -Iunix/bsd43/vf -O unix/rmprq.c
mv rmprq.o objects
cc -c -I. -Iunix -Iunix/bsd43 -Iunix/bsd43/vf -O unix/ruletab.c
mv ruletab.o objects
cc -c -I. -Iunix -Iunix/bsd43 -Iunix/bsd43/vf -O unix/runargv.c
mv runargv.o objects
cc -c -I. -Iunix -Iunix/bsd43 -Iunix/bsd43/vf -O unix/dcache.c
mv dcache.o objects
cc -c -I. -Iunix -Iunix/bsd43 -Iunix/bsd43/vf -O unix/bsd43/putenv.c
mv putenv.o objects
cc -c -I. -Iunix -Iunix/bsd43 -Iunix/bsd43/vf -O unix/bsd43/tempnam.c
mv tempnam.o objects
cc -c -I. -Iunix -Iunix/bsd43 -Iunix/bsd43/vf -O unix/bsd43/utime.c
mv utime.o objects
cc -c -I. -Iunix -Iunix/bsd43 -Iunix/bsd43/vf -O unix/bsd43/setvbuf.c
mv setvbuf.o objects
cc -c -I. -Iunix -Iunix/bsd43 -Iunix/bsd43/vf -O unix/bsd43/vf/memcpy.c
mv memcpy.o objects
cc -c -I. -Iunix -Iunix/bsd43 -Iunix/bsd43/vf -O unix/bsd43/vf/vfprintf.c
mv vfprintf.o objects
cc -O -o dmake  objects/infer.o objects/make.o objects/stat.o objects/expand.o \
objects/dmstring.o objects/hash.o objects/dag.o objects/dmake.o objects/path.o \
objects/imacs.o objects/sysintf.o objects/parse.o objects/getinp.o \
objects/quit.o objects/state.o objects/dmdump.o objects/macparse.o \
objects/rulparse.o objects/percent.o objects/function.o objects/arlib.o \
objects/dirbrk.o objects/rmprq.o objects/ruletab.o objects/runargv.o \
objects/dcache.o objects/putenv.o objects/tempnam.o objects/utime.o objects/setvbuf.o objects/memcpy.o objects/vfprintf.o
cp unix/bsd43/vf/template.mk startup/config.mk
