# unroll begin

.IF "$(RESLIB1TARGETN)"!=""

.IF "$(BUILDHIDS)"!=""
HIDRES1PARTICLE=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(RESLIB1NAME)_res.hid

#HACK cut off the dirty srs files which are included from solver
#RESLIB1HIDFILESx=$(shell @+echo $(RESLIB1SRSFILES:t"\n") | $(GREP) -v "$(SOLARRESDIR:s/\/\\/)" )
RESLIB1HIDFILES:=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(subst,.srs,_srs.hid $(RESLIB1SRSFILES)))
$(HIDRES1PARTICLE): $(RESLIB1HIDFILES)
    @echo ------------------------------
    @echo Making: $@
    @+if exist $@ rm $@
    $(TYPE) $(RESLIB1HIDFILES) > $@.$(ROUT).tmp 
    @+$(RENAME) $@.$(ROUT).tmp $@

ALLTAR : $(HIDRES1PARTICLE)

.ENDIF # "$(BUILDHIDS)"!=""

.IF "$(common_build_reslib)"!=""
$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB1NAME)$(RESLIB1VERSION).ilst2 : $(RSC_MULTI1)
.IF "$(use_shell)"!="4nt"	
    $(TYPE) $(mktmp $(foreach,i,$(alllangext) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB1NAME)$(RESLIB1VERSION)$i.ilst)) | xargs sed s\#%MODULE%\#%MODULE%$/$(PRJNAME)\# > $@
.ELSE			# "$(use_shell)"!="4nt"	
    +$(TYPE) $(foreach,i,$(alllangext) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB1NAME)$(RESLIB1VERSION)$i.ilst) | sed `s/MODULE%/MODULE%\/$(PRJNAME)/` > $@
.ENDIF			# "$(use_shell)"!="4nt"	

ALLTAR : $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB1NAME)$(RESLIB1VERSION).ilst2
.ELSE           # "$(common_build_reslib)"!=""
$(BIN)$/$(RESLIB1NAME)$(RESLIB1VERSION).ilst2 : $(RSC_MULTI1)
.IF "$(use_shell)"!="4nt"	
    $(TYPE) $(mktmp $(foreach,i,$(alllangext) $(BIN)$/$(RESLIB1NAME)$(RESLIB1VERSION)$i.ilst)) | xargs sed s\#%MODULE%\#%MODULE%$/$(PRJNAME)\# > $@
.ELSE			# "$(use_shell)"!="4nt"	
    +$(TYPE) $(foreach,i,$(alllangext) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB1NAME)$(RESLIB1VERSION)$i.ilst) | sed `s/MODULE%/MODULE%\/$(PRJNAME)/` > $@
.ENDIF			# "$(use_shell)"!="4nt"	

ALLTAR : $(BIN)$/$(RESLIB1NAME)$(RESLIB1VERSION).ilst2
.ENDIF          # "$(common_build_reslib)"!=""

$(RSC_MULTI1) : \
        $(RESLIB1SRSFILES) \
        $(RESLIB1TARGETN) \
        $(RESLIB1BMPS)
    @+echo using rsc multi-res feature
.IF "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangext) $(rsclang_{$i}) \
    $(rescharset_{$i}) \
    -fs{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB1NAME)$(RESLIB1VERSION)$i.res} \
    $(foreach,j,$(RESLIB1IMAGES) -lip{$j}$/$(lang_{$i}) \
    -lip{$j} ) \
    -lip$(SOLARSRC)$/res$/$(lang_{$i}) -lip$(SOLARSRC)$/res ) \
    -subMODULE=$(PRJ) \
    -subGLOBAL=$(SOLARSRC) \
    -subCUSTOM=to_be_defined \
    -oil{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))} \
    -ft$@ \
    -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC1HEADER) $(RESLIB1SRSFILES) \
    ) > $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangext) $(rsclang_{$i}) \
    $(rescharset_{$i}) \
    -fs{$(BIN)$/$(RESLIB1NAME)$(RESLIB1VERSION)$i.res} \
    $(foreach,j,$(RESLIB1IMAGES) -lip{$j}$/$(lang_{$i}) \
    -lip{$j} ) \
    -lip$(SOLARSRC)$/res$/$(lang_{$i}) -lip$(SOLARSRC)$/res ) \
    -subGLOBAL=$(SOLARSRC) \
    -subMODULE=$(PRJ) \
    -subCUSTOM=to_be_defined \
    -oil$(BIN) \
    -ft$@ \
    -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC1HEADER) $(RESLIB1SRSFILES) \
    ) > $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""

$(RESLIB1TARGETN): \
        $(RESLIB1SRSFILES) \
        $(RESLIB1BMPS)
    @echo Making: $@
.IF "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
    @+-$(RM) $(RSC_MULTI1) >& $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    @+-$(RM) $(RSC_MULTI1) >& $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""
.ELSE				# "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
    @+-$(RM) $(RSC_MULTI1) >& $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    @+-$(RM) $(RSC_MULTI1) >& $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""
.ENDIF              # "$(GUI)"=="UNX"
.ENDIF				# "$(RESLIB1TARGETN)"!=""


# Anweisungen fuer das Linken
# unroll begin

.IF "$(RESLIB2TARGETN)"!=""

.IF "$(BUILDHIDS)"!=""
HIDRES2PARTICLE=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(RESLIB2NAME)_res.hid

#HACK cut off the dirty srs files which are included from solver
#RESLIB2HIDFILESx=$(shell @+echo $(RESLIB2SRSFILES:t"\n") | $(GREP) -v "$(SOLARRESDIR:s/\/\\/)" )
RESLIB2HIDFILES:=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(subst,.srs,_srs.hid $(RESLIB2SRSFILES)))
$(HIDRES2PARTICLE): $(RESLIB2HIDFILES)
    @echo ------------------------------
    @echo Making: $@
    @+if exist $@ rm $@
    $(TYPE) $(RESLIB2HIDFILES) > $@.$(ROUT).tmp 
    @+$(RENAME) $@.$(ROUT).tmp $@

ALLTAR : $(HIDRES2PARTICLE)

.ENDIF # "$(BUILDHIDS)"!=""

.IF "$(common_build_reslib)"!=""
$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB2NAME)$(RESLIB2VERSION).ilst2 : $(RSC_MULTI2)
.IF "$(use_shell)"!="4nt"	
    $(TYPE) $(mktmp $(foreach,i,$(alllangext) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB2NAME)$(RESLIB2VERSION)$i.ilst)) | xargs sed s\#%MODULE%\#%MODULE%$/$(PRJNAME)\# > $@
