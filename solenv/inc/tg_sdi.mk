

# ------------------
# - SDITARGETS     -
# ------------------

.IF "$(MULTI_SDI_FLAG)" == ""

$(SDITARGET) .NULL	: SDI0

$(SDI1TARGET) .NULL : SDI1

$(SDI2TARGET) .NULL : SDI2

$(SDI3TARGET) .NULL : SDI3

$(SDI4TARGET) .NULL : SDI4

$(SDI5TARGET) .NULL : SDI5

.ENDIF


.IF "$(MULTI_SDI_FLAG)" == ""
SDI0 SDI1 SDI2 SDI3 SDI4 SDI5 :
.IF "$(GUI)" != "MAC"
    @+dmake $(SDI$(TNR)TARGET) MULTI_SDI_FLAG=true TNR:=$(TNR) $(MFLAGS) $(CALLMACROS)
.ELSE
    @+dmake "$(SDI$(TNR)TARGET)" MULTI_SDI_FLAG=true TNR:=$(TNR) $(MFLAGS) $(CALLMACROS)
.ENDIF
.ELSE			# "$(MULTI_SDI_FLAG)" == ""

.IF "$(make_srs_deps)"==""
dttt:
    echo $(TNR)------------

$(SDI$(TNR)TARGET): $(SVSDI$(TNR)DEPEND)
    @echo ------------------------------
    @echo Making: $@
    @+-$(RM) $@
    $(SVIDL) @$(mktmp \
    -fs$(INCCOMX)$/$(SDI$(TNR)NAME).hxx	\
    -fd$(INCCOMX)$/$(SDI$(TNR)NAME).ilb	\
    -fm$(MISCX)$/$(SDI$(TNR)NAME).don	\
    -fl$(MISC)$/$(SDI$(TNR)NAME).lst         \
    -fx$(SDI$(TNR)EXPORT).sdi		\
    -fy$(MISCX)$/xx$(PRJNAME).csv		\
    -fz$(MISCX)$/$(SDI$(TNR)NAME).sid	\
    $(SDI$(TNR)NAME).sdi -I$(MISCX) -I$(SVSDIINC) -I$(SOLARINCXDIR) -I$(INC) -I$(INCLUDE) -I$(SOLARVER)$/$(UPD)$/$(INPATH)$/inc )

.IF "$(NO_SRS_PATCH)"==""
        @+$(COPY) $(SRSX)$/$(SDI$(TNR)NAME).srs $(TEMP)$/$(SDI$(TNR)NAME).srs
.IF "$(GUI)" != "MAC"
        @+-$(RM) $(SRSX)$/$(SDI$(TNR)NAME).srs
.ENDIF
.IF "$(GUI)" == "OS2"
        @+gawk -f r:\bat\srspatch.awk $(TEMP)$/$(SDI$(TNR)NAME).srs > $(SRSX)$/$(SDI$(TNR)NAME).srs
        @+$(COPY) $(SRSX)$/$(SDI$(TNR)NAME).srs $(TEMP)$/$(SDI$(TNR)NAME).srs
        @+-$(RM) $(SRSX)$/$(SDI$(TNR)NAME).srs
        @+sed -f r:\bat\sichern.sed $(TEMP)$/$(SDI$(TNR)NAME).srs > $(SRSX)$/$(SDI$(TNR)NAME).srs
.ELSE
.IF "$(GUI)" == "UNX"
        @+$(AWK) -f $(SOLARROOT)$/scripts$/unx$/srspatch.awk $(TEMP)$/$(SDI$(TNR)NAME).srs > $(SRSX)$/$(SDI$(TNR)NAME).srs
.ELSE
        @+awk -f r:\bat\srspatch.awk $(TEMP)$/$(SDI$(TNR)NAME).srs > $(SRSX)$/$(SDI$(TNR)NAME).srs
.ENDIF
.ENDIF
        @+-$(RM) $(TEMP)$/$(SDI$(TNR)NAME).srs
.ENDIF			# "$(NO_SRS_PATCH)"!=""

.ELSE			# "$(make_srs_deps)"==""
$(SDI$(TNR)TARGET): $(SVSDI$(TNR)DEPEND)
    @+echo jetzt nicht...
.ENDIF			# "$(make_srs_deps)"==""
.ENDIF			# "$(MULTI_SDI_FLAG)" == ""

