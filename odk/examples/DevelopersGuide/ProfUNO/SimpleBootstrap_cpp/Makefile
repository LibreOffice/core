#*************************************************************************
#
#  The Contents of this file are made available subject to the terms of
#  the BSD license.
#  
#  Copyright 2000, 2010 Oracle and/or its affiliates.
#  All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
#  1. Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#  2. Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#  3. Neither the name of Sun Microsystems, Inc. nor the names of its
#     contributors may be used to endorse or promote products derived
#     from this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
#  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
#  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
#  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
#  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
#  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
#  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
#  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
#  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#     
#**************************************************************************

# Builds the SimpleBootstrap_cpp example of the SDK.

PRJ=../../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk

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