.ELSE			# "$(use_shell)"!="4nt"	
    +$(TYPE) $(foreach,i,$(alllangext) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB2NAME)$(RESLIB2VERSION)$i.ilst) | sed `s/MODULE%/MODULE%\/$(PRJNAME)/` > $@
.ENDIF			# "$(use_shell)"!="4nt"	

ALLTAR : $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB2NAME)$(RESLIB2VERSION).ilst2
.ELSE           # "$(common_build_reslib)"!=""
$(BIN)$/$(RESLIB2NAME)$(RESLIB2VERSION).ilst2 : $(RSC_MULTI2)
.IF "$(use_shell)"!="4nt"	
    $(TYPE) $(mktmp $(foreach,i,$(alllangext) $(BIN)$/$(RESLIB2NAME)$(RESLIB2VERSION)$i.ilst)) | xargs sed s\#%MODULE%\#%MODULE%$/$(PRJNAME)\# > $@
.ELSE			# "$(use_shell)"!="4nt"	
    +$(TYPE) $(foreach,i,$(alllangext) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB2NAME)$(RESLIB2VERSION)$i.ilst) | sed `s/MODULE%/MODULE%\/$(PRJNAME)/` > $@
.ENDIF			# "$(use_shell)"!="4nt"	

ALLTAR : $(BIN)$/$(RESLIB2NAME)$(RESLIB2VERSION).ilst2
.ENDIF          # "$(common_build_reslib)"!=""

$(RSC_MULTI2) : \
        $(RESLIB2SRSFILES) \
        $(RESLIB2TARGETN) \
        $(RESLIB2BMPS)
    @+echo using rsc multi-res feature
.IF "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangext) $(rsclang_{$i}) \
    $(rescharset_{$i}) \
    -fs{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB2NAME)$(RESLIB2VERSION)$i.res} \
    $(foreach,j,$(RESLIB1IMAGES) -lip{$j}$/$(lang_{$i}) \
    -lip{$j} ) \
    -lip$(SOLARSRC)$/res$/$(lang_{$i}) -lip$(SOLARSRC)$/res ) \
    -subMODULE=$(PRJ) \
    -subGLOBAL=$(SOLARSRC) \
    -subCUSTOM=to_be_defined \
    -oil{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))} \
    -ft$@ \
    -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC2HEADER) $(RESLIB2SRSFILES) \
    ) > $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangext) $(rsclang_{$i}) \
    $(rescharset_{$i}) \
    -fs{$(BIN)$/$(RESLIB2NAME)$(RESLIB2VERSION)$i.res} \
    $(foreach,j,$(RESLIB1IMAGES) -lip{$j}$/$(lang_{$i}) \
    -lip{$j} ) \
    -lip$(SOLARSRC)$/res$/$(lang_{$i}) -lip$(SOLARSRC)$/res ) \
    -subGLOBAL=$(SOLARSRC) \
    -subMODULE=$(PRJ) \
    -subCUSTOM=to_be_defined \
    -oil$(BIN) \
    -ft$@ \
    -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC2HEADER) $(RESLIB2SRSFILES) \
    ) > $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""

$(RESLIB2TARGETN): \
        $(RESLIB2SRSFILES) \
        $(RESLIB2BMPS)
    @echo Making: $@
.IF "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
    @+-$(RM) $(RSC_MULTI2) >& $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    @+-$(RM) $(RSC_MULTI2) >& $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""
.ELSE				# "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
    @+-$(RM) $(RSC_MULTI2) >& $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    @+-$(RM) $(RSC_MULTI2) >& $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""
.ENDIF              # "$(GUI)"=="UNX"
.ENDIF				# "$(RESLIB2TARGETN)"!=""


# Anweisungen fuer das Linken
# unroll begin

.IF "$(RESLIB3TARGETN)"!=""

.IF "$(BUILDHIDS)"!=""
HIDRES3PARTICLE=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(RESLIB3NAME)_res.hid

#HACK cut off the dirty srs files which are included from solver
#RESLIB3HIDFILESx=$(shell @+echo $(RESLIB3SRSFILES:t"\n") | $(GREP) -v "$(SOLARRESDIR:s/\/\\/)" )
RESLIB3HIDFILES:=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(subst,.srs,_srs.hid $(RESLIB3SRSFILES)))
$(HIDRES3PARTICLE): $(RESLIB3HIDFILES)
    @echo ------------------------------
    @echo Making: $@
    @+if exist $@ rm $@
    $(TYPE) $(RESLIB3HIDFILES) > $@.$(ROUT).tmp 
    @+$(RENAME) $@.$(ROUT).tmp $@

ALLTAR : $(HIDRES3PARTICLE)

.ENDIF # "$(BUILDHIDS)"!=""

.IF "$(common_build_reslib)"!=""
$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB3NAME)$(RESLIB3VERSION).ilst2 : $(RSC_MULTI3)
.IF "$(use_shell)"!="4nt"	
    $(TYPE) $(mktmp $(foreach,i,$(alllangext) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB3NAME)$(RESLIB3VERSION)$i.ilst)) | xargs sed s\#%MODULE%\#%MODULE%$/$(PRJNAME)\# > $@
.ELSE			# "$(use_shell)"!="4nt"	
    +$(TYPE) $(foreach,i,$(alllangext) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB3NAME)$(RESLIB3VERSION)$i.ilst) | sed `s/MODULE%/MODULE%\/$(PRJNAME)/` > $@
.ENDIF			# "$(use_shell)"!="4nt"	

ALLTAR : $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB3NAME)$(RESLIB3VERSION).ilst2
.ELSE           # "$(common_build_reslib)"!=""
$(BIN)$/$(RESLIB3NAME)$(RESLIB3VERSION).ilst2 : $(RSC_MULTI3)
.IF "$(use_shell)"!="4nt"	
    $(TYPE) $(mktmp $(foreach,i,$(alllangext) $(BIN)$/$(RESLIB3NAME)$(RESLIB3VERSION)$i.ilst)) | xargs sed s\#%MODULE%\#%MODULE%$/$(PRJNAME)\# > $@
.ELSE			# "$(use_shell)"!="4nt"	
    +$(TYPE) $(foreach,i,$(alllangext) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB3NAME)$(RESLIB3VERSION)$i.ilst) | sed `s/MODULE%/MODULE%\/$(PRJNAME)/` > $@
