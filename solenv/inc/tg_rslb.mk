#*************************************************************************
#
#   $RCSfile: tg_rslb.mk,v $
#
#   $Revision: 1.8 $
#
#   last change: $Author: hjs $ $Date: 2004-06-07 13:47:40 $
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


.IF "$(MULTI_RESLIB_FLAG)" == ""
$(RESLIB1TARGETN) .NULL : RESLIB1

$(RESLIB2TARGETN) .NULL : RESLIB2

$(RESLIB3TARGETN) .NULL : RESLIB3

$(RESLIB4TARGETN) .NULL : RESLIB4

$(RESLIB5TARGETN) .NULL : RESLIB5

$(RESLIB6TARGETN) .NULL : RESLIB6

$(RESLIB7TARGETN) .NULL : RESLIB7

$(RESLIB8TARGETN) .NULL : RESLIB8

$(RESLIB9TARGETN) .NULL : RESLIB9
.ENDIF


.IF "$(MULTI_RESLIB_FLAG)"==""
RESLIB1 RESLIB2 RESLIB3 RESLIB4 RESLIB5 RESLIB6 RESLIB7 RESLIB8 RESLIB9:
    @dmake $(RESLIB$(TNR)TARGETN) $(HIDRES$(TNR)PARTICLE) solarlang=$(solarlang) MULTI_RESLIB_FLAG=true TNR:=$(TNR) $(MFLAGS) $(CALLMACROS)
.ELSE


#######################################################
# Anweisungen fuer das Linken
# unroll begin

.IF "$(RESLIB$(TNR)TARGETN)"!=""

.IF "$(BUILDHIDS)"!=""
HIDRES$(TNR)PARTICLE=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(RESLIB$(TNR)NAME)_res.hid

#HACK cut off the dirty srs files which are included from solver
#RESLIB$(TNR)HIDFILESx=$(shell @+echo $(RESLIB$(TNR)SRSFILES:t"\n") | $(GREP) -v "$(SOLARRESDIR:s/\/\\/)" )
RESLIB$(TNR)HIDFILES:=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(subst,.srs,_srs.hid $(RESLIB$(TNR)SRSFILES)))
$(HIDRES$(TNR)PARTICLE): $(RESLIB$(TNR)HIDFILES)
    @echo ------------------------------
    @echo Making: $@
    @+if exist $@ rm $@
    $(TYPE) $(RESLIB$(TNR)HIDFILES) > $@.$(ROUT).tmp 
    @+$(RENAME) $@.$(ROUT).tmp $@

ALLTAR : $(HIDRES$(TNR)PARTICLE)

.ENDIF # "$(BUILDHIDS)"!=""

.IF "$(common_build_reslib)"!=""
$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB$(TNR)NAME)$(RESLIB$(TNR)VERSION).ilst2 : $(RSC_MULTI$(TNR))
.IF "$(use_shell)"!="4nt"	
    $(TYPE) $(mktmp $(foreach,i,$(alllangext) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB$(TNR)NAME)$(RESLIB$(TNR)VERSION)$i.ilst)) | xargs sed s\#%MODULE%\#%MODULE%$/$(PRJNAME)\# > $@
.ELSE			# "$(use_shell)"!="4nt"	
    +$(TYPE) $(foreach,i,$(alllangext) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB$(TNR)NAME)$(RESLIB$(TNR)VERSION)$i.ilst) | sed `s/MODULE%/MODULE%\/$(PRJNAME)/` > $@
.ENDIF			# "$(use_shell)"!="4nt"	

ALLTAR : $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB$(TNR)NAME)$(RESLIB$(TNR)VERSION).ilst2
.ELSE           # "$(common_build_reslib)"!=""
$(BIN)$/$(RESLIB$(TNR)NAME)$(RESLIB$(TNR)VERSION).ilst2 : $(RSC_MULTI$(TNR))
.IF "$(use_shell)"!="4nt"	
    $(TYPE) $(mktmp $(foreach,i,$(alllangext) $(BIN)$/$(RESLIB$(TNR)NAME)$(RESLIB$(TNR)VERSION)$i.ilst)) | xargs sed s\#%MODULE%\#%MODULE%$/$(PRJNAME)\# > $@
.ELSE			# "$(use_shell)"!="4nt"	
    +$(TYPE) $(foreach,i,$(alllangext) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB$(TNR)NAME)$(RESLIB$(TNR)VERSION)$i.ilst) | sed `s/MODULE%/MODULE%\/$(PRJNAME)/` > $@
.ENDIF			# "$(use_shell)"!="4nt"	

ALLTAR : $(BIN)$/$(RESLIB$(TNR)NAME)$(RESLIB$(TNR)VERSION).ilst2
.ENDIF          # "$(common_build_reslib)"!=""

$(RSC_MULTI$(TNR)) : \
        $(RESLIB$(TNR)SRSFILES) \
        $(RESLIB$(TNR)TARGETN) \
        $(RESLIB$(TNR)BMPS)
    @+echo using rsc multi-res feature
.IF "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangext) $(rsclang_{$i}) \
    $(rescharset_{$i}) \
    -fs{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB$(TNR)NAME)$(RESLIB$(TNR)VERSION)$i.res} \
    $(foreach,j,$(RESLIB1IMAGES) -lip{$j}$/$(lang_{$i}) \
    -lip{$j} ) \
    -lip$(SOLARSRC)$/res$/$(lang_{$i}) -lip$(SOLARSRC)$/res ) \
    -subMODULE=$(PRJ) \
    -subGLOBAL=$(SOLARSRC) \
    -subCUSTOM=to_be_defined \
    -oil{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))} \
    -ft$@ \
    -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC$(TNR)HEADER) $(RESLIB$(TNR)SRSFILES) \
    ) > $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangext) $(rsclang_{$i}) \
    $(rescharset_{$i}) \
    -fs{$(BIN)$/$(RESLIB$(TNR)NAME)$(RESLIB$(TNR)VERSION)$i.res} \
    $(foreach,j,$(RESLIB1IMAGES) -lip{$j}$/$(lang_{$i}) \
    -lip{$j} ) \
    -lip$(SOLARSRC)$/res$/$(lang_{$i}) -lip$(SOLARSRC)$/res ) \
    -subGLOBAL=$(SOLARSRC) \
    -subMODULE=$(PRJ) \
    -subCUSTOM=to_be_defined \
    -oil$(BIN) \
    -ft$@ \
    -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC$(TNR)HEADER) $(RESLIB$(TNR)SRSFILES) \
    ) > $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""

$(RESLIB$(TNR)TARGETN): \
        $(RESLIB$(TNR)SRSFILES) \
        $(RESLIB$(TNR)BMPS)
    @echo Making: $@
.IF "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
    @+-$(RM) $(RSC_MULTI$(TNR)) >& $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    @+-$(RM) $(RSC_MULTI$(TNR)) >& $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""
.ELSE				# "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
    @+-$(RM) $(RSC_MULTI$(TNR)) >& $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    @+-$(RM) $(RSC_MULTI$(TNR)) >& $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""
.ENDIF              # "$(GUI)"=="UNX"
.ENDIF				# "$(RESLIB$(TNR)TARGETN)"!=""


# Anweisungen fuer das Linken
# unroll end
#######################################################

.ENDIF				# "$(MULTI_RESLIB_FLAG)"==""
