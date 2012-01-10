#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



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

