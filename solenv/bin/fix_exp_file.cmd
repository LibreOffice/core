/* os2 build scripts

this script is used to process emxexp results

*/

lmax = 0
ordinal = 1

do while( lines())

	l = strip(linein())
	if POS(';', l) > 0 then l = LEFT(l,POS(';', l)-1)

	l = strip(translate(l,'','"'))
	l = strip(l,,X2C(9))

	/* remove empty lines */
	if l = '_' then l = ''

	/* remove component_getDescriptionFunc, since it is already added by tg_def */
	if l = '_component_getDescriptionFunc' then l = ''
	if l = '_GetVersionInfo' then l = ''


	/* remove GLOBAL symbols */
	if POS('_GLOBAL_', l) > 0 then l = ';'l

	if POS(';', l) > 0 then l = LEFT(l,POS(';', l)-1)

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
