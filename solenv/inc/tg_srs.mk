


# so sah es ueber Kommandozeile aus:
#  krank aber wahr, sonst steht da hinterher
#  Text = 1234	oder  Text = \1234	oder sonstiger Mist
# .IF "$(OS)" == "DOS"
# RSCUPDVERMAC=-DUPDVER="$(RSCUPDVER)"
# .ENDIF
# .IF "$(OS)" == "OS2"
# RSCUPDVERMAC=-DUPDVER=\\"$(RSCUPDVER)\\"
# .ENDIF
# .IF "$(OS)" == "WNT"
# RSCUPDVERMAC=-DUPDVER=\\\"$(RSCUPDVER)\\\"
# .ENDIF

# und so mit response file
#.IF "$(OS)" == "DOS"
# fuer DOSSTCIX nicht \" sondern "
.IF "$(GUI)" == "MAC"
RSCUPDVERMAC=-DUPDVER=¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶"$(RSCUPDVER)¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶"
.ELSE
RSCUPDVERMAC=-DUPDVER="$(RSCUPDVER)"
.ENDIF
#.ELSE
#RSCUPDVERMAC=-DUPDVER=\"$(RSCUPDVER)\"
#.ENDIF

# und so mit response file
.IF "$(GUI)" == "MAC"
RSCUPDVERMAC=-DUPDVER=¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶"$(RSCUPDVER)¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶"
.ELSE
RSCUPDVERMAC=-DUPDVER="$(RSCUPDVER)"
.ENDIF

.IF "$(SRCTARGET)"!=""
$(SRCTARGET) : $(SRCFILES)
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="WNT"
.IF "$(CPU)"=="I"
.IF "$(UPDATER)"=="YES"
.IF "$(BUILD_SOSL)"==""
.IF "$(no_hids)$(NO_HIDS)"==""
    @+type $(mktmp $(SRCFILES:+"\n")) > $(TMP)$/$(TARGET).tra
    @+-$(COPY) $(TMP)$/$(TARGET).tra $(TMP)$/$(TARGET).art
    +type $(TMP)$/$(TARGET).art | $(SORT) -u  > $(TMP)$/$(TARGET).tra
    +call resp.bat r:\bat\mhids.bat @$(TMP)$/$(TARGET).tra $(SRS) $(PRJNAME) $(CDEFS) $(INCLUDE)
    @+-del $(TMP)$/$(TARGET).tra
    @+-del $(TMP)$/$(TARGET).art
.ENDIF
.ENDIF
.ENDIF
.ENDIF
.ENDIF			 # "$(GUI)"=="WNT"
.IF "$(make_srs_deps)" != ""
    +$(RSC) $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) -I$(INCLOCPRJ)  -I$(INCLOCAL) -I$(INC) -I$(INCCOM) $(RSCDEFS) $(RSCUPDVERMAC) -fp$@ $(SRCFILES)
.ELSE
    $(RSC) -presponse @$(mktmp $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) \
        $(INCLUDE) $(RSCDEFS) $(RSCUPDVERMAC) \
        -fp$@ \
        $(SRCFILES) \
    )
.ENDIF
.ENDIF



.IF "$(MULTI_SRC_FLAG)"==""

SRC1 ?= TNR!:=1
$(SRC1TARGET) .NULL : SRC1

SRC2 ?= TNR!:=2
$(SRC2TARGET) .NULL : SRC2

SRC3 ?= TNR!:=3
$(SRC3TARGET) .NULL : SRC3

SRC4 ?= TNR!:=4
$(SRC4TARGET) .NULL : SRC4

SRC5 ?= TNR!:=5
$(SRC5TARGET) .NULL : SRC5

SRC6 ?= TNR!:=6
$(SRC6TARGET) .NULL : SRC6

SRC7 ?= TNR!:=7
$(SRC7TARGET) .NULL : SRC7

SRC8 ?= TNR!:=8
$(SRC8TARGET) .NULL : SRC8

SRC9 ?= TNR!:=9
$(SRC9TARGET) .NULL : SRC9

.ENDIF

# -------
# - SRC1 -
# -------

.IF "$(MULTI_SRC_FLAG)" == ""
SRC1 SRC2 SRC3 SRC4 SRC5 SRC6 SRC7 SRC8 SRC9 :
.IF "$(GUI)" != "MAC"
    @+dmake $(SRS)$/$(SRS$(TNR)NAME).srs MULTI_SRC_FLAG=true TNR:=$(TNR) $(MFLAGS) $(CALLMACROS)
.ELSE
    @+dmake "$(SRS)$/$(SRS$(TNR)NAME).srs" MULTI_SRC_FLAG=true TNR:=$(TNR) $(MFLAGS) $(CALLMACROS)
