# unroll begin

.IF "$(RESLIB1TARGETN)"!=""

$(RESLIB1TARGETN): \
        $(RESLIB1SRSFILES) \
        $(RESLIB1BMPS)
    @echo Making: $@
.IF "$(GUI)"=="UNX"
.IF "$(NO_REC_RES)"!=""
.IF "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r \
    $(rsclang_{$(subst,$(RESLIB1NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB1NAME)$(UPD), $(@:b))}) \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(langext_{$(subst,$(RESLIB1NAME)$(UPD), $(@:b))}) \
    -fs{$(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)} \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(PRJ)$/..$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC1HEADER) $(RESLIB1SRSFILES) \
    )
    +$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.new >& $(NULLDEV)
    +-$(RM) $@ >& $(NULLDEV)
    +$(RENAME) $@.new $@
.ELSE			# "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r \
    $(rsclang_{$(subst,$(RESLIB1NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB1NAME)$(UPD), $(@:b))}) \
    -I$(RES)$/$(langext_{$(subst,$(RESLIB1NAME)$(UPD), $(@:b))}) \
    -fs$@ \
    -I$(PRJ)$/$(INPATH)$/res$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(PRJ)$/..$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC1HEADER) $(RESLIB1SRSFILES) \
    )
.ENDIF			# "$(common_build_reslib)"!=""
.ELSE			# "$(NO_REC_RES)"!=""
    $(RSC) -presponse @$(mktmp \
    -r \
    $(rsclang_{$(subst,$(RESLIB1NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB1NAME)$(UPD), $(@:b))}) \
    -I$(RES) \
    -fs$@ \
    -I$(PRJ)$/$(INPATH)$/res \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(PRJ)$/..$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC1HEADER) $(RESLIB1SRSFILES) \
    )
.ENDIF			# "$(NO_REC_RES)"!=""
.ELSE				# "$(GUI)"=="UNX"
.IF "$(NO_REC_RES)"!=""
.IF "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    $(rsclang_{$(subst,$(RESLIB1NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB1NAME)$(UPD), $(@:b))}) \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(langext_{$(subst,$(RESLIB1NAME)$(UPD), $(@:b))}) \
    -fs{$(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)} \
    -r -p \
    -I$(RES) \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INCLUDE) \
    $(RSC1HEADER) $(RESLIB1SRSFILES) \
    )
    +-$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.new >& $(NULLDEV)
    +-$(RM) $@ >& $(NULLDEV)
    +$(RENAME) $@.new $@
.ELSE			# "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    $(rsclang_{$(subst,$(RESLIB1NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB1NAME)$(UPD), $(@:b))}) \
    -I$(RES)$/$(langext_{$(subst,$(RESLIB1NAME)$(UPD), $(@:b))}) \
    -fs$@ \
    -r -p \
    -I$(RES) \
    -I$(PRJ)$/$(INPATH)$/res$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INCLUDE) \
    $(RSC1HEADER) $(RESLIB1SRSFILES) \
    )
.ENDIF			# "$(common_build_reslib)"!=""
.ELSE			# "$(NO_REC_RES)"!=""
    $(RSC) -presponse @$(mktmp \
    $(rsclang_{$(subst,$(RESLIB1NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB1NAME)$(UPD), $(@:b))}) \
    -I$(RES) \
    -fs$@ \
    -r -p \
    -I$(RES) \
    -I$(PRJ)$/$(INPATH)$/res \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INCLUDE) \
    $(RSC1HEADER) $(RESLIB1SRSFILES) \
    )
.ENDIF			# "$(NO_REC_RES)"!=""
.ENDIF              # "$(GUI)"=="UNX"
.ENDIF				# "$(RESLIB1TARGETN)"!=""


# Anweisungen fuer das Linken
#next Target
# unroll begin

.IF "$(RESLIB2TARGETN)"!=""

$(RESLIB2TARGETN): \
        $(RESLIB2SRSFILES) \
        $(RESLIB2BMPS)
    @echo Making: $@
.IF "$(GUI)"=="UNX"
.IF "$(NO_REC_RES)"!=""
.IF "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r \
    $(rsclang_{$(subst,$(RESLIB2NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB2NAME)$(UPD), $(@:b))}) \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(langext_{$(subst,$(RESLIB2NAME)$(UPD), $(@:b))}) \
    -fs{$(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)} \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(PRJ)$/..$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC2HEADER) $(RESLIB2SRSFILES) \
    )
    +$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.new >& $(NULLDEV)
    +-$(RM) $@ >& $(NULLDEV)
    +$(RENAME) $@.new $@
