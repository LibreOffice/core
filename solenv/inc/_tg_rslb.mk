# unroll begin

.IF "$(RESLIB1TARGETN)"!=""

$(RSC_MULTI1) : \
        $(RESLIB1SRSFILES) \
        $(RESLIB1TARGETN) \
        $(RESLIB1BMPS)
    @echo Compiling: $(@:f)
.IF "$(common_build_reslib)"!=""
    $(COMMAND_ECHO)$(RSC) -presponse $(VERBOSITY) @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangiso) -lg$i \
    $(null,$(rescharset_{$i}) $(default$(LANG_GUI)) $(rescharset_{$i})) \
    -fs={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))/$(RESLIB1NAME)$i.res} \
    $(foreach,j,$(subst,$(PRJ),$(SOLARDEFIMG)/$(PRJNAME) $(RESLIB1IMAGES)) -lip={$j}/$i \
    -lip={$j} ) \
    -lip=$(SOLARDEFIMG)/res/$i -lip=$(SOLARDEFIMG)/res ) \
    -subMODULE=$(SOLARDEFIMG) \
    -subGLOBALRES=$(SOLARDEFIMG)/res \
    -oil={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))} \
    -ft=$@ \
    $(RSC1HEADER) $(RESLIB1SRSFILES) \
    ) > $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    $(COMMAND_ECHO)$(RSC) -presponse $(VERBOSITY) @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangiso) -lg$i \
    $(null,$(rescharset_{$i}) $(default$(LANG_GUI)) $(rescharset_{$i})) \
    -fs={$(BIN)/$(RESLIB1NAME)$i.res} \
    $(foreach,j,$(subst,$(PRJ),$(SOLARDEFIMG)/$(PRJNAME) $(RESLIB1IMAGES)) -lip={$j}/$i \
    -lip={$j} ) \
    -lip=$(SOLARDEFIMG)/res/$i -lip=$(SOLARDEFIMG)/res ) \
    -subMODULE=$(SOLARDEFIMG) \
    -subGLOBALRES=$(SOLARDEFIMG)/res \
    -oil=$(BIN) \
    -ft=$@ \
    $(RSC1HEADER) $(RESLIB1SRSFILES) \
    ) > $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""

$(RESLIB1TARGETN): \
        $(RESLIB1SRSFILES) \
        $(RESLIB1BMPS)
    @echo "Making:   " $(@:f)
.IF "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
    @@-$(RM) $(RSC_MULTI1)
.ELSE			# "$(common_build_reslib)"!=""
    @@-$(RM) $(RSC_MULTI1)
.ENDIF			# "$(common_build_reslib)"!=""
.ELSE				# "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
    @@-$(RM) $(RSC_MULTI1)
.ELSE			# "$(common_build_reslib)"!=""
    @@-$(RM) $(RSC_MULTI1)
.ENDIF			# "$(common_build_reslib)"!=""
.ENDIF              # "$(GUI)"=="UNX"
.ENDIF				# "$(RESLIB1TARGETN)"!=""


# unroll begin

.IF "$(RESLIB2TARGETN)"!=""

$(RSC_MULTI2) : \
        $(RESLIB2SRSFILES) \
        $(RESLIB2TARGETN) \
        $(RESLIB2BMPS)
    @echo Compiling: $(@:f)
