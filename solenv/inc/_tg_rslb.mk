# unroll begin

.IF "$(RESLIB1TARGETN)"!=""

$(RSC_MULTI1) : \
        $(RESLIB1SRSFILES) \
        $(RESLIB1TARGETN) \
        $(RESLIB1BMPS)
.IF "$(rsc_once)"!=""
    @+echo using rsc multi-res feature
.IF "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangext) $(rsclang_{$i}) \
    $(rescharset_{$i}) \
    -fs{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB1NAME)$(RESLIB1VERSION)$i.res} \
    -lip{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(langext_{$(i)}) ) \
    -ft$@ \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC1HEADER) $(RESLIB1SRSFILES) \
    ) > $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangext) $(rsclang_{$i}) \
    $(rescharset_{$i}) \
    -fs{$(BIN)$/$(RESLIB1NAME)$(RESLIB1VERSION)$i.res} \
    -lip{$(RES)}$/$(langext_{$(i)}) ) \
    -ft$@ \
    -I{$(RES)}$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC1HEADER) $(RESLIB1SRSFILES) \
    ) > $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""
.ENDIF           # "$(rsc_once)"!=""

$(RESLIB1TARGETN): \
        $(RESLIB1SRSFILES) \
        $(RESLIB1BMPS)
    @echo Making: $@
.IF "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
.IF "$(rsc_once)"!=""
    @+-$(RM) $(RSC_MULTI1) >& $(NULLDEV)
.ELSE           # "$(rsc_once)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(rsclang_{$(subst,$(RESLIB1NAME)$(RESLIB1VERSION), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB1NAME)$(RESLIB1VERSION), $(@:b))}) \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(langext_{$(subst,$(RESLIB1NAME)$(RESLIB1VERSION), $(@:b))}) \
    -fs{$(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)} \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(PRJ)$/..$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC1HEADER) $(RESLIB1SRSFILES) \
    )
    +$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.new >& $(NULLDEV)
    +-$(RM) $@ >& $(NULLDEV)
    +$(RENAME) $@.new $@
.ENDIF          # "$(rsc_once)"!=""
.ELSE			# "$(common_build_reslib)"!=""
.IF "$(rsc_once)"!=""
    @+-$(RM) $(RSC_MULTI1) >& $(NULLDEV)
.ELSE           # "$(rsc_once)"!=""
    $(RSC) -presponse @$(mktmp \
    -r \
    $(rsclang_{$(subst,$(RESLIB1NAME)$(RESLIB1VERSION), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB1NAME)$(RESLIB1VERSION), $(@:b))}) \
    -I$(RES)$/$(langext_{$(subst,$(RESLIB1NAME)$(RESLIB1VERSION), $(@:b))}) \
    -fs$@ \
    -I$(PRJ)$/$(INPATH)$/res$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(PRJ)$/..$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC1HEADER) $(RESLIB1SRSFILES) \
    )
.ENDIF          # "$(rsc_once)"!=""
.ENDIF			# "$(common_build_reslib)"!=""
.ELSE				# "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
.IF "$(rsc_once)"!=""
    @+-$(RM) $(RSC_MULTI1) >& $(NULLDEV)
.ELSE           # "$(rsc_once)"!=""
    $(RSC) -presponse @$(mktmp \
    $(rsclang_{$(subst,$(RESLIB1NAME)$(RESLIB1VERSION), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB1NAME)$(RESLIB1VERSION), $(@:b))}) \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(langext_{$(subst,$(RESLIB1NAME)$(RESLIB1VERSION), $(@:b))}) \
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
.ENDIF          # "$(rsc_once)"!=""
.ELSE			# "$(common_build_reslib)"!=""
.IF "$(rsc_once)"!=""
    @+-$(RM) $(RSC_MULTI1) >& $(NULLDEV)
.ELSE           # "$(rsc_once)"!=""
    $(RSC) -presponse @$(mktmp \
    $(rsclang_{$(subst,$(RESLIB1NAME)$(RESLIB1VERSION), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB1NAME)$(RESLIB1VERSION), $(@:b))}) \
    -I$(RES)$/$(langext_{$(subst,$(RESLIB1NAME)$(RESLIB1VERSION), $(@:b))}) \
    -fs$@ \
    -r -p \
    -I$(RES) \
    -I$(PRJ)$/$(INPATH)$/res$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INCLUDE) \
    $(RSC1HEADER) $(RESLIB1SRSFILES) \
    )
.ENDIF          # "$(rsc_once)"!=""
.ENDIF			# "$(common_build_reslib)"!=""
.ENDIF              # "$(GUI)"=="UNX"
.ENDIF				# "$(RESLIB1TARGETN)"!=""


# Anweisungen fuer das Linken
# unroll begin

.IF "$(RESLIB2TARGETN)"!=""

$(RSC_MULTI2) : \
        $(RESLIB2SRSFILES) \
        $(RESLIB2TARGETN) \
        $(RESLIB2BMPS)
.IF "$(rsc_once)"!=""
    @+echo using rsc multi-res feature
.IF "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangext) $(rsclang_{$i}) \
    $(rescharset_{$i}) \
    -fs{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB2NAME)$(RESLIB2VERSION)$i.res} \
    -lip{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(langext_{$(i)}) ) \
    -ft$@ \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC2HEADER) $(RESLIB2SRSFILES) \
    ) > $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangext) $(rsclang_{$i}) \
    $(rescharset_{$i}) \
    -fs{$(BIN)$/$(RESLIB2NAME)$(RESLIB2VERSION)$i.res} \
    -lip{$(RES)}$/$(langext_{$(i)}) ) \
    -ft$@ \
    -I{$(RES)}$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC2HEADER) $(RESLIB2SRSFILES) \
    ) > $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""
.ENDIF           # "$(rsc_once)"!=""

