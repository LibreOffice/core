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

# Builds the ProfUNO cpp binding example of the Developers Guide

PRJ=../../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk

# Define non-platform/compiler specific settings
SAMPLE_NAME=ProfUnoCppBinding
SAMPLE_INC_OUT = $(OUT_INC)/$(SAMPLE_NAME)
SAMPLE_GEN_OUT = $(OUT_MISC)/$(SAMPLE_NAME)
SAMPLE_OBJ_OUT=$(OUT_OBJ)/$(SAMPLE_NAME)

APP1_NAME=office_connect
APP1_BINARY=$(OUT_BIN)/$(APP1_NAME)$(EXE_EXT)

APP2_NAME=string_samples
APP2_BINARY=$(OUT_BIN)/$(APP2_NAME)$(EXE_EXT)

ENV_OFFICE_TYPES=-env:URE_MORE_TYPES=$(URLPREFIX)$(OFFICE_TYPES)

CXXFILES = \
	 office_connect.cxx \
	 string_samples.cxx

# Targets
.PHONY: ALL
ALL : \
    ProUNOCppBindingExample

include $(SETTINGS)/stdtarget.mk

$(SAMPLE_OBJ_OUT)/%.$(OBJ_EXT) : %.cxx $(SDKTYPEFLAG)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(CC) $(CC_FLAGS) $(CC_INCLUDES) -I$(SAMPLE_INC_OUT) $(CC_DEFINES) $(CC_OUTPUT_SWITCH)$(subst /,$(PS),$@) $<

$(OUT_BIN)/%$(EXE_EXT) : $(SAMPLE_OBJ_OUT)/%.$(OBJ_EXT)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	-$(MKDIR) $(subst /,$(PS),$(SAMPLE_GEN_OUT))
ifeq "$(OS)" "WIN"
	$(LINK) $(EXE_LINK_FLAGS) /OUT:$@ /MAP:$(SAMPLE_GEN_OUT)/$(subst $(EXE_EXT),.map,$(@F)) \
	  $< $(CPPUHELPERLIB) $(CPPULIB) $(SALHELPERLIB) $(SALLIB)
else
	$(LINK) $(EXE_LINK_FLAGS) $(LINK_LIBS) -o $@ $< \
	  $(CPPUHELPERLIB) $(CPPULIB) $(SALHELPERLIB) $(SALLIB) $(STDC++LIB)
ifeq "$(OS)" "MACOSX"
	$(INSTALL_NAME_URELIBS_BIN)  $@
endif
endif

$(OUT_BIN)/office_connect$(EXE_EXT) : $(SAMPLE_OBJ_OUT)/office_connect.$(OBJ_EXT) 

$(OUT_BIN)/string_samples$(EXE_EXT) : $(SAMPLE_OBJ_OUT)/string_samples.$(OBJ_EXT)

ProUNOCppBindingExample : $(APP1_BINARY) $(APP2_BINARY)
	@echo --------------------------------------------------------------------------------
	@echo Note: For the "$(QM)$(APP1_NAME)$(QM)" example you need a running office listening
	@echo $(SQM)      $(SQM)on port 2083. The example use the defaultBootstrap_InitialComponentContext method and provides 
	@echo $(SQM)      $(SQM)the additional office types via the UNO environment variable -env:URE_MORE_TYPES=... 
	@echo $(SQM)      $(SQM)Before you can run this example you have to start your office in listening mode.
	@echo -
	@echo $(SQM)  $(SQM)soffice "$(QM)--accept=socket,host=localhost,port=2083;urp;StarOffice.ServiceManager$(QM)"
	@echo -----
	@echo Please use one of the following commands to execute the examples!
	@echo -
	@echo $(MAKE) $(APP1_NAME).run
	@echo $(MAKE) $(APP2_NAME).run
	@echo --------------------------------------------------------------------------------

%.run: $(OUT_BIN)/%$(EXE_EXT) 
	cd $(subst /,$(PS),$(OUT_BIN)) && $(basename $@)

$(API1_NAME).run: $(API1_BINARY) 
	cd $(subst /,$(PS),$(OUT_BIN)) && $(basename $@) $(ENV_OFFICE_TYPES)

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(SAMPLE_INC_OUT))
	-$(DELRECURSIVE) $(subst /,$(PS),$(SAMPLE_GEN_OUT))
	-$(DELRECURSIVE) $(subst /,$(PS),$(SAMPLE_OBJ_OUT))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(OUT_BIN)/$(APP1_NAME)*))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(OUT_BIN)/$(APP2_NAME)*))