.IF "$(common_build_reslib)"!=""
    $(COMMAND_ECHO)$(RSC) -presponse $(VERBOSITY) @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangiso) -lg$i \
    $(null,$(rescharset_{$i}) $(default$(LANG_GUI)) $(rescharset_{$i})) \
    -fs={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))/$(RESLIB2NAME)$i.res} \
    $(foreach,j,$(subst,$(PRJ),$(SOLARDEFIMG)/$(PRJNAME) $(RESLIB2IMAGES)) -lip={$j}/$i \
    -lip={$j} ) \
    -lip=$(SOLARDEFIMG)/res/$i -lip=$(SOLARDEFIMG)/res ) \
    -subMODULE=$(SOLARDEFIMG) \
    -subGLOBALRES=$(SOLARDEFIMG)/res \
    -oil={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))} \
    -ft=$@ \
    $(RSC2HEADER) $(RESLIB2SRSFILES) \
    ) > $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    $(COMMAND_ECHO)$(RSC) -presponse $(VERBOSITY) @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangiso) -lg$i \
    $(null,$(rescharset_{$i}) $(default$(LANG_GUI)) $(rescharset_{$i})) \
    -fs={$(BIN)/$(RESLIB2NAME)$i.res} \
    $(foreach,j,$(subst,$(PRJ),$(SOLARDEFIMG)/$(PRJNAME) $(RESLIB2IMAGES)) -lip={$j}/$i \
    -lip={$j} ) \
    -lip=$(SOLARDEFIMG)/res/$i -lip=$(SOLARDEFIMG)/res ) \
    -subMODULE=$(SOLARDEFIMG) \
    -subGLOBALRES=$(SOLARDEFIMG)/res \
    -oil=$(BIN) \
    -ft=$@ \
    $(RSC2HEADER) $(RESLIB2SRSFILES) \
    ) > $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""

$(RESLIB2TARGETN): \
        $(RESLIB2SRSFILES) \
        $(RESLIB2BMPS)
    @echo "Making:   " $(@:f)
.IF "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
    @@-$(RM) $(RSC_MULTI2)
.ELSE			# "$(common_build_reslib)"!=""
    @@-$(RM) $(RSC_MULTI2)
.ENDIF			# "$(common_build_reslib)"!=""
.ELSE				# "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
    @@-$(RM) $(RSC_MULTI2)
.ELSE			# "$(common_build_reslib)"!=""
    @@-$(RM) $(RSC_MULTI2)
.ENDIF			# "$(common_build_reslib)"!=""
.ENDIF              # "$(GUI)"=="UNX"
.ENDIF				# "$(RESLIB2TARGETN)"!=""


# unroll begin

.IF "$(RESLIB3TARGETN)"!=""

$(RSC_MULTI3) : \
        $(RESLIB3SRSFILES) \
        $(RESLIB3TARGETN) \
        $(RESLIB3BMPS)
    @echo Compiling: $(@:f)
.IF "$(common_build_reslib)"!=""
    $(COMMAND_ECHO)$(RSC) -presponse $(VERBOSITY) @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangiso) -lg$i \
    $(null,$(rescharset_{$i}) $(default$(LANG_GUI)) $(rescharset_{$i})) \
    -fs={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))/$(RESLIB3NAME)$i.res} \
    $(foreach,j,$(subst,$(PRJ),$(SOLARDEFIMG)/$(PRJNAME) $(RESLIB3IMAGES)) -lip={$j}/$i \
    -lip={$j} ) \
    -lip=$(SOLARDEFIMG)/res/$i -lip=$(SOLARDEFIMG)/res ) \
    -subMODULE=$(SOLARDEFIMG) \
    -subGLOBALRES=$(SOLARDEFIMG)/res \
    -oil={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))} \
    -ft=$@ \
    $(RSC3HEADER) $(RESLIB3SRSFILES) \
    ) > $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    $(COMMAND_ECHO)$(RSC) -presponse $(VERBOSITY) @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangiso) -lg$i \
    $(null,$(rescharset_{$i}) $(default$(LANG_GUI)) $(rescharset_{$i})) \
    -fs={$(BIN)/$(RESLIB3NAME)$i.res} \
    $(foreach,j,$(subst,$(PRJ),$(SOLARDEFIMG)/$(PRJNAME) $(RESLIB3IMAGES)) -lip={$j}/$i \
    -lip={$j} ) \
    -lip=$(SOLARDEFIMG)/res/$i -lip=$(SOLARDEFIMG)/res ) \
    -subMODULE=$(SOLARDEFIMG) \
    -subGLOBALRES=$(SOLARDEFIMG)/res \
    -oil=$(BIN) \
    -ft=$@ \
    $(RSC3HEADER) $(RESLIB3SRSFILES) \
    ) > $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""

$(RESLIB3TARGETN): \
        $(RESLIB3SRSFILES) \
        $(RESLIB3BMPS)
    @echo "Making:   " $(@:f)
.IF "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
    @@-$(RM) $(RSC_MULTI3)
.ELSE			# "$(common_build_reslib)"!=""
    @@-$(RM) $(RSC_MULTI3)
.ENDIF			# "$(common_build_reslib)"!=""
.ELSE				# "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
    @@-$(RM) $(RSC_MULTI3)
.ELSE			# "$(common_build_reslib)"!=""
    @@-$(RM) $(RSC_MULTI3)
.ENDIF			# "$(common_build_reslib)"!=""
.ENDIF              # "$(GUI)"=="UNX"
.ENDIF				# "$(RESLIB3TARGETN)"!=""


# unroll begin

.IF "$(RESLIB4TARGETN)"!=""

$(RSC_MULTI4) : \
        $(RESLIB4SRSFILES) \
        $(RESLIB4TARGETN) \
        $(RESLIB4BMPS)
    @echo Compiling: $(@:f)
.IF "$(common_build_reslib)"!=""
    $(COMMAND_ECHO)$(RSC) -presponse $(VERBOSITY) @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangiso) -lg$i \
    $(null,$(rescharset_{$i}) $(default$(LANG_GUI)) $(rescharset_{$i})) \
    -fs={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))/$(RESLIB4NAME)$i.res} \
    $(foreach,j,$(subst,$(PRJ),$(SOLARDEFIMG)/$(PRJNAME) $(RESLIB4IMAGES)) -lip={$j}/$i \
    -lip={$j} ) \
    -lip=$(SOLARDEFIMG)/res/$i -lip=$(SOLARDEFIMG)/res ) \
    -subMODULE=$(SOLARDEFIMG) \
    -subGLOBALRES=$(SOLARDEFIMG)/res \
    -oil={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))} \
    -ft=$@ \
    $(RSC4HEADER) $(RESLIB4SRSFILES) \
    ) > $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    $(COMMAND_ECHO)$(RSC) -presponse $(VERBOSITY) @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangiso) -lg$i \
    $(null,$(rescharset_{$i}) $(default$(LANG_GUI)) $(rescharset_{$i})) \
    -fs={$(BIN)/$(RESLIB4NAME)$i.res} \
    $(foreach,j,$(subst,$(PRJ),$(SOLARDEFIMG)/$(PRJNAME) $(RESLIB4IMAGES)) -lip={$j}/$i \
    -lip={$j} ) \
    -lip=$(SOLARDEFIMG)/res/$i -lip=$(SOLARDEFIMG)/res ) \
    -subMODULE=$(SOLARDEFIMG) \
    -subGLOBALRES=$(SOLARDEFIMG)/res \
    -oil=$(BIN) \
    -ft=$@ \
    $(RSC4HEADER) $(RESLIB4SRSFILES) \
    ) > $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""

$(RESLIB4TARGETN): \
        $(RESLIB4SRSFILES) \
        $(RESLIB4BMPS)
    @echo "Making:   " $(@:f)
.IF "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
    @@-$(RM) $(RSC_MULTI4)
.ELSE			# "$(common_build_reslib)"!=""
    @@-$(RM) $(RSC_MULTI4)
.ENDIF			# "$(common_build_reslib)"!=""
.ELSE				# "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
    @@-$(RM) $(RSC_MULTI4)
.ELSE			# "$(common_build_reslib)"!=""
    @@-$(RM) $(RSC_MULTI4)
.ENDIF			# "$(common_build_reslib)"!=""
.ENDIF              # "$(GUI)"=="UNX"
.ENDIF				# "$(RESLIB4TARGETN)"!=""


# unroll begin

.IF "$(RESLIB5TARGETN)"!=""

$(RSC_MULTI5) : \
        $(RESLIB5SRSFILES) \
        $(RESLIB5TARGETN) \
        $(RESLIB5BMPS)
    @echo Compiling: $(@:f)
