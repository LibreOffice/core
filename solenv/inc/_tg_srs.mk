# unroll begin

.IF "$(SRS1NAME)"!=""
.IF "$(BUILDHIDS)"!=""
HID1FILES=$(foreach,i,$(SRC1FILES:f) $(SRS)$/$(i:s/.src/.hid/))
HIDSRS1PARTICLE=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(SRS))$/$(SRS1NAME)_srs.hid
$(HIDSRS1PARTICLE) : $(HID1FILES)
    @echo ------------------------------
    @echo Making: $@
    @-$(RM) $@
    $(TYPE) $(mktmp  $(subst,$/,/ $(HID1FILES))) | xargs -s 1000 cat > $@.$(ROUT).tmp
    @$(RENAME) $@.$(ROUT).tmp $@

ALLTAR : $(HIDSRS1PARTICLE)

.ENDIF # "$(BUILDHIDS)"!=""

$(MISC)$/$(PWD:f).$(SRS1NAME).dprr: $(SRC1FILES) $(HIDSRS1PARTICLE) $(HID1FILES)
    @echo ------------------------------
    @echo Making: $@
    @@-$(RM) $(MISC)$/$(PWD:f).$(SRS1NAME).dprr
    $(RSC) $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) -I$(INCLOCPRJ)  -I$(INCLOCAL) -I$(INC) -I$(INCCOM) $(RSCDEFS) $(RSCUPDVERDEF) -fp={$(SRS)$/$(SRS1NAME).srs} -fo=$@ $(SRC1FILES)

.IF "$(WITH_LANG)"!=""
$(foreach,i,$(SRC1FILES) $(COMMONMISC)$/$(TARGET)$/$i) : $$(@:f) localize.sdf 
    -$(MKDIR) $(@:d)
    -$(RM) $@
    $(TRANSEX) -p $(PRJNAME) -i $(@:f) -o $(@).$(INPATH) -m localize.sdf -l all
    $(RENAME) $@.$(INPATH) $@
    -$(RM) $@.$(INPATH)

$(SRS)$/$(SRS1NAME).srs: $(foreach,i,$(SRC1FILES) $(COMMONMISC)$/$(TARGET)$/$i)
.ELSE			# "$(WITH_LANG)"!=""
$(SRS)$/$(SRS1NAME).srs: $(SRC1FILES)
.ENDIF			# "$(WITH_LANG)"!=""
    @echo ------------------------------
    @echo Making: $@
    $(RSC) -presponse @$(mktmp \
        $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) \
        $(INCLUDE) $(RSCDEFS) $(RSCUPDVERDEF) \
        -fp=$@.$(INPATH) \
        $< \
    )
    -$(RM) $@
    $(RENAME) $@.$(INPATH) $@
    -$(RM) $@.$(INPATH)

.ENDIF          # "$(SRS1NAME)"!=""

# unroll begin

.IF "$(SRS2NAME)"!=""
.IF "$(BUILDHIDS)"!=""
HID2FILES=$(foreach,i,$(SRC2FILES:f) $(SRS)$/$(i:s/.src/.hid/))
HIDSRS2PARTICLE=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(SRS))$/$(SRS2NAME)_srs.hid
$(HIDSRS2PARTICLE) : $(HID2FILES)
    @echo ------------------------------
    @echo Making: $@
    @-$(RM) $@
    $(TYPE) $(mktmp  $(subst,$/,/ $(HID2FILES))) | xargs -s 1000 cat > $@.$(ROUT).tmp
    @$(RENAME) $@.$(ROUT).tmp $@

ALLTAR : $(HIDSRS2PARTICLE)

.ENDIF # "$(BUILDHIDS)"!=""

