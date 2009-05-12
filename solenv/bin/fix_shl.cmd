/* os2 build scripts

will return a 8.3 conformant name for modname.

21/02/2006 Actually this is a simple truncation, seems nothing more needed.

*/

parse arg modname
if pos('.',modname)>0 then modname = left(modname, pos('.',modname)-1)
say strip(left(modname,8))
