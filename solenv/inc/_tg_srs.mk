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
    +$(RSC) $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) -I$(INCLOCPRJ)  -I$(INCLOCAL) -I$(INC) -I$(INCCOM) $(RSCDEFS) $(RSCUPDVERDEF) -fp$@ $(SRC1FILES)

$(SRS)$/$(SRS1NAME).srs: $(SRC1FILES)
    @echo ------------------------------
    @echo Making: $@
    echo $(SRC1FILES)
    +$(RSC) -presponse @$(mktmp \
        $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) \
        $(INCLUDE) $(RSCDEFS) $(RSCUPDVERDEF) \
        -fp$@ \
        $(SRC1FILES) \
    )

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
    +$(RSC) $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) -I$(INCLOCPRJ)  -I$(INCLOCAL) -I$(INC) -I$(INCCOM) $(RSCDEFS) $(RSCUPDVERDEF) -fp$@ $(SRC2FILES)

$(SRS)$/$(SRS2NAME).srs: $(SRC2FILES)
    @echo ------------------------------
    @echo Making: $@
    echo $(SRC2FILES)
    +$(RSC) -presponse @$(mktmp \
        $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) \
        $(INCLUDE) $(RSCDEFS) $(RSCUPDVERDEF) \
        -fp$@ \
        $(SRC2FILES) \
    )

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
    +$(RSC) $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) -I$(INCLOCPRJ)  -I$(INCLOCAL) -I$(INC) -I$(INCCOM) $(RSCDEFS) $(RSCUPDVERDEF) -fp$@ $(SRC3FILES)

$(SRS)$/$(SRS3NAME).srs: $(SRC3FILES)
    @echo ------------------------------
    @echo Making: $@
    echo $(SRC3FILES)
    +$(RSC) -presponse @$(mktmp \
        $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) \
        $(INCLUDE) $(RSCDEFS) $(RSCUPDVERDEF) \
        -fp$@ \
        $(SRC3FILES) \
    )

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
    +$(RSC) $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) -I$(INCLOCPRJ)  -I$(INCLOCAL) -I$(INC) -I$(INCCOM) $(RSCDEFS) $(RSCUPDVERDEF) -fp$@ $(SRC4FILES)

$(SRS)$/$(SRS4NAME).srs: $(SRC4FILES)
    @echo ------------------------------
    @echo Making: $@
    echo $(SRC4FILES)
    +$(RSC) -presponse @$(mktmp \
        $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) \
        $(INCLUDE) $(RSCDEFS) $(RSCUPDVERDEF) \
        -fp$@ \
        $(SRC4FILES) \
    )

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
    +$(RSC) $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) -I$(INCLOCPRJ)  -I$(INCLOCAL) -I$(INC) -I$(INCCOM) $(RSCDEFS) $(RSCUPDVERDEF) -fp$@ $(SRC5FILES)

$(SRS)$/$(SRS5NAME).srs: $(SRC5FILES)
    @echo ------------------------------
    @echo Making: $@
    echo $(SRC5FILES)
    +$(RSC) -presponse @$(mktmp \
        $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) \
        $(INCLUDE) $(RSCDEFS) $(RSCUPDVERDEF) \
        -fp$@ \
        $(SRC5FILES) \
    )

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
    +$(RSC) $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) -I$(INCLOCPRJ)  -I$(INCLOCAL) -I$(INC) -I$(INCCOM) $(RSCDEFS) $(RSCUPDVERDEF) -fp$@ $(SRC6FILES)

$(SRS)$/$(SRS6NAME).srs: $(SRC6FILES)
    @echo ------------------------------
    @echo Making: $@
    echo $(SRC6FILES)
    +$(RSC) -presponse @$(mktmp \
        $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) \
        $(INCLUDE) $(RSCDEFS) $(RSCUPDVERDEF) \
        -fp$@ \
        $(SRC6FILES) \
    )

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
    +$(RSC) $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) -I$(INCLOCPRJ)  -I$(INCLOCAL) -I$(INC) -I$(INCCOM) $(RSCDEFS) $(RSCUPDVERDEF) -fp$@ $(SRC7FILES)

$(SRS)$/$(SRS7NAME).srs: $(SRC7FILES)
    @echo ------------------------------
    @echo Making: $@
    echo $(SRC7FILES)
    +$(RSC) -presponse @$(mktmp \
        $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) \
        $(INCLUDE) $(RSCDEFS) $(RSCUPDVERDEF) \
        -fp$@ \
        $(SRC7FILES) \
    )

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
    +$(RSC) $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) -I$(INCLOCPRJ)  -I$(INCLOCAL) -I$(INC) -I$(INCCOM) $(RSCDEFS) $(RSCUPDVERDEF) -fp$@ $(SRC8FILES)

$(SRS)$/$(SRS8NAME).srs: $(SRC8FILES)
    @echo ------------------------------
    @echo Making: $@
    echo $(SRC8FILES)
    +$(RSC) -presponse @$(mktmp \
        $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) \
        $(INCLUDE) $(RSCDEFS) $(RSCUPDVERDEF) \
        -fp$@ \
        $(SRC8FILES) \
    )

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
    +$(RSC) $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) -I$(INCLOCPRJ)  -I$(INCLOCAL) -I$(INC) -I$(INCCOM) $(RSCDEFS) $(RSCUPDVERDEF) -fp$@ $(SRC9FILES)

$(SRS)$/$(SRS9NAME).srs: $(SRC9FILES)
    @echo ------------------------------
    @echo Making: $@
    echo $(SRC9FILES)
    +$(RSC) -presponse @$(mktmp \
        $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) \
        $(INCLUDE) $(RSCDEFS) $(RSCUPDVERDEF) \
        -fp$@ \
        $(SRC9FILES) \
    )

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
    +$(RSC) $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) -I$(INCLOCPRJ)  -I$(INCLOCAL) -I$(INC) -I$(INCCOM) $(RSCDEFS) $(RSCUPDVERDEF) -fp$@ $(SRC10FILES)

$(SRS)$/$(SRS10NAME).srs: $(SRC10FILES)
    @echo ------------------------------
    @echo Making: $@
    echo $(SRC10FILES)
    +$(RSC) -presponse @$(mktmp \
        $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) \
        $(INCLUDE) $(RSCDEFS) $(RSCUPDVERDEF) \
        -fp$@ \
        $(SRC10FILES) \
    )

.ENDIF          # "$(SRS10NAME)"!=""