$(MISC)$/$(PWD:f).$(SRS2NAME).dprr: $(SRC2FILES) $(HIDSRS2PARTICLE) $(HID2FILES)
    @echo ------------------------------
    @echo Making: $@
    @@-$(RM) $(MISC)$/$(PWD:f).$(SRS2NAME).dprr
    $(RSC) $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) -I$(INCLOCPRJ)  -I$(INCLOCAL) -I$(INC) -I$(INCCOM) $(RSCDEFS) $(RSCUPDVERDEF) -fp={$(SRS)$/$(SRS2NAME).srs} -fo=$@ $(SRC2FILES)

.IF "$(WITH_LANG)"!=""
$(foreach,i,$(SRC2FILES) $(COMMONMISC)$/$(TARGET)$/$i) : $$(@:f) localize.sdf 
    -$(MKDIR) $(@:d)
    -$(RM) $@
    $(TRANSEX) -p $(PRJNAME) -i $(@:f) -o $(@).$(INPATH) -m localize.sdf -l all
    $(RENAME) $@.$(INPATH) $@
    -$(RM) $@.$(INPATH)

$(SRS)$/$(SRS2NAME).srs: $(foreach,i,$(SRC2FILES) $(COMMONMISC)$/$(TARGET)$/$i)
.ELSE			# "$(WITH_LANG)"!=""
$(SRS)$/$(SRS2NAME).srs: $(SRC2FILES)
.ENDIF			# "$(WITH_LANG)"!=""
    @echo ------------------------------
    @echo Making: $@
    $(RSC) -presponse @$(mktmp \
        $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) \
        $(INCLUDE) $(RSCDEFS) $(RSCUPDVERDEF) \
        -fp=$@.$(INPATH) \
        $< \
    )
    -$(RM) $@
    $(RENAME) $@.$(INPATH) $@
    -$(RM) $@.$(INPATH)

.ENDIF          # "$(SRS2NAME)"!=""

# unroll begin

.IF "$(SRS3NAME)"!=""
.IF "$(BUILDHIDS)"!=""
HID3FILES=$(foreach,i,$(SRC3FILES:f) $(SRS)$/$(i:s/.src/.hid/))
HIDSRS3PARTICLE=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(SRS))$/$(SRS3NAME)_srs.hid
$(HIDSRS3PARTICLE) : $(HID3FILES)
    @echo ------------------------------
    @echo Making: $@
    @-$(RM) $@
    $(TYPE) $(mktmp  $(subst,$/,/ $(HID3FILES))) | xargs -s 1000 cat > $@.$(ROUT).tmp
    @$(RENAME) $@.$(ROUT).tmp $@

ALLTAR : $(HIDSRS3PARTICLE)

.ENDIF # "$(BUILDHIDS)"!=""

$(MISC)$/$(PWD:f).$(SRS3NAME).dprr: $(SRC3FILES) $(HIDSRS3PARTICLE) $(HID3FILES)
    @echo ------------------------------
    @echo Making: $@
    @@-$(RM) $(MISC)$/$(PWD:f).$(SRS3NAME).dprr
    $(RSC) $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) -I$(INCLOCPRJ)  -I$(INCLOCAL) -I$(INC) -I$(INCCOM) $(RSCDEFS) $(RSCUPDVERDEF) -fp={$(SRS)$/$(SRS3NAME).srs} -fo=$@ $(SRC3FILES)

.IF "$(WITH_LANG)"!=""
$(foreach,i,$(SRC3FILES) $(COMMONMISC)$/$(TARGET)$/$i) : $$(@:f) localize.sdf 
    -$(MKDIR) $(@:d)
    -$(RM) $@
    $(TRANSEX) -p $(PRJNAME) -i $(@:f) -o $(@).$(INPATH) -m localize.sdf -l all
    $(RENAME) $@.$(INPATH) $@
    -$(RM) $@.$(INPATH)

$(SRS)$/$(SRS3NAME).srs: $(foreach,i,$(SRC3FILES) $(COMMONMISC)$/$(TARGET)$/$i)
.ELSE			# "$(WITH_LANG)"!=""
$(SRS)$/$(SRS3NAME).srs: $(SRC3FILES)
.ENDIF			# "$(WITH_LANG)"!=""
    @echo ------------------------------
    @echo Making: $@
    $(RSC) -presponse @$(mktmp \
        $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) \
        $(INCLUDE) $(RSCDEFS) $(RSCUPDVERDEF) \
        -fp=$@.$(INPATH) \
        $< \
    )
    -$(RM) $@
    $(RENAME) $@.$(INPATH) $@
    -$(RM) $@.$(INPATH)

