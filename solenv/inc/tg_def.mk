#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: tg_def.mk,v $
#
# $Revision: 1.49 $
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

#######################################################
# Anweisungen fuer das Linken
# unroll begin

.IF "$(DEF$(TNR)TARGETN)"!=""

#to make intuitiv naming work
.IF "$(DEF$(TNR)LIBNAME)"!=""
DEFLIB$(TNR)NAME*=$(DEF$(TNR)LIBNAME)
.ENDIF			# "$(DEF$(TNR)LIBNAME)"!=""

.IF "$(DEFLIB$(TNR)NAME)"!=""
.IF "$(UPDATER)"!=""
DEF$(TNR)DEPN+=$(foreach,i,$(DEFLIB$(TNR)NAME) $(SLB)$/$(i).lib)
.ENDIF			# "$(UPDATER)"!=""
.ENDIF

.IF "$(SHL$(TNR)VERSIONMAP)"!=""
.IF "$(DEF$(TNR)EXPORTFILE)"==""
.IF "$(GUI)"=="WNT"
DEF$(TNR)EXPORTFILE=$(MISC)$/$(SHL$(TNR)VERSIONMAP:b)_$(SHL$(TNR)TARGET).dxp
.IF "$(COM)"=="GCC"
$(DEF$(TNR)EXPORTFILE) : $(SHL$(TNR)OBJS) $(SHL$(TNR)LIBS)
.ENDIF # .IF "$(COM)"=="GCC"

$(DEF$(TNR)EXPORTFILE) : $(SHL$(TNR)VERSIONMAP)
    $(TYPE) $< | $(AWK) -f $(SOLARENV)$/bin$/getcsym.awk > $@
.IF "$(COM)"=="GCC"
    -$(GREP) -v "\*\|?" $@ | $(SED) -e 's@#.*@@' > $@.exported-symbols
    -$(GREP) "\*\|?" $@ > $@.symbols-regexp
# Shared libraries will be build out of the *.obj files specified in SHL?OBJS and SHL?LIBS
# Extract RTTI symbols from all the objects that will be used to build a shared library
    nm -gP $(SHL$(TNR)OBJS) \
        `$(TYPE) /dev/null $(foreach,j,$(SHL$(TNR)LIBS) $j) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
        | $(SOLARENV)$/bin$/addsym-mingw.sh $@.symbols-regexp @.symbols-regexp.tmp >> $@.exported-symbols
# overwrite the map file generate into the local output tree with the generated
# exported symbols list
    $(RENAME) $@.exported-symbols $@
.ENDIF # .IF "$(COM)"=="GCC"

.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="OS2"
DEF$(TNR)EXPORTFILE=$(MISC)$/$(SHL$(TNR)VERSIONMAP:b)_$(SHL$(TNR)TARGET).dxp
$(DEF$(TNR)EXPORTFILE) : $(SHL$(TNR)VERSIONMAP)
    $(TYPE) $< | $(AWK) -f $(SOLARENV)$/bin$/getcsym.awk > $@
.ENDIF			# "$(GUI)"=="OS2"

.ENDIF			# "$(DEF$(TNR)EXPORTFILE)"==""
.ENDIF			# "$(SHL$(TNR)VERSIONMAP)"!=""

.IF "$(GUI)"=="WNT"

DEF$(TNR)FILTER=$(SOLARENV)$/inc$/dummy.flt

RMHACK$(TNR):=$(RM)

.IF "$(DEF$(TNR)CEXP)"!=""
EXPORT_ALL_SWITCH=-A $(DEF$(TNR)CEXP)
.ENDIF

.IF "$(link_always)"==""
$(DEF$(TNR)TARGETN) : \
        $(DEF$(TNR)DEPN) \
        $(DEF$(TNR)EXPORTFILE)
.ELSE			# "$(link_always)"==""
$(DEF$(TNR)TARGETN) .PHONY :
.ENDIF			# "$(link_always)"==""
    @-$(RM) $@.tmpfile
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo LIBRARY	  $(EMQ)"$(SHL$(TNR)TARGETN:f)$(EMQ)" 								 >$@.tmpfile
.IF "$(COM)"!="GCC"
    @echo HEAPSIZE	  0 											>>$@.tmpfile
.ENDIF
    @echo EXPORTS													>>$@.tmpfile
.IF "$(VERSIONOBJ)"!=""
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@.tmpfile
.ENDIF
.IF "$(DEFLIB$(TNR)NAME)"!=""
.IF "$(COM)"=="GCC"
    @-$(RM) $(MISC)$/$(SHL$(TNR)TARGET).exp
    dlltool --output-def $(MISC)$/$(SHL$(TNR)TARGET).exp --export-all-symbols \
         `$(TYPE) $(foreach,i,$(DEFLIB$(TNR)NAME) $(SLB)$/$(i).lib) | sed s#$(ROUT)#$(PRJ)$/$(ROUT)#g`
    tail --lines +3 $(MISC)$/$(SHL$(TNR)TARGET).exp | sed '/^;/d' >>$@.tmpfile
    @-$(RM) $(MISC)$/$(SHL$(TNR)TARGET).exp