$(RESLIB2TARGETN): \
        $(RESLIB2SRSFILES) \
        $(RESLIB2BMPS)
    @echo Making: $@
.IF "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
.IF "$(rsc_once)"!=""
    @+-$(RM) $(RSC_MULTI2) >& $(NULLDEV)
.ELSE           # "$(rsc_once)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(rsclang_{$(subst,$(RESLIB2NAME)$(RESLIB2VERSION), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB2NAME)$(RESLIB2VERSION), $(@:b))}) \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(langext_{$(subst,$(RESLIB2NAME)$(RESLIB2VERSION), $(@:b))}) \
    -fs{$(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)} \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(PRJ)$/..$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC2HEADER) $(RESLIB2SRSFILES) \
    )
    +$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.new >& $(NULLDEV)
    +-$(RM) $@ >& $(NULLDEV)
    +$(RENAME) $@.new $@
.ENDIF          # "$(rsc_once)"!=""
.ELSE			# "$(common_build_reslib)"!=""
.IF "$(rsc_once)"!=""
    @+-$(RM) $(RSC_MULTI2) >& $(NULLDEV)
.ELSE           # "$(rsc_once)"!=""
    $(RSC) -presponse @$(mktmp \
    -r \
    $(rsclang_{$(subst,$(RESLIB2NAME)$(RESLIB2VERSION), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB2NAME)$(RESLIB2VERSION), $(@:b))}) \
    -I$(RES)$/$(langext_{$(subst,$(RESLIB2NAME)$(RESLIB2VERSION), $(@:b))}) \
    -fs$@ \
    -I$(PRJ)$/$(INPATH)$/res$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(PRJ)$/..$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC2HEADER) $(RESLIB2SRSFILES) \
    )
.ENDIF          # "$(rsc_once)"!=""
.ENDIF			# "$(common_build_reslib)"!=""
.ELSE				# "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
.IF "$(rsc_once)"!=""
    @+-$(RM) $(RSC_MULTI2) >& $(NULLDEV)
.ELSE           # "$(rsc_once)"!=""
    $(RSC) -presponse @$(mktmp \
    $(rsclang_{$(subst,$(RESLIB2NAME)$(RESLIB2VERSION), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB2NAME)$(RESLIB2VERSION), $(@:b))}) \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(langext_{$(subst,$(RESLIB2NAME)$(RESLIB2VERSION), $(@:b))}) \
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
.ENDIF          # "$(rsc_once)"!=""
.ELSE			# "$(common_build_reslib)"!=""
.IF "$(rsc_once)"!=""
    @+-$(RM) $(RSC_MULTI2) >& $(NULLDEV)
.ELSE           # "$(rsc_once)"!=""
    $(RSC) -presponse @$(mktmp \
    $(rsclang_{$(subst,$(RESLIB2NAME)$(RESLIB2VERSION), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB2NAME)$(RESLIB2VERSION), $(@:b))}) \
    -I$(RES)$/$(langext_{$(subst,$(RESLIB2NAME)$(RESLIB2VERSION), $(@:b))}) \
    -fs$@ \
    -r -p \
    -I$(RES) \
    -I$(PRJ)$/$(INPATH)$/res$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INCLUDE) \
    $(RSC2HEADER) $(RESLIB2SRSFILES) \
    )
.ENDIF          # "$(rsc_once)"!=""
.ENDIF			# "$(common_build_reslib)"!=""
.ENDIF              # "$(GUI)"=="UNX"
.ENDIF				# "$(RESLIB2TARGETN)"!=""


# Anweisungen fuer das Linken
# unroll begin

.IF "$(RESLIB3TARGETN)"!=""

$(RSC_MULTI3) : \
        $(RESLIB3SRSFILES) \
        $(RESLIB3TARGETN) \
        $(RESLIB3BMPS)
.IF "$(rsc_once)"!=""
    @+echo using rsc multi-res feature
.IF "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangext) $(rsclang_{$i}) \
    $(rescharset_{$i}) \
    -fs{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB3NAME)$(RESLIB3VERSION)$i.res} \
    -lip{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(langext_{$(i)}) ) \
    -ft$@ \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC3HEADER) $(RESLIB3SRSFILES) \
    ) > $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangext) $(rsclang_{$i}) \
    $(rescharset_{$i}) \
    -fs{$(BIN)$/$(RESLIB3NAME)$(RESLIB3VERSION)$i.res} \
    -lip{$(RES)}$/$(langext_{$(i)}) ) \
    -ft$@ \
    -I{$(RES)}$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC3HEADER) $(RESLIB3SRSFILES) \
    ) > $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""
.ENDIF           # "$(rsc_once)"!=""

$(RESLIB3TARGETN): \
        $(RESLIB3SRSFILES) \
        $(RESLIB3BMPS)
    @echo Making: $@
.IF "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
.IF "$(rsc_once)"!=""
    @+-$(RM) $(RSC_MULTI3) >& $(NULLDEV)
.ELSE           # "$(rsc_once)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(rsclang_{$(subst,$(RESLIB3NAME)$(RESLIB3VERSION), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB3NAME)$(RESLIB3VERSION), $(@:b))}) \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(langext_{$(subst,$(RESLIB3NAME)$(RESLIB3VERSION), $(@:b))}) \
    -fs{$(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)} \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(PRJ)$/..$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC3HEADER) $(RESLIB3SRSFILES) \
    )
    +$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.new >& $(NULLDEV)
    +-$(RM) $@ >& $(NULLDEV)
    +$(RENAME) $@.new $@
.ENDIF          # "$(rsc_once)"!=""
.ELSE			# "$(common_build_reslib)"!=""
.IF "$(rsc_once)"!=""
    @+-$(RM) $(RSC_MULTI3) >& $(NULLDEV)
