#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: tg_def.mk,v $
#
#   $Revision: 1.30 $
#
#   last change: $Author: kz $ $Date: 2006-07-05 21:57:49 $
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
DEF$(TNR)DEPN+=$(SLB)$/$(DEFLIB$(TNR)NAME).lib
.ENDIF			# "$(UPDATER)"!=""
.ENDIF

.IF "$(SHL$(TNR)VERSIONMAP)"!=""
.IF "$(DEF$(TNR)EXPORTFILE)"==""
.IF "$(GUI)"=="WNT"
DEF$(TNR)EXPORTFILE=$(MISC)$/$(SHL$(TNR)VERSIONMAP:b)_$(SHL$(TNR)TARGET).dxp
$(DEF$(TNR)EXPORTFILE) : $(SHL$(TNR)VERSIONMAP)
    +$(TYPE) $< | $(AWK) -f $(SOLARENV)$/bin$/getcsym.awk > $@

.ENDIF			# "$(GUI)"=="WNT"
.ENDIF			# "$(DEF$(TNR)EXPORTFILE)"==""
.ENDIF			# "$(SHL$(TNR)VERSIONMAP)"!=""

.IF "$(GUI)"=="WNT"

DEF$(TNR)FILTER=$(SOLARENV)$/inc$/dummy.flt

.IF "$(MWS_BUILD)"!=""
.IF "$(UPDATER)"!=""
.IF "$(DEFLIB$(TNR)NAME)"!=""

DEF$(TNR)UNIQE:=$(mktmp $(GUI))

# %_disk is a 4nt special; don't exppect it to work in any other shell
BUILD_DRIVE$(TNR):=$(shell +echo %_disk)
#BUILD_DRIVE$(TNR):=O

.IF "$(BUILD_DRIVE$(TNR))"=="O"
# in case of RE build, protect against failed lock
EXPORT$(TNR)_PROTECT=$(TMP)$/$(DEF$(TNR)UNIQE:b).bat && 
.ENDIF			# "$(BUILD_DRIVE$(TNR))"=="O"

.ENDIF			# "$(DEFLIB$(TNR)NAME)"!=""
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(MWS_BUILD)"!=""

.IF "$(link_always)"==""
$(DEF$(TNR)TARGETN) : \
        $(DEF$(TNR)DEPN) \
        $(DEF$(TNR)EXPORTFILE)
.ELSE			# "$(link_always)"==""
$(DEF$(TNR)TARGETN) .PHONY :
.ENDIF			# "$(link_always)"==""
.IF "$(MWS_BUILD)"!=""
.IF "$(UPDATER)"!=""
.IF "$(DEFLIB$(TNR)NAME)"!=""
.IF "$(BUILD_DRIVE$(TNR))"=="O"
#
# don't forget to have the right DEFSTAG set!
#
    +$(PERL) $(COMMON_ENV_TOOLS)$/lockcidef.pl -u$(DEF$(TNR)UNIQE:b) update $(DEFSTAG)
.ENDIF			# "$(BUILD_DRIVE$(TNR))"=="O"
.ENDIF				# "$(DEFLIB$(TNR)NAME)"!=""
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(MWS_BUILD)"!=""
#	+-attrib -r defs$/$(OUTPATH)
    @+-$(RM) $@.tmpfile
    @echo ------------------------------
    @echo Making Module-Definitionfile : $@
    @echo LIBRARY	  $(SHL$(TNR)TARGETN:f) 								 >$@.tmpfile
    @echo HEAPSIZE	  0 											>>$@.tmpfile
    @echo EXPORTS													>>$@.tmpfile
#	getversioninfo fuer alle!!
    @echo GetVersionInfo		>>$@.tmpfile
.IF "$(NO_SHL$(TNR)DESCRIPTION)"==""
    @echo component_getDescriptionFunc	>>$@.tmpfile