.ENDIF			# "$(use_shell)"!="4nt"	

ALLTAR : $(BIN)$/$(RESLIB3NAME)$(RESLIB3VERSION).ilst2
.ENDIF          # "$(common_build_reslib)"!=""

$(RSC_MULTI3) : \
        $(RESLIB3SRSFILES) \
        $(RESLIB3TARGETN) \
        $(RESLIB3BMPS)
    @+echo using rsc multi-res feature
.IF "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangext) $(rsclang_{$i}) \
    $(rescharset_{$i}) \
    -fs{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB3NAME)$(RESLIB3VERSION)$i.res} \
    $(foreach,j,$(RESLIB1IMAGES) -lip{$j}$/$(lang_{$i}) \
    -lip{$j} ) \
    -lip$(SOLARSRC)$/res$/$(lang_{$i}) -lip$(SOLARSRC)$/res ) \
    -subMODULE=$(PRJ) \
    -subGLOBAL=$(SOLARSRC) \
    -subCUSTOM=to_be_defined \
    -oil{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))} \
    -ft$@ \
    -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC3HEADER) $(RESLIB3SRSFILES) \
    ) > $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangext) $(rsclang_{$i}) \
    $(rescharset_{$i}) \
    -fs{$(BIN)$/$(RESLIB3NAME)$(RESLIB3VERSION)$i.res} \
    $(foreach,j,$(RESLIB1IMAGES) -lip{$j}$/$(lang_{$i}) \
    -lip{$j} ) \
    -lip$(SOLARSRC)$/res$/$(lang_{$i}) -lip$(SOLARSRC)$/res ) \
    -subGLOBAL=$(SOLARSRC) \
    -subMODULE=$(PRJ) \
    -subCUSTOM=to_be_defined \
    -oil$(BIN) \
    -ft$@ \
    -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC3HEADER) $(RESLIB3SRSFILES) \
    ) > $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""

$(RESLIB3TARGETN): \
        $(RESLIB3SRSFILES) \
        $(RESLIB3BMPS)
    @echo Making: $@
.IF "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
    @+-$(RM) $(RSC_MULTI3) >& $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    @+-$(RM) $(RSC_MULTI3) >& $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""
.ELSE				# "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
    @+-$(RM) $(RSC_MULTI3) >& $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    @+-$(RM) $(RSC_MULTI3) >& $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""
.ENDIF              # "$(GUI)"=="UNX"
.ENDIF				# "$(RESLIB3TARGETN)"!=""


# Anweisungen fuer das Linken
# unroll begin

.IF "$(RESLIB4TARGETN)"!=""

.IF "$(BUILDHIDS)"!=""
HIDRES4PARTICLE=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(RESLIB4NAME)_res.hid

#HACK cut off the dirty srs files which are included from solver
#RESLIB4HIDFILESx=$(shell @+echo $(RESLIB4SRSFILES:t"\n") | $(GREP) -v "$(SOLARRESDIR:s/\/\\/)" )
RESLIB4HIDFILES:=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(subst,.srs,_srs.hid $(RESLIB4SRSFILES)))
$(HIDRES4PARTICLE): $(RESLIB4HIDFILES)
    @echo ------------------------------
    @echo Making: $@
    @+if exist $@ rm $@
    $(TYPE) $(RESLIB4HIDFILES) > $@.$(ROUT).tmp 
    @+$(RENAME) $@.$(ROUT).tmp $@

ALLTAR : $(HIDRES4PARTICLE)

.ENDIF # "$(BUILDHIDS)"!=""

.IF "$(common_build_reslib)"!=""
$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB4NAME)$(RESLIB4VERSION).ilst2 : $(RSC_MULTI4)
.IF "$(use_shell)"!="4nt"	
    $(TYPE) $(mktmp $(foreach,i,$(alllangext) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB4NAME)$(RESLIB4VERSION)$i.ilst)) | xargs sed s\#%MODULE%\#%MODULE%$/$(PRJNAME)\# > $@
.ELSE			# "$(use_shell)"!="4nt"	
    +$(TYPE) $(foreach,i,$(alllangext) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB4NAME)$(RESLIB4VERSION)$i.ilst) | sed `s/MODULE%/MODULE%\/$(PRJNAME)/` > $@
.ENDIF			# "$(use_shell)"!="4nt"	

ALLTAR : $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB4NAME)$(RESLIB4VERSION).ilst2
.ELSE           # "$(common_build_reslib)"!=""
$(BIN)$/$(RESLIB4NAME)$(RESLIB4VERSION).ilst2 : $(RSC_MULTI4)
.IF "$(use_shell)"!="4nt"	
    $(TYPE) $(mktmp $(foreach,i,$(alllangext) $(BIN)$/$(RESLIB4NAME)$(RESLIB4VERSION)$i.ilst)) | xargs sed s\#%MODULE%\#%MODULE%$/$(PRJNAME)\# > $@
.ELSE			# "$(use_shell)"!="4nt"	
    +$(TYPE) $(foreach,i,$(alllangext) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB4NAME)$(RESLIB4VERSION)$i.ilst) | sed `s/MODULE%/MODULE%\/$(PRJNAME)/` > $@
.ENDIF			# "$(use_shell)"!="4nt"	

ALLTAR : $(BIN)$/$(RESLIB4NAME)$(RESLIB4VERSION).ilst2
.ENDIF          # "$(common_build_reslib)"!=""

$(RSC_MULTI4) : \
        $(RESLIB4SRSFILES) \
        $(RESLIB4TARGETN) \
        $(RESLIB4BMPS)
    @+echo using rsc multi-res feature
.IF "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangext) $(rsclang_{$i}) \
    $(rescharset_{$i}) \
    -fs{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB4NAME)$(RESLIB4VERSION)$i.res} \
    $(foreach,j,$(RESLIB1IMAGES) -lip{$j}$/$(lang_{$i}) \
    -lip{$j} ) \
    -lip$(SOLARSRC)$/res$/$(lang_{$i}) -lip$(SOLARSRC)$/res ) \
    -subMODULE=$(PRJ) \
    -subGLOBAL=$(SOLARSRC) \
    -subCUSTOM=to_be_defined \
    -oil{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))} \
    -ft$@ \
    -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC4HEADER) $(RESLIB4SRSFILES) \
    ) > $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangext) $(rsclang_{$i}) \
    $(rescharset_{$i}) \
    -fs{$(BIN)$/$(RESLIB4NAME)$(RESLIB4VERSION)$i.res} \
    $(foreach,j,$(RESLIB1IMAGES) -lip{$j}$/$(lang_{$i}) \
    -lip{$j} ) \
    -lip$(SOLARSRC)$/res$/$(lang_{$i}) -lip$(SOLARSRC)$/res ) \
    -subGLOBAL=$(SOLARSRC) \
    -subMODULE=$(PRJ) \
    -subCUSTOM=to_be_defined \
    -oil$(BIN) \
    -ft$@ \
    -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC4HEADER) $(RESLIB4SRSFILES) \
    ) > $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""