.ELSE           # "$(rsc_once)"!=""
    $(RSC) -presponse @$(mktmp \
    -r \
    $(rsclang_{$(subst,$(RESLIB3NAME)$(RESLIB3VERSION), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB3NAME)$(RESLIB3VERSION), $(@:b))}) \
    -I$(RES)$/$(langext_{$(subst,$(RESLIB3NAME)$(RESLIB3VERSION), $(@:b))}) \
    -fs$@ \
    -I$(PRJ)$/$(INPATH)$/res$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(PRJ)$/..$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC3HEADER) $(RESLIB3SRSFILES) \
    )
.ENDIF          # "$(rsc_once)"!=""
.ENDIF			# "$(common_build_reslib)"!=""
.ELSE				# "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
.IF "$(rsc_once)"!=""
    @+-$(RM) $(RSC_MULTI3) >& $(NULLDEV)
.ELSE           # "$(rsc_once)"!=""
    $(RSC) -presponse @$(mktmp \
    $(rsclang_{$(subst,$(RESLIB3NAME)$(RESLIB3VERSION), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB3NAME)$(RESLIB3VERSION), $(@:b))}) \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(langext_{$(subst,$(RESLIB3NAME)$(RESLIB3VERSION), $(@:b))}) \
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
.ENDIF          # "$(rsc_once)"!=""
.ELSE			# "$(common_build_reslib)"!=""
.IF "$(rsc_once)"!=""
    @+-$(RM) $(RSC_MULTI3) >& $(NULLDEV)
.ELSE           # "$(rsc_once)"!=""
    $(RSC) -presponse @$(mktmp \
    $(rsclang_{$(subst,$(RESLIB3NAME)$(RESLIB3VERSION), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB3NAME)$(RESLIB3VERSION), $(@:b))}) \
    -I$(RES)$/$(langext_{$(subst,$(RESLIB3NAME)$(RESLIB3VERSION), $(@:b))}) \
    -fs$@ \
    -r -p \
    -I$(RES) \
    -I$(PRJ)$/$(INPATH)$/res$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INCLUDE) \
    $(RSC3HEADER) $(RESLIB3SRSFILES) \
    )
.ENDIF          # "$(rsc_once)"!=""
.ENDIF			# "$(common_build_reslib)"!=""
.ENDIF              # "$(GUI)"=="UNX"
.ENDIF				# "$(RESLIB3TARGETN)"!=""


# Anweisungen fuer das Linken
# unroll begin

.IF "$(RESLIB4TARGETN)"!=""

$(RSC_MULTI4) : \
        $(RESLIB4SRSFILES) \
        $(RESLIB4TARGETN) \
        $(RESLIB4BMPS)
.IF "$(rsc_once)"!=""
    @+echo using rsc multi-res feature
.IF "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangext) $(rsclang_{$i}) \
    $(rescharset_{$i}) \
    -fs{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB4NAME)$(RESLIB4VERSION)$i.res} \
    -lip{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(langext_{$(i)}) ) \
    -ft$@ \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC4HEADER) $(RESLIB4SRSFILES) \
    ) > $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangext) $(rsclang_{$i}) \
    $(rescharset_{$i}) \
    -fs{$(BIN)$/$(RESLIB4NAME)$(RESLIB4VERSION)$i.res} \
    -lip{$(RES)}$/$(langext_{$(i)}) ) \
    -ft$@ \
    -I{$(RES)}$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC4HEADER) $(RESLIB4SRSFILES) \
    ) > $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""
.ENDIF           # "$(rsc_once)"!=""

$(RESLIB4TARGETN): \
        $(RESLIB4SRSFILES) \
        $(RESLIB4BMPS)
    @echo Making: $@
.IF "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
.IF "$(rsc_once)"!=""
    @+-$(RM) $(RSC_MULTI4) >& $(NULLDEV)
.ELSE           # "$(rsc_once)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(rsclang_{$(subst,$(RESLIB4NAME)$(RESLIB4VERSION), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB4NAME)$(RESLIB4VERSION), $(@:b))}) \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(langext_{$(subst,$(RESLIB4NAME)$(RESLIB4VERSION), $(@:b))}) \
    -fs{$(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)} \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(PRJ)$/..$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC4HEADER) $(RESLIB4SRSFILES) \
    )
    +$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.new >& $(NULLDEV)
    +-$(RM) $@ >& $(NULLDEV)
    +$(RENAME) $@.new $@
.ENDIF          # "$(rsc_once)"!=""
.ELSE			# "$(common_build_reslib)"!=""
.IF "$(rsc_once)"!=""
    @+-$(RM) $(RSC_MULTI4) >& $(NULLDEV)
.ELSE           # "$(rsc_once)"!=""
    $(RSC) -presponse @$(mktmp \
    -r \
    $(rsclang_{$(subst,$(RESLIB4NAME)$(RESLIB4VERSION), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB4NAME)$(RESLIB4VERSION), $(@:b))}) \
    -I$(RES)$/$(langext_{$(subst,$(RESLIB4NAME)$(RESLIB4VERSION), $(@:b))}) \
    -fs$@ \
    -I$(PRJ)$/$(INPATH)$/res$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(PRJ)$/..$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC4HEADER) $(RESLIB4SRSFILES) \
    )
.ENDIF          # "$(rsc_once)"!=""
.ENDIF			# "$(common_build_reslib)"!=""
.ELSE				# "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
.IF "$(rsc_once)"!=""
    @+-$(RM) $(RSC_MULTI4) >& $(NULLDEV)
.ELSE           # "$(rsc_once)"!=""
    $(RSC) -presponse @$(mktmp \
    $(rsclang_{$(subst,$(RESLIB4NAME)$(RESLIB4VERSION), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB4NAME)$(RESLIB4VERSION), $(@:b))}) \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(langext_{$(subst,$(RESLIB4NAME)$(RESLIB4VERSION), $(@:b))}) \
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
.ENDIF          # "$(rsc_once)"!=""
.ELSE			# "$(common_build_reslib)"!=""
.IF "$(rsc_once)"!=""
    @+-$(RM) $(RSC_MULTI4) >& $(NULLDEV)
