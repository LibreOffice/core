# Builds the remoteclient demo.

PRJ=..$(PS)..$(PS)..
SETTINGS=../../../settings

# Include settings makefile depending on platform/compiler
ifeq "$(MAKECMDGOALS)" "win_microcxx"
        -include $(SETTINGS)/win_microcxx_settings.mk
ifndef PS
$(error Platform/Compiler settings file not found)
endif
        SHAREDLIB_OUT=$(OUT_BIN)
endif   
ifeq "$(MAKECMDGOALS)" "solaris_workshopcxx"
        -include $(SETTINGS)/solaris_workshopcxx_settings.mk
ifndef PS
$(error Platform/Compiler settings file not found)
endif
        SHAREDLIB_OUT=$(OUT_LIB)
endif   
ifeq "$(MAKECMDGOALS)" "linux_gcc"
        -include $(SETTINGS)/linux_gcc_settings.mk
ifndef PS
$(error Platform/Compiler settings file not found)
endif
        SHAREDLIB_OUT=$(OUT_LIB)
endif

# Define non-platform/compiler specific settings

include $(SETTINGS)/dk.mk
include $(SETTINGS)/std.mk

# Define non-platform/compiler specific settings

# Targets
.PHONY: ALL
ALL :
	@echo -------------------------------------------------------------
	@echo You must supply a target where TARGET is one of the following:
	@echo 
	@echo win_microcxx        : Windows using Microsoft C++ compiler/linker
	@echo solaris_workshopcxx : Solaris using Sun Workshop C++ compiler
	@echo linux_gcc           : Linux using gcc
	@echo -------------------------------------------------------------

TYPEFLAGFILE= $(OUT_MISC)$(PS)type_generation_officeclient_is_ok.flag

REGISTERFLAGFILE= $(OUT_MISC)$(PS)register_components_officeclient_is_ok.flag

#win_microcxx : $(OUT) $(SHAREDLIB_OUT)$(PS)$(SHAREDLIB_PRE)FlatXml.$(SHAREDLIB_EXT)
win_microcxx : $(REGISTERFLAGFILE)

#solaris_workshopcxx : $(OUT) $(SHAREDLIB_OUT)$(PS)$(SHAREDLIB_PRE)FlatXml.$(SHAREDLIB_EXT)
solaris_workshopcxx : $(REGISTERFLAGFILE)

#linux_gcc : $(OUT) $(SHAREDLIB_OUT)$(PS)$(SHAREDLIB_PRE)FlatXml.$(SHAREDLIB_EXT)
linux_gcc : $(REGISTERFLAGFILE)
	
include $(SETTINGS)/stdtarget.mk

TYPES := $(foreach t,$(shell xml2cmp -types stdout FlatXml.xml),-T$(t))

$(TYPEFLAGFILE) : $(BIN_DIR)$(PS)$(DKREGISTRYNAME)
	-$(DEL) $(TYPEFLAGFILE)
	@echo Building for $(MAKECMDGOALS)
	@echo $(BIN_DIR)$(PS)cppumaker -BUCR -O$(OUT)$(PS)inc$(PS)examples $(TYPES) $(BIN_DIR)$(PS)$(DKREGISTRYNAME)
	$(BIN_DIR)$(PS)cppumaker -BUCR -O$(OUT)$(PS)inc$(PS)examples $(TYPES) $(BIN_DIR)$(PS)$(DKREGISTRYNAME)
	regmerge $(OUT_BIN)$(PS)FlatXml.rdb / $(BIN_DIR)$(PS)$(DKREGISTRYNAME)
	@echo bla >$(TYPEFLAGFILE)

$(OUT_SLO)$(PS)FlatXml.$(OBJ_EXT) : FlatXml.cxx $(TYPEFLAGFILE)
	$(CC) $(CC_FLAGS) $(CC_INCLUDES) $(CC_DEFINES) $(CC_OUTPUT_SWITCH)$@ $<


$(OUT_SLB)$(PS)FlatXml.lib : $(OUT_SLO)$(PS)FlatXml.$(OBJ_EXT)
ifeq "$(OS)" "WIN"
	$(BUILDLIB) /OUT:$@ $^
endif

$(OUT_LIB)$(PS)FlatXml.lib : $(OUT_SLO)$(PS)FlatXml.$(OBJ_EXT)
ifeq "$(OS)" "WIN"
	@echo EXPORTS > $(OUT_MISC)$(PS)FlatXml.def
	$(CAT) exports.dxp >> $(OUT_MISC)$(PS)FlatXml.def
	$(BUILDLIB) /OUT:$@ /DEF:$(OUT_MISC)$(PS)FlatXml.def
endif

$(SHAREDLIB_OUT)$(PS)$(SHAREDLIB_PRE)FlatXml.$(SHAREDLIB_EXT) : $(OUT_LIB)$(PS)FlatXml.lib \
			$(OUT_SLB)$(PS)FlatXml.lib
ifeq "$(OS)" "WIN"
	$(LINK) $(LIBRARY_LINK_FLAGS) /OUT:$@ /MAP:$(OUT_MISC)$(PS)FlatXml.map \
	 $(OUT_LIB)$(PS)FlatXmlexp $(OUT_SLB)$(PS)FlatXml.lib \
	  $(SALLIB) $(CPPULIB) $(CPPUHELPERLIB) msvcrt.lib kernel32.lib
else
	$(LINK) $(LIBRARY_LINK_FLAGS) $(LINK_LIBS) -o $@ $(OUT_SLO)$(PS)FlatXml.$(OBJ_EXT) \
	 $(SALLIB) $(CPPULIB) $(CPPUHELPERLIB)
endif

$(REGISTERFLAGFILE) : $(SHAREDLIB_OUT)$(PS)$(SHAREDLIB_PRE)FlatXml.$(SHAREDLIB_EXT) 
	-$(DEL) $(REGISTERFLAGFILE)
	

