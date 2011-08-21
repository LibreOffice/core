mkdir objects
gcc -c -I. -Itos -O infer.c
mv infer.o objects
gcc -c -I. -Itos -O make.c
mv make.o objects
gcc -c -I. -Itos -O stat.c
mv stat.o objects
gcc -c -I. -Itos -O expand.c
mv expand.o objects
gcc -c -I. -Itos -O dmstring.c
mv dmstring.o objects
gcc -c -I. -Itos -O hash.c
mv hash.o objects
gcc -c -I. -Itos -O dag.c
mv dag.o objects
gcc -c -I. -Itos -O dmake.c
mv dmake.o objects
gcc -c -I. -Itos -O path.c
mv path.o objects
gcc -c -I. -Itos -O imacs.c
mv imacs.o objects
gcc -c -I. -Itos -O sysintf.c
mv sysintf.o objects
gcc -c -I. -Itos -O parse.c
mv parse.o objects
gcc -c -I. -Itos -O getinp.c
mv getinp.o objects
gcc -c -I. -Itos -O quit.c
mv quit.o objects
gcc -c -I. -Itos -O state.c
mv state.o objects
gcc -c -I. -Itos -O dmdump.c
mv dmdump.o objects
gcc -c -I. -Itos -O macparse.c
mv macparse.o objects
gcc -c -I. -Itos -O rulparse.c
mv rulparse.o objects
gcc -c -I. -Itos -O percent.c
mv percent.o objects
gcc -c -I. -Itos -O function.c
mv function.o objects
gcc -c -I. -Itos -O tos/ruletab.c
mv ruletab.o objects
gcc -c -I. -Itos -O msdos/rmprq.c
mv rmprq.o objects
gcc -c -I. -Itos -O msdos/runargv.c
mv runargv.o objects
gcc -c -I. -Itos -O msdos/dirbrk.c
mv dirbrk.o objects
gcc -c -I. -Itos -O unix/arlib.c
mv arlib.o objects
gcc -c -I. -Itos -O unix/bsd43/putenv.c
mv putenv.o objects
gcc -c -I. -Itos -O unix/bsd43/tempnam.c
mv tempnam.o objects
gcc -s  -o dmake objects/*.o
cp tos/template.mk startup/config.mk
