md objects
cl -c -I. -Iwinnt -Iwinnt\microsft -Iwinnt\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\infer.obj infer.c
cl -c -I. -Iwinnt -Iwinnt\microsft -Iwinnt\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\make.obj make.c
cl -c -I. -Iwinnt -Iwinnt\microsft -Iwinnt\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\stat.obj stat.c
cl -c -I. -Iwinnt -Iwinnt\microsft -Iwinnt\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\expand.obj expand.c
cl -c -I. -Iwinnt -Iwinnt\microsft -Iwinnt\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\dmstring.obj dmstring.c
cl -c -I. -Iwinnt -Iwinnt\microsft -Iwinnt\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\hash.obj hash.c
cl -c -I. -Iwinnt -Iwinnt\microsft -Iwinnt\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\dag.obj dag.c
cl -c -I. -Iwinnt -Iwinnt\microsft -Iwinnt\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\dmake.obj dmake.c
cl -c -I. -Iwinnt -Iwinnt\microsft -Iwinnt\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\path.obj path.c
cl -c -I. -Iwinnt -Iwinnt\microsft -Iwinnt\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\imacs.obj imacs.c
cl -c -I. -Iwinnt -Iwinnt\microsft -Iwinnt\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\sysintf.obj sysintf.c
cl -c -I. -Iwinnt -Iwinnt\microsft -Iwinnt\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\parse.obj parse.c
cl -c -I. -Iwinnt -Iwinnt\microsft -Iwinnt\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\getinp.obj getinp.c
cl -c -I. -Iwinnt -Iwinnt\microsft -Iwinnt\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\quit.obj quit.c
cl -c -I. -Iwinnt -Iwinnt\microsft -Iwinnt\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\state.obj state.c
cl -c -I. -Iwinnt -Iwinnt\microsft -Iwinnt\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\dmdump.obj dmdump.c
cl -c -I. -Iwinnt -Iwinnt\microsft -Iwinnt\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\macparse.obj macparse.c
cl -c -I. -Iwinnt -Iwinnt\microsft -Iwinnt\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\rulparse.obj rulparse.c
cl -c -I. -Iwinnt -Iwinnt\microsft -Iwinnt\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\percent.obj percent.c
cl -c -I. -Iwinnt -Iwinnt\microsft -Iwinnt\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\function.obj function.c
cl -c -I. -Iwinnt -Iwinnt\microsft -Iwinnt\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\dchdir.obj winnt\dchdir.c
cl -c -I. -Iwinnt -Iwinnt\microsft -Iwinnt\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\dstrlwr.obj msdos\dstrlwr.c
cl -c -I. -Iwinnt -Iwinnt\microsft -Iwinnt\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\arlib.obj msdos\arlib.c
cl -c -I. -Iwinnt -Iwinnt\microsft -Iwinnt\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\dirbrk.obj msdos\dirbrk.c
cl -c -I. -Iwinnt -Iwinnt\microsft -Iwinnt\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\tempnam.obj winnt\microsft\tempnam.c
cl -c -I. -Iwinnt -Iwinnt\microsft -Iwinnt\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\ruletab.obj winnt\microsft\ruletab.c
cl -c -I. -Iwinnt -Iwinnt\microsft -Iwinnt\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\runargv.obj msdos\runargv.c
cl -c -I. -Iwinnt -Iwinnt\microsft -Iwinnt\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\rmprq.obj msdos\rmprq.c
link /nologo /out:dmake.exe @fix95nt\winnt\microsft\vpp40\obj.rsp 
copy winnt\microsft\vpp40\template.mk startup\config.mk
