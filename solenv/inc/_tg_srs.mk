# unroll begin

.IF "$(SRS1NAME)"!=""
.IF "$(BUILDHIDS)"!=""
HID1FILES=$(foreach,i,$(SRC1FILES:f) $(SRS)$/$(i:s/.src/.hid/))
HIDSRS1PARTICLE=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(SRS))$/$(SRS1NAME)_srs.hid
$(HIDSRS1PARTICLE) : $(HID1FILES)
        @echo ------------------------------
        @echo Making: $@
        @+if exist $@ rm $@
        +$(TYPE) $(HID1FILES) > $@.$(ROUT).tmp
        @+$(RENAME) $@.$(ROUT).tmp $@

.IF "$(L10N_framework)"==""
ALLTAR : $(HIDSRS1PARTICLE)
.ENDIF			# "$(L10N_framework)"==""

.ENDIF # "$(BUILDHIDS)"!=""

$(MISC)$/$(PWD:f).$(SRS1NAME).dprr: $(SRC1FILES) $(HIDSRS1PARTICLE) $(HID1FILES)
    @echo ------------------------------
    @echo Making: $@
    +-$(RM) $(MISC)$/$(PWD:f).$(SRS1NAME).dprr >& $(NULLDEV)
    +$(RSC) $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) -I$(INCLOCPRJ)  -I$(INCLOCAL) -I$(INC) -I$(INCCOM) $(RSCDEFS) $(RSCUPDVERDEF) -fp={$(SRS)$/$(SRS1NAME).srs} -fo=$@ $(SRC1FILES)

.IF "$(common_build_srs)"!=""
$(foreach,i,$(SRC1FILES) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(TARGET)$/$i) : $$(@:f) localize.sdf 
    +-$(MKDIR) $(@:d)
    +-$(RM) $@
    $(WRAPCMD) $(TRANSEX) -p $(PRJNAME) -i $(@:f) -o $(@).$(INPATH) -m localize.sdf -l all
# dirty hack
#     if (! -e $@.$(INPATH) ) cp $(@:f) $@.$(INPATH)
    +$(RENAME) $@.$(INPATH) $@
    +-$(RM) $@.$(INPATH)
.ELSE          # "$(common_build_srs)"!=""
$(foreach,i,$(SRC1FILES) $(MISC)$/$(TARGET)$/$i) : $$(@:f) localize.sdf
    +-$(MKDIR) $(@:d)
    +-$(RM) $@
    $(WRAPCMD) $(TRANSEX) -p $(PRJNAME) -i $(@:f) -o $(@).$(INPATH) -m localize.sdf -l all
# dirty hack
#     if (! -e $@.$(INPATH) ) cp $(@:f) $@.$(INPATH)
    +$(RENAME) $@.$(INPATH) $@
    +-$(RM) $@.$(INPATH)
.ENDIF          # "$(common_build_srs)"!=""

.IF "$(common_build_srs)"!=""
# SRS already pointing to common!?
#$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(SRS))$/$(SRS1NAME).srs: $(foreach,i,$(SRC1FILES) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(TARGET)$/$i)
$(SRS)$/$(SRS1NAME).srs: $(foreach,i,$(SRC1FILES) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(TARGET)$/$i)
    @echo ------------------------------
    @echo Making: $@
    +$(RSC) -presponse @$(mktmp \
        $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) \
        $(INCLUDE) $(RSCDEFS) $(RSCUPDVERDEF) \
        -fp$@.$(INPATH) \
        $(foreach,i,$(SRC1FILES) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(TARGET)$/$i) \
    )
    +-$(RM) $@
    +$(RENAME) $@.$(INPATH) $@
    +-$(RM) $@.$(INPATH)
.ELSE          # "$(common_build_srs)"!=""
$(SRS)$/$(SRS1NAME).srs: $(foreach,i,$(SRC1FILES) $(MISC)$/$(TARGET)$/$i)
    @echo ------------------------------
    @echo Making: $@
    +$(RSC) -presponse @$(mktmp \
        $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) \
        $(INCLUDE) $(RSCDEFS) $(RSCUPDVERDEF) \
        -fp=$@ \
        $(foreach,i,$(SRC1FILES) $(MISC)$/$(TARGET)$/$i) \
    )
.ENDIF          # "$(common_build_srs)"!=""

.ENDIF          # "$(SRS1NAME)"!=""

# unroll begin

.IF "$(SRS2NAME)"!=""
.IF "$(BUILDHIDS)"!=""
HID2FILES=$(foreach,i,$(SRC2FILES:f) $(SRS)$/$(i:s/.src/.hid/))
HIDSRS2PARTICLE=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(SRS))$/$(SRS2NAME)_srs.hid
$(HIDSRS2PARTICLE) : $(HID2FILES)
        @echo ------------------------------
        @echo Making: $@
        @+if exist $@ rm $@
        +$(TYPE) $(HID2FILES) > $@.$(ROUT).tmp
        @+$(RENAME) $@.$(ROUT).tmp $@

.IF "$(L10N_framework)"==""
ALLTAR : $(HIDSRS2PARTICLE)
.ENDIF			# "$(L10N_framework)"==""

.ENDIF # "$(BUILDHIDS)"!=""

