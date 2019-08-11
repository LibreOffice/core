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

# Builds the SpreadSheet examples of the Developers Guide.

PRJ = ..$/..
PRJNAME = cli_ure
TARGET := test_climaker
PACKAGE = cliversion

.INCLUDE: settings.mk

.IF "$(CROSS_COMPILING)"=="YES"

all:
# nothing

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
    @echo in a console without build environment.
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