.ELSE
.IF "$(SHL$(TNR)USE_EXPORTS)"==""
    @-$(RMHACK$(TNR)) $(MISC)$/$(SHL$(TNR)TARGET).exp
    @$(LIBMGR) -EXTRACT:/ /OUT:$(MISC)$/$(SHL$(TNR)TARGET).exp $(SLB)$/$(DEFLIB$(TNR)NAME).lib
    @$(LDUMP2) -N $(EXPORT_ALL_SWITCH) -F $(MISC)$/$(SHL$(TNR)TARGET).flt $(MISC)$/$(SHL$(TNR)TARGET).exp			   >>$@.tmpfile
    $(RMHACK$(TNR)) $(MISC)$/$(SHL$(TNR)TARGET).exp
.ELSE			# "$(SHL$(TNR)USE_EXPORTS)"==""
    @$(DUMPBIN) -DIRECTIVES  $(foreach,i,$(DEFLIB$(TNR)NAME) $(SLB)$/$(i).lib) | $(GREP) EXPORT: > $(MISC)$/$(SHL$(TNR)TARGET).direct
    @$(LDUMP2) -N -D $(EXPORT_ALL_SWITCH) -F $(DEF$(TNR)FILTER) $(MISC)$/$(SHL$(TNR)TARGET).direct >>$@.tmpfile
.ENDIF			# "$(SHL$(TNR)USE_EXPORTS)"==""
.ENDIF
.ENDIF				# "$(DEFLIB$(TNR)NAME)"!=""
.IF "$(DEF$(TNR)EXPORT1)"!=""
    @echo $(DEF$(TNR)EXPORT1)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT2)"!=""
    @echo $(DEF$(TNR)EXPORT2)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT3)"!=""
    @echo $(DEF$(TNR)EXPORT3)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT4)"!=""
    @echo $(DEF$(TNR)EXPORT4)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT5)"!=""
    @echo $(DEF$(TNR)EXPORT5)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT6)"!=""
    @echo $(DEF$(TNR)EXPORT6)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT7)"!=""
    @echo $(DEF$(TNR)EXPORT7)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT8)"!=""
    @echo $(DEF$(TNR)EXPORT8)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT9)"!=""
    @echo $(DEF$(TNR)EXPORT9)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT10)"!=""
    @echo $(DEF$(TNR)EXPORT10)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT11)"!=""
    @echo $(DEF$(TNR)EXPORT11)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT12)"!=""
    @echo $(DEF$(TNR)EXPORT12)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT13)"!=""
    @echo $(DEF$(TNR)EXPORT13)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT14)"!=""
    @echo $(DEF$(TNR)EXPORT14)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT15)"!=""
    @echo $(DEF$(TNR)EXPORT15)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT16)"!=""
    @echo $(DEF$(TNR)EXPORT16)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT17)"!=""
    @echo $(DEF$(TNR)EXPORT17)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT18)"!=""
    @echo $(DEF$(TNR)EXPORT18)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT19)"!=""
    @echo $(DEF$(TNR)EXPORT19)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT20)"!=""
    @echo $(DEF$(TNR)EXPORT20)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORTFILE)"!=""
.IF "$(COM)"=="GCC"
    $(TYPE) $(DEF$(TNR)EXPORTFILE) | sed -e s:PRIVATE:: >> $@.tmpfile
.ELSE
    $(TYPE) $(DEF$(TNR)EXPORTFILE) >> $@.tmpfile
.ENDIF
.ENDIF
    @-$(RM) $@
    @$(RENAME) $@.tmpfile $@
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="OS2"

#21/02/2006 YD dll names must be 8.3, invoke fix script
#check osl/os2/module.c/osl_loadModule()
SHL$(TNR)TARGET8=$(shell @fix_shl $(SHL$(TNR)TARGETN:f))

DEF$(TNR)FILTER=$(SOLARENV)$/inc$/dummy.flt
DEF$(TNR)NAMELIST=$(foreach,i,$(DEFLIB$(TNR)NAME) $(SLB)$/$(i).lib)

.IF "$(link_always)"==""
$(DEF$(TNR)TARGETN) : \
        $(DEF$(TNR)DEPN) \
        $(DEF$(TNR)EXPORTFILE)
.ELSE			# "$(link_always)"==""
$(DEF$(TNR)TARGETN) .PHONY :
.ENDIF			# "$(link_always)"==""
    @+-$(RM) $@.tmpfile
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo LIBRARY	  $(SHL$(TNR)TARGET8) INITINSTANCE TERMINSTANCE	 >$@.tmpfile
    @echo DATA MULTIPLE	 >>$@.tmpfile
    @echo DESCRIPTION	'StarView 3.00 $(DEF$(TNR)DES) $(UPD) $(UPDMINOR)' >>$@.tmpfile
    @echo EXPORTS													>>$@.tmpfile
.IF "$(VERSIONOBJ)"!=""
#	getversioninfo fuer alle!!
    @echo _GetVersionInfo		>$@.tmp_ord
.ENDIF

.IF "$(DEFLIB$(TNR)NAME)"!=""
    @+echo $(SLB)$/$(DEFLIB$(TNR)NAME).lib
    @+emxexpr $(DEF$(TNR)NAMELIST) | fix_exp_file >> $@.tmp_ord
.ENDIF				# "$(DEFLIB$(TNR)NAME)"!=""

.IF "$(DEF$(TNR)EXPORT1)"!=""
    @echo $(DEF$(TNR)EXPORT1)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT2)"!=""
    @echo $(DEF$(TNR)EXPORT2)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT3)"!=""
    @echo $(DEF$(TNR)EXPORT3)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT4)"!=""
    @echo $(DEF$(TNR)EXPORT4)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT5)"!=""
    @echo $(DEF$(TNR)EXPORT5)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT6)"!=""
    @echo $(DEF$(TNR)EXPORT6)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT7)"!=""
    @echo $(DEF$(TNR)EXPORT7)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT8)"!=""
    @echo $(DEF$(TNR)EXPORT8)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT9)"!=""
    @echo $(DEF$(TNR)EXPORT9)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT10)"!=""
    @echo $(DEF$(TNR)EXPORT10)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT11)"!=""
    @echo $(DEF$(TNR)EXPORT11)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT12)"!=""
    @echo $(DEF$(TNR)EXPORT12)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT13)"!=""
    @echo $(DEF$(TNR)EXPORT13)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT14)"!=""
    @echo $(DEF$(TNR)EXPORT14)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT15)"!=""
    @echo $(DEF$(TNR)EXPORT15)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT16)"!=""
    @echo $(DEF$(TNR)EXPORT16)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT17)"!=""
    @echo $(DEF$(TNR)EXPORT17)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT18)"!=""
    @echo $(DEF$(TNR)EXPORT18)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT19)"!=""
    @echo $(DEF$(TNR)EXPORT19)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT20)"!=""
    @echo $(DEF$(TNR)EXPORT20)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORTFILE)"!=""
    @fix_def_file < $(DEF$(TNR)EXPORTFILE) >> $@.tmp_ord
.ENDIF
    @sort < $@.tmp_ord | uniq > $@.exptmpfile
    @fix_def_ord < $@.exptmpfile >> $@.tmpfile
    @+-$(RM) $@
    +$(RENAME) $@.tmpfile $@
    @+-$(RM) $@.tmp_ord
    @+-$(RM) $@.exptmpfile
    +$(IMPLIB) $(IMPLIBFLAGS) $(SHL$(TNR)IMPLIBN:s/.lib/.a/) $@
    +emxomf -o $(SHL$(TNR)IMPLIBN) $(SHL$(TNR)IMPLIBN:s/.lib/.a/) 

.ENDIF			# "$(GUI)"=="OS2"

.IF "$(GUI)"=="UNX"
$(DEF$(TNR)TARGETN): \
        $(DEF$(TNR)DEPN) \
        $(DEF$(TNR)EXPORTFILE)
    @echo just a dummy for UNIX > $@
.ENDIF
.ENDIF			# "$(DEF$(TNR)TARGETN)"!=""


# Anweisungen fuer das Linken
# unroll end
#######################################################

