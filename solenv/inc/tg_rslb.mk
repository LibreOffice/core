#*************************************************************************
#
#   $RCSfile: tg_rslb.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: kz $ $Date: 2003-08-25 14:47:27 $
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

$(RSC_MULTI$(TNR)) : \
        $(RESLIB$(TNR)SRSFILES) \
        $(RESLIB$(TNR)TARGETN) \
        $(RESLIB$(TNR)BMPS)
.IF "$(rsc_once)"!=""
    @+echo using rsc multi-res feature
.IF "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangext) $(rsclang_{$i}) \
    $(rescharset_{$i}) \
    -fs{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB$(TNR)NAME)$(RESLIB$(TNR)VERSION)$i.res} \
    -lip{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(langext_{$(i)}) ) \
    -ft$@ \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC$(TNR)HEADER) $(RESLIB$(TNR)SRSFILES) \
    ) > $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangext) $(rsclang_{$i}) \
    $(rescharset_{$i}) \
    -fs{$(BIN)$/$(RESLIB$(TNR)NAME)$(RESLIB$(TNR)VERSION)$i.res} \
    -lip{$(RES)}$/$(langext_{$(i)}) ) \
    -ft$@ \
    -I{$(RES)}$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC$(TNR)HEADER) $(RESLIB$(TNR)SRSFILES) \
    ) > $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""
.ENDIF           # "$(rsc_once)"!=""

$(RESLIB$(TNR)TARGETN): \
        $(RESLIB$(TNR)SRSFILES) \
        $(RESLIB$(TNR)BMPS)
    @echo Making: $@
.IF "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
.IF "$(rsc_once)"!=""
    @+-$(RM) $(RSC_MULTI$(TNR)) >& $(NULLDEV)
.ELSE           # "$(rsc_once)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(rsclang_{$(subst,$(RESLIB$(TNR)NAME)$(RESLIB$(TNR)VERSION), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB$(TNR)NAME)$(RESLIB$(TNR)VERSION), $(@:b))}) \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(langext_{$(subst,$(RESLIB$(TNR)NAME)$(RESLIB$(TNR)VERSION), $(@:b))}) \
    -fs{$(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)} \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(PRJ)$/..$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC$(TNR)HEADER) $(RESLIB$(TNR)SRSFILES) \
    )
    +$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.new >& $(NULLDEV)
    +-$(RM) $@ >& $(NULLDEV)
    +$(RENAME) $@.new $@
.ENDIF          # "$(rsc_once)"!=""
.ELSE			# "$(common_build_reslib)"!=""
.IF "$(rsc_once)"!=""
    @+-$(RM) $(RSC_MULTI$(TNR)) >& $(NULLDEV)
.ELSE           # "$(rsc_once)"!=""
    $(RSC) -presponse @$(mktmp \
    -r \
    $(rsclang_{$(subst,$(RESLIB$(TNR)NAME)$(RESLIB$(TNR)VERSION), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB$(TNR)NAME)$(RESLIB$(TNR)VERSION), $(@:b))}) \
    -I$(RES)$/$(langext_{$(subst,$(RESLIB$(TNR)NAME)$(RESLIB$(TNR)VERSION), $(@:b))}) \
    -fs$@ \
    -I$(PRJ)$/$(INPATH)$/res$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(PRJ)$/..$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC$(TNR)HEADER) $(RESLIB$(TNR)SRSFILES) \
    )
.ENDIF          # "$(rsc_once)"!=""
.ENDIF			# "$(common_build_reslib)"!=""
.ELSE				# "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
.IF "$(rsc_once)"!=""
    @+-$(RM) $(RSC_MULTI$(TNR)) >& $(NULLDEV)
.ELSE           # "$(rsc_once)"!=""
    $(RSC) -presponse @$(mktmp \
    $(rsclang_{$(subst,$(RESLIB$(TNR)NAME)$(RESLIB$(TNR)VERSION), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB$(TNR)NAME)$(RESLIB$(TNR)VERSION), $(@:b))}) \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(langext_{$(subst,$(RESLIB$(TNR)NAME)$(RESLIB$(TNR)VERSION), $(@:b))}) \
    -fs{$(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)} \
    -r -p \
    -I$(RES) \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INCLUDE) \
    $(RSC$(TNR)HEADER) $(RESLIB$(TNR)SRSFILES) \
    )
    +-$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.new >& $(NULLDEV)
    +-$(RM) $@ >& $(NULLDEV)
    +$(RENAME) $@.new $@
.ENDIF          # "$(rsc_once)"!=""
.ELSE			# "$(common_build_reslib)"!=""
.IF "$(rsc_once)"!=""
    @+-$(RM) $(RSC_MULTI$(TNR)) >& $(NULLDEV)
.ELSE           # "$(rsc_once)"!=""
    $(RSC) -presponse @$(mktmp \
    $(rsclang_{$(subst,$(RESLIB$(TNR)NAME)$(RESLIB$(TNR)VERSION), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB$(TNR)NAME)$(RESLIB$(TNR)VERSION), $(@:b))}) \
    -I$(RES)$/$(langext_{$(subst,$(RESLIB$(TNR)NAME)$(RESLIB$(TNR)VERSION), $(@:b))}) \
    -fs$@ \
    -r -p \
    -I$(RES) \
    -I$(PRJ)$/$(INPATH)$/res$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INCLUDE) \
    $(RSC$(TNR)HEADER) $(RESLIB$(TNR)SRSFILES) \
    )
.ENDIF          # "$(rsc_once)"!=""
.ENDIF			# "$(common_build_reslib)"!=""
.ENDIF              # "$(GUI)"=="UNX"
.ENDIF				# "$(RESLIB$(TNR)TARGETN)"!=""


# Anweisungen fuer das Linken
# unroll end
#######################################################

.ENDIF				# "$(MULTI_RESLIB_FLAG)"==""
