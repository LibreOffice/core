#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
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
DEF$(TNR)DEPN+=$(foreach,i,$(DEFLIB$(TNR)NAME) $(SLB)/$(i).lib)
.ENDIF

.IF "$(SHL$(TNR)VERSIONMAP)"!=""
.IF "$(DEF$(TNR)EXPORTFILE)"==""
.IF "$(GUI)"=="WNT"
DEF$(TNR)EXPORTFILE=$(MISC)/$(SHL$(TNR)VERSIONMAP:b)_$(SHL$(TNR)TARGET).dxp
.IF "$(COM)"=="GCC"
$(DEF$(TNR)EXPORTFILE) : $(SHL$(TNR)OBJS) $(SHL$(TNR)LIBS)
.ENDIF # .IF "$(COM)"=="GCC"

$(DEF$(TNR)EXPORTFILE) : $(SHL$(TNR)VERSIONMAP)
    $(COMMAND_ECHO)$(TYPE) $< | $(AWK) -f $(SOLARENV)/bin/getcsym.awk > $@
.IF "$(COM)"=="GCC"
    $(COMMAND_ECHO)-$(GREP) -v "\*\|?" $@ | $(SED) -e 's@#.*@@' > $@.exported-symbols
    $(COMMAND_ECHO)-$(GREP) "\*\|?" $@ > $@.symbols-regexp
# Shared libraries will be build out of the *.obj files specified in SHL?OBJS and SHL?LIBS
# Extract RTTI symbols from all the objects that will be used to build a shared library
    $(COMMAND_ECHO)$(NM) -gP $(SHL$(TNR)OBJS) \
        `$(TYPE) /dev/null $(foreach,j,$(SHL$(TNR)LIBS) $j) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g` \
        | $(SOLARENV)/bin/addsym-mingw.sh $@.symbols-regexp $@.symbols-regexp.tmp >> $@.exported-symbols
# overwrite the map file generate into the local output tree with the generated
# exported symbols list
    $(COMMAND_ECHO)$(RENAME) $@.exported-symbols $@
.ENDIF # .IF "$(COM)"=="GCC"

.ENDIF			# "$(GUI)"=="WNT"

.ENDIF			# "$(DEF$(TNR)EXPORTFILE)"==""
.ENDIF			# "$(SHL$(TNR)VERSIONMAP)"!=""

.IF "$(GUI)"=="WNT"

DEF$(TNR)FILTER=$(SOLARENV)/inc/dummy.flt

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
    $(COMMAND_ECHO)-$(RM) $@.tmpfile
    @echo "Making:    module definition file" $(@:f)
    $(COMMAND_ECHO)echo LIBRARY	  $(EMQ)"$(SHL$(TNR)TARGETN:f)$(EMQ)" 								 >$@.tmpfile
.IF "$(COM)"!="GCC"
    $(COMMAND_ECHO)echo HEAPSIZE	  0 											>>$@.tmpfile
.ENDIF
    $(COMMAND_ECHO)echo EXPORTS													>>$@.tmpfile
.IF "$(VERSIONOBJ)"!=""
#	getversioninfo fuer alle!!
    $(COMMAND_ECHO)echo GetVersionInfo		>>$@.tmpfile
.ENDIF
.IF "$(DEFLIB$(TNR)NAME)"!=""
.IF "$(COM)"=="GCC"
    $(COMMAND_ECHO)-$(RM) $(MISC)/$(SHL$(TNR)TARGET).exp
    $(COMMAND_ECHO)dlltool --output-def $(MISC)/$(SHL$(TNR)TARGET).exp --export-all-symbols \
        `$(TYPE) $(foreach,i,$(DEFLIB$(TNR)NAME) $(SLB)/$(i).lib) | sed s#$(ROUT)#$(PRJ)/$(ROUT)#g`
    $(COMMAND_ECHO)tail --lines +3 $(MISC)/$(SHL$(TNR)TARGET).exp | sed '/^;/d' >>$@.tmpfile
    $(COMMAND_ECHO)-$(RM) $(MISC)/$(SHL$(TNR)TARGET).exp