.ELSE			# "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r \
    $(rsclang_{$(subst,$(RESLIB2NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB2NAME)$(UPD), $(@:b))}) \
    -I$(RES)$/$(langext_{$(subst,$(RESLIB2NAME)$(UPD), $(@:b))}) \
    -fs$@ \
    -I$(PRJ)$/$(INPATH)$/res$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(PRJ)$/..$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC2HEADER) $(RESLIB2SRSFILES) \
    )
.ENDIF			# "$(common_build_reslib)"!=""
.ELSE			# "$(NO_REC_RES)"!=""
    $(RSC) -presponse @$(mktmp \
    -r \
    $(rsclang_{$(subst,$(RESLIB2NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB2NAME)$(UPD), $(@:b))}) \
    -I$(RES) \
    -fs$@ \
    -I$(PRJ)$/$(INPATH)$/res \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(PRJ)$/..$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC2HEADER) $(RESLIB2SRSFILES) \
    )
.ENDIF			# "$(NO_REC_RES)"!=""
.ELSE				# "$(GUI)"=="UNX"
.IF "$(NO_REC_RES)"!=""
.IF "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    $(rsclang_{$(subst,$(RESLIB2NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB2NAME)$(UPD), $(@:b))}) \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(langext_{$(subst,$(RESLIB2NAME)$(UPD), $(@:b))}) \
    -fs{$(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)} \
    -r -p \
    -I$(RES) \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INCLUDE) \
    $(RSC2HEADER) $(RESLIB2SRSFILES) \
    )
    +-$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.new >& $(NULLDEV)
    +-$(RM) $@ >& $(NULLDEV)
    +$(RENAME) $@.new $@
.ELSE			# "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    $(rsclang_{$(subst,$(RESLIB2NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB2NAME)$(UPD), $(@:b))}) \
    -I$(RES)$/$(langext_{$(subst,$(RESLIB2NAME)$(UPD), $(@:b))}) \
    -fs$@ \
    -r -p \
    -I$(RES) \
    -I$(PRJ)$/$(INPATH)$/res$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INCLUDE) \
    $(RSC2HEADER) $(RESLIB2SRSFILES) \
    )
.ENDIF			# "$(common_build_reslib)"!=""
.ELSE			# "$(NO_REC_RES)"!=""
    $(RSC) -presponse @$(mktmp \
    $(rsclang_{$(subst,$(RESLIB2NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB2NAME)$(UPD), $(@:b))}) \
    -I$(RES) \
    -fs$@ \
    -r -p \
    -I$(RES) \
    -I$(PRJ)$/$(INPATH)$/res \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INCLUDE) \
    $(RSC2HEADER) $(RESLIB2SRSFILES) \
    )
.ENDIF			# "$(NO_REC_RES)"!=""
.ENDIF              # "$(GUI)"=="UNX"
.ENDIF				# "$(RESLIB2TARGETN)"!=""


# Anweisungen fuer das Linken
#next Target
# unroll begin

.IF "$(RESLIB3TARGETN)"!=""

$(RESLIB3TARGETN): \
        $(RESLIB3SRSFILES) \
        $(RESLIB3BMPS)
    @echo Making: $@
.IF "$(GUI)"=="UNX"
.IF "$(NO_REC_RES)"!=""
.IF "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r \
    $(rsclang_{$(subst,$(RESLIB3NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB3NAME)$(UPD), $(@:b))}) \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(langext_{$(subst,$(RESLIB3NAME)$(UPD), $(@:b))}) \
    -fs{$(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)} \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(PRJ)$/..$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC3HEADER) $(RESLIB3SRSFILES) \
    )
    +$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.new >& $(NULLDEV)
    +-$(RM) $@ >& $(NULLDEV)
    +$(RENAME) $@.new $@
.ELSE			# "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r \
    $(rsclang_{$(subst,$(RESLIB3NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB3NAME)$(UPD), $(@:b))}) \
    -I$(RES)$/$(langext_{$(subst,$(RESLIB3NAME)$(UPD), $(@:b))}) \
    -fs$@ \
    -I$(PRJ)$/$(INPATH)$/res$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(PRJ)$/..$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC3HEADER) $(RESLIB3SRSFILES) \
    )
.ENDIF			# "$(common_build_reslib)"!=""
.ELSE			# "$(NO_REC_RES)"!=""
    $(RSC) -presponse @$(mktmp \
    -r \
    $(rsclang_{$(subst,$(RESLIB3NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB3NAME)$(UPD), $(@:b))}) \
    -I$(RES) \
    -fs$@ \
    -I$(PRJ)$/$(INPATH)$/res \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(PRJ)$/..$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC3HEADER) $(RESLIB3SRSFILES) \
    )
