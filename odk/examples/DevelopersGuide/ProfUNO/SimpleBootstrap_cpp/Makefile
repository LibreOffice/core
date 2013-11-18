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

# Builds the SimpleBootstrap_cpp example of the SDK.

PRJ=../../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk
include $(SETTINGS)/dk.mk

# Define non-platform/compiler specific settings
APP_NAME=SimpleBootstrap_cpp

OUT_APP_INC = $(OUT_INC)/$(APP_NAME)
OUT_APP_GEN = $(OUT_MISC)/$(APP_NAME)
OUT_APP_OBJ=$(OUT_OBJ)/$(APP_NAME)

CXXFILES = SimpleBootstrap_cpp.cxx

OBJFILES = $(patsubst %.cxx,$(OUT_SLO_COMP)/%.$(OBJ_EXT),$(CXXFILES))

# Targets
.PHONY: ALL
ALL : \
	SimpleBootstrapCppExample

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
	  $(CPPUHELPERLIB) $(CPPULIB) $(SALHELPERLIB) $(SALLIB) $(STDC++LIB) $(CPPUHELPERDYLIB) $(CPPUDYLIB) $(SALHELPERDYLIB) $(SALDYLIB)
ifeq "$(OS)" "MACOSX"
	$(INSTALL_NAME_URELIBS_BIN)  $@
endif
endif

$(OUT_BIN)/$(APP_NAME)$(EXE_EXT) : $(OUT_BIN)/_$(APP_NAME)$(EXE_EXT)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(COPY) $(subst /,$(PS),$(BIN_DIR)/unoapploader$(EXE_EXT)) $(subst /,$(PS),$@)
# workaround for touch problem under Windows with full qualified paths
	make -t $@

SimpleBootstrapCppExample : $(OUT_BIN)/$(APP_NAME)$(EXE_EXT)
	@echo --------------------------------------------------------------------------------
	@echo Please use the following command to execute the example!
	@echo -
	@echo $(MAKE) SimpleBootstrap_cpp.run
	@echo --------------------------------------------------------------------------------	

%.run: $(OUT_BIN)/SimpleBootstrap_cpp$(EXE_EXT)
	cd $(subst /,$(PS),$(OUT_BIN)) && $(basename $@)

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_APP_INC))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_APP_GEN))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_APP_OBJ))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(OUT_BIN)/*SimpleBootstrap*))