.IF "$(common_build_reslib)"!=""
    $(COMMAND_ECHO)$(RSC) -presponse $(VERBOSITY) @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangiso) -lg$i \
    $(null,$(rescharset_{$i}) $(default$(LANG_GUI)) $(rescharset_{$i})) \
    -fs={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))/$(RESLIB5NAME)$i.res} \
    $(foreach,j,$(subst,$(PRJ),$(SOLARDEFIMG)/$(PRJNAME) $(RESLIB5IMAGES)) -lip={$j}/$i \
    -lip={$j} ) \
    -lip=$(SOLARDEFIMG)/res/$i -lip=$(SOLARDEFIMG)/res ) \
    -subMODULE=$(SOLARDEFIMG) \
    -subGLOBALRES=$(SOLARDEFIMG)/res \
    -oil={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))} \
    -ft=$@ \
    $(RSC5HEADER) $(RESLIB5SRSFILES) \
    ) > $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    $(COMMAND_ECHO)$(RSC) -presponse $(VERBOSITY) @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangiso) -lg$i \
    $(null,$(rescharset_{$i}) $(default$(LANG_GUI)) $(rescharset_{$i})) \
    -fs={$(BIN)/$(RESLIB5NAME)$i.res} \
    $(foreach,j,$(subst,$(PRJ),$(SOLARDEFIMG)/$(PRJNAME) $(RESLIB5IMAGES)) -lip={$j}/$i \
    -lip={$j} ) \
    -lip=$(SOLARDEFIMG)/res/$i -lip=$(SOLARDEFIMG)/res ) \
    -subMODULE=$(SOLARDEFIMG) \
    -subGLOBALRES=$(SOLARDEFIMG)/res \
    -oil=$(BIN) \
    -ft=$@ \
    $(RSC5HEADER) $(RESLIB5SRSFILES) \
    ) > $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""

$(RESLIB5TARGETN): \
        $(RESLIB5SRSFILES) \
        $(RESLIB5BMPS)
    @echo "Making:   " $(@:f)
.IF "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
    @@-$(RM) $(RSC_MULTI5)
.ELSE			# "$(common_build_reslib)"!=""
    @@-$(RM) $(RSC_MULTI5)
.ENDIF			# "$(common_build_reslib)"!=""
.ELSE				# "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
    @@-$(RM) $(RSC_MULTI5)
.ELSE			# "$(common_build_reslib)"!=""
    @@-$(RM) $(RSC_MULTI5)
.ENDIF			# "$(common_build_reslib)"!=""
.ENDIF              # "$(GUI)"=="UNX"
.ENDIF				# "$(RESLIB5TARGETN)"!=""


# unroll begin

.IF "$(RESLIB6TARGETN)"!=""

$(RSC_MULTI6) : \
        $(RESLIB6SRSFILES) \
        $(RESLIB6TARGETN) \
        $(RESLIB6BMPS)
    @echo Compiling: $(@:f)
.IF "$(common_build_reslib)"!=""
    $(COMMAND_ECHO)$(RSC) -presponse $(VERBOSITY) @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangiso) -lg$i \
    $(null,$(rescharset_{$i}) $(default$(LANG_GUI)) $(rescharset_{$i})) \
    -fs={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))/$(RESLIB6NAME)$i.res} \
    $(foreach,j,$(subst,$(PRJ),$(SOLARDEFIMG)/$(PRJNAME) $(RESLIB6IMAGES)) -lip={$j}/$i \
    -lip={$j} ) \
    -lip=$(SOLARDEFIMG)/res/$i -lip=$(SOLARDEFIMG)/res ) \
    -subMODULE=$(SOLARDEFIMG) \
    -subGLOBALRES=$(SOLARDEFIMG)/res \
    -oil={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))} \
    -ft=$@ \
    $(RSC6HEADER) $(RESLIB6SRSFILES) \
    ) > $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    $(COMMAND_ECHO)$(RSC) -presponse $(VERBOSITY) @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangiso) -lg$i \
    $(null,$(rescharset_{$i}) $(default$(LANG_GUI)) $(rescharset_{$i})) \
    -fs={$(BIN)/$(RESLIB6NAME)$i.res} \
    $(foreach,j,$(subst,$(PRJ),$(SOLARDEFIMG)/$(PRJNAME) $(RESLIB6IMAGES)) -lip={$j}/$i \
    -lip={$j} ) \
    -lip=$(SOLARDEFIMG)/res/$i -lip=$(SOLARDEFIMG)/res ) \
    -subMODULE=$(SOLARDEFIMG) \
    -subGLOBALRES=$(SOLARDEFIMG)/res \
    -oil=$(BIN) \
    -ft=$@ \
    $(RSC6HEADER) $(RESLIB6SRSFILES) \
    ) > $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""