$(MISC)$/$(PWD:f).$(SRS2NAME).dprr: $(SRC2FILES) $(HIDSRS2PARTICLE) $(HID2FILES)
    @echo ------------------------------
    @echo Making: $@
    +-$(RM) $(MISC)$/$(PWD:f).$(SRS2NAME).dprr >& $(NULLDEV)
    +$(RSC) $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) -I$(INCLOCPRJ)  -I$(INCLOCAL) -I$(INC) -I$(INCCOM) $(RSCDEFS) $(RSCUPDVERDEF) -fp={$(SRS)$/$(SRS2NAME).srs} -fo=$@ $(SRC2FILES)

.IF "$(common_build_srs)"!=""
$(foreach,i,$(SRC2FILES) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(TARGET)$/$i) : $$(@:f) localize.sdf 
    +-$(MKDIR) $(@:d)
    +-$(RM) $@
    $(WRAPCMD) $(TRANSEX) -p $(PRJNAME) -i $(@:f) -o $(@).$(INPATH) -m localize.sdf -l all
# dirty hack
#     if (! -e $@.$(INPATH) ) cp $(@:f) $@.$(INPATH)
    +$(RENAME) $@.$(INPATH) $@
    +-$(RM) $@.$(INPATH)
.ELSE          # "$(common_build_srs)"!=""
$(foreach,i,$(SRC2FILES) $(MISC)$/$(TARGET)$/$i) : $$(@:f) localize.sdf
    +-$(MKDIR) $(@:d)
    +-$(RM) $@
    $(WRAPCMD) $(TRANSEX) -p $(PRJNAME) -i $(@:f) -o $(@).$(INPATH) -m localize.sdf -l all
# dirty hack
#     if (! -e $@.$(INPATH) ) cp $(@:f) $@.$(INPATH)
    +$(RENAME) $@.$(INPATH) $@
    +-$(RM) $@.$(INPATH)
.ENDIF          # "$(common_build_srs)"!=""

.IF "$(common_build_srs)"!=""
# SRS already pointing to common!?
#$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(SRS))$/$(SRS2NAME).srs: $(foreach,i,$(SRC2FILES) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(TARGET)$/$i)
$(SRS)$/$(SRS2NAME).srs: $(foreach,i,$(SRC2FILES) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(TARGET)$/$i)
    @echo ------------------------------
    @echo Making: $@
    +$(RSC) -presponse @$(mktmp \
        $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) \
        $(INCLUDE) $(RSCDEFS) $(RSCUPDVERDEF) \
        -fp$@.$(INPATH) \
        $(foreach,i,$(SRC2FILES) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(TARGET)$/$i) \
    )
    +-$(RM) $@
    +$(RENAME) $@.$(INPATH) $@
    +-$(RM) $@.$(INPATH)
.ELSE          # "$(common_build_srs)"!=""
$(SRS)$/$(SRS2NAME).srs: $(foreach,i,$(SRC2FILES) $(MISC)$/$(TARGET)$/$i)
    @echo ------------------------------
    @echo Making: $@
    +$(RSC) -presponse @$(mktmp \
        $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) \
        $(INCLUDE) $(RSCDEFS) $(RSCUPDVERDEF) \
        -fp=$@ \
        $(foreach,i,$(SRC2FILES) $(MISC)$/$(TARGET)$/$i) \
    )
.ENDIF          # "$(common_build_srs)"!=""

.ENDIF          # "$(SRS2NAME)"!=""

# unroll begin

.IF "$(SRS3NAME)"!=""
.IF "$(BUILDHIDS)"!=""
HID3FILES=$(foreach,i,$(SRC3FILES:f) $(SRS)$/$(i:s/.src/.hid/))
HIDSRS3PARTICLE=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(SRS))$/$(SRS3NAME)_srs.hid
$(HIDSRS3PARTICLE) : $(HID3FILES)
        @echo ------------------------------
        @echo Making: $@
        @+if exist $@ rm $@
        +$(TYPE) $(HID3FILES) > $@.$(ROUT).tmp
        @+$(RENAME) $@.$(ROUT).tmp $@

.IF "$(L10N_framework)"==""
ALLTAR : $(HIDSRS3PARTICLE)
.ENDIF			# "$(L10N_framework)"==""

.ENDIF # "$(BUILDHIDS)"!=""

$(MISC)$/$(PWD:f).$(SRS3NAME).dprr: $(SRC3FILES) $(HIDSRS3PARTICLE) $(HID3FILES)
    @echo ------------------------------
    @echo Making: $@
    +-$(RM) $(MISC)$/$(PWD:f).$(SRS3NAME).dprr >& $(NULLDEV)
    +$(RSC) $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) -I$(INCLOCPRJ)  -I$(INCLOCAL) -I$(INC) -I$(INCCOM) $(RSCDEFS) $(RSCUPDVERDEF) -fp={$(SRS)$/$(SRS3NAME).srs} -fo=$@ $(SRC3FILES)

.IF "$(common_build_srs)"!=""
$(foreach,i,$(SRC3FILES) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(TARGET)$/$i) : $$(@:f) localize.sdf 
    +-$(MKDIR) $(@:d)
    +-$(RM) $@
    $(WRAPCMD) $(TRANSEX) -p $(PRJNAME) -i $(@:f) -o $(@).$(INPATH) -m localize.sdf -l all
# dirty hack
#     if (! -e $@.$(INPATH) ) cp $(@:f) $@.$(INPATH)
    +$(RENAME) $@.$(INPATH) $@
    +-$(RM) $@.$(INPATH)