.ELSE
.IF "$(SHL$(TNR)USE_EXPORTS)"==""
    $(COMMAND_ECHO)-$(RMHACK$(TNR)) $(MISC)/$(SHL$(TNR)TARGET).exp
    $(COMMAND_ECHO)$(LIBMGR) -EXTRACT:/ /OUT:$(MISC)/$(SHL$(TNR)TARGET).exp $(SLB)/$(DEFLIB$(TNR)NAME).lib
    $(COMMAND_ECHO)$(LDUMP2) -N $(EXPORT_ALL_SWITCH) -F $(MISC)/$(SHL$(TNR)TARGET).flt $(MISC)/$(SHL$(TNR)TARGET).exp			   >>$@.tmpfile
    $(COMMAND_ECHO)$(RMHACK$(TNR)) $(MISC)/$(SHL$(TNR)TARGET).exp
.ELSE			# "$(SHL$(TNR)USE_EXPORTS)"==""
    $(COMMAND_ECHO)$(DUMPBIN) -DIRECTIVES  $(foreach,i,$(DEFLIB$(TNR)NAME) $(SLB)/$(i).lib) | $(GREP) EXPORT: > $(MISC)/$(SHL$(TNR)TARGET).direct
    $(COMMAND_ECHO)$(LDUMP2) -N -D $(EXPORT_ALL_SWITCH) -F $(DEF$(TNR)FILTER) $(MISC)/$(SHL$(TNR)TARGET).direct >>$@.tmpfile
.ENDIF			# "$(SHL$(TNR)USE_EXPORTS)"==""
.ENDIF
.ENDIF				# "$(DEFLIB$(TNR)NAME)"!=""
.IF "$(DEF$(TNR)EXPORT1)"!=""
    $(COMMAND_ECHO)echo $(DEF$(TNR)EXPORT1)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT2)"!=""
    $(COMMAND_ECHO)echo $(DEF$(TNR)EXPORT2)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT3)"!=""
    $(COMMAND_ECHO)echo $(DEF$(TNR)EXPORT3)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT4)"!=""
    $(COMMAND_ECHO)echo $(DEF$(TNR)EXPORT4)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT5)"!=""
    $(COMMAND_ECHO)echo $(DEF$(TNR)EXPORT5)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT6)"!=""
    $(COMMAND_ECHO)echo $(DEF$(TNR)EXPORT6)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT7)"!=""
    $(COMMAND_ECHO)echo $(DEF$(TNR)EXPORT7)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT8)"!=""
    $(COMMAND_ECHO)echo $(DEF$(TNR)EXPORT8)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT9)"!=""
    $(COMMAND_ECHO)echo $(DEF$(TNR)EXPORT9)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT10)"!=""
    $(COMMAND_ECHO)echo $(DEF$(TNR)EXPORT10)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT11)"!=""
    $(COMMAND_ECHO)echo $(DEF$(TNR)EXPORT11)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT12)"!=""
    $(COMMAND_ECHO)echo $(DEF$(TNR)EXPORT12)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT13)"!=""
    $(COMMAND_ECHO)echo $(DEF$(TNR)EXPORT13)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT14)"!=""
    $(COMMAND_ECHO)echo $(DEF$(TNR)EXPORT14)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT15)"!=""
    $(COMMAND_ECHO)echo $(DEF$(TNR)EXPORT15)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT16)"!=""
    $(COMMAND_ECHO)echo $(DEF$(TNR)EXPORT16)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT17)"!=""
    $(COMMAND_ECHO)echo $(DEF$(TNR)EXPORT17)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT18)"!=""
    $(COMMAND_ECHO)echo $(DEF$(TNR)EXPORT18)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT19)"!=""
    $(COMMAND_ECHO)echo $(DEF$(TNR)EXPORT19)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORT20)"!=""
    $(COMMAND_ECHO)echo $(DEF$(TNR)EXPORT20)										>>$@.tmpfile
.ENDIF
.IF "$(DEF$(TNR)EXPORTFILE)"!=""
.IF "$(COM)"=="GCC"
    $(COMMAND_ECHO)$(TYPE) $(DEF$(TNR)EXPORTFILE) | sed -e s:PRIVATE:: >> $@.tmpfile
.ELSE
    $(COMMAND_ECHO)$(TYPE) $(DEF$(TNR)EXPORTFILE) >> $@.tmpfile
.ENDIF
.ENDIF
    $(COMMAND_ECHO)-$(RM) $@
    $(COMMAND_ECHO)$(RENAME) $@.tmpfile $@
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="UNX"
$(DEF$(TNR)TARGETN): \
        $(DEF$(TNR)DEPN) \
        $(DEF$(TNR)EXPORTFILE)
    $(COMMAND_ECHO)echo just a dummy for UNIX > $@
.ENDIF
.ENDIF			# "$(DEF$(TNR)TARGETN)"!=""


# Anweisungen fuer das Linken
# unroll end
#######################################################

