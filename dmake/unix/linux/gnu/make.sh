platform=`uname -m`; export platform;

mkdir objects

if test $platform = sparc -o $platform = sparc64; then
gcc -c -ansi -I. -Iunix -Iunix/linux -Iunix/linux/gnu -O infer.c
else
gcc -c -I. -Iunix -Iunix/linux -Iunix/linux/gnu -O infer.c
fi
mv infer.o objects

if test $platform = sparc -o $platform = sparc64; then
gcc -c -ansi -I. -Iunix -Iunix/linux -Iunix/linux/gnu -O make.c
else
gcc -c -I. -Iunix -Iunix/linux -Iunix/linux/gnu -O make.c
fi
mv make.o objects

if test $platform = sparc -o $platform = sparc64; then
gcc -c -ansi -I. -Iunix -Iunix/linux -Iunix/linux/gnu -O stat.c
else
gcc -c -I. -Iunix -Iunix/linux -Iunix/linux/gnu -O stat.c
fi
mv stat.o objects

if test $platform = sparc -o $platform = sparc64; then
gcc -c -ansi -I. -Iunix -Iunix/linux -Iunix/linux/gnu -O expand.c
else
gcc -c -I. -Iunix -Iunix/linux -Iunix/linux/gnu -O expand.c
fi
mv expand.o objects

if test $platform = sparc -o $platform = sparc64; then
gcc -c -ansi -I. -Iunix -Iunix/linux -Iunix/linux/gnu -O dmstring.c
else
gcc -c -I. -Iunix -Iunix/linux -Iunix/linux/gnu -O dmstring.c
fi
mv dmstring.o objects

if test $platform = sparc -o $platform = sparc64; then
gcc -c -ansi -I. -Iunix -Iunix/linux -Iunix/linux/gnu -O hash.c
else
gcc -c -I. -Iunix -Iunix/linux -Iunix/linux/gnu -O hash.c
fi
mv hash.o objects

if test $platform = sparc -o $platform = sparc64; then
gcc -c -ansi -I. -Iunix -Iunix/linux -Iunix/linux/gnu -O dag.c
else
gcc -c -I. -Iunix -Iunix/linux -Iunix/linux/gnu -O dag.c
fi
mv dag.o objects

if test $platform = sparc -o $platform = sparc64; then
gcc -c -ansi -I. -Iunix -Iunix/linux -Iunix/linux/gnu -O dmake.c
else
gcc -c -I. -Iunix -Iunix/linux -Iunix/linux/gnu -O dmake.c
fi
mv dmake.o objects

if test $platform = sparc -o $platform = sparc64; then
gcc -c -ansi -I. -Iunix -Iunix/linux -Iunix/linux/gnu -O path.c
else
gcc -c -I. -Iunix -Iunix/linux -Iunix/linux/gnu -O path.c
fi
mv path.o objects

if test $platform = sparc -o $platform = sparc64; then
gcc -c -ansi -I. -Iunix -Iunix/linux -Iunix/linux/gnu -O imacs.c
else
gcc -c -I. -Iunix -Iunix/linux -Iunix/linux/gnu -O imacs.c
fi
mv imacs.o objects

if test $platform = sparc -o $platform = sparc64; then
gcc -c -ansi -I. -Iunix -Iunix/linux -Iunix/linux/gnu -O sysintf.c
else
gcc -c -I. -Iunix -Iunix/linux -Iunix/linux/gnu -O sysintf.c
fi
mv sysintf.o objects

if test $platform = sparc -o $platform = sparc64; then
gcc -c -ansi -I. -Iunix -Iunix/linux -Iunix/linux/gnu -O parse.c
else
gcc -c -I. -Iunix -Iunix/linux -Iunix/linux/gnu -O parse.c
fi
mv parse.o objects

if test $platform = sparc -o $platform = sparc64; then
gcc -c -ansi -I. -Iunix -Iunix/linux -Iunix/linux/gnu -O getinp.c
else
gcc -c -I. -Iunix -Iunix/linux -Iunix/linux/gnu -O getinp.c
fi
mv getinp.o objects

if test $platform = sparc -o $platform = sparc64; then
gcc -c -ansi -I. -Iunix -Iunix/linux -Iunix/linux/gnu -O quit.c
else
gcc -c -I. -Iunix -Iunix/linux -Iunix/linux/gnu -O quit.c
fi
mv quit.o objects