.ELSE           # "$(rsc_once)"!=""
    $(RSC) -presponse @$(mktmp \
    $(rsclang_{$(subst,$(RESLIB4NAME)$(RESLIB4VERSION), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB4NAME)$(RESLIB4VERSION), $(@:b))}) \
    -I$(RES)$/$(langext_{$(subst,$(RESLIB4NAME)$(RESLIB4VERSION), $(@:b))}) \
    -fs$@ \
    -r -p \
    -I$(RES) \
    -I$(PRJ)$/$(INPATH)$/res$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INCLUDE) \
    $(RSC4HEADER) $(RESLIB4SRSFILES) \
    )
.ENDIF          # "$(rsc_once)"!=""
.ENDIF			# "$(common_build_reslib)"!=""
.ENDIF              # "$(GUI)"=="UNX"
.ENDIF				# "$(RESLIB4TARGETN)"!=""


# Anweisungen fuer das Linken
# unroll begin

.IF "$(RESLIB5TARGETN)"!=""

$(RSC_MULTI5) : \
        $(RESLIB5SRSFILES) \
        $(RESLIB5TARGETN) \
        $(RESLIB5BMPS)
.IF "$(rsc_once)"!=""
    @+echo using rsc multi-res feature
.IF "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangext) $(rsclang_{$i}) \
    $(rescharset_{$i}) \
    -fs{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB5NAME)$(RESLIB5VERSION)$i.res} \
    -lip{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(langext_{$(i)}) ) \
    -ft$@ \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC5HEADER) $(RESLIB5SRSFILES) \
    ) > $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangext) $(rsclang_{$i}) \
    $(rescharset_{$i}) \
    -fs{$(BIN)$/$(RESLIB5NAME)$(RESLIB5VERSION)$i.res} \
    -lip{$(RES)}$/$(langext_{$(i)}) ) \
    -ft$@ \
    -I{$(RES)}$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC5HEADER) $(RESLIB5SRSFILES) \
    ) > $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""
.ENDIF           # "$(rsc_once)"!=""

$(RESLIB5TARGETN): \
        $(RESLIB5SRSFILES) \
        $(RESLIB5BMPS)
    @echo Making: $@
.IF "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
.IF "$(rsc_once)"!=""
    @+-$(RM) $(RSC_MULTI5) >& $(NULLDEV)
.ELSE           # "$(rsc_once)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(rsclang_{$(subst,$(RESLIB5NAME)$(RESLIB5VERSION), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB5NAME)$(RESLIB5VERSION), $(@:b))}) \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(langext_{$(subst,$(RESLIB5NAME)$(RESLIB5VERSION), $(@:b))}) \
    -fs{$(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)} \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(PRJ)$/..$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC5HEADER) $(RESLIB5SRSFILES) \
    )
    +$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.new >& $(NULLDEV)
    +-$(RM) $@ >& $(NULLDEV)
    +$(RENAME) $@.new $@
.ENDIF          # "$(rsc_once)"!=""
.ELSE			# "$(common_build_reslib)"!=""
.IF "$(rsc_once)"!=""
    @+-$(RM) $(RSC_MULTI5) >& $(NULLDEV)
.ELSE           # "$(rsc_once)"!=""
    $(RSC) -presponse @$(mktmp \
    -r \
    $(rsclang_{$(subst,$(RESLIB5NAME)$(RESLIB5VERSION), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB5NAME)$(RESLIB5VERSION), $(@:b))}) \
    -I$(RES)$/$(langext_{$(subst,$(RESLIB5NAME)$(RESLIB5VERSION), $(@:b))}) \
    -fs$@ \
    -I$(PRJ)$/$(INPATH)$/res$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(PRJ)$/..$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC5HEADER) $(RESLIB5SRSFILES) \
    )
.ENDIF          # "$(rsc_once)"!=""
.ENDIF			# "$(common_build_reslib)"!=""
.ELSE				# "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
.IF "$(rsc_once)"!=""
    @+-$(RM) $(RSC_MULTI5) >& $(NULLDEV)
.ELSE           # "$(rsc_once)"!=""
    $(RSC) -presponse @$(mktmp \
    $(rsclang_{$(subst,$(RESLIB5NAME)$(RESLIB5VERSION), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB5NAME)$(RESLIB5VERSION), $(@:b))}) \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(langext_{$(subst,$(RESLIB5NAME)$(RESLIB5VERSION), $(@:b))}) \
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
.ENDIF          # "$(rsc_once)"!=""
.ELSE			# "$(common_build_reslib)"!=""
.IF "$(rsc_once)"!=""
    @+-$(RM) $(RSC_MULTI5) >& $(NULLDEV)
.ELSE           # "$(rsc_once)"!=""
    $(RSC) -presponse @$(mktmp \
    $(rsclang_{$(subst,$(RESLIB5NAME)$(RESLIB5VERSION), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB5NAME)$(RESLIB5VERSION), $(@:b))}) \
    -I$(RES)$/$(langext_{$(subst,$(RESLIB5NAME)$(RESLIB5VERSION), $(@:b))}) \
    -fs$@ \
    -r -p \
    -I$(RES) \
    -I$(PRJ)$/$(INPATH)$/res$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INCLUDE) \
    $(RSC5HEADER) $(RESLIB5SRSFILES) \
    )
.ENDIF          # "$(rsc_once)"!=""
.ENDIF			# "$(common_build_reslib)"!=""
.ENDIF              # "$(GUI)"=="UNX"
.ENDIF				# "$(RESLIB5TARGETN)"!=""


# Anweisungen fuer das Linken
# unroll begin

.IF "$(RESLIB6TARGETN)"!=""

