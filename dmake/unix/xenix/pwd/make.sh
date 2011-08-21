mkdir objects
cc -c -I. -Iunix -Iunix/xenix -DM_XENIX -Iunix/xenix/pwd -O infer.c
mv infer.o objects
cc -c -I. -Iunix -Iunix/xenix -DM_XENIX -Iunix/xenix/pwd -O make.c
mv make.o objects
cc -c -I. -Iunix -Iunix/xenix -DM_XENIX -Iunix/xenix/pwd -O stat.c
mv stat.o objects
cc -c -I. -Iunix -Iunix/xenix -DM_XENIX -Iunix/xenix/pwd -O expand.c
mv expand.o objects
cc -c -I. -Iunix -Iunix/xenix -DM_XENIX -Iunix/xenix/pwd -O dmstring.c
mv dmstring.o objects
cc -c -I. -Iunix -Iunix/xenix -DM_XENIX -Iunix/xenix/pwd -O hash.c
mv hash.o objects
cc -c -I. -Iunix -Iunix/xenix -DM_XENIX -Iunix/xenix/pwd -O dag.c
mv dag.o objects
cc -c -I. -Iunix -Iunix/xenix -DM_XENIX -Iunix/xenix/pwd -O dmake.c
mv dmake.o objects
cc -c -I. -Iunix -Iunix/xenix -DM_XENIX -Iunix/xenix/pwd -O path.c
mv path.o objects
cc -c -I. -Iunix -Iunix/xenix -DM_XENIX -Iunix/xenix/pwd -O imacs.c
mv imacs.o objects
cc -c -I. -Iunix -Iunix/xenix -DM_XENIX -Iunix/xenix/pwd -O sysintf.c
mv sysintf.o objects
cc -c -I. -Iunix -Iunix/xenix -DM_XENIX -Iunix/xenix/pwd -O parse.c
mv parse.o objects
cc -c -I. -Iunix -Iunix/xenix -DM_XENIX -Iunix/xenix/pwd -O getinp.c
mv getinp.o objects
cc -c -I. -Iunix -Iunix/xenix -DM_XENIX -Iunix/xenix/pwd -O quit.c
mv quit.o objects
cc -c -I. -Iunix -Iunix/xenix -DM_XENIX -Iunix/xenix/pwd -O state.c
mv state.o objects
cc -c -I. -Iunix -Iunix/xenix -DM_XENIX -Iunix/xenix/pwd -O dmdump.c
mv dmdump.o objects
cc -c -I. -Iunix -Iunix/xenix -DM_XENIX -Iunix/xenix/pwd -O macparse.c
mv macparse.o objects
cc -c -I. -Iunix -Iunix/xenix -DM_XENIX -Iunix/xenix/pwd -O rulparse.c
mv rulparse.o objects
cc -c -I. -Iunix -Iunix/xenix -DM_XENIX -Iunix/xenix/pwd -O percent.c
mv percent.o objects
cc -c -I. -Iunix -Iunix/xenix -DM_XENIX -Iunix/xenix/pwd -O function.c
mv function.o objects
cc -c -I. -Iunix -Iunix/xenix -DM_XENIX -Iunix/xenix/pwd -O unix/arlib.c
mv arlib.o objects
cc -c -I. -Iunix -Iunix/xenix -DM_XENIX -Iunix/xenix/pwd -O unix/dirbrk.c
mv dirbrk.o objects
cc -c -I. -Iunix -Iunix/xenix -DM_XENIX -Iunix/xenix/pwd -O unix/rmprq.c
mv rmprq.o objects
cc -c -I. -Iunix -Iunix/xenix -DM_XENIX -Iunix/xenix/pwd -O unix/ruletab.c
mv ruletab.o objects
cc -c -I. -Iunix -Iunix/xenix -DM_XENIX -Iunix/xenix/pwd -O unix/runargv.c
mv runargv.o objects
cc -c -I. -Iunix -Iunix/xenix -DM_XENIX -Iunix/xenix/pwd -O unix/dcache.c
mv dcache.o objects
cc -c -I. -Iunix -Iunix/xenix -DM_XENIX -Iunix/xenix/pwd -O unix/xenix/pwd/getcwd.c
mv getcwd.o objects
cc -O -o dmake  objects/infer.o objects/make.o objects/stat.o objects/expand.o \
objects/dmstring.o objects/hash.o objects/dag.o objects/dmake.o objects/path.o \
objects/imacs.o objects/sysintf.o objects/parse.o objects/getinp.o \
objects/quit.o objects/state.o objects/dmdump.o objects/macparse.o \
objects/rulparse.o objects/percent.o objects/function.o objects/arlib.o \
objects/dirbrk.o objects/rmprq.o objects/ruletab.o objects/runargv.o objects/dcache.o objects/getcwd.o -lx
cp unix/xenix/pwd/template.mk startup/config.mk
