#*************************************************************************
#
#   $RCSfile: tg_rslb.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: mh $ $Date: 2001-06-20 09:09:38 $
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
.IF "$(GUI)"!="MAC"
.IF "$(GUI)"=="UNX"
    @dmake $(RESLIB$(TNR)TARGETN) solarlang=$(solarlang) MULTI_RESLIB_FLAG=true TNR:=$(TNR) $(MFLAGS) $(CALLMACROS)
.ELSE
    dmake $(RESLIB$(TNR)TARGETN) solarlang=$(solarlang) MULTI_RESLIB_FLAG=true TNR:=$(TNR) $(MFLAGS) $(CALLMACROS)
.ENDIF
.ELSE
    @dmake "$(RESLIB$(TNR)TARGETN)" MULTI_RESLIB_FLAG=true TNR:=$(TNR) $(MFLAGS) $(CALLMACROS)
.ENDIF
.ELSE


#######################################################
# Anweisungen fuer das Linken
# unroll begin

.IF "$(RESLIB$(TNR)TARGETN)"!=""

$(RESLIB$(TNR)TARGETN): \
        $(RESLIB$(TNR)SRSFILES) \
        $(RESLIB$(TNR)BMPS)
    @echo Making: $@
.IF "$(GUI)"=="UNX"
.IF "$(NO_REC_RES)"!=""
.IF "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r \
    $(rsclang_{$(subst,$(RESLIB$(TNR)NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB$(TNR)NAME)$(UPD), $(@:b))}) \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(langext_{$(subst,$(RESLIB$(TNR)NAME)$(UPD), $(@:b))}) \
    -fs{$(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)} \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(PRJ)$/..$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC$(TNR)HEADER) $(RESLIB$(TNR)SRSFILES) \
    )
    +$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.new >& $(NULLDEV)
    +-$(RM) $@ >& $(NULLDEV)
    +$(RENAME) $@.new $@
.ELSE			# "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r \
    $(rsclang_{$(subst,$(RESLIB$(TNR)NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB$(TNR)NAME)$(UPD), $(@:b))}) \
    -I$(RES)$/$(langext_{$(subst,$(RESLIB$(TNR)NAME)$(UPD), $(@:b))}) \
    -fs$@ \
    -I$(PRJ)$/$(INPATH)$/res$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(PRJ)$/..$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC$(TNR)HEADER) $(RESLIB$(TNR)SRSFILES) \
    )
.ENDIF			# "$(common_build_reslib)"!=""
.ELSE			# "$(NO_REC_RES)"!=""
    $(RSC) -presponse @$(mktmp \
    -r \
    $(rsclang_{$(subst,$(RESLIB$(TNR)NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB$(TNR)NAME)$(UPD), $(@:b))}) \
    -I$(RES) \
    -fs$@ \
    -I$(PRJ)$/$(INPATH)$/res \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(PRJ)$/..$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC$(TNR)HEADER) $(RESLIB$(TNR)SRSFILES) \
    )
.ENDIF			# "$(NO_REC_RES)"!=""
.ELSE				# "$(GUI)"=="UNX"
.IF "$(NO_REC_RES)"!=""
.IF "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    $(rsclang_{$(subst,$(RESLIB$(TNR)NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB$(TNR)NAME)$(UPD), $(@:b))}) \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(langext_{$(subst,$(RESLIB$(TNR)NAME)$(UPD), $(@:b))}) \
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
.ELSE			# "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    $(rsclang_{$(subst,$(RESLIB$(TNR)NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB$(TNR)NAME)$(UPD), $(@:b))}) \
    -I$(RES)$/$(langext_{$(subst,$(RESLIB$(TNR)NAME)$(UPD), $(@:b))}) \
    -fs$@ \
    -r -p \
    -I$(RES) \
    -I$(PRJ)$/$(INPATH)$/res$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INCLUDE) \
    $(RSC$(TNR)HEADER) $(RESLIB$(TNR)SRSFILES) \
    )
.ENDIF			# "$(common_build_reslib)"!=""
.ELSE			# "$(NO_REC_RES)"!=""
    $(RSC) -presponse @$(mktmp \
    $(rsclang_{$(subst,$(RESLIB$(TNR)NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB$(TNR)NAME)$(UPD), $(@:b))}) \
    -I$(RES) \
    -fs$@ \
    -r -p \
    -I$(RES) \
    -I$(PRJ)$/$(INPATH)$/res \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INCLUDE) \
    $(RSC$(TNR)HEADER) $(RESLIB$(TNR)SRSFILES) \
    )
.ENDIF			# "$(NO_REC_RES)"!=""
.ENDIF              # "$(GUI)"=="UNX"
.ENDIF				# "$(RESLIB$(TNR)TARGETN)"!=""


# Anweisungen fuer das Linken
# unroll end
#######################################################

.ENDIF				# "$(MULTI_RESLIB_FLAG)"==""