$(RSC_MULTI6) : \
        $(RESLIB6SRSFILES) \
        $(RESLIB6TARGETN) \
        $(RESLIB6BMPS)
.IF "$(rsc_once)"!=""
    @+echo using rsc multi-res feature
.IF "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangext) $(rsclang_{$i}) \
    $(rescharset_{$i}) \
    -fs{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB6NAME)$(RESLIB6VERSION)$i.res} \
    -lip{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(langext_{$(i)}) ) \
    -ft$@ \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC6HEADER) $(RESLIB6SRSFILES) \
    ) > $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangext) $(rsclang_{$i}) \
    $(rescharset_{$i}) \
    -fs{$(BIN)$/$(RESLIB6NAME)$(RESLIB6VERSION)$i.res} \
    -lip{$(RES)}$/$(langext_{$(i)}) ) \
    -ft$@ \
    -I{$(RES)}$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC6HEADER) $(RESLIB6SRSFILES) \
    ) > $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""
.ENDIF           # "$(rsc_once)"!=""

$(RESLIB6TARGETN): \
        $(RESLIB6SRSFILES) \
        $(RESLIB6BMPS)
    @echo Making: $@
.IF "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
.IF "$(rsc_once)"!=""
    @+-$(RM) $(RSC_MULTI6) >& $(NULLDEV)
.ELSE           # "$(rsc_once)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(rsclang_{$(subst,$(RESLIB6NAME)$(RESLIB6VERSION), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB6NAME)$(RESLIB6VERSION), $(@:b))}) \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(langext_{$(subst,$(RESLIB6NAME)$(RESLIB6VERSION), $(@:b))}) \
    -fs{$(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)} \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(PRJ)$/..$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC6HEADER) $(RESLIB6SRSFILES) \
    )
    +$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.new >& $(NULLDEV)
    +-$(RM) $@ >& $(NULLDEV)
    +$(RENAME) $@.new $@
.ENDIF          # "$(rsc_once)"!=""
.ELSE			# "$(common_build_reslib)"!=""
.IF "$(rsc_once)"!=""
    @+-$(RM) $(RSC_MULTI6) >& $(NULLDEV)
.ELSE           # "$(rsc_once)"!=""
    $(RSC) -presponse @$(mktmp \
    -r \
    $(rsclang_{$(subst,$(RESLIB6NAME)$(RESLIB6VERSION), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB6NAME)$(RESLIB6VERSION), $(@:b))}) \
    -I$(RES)$/$(langext_{$(subst,$(RESLIB6NAME)$(RESLIB6VERSION), $(@:b))}) \
    -fs$@ \
    -I$(PRJ)$/$(INPATH)$/res$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(PRJ)$/..$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC6HEADER) $(RESLIB6SRSFILES) \
    )
.ENDIF          # "$(rsc_once)"!=""
.ENDIF			# "$(common_build_reslib)"!=""
.ELSE				# "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
.IF "$(rsc_once)"!=""
    @+-$(RM) $(RSC_MULTI6) >& $(NULLDEV)
.ELSE           # "$(rsc_once)"!=""
    $(RSC) -presponse @$(mktmp \
    $(rsclang_{$(subst,$(RESLIB6NAME)$(RESLIB6VERSION), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB6NAME)$(RESLIB6VERSION), $(@:b))}) \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(langext_{$(subst,$(RESLIB6NAME)$(RESLIB6VERSION), $(@:b))}) \
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
.ENDIF          # "$(rsc_once)"!=""
.ELSE			# "$(common_build_reslib)"!=""
.IF "$(rsc_once)"!=""
    @+-$(RM) $(RSC_MULTI6) >& $(NULLDEV)
.ELSE           # "$(rsc_once)"!=""
    $(RSC) -presponse @$(mktmp \
    $(rsclang_{$(subst,$(RESLIB6NAME)$(RESLIB6VERSION), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB6NAME)$(RESLIB6VERSION), $(@:b))}) \
    -I$(RES)$/$(langext_{$(subst,$(RESLIB6NAME)$(RESLIB6VERSION), $(@:b))}) \
    -fs$@ \
    -r -p \
    -I$(RES) \
    -I$(PRJ)$/$(INPATH)$/res$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INCLUDE) \
    $(RSC6HEADER) $(RESLIB6SRSFILES) \
    )
.ENDIF          # "$(rsc_once)"!=""
.ENDIF			# "$(common_build_reslib)"!=""
.ENDIF              # "$(GUI)"=="UNX"
.ENDIF				# "$(RESLIB6TARGETN)"!=""


# Anweisungen fuer das Linken
# unroll begin

.IF "$(RESLIB7TARGETN)"!=""

$(RSC_MULTI7) : \
        $(RESLIB7SRSFILES) \
        $(RESLIB7TARGETN) \
        $(RESLIB7BMPS)
.IF "$(rsc_once)"!=""
    @+echo using rsc multi-res feature
.IF "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangext) $(rsclang_{$i}) \
    $(rescharset_{$i}) \
    -fs{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB7NAME)$(RESLIB7VERSION)$i.res} \
    -lip{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(langext_{$(i)}) ) \
    -ft$@ \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC7HEADER) $(RESLIB7SRSFILES) \
    ) > $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangext) $(rsclang_{$i}) \
    $(rescharset_{$i}) \
    -fs{$(BIN)$/$(RESLIB7NAME)$(RESLIB7VERSION)$i.res} \
    -lip{$(RES)}$/$(langext_{$(i)}) ) \
    -ft$@ \
    -I{$(RES)}$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC7HEADER) $(RESLIB7SRSFILES) \
    ) > $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""
.ENDIF           # "$(rsc_once)"!=""

$(RESLIB7TARGETN): \
        $(RESLIB7SRSFILES) \
        $(RESLIB7BMPS)
    @echo Making: $@
.IF "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
.IF "$(rsc_once)"!=""
    @+-$(RM) $(RSC_MULTI7) >& $(NULLDEV)