$(RESLIB4TARGETN): \
        $(RESLIB4SRSFILES) \
        $(RESLIB4BMPS)
    @echo Making: $@
.IF "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
    @+-$(RM) $(RSC_MULTI4) >& $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    @+-$(RM) $(RSC_MULTI4) >& $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""
.ELSE				# "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
    @+-$(RM) $(RSC_MULTI4) >& $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    @+-$(RM) $(RSC_MULTI4) >& $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""
.ENDIF              # "$(GUI)"=="UNX"
.ENDIF				# "$(RESLIB4TARGETN)"!=""


# Anweisungen fuer das Linken
# unroll begin

.IF "$(RESLIB5TARGETN)"!=""

.IF "$(BUILDHIDS)"!=""
HIDRES5PARTICLE=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(RESLIB5NAME)_res.hid

#HACK cut off the dirty srs files which are included from solver
#RESLIB5HIDFILESx=$(shell @+echo $(RESLIB5SRSFILES:t"\n") | $(GREP) -v "$(SOLARRESDIR:s/\/\\/)" )
RESLIB5HIDFILES:=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(subst,.srs,_srs.hid $(RESLIB5SRSFILES)))
$(HIDRES5PARTICLE): $(RESLIB5HIDFILES)
    @echo ------------------------------
    @echo Making: $@
    @+if exist $@ rm $@
    $(TYPE) $(RESLIB5HIDFILES) > $@.$(ROUT).tmp 
    @+$(RENAME) $@.$(ROUT).tmp $@

ALLTAR : $(HIDRES5PARTICLE)

.ENDIF # "$(BUILDHIDS)"!=""

.IF "$(common_build_reslib)"!=""
$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB5NAME)$(RESLIB5VERSION).ilst2 : $(RSC_MULTI5)
.IF "$(use_shell)"!="4nt"	
    $(TYPE) $(mktmp $(foreach,i,$(alllangext) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB5NAME)$(RESLIB5VERSION)$i.ilst)) | xargs sed s\#%MODULE%\#%MODULE%$/$(PRJNAME)\# > $@
.ELSE			# "$(use_shell)"!="4nt"	
    +$(TYPE) $(foreach,i,$(alllangext) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB5NAME)$(RESLIB5VERSION)$i.ilst) | sed `s/MODULE%/MODULE%\/$(PRJNAME)/` > $@
.ENDIF			# "$(use_shell)"!="4nt"	

ALLTAR : $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB5NAME)$(RESLIB5VERSION).ilst2
.ELSE           # "$(common_build_reslib)"!=""
$(BIN)$/$(RESLIB5NAME)$(RESLIB5VERSION).ilst2 : $(RSC_MULTI5)
.IF "$(use_shell)"!="4nt"	
    $(TYPE) $(mktmp $(foreach,i,$(alllangext) $(BIN)$/$(RESLIB5NAME)$(RESLIB5VERSION)$i.ilst)) | xargs sed s\#%MODULE%\#%MODULE%$/$(PRJNAME)\# > $@
.ELSE			# "$(use_shell)"!="4nt"	
    +$(TYPE) $(foreach,i,$(alllangext) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB5NAME)$(RESLIB5VERSION)$i.ilst) | sed `s/MODULE%/MODULE%\/$(PRJNAME)/` > $@
.ENDIF			# "$(use_shell)"!="4nt"	

ALLTAR : $(BIN)$/$(RESLIB5NAME)$(RESLIB5VERSION).ilst2
.ENDIF          # "$(common_build_reslib)"!=""

$(RSC_MULTI5) : \
        $(RESLIB5SRSFILES) \
        $(RESLIB5TARGETN) \
        $(RESLIB5BMPS)
    @+echo using rsc multi-res feature
.IF "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangext) $(rsclang_{$i}) \
    $(rescharset_{$i}) \
    -fs{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB5NAME)$(RESLIB5VERSION)$i.res} \
    $(foreach,j,$(RESLIB1IMAGES) -lip{$j}$/$(lang_{$i}) \
    -lip{$j} ) \
    -lip$(SOLARSRC)$/res$/$(lang_{$i}) -lip$(SOLARSRC)$/res ) \
    -subMODULE=$(PRJ) \
    -subGLOBAL=$(SOLARSRC) \
    -subCUSTOM=to_be_defined \
    -oil{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))} \
    -ft$@ \
    -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC5HEADER) $(RESLIB5SRSFILES) \
    ) > $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangext) $(rsclang_{$i}) \
    $(rescharset_{$i}) \
    -fs{$(BIN)$/$(RESLIB5NAME)$(RESLIB5VERSION)$i.res} \
    $(foreach,j,$(RESLIB1IMAGES) -lip{$j}$/$(lang_{$i}) \
    -lip{$j} ) \
    -lip$(SOLARSRC)$/res$/$(lang_{$i}) -lip$(SOLARSRC)$/res ) \
    -subGLOBAL=$(SOLARSRC) \
    -subMODULE=$(PRJ) \
    -subCUSTOM=to_be_defined \
    -oil$(BIN) \
    -ft$@ \
    -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC5HEADER) $(RESLIB5SRSFILES) \
    ) > $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""

$(RESLIB5TARGETN): \
        $(RESLIB5SRSFILES) \
        $(RESLIB5BMPS)
    @echo Making: $@
.IF "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
    @+-$(RM) $(RSC_MULTI5) >& $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    @+-$(RM) $(RSC_MULTI5) >& $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""
.ELSE				# "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
    @+-$(RM) $(RSC_MULTI5) >& $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    @+-$(RM) $(RSC_MULTI5) >& $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""
.ENDIF              # "$(GUI)"=="UNX"
.ENDIF				# "$(RESLIB5TARGETN)"!=""


# Anweisungen fuer das Linken
# unroll begin

.IF "$(RESLIB6TARGETN)"!=""

.IF "$(BUILDHIDS)"!=""
HIDRES6PARTICLE=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(RESLIB6NAME)_res.hid