.ENDIF          # "$(SRS3NAME)"!=""

# unroll begin

.IF "$(SRS4NAME)"!=""
.IF "$(BUILDHIDS)"!=""
HID4FILES=$(foreach,i,$(SRC4FILES:f) $(SRS)$/$(i:s/.src/.hid/))
HIDSRS4PARTICLE=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(SRS))$/$(SRS4NAME)_srs.hid
$(HIDSRS4PARTICLE) : $(HID4FILES)
    @echo ------------------------------
    @echo Making: $@
    @-$(RM) $@
    $(TYPE) $(mktmp  $(subst,$/,/ $(HID4FILES))) | xargs -s 1000 cat > $@.$(ROUT).tmp
    @$(RENAME) $@.$(ROUT).tmp $@

ALLTAR : $(HIDSRS4PARTICLE)

.ENDIF # "$(BUILDHIDS)"!=""

$(MISC)$/$(PWD:f).$(SRS4NAME).dprr: $(SRC4FILES) $(HIDSRS4PARTICLE) $(HID4FILES)
    @echo ------------------------------
    @echo Making: $@
    @@-$(RM) $(MISC)$/$(PWD:f).$(SRS4NAME).dprr
    $(RSC) $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) -I$(INCLOCPRJ)  -I$(INCLOCAL) -I$(INC) -I$(INCCOM) $(RSCDEFS) $(RSCUPDVERDEF) -fp={$(SRS)$/$(SRS4NAME).srs} -fo=$@ $(SRC4FILES)

.IF "$(WITH_LANG)"!=""
$(foreach,i,$(SRC4FILES) $(COMMONMISC)$/$(TARGET)$/$i) : $$(@:f) localize.sdf 
    -$(MKDIR) $(@:d)
    -$(RM) $@
    $(TRANSEX) -p $(PRJNAME) -i $(@:f) -o $(@).$(INPATH) -m localize.sdf -l all
    $(RENAME) $@.$(INPATH) $@
    -$(RM) $@.$(INPATH)

$(SRS)$/$(SRS4NAME).srs: $(foreach,i,$(SRC4FILES) $(COMMONMISC)$/$(TARGET)$/$i)
.ELSE			# "$(WITH_LANG)"!=""
$(SRS)$/$(SRS4NAME).srs: $(SRC4FILES)
.ENDIF			# "$(WITH_LANG)"!=""
    @echo ------------------------------
    @echo Making: $@
    $(RSC) -presponse @$(mktmp \
        $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) \
        $(INCLUDE) $(RSCDEFS) $(RSCUPDVERDEF) \
        -fp=$@.$(INPATH) \
        $< \
    )
    -$(RM) $@
    $(RENAME) $@.$(INPATH) $@
    -$(RM) $@.$(INPATH)

.ENDIF          # "$(SRS4NAME)"!=""

# unroll begin

.IF "$(SRS5NAME)"!=""
.IF "$(BUILDHIDS)"!=""
HID5FILES=$(foreach,i,$(SRC5FILES:f) $(SRS)$/$(i:s/.src/.hid/))
HIDSRS5PARTICLE=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(SRS))$/$(SRS5NAME)_srs.hid
$(HIDSRS5PARTICLE) : $(HID5FILES)
    @echo ------------------------------
    @echo Making: $@
    @-$(RM) $@
    $(TYPE) $(mktmp  $(subst,$/,/ $(HID5FILES))) | xargs -s 1000 cat > $@.$(ROUT).tmp
    @$(RENAME) $@.$(ROUT).tmp $@

ALLTAR : $(HIDSRS5PARTICLE)