if test $platform = sparc -o $platform = sparc64; then
gcc -c -ansi -I. -Iunix -Iunix/linux -Iunix/linux/gnu -O state.c
else
gcc -c -I. -Iunix -Iunix/linux -Iunix/linux/gnu -O state.c
fi
mv state.o objects

if test $platform = sparc -o $platform = sparc64; then
gcc -c -ansi -I. -Iunix -Iunix/linux -Iunix/linux/gnu -O dmdump.c
else
gcc -c -I. -Iunix -Iunix/linux -Iunix/linux/gnu -O dmdump.c
fi
mv dmdump.o objects

if test $platform = sparc -o $platform = sparc64; then
gcc -c -ansi -I. -Iunix -Iunix/linux -Iunix/linux/gnu -O macparse.c
else
gcc -c -I. -Iunix -Iunix/linux -Iunix/linux/gnu -O macparse.c
fi
mv macparse.o objects

if test $platform = sparc -o $platform = sparc64; then
gcc -c -ansi -I. -Iunix -Iunix/linux -Iunix/linux/gnu -O rulparse.c
else
gcc -c -I. -Iunix -Iunix/linux -Iunix/linux/gnu -O rulparse.c
fi
mv rulparse.o objects

if test $platform = sparc -o $platform = sparc64; then
gcc -c -ansi -I. -Iunix -Iunix/linux -Iunix/linux/gnu -O percent.c
else
gcc -c -I. -Iunix -Iunix/linux -Iunix/linux/gnu -O percent.c
fi
mv percent.o objects

if test $platform = sparc -o $platform = sparc64; then
gcc -c -ansi -I. -Iunix -Iunix/linux -Iunix/linux/gnu -O function.c
else
gcc -c -I. -Iunix -Iunix/linux -Iunix/linux/gnu -O function.c
fi
mv function.o objects

if test $platform = sparc -o $platform = sparc64; then
gcc -c -ansi -I. -Iunix -Iunix/linux -Iunix/linux/gnu -O unix/arlib.c
else
gcc -c -I. -Iunix -Iunix/linux -Iunix/linux/gnu -O unix/arlib.c
fi
mv arlib.o objects

if test $platform = sparc -o $platform = sparc64; then
gcc -c -ansi -I. -Iunix -Iunix/linux -Iunix/linux/gnu -O unix/dirbrk.c
else
gcc -c -I. -Iunix -Iunix/linux -Iunix/linux/gnu -O unix/dirbrk.c
fi
mv dirbrk.o objects

if test $platform = sparc -o $platform = sparc64; then
gcc -c -ansi -I. -Iunix -Iunix/linux -Iunix/linux/gnu -O unix/rmprq.c
else
gcc -c -I. -Iunix -Iunix/linux -Iunix/linux/gnu -O unix/rmprq.c
fi
mv rmprq.o objects

if test $platform = sparc -o $platform = sparc64; then
gcc -c -ansi -I. -Iunix -Iunix/linux -Iunix/linux/gnu -O unix/ruletab.c
else
gcc -c -I. -Iunix -Iunix/linux -Iunix/linux/gnu -O unix/ruletab.c
fi
mv ruletab.o objects

if test $platform = sparc -o $platform = sparc64; then
gcc -c -ansi -I. -Iunix -Iunix/linux -Iunix/linux/gnu -O unix/runargv.c
else
gcc -c -I. -Iunix -Iunix/linux -Iunix/linux/gnu -O unix/runargv.c
fi
mv runargv.o objects

if test $platform = sparc -o $platform = sparc64; then
gcc -c -ansi -I. -Iunix -Iunix/linux -Iunix/linux/gnu -O unix/dcache.c
else
gcc -c -I. -Iunix -Iunix/linux -Iunix/linux/gnu -O unix/dcache.c
fi
mv dcache.o objects

gcc -O -o dmake  objects/infer.o objects/make.o objects/stat.o objects/expand.o \
objects/dmstring.o objects/hash.o objects/dag.o objects/dmake.o objects/path.o \
objects/imacs.o objects/sysintf.o objects/parse.o objects/getinp.o \
objects/quit.o objects/state.o objects/dmdump.o objects/macparse.o \
objects/rulparse.o objects/percent.o objects/function.o objects/arlib.o \
objects/dirbrk.o objects/rmprq.o objects/ruletab.o objects/runargv.o objects/dcache.o
cp unix/linux/gnu/template.mk startup/config.mk