.ELSE           # "$(rsc_once)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(rsclang_{$(subst,$(RESLIB7NAME)$(RESLIB7VERSION), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB7NAME)$(RESLIB7VERSION), $(@:b))}) \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(langext_{$(subst,$(RESLIB7NAME)$(RESLIB7VERSION), $(@:b))}) \
    -fs{$(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)} \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(PRJ)$/..$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC7HEADER) $(RESLIB7SRSFILES) \
    )
    +$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.new >& $(NULLDEV)
    +-$(RM) $@ >& $(NULLDEV)
    +$(RENAME) $@.new $@
.ENDIF          # "$(rsc_once)"!=""
.ELSE			# "$(common_build_reslib)"!=""
.IF "$(rsc_once)"!=""
    @+-$(RM) $(RSC_MULTI7) >& $(NULLDEV)
.ELSE           # "$(rsc_once)"!=""
    $(RSC) -presponse @$(mktmp \
    -r \
    $(rsclang_{$(subst,$(RESLIB7NAME)$(RESLIB7VERSION), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB7NAME)$(RESLIB7VERSION), $(@:b))}) \
    -I$(RES)$/$(langext_{$(subst,$(RESLIB7NAME)$(RESLIB7VERSION), $(@:b))}) \
    -fs$@ \
    -I$(PRJ)$/$(INPATH)$/res$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(PRJ)$/..$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC7HEADER) $(RESLIB7SRSFILES) \
    )
.ENDIF          # "$(rsc_once)"!=""
.ENDIF			# "$(common_build_reslib)"!=""
.ELSE				# "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
.IF "$(rsc_once)"!=""
    @+-$(RM) $(RSC_MULTI7) >& $(NULLDEV)
.ELSE           # "$(rsc_once)"!=""
    $(RSC) -presponse @$(mktmp \
    $(rsclang_{$(subst,$(RESLIB7NAME)$(RESLIB7VERSION), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB7NAME)$(RESLIB7VERSION), $(@:b))}) \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(langext_{$(subst,$(RESLIB7NAME)$(RESLIB7VERSION), $(@:b))}) \
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
.ENDIF          # "$(rsc_once)"!=""
.ELSE			# "$(common_build_reslib)"!=""
.IF "$(rsc_once)"!=""
    @+-$(RM) $(RSC_MULTI7) >& $(NULLDEV)
.ELSE           # "$(rsc_once)"!=""
    $(RSC) -presponse @$(mktmp \
    $(rsclang_{$(subst,$(RESLIB7NAME)$(RESLIB7VERSION), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB7NAME)$(RESLIB7VERSION), $(@:b))}) \
    -I$(RES)$/$(langext_{$(subst,$(RESLIB7NAME)$(RESLIB7VERSION), $(@:b))}) \
    -fs$@ \
    -r -p \
    -I$(RES) \
    -I$(PRJ)$/$(INPATH)$/res$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INCLUDE) \
    $(RSC7HEADER) $(RESLIB7SRSFILES) \
    )
.ENDIF          # "$(rsc_once)"!=""
.ENDIF			# "$(common_build_reslib)"!=""
.ENDIF              # "$(GUI)"=="UNX"
.ENDIF				# "$(RESLIB7TARGETN)"!=""


# Anweisungen fuer das Linken
# unroll begin

.IF "$(RESLIB8TARGETN)"!=""

$(RSC_MULTI8) : \
        $(RESLIB8SRSFILES) \
        $(RESLIB8TARGETN) \
        $(RESLIB8BMPS)
.IF "$(rsc_once)"!=""
    @+echo using rsc multi-res feature
.IF "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangext) $(rsclang_{$i}) \
    $(rescharset_{$i}) \
    -fs{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB8NAME)$(RESLIB8VERSION)$i.res} \
    -lip{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(langext_{$(i)}) ) \
    -ft$@ \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC8HEADER) $(RESLIB8SRSFILES) \
    ) > $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangext) $(rsclang_{$i}) \
    $(rescharset_{$i}) \
    -fs{$(BIN)$/$(RESLIB8NAME)$(RESLIB8VERSION)$i.res} \
    -lip{$(RES)}$/$(langext_{$(i)}) ) \
    -ft$@ \
    -I{$(RES)}$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC8HEADER) $(RESLIB8SRSFILES) \
    ) > $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""
.ENDIF           # "$(rsc_once)"!=""

$(RESLIB8TARGETN): \
        $(RESLIB8SRSFILES) \
        $(RESLIB8BMPS)
    @echo Making: $@
.IF "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
.IF "$(rsc_once)"!=""
    @+-$(RM) $(RSC_MULTI8) >& $(NULLDEV)
.ELSE           # "$(rsc_once)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(rsclang_{$(subst,$(RESLIB8NAME)$(RESLIB8VERSION), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB8NAME)$(RESLIB8VERSION), $(@:b))}) \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(langext_{$(subst,$(RESLIB8NAME)$(RESLIB8VERSION), $(@:b))}) \
    -fs{$(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)} \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(PRJ)$/..$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC8HEADER) $(RESLIB8SRSFILES) \
    )
    +$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.new >& $(NULLDEV)
    +-$(RM) $@ >& $(NULLDEV)
    +$(RENAME) $@.new $@
.ENDIF          # "$(rsc_once)"!=""
.ELSE			# "$(common_build_reslib)"!=""
.IF "$(rsc_once)"!=""
    @+-$(RM) $(RSC_MULTI8) >& $(NULLDEV)
.ELSE           # "$(rsc_once)"!=""
    $(RSC) -presponse @$(mktmp \
    -r \
    $(rsclang_{$(subst,$(RESLIB8NAME)$(RESLIB8VERSION), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB8NAME)$(RESLIB8VERSION), $(@:b))}) \
    -I$(RES)$/$(langext_{$(subst,$(RESLIB8NAME)$(RESLIB8VERSION), $(@:b))}) \
    -fs$@ \
    -I$(PRJ)$/$(INPATH)$/res$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(PRJ)$/..$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC8HEADER) $(RESLIB8SRSFILES) \
    )