.ENDIF			# "$(NO_REC_RES)"!=""
.ELSE				# "$(GUI)"=="UNX"
.IF "$(NO_REC_RES)"!=""
.IF "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    $(rsclang_{$(subst,$(RESLIB3NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB3NAME)$(UPD), $(@:b))}) \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(langext_{$(subst,$(RESLIB3NAME)$(UPD), $(@:b))}) \
    -fs{$(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)} \
    -r -p \
    -I$(RES) \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INCLUDE) \
    $(RSC3HEADER) $(RESLIB3SRSFILES) \
    )
    +-$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.new >& $(NULLDEV)
    +-$(RM) $@ >& $(NULLDEV)
    +$(RENAME) $@.new $@
.ELSE			# "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    $(rsclang_{$(subst,$(RESLIB3NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB3NAME)$(UPD), $(@:b))}) \
    -I$(RES)$/$(langext_{$(subst,$(RESLIB3NAME)$(UPD), $(@:b))}) \
    -fs$@ \
    -r -p \
    -I$(RES) \
    -I$(PRJ)$/$(INPATH)$/res$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INCLUDE) \
    $(RSC3HEADER) $(RESLIB3SRSFILES) \
    )
.ENDIF			# "$(common_build_reslib)"!=""
.ELSE			# "$(NO_REC_RES)"!=""
    $(RSC) -presponse @$(mktmp \
    $(rsclang_{$(subst,$(RESLIB3NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB3NAME)$(UPD), $(@:b))}) \
    -I$(RES) \
    -fs$@ \
    -r -p \
    -I$(RES) \
    -I$(PRJ)$/$(INPATH)$/res \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INCLUDE) \
    $(RSC3HEADER) $(RESLIB3SRSFILES) \
    )
.ENDIF			# "$(NO_REC_RES)"!=""
.ENDIF              # "$(GUI)"=="UNX"
.ENDIF				# "$(RESLIB3TARGETN)"!=""


# Anweisungen fuer das Linken
#next Target
# unroll begin

.IF "$(RESLIB4TARGETN)"!=""

$(RESLIB4TARGETN): \
        $(RESLIB4SRSFILES) \
        $(RESLIB4BMPS)
    @echo Making: $@
.IF "$(GUI)"=="UNX"
.IF "$(NO_REC_RES)"!=""
.IF "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r \
    $(rsclang_{$(subst,$(RESLIB4NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB4NAME)$(UPD), $(@:b))}) \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(langext_{$(subst,$(RESLIB4NAME)$(UPD), $(@:b))}) \
    -fs{$(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)} \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(PRJ)$/..$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC4HEADER) $(RESLIB4SRSFILES) \
    )
    +$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.new >& $(NULLDEV)
    +-$(RM) $@ >& $(NULLDEV)
    +$(RENAME) $@.new $@
.ELSE			# "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r \
    $(rsclang_{$(subst,$(RESLIB4NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB4NAME)$(UPD), $(@:b))}) \
    -I$(RES)$/$(langext_{$(subst,$(RESLIB4NAME)$(UPD), $(@:b))}) \
    -fs$@ \
    -I$(PRJ)$/$(INPATH)$/res$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(PRJ)$/..$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC4HEADER) $(RESLIB4SRSFILES) \
    )
.ENDIF			# "$(common_build_reslib)"!=""
.ELSE			# "$(NO_REC_RES)"!=""
    $(RSC) -presponse @$(mktmp \
    -r \
    $(rsclang_{$(subst,$(RESLIB4NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB4NAME)$(UPD), $(@:b))}) \
    -I$(RES) \
    -fs$@ \
    -I$(PRJ)$/$(INPATH)$/res \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(PRJ)$/..$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC4HEADER) $(RESLIB4SRSFILES) \
    )
.ENDIF			# "$(NO_REC_RES)"!=""
.ELSE				# "$(GUI)"=="UNX"
.IF "$(NO_REC_RES)"!=""
.IF "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    $(rsclang_{$(subst,$(RESLIB4NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB4NAME)$(UPD), $(@:b))}) \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(langext_{$(subst,$(RESLIB4NAME)$(UPD), $(@:b))}) \
    -fs{$(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)} \
    -r -p \
    -I$(RES) \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INCLUDE) \
    $(RSC4HEADER) $(RESLIB4SRSFILES) \
    )
    +-$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.new >& $(NULLDEV)
    +-$(RM) $@ >& $(NULLDEV)
    +$(RENAME) $@.new $@
.ELSE			# "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    $(rsclang_{$(subst,$(RESLIB4NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB4NAME)$(UPD), $(@:b))}) \
    -I$(RES)$/$(langext_{$(subst,$(RESLIB4NAME)$(UPD), $(@:b))}) \
    -fs$@ \
    -r -p \
    -I$(RES) \
    -I$(PRJ)$/$(INPATH)$/res$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INCLUDE) \
    $(RSC4HEADER) $(RESLIB4SRSFILES) \
    )
