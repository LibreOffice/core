/* os2 build scripts

this script is used to process dxp files produced from .map

*/

do while( lines())

	l = linein()

	l = strip(l)
	l = strip(l,,X2C(9))
	l = strip(l,,";")
	if LEFT( l,4) \= 'Java' THEN l = '_'l

	/* remove empty lines */
	if l = '_' then l = ''

	/* remove component_getDescriptionFunc, since it is already added by tg_def */
	if l = '_component_getDescriptionFunc' then l = ''
	if l = '_GetVersionInfo' then l = ''


	say l

end