.ENDIF          # "$(rsc_once)"!=""
.ENDIF			# "$(common_build_reslib)"!=""
.ELSE				# "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
.IF "$(rsc_once)"!=""
    @+-$(RM) $(RSC_MULTI8) >& $(NULLDEV)
.ELSE           # "$(rsc_once)"!=""
    $(RSC) -presponse @$(mktmp \
    $(rsclang_{$(subst,$(RESLIB8NAME)$(RESLIB8VERSION), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB8NAME)$(RESLIB8VERSION), $(@:b))}) \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(langext_{$(subst,$(RESLIB8NAME)$(RESLIB8VERSION), $(@:b))}) \
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
.ENDIF          # "$(rsc_once)"!=""
.ELSE			# "$(common_build_reslib)"!=""
.IF "$(rsc_once)"!=""
    @+-$(RM) $(RSC_MULTI8) >& $(NULLDEV)
.ELSE           # "$(rsc_once)"!=""
    $(RSC) -presponse @$(mktmp \
    $(rsclang_{$(subst,$(RESLIB8NAME)$(RESLIB8VERSION), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB8NAME)$(RESLIB8VERSION), $(@:b))}) \
    -I$(RES)$/$(langext_{$(subst,$(RESLIB8NAME)$(RESLIB8VERSION), $(@:b))}) \
    -fs$@ \
    -r -p \
    -I$(RES) \
    -I$(PRJ)$/$(INPATH)$/res$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INCLUDE) \
    $(RSC8HEADER) $(RESLIB8SRSFILES) \
    )
.ENDIF          # "$(rsc_once)"!=""
.ENDIF			# "$(common_build_reslib)"!=""
.ENDIF              # "$(GUI)"=="UNX"
.ENDIF				# "$(RESLIB8TARGETN)"!=""


# Anweisungen fuer das Linken
# unroll begin

.IF "$(RESLIB9TARGETN)"!=""

$(RSC_MULTI9) : \
        $(RESLIB9SRSFILES) \
        $(RESLIB9TARGETN) \
        $(RESLIB9BMPS)
.IF "$(rsc_once)"!=""
    @+echo using rsc multi-res feature
.IF "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangext) $(rsclang_{$i}) \
    $(rescharset_{$i}) \
    -fs{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB9NAME)$(RESLIB9VERSION)$i.res} \
    -lip{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(langext_{$(i)}) ) \
    -ft$@ \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC9HEADER) $(RESLIB9SRSFILES) \
    ) > $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangext) $(rsclang_{$i}) \
    $(rescharset_{$i}) \
    -fs{$(BIN)$/$(RESLIB9NAME)$(RESLIB9VERSION)$i.res} \
    -lip{$(RES)}$/$(langext_{$(i)}) ) \
    -ft$@ \
    -I{$(RES)}$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC9HEADER) $(RESLIB9SRSFILES) \
    ) > $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""
.ENDIF           # "$(rsc_once)"!=""

$(RESLIB9TARGETN): \
        $(RESLIB9SRSFILES) \
        $(RESLIB9BMPS)
    @echo Making: $@
.IF "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
.IF "$(rsc_once)"!=""
    @+-$(RM) $(RSC_MULTI9) >& $(NULLDEV)
.ELSE           # "$(rsc_once)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(rsclang_{$(subst,$(RESLIB9NAME)$(RESLIB9VERSION), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB9NAME)$(RESLIB9VERSION), $(@:b))}) \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(langext_{$(subst,$(RESLIB9NAME)$(RESLIB9VERSION), $(@:b))}) \
    -fs{$(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)} \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(PRJ)$/..$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC9HEADER) $(RESLIB9SRSFILES) \
    )
    +$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.new >& $(NULLDEV)
    +-$(RM) $@ >& $(NULLDEV)
    +$(RENAME) $@.new $@
.ENDIF          # "$(rsc_once)"!=""
.ELSE			# "$(common_build_reslib)"!=""
.IF "$(rsc_once)"!=""
    @+-$(RM) $(RSC_MULTI9) >& $(NULLDEV)
.ELSE           # "$(rsc_once)"!=""
    $(RSC) -presponse @$(mktmp \
    -r \
    $(rsclang_{$(subst,$(RESLIB9NAME)$(RESLIB9VERSION), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB9NAME)$(RESLIB9VERSION), $(@:b))}) \
    -I$(RES)$/$(langext_{$(subst,$(RESLIB9NAME)$(RESLIB9VERSION), $(@:b))}) \
    -fs$@ \
    -I$(PRJ)$/$(INPATH)$/res$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(PRJ)$/..$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC9HEADER) $(RESLIB9SRSFILES) \
    )
.ENDIF          # "$(rsc_once)"!=""
.ENDIF			# "$(common_build_reslib)"!=""
.ELSE				# "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
.IF "$(rsc_once)"!=""
    @+-$(RM) $(RSC_MULTI9) >& $(NULLDEV)
.ELSE           # "$(rsc_once)"!=""
    $(RSC) -presponse @$(mktmp \
    $(rsclang_{$(subst,$(RESLIB9NAME)$(RESLIB9VERSION), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB9NAME)$(RESLIB9VERSION), $(@:b))}) \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(langext_{$(subst,$(RESLIB9NAME)$(RESLIB9VERSION), $(@:b))}) \
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
.ENDIF          # "$(rsc_once)"!=""
.ELSE			# "$(common_build_reslib)"!=""
.IF "$(rsc_once)"!=""
    @+-$(RM) $(RSC_MULTI9) >& $(NULLDEV)