.ENDIF			# "$(common_build_reslib)"!=""
.ELSE			# "$(NO_REC_RES)"!=""
    $(RSC) -presponse @$(mktmp \
    $(rsclang_{$(subst,$(RESLIB4NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB4NAME)$(UPD), $(@:b))}) \
    -I$(RES) \
    -fs$@ \
    -r -p \
    -I$(RES) \
    -I$(PRJ)$/$(INPATH)$/res \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INCLUDE) \
    $(RSC4HEADER) $(RESLIB4SRSFILES) \
    )
.ENDIF			# "$(NO_REC_RES)"!=""
.ENDIF              # "$(GUI)"=="UNX"
.ENDIF				# "$(RESLIB4TARGETN)"!=""


# Anweisungen fuer das Linken
#next Target
# unroll begin

.IF "$(RESLIB5TARGETN)"!=""

$(RESLIB5TARGETN): \
        $(RESLIB5SRSFILES) \
        $(RESLIB5BMPS)
    @echo Making: $@
.IF "$(GUI)"=="UNX"
.IF "$(NO_REC_RES)"!=""
.IF "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r \
    $(rsclang_{$(subst,$(RESLIB5NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB5NAME)$(UPD), $(@:b))}) \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(langext_{$(subst,$(RESLIB5NAME)$(UPD), $(@:b))}) \
    -fs{$(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)} \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(PRJ)$/..$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC5HEADER) $(RESLIB5SRSFILES) \
    )
    +$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.new >& $(NULLDEV)
    +-$(RM) $@ >& $(NULLDEV)
    +$(RENAME) $@.new $@
.ELSE			# "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r \
    $(rsclang_{$(subst,$(RESLIB5NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB5NAME)$(UPD), $(@:b))}) \
    -I$(RES)$/$(langext_{$(subst,$(RESLIB5NAME)$(UPD), $(@:b))}) \
    -fs$@ \
    -I$(PRJ)$/$(INPATH)$/res$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(PRJ)$/..$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC5HEADER) $(RESLIB5SRSFILES) \
    )
.ENDIF			# "$(common_build_reslib)"!=""
.ELSE			# "$(NO_REC_RES)"!=""
    $(RSC) -presponse @$(mktmp \
    -r \
    $(rsclang_{$(subst,$(RESLIB5NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB5NAME)$(UPD), $(@:b))}) \
    -I$(RES) \
    -fs$@ \
    -I$(PRJ)$/$(INPATH)$/res \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(PRJ)$/..$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC5HEADER) $(RESLIB5SRSFILES) \
    )
.ENDIF			# "$(NO_REC_RES)"!=""
.ELSE				# "$(GUI)"=="UNX"
.IF "$(NO_REC_RES)"!=""
.IF "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    $(rsclang_{$(subst,$(RESLIB5NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB5NAME)$(UPD), $(@:b))}) \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(langext_{$(subst,$(RESLIB5NAME)$(UPD), $(@:b))}) \
    -fs{$(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)} \
    -r -p \
    -I$(RES) \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INCLUDE) \
    $(RSC5HEADER) $(RESLIB5SRSFILES) \
    )
    +-$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.new >& $(NULLDEV)
    +-$(RM) $@ >& $(NULLDEV)
    +$(RENAME) $@.new $@
.ELSE			# "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    $(rsclang_{$(subst,$(RESLIB5NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB5NAME)$(UPD), $(@:b))}) \
    -I$(RES)$/$(langext_{$(subst,$(RESLIB5NAME)$(UPD), $(@:b))}) \
    -fs$@ \
    -r -p \
    -I$(RES) \
    -I$(PRJ)$/$(INPATH)$/res$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INCLUDE) \
    $(RSC5HEADER) $(RESLIB5SRSFILES) \
    )
.ENDIF			# "$(common_build_reslib)"!=""
.ELSE			# "$(NO_REC_RES)"!=""
    $(RSC) -presponse @$(mktmp \
    $(rsclang_{$(subst,$(RESLIB5NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB5NAME)$(UPD), $(@:b))}) \
    -I$(RES) \
    -fs$@ \
    -r -p \
    -I$(RES) \
    -I$(PRJ)$/$(INPATH)$/res \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INCLUDE) \
    $(RSC5HEADER) $(RESLIB5SRSFILES) \
    )
.ENDIF			# "$(NO_REC_RES)"!=""
.ENDIF              # "$(GUI)"=="UNX"
.ENDIF				# "$(RESLIB5TARGETN)"!=""


# Anweisungen fuer das Linken
#next Target
# unroll begin

