# Builds the C++ DocumentLoader example of the SDK.

PRJ=../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk
include $(SETTINGS)/dk.mk

# Define non-platform/compiler specific settings
COMPONENT_NAME=DocumentLoader
COMPONENT_RDB_NAME = $(COMPONENT_NAME).rdb
COMPONENT_RDB = $(OUT_BIN)/$(COMPONENT_RDB_NAME)

OUT_COMP_INC = $(OUT_INC)/$(COMPONENT_NAME)
OUT_COMP_GEN = $(OUT_MISC)/$(COMPONENT_NAME)
OUT_COMP_OBJ=$(OUT_OBJ)/$(COMPONENT_NAME)

COMPOENNT_ENV_FLAG = $(OUT_MISC)/cpp_$(COMPONENT_NAME)_prepare_env.flag
COMPONENT_TYPEFLAG = $(OUT_MISC)/cpp_$(COMPONENT_NAME)_types.flag

CXXFILES = DocumentLoader.cxx

OBJFILES = $(patsubst %.cxx,$(OUT_SLO_COMP)/%.$(OBJ_EXT),$(CXXFILES))

TYPES = \
	com.sun.star.lang.XMultiServiceFactory \
	com.sun.star.lang.XComponent \
	com.sun.star.beans.XPropertySet \
	com.sun.star.bridge.XUnoUrlResolver \
	com.sun.star.frame.XComponentLoader \
	com.sun.star.lang.XMultiComponentFactory \
	com.sun.star.container.XHierarchicalNameAccess \
	com.sun.star.registry.XSimpleRegistry

TYPESLIST = $(foreach t,$(TYPES),-T$(t))

# Targets
.PHONY: ALL
ALL : \
	CppDocumentLoaderExample

include $(SETTINGS)/stdtarget.mk

# create all component sepcific output directories
# and use OUT_COMP_GEN for dependencies 
$(OUT_COMP_GEN) : $(OUT)
	$(MKDIR) $(subst /,$(PS),$@)
	$(MKDIR) $(subst /,$(PS),$(OUT_COMP_INC))
	$(MKDIR) $(subst /,$(PS),$(OUT_COMP_OBJ))

# This example type library will be extended by the office types
$(OUT_BIN)/%.rdb : $(OUT_COMP_GEN)
	-$(DEL) $(subst /,$(PS),$@)
	regmerge $@ / $(DKREGISTRYNAME) 

$(COMPONENT_TYPEFLAG) : $(COMPONENT_RDB) $(OUT_COMP_GEN)
	-$(DEL) $(subst /,$(PS),$(COMPONENT_TYPEFLAG))
	cppumaker -Gc -BUCR -O$(OUT_COMP_INC) $(TYPESLIST) $(COMPONENT_RDB)
	echo flagged > $@

$(OUT_COMP_OBJ)/%.$(OBJ_EXT) : %.cxx $(COMPONENT_TYPEFLAG) $(OUT_COMP_GEN)
	$(CC) $(CC_FLAGS) $(CC_INCLUDES) -I$(OUT_COMP_INC) $(CC_DEFINES) $(CC_OUTPUT_SWITCH)$(subst /,$(PS),$@) $<

$(OUT_BIN)/%$(EXE_EXT) : $(OUT_COMP_OBJ)/%.$(OBJ_EXT) $(OUT_COMP_GEN)
ifeq "$(OS)" "WIN"
	$(LINK) $(EXE_LINK_FLAGS) /OUT:$@ /MAP:$(OUT_COMP_GEN)/$(basename $(@F)).map \
	  $< $(CPPUHELPERLIB) $(CPPULIB) $(SALHELPERLIB) $(SALLIB) $(STLPORTLIB)
else
	$(LINK) $(EXE_LINK_FLAGS) $(LINK_LIBS) -o $@ $< \
	  $(CPPUHELPERLIB) $(CPPULIB) $(SALHELPERLIB) $(SALLIB) $(STLPORTLIB) $(STDC++LIB)
endif

$(COMPOENNT_ENV_FLAG) : 
	-$(DEL) $@
	@echo --------------------------------------------------------------------------------
	@echo        Register necessary runtime components in the DocumentLoader.rdb 
	@echo --------------------------------------------------------------------------------
	regcomp -register -r $(OUT_BIN)/DocumentLoader.rdb -c $(SHAREDLIB_PRE)connectr.$(SHAREDLIB_EXT)
	regcomp -register -r $(OUT_BIN)/DocumentLoader.rdb -c $(SHAREDLIB_PRE)remotebridge.$(SHAREDLIB_EXT)
	regcomp -register -r $(OUT_BIN)/DocumentLoader.rdb -c $(SHAREDLIB_PRE)brdgfctr.$(SHAREDLIB_EXT)
	regcomp -register -r $(OUT_BIN)/DocumentLoader.rdb -c $(SHAREDLIB_PRE)uuresolver.$(SHAREDLIB_EXT)
	@echo bla > $@

CppDocumentLoaderExample : $(OUT_BIN)/DocumentLoader$(EXE_EXT) $(COMPOENNT_ENV_FLAG)
	@echo --------------------------------------------------------------------------------
	@echo The example loads the "$(QM)test.sxw$(QM)" document in the DocumentLoader example directory.
	@echo I you want to load your own document, please use: DocumentLoader "$(QM)filename$(QM)" [connection_url]
	@echo Use the following command to execute the example!
	@echo -
	@echo make DocumentLoader.run
	@echo --------------------------------------------------------------------------------

%.run: $(OUT_BIN)/DocumentLoader$(EXE_EXT)
	cd $(OUT_BIN) && $(basename $@) "../../examples/cpp/DocumentLoader/test.sxw"

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_INC))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_GEN))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_OBJ))
	-$(DEL) $(subst /,$(PS),$(COMPONENT_TYPEFLAG))
	-$(DEL) $(subst /,$(PS),$(COMPOENNT_ENV_FLAG))
	-$(DEL) $(subst /,$(PS),$(COMPONENT_RDB))
	-$(DEL) $(subst /,$(PS),$(OUT_BIN)/DocumentLoader*)
