#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

PRJ=..$/..

PRJNAME=crashrep
TARGET=soreport
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE
TARGETTYPE=GUI

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Resources ----------------------------------------------------

RCFILES=$(RES)$/$(TARGET).rc

# --- Files --------------------------------------------------------

OBJFILES=\
    $(OBJ)$/soreport.obj\
    $(OBJ)$/base64.obj
    
.IF "$(WITH_LANG)"!=""
ULFDIR:=$(COMMONMISC)$/crash_res
.ELSE			# "$(WITH_LANG)"!=""
ULFDIR:=..$/all
.ENDIF			# "$(WITH_LANG)"!=""
LNGFILES=$(ULFDIR)$/crashrep.ulf

APP1OBJS=$(OBJFILES)

APP1NOSAL=TRUE
                        
APP1TARGET=$(TARGET)
APP1RPATH=OOO

STDLIB1=\
    $(SALLIB)\
    $(GDI32LIB)\
    $(COMCTL32LIB)\
    $(COMDLG32LIB)\
    $(ADVAPI32LIB)\
    $(WS2_32LIB)\
    $(SHELL32LIB)\
    $(DBGHELPLIB)\
    psapi.lib

APP1NOSVRES=$(RES)$/$(TARGET).res

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

$(OBJ)$/soreport.obj: $(INCCOM)$/_version.h

# Generate the native Windows resource file
# using lngconvex.exe 

$(RCFILES) : $(LNGFILES) makefile.mk rcfooter.txt rcheader.txt rctemplate.txt ctrylnglist.txt
    $(LNGCONVEX) -ulf $(ULFDIR)$/crashrep.ulf -rc $(RES)$/$(TARGET).rc  -rct rctemplate.txt -rch rcheader.txt -rcf rcfooter.txt
    
