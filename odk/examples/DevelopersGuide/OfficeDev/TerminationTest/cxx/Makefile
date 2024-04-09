# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# Builds the TerminationTest example of the SDK.

PRJ=../../../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk

# Define non-platform/compiler specific settings
APP_NAME=TerminationTest

OUT_APP_INC = $(OUT_INC)/$(APP_NAME)
OUT_APP_GEN = $(OUT_MISC)/$(APP_NAME)
OUT_APP_OBJ=$(OUT_OBJ)/$(APP_NAME)

CXXFILES = TerminationTest.cxx

OBJFILES = $(patsubst %.cxx,$(OUT_SLO_COMP)/%.$(OBJ_EXT),$(CXXFILES))

# Targets
.PHONY: ALL
ALL : \
	TerminationTest

include $(SETTINGS)/stdtarget.mk

$(OUT_APP_OBJ)/%.$(OBJ_EXT) : %.cxx $(SDKTYPEFLAG)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(CC) $(CC_FLAGS) $(CC_INCLUDES) -I$(OUT_APP_INC) $(CC_DEFINES) $(CC_OUTPUT_SWITCH)$(subst /,$(PS),$@) $<

$(OUT_BIN)/_$(APP_NAME)$(EXE_EXT) : $(OUT_APP_OBJ)/$(APP_NAME).$(OBJ_EXT)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	-$(MKDIR) $(subst /,$(PS),$(OUT_APP_GEN))
ifeq "$(OS)" "WIN"
	$(LINK) $(EXE_LINK_FLAGS) /OUT:$@ /MAP:$(OUT_APP_GEN)/$(basename $(@F)).map \
	  $< $(CPPUHELPERLIB) $(CPPULIB) $(SALHELPERLIB) $(SALLIB)
else
	$(LINK) $(EXE_LINK_FLAGS) $(LINK_LIBS) -o $@ $< \
	  $(CPPUHELPERLIB) $(CPPULIB) $(SALHELPERLIB) $(SALLIB) $(STDC++LIB)
ifeq "$(OS)" "MACOSX"
	$(INSTALL_NAME_URELIBS_BIN)  $@
endif
endif

$(OUT_BIN)/$(APP_NAME)$(EXE_EXT) : $(OUT_BIN)/_$(APP_NAME)$(EXE_EXT)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(COPY) $(subst /,$(PS),$(BIN_DIR)/unoapploader$(EXE_EXT)) $(subst /,$(PS),$@)
# workaround for touch problem under Windows with full qualified paths
	make -t $@

TerminationTest: $(OUT_BIN)/$(APP_NAME)$(EXE_EXT)
	@echo --------------------------------------------------------------------------------
	@echo Please use the following command to execute the example!
	@echo -
	@echo $(MAKE) TerminationTest.run
	@echo --------------------------------------------------------------------------------

%.run: $(OUT_BIN)/TerminationTest$(EXE_EXT)
	cd $(subst /,$(PS),$(OUT_BIN)) && $(basename $@)

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_APP_INC))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_APP_GEN))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_APP_OBJ))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(OUT_BIN)/*TerminationTest*))

# vim: set noet sw=4 ts=4:
