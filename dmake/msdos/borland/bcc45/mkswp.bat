md objects
tasm -t -mx -dmlarge msdos\exec.asm,,,;
mv exec.obj objects
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc45 -w-pro  infer.c
copy infer.obj objects
del infer.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc45 -w-pro  make.c
copy make.obj objects
del make.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc45 -w-pro  stat.c
copy stat.obj objects
del stat.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc45 -w-pro  expand.c
copy expand.obj objects
del expand.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc45 -w-pro  dmstring.c
copy dmstring.obj objects
del dmstring.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc45 -w-pro  hash.c
copy hash.obj objects
del hash.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc45 -w-pro  dag.c
copy dag.obj objects
del dag.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc45 -w-pro  dmake.c
copy dmake.obj objects
del dmake.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc45 -w-pro  path.c
copy path.obj objects
del path.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc45 -w-pro  imacs.c
copy imacs.obj objects
del imacs.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc45 -w-pro  sysintf.c
copy sysintf.obj objects
del sysintf.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc45 -w-pro  parse.c
copy parse.obj objects
del parse.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc45 -w-pro  getinp.c
copy getinp.obj objects
del getinp.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc45 -w-pro  quit.c
copy quit.obj objects
del quit.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc45 -w-pro  state.c
copy state.obj objects
del state.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc45 -w-pro  dmdump.c
copy dmdump.obj objects
del dmdump.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc45 -w-pro  macparse.c
copy macparse.obj objects
del macparse.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc45 -w-pro  rulparse.c
copy rulparse.obj objects
del rulparse.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc45 -w-pro  percent.c
copy percent.obj objects
del percent.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc45 -w-pro  function.c
copy function.obj objects
del function.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc45 -w-pro  msdos\ruletab.c
copy ruletab.obj objects
del ruletab.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc45 -w-pro  msdos\dirbrk.c
copy dirbrk.obj objects
del dirbrk.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc45 -w-pro  msdos\runargv.c
copy runargv.obj objects
del runargv.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc45 -w-pro  msdos\arlib.c
copy arlib.obj objects
del arlib.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc45 -w-pro  msdos\dchdir.c
copy dchdir.obj objects
del dchdir.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc45 -w-pro  msdos\switchar.c
copy switchar.obj objects
del switchar.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc45 -w-pro  msdos\rmprq.c
copy rmprq.obj objects
del rmprq.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc45 -w-pro  msdos\spawn.c
copy spawn.obj objects
del spawn.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc45 -w-pro  msdos\find.c
copy find.obj objects
del find.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc45 -w-pro  msdos\dirlib.c
copy dirlib.obj objects
del dirlib.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc45 -w-pro  msdos\dstrlwr.c
copy dstrlwr.obj objects
del dstrlwr.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc45 -w-pro  unix\dcache.c
copy dcache.obj objects
del dcache.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc45 -w-pro  msdos\borland\tempnam.c
copy tempnam.obj objects
del tempnam.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc45 -w-pro  msdos\borland\utime.c
copy utime.obj objects
del utime.obj
tlink   @msdos\borland\bcc45\objswp.rsp,dmake.exe,NUL.MAP,@msdos\borland\bcc45\libswp.rsp
copy msdos\borland\bcc45\template.mk startup\config.mk