.IF "$(RESLIB6TARGETN)"!=""

$(RESLIB6TARGETN): \
        $(RESLIB6SRSFILES) \
        $(RESLIB6BMPS)
    @echo Making: $@
.IF "$(GUI)"=="UNX"
.IF "$(NO_REC_RES)"!=""
.IF "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r \
    $(rsclang_{$(subst,$(RESLIB6NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB6NAME)$(UPD), $(@:b))}) \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(langext_{$(subst,$(RESLIB6NAME)$(UPD), $(@:b))}) \
    -fs{$(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)} \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(PRJ)$/..$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC6HEADER) $(RESLIB6SRSFILES) \
    )
    +$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.new >& $(NULLDEV)
    +-$(RM) $@ >& $(NULLDEV)
    +$(RENAME) $@.new $@
.ELSE			# "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r \
    $(rsclang_{$(subst,$(RESLIB6NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB6NAME)$(UPD), $(@:b))}) \
    -I$(RES)$/$(langext_{$(subst,$(RESLIB6NAME)$(UPD), $(@:b))}) \
    -fs$@ \
    -I$(PRJ)$/$(INPATH)$/res$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(PRJ)$/..$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC6HEADER) $(RESLIB6SRSFILES) \
    )
.ENDIF			# "$(common_build_reslib)"!=""
.ELSE			# "$(NO_REC_RES)"!=""
    $(RSC) -presponse @$(mktmp \
    -r \
    $(rsclang_{$(subst,$(RESLIB6NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB6NAME)$(UPD), $(@:b))}) \
    -I$(RES) \
    -fs$@ \
    -I$(PRJ)$/$(INPATH)$/res \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(PRJ)$/..$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC6HEADER) $(RESLIB6SRSFILES) \
    )
.ENDIF			# "$(NO_REC_RES)"!=""
.ELSE				# "$(GUI)"=="UNX"
.IF "$(NO_REC_RES)"!=""
.IF "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    $(rsclang_{$(subst,$(RESLIB6NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB6NAME)$(UPD), $(@:b))}) \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(langext_{$(subst,$(RESLIB6NAME)$(UPD), $(@:b))}) \
    -fs{$(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)} \
    -r -p \
    -I$(RES) \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INCLUDE) \
    $(RSC6HEADER) $(RESLIB6SRSFILES) \
    )
    +-$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.new >& $(NULLDEV)
    +-$(RM) $@ >& $(NULLDEV)
    +$(RENAME) $@.new $@
.ELSE			# "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    $(rsclang_{$(subst,$(RESLIB6NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB6NAME)$(UPD), $(@:b))}) \
    -I$(RES)$/$(langext_{$(subst,$(RESLIB6NAME)$(UPD), $(@:b))}) \
    -fs$@ \
    -r -p \
    -I$(RES) \
    -I$(PRJ)$/$(INPATH)$/res$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INCLUDE) \
    $(RSC6HEADER) $(RESLIB6SRSFILES) \
    )
.ENDIF			# "$(common_build_reslib)"!=""
.ELSE			# "$(NO_REC_RES)"!=""
    $(RSC) -presponse @$(mktmp \
    $(rsclang_{$(subst,$(RESLIB6NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB6NAME)$(UPD), $(@:b))}) \
    -I$(RES) \
    -fs$@ \
    -r -p \
    -I$(RES) \
    -I$(PRJ)$/$(INPATH)$/res \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INCLUDE) \
    $(RSC6HEADER) $(RESLIB6SRSFILES) \
    )
.ENDIF			# "$(NO_REC_RES)"!=""
.ENDIF              # "$(GUI)"=="UNX"
.ENDIF				# "$(RESLIB6TARGETN)"!=""


# Anweisungen fuer das Linken
#next Target
# unroll begin

.IF "$(RESLIB7TARGETN)"!=""

$(RESLIB7TARGETN): \
        $(RESLIB7SRSFILES) \
        $(RESLIB7BMPS)
    @echo Making: $@
.IF "$(GUI)"=="UNX"
.IF "$(NO_REC_RES)"!=""
.IF "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r \
    $(rsclang_{$(subst,$(RESLIB7NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB7NAME)$(UPD), $(@:b))}) \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(langext_{$(subst,$(RESLIB7NAME)$(UPD), $(@:b))}) \
    -fs{$(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)} \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(PRJ)$/..$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC7HEADER) $(RESLIB7SRSFILES) \
    )
    +$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.new >& $(NULLDEV)
    +-$(RM) $@ >& $(NULLDEV)
    +$(RENAME) $@.new $@
