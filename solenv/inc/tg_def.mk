#*************************************************************************
#
#   $RCSfile: tg_def.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: hjs $ $Date: 2000-11-14 14:23:41 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************



# -------
# - DEF -
# -------
.IF "$(MULTI_DEF_FLAG)" == ""
$(DEF1TARGETN) .NULL : DEF1

$(DEF2TARGETN) .NULL : DEF2

$(DEF3TARGETN) .NULL : DEF3

$(DEF4TARGETN) .NULL : DEF4

$(DEF5TARGETN) .NULL : DEF5

$(DEF6TARGETN) .NULL : DEF6

$(DEF7TARGETN) .NULL : DEF7

$(DEF8TARGETN) .NULL : DEF8

$(DEF9TARGETN) .NULL : DEF9
.ENDIF



.IF "$(MULTI_DEF_FLAG)"==""
DEF1 DEF2 DEF3 DEF4 DEF5 DEF6 DEF7 DEF8 DEF9:  \
                $($@:+"DEPN")
#				$(SLB)$/$($(@:s/DEF/DEFLIB/:+"NAME")).lib
.IF "$(GUI)"=="MAC"
    @dmake "$(DEF$(TNR)TARGETN)" MULTI_DEF_FLAG=true TNR:=$(TNR) $(MFLAGS) $(CALLMACROS)
.ELSE
    @dmake $(DEF$(TNR)TARGETN) MULTI_DEF_FLAG=true TNR:=$(TNR) $(MFLAGS) $(CALLMACROS)
.ENDIF
.ELSE			# "$(MULTI_DEF_FLAG)"==""


#######################################################
# Anweisungen fuer das Linken
# unroll begin

.IF "$(DEF$(TNR)TARGETN)"!=""

.IF "$(DEFLIB$(TNR)NAME)"!=""
.IF "$(UPDATER)"!=""
DEF$(TNR)DEPN+=$(SLB)$/$(DEFLIB$(TNR)NAME).lib
.ENDIF			# "$(UPDATER)"!=""
.ENDIF

.IF "$(GUI)"=="WNT"
.IF "$(APP$(TNR)HEAP)"==""
.IF "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
$(DEF$(TNR)TARGETN) : \
        $(DEF$(TNR)DEPN) \
        $(DEF$(TNR)EXPORTFILE)
.ELSE			# "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
$(DEF$(TNR)TARGETN) .PHONY : \
        $(DEF$(TNR)DEPN) \
        $(DEF$(TNR)EXPORTFILE)
.ENDIF			# "$(UPDATER)"=="" || "$(solarlang)"!="deut" || "$(link_always)"==""
# %_cwd is a 4nt special; don't exppect it to work in any other shell
.IF "$(shell echo %_cwd | sed s/:.*/:/)"=="O:"
#
# don't forget to hav the right DEFSTAG set!
#
    +$(PERL) $(COMMON_ENV_TOOLS)$/cidef.pl update $(DEFSTAG)
.ENDIF			# "$(shell echo %_cwd | sed s/:.*/:/)"=="O:"
    +-attrib -r defs\$(OUTPATH)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo LIBRARY	  $(SHL$(TNR)TARGET) 								 >$@
    @echo DESCRIPTION	'StarView 3.00 $(DEF$(TNR)DES) $(UPD) $(UPDMINOR)' >>$@
    @echo DATA		  READ WRITE SHARED  							>>$@
    @echo HEAPSIZE	  0 											>>$@
    @echo EXPORTS													>>$@
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@
.IF "$(DEFLIB$(TNR)NAME)"!=""
    lib -EXTRACT:/ /OUT:$(SHL$(TNR)TARGET).exp $(SLB)\$(DEFLIB$(TNR)NAME).lib