#HACK cut off the dirty srs files which are included from solver
#RESLIB6HIDFILESx=$(shell @+echo $(RESLIB6SRSFILES:t"\n") | $(GREP) -v "$(SOLARRESDIR:s/\/\\/)" )
RESLIB6HIDFILES:=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(subst,.srs,_srs.hid $(RESLIB6SRSFILES)))
$(HIDRES6PARTICLE): $(RESLIB6HIDFILES)
    @echo ------------------------------
    @echo Making: $@
    @+if exist $@ rm $@
    $(TYPE) $(RESLIB6HIDFILES) > $@.$(ROUT).tmp 
    @+$(RENAME) $@.$(ROUT).tmp $@

ALLTAR : $(HIDRES6PARTICLE)

.ENDIF # "$(BUILDHIDS)"!=""

.IF "$(common_build_reslib)"!=""
$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB6NAME)$(RESLIB6VERSION).ilst2 : $(RSC_MULTI6)
.IF "$(use_shell)"!="4nt"	
    $(TYPE) $(mktmp $(foreach,i,$(alllangext) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB6NAME)$(RESLIB6VERSION)$i.ilst)) | xargs sed s\#%MODULE%\#%MODULE%$/$(PRJNAME)\# > $@
.ELSE			# "$(use_shell)"!="4nt"	
    +$(TYPE) $(foreach,i,$(alllangext) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB6NAME)$(RESLIB6VERSION)$i.ilst) | sed `s/MODULE%/MODULE%\/$(PRJNAME)/` > $@
.ENDIF			# "$(use_shell)"!="4nt"	

ALLTAR : $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB6NAME)$(RESLIB6VERSION).ilst2
.ELSE           # "$(common_build_reslib)"!=""
$(BIN)$/$(RESLIB6NAME)$(RESLIB6VERSION).ilst2 : $(RSC_MULTI6)
.IF "$(use_shell)"!="4nt"	
    $(TYPE) $(mktmp $(foreach,i,$(alllangext) $(BIN)$/$(RESLIB6NAME)$(RESLIB6VERSION)$i.ilst)) | xargs sed s\#%MODULE%\#%MODULE%$/$(PRJNAME)\# > $@
.ELSE			# "$(use_shell)"!="4nt"	
    +$(TYPE) $(foreach,i,$(alllangext) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB6NAME)$(RESLIB6VERSION)$i.ilst) | sed `s/MODULE%/MODULE%\/$(PRJNAME)/` > $@
.ENDIF			# "$(use_shell)"!="4nt"	

ALLTAR : $(BIN)$/$(RESLIB6NAME)$(RESLIB6VERSION).ilst2
.ENDIF          # "$(common_build_reslib)"!=""

$(RSC_MULTI6) : \
        $(RESLIB6SRSFILES) \
        $(RESLIB6TARGETN) \
        $(RESLIB6BMPS)
    @+echo using rsc multi-res feature
.IF "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangext) $(rsclang_{$i}) \
    $(rescharset_{$i}) \
    -fs{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB6NAME)$(RESLIB6VERSION)$i.res} \
    $(foreach,j,$(RESLIB1IMAGES) -lip{$j}$/$(lang_{$i}) \
    -lip{$j} ) \
    -lip$(SOLARSRC)$/res$/$(lang_{$i}) -lip$(SOLARSRC)$/res ) \
    -subMODULE=$(PRJ) \
    -subGLOBAL=$(SOLARSRC) \
    -subCUSTOM=to_be_defined \
    -oil{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))} \
    -ft$@ \
    -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC6HEADER) $(RESLIB6SRSFILES) \
    ) > $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangext) $(rsclang_{$i}) \
    $(rescharset_{$i}) \
    -fs{$(BIN)$/$(RESLIB6NAME)$(RESLIB6VERSION)$i.res} \
    $(foreach,j,$(RESLIB1IMAGES) -lip{$j}$/$(lang_{$i}) \
    -lip{$j} ) \
    -lip$(SOLARSRC)$/res$/$(lang_{$i}) -lip$(SOLARSRC)$/res ) \
    -subGLOBAL=$(SOLARSRC) \
    -subMODULE=$(PRJ) \
    -subCUSTOM=to_be_defined \
    -oil$(BIN) \
    -ft$@ \
    -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC6HEADER) $(RESLIB6SRSFILES) \
    ) > $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""

$(RESLIB6TARGETN): \
        $(RESLIB6SRSFILES) \
        $(RESLIB6BMPS)
    @echo Making: $@
.IF "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
    @+-$(RM) $(RSC_MULTI6) >& $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    @+-$(RM) $(RSC_MULTI6) >& $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""
.ELSE				# "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
    @+-$(RM) $(RSC_MULTI6) >& $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    @+-$(RM) $(RSC_MULTI6) >& $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""
.ENDIF              # "$(GUI)"=="UNX"
.ENDIF				# "$(RESLIB6TARGETN)"!=""


# Anweisungen fuer das Linken
# unroll begin

.IF "$(RESLIB7TARGETN)"!=""

.IF "$(BUILDHIDS)"!=""
HIDRES7PARTICLE=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(RESLIB7NAME)_res.hid

#HACK cut off the dirty srs files which are included from solver
#RESLIB7HIDFILESx=$(shell @+echo $(RESLIB7SRSFILES:t"\n") | $(GREP) -v "$(SOLARRESDIR:s/\/\\/)" )
RESLIB7HIDFILES:=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(subst,.srs,_srs.hid $(RESLIB7SRSFILES)))
$(HIDRES7PARTICLE): $(RESLIB7HIDFILES)
    @echo ------------------------------
    @echo Making: $@
    @+if exist $@ rm $@
    $(TYPE) $(RESLIB7HIDFILES) > $@.$(ROUT).tmp 
    @+$(RENAME) $@.$(ROUT).tmp $@

ALLTAR : $(HIDRES7PARTICLE)

.ENDIF # "$(BUILDHIDS)"!=""

.IF "$(common_build_reslib)"!=""
$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB7NAME)$(RESLIB7VERSION).ilst2 : $(RSC_MULTI7)
.IF "$(use_shell)"!="4nt"	
    $(TYPE) $(mktmp $(foreach,i,$(alllangext) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB7NAME)$(RESLIB7VERSION)$i.ilst)) | xargs sed s\#%MODULE%\#%MODULE%$/$(PRJNAME)\# > $@