.ENDIF			# "$(NO_SHL$(TNR)DESCRIPTION)"==""
.IF "$(DEFLIB$(TNR)NAME)"!=""
.IF "$(SHL$(TNR)USE_EXPORTS)"!="ordinal"
    @-+$(EXPORT$(TNR)_PROTECT) $(RM) $(MISC)$/$(SHL$(TNR)TARGET).exp
    @+$(EXPORT$(TNR)_PROTECT) $(LIBMGR) -EXTRACT:/ /OUT:$(MISC)$/$(SHL$(TNR)TARGET).exp $(SLB)$/$(DEFLIB$(TNR)NAME).lib
.IF "$(DEF$(TNR)CEXP)"!=""
    @+$(EXPORT$(TNR)_PROTECT) $(LDUMP2) -A $(DEF$(TNR)CEXP) -E 20 -F $(MISC)$/$(SHL$(TNR)TARGET).flt $(MISC)$/$(SHL$(TNR)TARGET).exp			   >>$@.tmpfile
.ELSE
    @+$(EXPORT$(TNR)_PROTECT) $(LDUMP2) -E 20 -F $(MISC)$/$(SHL$(TNR)TARGET).flt $(MISC)$/$(SHL$(TNR)TARGET).exp			   >>$@.tmpfile
.ENDIF
    +$(EXPORT$(TNR)_PROTECT) $(RM) $(MISC)$/$(SHL$(TNR)TARGET).exp
.ELSE			# "$(SHL$(TNR)USE_EXPORTS)"!="ordinal"
    @+$(EXPORT$(TNR)_PROTECT) $(DUMPBIN) -DIRECTIVES $(SLB)$/$(DEFLIB$(TNR)NAME).lib | $(GREP) EXPORT: > $(MISC)$/$(SHL$(TNR)TARGET).direct
.IF "$(DEF$(TNR)CEXP)"!=""
    @+$(EXPORT$(TNR)_PROTECT) $(LDUMP2) -D -A $(DEF$(TNR)CEXP) -E 20 -F $(DEF$(TNR)FILTER) $(MISC)$/$(SHL$(TNR)TARGET).direct >>$@.tmpfile
.ELSE
    @+$(EXPORT$(TNR)_PROTECT) $(LDUMP2) -D -E 20 -F $(DEF$(TNR)FILTER) $(MISC)$/$(SHL$(TNR)TARGET).direct >>$@.tmpfile
.ENDIF
.ENDIF			# "$(SHL$(TNR)USE_EXPORTS)"!="ordinal"
# now *\defs\$(OUTPATH)	exists, commit it
.IF "$(MWS_BUILD)"!=""
.IF "$(UPDATER)"!=""
.IF "$(BUILD_DRIVE$(TNR))"=="O"
#
# don't forget to have the right DEFSTAG set!
#
    +$(PERL) $(COMMON_ENV_TOOLS)$/lockcidef.pl -u$(DEF$(TNR)UNIQE:b) commit
    +$(TMP)$/$(DEF$(TNR)UNIQE:b).bat && $(RM) $(TMP)$/$(DEF$(TNR)UNIQE:b).bat
.ENDIF			# "$(BUILD_DRIVE$(TNR))"=="O"
.ENDIF			# "$(UPDATER)"!=""
.ENDIF			# "$(MWS_BUILD)"!=""
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
    +$(TYPE) $(DEF$(TNR)EXPORTFILE) >> $@.tmpfile
.ENDIF
    @+-$(RM) $@
    @+$(RENAME) $@.tmpfile $@
.ENDIF			# "$(GUI)"=="WNT"

.IF "$(GUI)"=="UNX"
$(DEF$(TNR)TARGETN): \
        $(DEF$(TNR)DEPN) \
        $(DEF$(TNR)EXPORTFILE)
    @+echo ------------------------------
    @+echo Making Module-Definitionfile : $@
    @+echo just a dummy for UNIX
    @+echo > $@
.ENDIF
.ENDIF			# "$(DEF$(TNR)TARGETN)"!=""


# Anweisungen fuer das Linken
# unroll end
#######################################################

