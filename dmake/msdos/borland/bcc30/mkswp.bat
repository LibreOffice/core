md objects
tasm -t -mx -dmlarge msdos\exec.asm,,,;
mv exec.obj objects
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc30 -w-pia  infer.c
copy infer.obj objects
del infer.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc30 -w-pia  make.c
copy make.obj objects
del make.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc30 -w-pia  stat.c
copy stat.obj objects
del stat.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc30 -w-pia  expand.c
copy expand.obj objects
del expand.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc30 -w-pia  dmstring.c
copy dmstring.obj objects
del dmstring.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc30 -w-pia  hash.c
copy hash.obj objects
del hash.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc30 -w-pia  dag.c
copy dag.obj objects
del dag.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc30 -w-pia  dmake.c
copy dmake.obj objects
del dmake.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc30 -w-pia  path.c
copy path.obj objects
del path.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc30 -w-pia  imacs.c
copy imacs.obj objects
del imacs.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc30 -w-pia  sysintf.c
copy sysintf.obj objects
del sysintf.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc30 -w-pia  parse.c
copy parse.obj objects
del parse.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc30 -w-pia  getinp.c
copy getinp.obj objects
del getinp.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc30 -w-pia  quit.c
copy quit.obj objects
del quit.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc30 -w-pia  state.c
copy state.obj objects
del state.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc30 -w-pia  dmdump.c
copy dmdump.obj objects
del dmdump.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc30 -w-pia  macparse.c
copy macparse.obj objects
del macparse.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc30 -w-pia  rulparse.c
copy rulparse.obj objects
del rulparse.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc30 -w-pia  percent.c
copy percent.obj objects
del percent.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc30 -w-pia  function.c
copy function.obj objects
del function.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc30 -w-pia  msdos\ruletab.c
copy ruletab.obj objects
del ruletab.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc30 -w-pia  msdos\dirbrk.c
copy dirbrk.obj objects
del dirbrk.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc30 -w-pia  msdos\runargv.c
copy runargv.obj objects
del runargv.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc30 -w-pia  msdos\arlib.c
copy arlib.obj objects
del arlib.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc30 -w-pia  msdos\dchdir.c
copy dchdir.obj objects
del dchdir.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc30 -w-pia  msdos\switchar.c
copy switchar.obj objects
del switchar.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc30 -w-pia  msdos\rmprq.c
copy rmprq.obj objects
del rmprq.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc30 -w-pia  msdos\spawn.c
copy spawn.obj objects
del spawn.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc30 -w-pia  msdos\find.c
copy find.obj objects
del find.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc30 -w-pia  msdos\dirlib.c
copy dirlib.obj objects
del dirlib.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc30 -w-pia  msdos\dstrlwr.c
copy dstrlwr.obj objects
del dstrlwr.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc30 -w-pia  unix\dcache.c
copy dcache.obj objects
del dcache.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc30 -w-pia  msdos\borland\tempnam.c
copy tempnam.obj objects
del tempnam.obj
bcc -c -I. -Imsdos -Imsdos\borland -d -O -N- -w-nod -ml -Imsdos\borland\bcc30 -w-pia  msdos\borland\utime.c
copy utime.obj objects
del utime.obj
tlink   @msdos\borland\bcc30\objswp.rsp,dmake.exe,NUL.MAP,@msdos\borland\bcc30\libswp.rsp
copy msdos\borland\bcc30\template.mk startup\config.mk