$(RESLIB6TARGETN): \
        $(RESLIB6SRSFILES) \
        $(RESLIB6BMPS)
    @echo "Making:   " $(@:f)
.IF "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
    @@-$(RM) $(RSC_MULTI6)
.ELSE			# "$(common_build_reslib)"!=""
    @@-$(RM) $(RSC_MULTI6)
.ENDIF			# "$(common_build_reslib)"!=""
.ELSE				# "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
    @@-$(RM) $(RSC_MULTI6)
.ELSE			# "$(common_build_reslib)"!=""
    @@-$(RM) $(RSC_MULTI6)
.ENDIF			# "$(common_build_reslib)"!=""
.ENDIF              # "$(GUI)"=="UNX"
.ENDIF				# "$(RESLIB6TARGETN)"!=""


# unroll begin

.IF "$(RESLIB7TARGETN)"!=""

$(RSC_MULTI7) : \
        $(RESLIB7SRSFILES) \
        $(RESLIB7TARGETN) \
        $(RESLIB7BMPS)
    @echo Compiling: $(@:f)
.IF "$(common_build_reslib)"!=""
    $(COMMAND_ECHO)$(RSC) -presponse $(VERBOSITY) @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangiso) -lg$i \
    $(null,$(rescharset_{$i}) $(default$(LANG_GUI)) $(rescharset_{$i})) \
    -fs={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))/$(RESLIB7NAME)$i.res} \
    $(foreach,j,$(subst,$(PRJ),$(SOLARDEFIMG)/$(PRJNAME) $(RESLIB7IMAGES)) -lip={$j}/$i \
    -lip={$j} ) \
    -lip=$(SOLARDEFIMG)/res/$i -lip=$(SOLARDEFIMG)/res ) \
    -subMODULE=$(SOLARDEFIMG) \
    -subGLOBALRES=$(SOLARDEFIMG)/res \
    -oil={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))} \
    -ft=$@ \
    $(RSC7HEADER) $(RESLIB7SRSFILES) \
    ) > $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    $(COMMAND_ECHO)$(RSC) -presponse $(VERBOSITY) @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangiso) -lg$i \
    $(null,$(rescharset_{$i}) $(default$(LANG_GUI)) $(rescharset_{$i})) \
    -fs={$(BIN)/$(RESLIB7NAME)$i.res} \
    $(foreach,j,$(subst,$(PRJ),$(SOLARDEFIMG)/$(PRJNAME) $(RESLIB7IMAGES)) -lip={$j}/$i \
    -lip={$j} ) \
    -lip=$(SOLARDEFIMG)/res/$i -lip=$(SOLARDEFIMG)/res ) \
    -subMODULE=$(SOLARDEFIMG) \
    -subGLOBALRES=$(SOLARDEFIMG)/res \
    -oil=$(BIN) \
    -ft=$@ \
    $(RSC7HEADER) $(RESLIB7SRSFILES) \
    ) > $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""

$(RESLIB7TARGETN): \
        $(RESLIB7SRSFILES) \
        $(RESLIB7BMPS)
    @echo "Making:   " $(@:f)
.IF "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
    @@-$(RM) $(RSC_MULTI7)
.ELSE			# "$(common_build_reslib)"!=""
    @@-$(RM) $(RSC_MULTI7)
.ENDIF			# "$(common_build_reslib)"!=""
.ELSE				# "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
    @@-$(RM) $(RSC_MULTI7)
.ELSE			# "$(common_build_reslib)"!=""
    @@-$(RM) $(RSC_MULTI7)
.ENDIF			# "$(common_build_reslib)"!=""
.ENDIF              # "$(GUI)"=="UNX"
.ENDIF				# "$(RESLIB7TARGETN)"!=""


# unroll begin

.IF "$(RESLIB8TARGETN)"!=""

$(RSC_MULTI8) : \
        $(RESLIB8SRSFILES) \
        $(RESLIB8TARGETN) \
        $(RESLIB8BMPS)
    @echo Compiling: $(@:f)
