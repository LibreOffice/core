#*************************************************************************
#*
#*    $Workfile:   tg_scp.mk  $
#*
#*    Ersterstellung    XX  TT.MM.JJ
#*    Letzte Aenderung  $Author: obo $ $Date: 2001-04-06 14:27:08 $
#*    $Revision: 1.4 $
#*
#*    $Logfile:   T:/solar/inc/tg_scp.mkv  $
#*
#*    Copyright (c) 1990-1997, STAR DIVISION
#*
#*************************************************************************

.IF "$(MULTI_SCP_FLAG)" == ""
$(SCP1TARGETN) .NULL : SCP1

$(SCP2TARGETN) .NULL : SCP2

$(SCP3TARGETN) .NULL : SCP3

$(SCP4TARGETN) .NULL : SCP4

$(SCP5TARGETN) .NULL : SCP5

$(SCP6TARGETN) .NULL : SCP6

$(SCP7TARGETN) .NULL : SCP7

$(SCP8TARGETN) .NULL : SCP8

$(SCP9TARGETN) .NULL : SCP9
.ENDIF

.IF "$(MULTI_SCP_FLAG)"==""
SCP1 SCP2 SCP3 SCP4 SCP5 SCP6 SCP7 SCP8 SCP9:
.IF "$(GUI)" != "MAC"
.IF "$(GUI)" == "UNX"
    @echo $(SHELL) AUSE $(COMSPEC) MARTIN $(SHELLFLAGS)
.ENDIF
    +@dmake $(SCP$(TNR)TARGETN) MULTI_SCP_FLAG=true TNR:=$(TNR) $(MFLAGS) $(CALLMACROS)
.ELSE
    @dmake "$(SCP$(TNR)TARGETN)" MULTI_SCP_FLAG=true TNR:=$(TNR) $(MFLAGS) $(CALLMACROS)
.ENDIF
.ELSE


#######################################################
# Anweisungen fuer das Linken
# unroll begin

.IF "$(SCP$(TNR)TARGETN)"!=""

# try to get missing parfiles
$(PAR)$/%.par : $(SOLARPARDIR)$/%.par
    +$(COPY) $< $@

LOCALSCP$(TNR)FILES=$(foreach,i,$(SCP$(TNR)FILES) $(foreach,j,$(SCP$(TNR)LINK_PRODUCT_TYPE) $(PAR)$/$j$/$i ))

$(SCP$(TNR)TARGETN): $(LOCALSCP$(TNR)FILES)
    @echo ------------------------------
    @echo Making: $@
    scplink $(SCPLINKFLAGS) @$(mktmp $(foreach,i,$(SCP$(TNR)FILES) $(subst,$(@:d:d:d), $(@:d:d))$/$(i:+","))) -o $@
    $(SCP_CHECK_TOOL) $@
.ENDIF

# Anweisungen fuer das Linken
# unroll end
#######################################################

.ENDIF		# "$(MULTI_SCP_FLAG)"==""
