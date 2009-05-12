#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.21 $
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

PRJ=..$/..$/..

PRJNAME=testtools
TARGET=cli_cpp_bridgetest.uno
USE_DEFFILE=TRUE
NO_BSYMBOLIC=TRUE
ENABLE_EXCEPTIONS=TRUE
LIBTARGET=NO

# disable caching to avoid stale objects
# on version changes
CCACHE_DISABLE=TRUE
.EXPORT : CCACHE_DISABLE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
DLLPRE =
CFLAGSENABLESYMBOLS:=-Z7
# ------------------------------------------------------------------

#These tests are for Windows only
.IF "$(COM)" == "MSC" && "$(GUI)" == "WNT"

.IF "$(CCNUMVER)" >= "001399999999"
CFLAGSCXX += -clr:oldSyntax -AI $(OUT)$/bin -AI $(SOLARBINDIR)
SHL1STDLIBS = \
    mscoree.lib \
    msvcmrt.lib
.ELSE
CFLAGSCXX += -clr -AI $(OUT)$/bin -AI $(SOLARBINDIR)
SHL1STDLIBS = \
    mscoree.lib
.ENDIF
SLOFILES= \
    $(SLO)$/cli_cpp_bridgetest.obj

SHL1OBJS = $(SLOFILES)

SHL1TARGET = $(TARGET)

SHL1DEF = $(MISC)$/$(SHL1TARGET).def
DEF1NAME = $(SHL1TARGET)

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

.IF "$(COM)" == "MSC" && "$(GUI)" == "WNT"

ALLTAR : $(BIN)$/cli_bridgetest_inprocess.exe

#################################################################

CLI_URE = $(SOLARBINDIR)$/cli_ure.dll
CLI_URETYPES = $(SOLARBINDIR)$/cli_uretypes.dll
CLI_BASETYPES = $(SOLARBINDIR)$/cli_basetypes.dll
CLI_CPPUHELPER = $(SOLARBINDIR)$/cli_cppuhelper.dll
CLI_OOOTYPES = $(SOLARBINDIR)$/cli_oootypes.dll
CLI_TYPES_BRIDGETEST = $(BIN)$/cli_types_bridgetest.dll

CSCFLAGS = -warnaserror+ 
.IF "$(CCNUMVER)" <= "001399999999"
VBC_FLAGS = -warnaserror+
.ELSE
VBC_FLAGS = -nowarn:42030 -warnaserror+
.ENDIF
.IF "$(debug)" != ""
CSCFLAGS += -debug+ -checked+ -define:DEBUG -define:TRACE
VBC_FLAGS += -debug+ -define:DEBUG=TRUE -define:TRACE=TRUE
.ELSE
CSCFLAGS += -optimize
VBC_FLAGS += -optimize
.ENDIF


# C# ----------------------------------------------
$(BIN)$/cli_cs_testobj.uno.dll : \
        cli_cs_testobj.cs \
        cli_cs_multi.cs \
        $(CLI_BASETYPES) \
        $(CLI_URETYPES) \
        $(CLI_URE)
    $(CSC) $(CSCFLAGS) -target:library -out:$@ \
        -reference:$(CLI_TYPES_BRIDGETEST) \
        -reference:$(CLI_URE) \
        -reference:$(CLI_BASETYPES) \
         -reference:$(CLI_URETYPES) \
        cli_cs_testobj.cs cli_cs_multi.cs

$(BIN)$/cli_cs_bridgetest.uno.dll : \
        cli_cs_bridgetest.cs \
        $(CLI_BASETYPES) \
        $(CLI_URETYPES) \
        $(CLI_URE)
    $(CSC) $(CSCFLAGS) -target:library -out:$@ \
        -reference:$(CLI_TYPES_BRIDGETEST) \
        -reference:$(CLI_URETYPES) \
        -reference:$(CLI_BASETYPES) \
        -reference:$(CLI_URE) \
        -reference:System.dll \
        cli_cs_bridgetest.cs

# Visual Basic ------------------------------------------
$(BIN)$/cli_vb_bridgetest.uno.dll : \
        cli_vb_bridgetest.vb \
        $(CLI_BASETYPES) \
        $(CLI_URETYPES) \
        $(CLI_URE)
    $(VBC) $(VBC_FLAGS) \
        -target:library \
        -out:$@ \
        -reference:$(CLI_URETYPES) \
        -reference:$(CLI_BASETYPES) \
        -reference:$(CLI_URE) \
        -reference:$(CLI_TYPES_BRIDGETEST) \
        -reference:System.dll \
        -reference:System.Drawing.dll \
        -reference:System.Windows.Forms.dll \
        cli_vb_bridgetest.vb

$(BIN)$/cli_vb_testobj.uno.dll : \
        cli_vb_testobj.vb \
        $(CLI_BASETYPES) \
        $(CLI_URETYPES) \
        $(CLI_URE)
    $(VBC) $(VBC_FLAGS) \
        -target:library \
        -out:$@ \
        -reference:$(CLI_BASETYPES) \
        -reference:$(CLI_URETYPES) \
        -reference:$(CLI_URE) \
        -reference:$(CLI_TYPES_BRIDGETEST) \
        -reference:System.dll \
        -reference:System.Drawing.dll \
        -reference:System.Windows.Forms.dll \
        cli_vb_testobj.vb

$(MISC)$/copyassemblies.done .ERRREMOVE: 
    $(GNUCOPY) $(CLI_CPPUHELPER) $(BIN)$/$(CLI_CPPUHELPER:f)
    $(GNUCOPY) $(CLI_BASETYPES) $(BIN)$/$(CLI_BASETYPES:f)
    $(GNUCOPY) $(CLI_URETYPES) $(BIN)$/$(CLI_URETYPES:f)
    $(GNUCOPY) $(CLI_URE) $(BIN)$/$(CLI_URE:f)
    $(GNUCOPY) $(CLI_OOOTYPES) $(BIN)$/$(CLI_OOOTYPES:f)
    $(TOUCH) $@

$(BIN)$/cli_bridgetest_inprocess.exe : \
        cli_bridgetest_inprocess.cs \
        $(BIN)$/cli_cs_bridgetest.uno.dll \
        $(BIN)$/cli_cs_testobj.uno.dll \
        $(BIN)$/cli_vb_bridgetest.uno.dll \
        $(BIN)$/cli_vb_testobj.uno.dll \
        $(BIN)$/cli_cpp_bridgetest.uno.dll \
        $(MISC)$/copyassemblies.done \
        $(CLI_BASETYPES) \
        $(CLI_URETYPES) \
        $(CLI_URE) \
        $(CLI_CPPUHELPER)
    $(CSC) $(CSCFLAGS) -target:exe -out:$@ \
        -reference:$(CLI_TYPES_BRIDGETEST) \
        -reference:$(CLI_BASETYPES) \
        -reference:$(CLI_URETYPES) \
        -reference:$(CLI_URE) \
        -reference:$(CLI_CPPUHELPER) \
        -reference:$(BIN)$/cli_cs_bridgetest.uno.dll \
        -reference:$(BIN)$/cli_cs_testobj.uno.dll \
        -reference:$(BIN)$/cli_vb_bridgetest.uno.dll \
        -reference:$(BIN)$/cli_vb_testobj.uno.dll \
        -reference:$(BIN)$/cli_cpp_bridgetest.uno.dll \
        cli_bridgetest_inprocess.cs
    $(GNUCOPY) cli_bridgetest_inprocess.ini $(BIN)

.ENDIF