.ENDIF # "$(BUILDHIDS)"!=""

$(MISC)$/$(PWD:f).$(SRS5NAME).dprr: $(SRC5FILES) $(HIDSRS5PARTICLE) $(HID5FILES)
    @echo ------------------------------
    @echo Making: $@
    @@-$(RM) $(MISC)$/$(PWD:f).$(SRS5NAME).dprr
    $(RSC) $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) -I$(INCLOCPRJ)  -I$(INCLOCAL) -I$(INC) -I$(INCCOM) $(RSCDEFS) $(RSCUPDVERDEF) -fp={$(SRS)$/$(SRS5NAME).srs} -fo=$@ $(SRC5FILES)

.IF "$(WITH_LANG)"!=""
$(foreach,i,$(SRC5FILES) $(COMMONMISC)$/$(TARGET)$/$i) : $$(@:f) localize.sdf 
    -$(MKDIR) $(@:d)
    -$(RM) $@
    $(TRANSEX) -p $(PRJNAME) -i $(@:f) -o $(@).$(INPATH) -m localize.sdf -l all
    $(RENAME) $@.$(INPATH) $@
    -$(RM) $@.$(INPATH)

$(SRS)$/$(SRS5NAME).srs: $(foreach,i,$(SRC5FILES) $(COMMONMISC)$/$(TARGET)$/$i)
.ELSE			# "$(WITH_LANG)"!=""
$(SRS)$/$(SRS5NAME).srs: $(SRC5FILES)
.ENDIF			# "$(WITH_LANG)"!=""
    @echo ------------------------------
    @echo Making: $@
    $(RSC) -presponse @$(mktmp \
        $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) \
        $(INCLUDE) $(RSCDEFS) $(RSCUPDVERDEF) \
        -fp=$@.$(INPATH) \
        $< \
    )
    -$(RM) $@
    $(RENAME) $@.$(INPATH) $@
    -$(RM) $@.$(INPATH)

.ENDIF          # "$(SRS5NAME)"!=""

# unroll begin

.IF "$(SRS6NAME)"!=""
.IF "$(BUILDHIDS)"!=""
HID6FILES=$(foreach,i,$(SRC6FILES:f) $(SRS)$/$(i:s/.src/.hid/))
HIDSRS6PARTICLE=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(SRS))$/$(SRS6NAME)_srs.hid
$(HIDSRS6PARTICLE) : $(HID6FILES)
    @echo ------------------------------
    @echo Making: $@
    @-$(RM) $@
    $(TYPE) $(mktmp  $(subst,$/,/ $(HID6FILES))) | xargs -s 1000 cat > $@.$(ROUT).tmp
    @$(RENAME) $@.$(ROUT).tmp $@

ALLTAR : $(HIDSRS6PARTICLE)

.ENDIF # "$(BUILDHIDS)"!=""

$(MISC)$/$(PWD:f).$(SRS6NAME).dprr: $(SRC6FILES) $(HIDSRS6PARTICLE) $(HID6FILES)
    @echo ------------------------------
    @echo Making: $@
    @@-$(RM) $(MISC)$/$(PWD:f).$(SRS6NAME).dprr
    $(RSC) $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) -I$(INCLOCPRJ)  -I$(INCLOCAL) -I$(INC) -I$(INCCOM) $(RSCDEFS) $(RSCUPDVERDEF) -fp={$(SRS)$/$(SRS6NAME).srs} -fo=$@ $(SRC6FILES)

.IF "$(WITH_LANG)"!=""
$(foreach,i,$(SRC6FILES) $(COMMONMISC)$/$(TARGET)$/$i) : $$(@:f) localize.sdf 
    -$(MKDIR) $(@:d)
    -$(RM) $@
    $(TRANSEX) -p $(PRJNAME) -i $(@:f) -o $(@).$(INPATH) -m localize.sdf -l all
    $(RENAME) $@.$(INPATH) $@
    -$(RM) $@.$(INPATH)

