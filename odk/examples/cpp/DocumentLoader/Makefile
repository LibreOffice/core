# Builds the DocumentLoader demo.

PRJ=..$(PS)..$(PS)..
SETTINGS=../../../settings

include $(SETTINGS)/settings.mk

include $(SETTINGS)/dk.mk
include $(SETTINGS)/std.mk

# Define non-platform/compiler specific settings

REGISTERFLAGFILE= $(OUT_MISC)$(PS)documentloader_register_component.flag
TYPEFLAGFILE= $(OUT_MISC)$(PS)documentloader_type_generation.flag

TYPES = \
	-Tcom.sun.star.lang.XMultiServiceFactory \
	-Tcom.sun.star.lang.XComponent \
	-Tcom.sun.star.bridge.XUnoUrlResolver \
	-Tcom.sun.star.frame.XComponentLoader \
	-Tcom.sun.star.lang.XMultiComponentFactory \
	-Tcom.sun.star.container.XHierarchicalNameAccess \
	-Tcom.sun.star.registry.XSimpleRegistry

TYPEFILES = $(subst \\,\,$(patsubst -T%,$(OUT)$(PS)inc$(PS)examples$(PS)%.hpp,$(TYPES)))

# Targets
.PHONY: ALL
ALL : $(OUT_BIN)$(PS)DocumentLoader$(EXE_EXT) \
	register

include $(SETTINGS)/stdtarget.mk

#$(BIN_DIR)$(PS)rdbmaker -BUCR -bUCR -O$(OUT_BIN)$(PS)DocumentLoader.rdb $(TYPES) $(BIN_DIR)$(PS)$(DKREGISTRYNAME)

$(TYPEFLAGFILE) : $(BIN_DIR)$(PS)$(DKREGISTRYNAME)
	@echo $(TYPEFILES)
	-$(DEL) $@ $(OUT_BIN)$(PS)DocumentLoader.rdb
	@echo Building for $(MAKECMDGOALS)
	$(BIN_DIR)$(PS)cppumaker -BUCR -O$(OUT)$(PS)inc$(PS)examples $(TYPES) $(BIN_DIR)$(PS)$(DKREGISTRYNAME)
	$(BIN_DIR)$(PS)regmerge $(OUT_BIN)$(PS)DocumentLoader.rdb / $(BIN_DIR)$(PS)$(DKREGISTRYNAME)
	-$(DEL) $(REGISTERFLAGFILE)
	@echo bla > $@

$(OUT_OBJ)$(PS)DocumentLoader.$(OBJ_EXT) : DocumentLoader.cxx $(OUT) $(TYPEFLAGFILE)
	@echo $(TYPEFILES)
	$(CC) $(CC_FLAGS) $(CC_INCLUDES) $(CC_DEFINES) $(CC_OUTPUT_SWITCH)$@ $<

$(OUT_BIN)$(PS)DocumentLoader$(EXE_EXT) : $(OUT_OBJ)$(PS)DocumentLoader.$(OBJ_EXT)
ifeq "$(OS)" "WIN"
	$(LINK) $(EXE_LINK_FLAGS) /OUT:$@ /MAP:$(OUT_MISC)$(PS)DocumentLoader.map \
	  $(OUT_OBJ)$(PS)DocumentLoader.$(OBJ_EXT) isal.lib icppu.lib icppuhelper.lib
else
	$(LINK) $(EXE_LINK_FLAGS) $(LINK_LIBS) -o $@ $(OUT_OBJ)$(PS)DocumentLoader.$(OBJ_EXT) \
	  $(SALLIB) $(VOSLIB) $(CPPULIB) $(CPPUHELPERLIB) $(STDC++_LIB)
endif

$(REGISTERFLAGFILE) : $(TYPEFLAGFILE) 
	-$(DEL) $@
	@echo --------------------------------------------------------------------------------
	@echo                     Register components in DocumentLoader.rdb
	@echo --------------------------------------------------------------------------------
	regcomp -register -r $(OUT_BIN)$(PS)DocumentLoader.rdb -c $(SHAREDLIB_PRE)connectr.$(SHAREDLIB_EXT)
	regcomp -register -r $(OUT_BIN)$(PS)DocumentLoader.rdb -c $(SHAREDLIB_PRE)remotebridge.$(SHAREDLIB_EXT)
	regcomp -register -r $(OUT_BIN)$(PS)DocumentLoader.rdb -c $(SHAREDLIB_PRE)brdgfctr.$(SHAREDLIB_EXT)
	regcomp -register -r $(OUT_BIN)$(PS)DocumentLoader.rdb -c $(SHAREDLIB_PRE)uuresolver.$(SHAREDLIB_EXT)
	@echo bla > $@

register: $(REGISTERFLAGFILE)
