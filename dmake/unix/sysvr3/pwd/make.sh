mkdir objects
cc -c -I. -Iunix -Iunix/sysvr3 -Iunix/sysvr3/pwd -O infer.c
mv infer.o objects
cc -c -I. -Iunix -Iunix/sysvr3 -Iunix/sysvr3/pwd -O make.c
mv make.o objects
cc -c -I. -Iunix -Iunix/sysvr3 -Iunix/sysvr3/pwd -O stat.c
mv stat.o objects
cc -c -I. -Iunix -Iunix/sysvr3 -Iunix/sysvr3/pwd -O expand.c
mv expand.o objects
cc -c -I. -Iunix -Iunix/sysvr3 -Iunix/sysvr3/pwd -O dmstring.c
mv dmstring.o objects
cc -c -I. -Iunix -Iunix/sysvr3 -Iunix/sysvr3/pwd -O hash.c
mv hash.o objects
cc -c -I. -Iunix -Iunix/sysvr3 -Iunix/sysvr3/pwd -O dag.c
mv dag.o objects
cc -c -I. -Iunix -Iunix/sysvr3 -Iunix/sysvr3/pwd -O dmake.c
mv dmake.o objects
cc -c -I. -Iunix -Iunix/sysvr3 -Iunix/sysvr3/pwd -O path.c
mv path.o objects
cc -c -I. -Iunix -Iunix/sysvr3 -Iunix/sysvr3/pwd -O imacs.c
mv imacs.o objects
cc -c -I. -Iunix -Iunix/sysvr3 -Iunix/sysvr3/pwd -O sysintf.c
mv sysintf.o objects
cc -c -I. -Iunix -Iunix/sysvr3 -Iunix/sysvr3/pwd -O parse.c
mv parse.o objects
cc -c -I. -Iunix -Iunix/sysvr3 -Iunix/sysvr3/pwd -O getinp.c
mv getinp.o objects
cc -c -I. -Iunix -Iunix/sysvr3 -Iunix/sysvr3/pwd -O quit.c
mv quit.o objects
cc -c -I. -Iunix -Iunix/sysvr3 -Iunix/sysvr3/pwd -O state.c
mv state.o objects
cc -c -I. -Iunix -Iunix/sysvr3 -Iunix/sysvr3/pwd -O dmdump.c
mv dmdump.o objects
cc -c -I. -Iunix -Iunix/sysvr3 -Iunix/sysvr3/pwd -O macparse.c
mv macparse.o objects
cc -c -I. -Iunix -Iunix/sysvr3 -Iunix/sysvr3/pwd -O rulparse.c
mv rulparse.o objects
cc -c -I. -Iunix -Iunix/sysvr3 -Iunix/sysvr3/pwd -O percent.c
mv percent.o objects
cc -c -I. -Iunix -Iunix/sysvr3 -Iunix/sysvr3/pwd -O function.c
mv function.o objects
cc -c -I. -Iunix -Iunix/sysvr3 -Iunix/sysvr3/pwd -O unix/arlib.c
mv arlib.o objects
cc -c -I. -Iunix -Iunix/sysvr3 -Iunix/sysvr3/pwd -O unix/dirbrk.c
mv dirbrk.o objects
cc -c -I. -Iunix -Iunix/sysvr3 -Iunix/sysvr3/pwd -O unix/rmprq.c
mv rmprq.o objects
cc -c -I. -Iunix -Iunix/sysvr3 -Iunix/sysvr3/pwd -O unix/ruletab.c
mv ruletab.o objects
cc -c -I. -Iunix -Iunix/sysvr3 -Iunix/sysvr3/pwd -O unix/runargv.c
mv runargv.o objects
cc -c -I. -Iunix -Iunix/sysvr3 -Iunix/sysvr3/pwd -O unix/dcache.c
mv dcache.o objects
cc -c -I. -Iunix -Iunix/sysvr3 -Iunix/sysvr3/pwd -O unix/sysvr3/pwd/getcwd.c
mv getcwd.o objects
cc -O -o dmake  objects/infer.o objects/make.o objects/stat.o objects/expand.o \
objects/dmstring.o objects/hash.o objects/dag.o objects/dmake.o objects/path.o \
objects/imacs.o objects/sysintf.o objects/parse.o objects/getinp.o \
objects/quit.o objects/state.o objects/dmdump.o objects/macparse.o \
objects/rulparse.o objects/percent.o objects/function.o objects/arlib.o \
objects/dirbrk.o objects/rmprq.o objects/ruletab.o objects/runargv.o objects/dcache.o objects/getcwd.o
cp unix/sysvr3/pwd/template.mk startup/config.mk
