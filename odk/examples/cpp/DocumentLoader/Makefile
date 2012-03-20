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

# Builds the C++ DocumentLoader example of the SDK.

PRJ=../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk
include $(SETTINGS)/dk.mk

# Define non-platform/compiler specific settings
COMPONENT_NAME=DocumentLoader

OUT_COMP_INC = $(OUT_INC)/$(COMPONENT_NAME)
OUT_COMP_GEN = $(OUT_MISC)/$(COMPONENT_NAME)
OUT_COMP_OBJ=$(OUT_OBJ)/$(COMPONENT_NAME)

CXXFILES = DocumentLoader.cxx

OBJFILES = $(patsubst %.cxx,$(OUT_SLO_COMP)/%.$(OBJ_EXT),$(CXXFILES))

ENV_OFFICE_TYPES=-env:URE_MORE_TYPES=$(URLPREFIX)$(OFFICE_TYPES)

# Targets
.PHONY: ALL
ALL : \
	CppDocumentLoaderExample

include $(SETTINGS)/stdtarget.mk

$(OUT_COMP_OBJ)/%.$(OBJ_EXT) : %.cxx $(SDKTYPEFLAG)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(CC) $(CC_FLAGS) $(CC_INCLUDES) -I$(OUT_COMP_INC) $(CC_DEFINES) $(CC_OUTPUT_SWITCH)$(subst /,$(PS),$@) $<

$(OUT_BIN)/DocumentLoader$(EXE_EXT) : $(OUT_COMP_OBJ)/DocumentLoader.$(OBJ_EXT)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	-$(MKDIR) $(subst /,$(PS),$(OUT_COMP_GEN))
ifeq "$(OS)" "WIN"
	$(LINK) $(EXE_LINK_FLAGS) /OUT:$@ /MAP:$(OUT_COMP_GEN)/$(basename $(@F)).map \
	  $< $(CPPUHELPERLIB) $(CPPULIB) $(SALHELPERLIB) $(SALLIB) $(STLPORTLIB)
else
	$(LINK) $(EXE_LINK_FLAGS) $(LINK_LIBS) -o $@ $< \
	  $(CPPUHELPERLIB) $(CPPULIB) $(SALHELPERLIB) $(SALLIB) $(STLPORTLIB) $(STDC++LIB) $(CPPUHELPERDYLIB) $(CPPUDYLIB) $(SALHELPERDYLIB) $(SALDYLIB)
ifeq "$(OS)" "MACOSX"
	$(INSTALL_NAME_URELIBS_BIN)  $@
endif
endif

CppDocumentLoaderExample : $(OUT_BIN)/DocumentLoader$(EXE_EXT)
	@echo --------------------------------------------------------------------------------
	@echo The example loads the "$(QM)test.odt$(QM)" document in the DocumentLoader example directory.
	@echo If you want to load your own document, please use: 
	@echo $(SQM)  $(SQM)DocumentLoader -env:URE_MORE_TYPES="$(QM)<fileurl_office_types_rdb>$(QM)" "$(QM)filename$(QM)" [connection_url]
	@echo -
	@echo Use the following command to execute the example!
	@echo -
	@echo $(MAKE) DocumentLoader.run
	@echo -
	@echo NOTE: This example does not use the new UNO bootstrap mechanism, it uses still a socket
	@echo $(SQM)      $(SQM)connection. The example use the defaultBootstrap_InitialComponentContext method and provides 
	@echo $(SQM)      $(SQM)the additional office types via the UNO environment variable -env:URE_MORE_TYPES=... 
	@echo $(SQM)      $(SQM)Before you can run this example you have to start your office in listening mode.
	@echo -
	@echo $(SQM)  $(SQM)soffice "$(QM)-accept=socket,host=localhost,port=2083;urp;StarOffice.ServiceManager$(QM)"
	@echo --------------------------------------------------------------------------------

%.run: $(OUT_BIN)/DocumentLoader$(EXE_EXT)
	cd $(subst /,$(PS),$(OUT_BIN)) && $(basename $@) $(ENV_OFFICE_TYPES) $(subst \\,/,$(subst /,$(PS),"$(OO_SDK_HOME)/examples/cpp/DocumentLoader/test.odt"))

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_INC))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_GEN))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_OBJ))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(OUT_BIN)/DocumentLoader*))
