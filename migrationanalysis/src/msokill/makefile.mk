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

PRJNAME		=	migrationanalysis
TARGET		=	msokill
LIBTARGET	=	NO
TARGETTYPE	=	CUI

ENABLE_EXCEPTIONS	=	TRUE

# --- Settings ------------------------------------------------------------

.INCLUDE : settings.mk

# --- Allgemein -----------------------------------------------------------

OBJFILES= 	$(OBJ)$/msokill.obj \
            $(OBJ)$/StdAfx.obj

# --- Targets ------------------------------------------------------

UWINAPILIB=
LIBSALCPPRT=

APP1NOSAL=		TRUE
APP1TARGET=		msokill

STDLIB1=msi.lib\
    shell32.lib\
    oleaut32.lib\
    gdi32.lib\
    comdlg32.lib\
    advapi32.lib\
    comctl32.lib\
    shlwapi.lib\
    oledlg.lib\
    ole32.lib\
    uuid.lib\
    oleacc.lib\
    winspool.lib\

APP1OBJS=		$(OBJFILES)

PAW_RES_DIR:=$(BIN)$/ProAnalysisWizard$/Resources
PAW_RES_EXE:=$(PAW_RES_DIR)$/$(APP1TARGET).exe

# --- setup --------------------------------------------------------------

.INCLUDE :  target.mk

ALLTAR : $(PAW_RES_EXE)

$(PAW_RES_EXE) : $(BIN)$/$$(@:f)
    -$(MKDIRHIER) $(@:d)
    $(COPY) $(BIN)$/$(@:f) $@

