#*************************************************************************
#
#   $RCSfile: tg_srs.mk,v $
#
#   $Revision: 1.9 $
#
#   last change: $Author: hjs $ $Date: 2003-08-18 14:49:21 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************



# und so mit response file
#.IF "$(OS)" == "DOS"
# fuer DOSSTCIX nicht \" sondern "
.IF "$(GUI)" == "MAC"
RSCUPDVERMAC+=-DUPDVER=¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶"$(RSCUPDVER)¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶"
.ELSE
RSCUPDVERMAC+=-DUPDVER="$(RSCUPDVER)"
.ENDIF
#.ELSE
#RSCUPDVERMAC=-DUPDVER=\"$(RSCUPDVER)\"
#.ENDIF

# und so mit response file
.IF "$(GUI)" == "MAC"
RSCUPDVERMAC+=-DUPDVER=¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶"$(RSCUPDVER)¶¶¶¶¶¶¶¶¶¶¶¶¶¶¶"
.ELSE
RSCUPDVERMAC+=-DUPDVER="$(RSCUPDVER)"
.ENDIF

.IF "$(lintit)"==""
.IF "$(GUI)"=="WNT"
.IF "$(CPU)"=="I"
.IF "$(no_hids)$(NO_HIDS)"==""
.IF "$(USE_SHELL)"=="4nt"
.IF "$(SRCFILES)"!=""
HIDFILES=$(foreach,i,$(SRCFILES:f) $(SRS)$/$(i:s/.src/.hid/))
HIDSRSPARTICLE=$(MISC)$/$(TARGET)_srs.hid
$(HIDSRSPARTICLE) : $(HIDFILES)
    @echo ------------------------------
    @echo Making: $@
    @+if exist $(HIDSRSPARTICLE) rm $(HIDSRSPARTICLE)
    +$(TYPE) $(HIDFILES) > $(HIDSRSPARTICLE)
ALLTAR : $(HIDSRSPARTICLE)
.ENDIF
.ENDIF                  # "$(USE_SHELL)"=="4nt"
.ENDIF
.ENDIF
.ENDIF
.ENDIF


.IF "$(SRCTARGET)"!=""
$(SRCTARGET) : $(SRCFILES)
    @echo ------------------------------
    @echo Making: $@
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

.IF "$(lintit)"==""
.IF "$(GUI)"=="WNT"
.IF "$(CPU)"=="I"
.IF "$(no_hids)$(NO_HIDS)"==""
.IF "$(USE_SHELL)"=="4nt"
HID$(TNR)FILES=$(foreach,i,$(SRC$(TNR)FILES:f) $(SRS)$/$(i:s/.src/.hid/))
HIDSRS$(TNR)PARTICLE=$(MISC)$/$(SRS$(TNR)NAME)_srs.hid
$(HIDSRS$(TNR)PARTICLE) : $(HID$(TNR)FILES)
        @echo ------------------------------
        @echo Making: $@
        @+if exist $(HIDSRS$(TNR)PARTICLE) rm $(HIDSRS$(TNR)PARTICLE)
        +$(TYPE) $(HID$(TNR)FILES) > $(HIDSRS$(TNR)PARTICLE)

SRS$(TNR) : $(HIDSRS$(TNR)PARTICLE) $(HID$(TNR)FILES)
.ENDIF                  # "$(USE_SHELL)"=="4nt"
.ENDIF
.ENDIF
.ENDIF
.ENDIF

$(SRS)$/$(SRS$(TNR)NAME).srs: $(SRC$(TNR)FILES) $(HIDSRS$(TNR)PARTICLE) $(HID$(TNR)FILES)
    @echo ------------------------------
    @echo Making: $@
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
.ENDIF		# "$(MULTI_SRS_FLAG)" == ""