.ELSE          # "$(common_build_srs)"!=""
$(foreach,i,$(SRC3FILES) $(MISC)$/$(TARGET)$/$i) : $$(@:f) localize.sdf
    +-$(MKDIR) $(@:d)
    +-$(RM) $@
    $(WRAPCMD) $(TRANSEX) -p $(PRJNAME) -i $(@:f) -o $(@).$(INPATH) -m localize.sdf -l all
# dirty hack
#     if (! -e $@.$(INPATH) ) cp $(@:f) $@.$(INPATH)
    +$(RENAME) $@.$(INPATH) $@
    +-$(RM) $@.$(INPATH)
.ENDIF          # "$(common_build_srs)"!=""

.IF "$(common_build_srs)"!=""
# SRS already pointing to common!?
#$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(SRS))$/$(SRS3NAME).srs: $(foreach,i,$(SRC3FILES) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(TARGET)$/$i)
$(SRS)$/$(SRS3NAME).srs: $(foreach,i,$(SRC3FILES) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(TARGET)$/$i)
    @echo ------------------------------
    @echo Making: $@
    +$(RSC) -presponse @$(mktmp \
        $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) \
        $(INCLUDE) $(RSCDEFS) $(RSCUPDVERDEF) \
        -fp$@.$(INPATH) \
        $(foreach,i,$(SRC3FILES) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(TARGET)$/$i) \
    )
    +-$(RM) $@
    +$(RENAME) $@.$(INPATH) $@
    +-$(RM) $@.$(INPATH)
.ELSE          # "$(common_build_srs)"!=""
$(SRS)$/$(SRS3NAME).srs: $(foreach,i,$(SRC3FILES) $(MISC)$/$(TARGET)$/$i)
    @echo ------------------------------
    @echo Making: $@
    +$(RSC) -presponse @$(mktmp \
        $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) \
        $(INCLUDE) $(RSCDEFS) $(RSCUPDVERDEF) \
        -fp=$@ \
        $(foreach,i,$(SRC3FILES) $(MISC)$/$(TARGET)$/$i) \
    )
.ENDIF          # "$(common_build_srs)"!=""

.ENDIF          # "$(SRS3NAME)"!=""

# unroll begin

.IF "$(SRS4NAME)"!=""
.IF "$(BUILDHIDS)"!=""
HID4FILES=$(foreach,i,$(SRC4FILES:f) $(SRS)$/$(i:s/.src/.hid/))
HIDSRS4PARTICLE=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(SRS))$/$(SRS4NAME)_srs.hid
$(HIDSRS4PARTICLE) : $(HID4FILES)
        @echo ------------------------------
        @echo Making: $@
        @+if exist $@ rm $@
        +$(TYPE) $(HID4FILES) > $@.$(ROUT).tmp
        @+$(RENAME) $@.$(ROUT).tmp $@

.IF "$(L10N_framework)"==""
ALLTAR : $(HIDSRS4PARTICLE)
.ENDIF			# "$(L10N_framework)"==""

.ENDIF # "$(BUILDHIDS)"!=""

$(MISC)$/$(PWD:f).$(SRS4NAME).dprr: $(SRC4FILES) $(HIDSRS4PARTICLE) $(HID4FILES)
    @echo ------------------------------
    @echo Making: $@
    +-$(RM) $(MISC)$/$(PWD:f).$(SRS4NAME).dprr >& $(NULLDEV)
    +$(RSC) $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) -I$(INCLOCPRJ)  -I$(INCLOCAL) -I$(INC) -I$(INCCOM) $(RSCDEFS) $(RSCUPDVERDEF) -fp={$(SRS)$/$(SRS4NAME).srs} -fo=$@ $(SRC4FILES)

.IF "$(common_build_srs)"!=""
$(foreach,i,$(SRC4FILES) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(TARGET)$/$i) : $$(@:f) localize.sdf 
    +-$(MKDIR) $(@:d)
    +-$(RM) $@
    $(WRAPCMD) $(TRANSEX) -p $(PRJNAME) -i $(@:f) -o $(@).$(INPATH) -m localize.sdf -l all
# dirty hack
#     if (! -e $@.$(INPATH) ) cp $(@:f) $@.$(INPATH)
    +$(RENAME) $@.$(INPATH) $@
    +-$(RM) $@.$(INPATH)
.ELSE          # "$(common_build_srs)"!=""
$(foreach,i,$(SRC4FILES) $(MISC)$/$(TARGET)$/$i) : $$(@:f) localize.sdf
    +-$(MKDIR) $(@:d)
    +-$(RM) $@
    $(WRAPCMD) $(TRANSEX) -p $(PRJNAME) -i $(@:f) -o $(@).$(INPATH) -m localize.sdf -l all
# dirty hack
#     if (! -e $@.$(INPATH) ) cp $(@:f) $@.$(INPATH)
    +$(RENAME) $@.$(INPATH) $@
    +-$(RM) $@.$(INPATH)
.ENDIF          # "$(common_build_srs)"!=""

