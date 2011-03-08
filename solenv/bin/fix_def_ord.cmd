/* os2 build scripts

this script is used to process def results.
Adds ordinal number to every line.

*/

lmax = 0
smax = ''

ord = 1
do while( lines())

	l = linein()
	IF LENGTH(l)>0 THEN DO
  	  say l 
	  ord = ord + 1
	END
end
