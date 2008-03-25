#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: tg_def.mk,v $
#
#   $Revision: 1.46 $
#
#   last change: $Author: obo $ $Date: 2008-03-25 14:21:49 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
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
    -grep -v "\*\|?" $@ > $@.exported-symbols
    -grep "\*\|?" $@ > $@.symbols-regexp
# Shared libraries will be build out of the *.obj files specified in SHL?OBJS and SHL?LIBS
# Extract RTTI symbols from all the objects that will be used to build a shared library
.IF "$(SHL$(TNR)OBJS)"!=""
    -echo $(foreach,i,$(SHL$(TNR)OBJS) $i) | xargs -n1 nm -gP | $(SOLARENV)$/bin$/addsym-mingw.sh $@.symbols-regexp $(MISC)$/symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
.IF "$(SHL$(TNR)LIBS)"!=""
    -$(TYPE) $(foreach,j,$(SHL$(TNR)LIBS) $j) | $(SED) s\#$(ROUT)\#$(PRJ)$/$(ROUT)\#g | xargs -n1 nm -gP | $(SOLARENV)$/bin$/addsym-mingw.sh $@.symbols-regexp $(MISC)$/symbols-regexp.tmp >> $@.exported-symbols
.ENDIF
# overwrite the map file generate into the local output tree with the generated
# exported symbols list
    cp $@.exported-symbols $@
.ENDIF # .IF "$(COM)"=="GCC"

.ENDIF			# "$(GUI)"=="WNT"
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
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@.tmpfile
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

.IF "$(GUI)"=="UNX"
$(DEF$(TNR)TARGETN): \
        $(DEF$(TNR)DEPN) \
        $(DEF$(TNR)EXPORTFILE)
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo just a dummy for UNIX
    @echo > $@
.ENDIF
.ENDIF			# "$(DEF$(TNR)TARGETN)"!=""


# Anweisungen fuer das Linken
# unroll end
#######################################################