.IF "$(common_build_srs)"!=""
# SRS already pointing to common!?
#$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(SRS))$/$(SRS4NAME).srs: $(foreach,i,$(SRC4FILES) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(TARGET)$/$i)
$(SRS)$/$(SRS4NAME).srs: $(foreach,i,$(SRC4FILES) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(TARGET)$/$i)
    @echo ------------------------------
    @echo Making: $@
    +$(RSC) -presponse @$(mktmp \
        $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) \
        $(INCLUDE) $(RSCDEFS) $(RSCUPDVERDEF) \
        -fp$@.$(INPATH) \
        $(foreach,i,$(SRC4FILES) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(TARGET)$/$i) \
    )
    +-$(RM) $@
    +$(RENAME) $@.$(INPATH) $@
    +-$(RM) $@.$(INPATH)
.ELSE          # "$(common_build_srs)"!=""
$(SRS)$/$(SRS4NAME).srs: $(foreach,i,$(SRC4FILES) $(MISC)$/$(TARGET)$/$i)
    @echo ------------------------------
    @echo Making: $@
    +$(RSC) -presponse @$(mktmp \
        $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) \
        $(INCLUDE) $(RSCDEFS) $(RSCUPDVERDEF) \
        -fp=$@ \
        $(foreach,i,$(SRC4FILES) $(MISC)$/$(TARGET)$/$i) \
    )
.ENDIF          # "$(common_build_srs)"!=""

.ENDIF          # "$(SRS4NAME)"!=""

# unroll begin

.IF "$(SRS5NAME)"!=""
.IF "$(BUILDHIDS)"!=""
HID5FILES=$(foreach,i,$(SRC5FILES:f) $(SRS)$/$(i:s/.src/.hid/))
HIDSRS5PARTICLE=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(SRS))$/$(SRS5NAME)_srs.hid
$(HIDSRS5PARTICLE) : $(HID5FILES)
        @echo ------------------------------
        @echo Making: $@
        @+if exist $@ rm $@
        +$(TYPE) $(HID5FILES) > $@.$(ROUT).tmp
        @+$(RENAME) $@.$(ROUT).tmp $@

.IF "$(L10N_framework)"==""
ALLTAR : $(HIDSRS5PARTICLE)
.ENDIF			# "$(L10N_framework)"==""

.ENDIF # "$(BUILDHIDS)"!=""

$(MISC)$/$(PWD:f).$(SRS5NAME).dprr: $(SRC5FILES) $(HIDSRS5PARTICLE) $(HID5FILES)
    @echo ------------------------------
    @echo Making: $@
    +-$(RM) $(MISC)$/$(PWD:f).$(SRS5NAME).dprr >& $(NULLDEV)
    +$(RSC) $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) -I$(INCLOCPRJ)  -I$(INCLOCAL) -I$(INC) -I$(INCCOM) $(RSCDEFS) $(RSCUPDVERDEF) -fp={$(SRS)$/$(SRS5NAME).srs} -fo=$@ $(SRC5FILES)

.IF "$(common_build_srs)"!=""
$(foreach,i,$(SRC5FILES) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(TARGET)$/$i) : $$(@:f) localize.sdf 
    +-$(MKDIR) $(@:d)
    +-$(RM) $@
    $(WRAPCMD) $(TRANSEX) -p $(PRJNAME) -i $(@:f) -o $(@).$(INPATH) -m localize.sdf -l all
# dirty hack
#     if (! -e $@.$(INPATH) ) cp $(@:f) $@.$(INPATH)
    +$(RENAME) $@.$(INPATH) $@
    +-$(RM) $@.$(INPATH)
.ELSE          # "$(common_build_srs)"!=""
$(foreach,i,$(SRC5FILES) $(MISC)$/$(TARGET)$/$i) : $$(@:f) localize.sdf
    +-$(MKDIR) $(@:d)
    +-$(RM) $@
    $(WRAPCMD) $(TRANSEX) -p $(PRJNAME) -i $(@:f) -o $(@).$(INPATH) -m localize.sdf -l all
# dirty hack
#     if (! -e $@.$(INPATH) ) cp $(@:f) $@.$(INPATH)
    +$(RENAME) $@.$(INPATH) $@
    +-$(RM) $@.$(INPATH)
.ENDIF          # "$(common_build_srs)"!=""

.IF "$(common_build_srs)"!=""
# SRS already pointing to common!?
#$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(SRS))$/$(SRS5NAME).srs: $(foreach,i,$(SRC5FILES) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(TARGET)$/$i)
$(SRS)$/$(SRS5NAME).srs: $(foreach,i,$(SRC5FILES) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(TARGET)$/$i)
    @echo ------------------------------
    @echo Making: $@
    +$(RSC) -presponse @$(mktmp \
        $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) \
        $(INCLUDE) $(RSCDEFS) $(RSCUPDVERDEF) \
        -fp$@.$(INPATH) \
        $(foreach,i,$(SRC5FILES) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(TARGET)$/$i) \
    )
    +-$(RM) $@
    +$(RENAME) $@.$(INPATH) $@
    +-$(RM) $@.$(INPATH)
.ELSE          # "$(common_build_srs)"!=""
$(SRS)$/$(SRS5NAME).srs: $(foreach,i,$(SRC5FILES) $(MISC)$/$(TARGET)$/$i)
    @echo ------------------------------
    @echo Making: $@
    +$(RSC) -presponse @$(mktmp \
        $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) \
        $(INCLUDE) $(RSCDEFS) $(RSCUPDVERDEF) \
        -fp=$@ \
        $(foreach,i,$(SRC5FILES) $(MISC)$/$(TARGET)$/$i) \
    )
.ENDIF          # "$(common_build_srs)"!=""

.ENDIF          # "$(SRS5NAME)"!=""