.ELSE			# "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r \
    $(rsclang_{$(subst,$(RESLIB7NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB7NAME)$(UPD), $(@:b))}) \
    -I$(RES)$/$(langext_{$(subst,$(RESLIB7NAME)$(UPD), $(@:b))}) \
    -fs$@ \
    -I$(PRJ)$/$(INPATH)$/res$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(PRJ)$/..$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC7HEADER) $(RESLIB7SRSFILES) \
    )
.ENDIF			# "$(common_build_reslib)"!=""
.ELSE			# "$(NO_REC_RES)"!=""
    $(RSC) -presponse @$(mktmp \
    -r \
    $(rsclang_{$(subst,$(RESLIB7NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB7NAME)$(UPD), $(@:b))}) \
    -I$(RES) \
    -fs$@ \
    -I$(PRJ)$/$(INPATH)$/res \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(PRJ)$/..$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC7HEADER) $(RESLIB7SRSFILES) \
    )
.ENDIF			# "$(NO_REC_RES)"!=""
.ELSE				# "$(GUI)"=="UNX"
.IF "$(NO_REC_RES)"!=""
.IF "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    $(rsclang_{$(subst,$(RESLIB7NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB7NAME)$(UPD), $(@:b))}) \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(langext_{$(subst,$(RESLIB7NAME)$(UPD), $(@:b))}) \
    -fs{$(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)} \
    -r -p \
    -I$(RES) \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INCLUDE) \
    $(RSC7HEADER) $(RESLIB7SRSFILES) \
    )
    +-$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.new >& $(NULLDEV)
    +-$(RM) $@ >& $(NULLDEV)
    +$(RENAME) $@.new $@
.ELSE			# "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    $(rsclang_{$(subst,$(RESLIB7NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB7NAME)$(UPD), $(@:b))}) \
    -I$(RES)$/$(langext_{$(subst,$(RESLIB7NAME)$(UPD), $(@:b))}) \
    -fs$@ \
    -r -p \
    -I$(RES) \
    -I$(PRJ)$/$(INPATH)$/res$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INCLUDE) \
    $(RSC7HEADER) $(RESLIB7SRSFILES) \
    )
.ENDIF			# "$(common_build_reslib)"!=""
.ELSE			# "$(NO_REC_RES)"!=""
    $(RSC) -presponse @$(mktmp \
    $(rsclang_{$(subst,$(RESLIB7NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB7NAME)$(UPD), $(@:b))}) \
    -I$(RES) \
    -fs$@ \
    -r -p \
    -I$(RES) \
    -I$(PRJ)$/$(INPATH)$/res \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INCLUDE) \
    $(RSC7HEADER) $(RESLIB7SRSFILES) \
    )
.ENDIF			# "$(NO_REC_RES)"!=""
.ENDIF              # "$(GUI)"=="UNX"
.ENDIF				# "$(RESLIB7TARGETN)"!=""


# Anweisungen fuer das Linken
#next Target
# unroll begin

.IF "$(RESLIB8TARGETN)"!=""

$(RESLIB8TARGETN): \
        $(RESLIB8SRSFILES) \
        $(RESLIB8BMPS)
    @echo Making: $@
.IF "$(GUI)"=="UNX"
.IF "$(NO_REC_RES)"!=""
.IF "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r \
    $(rsclang_{$(subst,$(RESLIB8NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB8NAME)$(UPD), $(@:b))}) \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(langext_{$(subst,$(RESLIB8NAME)$(UPD), $(@:b))}) \
    -fs{$(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)} \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(PRJ)$/..$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC8HEADER) $(RESLIB8SRSFILES) \
    )
    +$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.new >& $(NULLDEV)
    +-$(RM) $@ >& $(NULLDEV)
    +$(RENAME) $@.new $@
.ELSE			# "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r \
    $(rsclang_{$(subst,$(RESLIB8NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB8NAME)$(UPD), $(@:b))}) \
    -I$(RES)$/$(langext_{$(subst,$(RESLIB8NAME)$(UPD), $(@:b))}) \
    -fs$@ \
    -I$(PRJ)$/$(INPATH)$/res$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(PRJ)$/..$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC8HEADER) $(RESLIB8SRSFILES) \
    )
.ENDIF			# "$(common_build_reslib)"!=""
.ELSE			# "$(NO_REC_RES)"!=""
    $(RSC) -presponse @$(mktmp \
    -r \
    $(rsclang_{$(subst,$(RESLIB8NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB8NAME)$(UPD), $(@:b))}) \
    -I$(RES) \
    -fs$@ \
    -I$(PRJ)$/$(INPATH)$/res \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(PRJ)$/..$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC8HEADER) $(RESLIB8SRSFILES) \
    )
