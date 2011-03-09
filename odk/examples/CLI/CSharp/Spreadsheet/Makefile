# Builds the SpreadSheet examples of the Developers Guide.

PRJ = ../../../..
SETTINGS = $(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk
include $(SETTINGS)/dk.mk

CSC_FLAGS = -warnaserror+ -noconfig
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
CLI_LIB_LOCATION = $(PRJ)\cli

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
Info : SpreadsheetSample GenerateTableSample ViewSample
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