# unroll begin

.IF "$(SRS6NAME)"!=""
.IF "$(BUILDHIDS)"!=""
HID6FILES=$(foreach,i,$(SRC6FILES:f) $(SRS)$/$(i:s/.src/.hid/))
HIDSRS6PARTICLE=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(SRS))$/$(SRS6NAME)_srs.hid
$(HIDSRS6PARTICLE) : $(HID6FILES)
        @echo ------------------------------
        @echo Making: $@
        @+if exist $@ rm $@
        +$(TYPE) $(HID6FILES) > $@.$(ROUT).tmp
        @+$(RENAME) $@.$(ROUT).tmp $@

.IF "$(L10N_framework)"==""
ALLTAR : $(HIDSRS6PARTICLE)
.ENDIF			# "$(L10N_framework)"==""

.ENDIF # "$(BUILDHIDS)"!=""

$(MISC)$/$(PWD:f).$(SRS6NAME).dprr: $(SRC6FILES) $(HIDSRS6PARTICLE) $(HID6FILES)
    @echo ------------------------------
    @echo Making: $@
    +-$(RM) $(MISC)$/$(PWD:f).$(SRS6NAME).dprr >& $(NULLDEV)
    +$(RSC) $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) -I$(INCLOCPRJ)  -I$(INCLOCAL) -I$(INC) -I$(INCCOM) $(RSCDEFS) $(RSCUPDVERDEF) -fp={$(SRS)$/$(SRS6NAME).srs} -fo=$@ $(SRC6FILES)

.IF "$(common_build_srs)"!=""
$(foreach,i,$(SRC6FILES) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(TARGET)$/$i) : $$(@:f) localize.sdf 
    +-$(MKDIR) $(@:d)
    +-$(RM) $@
    $(WRAPCMD) $(TRANSEX) -p $(PRJNAME) -i $(@:f) -o $(@).$(INPATH) -m localize.sdf -l all
# dirty hack
#     if (! -e $@.$(INPATH) ) cp $(@:f) $@.$(INPATH)
    +$(RENAME) $@.$(INPATH) $@
    +-$(RM) $@.$(INPATH)
.ELSE          # "$(common_build_srs)"!=""
$(foreach,i,$(SRC6FILES) $(MISC)$/$(TARGET)$/$i) : $$(@:f) localize.sdf
    +-$(MKDIR) $(@:d)
    +-$(RM) $@
    $(WRAPCMD) $(TRANSEX) -p $(PRJNAME) -i $(@:f) -o $(@).$(INPATH) -m localize.sdf -l all
# dirty hack
#     if (! -e $@.$(INPATH) ) cp $(@:f) $@.$(INPATH)
    +$(RENAME) $@.$(INPATH) $@
    +-$(RM) $@.$(INPATH)
.ENDIF          # "$(common_build_srs)"!=""

.IF "$(common_build_srs)"!=""
# SRS already pointing to common!?
#$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(SRS))$/$(SRS6NAME).srs: $(foreach,i,$(SRC6FILES) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(TARGET)$/$i)
$(SRS)$/$(SRS6NAME).srs: $(foreach,i,$(SRC6FILES) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(TARGET)$/$i)
    @echo ------------------------------
    @echo Making: $@
    +$(RSC) -presponse @$(mktmp \
        $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) \
        $(INCLUDE) $(RSCDEFS) $(RSCUPDVERDEF) \
        -fp$@.$(INPATH) \
        $(foreach,i,$(SRC6FILES) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(TARGET)$/$i) \
    )
    +-$(RM) $@
    +$(RENAME) $@.$(INPATH) $@
    +-$(RM) $@.$(INPATH)
.ELSE          # "$(common_build_srs)"!=""
$(SRS)$/$(SRS6NAME).srs: $(foreach,i,$(SRC6FILES) $(MISC)$/$(TARGET)$/$i)
    @echo ------------------------------
    @echo Making: $@
    +$(RSC) -presponse @$(mktmp \
        $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) \
        $(INCLUDE) $(RSCDEFS) $(RSCUPDVERDEF) \
        -fp=$@ \
        $(foreach,i,$(SRC6FILES) $(MISC)$/$(TARGET)$/$i) \
    )
.ENDIF          # "$(common_build_srs)"!=""

.ENDIF          # "$(SRS6NAME)"!=""

# unroll begin

.IF "$(SRS7NAME)"!=""
.IF "$(BUILDHIDS)"!=""
HID7FILES=$(foreach,i,$(SRC7FILES:f) $(SRS)$/$(i:s/.src/.hid/))
HIDSRS7PARTICLE=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(SRS))$/$(SRS7NAME)_srs.hid
$(HIDSRS7PARTICLE) : $(HID7FILES)
        @echo ------------------------------
        @echo Making: $@
        @+if exist $@ rm $@
        +$(TYPE) $(HID7FILES) > $@.$(ROUT).tmp
        @+$(RENAME) $@.$(ROUT).tmp $@

.IF "$(L10N_framework)"==""
ALLTAR : $(HIDSRS7PARTICLE)
.ENDIF			# "$(L10N_framework)"==""

.ENDIF # "$(BUILDHIDS)"!=""

