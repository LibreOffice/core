if "%1" != "" goto link 
md objects
cl -c %c_flg -I. -Iwin95 -Iwin95\microsft -Iwin95\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\infer.obj infer.c
cl -c %c_flg -I. -Iwin95 -Iwin95\microsft -Iwin95\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\make.obj make.c
cl -c %c_flg -I. -Iwin95 -Iwin95\microsft -Iwin95\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\stat.obj stat.c
cl -c %c_flg -I. -Iwin95 -Iwin95\microsft -Iwin95\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\expand.obj expand.c
cl -c %c_flg -I. -Iwin95 -Iwin95\microsft -Iwin95\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\dmstring.obj dmstring.c
cl -c %c_flg -I. -Iwin95 -Iwin95\microsft -Iwin95\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\hash.obj hash.c
cl -c %c_flg -I. -Iwin95 -Iwin95\microsft -Iwin95\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\dag.obj dag.c
cl -c %c_flg -I. -Iwin95 -Iwin95\microsft -Iwin95\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\dmake.obj dmake.c
cl -c %c_flg -I. -Iwin95 -Iwin95\microsft -Iwin95\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\path.obj path.c
cl -c %c_flg -I. -Iwin95 -Iwin95\microsft -Iwin95\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\imacs.obj imacs.c
cl -c %c_flg -I. -Iwin95 -Iwin95\microsft -Iwin95\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\sysintf.obj sysintf.c
cl -c %c_flg -I. -Iwin95 -Iwin95\microsft -Iwin95\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\parse.obj parse.c
cl -c %c_flg -I. -Iwin95 -Iwin95\microsft -Iwin95\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\getinp.obj getinp.c
cl -c %c_flg -I. -Iwin95 -Iwin95\microsft -Iwin95\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\quit.obj quit.c
cl -c %c_flg -I. -Iwin95 -Iwin95\microsft -Iwin95\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\state.obj state.c
cl -c %c_flg -I. -Iwin95 -Iwin95\microsft -Iwin95\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\dmdump.obj dmdump.c
cl -c %c_flg -I. -Iwin95 -Iwin95\microsft -Iwin95\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\macparse.obj macparse.c
cl -c %c_flg -I. -Iwin95 -Iwin95\microsft -Iwin95\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\rulparse.obj rulparse.c
cl -c %c_flg -I. -Iwin95 -Iwin95\microsft -Iwin95\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\percent.obj percent.c
cl -c %c_flg -I. -Iwin95 -Iwin95\microsft -Iwin95\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\function.obj function.c
cl -c %c_flg -I. -Iwin95 -Iwin95\microsft -Iwin95\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\dchdir.obj win95\dchdir.c
cl -c %c_flg -I. -Iwin95 -Iwin95\microsft -Iwin95\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\switchar.obj win95\switchar.c
cl -c %c_flg -I. -Iwin95 -Iwin95\microsft -Iwin95\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\dstrlwr.obj msdos\dstrlwr.c
cl -c %c_flg -I. -Iwin95 -Iwin95\microsft -Iwin95\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\arlib.obj msdos\arlib.c
cl -c %c_flg -I. -Iwin95 -Iwin95\microsft -Iwin95\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\dirbrk.obj msdos\dirbrk.c
cl -c %c_flg -I. -Iwin95 -Iwin95\microsft -Iwin95\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\tempnam.obj tempnam.c
cl -c %c_flg -I. -Iwin95 -Iwin95\microsft -Iwin95\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\ruletab.obj win95\microsft\ruletab.c
cl -c %c_flg -I. -Iwin95 -Iwin95\microsft -Iwin95\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\runargv.obj msdos\runargv.c
cl -c %c_flg -I. -Iwin95 -Iwin95\microsft -Iwin95\microsft\vpp40 /nologo -Od -GF -Ge -Foobjects\rmprq.obj msdos\rmprq.c

:link
rem link /nologo /out:dmake.exe @fix95nt\win95\microsft\vpp40\obj.rsp
if "%c_flg" == "" link /out:dmake.exe @.\win95\microsft\vpp40\obj.rsp
if "%c_flg" != "" link /DEBUG:notmapped,full /DEBUGTYPE:cv /PDB:NONE /out:dmake.exe @.\win95\microsft\vpp40\obj.rsp
copy win95\microsft\vpp40\template.mk startup\config.mk
