md objects
tasm -t -mx -dmlarge msdos\exec.asm,,,;
mv exec.obj objects
tcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\tcc20 -f-  infer.c
copy infer.obj objects
del infer.obj
tcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\tcc20 -f-  make.c
copy make.obj objects
del make.obj
tcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\tcc20 -f-  stat.c
copy stat.obj objects
del stat.obj
tcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\tcc20 -f-  expand.c
copy expand.obj objects
del expand.obj
tcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\tcc20 -f-  dmstring.c
copy dmstring.obj objects
del dmstring.obj
tcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\tcc20 -f-  hash.c
copy hash.obj objects
del hash.obj
tcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\tcc20 -f-  dag.c
copy dag.obj objects
del dag.obj
tcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\tcc20 -f-  dmake.c
copy dmake.obj objects
del dmake.obj
tcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\tcc20 -f-  path.c
copy path.obj objects
del path.obj
tcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\tcc20 -f-  imacs.c
copy imacs.obj objects
del imacs.obj
tcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\tcc20 -f-  sysintf.c
copy sysintf.obj objects
del sysintf.obj
tcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\tcc20 -f-  parse.c
copy parse.obj objects
del parse.obj
tcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\tcc20 -f-  getinp.c
copy getinp.obj objects
del getinp.obj
tcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\tcc20 -f-  quit.c
copy quit.obj objects
del quit.obj
tcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\tcc20 -f-  state.c
copy state.obj objects
del state.obj
tcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\tcc20 -f-  dmdump.c
copy dmdump.obj objects
del dmdump.obj
tcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\tcc20 -f-  macparse.c
copy macparse.obj objects
del macparse.obj
tcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\tcc20 -f-  rulparse.c
copy rulparse.obj objects
del rulparse.obj
tcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\tcc20 -f-  percent.c
copy percent.obj objects
del percent.obj
tcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\tcc20 -f-  function.c
copy function.obj objects
del function.obj
tcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\tcc20 -f-  msdos\ruletab.c
copy ruletab.obj objects
del ruletab.obj
tcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\tcc20 -f-  msdos\dirbrk.c
copy dirbrk.obj objects
del dirbrk.obj
tcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\tcc20 -f-  msdos\runargv.c
copy runargv.obj objects
del runargv.obj
tcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\tcc20 -f-  msdos\arlib.c
copy arlib.obj objects
del arlib.obj
tcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\tcc20 -f-  msdos\dchdir.c
copy dchdir.obj objects
del dchdir.obj
tcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\tcc20 -f-  msdos\switchar.c
copy switchar.obj objects
del switchar.obj
tcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\tcc20 -f-  msdos\rmprq.c
copy rmprq.obj objects
del rmprq.obj
tcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\tcc20 -f-  msdos\spawn.c
copy spawn.obj objects
del spawn.obj
tcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\tcc20 -f-  msdos\find.c
copy find.obj objects
del find.obj
tcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\tcc20 -f-  msdos\dirlib.c
copy dirlib.obj objects
del dirlib.obj
tcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\tcc20 -f-  msdos\dstrlwr.c
copy dstrlwr.obj objects
del dstrlwr.obj
tcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\tcc20 -f-  unix\dcache.c
copy dcache.obj objects
del dcache.obj
tcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\tcc20 -f-  msdos\borland\tempnam.c
copy tempnam.obj objects
del tempnam.obj
tcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\tcc20 -f-  msdos\borland\utime.c
copy utime.obj objects
del utime.obj
tlink  /c @msdos\borland\tcc20\objswp.rsp,dmake.exe,NUL.MAP,@msdos\borland\tcc20\libswp.rsp
copy msdos\borland\tcc20\template.mk startup\config.mk
