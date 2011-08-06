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
# ************************************************************************/

# Builds the SpreadSheet examples of the Developers Guide.

PRJ = ..$/..
PRJNAME = cli_ure
TARGET := test_climaker
PACKAGE = cliversion

.INCLUDE: settings.mk

.IF "$(CROSS_COMPILING)"=="YES"
all:
    @echo Nothing done when cross-compiling
.ENDIF

.INCLUDE : $(PRJ)$/util$/makefile.pmk

.IF "$(debug)" != ""
CSCFLAGS += -checked+ -define:DEBUG -define:TRACE -debug+
.ELSE
CSCFLAGS += -optimize+
.ENDIF


EXETARGET2=$(BIN)$/runtests.exe 

.IF "$(name)" != ""
TESTLIB=$(BIN)$/$(name)
.ELSE
TESTLIB=$(BIN)$/version_current.dll
.ENDIF
VERSIONLIBS=version_libs

CLI_URE = $(SOLARBINDIR)$/cli_ure.dll
CLI_URETYPES = $(SOLARBINDIR)$/cli_uretypes.dll
CLI_BASETYPES = $(SOLARBINDIR)$/cli_basetypes.dll
CLI_CPPUHELPER = $(SOLARBINDIR)$/cli_cppuhelper.dll
CLI_OOOTYPES = $(SOLARBINDIR)$/cli_oootypes.dll


.INCLUDE: target.mk

.IF "$(BUILD_FOR_CLI)" == ""
ALLTAR:

.ELSE
ALLTAR: \
    $(EXETARGET2) \
    $(TESTLIB) \
    $(MISC)$/copyassemblies.done \
    COPYVERSIONLIBS \
    RUNINSTRUCTIONS
    
.ENDIF


COPYVERSIONLIBS: 
        -$(GNUCOPY) $(VERSIONLIBS)$/* $(BIN)


$(MISC)$/copyassemblies.done .ERRREMOVE: 
    $(GNUCOPY) $(CLI_CPPUHELPER) $(BIN)$/$(CLI_CPPUHELPER:f)
    $(GNUCOPY) $(CLI_BASETYPES) $(BIN)$/$(CLI_BASETYPES:f)
    $(GNUCOPY) $(CLI_URETYPES) $(BIN)$/$(CLI_URETYPES:f)
    $(GNUCOPY) $(CLI_URE) $(BIN)$/$(CLI_URE:f)
    $(GNUCOPY) $(CLI_OOOTYPES) $(BIN)$/$(CLI_OOOTYPES:f)
    $(TOUCH) $@
 
CSFILES2 = runtests.cs

.IF "$(CCNUMVER)" >= "001399999999"
CSCPLATFORMX86 = -platform:x86
.ELSE
CSCPLATFORMX86 =
.ENDIF

$(EXETARGET2): $(CSFILES2)
    $(CSC) $(CSCFLAGS) -target:exe -out:$(EXETARGET2) $(CSCPLATFORMX86)\
        $(CSFILES2)

CSFILESLIB = version.cs
$(TESTLIB): $(CSFILESLIB) $(SOLARBINDIR)$/cliureversion.mk
    $(CSC) $(CSCFLAGS) -target:library -out:$(TESTLIB) \
        -reference:$(SOLARBINDIR)$/cli_ure.dll \
         -reference:$(SOLARBINDIR)$/cli_uretypes.dll \
         -reference:$(SOLARBINDIR)$/cli_basetypes.dll \
        -reference:$(SOLARBINDIR)$/cli_cppuhelper.dll \
        -reference:$(SOLARBINDIR)$/cli_oootypes.dll \
        $(CSFILESLIB)



RUNINSTRUCTIONS : 
    @echo .
    @echo ###########################   N O T E  ######################################
    @echo . 
    @echo To run the test go to qa\cliversioning, run dmake and follow instructions.
    @echo .
    @echo Or install an office with full system integration and run $(BIN)$/runtests.exe 
    @echo in a console withouth build environment.
    @echo .
    @echo Or install an office with setup /a. Expand PATH with the bin folder of the URE.
    @echo Set UNO_PATH=system_path_to_program_folder
    @echo This must be the program folder of the brand layer.
    @echo For example, c:\staroffice\Sun\staroffice 9\program
    @echo Run runtests.exe.
    @echo .
    @echo To build a test library with run.
    @echo
    @echo dmake name=version_10_10_10.dll
    @echo
    @echo  The name must always start with 'version_'. 	
    @echo ###########################   N O T E  ######################################
    @echo .
    @echo .	