.ELSE			# "$(use_shell)"!="4nt"	
    +$(TYPE) $(foreach,i,$(alllangext) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB7NAME)$(RESLIB7VERSION)$i.ilst) | sed `s/MODULE%/MODULE%\/$(PRJNAME)/` > $@
.ENDIF			# "$(use_shell)"!="4nt"	

ALLTAR : $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB7NAME)$(RESLIB7VERSION).ilst2
.ELSE           # "$(common_build_reslib)"!=""
$(BIN)$/$(RESLIB7NAME)$(RESLIB7VERSION).ilst2 : $(RSC_MULTI7)
.IF "$(use_shell)"!="4nt"	
    $(TYPE) $(mktmp $(foreach,i,$(alllangext) $(BIN)$/$(RESLIB7NAME)$(RESLIB7VERSION)$i.ilst)) | xargs sed s\#%MODULE%\#%MODULE%$/$(PRJNAME)\# > $@
.ELSE			# "$(use_shell)"!="4nt"	
    +$(TYPE) $(foreach,i,$(alllangext) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB7NAME)$(RESLIB7VERSION)$i.ilst) | sed `s/MODULE%/MODULE%\/$(PRJNAME)/` > $@
.ENDIF			# "$(use_shell)"!="4nt"	

ALLTAR : $(BIN)$/$(RESLIB7NAME)$(RESLIB7VERSION).ilst2
.ENDIF          # "$(common_build_reslib)"!=""

$(RSC_MULTI7) : \
        $(RESLIB7SRSFILES) \
        $(RESLIB7TARGETN) \
        $(RESLIB7BMPS)
    @+echo using rsc multi-res feature
.IF "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangext) $(rsclang_{$i}) \
    $(rescharset_{$i}) \
    -fs{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB7NAME)$(RESLIB7VERSION)$i.res} \
    $(foreach,j,$(RESLIB1IMAGES) -lip{$j}$/$(lang_{$i}) \
    -lip{$j} ) \
    -lip$(SOLARSRC)$/res$/$(lang_{$i}) -lip$(SOLARSRC)$/res ) \
    -subMODULE=$(PRJ) \
    -subGLOBAL=$(SOLARSRC) \
    -subCUSTOM=to_be_defined \
    -oil{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))} \
    -ft$@ \
    -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC7HEADER) $(RESLIB7SRSFILES) \
    ) > $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangext) $(rsclang_{$i}) \
    $(rescharset_{$i}) \
    -fs{$(BIN)$/$(RESLIB7NAME)$(RESLIB7VERSION)$i.res} \
    $(foreach,j,$(RESLIB1IMAGES) -lip{$j}$/$(lang_{$i}) \
    -lip{$j} ) \
    -lip$(SOLARSRC)$/res$/$(lang_{$i}) -lip$(SOLARSRC)$/res ) \
    -subGLOBAL=$(SOLARSRC) \
    -subMODULE=$(PRJ) \
    -subCUSTOM=to_be_defined \
    -oil$(BIN) \
    -ft$@ \
    -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC7HEADER) $(RESLIB7SRSFILES) \
    ) > $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""

$(RESLIB7TARGETN): \
        $(RESLIB7SRSFILES) \
        $(RESLIB7BMPS)
    @echo Making: $@
.IF "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
    @+-$(RM) $(RSC_MULTI7) >& $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    @+-$(RM) $(RSC_MULTI7) >& $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""
.ELSE				# "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
    @+-$(RM) $(RSC_MULTI7) >& $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    @+-$(RM) $(RSC_MULTI7) >& $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""
.ENDIF              # "$(GUI)"=="UNX"
.ENDIF				# "$(RESLIB7TARGETN)"!=""


# Anweisungen fuer das Linken
# unroll begin

.IF "$(RESLIB8TARGETN)"!=""

.IF "$(BUILDHIDS)"!=""
HIDRES8PARTICLE=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(RESLIB8NAME)_res.hid

#HACK cut off the dirty srs files which are included from solver
#RESLIB8HIDFILESx=$(shell @+echo $(RESLIB8SRSFILES:t"\n") | $(GREP) -v "$(SOLARRESDIR:s/\/\\/)" )
RESLIB8HIDFILES:=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(subst,.srs,_srs.hid $(RESLIB8SRSFILES)))
$(HIDRES8PARTICLE): $(RESLIB8HIDFILES)
    @echo ------------------------------
    @echo Making: $@
    @+if exist $@ rm $@
    $(TYPE) $(RESLIB8HIDFILES) > $@.$(ROUT).tmp 
    @+$(RENAME) $@.$(ROUT).tmp $@

ALLTAR : $(HIDRES8PARTICLE)

.ENDIF # "$(BUILDHIDS)"!=""

.IF "$(common_build_reslib)"!=""
$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB8NAME)$(RESLIB8VERSION).ilst2 : $(RSC_MULTI8)
.IF "$(use_shell)"!="4nt"	
    $(TYPE) $(mktmp $(foreach,i,$(alllangext) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB8NAME)$(RESLIB8VERSION)$i.ilst)) | xargs sed s\#%MODULE%\#%MODULE%$/$(PRJNAME)\# > $@
.ELSE			# "$(use_shell)"!="4nt"	
    +$(TYPE) $(foreach,i,$(alllangext) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB8NAME)$(RESLIB8VERSION)$i.ilst) | sed `s/MODULE%/MODULE%\/$(PRJNAME)/` > $@
.ENDIF			# "$(use_shell)"!="4nt"	

ALLTAR : $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB8NAME)$(RESLIB8VERSION).ilst2
.ELSE           # "$(common_build_reslib)"!=""
$(BIN)$/$(RESLIB8NAME)$(RESLIB8VERSION).ilst2 : $(RSC_MULTI8)
.IF "$(use_shell)"!="4nt"	
    $(TYPE) $(mktmp $(foreach,i,$(alllangext) $(BIN)$/$(RESLIB8NAME)$(RESLIB8VERSION)$i.ilst)) | xargs sed s\#%MODULE%\#%MODULE%$/$(PRJNAME)\# > $@
.ELSE			# "$(use_shell)"!="4nt"	
    +$(TYPE) $(foreach,i,$(alllangext) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB8NAME)$(RESLIB8VERSION)$i.ilst) | sed `s/MODULE%/MODULE%\/$(PRJNAME)/` > $@
.ENDIF			# "$(use_shell)"!="4nt"	

ALLTAR : $(BIN)$/$(RESLIB8NAME)$(RESLIB8VERSION).ilst2
.ENDIF          # "$(common_build_reslib)"!=""