$(MISC)$/$(PWD:f).$(SRS7NAME).dprr: $(SRC7FILES) $(HIDSRS7PARTICLE) $(HID7FILES)
    @echo ------------------------------
    @echo Making: $@
    +-$(RM) $(MISC)$/$(PWD:f).$(SRS7NAME).dprr >& $(NULLDEV)
    +$(RSC) $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) -I$(INCLOCPRJ)  -I$(INCLOCAL) -I$(INC) -I$(INCCOM) $(RSCDEFS) $(RSCUPDVERDEF) -fp={$(SRS)$/$(SRS7NAME).srs} -fo=$@ $(SRC7FILES)

.IF "$(common_build_srs)"!=""
$(foreach,i,$(SRC7FILES) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(TARGET)$/$i) : $$(@:f) localize.sdf 
    +-$(MKDIR) $(@:d)
    +-$(RM) $@
    $(WRAPCMD) $(TRANSEX) -p $(PRJNAME) -i $(@:f) -o $(@).$(INPATH) -m localize.sdf -l all
# dirty hack
#     if (! -e $@.$(INPATH) ) cp $(@:f) $@.$(INPATH)
    +$(RENAME) $@.$(INPATH) $@
    +-$(RM) $@.$(INPATH)
.ELSE          # "$(common_build_srs)"!=""
$(foreach,i,$(SRC7FILES) $(MISC)$/$(TARGET)$/$i) : $$(@:f) localize.sdf
    +-$(MKDIR) $(@:d)
    +-$(RM) $@
    $(WRAPCMD) $(TRANSEX) -p $(PRJNAME) -i $(@:f) -o $(@).$(INPATH) -m localize.sdf -l all
# dirty hack
#     if (! -e $@.$(INPATH) ) cp $(@:f) $@.$(INPATH)
    +$(RENAME) $@.$(INPATH) $@
    +-$(RM) $@.$(INPATH)
.ENDIF          # "$(common_build_srs)"!=""

.IF "$(common_build_srs)"!=""
# SRS already pointing to common!?
#$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(SRS))$/$(SRS7NAME).srs: $(foreach,i,$(SRC7FILES) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(TARGET)$/$i)
$(SRS)$/$(SRS7NAME).srs: $(foreach,i,$(SRC7FILES) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(TARGET)$/$i)
    @echo ------------------------------
    @echo Making: $@
    +$(RSC) -presponse @$(mktmp \
        $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) \
        $(INCLUDE) $(RSCDEFS) $(RSCUPDVERDEF) \
        -fp$@.$(INPATH) \
        $(foreach,i,$(SRC7FILES) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(TARGET)$/$i) \
    )
    +-$(RM) $@
    +$(RENAME) $@.$(INPATH) $@
    +-$(RM) $@.$(INPATH)
.ELSE          # "$(common_build_srs)"!=""
$(SRS)$/$(SRS7NAME).srs: $(foreach,i,$(SRC7FILES) $(MISC)$/$(TARGET)$/$i)
    @echo ------------------------------
    @echo Making: $@
    +$(RSC) -presponse @$(mktmp \
        $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) \
        $(INCLUDE) $(RSCDEFS) $(RSCUPDVERDEF) \
        -fp=$@ \
        $(foreach,i,$(SRC7FILES) $(MISC)$/$(TARGET)$/$i) \
    )
.ENDIF          # "$(common_build_srs)"!=""

.ENDIF          # "$(SRS7NAME)"!=""

# unroll begin

.IF "$(SRS8NAME)"!=""
.IF "$(BUILDHIDS)"!=""
HID8FILES=$(foreach,i,$(SRC8FILES:f) $(SRS)$/$(i:s/.src/.hid/))
HIDSRS8PARTICLE=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(SRS))$/$(SRS8NAME)_srs.hid
$(HIDSRS8PARTICLE) : $(HID8FILES)
        @echo ------------------------------
        @echo Making: $@
        @+if exist $@ rm $@
        +$(TYPE) $(HID8FILES) > $@.$(ROUT).tmp
        @+$(RENAME) $@.$(ROUT).tmp $@

.IF "$(L10N_framework)"==""
ALLTAR : $(HIDSRS8PARTICLE)
.ENDIF			# "$(L10N_framework)"==""

.ENDIF # "$(BUILDHIDS)"!=""

$(MISC)$/$(PWD:f).$(SRS8NAME).dprr: $(SRC8FILES) $(HIDSRS8PARTICLE) $(HID8FILES)
    @echo ------------------------------
    @echo Making: $@
    +-$(RM) $(MISC)$/$(PWD:f).$(SRS8NAME).dprr >& $(NULLDEV)
    +$(RSC) $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) -I$(INCLOCPRJ)  -I$(INCLOCAL) -I$(INC) -I$(INCCOM) $(RSCDEFS) $(RSCUPDVERDEF) -fp={$(SRS)$/$(SRS8NAME).srs} -fo=$@ $(SRC8FILES)

.IF "$(common_build_srs)"!=""
$(foreach,i,$(SRC8FILES) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(TARGET)$/$i) : $$(@:f) localize.sdf 
    +-$(MKDIR) $(@:d)
    +-$(RM) $@
    $(WRAPCMD) $(TRANSEX) -p $(PRJNAME) -i $(@:f) -o $(@).$(INPATH) -m localize.sdf -l all
# dirty hack
#     if (! -e $@.$(INPATH) ) cp $(@:f) $@.$(INPATH)
    +$(RENAME) $@.$(INPATH) $@
    +-$(RM) $@.$(INPATH)