$(SRS)$/$(SRS6NAME).srs: $(foreach,i,$(SRC6FILES) $(COMMONMISC)$/$(TARGET)$/$i)
.ELSE			# "$(WITH_LANG)"!=""
$(SRS)$/$(SRS6NAME).srs: $(SRC6FILES)
.ENDIF			# "$(WITH_LANG)"!=""
    @echo ------------------------------
    @echo Making: $@
    $(RSC) -presponse @$(mktmp \
        $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) \
        $(INCLUDE) $(RSCDEFS) $(RSCUPDVERDEF) \
        -fp=$@.$(INPATH) \
        $< \
    )
    -$(RM) $@
    $(RENAME) $@.$(INPATH) $@
    -$(RM) $@.$(INPATH)

.ENDIF          # "$(SRS6NAME)"!=""

# unroll begin

.IF "$(SRS7NAME)"!=""
.IF "$(BUILDHIDS)"!=""
HID7FILES=$(foreach,i,$(SRC7FILES:f) $(SRS)$/$(i:s/.src/.hid/))
HIDSRS7PARTICLE=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(SRS))$/$(SRS7NAME)_srs.hid
$(HIDSRS7PARTICLE) : $(HID7FILES)
    @echo ------------------------------
    @echo Making: $@
    @-$(RM) $@
    $(TYPE) $(mktmp  $(subst,$/,/ $(HID7FILES))) | xargs -s 1000 cat > $@.$(ROUT).tmp
    @$(RENAME) $@.$(ROUT).tmp $@

ALLTAR : $(HIDSRS7PARTICLE)

.ENDIF # "$(BUILDHIDS)"!=""

$(MISC)$/$(PWD:f).$(SRS7NAME).dprr: $(SRC7FILES) $(HIDSRS7PARTICLE) $(HID7FILES)
    @echo ------------------------------
    @echo Making: $@
    @@-$(RM) $(MISC)$/$(PWD:f).$(SRS7NAME).dprr
    $(RSC) $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) -I$(INCLOCPRJ)  -I$(INCLOCAL) -I$(INC) -I$(INCCOM) $(RSCDEFS) $(RSCUPDVERDEF) -fp={$(SRS)$/$(SRS7NAME).srs} -fo=$@ $(SRC7FILES)

.IF "$(WITH_LANG)"!=""
$(foreach,i,$(SRC7FILES) $(COMMONMISC)$/$(TARGET)$/$i) : $$(@:f) localize.sdf 
    -$(MKDIR) $(@:d)
    -$(RM) $@
    $(TRANSEX) -p $(PRJNAME) -i $(@:f) -o $(@).$(INPATH) -m localize.sdf -l all
    $(RENAME) $@.$(INPATH) $@
    -$(RM) $@.$(INPATH)

$(SRS)$/$(SRS7NAME).srs: $(foreach,i,$(SRC7FILES) $(COMMONMISC)$/$(TARGET)$/$i)
.ELSE			# "$(WITH_LANG)"!=""
$(SRS)$/$(SRS7NAME).srs: $(SRC7FILES)
.ENDIF			# "$(WITH_LANG)"!=""
    @echo ------------------------------
    @echo Making: $@
    $(RSC) -presponse @$(mktmp \
        $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) \
        $(INCLUDE) $(RSCDEFS) $(RSCUPDVERDEF) \
        -fp=$@.$(INPATH) \
        $< \
    )
    -$(RM) $@
    $(RENAME) $@.$(INPATH) $@
    -$(RM) $@.$(INPATH)

.ENDIF          # "$(SRS7NAME)"!=""

# unroll begin

.IF "$(SRS8NAME)"!=""
.IF "$(BUILDHIDS)"!=""
HID8FILES=$(foreach,i,$(SRC8FILES:f) $(SRS)$/$(i:s/.src/.hid/))
HIDSRS8PARTICLE=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(SRS))$/$(SRS8NAME)_srs.hid
$(HIDSRS8PARTICLE) : $(HID8FILES)
    @echo ------------------------------
    @echo Making: $@
    @-$(RM) $@
    $(TYPE) $(mktmp  $(subst,$/,/ $(HID8FILES))) | xargs -s 1000 cat > $@.$(ROUT).tmp
    @$(RENAME) $@.$(ROUT).tmp $@