.ENDIF			# "$(NO_REC_RES)"!=""
.ELSE				# "$(GUI)"=="UNX"
.IF "$(NO_REC_RES)"!=""
.IF "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    $(rsclang_{$(subst,$(RESLIB8NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB8NAME)$(UPD), $(@:b))}) \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(langext_{$(subst,$(RESLIB8NAME)$(UPD), $(@:b))}) \
    -fs{$(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)} \
    -r -p \
    -I$(RES) \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INCLUDE) \
    $(RSC8HEADER) $(RESLIB8SRSFILES) \
    )
    +-$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.new >& $(NULLDEV)
    +-$(RM) $@ >& $(NULLDEV)
    +$(RENAME) $@.new $@
.ELSE			# "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    $(rsclang_{$(subst,$(RESLIB8NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB8NAME)$(UPD), $(@:b))}) \
    -I$(RES)$/$(langext_{$(subst,$(RESLIB8NAME)$(UPD), $(@:b))}) \
    -fs$@ \
    -r -p \
    -I$(RES) \
    -I$(PRJ)$/$(INPATH)$/res$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INCLUDE) \
    $(RSC8HEADER) $(RESLIB8SRSFILES) \
    )
.ENDIF			# "$(common_build_reslib)"!=""
.ELSE			# "$(NO_REC_RES)"!=""
    $(RSC) -presponse @$(mktmp \
    $(rsclang_{$(subst,$(RESLIB8NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB8NAME)$(UPD), $(@:b))}) \
    -I$(RES) \
    -fs$@ \
    -r -p \
    -I$(RES) \
    -I$(PRJ)$/$(INPATH)$/res \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INCLUDE) \
    $(RSC8HEADER) $(RESLIB8SRSFILES) \
    )
.ENDIF			# "$(NO_REC_RES)"!=""
.ENDIF              # "$(GUI)"=="UNX"
.ENDIF				# "$(RESLIB8TARGETN)"!=""


# Anweisungen fuer das Linken
#next Target
# unroll begin

.IF "$(RESLIB9TARGETN)"!=""

$(RESLIB9TARGETN): \
        $(RESLIB9SRSFILES) \
        $(RESLIB9BMPS)
    @echo Making: $@
.IF "$(GUI)"=="UNX"
.IF "$(NO_REC_RES)"!=""
.IF "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r \
    $(rsclang_{$(subst,$(RESLIB9NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB9NAME)$(UPD), $(@:b))}) \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(langext_{$(subst,$(RESLIB9NAME)$(UPD), $(@:b))}) \
    -fs{$(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)} \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(PRJ)$/..$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC9HEADER) $(RESLIB9SRSFILES) \
    )
    +$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.new >& $(NULLDEV)
    +-$(RM) $@ >& $(NULLDEV)
    +$(RENAME) $@.new $@
.ELSE			# "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r \
    $(rsclang_{$(subst,$(RESLIB9NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB9NAME)$(UPD), $(@:b))}) \
    -I$(RES)$/$(langext_{$(subst,$(RESLIB9NAME)$(UPD), $(@:b))}) \
    -fs$@ \
    -I$(PRJ)$/$(INPATH)$/res$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(PRJ)$/..$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC9HEADER) $(RESLIB9SRSFILES) \
    )
.ENDIF			# "$(common_build_reslib)"!=""
.ELSE			# "$(NO_REC_RES)"!=""
    $(RSC) -presponse @$(mktmp \
    -r \
    $(rsclang_{$(subst,$(RESLIB9NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB9NAME)$(UPD), $(@:b))}) \
    -I$(RES) \
    -fs$@ \
    -I$(PRJ)$/$(INPATH)$/res \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(PRJ)$/..$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC9HEADER) $(RESLIB9SRSFILES) \
    )
.ENDIF			# "$(NO_REC_RES)"!=""
.ELSE				# "$(GUI)"=="UNX"
.IF "$(NO_REC_RES)"!=""
.IF "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    $(rsclang_{$(subst,$(RESLIB9NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB9NAME)$(UPD), $(@:b))}) \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(langext_{$(subst,$(RESLIB9NAME)$(UPD), $(@:b))}) \
    -fs{$(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)} \
    -r -p \
    -I$(RES) \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INCLUDE) \
    $(RSC9HEADER) $(RESLIB9SRSFILES) \
    )
    +-$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.new >& $(NULLDEV)
    +-$(RM) $@ >& $(NULLDEV)
    +$(RENAME) $@.new $@
.ELSE			# "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    $(rsclang_{$(subst,$(RESLIB9NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB9NAME)$(UPD), $(@:b))}) \
    -I$(RES)$/$(langext_{$(subst,$(RESLIB9NAME)$(UPD), $(@:b))}) \
    -fs$@ \
    -r -p \
    -I$(RES) \
    -I$(PRJ)$/$(INPATH)$/res$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INCLUDE) \
    $(RSC9HEADER) $(RESLIB9SRSFILES) \
    )
