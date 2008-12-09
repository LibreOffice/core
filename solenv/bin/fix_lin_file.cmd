/* os2 build scripts
*/

parse arg dir

do while( lines())

	l = linein()

	/* skip empty lines */
	if l \= '' then say dir || '\' || l

end