$(RSC_MULTI8) : \
        $(RESLIB8SRSFILES) \
        $(RESLIB8TARGETN) \
        $(RESLIB8BMPS)
    @+echo using rsc multi-res feature
.IF "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangext) $(rsclang_{$i}) \
    $(rescharset_{$i}) \
    -fs{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB8NAME)$(RESLIB8VERSION)$i.res} \
    $(foreach,j,$(RESLIB1IMAGES) -lip{$j}$/$(lang_{$i}) \
    -lip{$j} ) \
    -lip$(SOLARSRC)$/res$/$(lang_{$i}) -lip$(SOLARSRC)$/res ) \
    -subMODULE=$(PRJ) \
    -subGLOBAL=$(SOLARSRC) \
    -subCUSTOM=to_be_defined \
    -oil{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))} \
    -ft$@ \
    -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC8HEADER) $(RESLIB8SRSFILES) \
    ) > $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangext) $(rsclang_{$i}) \
    $(rescharset_{$i}) \
    -fs{$(BIN)$/$(RESLIB8NAME)$(RESLIB8VERSION)$i.res} \
    $(foreach,j,$(RESLIB1IMAGES) -lip{$j}$/$(lang_{$i}) \
    -lip{$j} ) \
    -lip$(SOLARSRC)$/res$/$(lang_{$i}) -lip$(SOLARSRC)$/res ) \
    -subGLOBAL=$(SOLARSRC) \
    -subMODULE=$(PRJ) \
    -subCUSTOM=to_be_defined \
    -oil$(BIN) \
    -ft$@ \
    -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC8HEADER) $(RESLIB8SRSFILES) \
    ) > $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""

$(RESLIB8TARGETN): \
        $(RESLIB8SRSFILES) \
        $(RESLIB8BMPS)
    @echo Making: $@
.IF "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
    @+-$(RM) $(RSC_MULTI8) >& $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    @+-$(RM) $(RSC_MULTI8) >& $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""
.ELSE				# "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
    @+-$(RM) $(RSC_MULTI8) >& $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    @+-$(RM) $(RSC_MULTI8) >& $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""
.ENDIF              # "$(GUI)"=="UNX"
.ENDIF				# "$(RESLIB8TARGETN)"!=""


# Anweisungen fuer das Linken
# unroll begin

.IF "$(RESLIB9TARGETN)"!=""

.IF "$(BUILDHIDS)"!=""
HIDRES9PARTICLE=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(RESLIB9NAME)_res.hid

#HACK cut off the dirty srs files which are included from solver
#RESLIB9HIDFILESx=$(shell @+echo $(RESLIB9SRSFILES:t"\n") | $(GREP) -v "$(SOLARRESDIR:s/\/\\/)" )
RESLIB9HIDFILES:=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(subst,.srs,_srs.hid $(RESLIB9SRSFILES)))
$(HIDRES9PARTICLE): $(RESLIB9HIDFILES)
    @echo ------------------------------
    @echo Making: $@
    @+if exist $@ rm $@
    $(TYPE) $(RESLIB9HIDFILES) > $@.$(ROUT).tmp 
    @+$(RENAME) $@.$(ROUT).tmp $@

ALLTAR : $(HIDRES9PARTICLE)

.ENDIF # "$(BUILDHIDS)"!=""

.IF "$(common_build_reslib)"!=""
$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB9NAME)$(RESLIB9VERSION).ilst2 : $(RSC_MULTI9)
.IF "$(use_shell)"!="4nt"	
    $(TYPE) $(mktmp $(foreach,i,$(alllangext) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB9NAME)$(RESLIB9VERSION)$i.ilst)) | xargs sed s\#%MODULE%\#%MODULE%$/$(PRJNAME)\# > $@
.ELSE			# "$(use_shell)"!="4nt"	
    +$(TYPE) $(foreach,i,$(alllangext) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB9NAME)$(RESLIB9VERSION)$i.ilst) | sed `s/MODULE%/MODULE%\/$(PRJNAME)/` > $@
.ENDIF			# "$(use_shell)"!="4nt"	

ALLTAR : $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB9NAME)$(RESLIB9VERSION).ilst2
.ELSE           # "$(common_build_reslib)"!=""
$(BIN)$/$(RESLIB9NAME)$(RESLIB9VERSION).ilst2 : $(RSC_MULTI9)
.IF "$(use_shell)"!="4nt"	
    $(TYPE) $(mktmp $(foreach,i,$(alllangext) $(BIN)$/$(RESLIB9NAME)$(RESLIB9VERSION)$i.ilst)) | xargs sed s\#%MODULE%\#%MODULE%$/$(PRJNAME)\# > $@
.ELSE			# "$(use_shell)"!="4nt"	
    +$(TYPE) $(foreach,i,$(alllangext) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB9NAME)$(RESLIB9VERSION)$i.ilst) | sed `s/MODULE%/MODULE%\/$(PRJNAME)/` > $@
.ENDIF			# "$(use_shell)"!="4nt"	

ALLTAR : $(BIN)$/$(RESLIB9NAME)$(RESLIB9VERSION).ilst2
.ENDIF          # "$(common_build_reslib)"!=""

$(RSC_MULTI9) : \
        $(RESLIB9SRSFILES) \
        $(RESLIB9TARGETN) \
        $(RESLIB9BMPS)
    @+echo using rsc multi-res feature
.IF "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangext) $(rsclang_{$i}) \
    $(rescharset_{$i}) \
    -fs{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB9NAME)$(RESLIB9VERSION)$i.res} \
    $(foreach,j,$(RESLIB1IMAGES) -lip{$j}$/$(lang_{$i}) \
    -lip{$j} ) \
    -lip$(SOLARSRC)$/res$/$(lang_{$i}) -lip$(SOLARSRC)$/res ) \
    -subMODULE=$(PRJ) \
    -subGLOBAL=$(SOLARSRC) \
    -subCUSTOM=to_be_defined \
    -oil{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))} \
    -ft$@ \
    -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC9HEADER) $(RESLIB9SRSFILES) \
    ) > $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangext) $(rsclang_{$i}) \
    $(rescharset_{$i}) \
    -fs{$(BIN)$/$(RESLIB9NAME)$(RESLIB9VERSION)$i.res} \
    $(foreach,j,$(RESLIB1IMAGES) -lip{$j}$/$(lang_{$i}) \
    -lip{$j} ) \
    -lip$(SOLARSRC)$/res$/$(lang_{$i}) -lip$(SOLARSRC)$/res ) \
    -subGLOBAL=$(SOLARSRC) \
    -subMODULE=$(PRJ) \
    -subCUSTOM=to_be_defined \
    -oil$(BIN) \
    -ft$@ \
    -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC9HEADER) $(RESLIB9SRSFILES) \
    ) > $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""