ALLTAR : $(HIDSRS8PARTICLE)

.ENDIF # "$(BUILDHIDS)"!=""

$(MISC)$/$(PWD:f).$(SRS8NAME).dprr: $(SRC8FILES) $(HIDSRS8PARTICLE) $(HID8FILES)
    @echo ------------------------------
    @echo Making: $@
    @@-$(RM) $(MISC)$/$(PWD:f).$(SRS8NAME).dprr
    $(RSC) $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) -I$(INCLOCPRJ)  -I$(INCLOCAL) -I$(INC) -I$(INCCOM) $(RSCDEFS) $(RSCUPDVERDEF) -fp={$(SRS)$/$(SRS8NAME).srs} -fo=$@ $(SRC8FILES)

.IF "$(WITH_LANG)"!=""
$(foreach,i,$(SRC8FILES) $(COMMONMISC)$/$(TARGET)$/$i) : $$(@:f) localize.sdf 
    -$(MKDIR) $(@:d)
    -$(RM) $@
    $(TRANSEX) -p $(PRJNAME) -i $(@:f) -o $(@).$(INPATH) -m localize.sdf -l all
    $(RENAME) $@.$(INPATH) $@
    -$(RM) $@.$(INPATH)

$(SRS)$/$(SRS8NAME).srs: $(foreach,i,$(SRC8FILES) $(COMMONMISC)$/$(TARGET)$/$i)
.ELSE			# "$(WITH_LANG)"!=""
$(SRS)$/$(SRS8NAME).srs: $(SRC8FILES)
.ENDIF			# "$(WITH_LANG)"!=""
    @echo ------------------------------
    @echo Making: $@
    $(RSC) -presponse @$(mktmp \
        $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) \
        $(INCLUDE) $(RSCDEFS) $(RSCUPDVERDEF) \
        -fp=$@.$(INPATH) \
        $< \
    )
    -$(RM) $@
    $(RENAME) $@.$(INPATH) $@
    -$(RM) $@.$(INPATH)

.ENDIF          # "$(SRS8NAME)"!=""

# unroll begin

.IF "$(SRS9NAME)"!=""
.IF "$(BUILDHIDS)"!=""
HID9FILES=$(foreach,i,$(SRC9FILES:f) $(SRS)$/$(i:s/.src/.hid/))
HIDSRS9PARTICLE=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(SRS))$/$(SRS9NAME)_srs.hid
$(HIDSRS9PARTICLE) : $(HID9FILES)
    @echo ------------------------------
    @echo Making: $@
    @-$(RM) $@
    $(TYPE) $(mktmp  $(subst,$/,/ $(HID9FILES))) | xargs -s 1000 cat > $@.$(ROUT).tmp
    @$(RENAME) $@.$(ROUT).tmp $@

ALLTAR : $(HIDSRS9PARTICLE)

.ENDIF # "$(BUILDHIDS)"!=""

$(MISC)$/$(PWD:f).$(SRS9NAME).dprr: $(SRC9FILES) $(HIDSRS9PARTICLE) $(HID9FILES)
    @echo ------------------------------
    @echo Making: $@
    @@-$(RM) $(MISC)$/$(PWD:f).$(SRS9NAME).dprr
    $(RSC) $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) -I$(INCLOCPRJ)  -I$(INCLOCAL) -I$(INC) -I$(INCCOM) $(RSCDEFS) $(RSCUPDVERDEF) -fp={$(SRS)$/$(SRS9NAME).srs} -fo=$@ $(SRC9FILES)

