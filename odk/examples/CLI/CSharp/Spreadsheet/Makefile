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

# This Makefile is supposed to be run by a GNU Make built for native
# Windows Not the Cygwin GNU Make, not dmake, not Microsoft's NMAKE.
# But yeah, that is what the setsdkenv_windows.bat file in
# ../../../.. presumably tries to set up.

PRJ = ../../../..
SETTINGS = $(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk

CSC_FLAGS = -warnaserror+ -noconfig -platform:x86
VBC_FLAGS = -warnaserror+
ifeq "$(DEBUG)" "yes"
CC_FLAGS+=-Zi -MT
CSC_FLAGS += -debug+ -checked+ -define:DEBUG -define:TRACE
VBC_FLAGS += -debug+ -define:DEBUG=1 -define:TRACE=1
else
CSC_FLAGS += -o
VBC_FLAGS += -o
endif

THIS_OUT_MISC = $(OUT_MISC)/csharp_SpreadsheetExamples

SPREADSHEET_OUT:= $(OUT_BIN)/CSharp_SpreadsheetExamples

ifneq "$(OUTDIR)" ""
# We are running in a LO build environment, good
CLI_LIB_LOCATION = $(OUTDIR)\bin
else
# Bad guess, but hey, this is how it was
CLI_LIB_LOCATION = $(PRJ)\cli
endif

# Targets
.PHONY : ALL
ALL : Info	

include $(SETTINGS)/stdtarget.mk


# build executables
#csc -lib:"d:\StarOffice 8" does not work. csc does not understand the quotes. 
#but they are needed if the path contains a space. Therefore we use full path 
#with the -reference switch
$(SPREADSHEET_OUT)/%.exe : %.cs SpreadsheetDocHelper.cs
	-$(MKDIR) $(subst /,$(PS),$(@D))
	csc $(CSC_FLAGS) \
		-target:exe \
		-out:$@ \
		-reference:"$(CLI_LIB_LOCATION)"\cli_basetypes.dll \
		-reference:"$(CLI_LIB_LOCATION)"\cli_uretypes.dll \
		-reference:"$(CLI_LIB_LOCATION)"\cli_oootypes.dll \
		-reference:"$(CLI_LIB_LOCATION)"\cli_ure.dll \
		-reference:"$(CLI_LIB_LOCATION)"\cli_cppuhelper.dll \
		$^

SpreadsheetSample : $(SPREADSHEET_OUT)/SpreadsheetSample.exe 

GeneralTableSample : $(SPREADSHEET_OUT)/GeneralTableSample.exe 

ViewSample : $(SPREADSHEET_OUT)/ViewSample.exe

.PHONY : Info
ifeq "$(OS)" "WIN"
Info : SpreadsheetSample GeneralTableSample ViewSample
	@echo -------------------------------------------------------------------------------
	@echo Please use one of the following commands to execute the examples!
	@echo -
	@echo $(MAKE) GeneralTableSample.run
	@echo $(MAKE) SpreadsheetSample.run
	@echo $(MAKE) ViewSample.run
	@echo -------------------------------------------------------------------------------
else
Info : 
	@echo --------------------------------------------------------------------------------
	@echo This example works only under Windows!
	@echo --------------------------------------------------------------------------------
endif

%.run : $(SPREADSHEET_OUT)/%.exe
	@echo -------------------------------------------------------------------------------
	@echo Executing sample $@ $(UNORC) $(CONNECT_STRING)...
	@echo -------------------------------------------------------------------------------
	cd $(subst /,\\,$(SPREADSHEET_OUT)) && $(basename $@).exe 

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_BIN)/CSharp_SpreadsheetExamples)


