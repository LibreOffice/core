#*************************************************************************
#*
#*    $Workfile:   tg_rslb.mk  $
#*
#*    Ersterstellung    XX  TT.MM.JJ
#*    Letzte Aenderung  $Author: hr $ $Date: 2000-09-20 14:43:17 $
#*    $Revision: 1.1.1.1 $
#*
#*    $Logfile:   T:/solar/inc/tg_rslb.mkv  $
#*
#*    Copyright (c) 1990-1997, STAR DIVISION
#*
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