.IF "$(WITH_LANG)"!=""
$(foreach,i,$(SRC9FILES) $(COMMONMISC)$/$(TARGET)$/$i) : $$(@:f) localize.sdf 
    -$(MKDIR) $(@:d)
    -$(RM) $@
    $(TRANSEX) -p $(PRJNAME) -i $(@:f) -o $(@).$(INPATH) -m localize.sdf -l all
    $(RENAME) $@.$(INPATH) $@
    -$(RM) $@.$(INPATH)

$(SRS)$/$(SRS9NAME).srs: $(foreach,i,$(SRC9FILES) $(COMMONMISC)$/$(TARGET)$/$i)
.ELSE			# "$(WITH_LANG)"!=""
$(SRS)$/$(SRS9NAME).srs: $(SRC9FILES)
.ENDIF			# "$(WITH_LANG)"!=""
    @echo ------------------------------
    @echo Making: $@
    $(RSC) -presponse @$(mktmp \
        $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) \
        $(INCLUDE) $(RSCDEFS) $(RSCUPDVERDEF) \
        -fp=$@.$(INPATH) \
        $< \
    )
    -$(RM) $@
    $(RENAME) $@.$(INPATH) $@
    -$(RM) $@.$(INPATH)

.ENDIF          # "$(SRS9NAME)"!=""

# unroll begin

.IF "$(SRS10NAME)"!=""
.IF "$(BUILDHIDS)"!=""
HID10FILES=$(foreach,i,$(SRC10FILES:f) $(SRS)$/$(i:s/.src/.hid/))
HIDSRS10PARTICLE=$(subst,$(OUTPATH),$(COMMON_OUTDIR) $(SRS))$/$(SRS10NAME)_srs.hid
$(HIDSRS10PARTICLE) : $(HID10FILES)
    @echo ------------------------------
    @echo Making: $@
    @-$(RM) $@
    $(TYPE) $(mktmp  $(subst,$/,/ $(HID10FILES))) | xargs -s 1000 cat > $@.$(ROUT).tmp
    @$(RENAME) $@.$(ROUT).tmp $@

ALLTAR : $(HIDSRS10PARTICLE)

.ENDIF # "$(BUILDHIDS)"!=""

$(MISC)$/$(PWD:f).$(SRS10NAME).dprr: $(SRC10FILES) $(HIDSRS10PARTICLE) $(HID10FILES)
    @echo ------------------------------
    @echo Making: $@
    @@-$(RM) $(MISC)$/$(PWD:f).$(SRS10NAME).dprr
    $(RSC) $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) -I$(INCLOCPRJ)  -I$(INCLOCAL) -I$(INC) -I$(INCCOM) $(RSCDEFS) $(RSCUPDVERDEF) -fp={$(SRS)$/$(SRS10NAME).srs} -fo=$@ $(SRC10FILES)

.IF "$(WITH_LANG)"!=""
$(foreach,i,$(SRC10FILES) $(COMMONMISC)$/$(TARGET)$/$i) : $$(@:f) localize.sdf 
    -$(MKDIR) $(@:d)
    -$(RM) $@
    $(TRANSEX) -p $(PRJNAME) -i $(@:f) -o $(@).$(INPATH) -m localize.sdf -l all
    $(RENAME) $@.$(INPATH) $@
    -$(RM) $@.$(INPATH)

$(SRS)$/$(SRS10NAME).srs: $(foreach,i,$(SRC10FILES) $(COMMONMISC)$/$(TARGET)$/$i)
.ELSE			# "$(WITH_LANG)"!=""
$(SRS)$/$(SRS10NAME).srs: $(SRC10FILES)
.ENDIF			# "$(WITH_LANG)"!=""
    @echo ------------------------------
    @echo Making: $@
    $(RSC) -presponse @$(mktmp \
        $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) \
        $(INCLUDE) $(RSCDEFS) $(RSCUPDVERDEF) \
        -fp=$@.$(INPATH) \
        $< \
    )
    -$(RM) $@
    $(RENAME) $@.$(INPATH) $@
    -$(RM) $@.$(INPATH)

.ENDIF          # "$(SRS10NAME)"!=""