.ENDIF
.ELSE
$(SRS)$/$(SRS$(TNR)NAME).srs: $(SRC$(TNR)FILES)
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)"=="WNT"
.IF "$(CPU)"=="I"
.IF "$(UPDATER)"=="YES"
.IF "$(BUILD_SOSL)"==""
.IF "$(no_hids)$(NO_HIDS)"==""
    @+type $(mktmp $(SRC$(TNR)FILES:+"\n")) > $(TMP)$/$(TARGET).tra
    @+-$(COPY) $(TMP)$/$(TARGET).tra $(TMP)$/$(TARGET).art
    +type $(TMP)$/$(TARGET).art | $(SORT) -u > $(TMP)$/$(TARGET).tra
    +call resp.bat r:\bat\mhids.bat @$(TMP)$/$(TARGET).tra $(SRS) $(PRJNAME) $(CDEFS) $(INCLUDE)
    @+del $(TMP)$/$(TARGET).art
    @+del $(TMP)$/$(TARGET).tra
.ENDIF
.ENDIF
.ENDIF
.ENDIF
.ENDIF
.IF "$(make_srs_deps)" != ""
    +$(RSC) $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) -I$(INCLOCPRJ)  -I$(INCLOCAL) -I$(INC) -I$(INCCOM) $(RSCDEFS) $(RSCUPDVERMAC) -fp$@ $(SRC$(TNR)FILES)
.ELSE
    echo $(SRC$(TNR)FILES)
    +$(RSC) -presponse @$(mktmp \
        $(SRSDEFAULT) $(RSC_SRS_CHARSET) $(RSCFLAGS) -I$(RSCEXTINC) \
        $(INCLUDE) $(RSCDEFS) $(RSCUPDVERMAC) \
        -fp$@ \
        $(SRC$(TNR)FILES) \
    )
.ENDIF
.ENDIF

.IF "$(MULTI_SRS_FLAG)"==""

SRS1 ?= TNR!:=1
$(SRS1TARGET) .NULL : SRS1

SRS2 ?= TNR!:=2
$(SRS2TARGET) .NULL : SRS2

SRS3 ?= TNR!:=3
$(SRS3TARGET) .NULL : SRS3

SRS4 ?= TNR!:=4
$(SRS4TARGET) .NULL : SRS4

SRS5 ?= TNR!:=5
$(SRS5TARGET) .NULL : SRS5

SRS6 ?= TNR!:=6
$(SRS6TARGET) .NULL : SRS6

SRS7 ?= TNR!:=7
$(SRS7TARGET) .NULL : SRS7

SRS8 ?= TNR!:=8
$(SRS8TARGET) .NULL : SRS8

SRS9 ?= TNR!:=9
$(SRS9TARGET) .NULL : SRS9

.ENDIF

# -------
# - SRS1 -
# -------

.IF "$(MULTI_SRS_FLAG)" == ""
SRS1 SRS2 SRS3 SRS4 SRS5 SRS6 SRS7 SRS8 SRS9:
.IF "$(GUI)" != "MAC"
    @+dmake $(SRS$(TNR)TARGET) MULTI_SRS_FLAG=true TNR:=$(TNR) $(MFLAGS) $(CALLMACROS)
.ELSE
    @+dmake "$(SRS$(TNR)TARGET)" MULTI_SRS_FLAG=true TNR:=$(TNR) $(MFLAGS) $(CALLMACROS)
.ENDIF	# "$(GUI)" != "MAC"
.ELSE
.IF "$(VCL)" == ""
.ELSE
$(SRS$(TNR)TARGET): $(SRS$(TNR)FILES) $(SRS$(TNR)DEPN)
    @echo ------------------------------
    @echo Making: $@
.IF "$(GUI)" != "MAC"
.IF "$(GUI)" == "UNX"
    +$(RSC) -r $(RSC_RES_CHARSET) -fs$(SRS$(TNR)TARGET)	-I$(INPATH)$/res \
    -I$(RES) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(RSCLOCINC) -I$(RSCGLOINC) $(SOLARINC) \
    $(RSC$(TNR)HEADER) $(SRS$(TNR)FILES)
.ELSE		# "$(GUI)" == "UNX"
    +$(RSC) -r $(RSC_RES_CHARSET) -fs$(SRS$(TNR)TARGET) 					\
    -I$(INPATH)$/res -I$(RES) \
    -I$(PRJ)$/$(GUIBASE)$/res -I$(PRJ)$/res -I$(RSCLOCINC) -I$(RSCGLOINC) -I$(INCLUDE) \
    $(RSC$(TNR)HEADER) $(SRS$(TNR)FILES)
.ENDIF		# "$(GUI)" == "UNX"
.ELSE		# "$(GUI)" != "MAC"
    +$(RSC) -r $(RSC_RES_CHARSET) -fs$(SRS$(TNR)TARGET)  -fo$(RES)$/$(RES$(TNR)TARGET).res -I$(SOLARENV)$/res -I$(INPATH)$/res -I$(RSCEXTINC) -I"$(INCLUDE:s/inc/inc:/)" -I$(SOLARINCDIR)$/ $(RSCDEFS) $(RSCUPDVERMAC)  $(RSC$(TNR)HEADER) $(SRS$(TNR)FILES)
.ENDIF		# "$(GUI)" != "MAC"
.ENDIF		# "$(VCL)" == ""
.ENDIF		# "$(MULTI_SRS_FLAG)" == ""