.ELSE           # "$(rsc_once)"!=""
    $(RSC) -presponse @$(mktmp \
    $(rsclang_{$(subst,$(RESLIB9NAME)$(RESLIB9VERSION), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB9NAME)$(RESLIB9VERSION), $(@:b))}) \
    -I$(RES)$/$(langext_{$(subst,$(RESLIB9NAME)$(RESLIB9VERSION), $(@:b))}) \
    -fs$@ \
    -r -p \
    -I$(RES) \
    -I$(PRJ)$/$(INPATH)$/res$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INCLUDE) \
    $(RSC9HEADER) $(RESLIB9SRSFILES) \
    )
.ENDIF          # "$(rsc_once)"!=""
.ENDIF			# "$(common_build_reslib)"!=""
.ENDIF              # "$(GUI)"=="UNX"
.ENDIF				# "$(RESLIB9TARGETN)"!=""


# Anweisungen fuer das Linken
# unroll begin

.IF "$(RESLIB10TARGETN)"!=""

$(RSC_MULTI10) : \
        $(RESLIB10SRSFILES) \
        $(RESLIB10TARGETN) \
        $(RESLIB10BMPS)
.IF "$(rsc_once)"!=""
    @+echo using rsc multi-res feature
.IF "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangext) $(rsclang_{$i}) \
    $(rescharset_{$i}) \
    -fs{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB10NAME)$(RESLIB10VERSION)$i.res} \
    -lip{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(langext_{$(i)}) ) \
    -ft$@ \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC10HEADER) $(RESLIB10SRSFILES) \
    ) > $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangext) $(rsclang_{$i}) \
    $(rescharset_{$i}) \
    -fs{$(BIN)$/$(RESLIB10NAME)$(RESLIB10VERSION)$i.res} \
    -lip{$(RES)}$/$(langext_{$(i)}) ) \
    -ft$@ \
    -I{$(RES)}$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC10HEADER) $(RESLIB10SRSFILES) \
    ) > $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""
.ENDIF           # "$(rsc_once)"!=""

$(RESLIB10TARGETN): \
        $(RESLIB10SRSFILES) \
        $(RESLIB10BMPS)
    @echo Making: $@
.IF "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
.IF "$(rsc_once)"!=""
    @+-$(RM) $(RSC_MULTI10) >& $(NULLDEV)
.ELSE           # "$(rsc_once)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(rsclang_{$(subst,$(RESLIB10NAME)$(RESLIB10VERSION), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB10NAME)$(RESLIB10VERSION), $(@:b))}) \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(langext_{$(subst,$(RESLIB10NAME)$(RESLIB10VERSION), $(@:b))}) \
    -fs{$(subst,$(COMMON_OUTDIR),$(OUTPATH) $@)} \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(PRJ)$/..$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC10HEADER) $(RESLIB10SRSFILES) \
    )
    +$(GNUCOPY) -p $(subst,$(COMMON_OUTDIR),$(OUTPATH) $@) $@.new >& $(NULLDEV)
    +-$(RM) $@ >& $(NULLDEV)
    +$(RENAME) $@.new $@
.ENDIF          # "$(rsc_once)"!=""
.ELSE			# "$(common_build_reslib)"!=""
.IF "$(rsc_once)"!=""
    @+-$(RM) $(RSC_MULTI10) >& $(NULLDEV)
.ELSE           # "$(rsc_once)"!=""
    $(RSC) -presponse @$(mktmp \
    -r \
    $(rsclang_{$(subst,$(RESLIB10NAME)$(RESLIB10VERSION), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB10NAME)$(RESLIB10VERSION), $(@:b))}) \
    -I$(RES)$/$(langext_{$(subst,$(RESLIB10NAME)$(RESLIB10VERSION), $(@:b))}) \
    -fs$@ \
    -I$(PRJ)$/$(INPATH)$/res$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(PRJ)$/win$/res -I$(PRJ)$/..$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC10HEADER) $(RESLIB10SRSFILES) \
    )
.ENDIF          # "$(rsc_once)"!=""
.ENDIF			# "$(common_build_reslib)"!=""
.ELSE				# "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
.IF "$(rsc_once)"!=""
    @+-$(RM) $(RSC_MULTI10) >& $(NULLDEV)
.ELSE           # "$(rsc_once)"!=""
    $(RSC) -presponse @$(mktmp \
    $(rsclang_{$(subst,$(RESLIB10NAME)$(RESLIB10VERSION), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB10NAME)$(RESLIB10VERSION), $(@:b))}) \
    -I{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))}$/$(langext_{$(subst,$(RESLIB10NAME)$(RESLIB10VERSION), $(@:b))}) \
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
.ENDIF          # "$(rsc_once)"!=""
.ELSE			# "$(common_build_reslib)"!=""
.IF "$(rsc_once)"!=""
    @+-$(RM) $(RSC_MULTI10) >& $(NULLDEV)
.ELSE           # "$(rsc_once)"!=""
    $(RSC) -presponse @$(mktmp \
    $(rsclang_{$(subst,$(RESLIB10NAME)$(RESLIB10VERSION), $(@:b))}) \
    $(rescharset_{$(subst,$(RESLIB10NAME)$(RESLIB10VERSION), $(@:b))}) \
    -I$(RES)$/$(langext_{$(subst,$(RESLIB10NAME)$(RESLIB10VERSION), $(@:b))}) \
    -fs$@ \
    -r -p \
    -I$(RES) \
    -I$(PRJ)$/$(INPATH)$/res$/$(defaultlangext) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INCLUDE) \
    $(RSC10HEADER) $(RESLIB10SRSFILES) \
    )
.ENDIF          # "$(rsc_once)"!=""
.ENDIF			# "$(common_build_reslib)"!=""
.ENDIF              # "$(GUI)"=="UNX"
.ENDIF				# "$(RESLIB10TARGETN)"!=""


# Anweisungen fuer das Linken
