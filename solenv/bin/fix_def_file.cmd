/* os2 build scripts

this script is used to process def results

*/

lmax = 0
smax = ''
ordinal = 1

do while( lines())

	l = strip(linein())
	l = strip(l,,X2C(9))
	l = strip(l,,";")
	if LEFT( l,4) \= 'Java' THEN l = '_'l

	/* remove comments */
	if POS(';', l) > 0 then l = LEFT( l, POS(';', l)-1)
	if POS('#', l) > 0 then l = LEFT( l, POS('#', l)-1)
	/* remove wildcards */
	if POS('*', l) > 0 then l = ''

	/* remove empty lines */
	if l = '_' then l = ''

	/* remove component_getDescriptionFunc, since it is already added by tg_def */
	if l = '_component_getDescriptionFunc' then l = ''
	if l = '_GetVersionInfo' then l = ''

	/* remove GLOBAL symbols */

	IF LENGTH(l)>0 THEN DO
	  say l
  	  ordinal = ordinal + 1
	END

	if LENGTH(l)>lmax then do
          lmax = LENGTH(l)
          smax = l
        end

end

say ';lmax='lmax
say ';smax='smax