.IF "$(common_build_reslib)"!=""
    $(COMMAND_ECHO)$(RSC) -presponse $(VERBOSITY) @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangiso) -lg$i \
    $(null,$(rescharset_{$i}) $(default$(LANG_GUI)) $(rescharset_{$i})) \
    -fs={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))/$(RESLIB8NAME)$i.res} \
    $(foreach,j,$(subst,$(PRJ),$(SOLARDEFIMG)/$(PRJNAME) $(RESLIB8IMAGES)) -lip={$j}/$i \
    -lip={$j} ) \
    -lip=$(SOLARDEFIMG)/res/$i -lip=$(SOLARDEFIMG)/res ) \
    -subMODULE=$(SOLARDEFIMG) \
    -subGLOBALRES=$(SOLARDEFIMG)/res \
    -oil={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))} \
    -ft=$@ \
    $(RSC8HEADER) $(RESLIB8SRSFILES) \
    ) > $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    $(COMMAND_ECHO)$(RSC) -presponse $(VERBOSITY) @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangiso) -lg$i \
    $(null,$(rescharset_{$i}) $(default$(LANG_GUI)) $(rescharset_{$i})) \
    -fs={$(BIN)/$(RESLIB8NAME)$i.res} \
    $(foreach,j,$(subst,$(PRJ),$(SOLARDEFIMG)/$(PRJNAME) $(RESLIB8IMAGES)) -lip={$j}/$i \
    -lip={$j} ) \
    -lip=$(SOLARDEFIMG)/res/$i -lip=$(SOLARDEFIMG)/res ) \
    -subMODULE=$(SOLARDEFIMG) \
    -subGLOBALRES=$(SOLARDEFIMG)/res \
    -oil=$(BIN) \
    -ft=$@ \
    $(RSC8HEADER) $(RESLIB8SRSFILES) \
    ) > $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""

$(RESLIB8TARGETN): \
        $(RESLIB8SRSFILES) \
        $(RESLIB8BMPS)
    @echo "Making:   " $(@:f)
.IF "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
    @@-$(RM) $(RSC_MULTI8)
.ELSE			# "$(common_build_reslib)"!=""
    @@-$(RM) $(RSC_MULTI8)
.ENDIF			# "$(common_build_reslib)"!=""
.ELSE				# "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
    @@-$(RM) $(RSC_MULTI8)
.ELSE			# "$(common_build_reslib)"!=""
    @@-$(RM) $(RSC_MULTI8)
.ENDIF			# "$(common_build_reslib)"!=""
.ENDIF              # "$(GUI)"=="UNX"
.ENDIF				# "$(RESLIB8TARGETN)"!=""


# unroll begin

.IF "$(RESLIB9TARGETN)"!=""

$(RSC_MULTI9) : \
        $(RESLIB9SRSFILES) \
        $(RESLIB9TARGETN) \
        $(RESLIB9BMPS)
    @echo Compiling: $(@:f)
.IF "$(common_build_reslib)"!=""
    $(COMMAND_ECHO)$(RSC) -presponse $(VERBOSITY) @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangiso) -lg$i \
    $(null,$(rescharset_{$i}) $(default$(LANG_GUI)) $(rescharset_{$i})) \
    -fs={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))/$(RESLIB9NAME)$i.res} \
    $(foreach,j,$(subst,$(PRJ),$(SOLARDEFIMG)/$(PRJNAME) $(RESLIB9IMAGES)) -lip={$j}/$i \
    -lip={$j} ) \
    -lip=$(SOLARDEFIMG)/res/$i -lip=$(SOLARDEFIMG)/res ) \
    -subMODULE=$(SOLARDEFIMG) \
    -subGLOBALRES=$(SOLARDEFIMG)/res \
    -oil={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))} \
    -ft=$@ \
    $(RSC9HEADER) $(RESLIB9SRSFILES) \
    ) > $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    $(COMMAND_ECHO)$(RSC) -presponse $(VERBOSITY) @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangiso) -lg$i \
    $(null,$(rescharset_{$i}) $(default$(LANG_GUI)) $(rescharset_{$i})) \
    -fs={$(BIN)/$(RESLIB9NAME)$i.res} \
    $(foreach,j,$(subst,$(PRJ),$(SOLARDEFIMG)/$(PRJNAME) $(RESLIB9IMAGES)) -lip={$j}/$i \
    -lip={$j} ) \
    -lip=$(SOLARDEFIMG)/res/$i -lip=$(SOLARDEFIMG)/res ) \
    -subMODULE=$(SOLARDEFIMG) \
    -subGLOBALRES=$(SOLARDEFIMG)/res \
    -oil=$(BIN) \
    -ft=$@ \
    $(RSC9HEADER) $(RESLIB9SRSFILES) \
    ) > $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""

