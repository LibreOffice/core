mkdir objects
gcc -c -I. -Iunix -Iunix/solaris -DSolaris -O infer.c
mv infer.o objects
gcc -c -I. -Iunix -Iunix/solaris -DSolaris -O make.c
mv make.o objects
gcc -c -I. -Iunix -Iunix/solaris -DSolaris -O stat.c
mv stat.o objects
gcc -c -I. -Iunix -Iunix/solaris -DSolaris -O expand.c
mv expand.o objects
gcc -c -I. -Iunix -Iunix/solaris -DSolaris -O dmstring.c
mv dmstring.o objects
gcc -c -I. -Iunix -Iunix/solaris -DSolaris -O hash.c
mv hash.o objects
gcc -c -I. -Iunix -Iunix/solaris -DSolaris -O dag.c
mv dag.o objects
gcc -c -I. -Iunix -Iunix/solaris -DSolaris -O dmake.c
mv dmake.o objects
gcc -c -I. -Iunix -Iunix/solaris -DSolaris -O path.c
mv path.o objects
gcc -c -I. -Iunix -Iunix/solaris -DSolaris -O imacs.c
mv imacs.o objects
gcc -c -I. -Iunix -Iunix/solaris -DSolaris -O sysintf.c
mv sysintf.o objects
gcc -c -I. -Iunix -Iunix/solaris -DSolaris -O parse.c
mv parse.o objects
gcc -c -I. -Iunix -Iunix/solaris -DSolaris -O getinp.c
mv getinp.o objects
gcc -c -I. -Iunix -Iunix/solaris -DSolaris -O quit.c
mv quit.o objects
gcc -c -I. -Iunix -Iunix/solaris -DSolaris -O state.c
mv state.o objects
gcc -c -I. -Iunix -Iunix/solaris -DSolaris -O dmdump.c
mv dmdump.o objects
gcc -c -I. -Iunix -Iunix/solaris -DSolaris -O macparse.c
mv macparse.o objects
gcc -c -I. -Iunix -Iunix/solaris -DSolaris -O rulparse.c
mv rulparse.o objects
gcc -c -I. -Iunix -Iunix/solaris -DSolaris -O percent.c
mv percent.o objects
gcc -c -I. -Iunix -Iunix/solaris -DSolaris -O function.c
mv function.o objects
gcc -c -I. -Iunix -Iunix/solaris -DSolaris -O unix/arlib.c
mv arlib.o objects
gcc -c -I. -Iunix -Iunix/solaris -DSolaris -O unix/dirbrk.c
mv dirbrk.o objects
gcc -c -I. -Iunix -Iunix/solaris -DSolaris -O unix/rmprq.c
mv rmprq.o objects
gcc -c -I. -Iunix -Iunix/solaris -DSolaris -O unix/ruletab.c
mv ruletab.o objects
gcc -c -I. -Iunix -Iunix/solaris -DSolaris -O -g unix/runargv.c
mv runargv.o objects
gcc -c -I. -Iunix -Iunix/solaris -DSolaris -O unix/dcache.c
mv dcache.o objects
gcc -c -I. -Iunix -Iunix/solaris -DSolaris -O unix/solaris/tempnam.c
mv tempnam.o objects
gcc -c -I. -Iunix -Iunix/solaris -DSolaris -O unix/solaris/getcwd.c
mv getcwd.o objects
gcc -O -o dmake  objects/infer.o objects/make.o objects/stat.o objects/expand.o \
objects/dmstring.o objects/hash.o objects/dag.o objects/dmake.o objects/path.o \
objects/imacs.o objects/sysintf.o objects/parse.o objects/getinp.o \
objects/quit.o objects/state.o objects/dmdump.o objects/macparse.o \
objects/rulparse.o objects/percent.o objects/function.o objects/arlib.o \
objects/dirbrk.o objects/rmprq.o objects/ruletab.o objects/runargv.o objects/dcache.o objects/tempnam.o objects/getcwd.o
cp unix/solaris/gnu/template.mk startup/config.mk