$(RESLIB9TARGETN): \
        $(RESLIB9SRSFILES) \
        $(RESLIB9BMPS)
    @echo Making: $@
.IF "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
    @+-$(RM) $(RSC_MULTI9) >& $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    @+-$(RM) $(RSC_MULTI9) >& $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""
.ELSE				# "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
    @+-$(RM) $(RSC_MULTI9) >& $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    @+-$(RM) $(RSC_MULTI9) >& $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""
.ENDIF              # "$(GUI)"=="UNX"
.ENDIF				# "$(RESLIB9TARGETN)"!=""


# Anweisungen fuer das Linken
# unroll begin

.IF "$(RESLIB10TARGETN)"!=""

.IF "$(BUILDHIDS)"!=""
HIDRES10PARTICLE=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(RESLIB10NAME)_res.hid

#HACK cut off the dirty srs files which are included from solver
#RESLIB10HIDFILESx=$(shell @+echo $(RESLIB10SRSFILES:t"\n") | $(GREP) -v "$(SOLARRESDIR:s/\/\\/)" )
RESLIB10HIDFILES:=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(subst,.srs,_srs.hid $(RESLIB10SRSFILES)))
$(HIDRES10PARTICLE): $(RESLIB10HIDFILES)
    @echo ------------------------------
    @echo Making: $@
    @+if exist $@ rm $@
    $(TYPE) $(RESLIB10HIDFILES) > $@.$(ROUT).tmp 
    @+$(RENAME) $@.$(ROUT).tmp $@

ALLTAR : $(HIDRES10PARTICLE)

.ENDIF # "$(BUILDHIDS)"!=""

.IF "$(common_build_reslib)"!=""
$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB10NAME)$(RESLIB10VERSION).ilst2 : $(RSC_MULTI10)
.IF "$(use_shell)"!="4nt"	
    $(TYPE) $(mktmp $(foreach,i,$(alllangext) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB10NAME)$(RESLIB10VERSION)$i.ilst)) | xargs sed s\#%MODULE%\#%MODULE%$/$(PRJNAME)\# > $@
.ELSE			# "$(use_shell)"!="4nt"	
    +$(TYPE) $(foreach,i,$(alllangext) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB10NAME)$(RESLIB10VERSION)$i.ilst) | sed `s/MODULE%/MODULE%\/$(PRJNAME)/` > $@
.ENDIF			# "$(use_shell)"!="4nt"	

ALLTAR : $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB10NAME)$(RESLIB10VERSION).ilst2
.ELSE           # "$(common_build_reslib)"!=""
$(BIN)$/$(RESLIB10NAME)$(RESLIB10VERSION).ilst2 : $(RSC_MULTI10)
.IF "$(use_shell)"!="4nt"	
    $(TYPE) $(mktmp $(foreach,i,$(alllangext) $(BIN)$/$(RESLIB10NAME)$(RESLIB10VERSION)$i.ilst)) | xargs sed s\#%MODULE%\#%MODULE%$/$(PRJNAME)\# > $@
.ELSE			# "$(use_shell)"!="4nt"	
    +$(TYPE) $(foreach,i,$(alllangext) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB10NAME)$(RESLIB10VERSION)$i.ilst) | sed `s/MODULE%/MODULE%\/$(PRJNAME)/` > $@
.ENDIF			# "$(use_shell)"!="4nt"	

ALLTAR : $(BIN)$/$(RESLIB10NAME)$(RESLIB10VERSION).ilst2
.ENDIF          # "$(common_build_reslib)"!=""

$(RSC_MULTI10) : \
        $(RESLIB10SRSFILES) \
        $(RESLIB10TARGETN) \
        $(RESLIB10BMPS)
    @+echo using rsc multi-res feature
.IF "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangext) $(rsclang_{$i}) \
    $(rescharset_{$i}) \
    -fs{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))$/$(RESLIB10NAME)$(RESLIB10VERSION)$i.res} \
    $(foreach,j,$(RESLIB1IMAGES) -lip{$j}$/$(lang_{$i}) \
    -lip{$j} ) \
    -lip$(SOLARSRC)$/res$/$(lang_{$i}) -lip$(SOLARSRC)$/res ) \
    -subMODULE=$(PRJ) \
    -subGLOBAL=$(SOLARSRC) \
    -subCUSTOM=to_be_defined \
    -oil{$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(BIN))} \
    -ft$@ \
    -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC10HEADER) $(RESLIB10SRSFILES) \
    ) > $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    $(RSC) -presponse @$(mktmp \
    -r -p \
    $(foreach,i,$(alllangext) $(rsclang_{$i}) \
    $(rescharset_{$i}) \
    -fs{$(BIN)$/$(RESLIB10NAME)$(RESLIB10VERSION)$i.res} \
    $(foreach,j,$(RESLIB1IMAGES) -lip{$j}$/$(lang_{$i}) \
    -lip{$j} ) \
    -lip$(SOLARSRC)$/res$/$(lang_{$i}) -lip$(SOLARSRC)$/res ) \
    -subGLOBAL=$(SOLARSRC) \
    -subMODULE=$(PRJ) \
    -subCUSTOM=to_be_defined \
    -oil$(BIN) \
    -ft$@ \
    -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INC) $(SOLARINC) \
    $(RSC10HEADER) $(RESLIB10SRSFILES) \
    ) > $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""

$(RESLIB10TARGETN): \
        $(RESLIB10SRSFILES) \
        $(RESLIB10BMPS)
    @echo Making: $@
.IF "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
    @+-$(RM) $(RSC_MULTI10) >& $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    @+-$(RM) $(RSC_MULTI10) >& $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""
.ELSE				# "$(GUI)"=="UNX"
.IF "$(common_build_reslib)"!=""
    @+-$(RM) $(RSC_MULTI10) >& $(NULLDEV)
.ELSE			# "$(common_build_reslib)"!=""
    @+-$(RM) $(RSC_MULTI10) >& $(NULLDEV)
.ENDIF			# "$(common_build_reslib)"!=""
.ENDIF              # "$(GUI)"=="UNX"
.ENDIF				# "$(RESLIB10TARGETN)"!=""


# Anweisungen fuer das Linken