.ENDIF			# "$(common_build_reslib)"!=""
.ELSE			# "$(NO_REC_RES)"!=""
    $(RSC) -presponse @$(mktmp \
    $(rsclang_{$(subst,$(RESLIB9NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB9NAME)$(UPD), $(@:b))}) \
    -I$(RES) \
    -fs$@ \
    -r -p \
    -I$(RES) \
    -I$(PRJ)$/$(INPATH)$/res \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INCLUDE) \
    $(RSC9HEADER) $(RESLIB9SRSFILES) \
    )
.ENDIF			# "$(NO_REC_RES)"!=""
.ENDIF              # "$(GUI)"=="UNX"
.ENDIF				# "$(RESLIB9TARGETN)"!=""


# Anweisungen fuer das Linken
#next Target
# unroll begin

.IF "$(RESLIB10TARGETN)"!=""

$(RESLIB10TARGETN): \
        $(RESLIB10SRSFILES) \
        $(RESLIB10BMPS)
    @echo Making: $@
.IF "$(GUI)"=="UNX"
.IF "$(NO_REC_RES)"!=""
.IF "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r \
    $(rsclang_{$(subst,$(RESLIB10NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB10NAME)$(UPD), $(@:b))}) \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(langext_{$(subst,$(RESLIB10NAME)$(UPD), $(@:b))}) \
    -fs{$(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)} \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(PRJ)$/..$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC10HEADER) $(RESLIB10SRSFILES) \
    )
    +$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.new >& $(NULLDEV)
    +-$(RM) $@ >& $(NULLDEV)
    +$(RENAME) $@.new $@
.ELSE			# "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r \
    $(rsclang_{$(subst,$(RESLIB10NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB10NAME)$(UPD), $(@:b))}) \
    -I$(RES)$/$(langext_{$(subst,$(RESLIB10NAME)$(UPD), $(@:b))}) \
    -fs$@ \
    -I$(PRJ)$/$(INPATH)$/res$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(PRJ)$/..$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC10HEADER) $(RESLIB10SRSFILES) \
    )
.ENDIF			# "$(common_build_reslib)"!=""
.ELSE			# "$(NO_REC_RES)"!=""
    $(RSC) -presponse @$(mktmp \
    -r \
    $(rsclang_{$(subst,$(RESLIB10NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB10NAME)$(UPD), $(@:b))}) \
    -I$(RES) \
    -fs$@ \
    -I$(PRJ)$/$(INPATH)$/res \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(PRJ)$/..$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC10HEADER) $(RESLIB10SRSFILES) \
    )
.ENDIF			# "$(NO_REC_RES)"!=""
.ELSE				# "$(GUI)"=="UNX"
.IF "$(NO_REC_RES)"!=""
.IF "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    $(rsclang_{$(subst,$(RESLIB10NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB10NAME)$(UPD), $(@:b))}) \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(langext_{$(subst,$(RESLIB10NAME)$(UPD), $(@:b))}) \
    -fs{$(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)} \
    -r -p \
    -I$(RES) \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INCLUDE) \
    $(RSC10HEADER) $(RESLIB10SRSFILES) \
    )
    +-$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.new >& $(NULLDEV)
    +-$(RM) $@ >& $(NULLDEV)
    +$(RENAME) $@.new $@
.ELSE			# "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    $(rsclang_{$(subst,$(RESLIB10NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB10NAME)$(UPD), $(@:b))}) \
    -I$(RES)$/$(langext_{$(subst,$(RESLIB10NAME)$(UPD), $(@:b))}) \
    -fs$@ \
    -r -p \
    -I$(RES) \
    -I$(PRJ)$/$(INPATH)$/res$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INCLUDE) \
    $(RSC10HEADER) $(RESLIB10SRSFILES) \
    )
.ENDIF			# "$(common_build_reslib)"!=""
.ELSE			# "$(NO_REC_RES)"!=""
    $(RSC) -presponse @$(mktmp \
    $(rsclang_{$(subst,$(RESLIB10NAME)$(UPD), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB10NAME)$(UPD), $(@:b))}) \
    -I$(RES) \
    -fs$@ \
    -r -p \
    -I$(RES) \
    -I$(PRJ)$/$(INPATH)$/res \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INCLUDE) \
    $(RSC10HEADER) $(RESLIB10SRSFILES) \
    )
.ENDIF			# "$(NO_REC_RES)"!=""
.ENDIF              # "$(GUI)"=="UNX"
.ENDIF				# "$(RESLIB10TARGETN)"!=""


# Anweisungen fuer das Linken
#next Target