.IF "$(USE_LDUMP2)"=!""
.IF "$(DEF$(TNR)CEXP)"!=""
    @$(LDUMP2) -A $(DEF$(TNR)CEXP) -E 20 -F $(MISC)\$(SHL$(TNR)TARGET).flt $(SHL$(TNR)TARGET).exp			   >>$@
.ELSE
    @$(LDUMP2) -E 20 -F $(MISC)\$(SHL$(TNR)TARGET).flt $(SHL$(TNR)TARGET).exp			   >>$@
.ENDIF
.ELSE				# "$(USE_LDUMP2)"=!""
    @$(LDUMP) -E20 -F$(MISC)\$(SHL$(TNR)TARGET).flt $(SHL$(TNR)TARGET).exp			   >>$@
.ENDIF				# "$(USE_LDUMP2)"=!""
    +-$(RM) $(SHL$(TNR)TARGET).exp
# now *\defs\$(OUTPATH)	exists, commit it
# %_cwd is a 4nt special; don't exppect it to work in any other shell
.IF "$(shell echo %_cwd | sed s/:.*/:/)"=="O:"
#
# don't forget to hav the right DEFSTAG set!
#
    +$(PERL) $(COMMON_ENV_TOOLS)$/cidef.pl commit
.ENDIF			# "$(shell echo %_cwd | sed s/:.*/:/)"=="O:"
.ENDIF				# "$(DEFLIB$(TNR)NAME)"!=""
.IF "$(DEF$(TNR)EXPORT1)"!=""
    @echo $(DEF$(TNR)EXPORT1)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT2)"!=""
    @echo $(DEF$(TNR)EXPORT2)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT3)"!=""
    @echo $(DEF$(TNR)EXPORT3)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT4)"!=""
    @echo $(DEF$(TNR)EXPORT4)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT5)"!=""
    @echo $(DEF$(TNR)EXPORT5)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT6)"!=""
    @echo $(DEF$(TNR)EXPORT6)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT7)"!=""
    @echo $(DEF$(TNR)EXPORT7)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT8)"!=""
    @echo $(DEF$(TNR)EXPORT8)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT9)"!=""
    @echo $(DEF$(TNR)EXPORT9)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT10)"!=""
    @echo $(DEF$(TNR)EXPORT10)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT11)"!=""
    @echo $(DEF$(TNR)EXPORT11)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT12)"!=""
    @echo $(DEF$(TNR)EXPORT12)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT13)"!=""
    @echo $(DEF$(TNR)EXPORT13)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT14)"!=""
    @echo $(DEF$(TNR)EXPORT14)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT15)"!=""
    @echo $(DEF$(TNR)EXPORT15)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT16)"!=""
    @echo $(DEF$(TNR)EXPORT16)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT17)"!=""
    @echo $(DEF$(TNR)EXPORT17)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT18)"!=""
    @echo $(DEF$(TNR)EXPORT18)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT19)"!=""
    @echo $(DEF$(TNR)EXPORT19)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORT20)"!=""
    @echo $(DEF$(TNR)EXPORT20)										>>$@
.ENDIF
.IF "$(DEF$(TNR)EXPORTFILE)"!=""
    $(TYPE) $(DEF$(TNR)EXPORTFILE) >> $@
.ENDIF
.ENDIF			# "$(APP$(TNR)HEAP)"==""
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="UNX"
$(DEF$(TNR)TARGETN): \
        $(DEF$(TNR)DEPN) \
        $(DEF$(TNR)EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo just a dummy for UNIX
    @echo > $@
.ENDIF
.IF "$(GUI)"=="MAC"
$(DEF$(TNR)TARGETN): \
        $(DEF$(TNR)DEPN) \
        $(DEF$(TNR)EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo fuer MAC nur dummy
    @echo > $@
.ENDIF
.ENDIF			# "$(DEF$(TNR)TARGETN)"!=""


# Anweisungen fuer das Linken
# unroll end
#######################################################

.ENDIF			# "$(MULTI_DEF_FLAG)"==""