.ELSE          # "$(common_build_srs)"!=""
$(foreach,i,$(SRC8FILES) $(MISC)$/$(TARGET)$/$i) : $$(@:f) localize.sdf
    +-$(MKDIR) $(@:d)
    +-$(RM) $@
    $(WRAPCMD) $(TRANSEX) -p $(PRJNAME) -i $(@:f) -o $(@).$(INPATH) -m localize.sdf -l all
# dirty hack
#     if (! -e $@.$(INPATH) ) cp $(@:f) $@.$(INPATH)
    +$(RENAME) $@.$(INPATH) $@
    +-$(RM) $@.$(INPATH)
.ENDIF          # "$(common_build_srs)"!=""

.IF "$(common_build_srs)"!=""
# SRS already pointing to common!?
#$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(SRS))$/$(SRS8NAME).srs: $(foreach,i,$(SRC8FILES) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(TARGET)$/$i)
$(SRS)$/$(SRS8NAME).srs: $(foreach,i,$(SRC8FILES) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(TARGET)$/$i)
    @echo ------------------------------
    @echo Making: $@
    +$(RSC) -presponse @$(mktmp \
        $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) \
        $(INCLUDE) $(RSCDEFS) $(RSCUPDVERDEF) \
        -fp$@.$(INPATH) \
        $(foreach,i,$(SRC8FILES) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(TARGET)$/$i) \
    )
    +-$(RM) $@
    +$(RENAME) $@.$(INPATH) $@
    +-$(RM) $@.$(INPATH)
.ELSE          # "$(common_build_srs)"!=""
$(SRS)$/$(SRS8NAME).srs: $(foreach,i,$(SRC8FILES) $(MISC)$/$(TARGET)$/$i)
    @echo ------------------------------
    @echo Making: $@
    +$(RSC) -presponse @$(mktmp \
        $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) \
        $(INCLUDE) $(RSCDEFS) $(RSCUPDVERDEF) \
        -fp=$@ \
        $(foreach,i,$(SRC8FILES) $(MISC)$/$(TARGET)$/$i) \
    )
.ENDIF          # "$(common_build_srs)"!=""

.ENDIF          # "$(SRS8NAME)"!=""

# unroll begin

.IF "$(SRS9NAME)"!=""
.IF "$(BUILDHIDS)"!=""
HID9FILES=$(foreach,i,$(SRC9FILES:f) $(SRS)$/$(i:s/.src/.hid/))
HIDSRS9PARTICLE=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(SRS))$/$(SRS9NAME)_srs.hid
$(HIDSRS9PARTICLE) : $(HID9FILES)
        @echo ------------------------------
        @echo Making: $@
        @+if exist $@ rm $@
        +$(TYPE) $(HID9FILES) > $@.$(ROUT).tmp
        @+$(RENAME) $@.$(ROUT).tmp $@

.IF "$(L10N_framework)"==""
ALLTAR : $(HIDSRS9PARTICLE)
.ENDIF			# "$(L10N_framework)"==""

.ENDIF # "$(BUILDHIDS)"!=""

$(MISC)$/$(PWD:f).$(SRS9NAME).dprr: $(SRC9FILES) $(HIDSRS9PARTICLE) $(HID9FILES)
    @echo ------------------------------
    @echo Making: $@
    +-$(RM) $(MISC)$/$(PWD:f).$(SRS9NAME).dprr >& $(NULLDEV)
    +$(RSC) $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) -I$(INCLOCPRJ)  -I$(INCLOCAL) -I$(INC) -I$(INCCOM) $(RSCDEFS) $(RSCUPDVERDEF) -fp={$(SRS)$/$(SRS9NAME).srs} -fo=$@ $(SRC9FILES)

.IF "$(common_build_srs)"!=""
$(foreach,i,$(SRC9FILES) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(TARGET)$/$i) : $$(@:f) localize.sdf 
    +-$(MKDIR) $(@:d)
    +-$(RM) $@
    $(WRAPCMD) $(TRANSEX) -p $(PRJNAME) -i $(@:f) -o $(@).$(INPATH) -m localize.sdf -l all
# dirty hack
#     if (! -e $@.$(INPATH) ) cp $(@:f) $@.$(INPATH)
    +$(RENAME) $@.$(INPATH) $@
    +-$(RM) $@.$(INPATH)
.ELSE          # "$(common_build_srs)"!=""
$(foreach,i,$(SRC9FILES) $(MISC)$/$(TARGET)$/$i) : $$(@:f) localize.sdf
    +-$(MKDIR) $(@:d)
    +-$(RM) $@
    $(WRAPCMD) $(TRANSEX) -p $(PRJNAME) -i $(@:f) -o $(@).$(INPATH) -m localize.sdf -l all
# dirty hack
#     if (! -e $@.$(INPATH) ) cp $(@:f) $@.$(INPATH)
    +$(RENAME) $@.$(INPATH) $@
    +-$(RM) $@.$(INPATH)
.ENDIF          # "$(common_build_srs)"!=""

