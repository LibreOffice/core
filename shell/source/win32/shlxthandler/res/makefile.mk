#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************


PRJ=..$/..$/..$/..

PRJNAME=shell
TARGET=shlxthdl

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

INCPRE+=$(MFC_INCLUDE)

# --- Files --------------------------------------------------------


RCFILES=$(RES)$/$(TARGET).rc

ULFFILES=$(TARGET).ulf

.IF "$(WITH_LANG)"!=""
ULFDIR:=$(COMMONMISC)$/$(TARGET)
.ELSE			# "$(WITH_LANG)"!=""
ULFDIR:=.
.ENDIF			# "$(WITH_LANG)"!=""

# --- Targets ------------------------------------------------------

.IF "$(BUILD_X64)"!=""
$(RES)$/x64$/$(TARGET).res : $(RES)$/$(TARGET).res
    -$(MKDIR) $(RES)$/x64
    @@cp $< $@
.ENDIF # "$(BUILD_X64)"!=""

.INCLUDE :	set_wntx64.mk
.INCLUDE :	target.mk
.INCLUDE :	tg_wntx64.mk



# Generate the native Windows resource file
# using lngconvex.exe 

$(RCFILES) : $(ULFDIR)$/shlxthdl.ulf makefile.mk rcfooter.txt rcheader.txt rctmpl.txt
    $(subst,$(SOLARBINDIR)$/lngconvex,$(BIN)$/lngconvex $(LNGCONVEX)) -ulf $(ULFDIR)$/shlxthdl.ulf -rc $(RES)$/shlxthdl.rc -rct rctmpl.txt -rch rcheader.txt -rcf rcfooter.txt
    