$(RESLIB9TARGETN): \
        $(RESLIB9SRSFILES) \
        $(RESLIB9BMPS)
    @echo "Making:   " $(@:f)
.IF "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
    @@-$(RM) $(RSC_MULTI9)
.ELSE			# "$(common_build_reslib)"!=""
    @@-$(RM) $(RSC_MULTI9)
.ENDIF			# "$(common_build_reslib)"!=""
.ELSE				# "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
    @@-$(RM) $(RSC_MULTI9)
.ELSE			# "$(common_build_reslib)"!=""
    @@-$(RM) $(RSC_MULTI9)
.ENDIF			# "$(common_build_reslib)"!=""
.ENDIF              # "$(GUI)"=="UNX"
.ENDIF				# "$(RESLIB9TARGETN)"!=""


# unroll begin

.IF "$(RESLIB10TARGETN)"!=""

$(RSC_MULTI10) : \
        $(RESLIB10SRSFILES) \
        $(RESLIB10TARGETN) \
        $(RESLIB10BMPS)
    @echo Compiling: $(@:f)
.IF "$(common_build_reslib)"!=""
    $(COMMAND_ECHO)$(RSC) -presponse $(VERBOSITY) @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangiso) -lg$i \
    $(null,$(rescharset_{$i}) $(default$(LANG_GUI)) $(rescharset_{$i})) \
    -fs={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))/$(RESLIB10NAME)$i.res} \
    $(foreach,j,$(subst,$(PRJ),$(SOLARDEFIMG)/$(PRJNAME) $(RESLIB10IMAGES)) -lip={$j}/$i \
    -lip={$j} ) \
    -lip=$(SOLARDEFIMG)/res/$i -lip=$(SOLARDEFIMG)/res ) \
    -subMODULE=$(SOLARDEFIMG) \
    -subGLOBALRES=$(SOLARDEFIMG)/res \
    -oil={$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))} \
    -ft=$@ \
    $(RSC10HEADER) $(RESLIB10SRSFILES) \
    ) > $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    $(COMMAND_ECHO)$(RSC) -presponse $(VERBOSITY) @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangiso) -lg$i \
    $(null,$(rescharset_{$i}) $(default$(LANG_GUI)) $(rescharset_{$i})) \
    -fs={$(BIN)/$(RESLIB10NAME)$i.res} \
    $(foreach,j,$(subst,$(PRJ),$(SOLARDEFIMG)/$(PRJNAME) $(RESLIB10IMAGES)) -lip={$j}/$i \
    -lip={$j} ) \
    -lip=$(SOLARDEFIMG)/res/$i -lip=$(SOLARDEFIMG)/res ) \
    -subMODULE=$(SOLARDEFIMG) \
    -subGLOBALRES=$(SOLARDEFIMG)/res \
    -oil=$(BIN) \
    -ft=$@ \
    $(RSC10HEADER) $(RESLIB10SRSFILES) \
    ) > $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""

$(RESLIB10TARGETN): \
        $(RESLIB10SRSFILES) \
        $(RESLIB10BMPS)
    @echo "Making:   " $(@:f)
.IF "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
    @@-$(RM) $(RSC_MULTI10)
.ELSE			# "$(common_build_reslib)"!=""
    @@-$(RM) $(RSC_MULTI10)
.ENDIF			# "$(common_build_reslib)"!=""
.ELSE				# "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
    @@-$(RM) $(RSC_MULTI10)
.ELSE			# "$(common_build_reslib)"!=""
    @@-$(RM) $(RSC_MULTI10)
.ENDIF			# "$(common_build_reslib)"!=""
.ENDIF              # "$(GUI)"=="UNX"
.ENDIF				# "$(RESLIB10TARGETN)"!=""