.IF "$(common_build_srs)"!=""
# SRS already pointing to common!?
#$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(SRS))$/$(SRS9NAME).srs: $(foreach,i,$(SRC9FILES) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(TARGET)$/$i)
$(SRS)$/$(SRS9NAME).srs: $(foreach,i,$(SRC9FILES) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(TARGET)$/$i)
    @echo ------------------------------
    @echo Making: $@
    +$(RSC) -presponse @$(mktmp \
        $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) \
        $(INCLUDE) $(RSCDEFS) $(RSCUPDVERDEF) \
        -fp$@.$(INPATH) \
        $(foreach,i,$(SRC9FILES) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(TARGET)$/$i) \
    )
    +-$(RM) $@
    +$(RENAME) $@.$(INPATH) $@
    +-$(RM) $@.$(INPATH)
.ELSE          # "$(common_build_srs)"!=""
$(SRS)$/$(SRS9NAME).srs: $(foreach,i,$(SRC9FILES) $(MISC)$/$(TARGET)$/$i)
    @echo ------------------------------
    @echo Making: $@
    +$(RSC) -presponse @$(mktmp \
        $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) \
        $(INCLUDE) $(RSCDEFS) $(RSCUPDVERDEF) \
        -fp=$@ \
        $(foreach,i,$(SRC9FILES) $(MISC)$/$(TARGET)$/$i) \
    )
.ENDIF          # "$(common_build_srs)"!=""

.ENDIF          # "$(SRS9NAME)"!=""

# unroll begin

.IF "$(SRS10NAME)"!=""
.IF "$(BUILDHIDS)"!=""
HID10FILES=$(foreach,i,$(SRC10FILES:f) $(SRS)$/$(i:s/.src/.hid/))
HIDSRS10PARTICLE=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(SRS))$/$(SRS10NAME)_srs.hid
$(HIDSRS10PARTICLE) : $(HID10FILES)
        @echo ------------------------------
        @echo Making: $@
        @+if exist $@ rm $@
        +$(TYPE) $(HID10FILES) > $@.$(ROUT).tmp
        @+$(RENAME) $@.$(ROUT).tmp $@

.IF "$(L10N_framework)"==""
ALLTAR : $(HIDSRS10PARTICLE)
.ENDIF			# "$(L10N_framework)"==""

.ENDIF # "$(BUILDHIDS)"!=""

$(MISC)$/$(PWD:f).$(SRS10NAME).dprr: $(SRC10FILES) $(HIDSRS10PARTICLE) $(HID10FILES)
    @echo ------------------------------
    @echo Making: $@
    +-$(RM) $(MISC)$/$(PWD:f).$(SRS10NAME).dprr >& $(NULLDEV)
    +$(RSC) $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) -I$(INCLOCPRJ)  -I$(INCLOCAL) -I$(INC) -I$(INCCOM) $(RSCDEFS) $(RSCUPDVERDEF) -fp={$(SRS)$/$(SRS10NAME).srs} -fo=$@ $(SRC10FILES)

.IF "$(common_build_srs)"!=""
$(foreach,i,$(SRC10FILES) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(TARGET)$/$i) : $$(@:f) localize.sdf 
    +-$(MKDIR) $(@:d)
    +-$(RM) $@
    $(WRAPCMD) $(TRANSEX) -p $(PRJNAME) -i $(@:f) -o $(@).$(INPATH) -m localize.sdf -l all
# dirty hack
#     if (! -e $@.$(INPATH) ) cp $(@:f) $@.$(INPATH)
    +$(RENAME) $@.$(INPATH) $@
    +-$(RM) $@.$(INPATH)
.ELSE          # "$(common_build_srs)"!=""
$(foreach,i,$(SRC10FILES) $(MISC)$/$(TARGET)$/$i) : $$(@:f) localize.sdf
    +-$(MKDIR) $(@:d)
    +-$(RM) $@
    $(WRAPCMD) $(TRANSEX) -p $(PRJNAME) -i $(@:f) -o $(@).$(INPATH) -m localize.sdf -l all
# dirty hack
#     if (! -e $@.$(INPATH) ) cp $(@:f) $@.$(INPATH)
    +$(RENAME) $@.$(INPATH) $@
    +-$(RM) $@.$(INPATH)
.ENDIF          # "$(common_build_srs)"!=""

.IF "$(common_build_srs)"!=""
# SRS already pointing to common!?
#$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(SRS))$/$(SRS10NAME).srs: $(foreach,i,$(SRC10FILES) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(TARGET)$/$i)
$(SRS)$/$(SRS10NAME).srs: $(foreach,i,$(SRC10FILES) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(TARGET)$/$i)
    @echo ------------------------------
    @echo Making: $@
    +$(RSC) -presponse @$(mktmp \
        $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) \
        $(INCLUDE) $(RSCDEFS) $(RSCUPDVERDEF) \
        -fp$@.$(INPATH) \
        $(foreach,i,$(SRC10FILES) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(TARGET)$/$i) \
    )
    +-$(RM) $@
    +$(RENAME) $@.$(INPATH) $@
    +-$(RM) $@.$(INPATH)
.ELSE          # "$(common_build_srs)"!=""
$(SRS)$/$(SRS10NAME).srs: $(foreach,i,$(SRC10FILES) $(MISC)$/$(TARGET)$/$i)
    @echo ------------------------------
    @echo Making: $@
    +$(RSC) -presponse @$(mktmp \
        $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) \
        $(INCLUDE) $(RSCDEFS) $(RSCUPDVERDEF) \
        -fp=$@ \
        $(foreach,i,$(SRC10FILES) $(MISC)$/$(TARGET)$/$i) \
    )
.ENDIF          # "$(common_build_srs)"!=""

.ENDIF          # "$(SRS10NAME)"!=""

