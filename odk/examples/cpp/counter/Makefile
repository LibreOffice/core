# Builds the C++ counter example of the SDK.

PRJ=../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk
include $(SETTINGS)/dk.mk

# Define non-platform/compiler specific settings
COMPONENT_NAME=counter
COMPONENT_IMPL_NAME=$(SHAREDLIB_PRE)$(COMPONENT_NAME).$(SHAREDLIB_EXT) 
COMPONENT_IMPL=$(SHAREDLIB_OUT)/$(COMPONENT_IMPL_NAME)
COMPONENT_XML=$(COMPONENT_NAME).xml
COMPONENT_RDB_NAME = $(COMPONENT_NAME).rdb
COMPONENT_RDB = $(OUT_BIN)/$(COMPONENT_RDB_NAME)

OUT_COMP_INC = $(OUT_INC)/$(COMPONENT_NAME)
OUT_COMP_GEN = $(OUT_MISC)/$(COMPONENT_NAME)
OUT_COMP_OBJ=$(OUT_OBJ)/$(COMPONENT_NAME)
OUT_COMP_SLO=$(OUT_SLO)/$(COMPONENT_NAME)

COMPONENT_TYPEFLAG = $(OUT_MISC)/cpp_$(COMPONENT_NAME)_types.flag

IDLFILES = XCountable.idl

COMPCXXFILES = counter.cxx

CXXFILES = countermain.cxx

SLOFILES = $(patsubst %.cxx,$(OUT_COMP_SLO)/%.$(OBJ_EXT),$(COMPCXXFILES))
OBJFILES = $(patsubst %.cxx,$(OUT_COMP_SLO)/%.$(OBJ_EXT),$(CXXFILES))

GENURDFILES = $(patsubst %.idl,$(OUT_COMP_GEN)/%.urd,$(IDLFILES))

TYPES = $(shell xml2cmp -types stdout $(COMPONENT_XML))
TYPESLIST = $(foreach t,$(TYPES),-T$(t))

# Targets
.PHONY: ALL
ALL : \
	CppCounterExample

include $(SETTINGS)/stdtarget.mk

# create all component sepcific output directories
# and use OUT_COMP_GEN for dependencies 
$(OUT_COMP_GEN) : $(OUT)
	$(MKDIR) $(subst /,$(PS),$@)
	$(MKDIR) $(subst /,$(PS),$(OUT_COMP_INC))
	$(MKDIR) $(subst /,$(PS),$(OUT_COMP_OBJ))
	$(MKDIR) $(subst /,$(PS),$(OUT_COMP_SLO))

$(OUT_COMP_GEN)/%.urd : %.idl $(OUT_COMP_GEN)
	idlc -I. -I$(IDL_DIR) -O$(OUT_COMP_GEN) $<

# This example type library will be extended by the office types
$(OUT_BIN)/%.rdb : $(GENURDFILES) $(OUT_COMP_GEN)
	-$(DEL) $(subst /,$(PS),$@)
	regmerge $@ /UCR $(GENURDFILES)
	regmerge $@ / $(DKREGISTRYNAME) 

$(COMPONENT_TYPEFLAG) : $(COMPONENT_RDB) $(COMPONENT_XML) $(OUT_COMP_GEN)
	-$(DEL) $(subst /,$(PS),$(COMPONENT_TYPEFLAG))
	cppumaker -Gc -BUCR -O$(OUT_COMP_INC) $(TYPESLIST) $(COMPONENT_RDB)
	echo flagged > $@

$(OUT_COMP_OBJ)/%.$(OBJ_EXT) : %.cxx $(COMPONENT_TYPEFLAG) $(OUT_COMP_GEN)
	$(CC) $(CC_FLAGS) $(CC_INCLUDES) -I$(OUT_COMP_INC) $(CC_DEFINES) $(CC_OUTPUT_SWITCH)$(subst /,$(PS),$@) $<

$(OUT_COMP_SLO)/%.$(OBJ_EXT) : %.cxx $(COMPONENT_TYPEFLAG) $(OUT_COMP_GEN)
	$(CC) $(CC_FLAGS) $(CC_INCLUDES) -I$(OUT_COMP_INC) $(CC_DEFINES) $(CC_OUTPUT_SWITCH)$(subst /,$(PS),$@) $<

$(OUT_COMP_GEN)/%.def : exports.dxp
ifeq "$(OS)" "WIN"
	@echo EXPORTS > $@
	$(CAT) exports.dxp >> $@
endif

ifeq "$(OS)" "WIN"
$(SHAREDLIB_OUT)/$(SHAREDLIB_PRE)%.$(SHAREDLIB_EXT) : $(SLOFILES) $(OUT_COMP_GEN)/$(COMPONENT_NAME).def $(OUT_COMP_GEN)
	$(LINK) $(LIBRARY_LINK_FLAGS) /OUT:$@ /MAP:$(OUT_COMP_GEN)/$(COMPONENT_NAME).map \
	/DEF:$(OUT_COMP_GEN)/$(COMPONENT_NAME).def $(SLOFILES) \
	$(CPPUHELPERLIB) $(CPPULIB) $(SALLIB) $(STLPORTLIB) msvcrt.lib kernel32.lib
else
$(SHAREDLIB_OUT)/$(SHAREDLIB_PRE)%.$(SHAREDLIB_EXT) : $(SLOFILES)
	$(LINK) $(LIBRARY_LINK_FLAGS) $(LINK_LIBS) -o $@ $^\
	$(CPPUHELPERLIB) $(CPPULIB) $(SALLIB) $(STLPORTLIB) $(STC++LIB)
endif	

$(OUT_BIN)/%$(EXE_EXT) : $(OUT_COMP_OBJ)/%.$(OBJ_EXT) $(SHAREDLIB_OUT)/$(SHAREDLIB_PRE)counter.$(SHAREDLIB_EXT) $(OUT_COMP_GEN)
ifeq "$(OS)" "WIN"
	$(LINK) $(EXE_LINK_FLAGS) /OUT:$@ /MAP:$(OUT_COMP_GEN)/$(basename $(@F)).map \
	  $< $(CPPUHELPERLIB) $(CPPULIB) $(SALHELPERLIB) $(SALLIB) $(STLPORTLIB)
else
	$(LINK) $(EXE_LINK_FLAGS) $(LINK_LIBS) -o $@ $< \
	  $(CPPUHELPERLIB) $(CPPULIB) $(SALHELPERLIB) $(SALLIB) $(STLPORTLIB) $(STDC++LIB)
endif


CppCounterExample : $(OUT_BIN)/countermain$(EXE_EXT) $(COMPONENT_IMPL)
	@echo --------------------------------------------------------------------------------
	@echo Please use the following command to execute the example!
	@echo -
	@echo make countermain.run
	@echo --------------------------------------------------------------------------------

%.run: $(OUT_BIN)/countermain$(EXE_EXT) $(COMPONENT_IMPL)
	cd $(OUT_BIN) && $(basename $@)

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_INC))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_GEN))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_OBJ))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_SLO))
	-$(DEL) $(subst /,$(PS),$(COMPONENT_TYPEFLAG))
	-$(DEL) $(subst /,$(PS),$(COMPONENT_RDB))
	-$(DEL) $(subst /,$(PS),$(SHAREDLIB_OUT)/$(SHAREDLIB_PRE)$(COMPONENT_NAME).*)
	-$(DEL) $(subst /,$(PS),$(OUT_BIN)/countermain*)
