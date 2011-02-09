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

CXXFILES = DocumentLoader.cxx

OBJFILES = $(patsubst %.cxx,$(OUT_SLO_COMP)/%.$(OBJ_EXT),$(CXXFILES))

# Targets
.PHONY: ALL
ALL : \
	CppDocumentLoaderExample

include $(SETTINGS)/stdtarget.mk

# This example type library will be extended by the URE and office types
$(OUT_BIN)/%.rdb :
	-$(MKDIR) $(subst /,$(PS),$(@D))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	$(REGMERGE) $@ / $(URE_TYPES) $(OFFICE_TYPES) 

$(OUT_COMP_OBJ)/%.$(OBJ_EXT) : %.cxx $(SDKTYPEFLAG)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(CC) $(CC_FLAGS) $(CC_INCLUDES) -I$(OUT_COMP_INC) $(CC_DEFINES) $(CC_OUTPUT_SWITCH)$(subst /,$(PS),$@) $<

$(OUT_BIN)/DocumentLoader$(EXE_EXT) : $(OUT_COMP_OBJ)/DocumentLoader.$(OBJ_EXT)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	-$(MKDIR) $(subst /,$(PS),$(OUT_COMP_GEN))
ifeq "$(OS)" "WIN"
	$(LINK) $(EXE_LINK_FLAGS) /OUT:$@ /MAP:$(OUT_COMP_GEN)/$(basename $(@F)).map \
	  $< $(CPPUHELPERLIB) $(CPPULIB) $(SALHELPERLIB) $(SALLIB)
else
	$(LINK) $(EXE_LINK_FLAGS) $(LINK_LIBS) -o $@ $< \
	  $(CPPUHELPERLIB) $(CPPULIB) $(SALHELPERLIB) $(SALLIB) $(STDC++LIB) $(CPPUHELPERDYLIB) $(CPPUDYLIB) $(SALHELPERDYLIB) $(SALDYLIB)
ifeq "$(OS)" "MACOSX"
	$(INSTALL_NAME_URELIBS_BIN)  $@
endif
endif

$(COMPOENNT_ENV_FLAG) : $(COMPONENT_RDB)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	@echo --------------------------------------------------------------------------------
	@echo        Register necessary runtime components in the DocumentLoader.rdb 
	@echo --------------------------------------------------------------------------------
	$(REGCOMP) -register -r $(COMPONENT_RDB) -c connector.uno.$(SHAREDLIB_EXT)
	$(REGCOMP) -register -r $(COMPONENT_RDB) -c remotebridge.uno.$(SHAREDLIB_EXT)
	$(REGCOMP) -register -r $(COMPONENT_RDB) -c bridgefac.uno.$(SHAREDLIB_EXT)
	$(REGCOMP) -register -r $(COMPONENT_RDB) -c uuresolver.uno.$(SHAREDLIB_EXT)
	@echo bla > $@

CppDocumentLoaderExample : $(OUT_BIN)/DocumentLoader$(EXE_EXT) $(COMPOENNT_ENV_FLAG)
	@echo --------------------------------------------------------------------------------
	@echo The example loads the "$(QM)test.odt$(QM)" document in the DocumentLoader example directory.
	@echo I you want to load your own document, please use: DocumentLoader "$(QM)filename$(QM)" [connection_url]
	@echo -
	@echo Use the following command to execute the example!
	@echo -
	@echo $(MAKE) DocumentLoader.run
	@echo -
	@echo NOTE: This example does not use the new UNO bootstrap mechanism, it uses still a socket
	@echo $(SQM)      $(SQM)connection. Before you can run this example you have to start your office in listening mode. 
	@echo -
	@echo $(SQM)  $(SQM)soffice "$(QM)-accept=socket,host=localhost,port=2083;urp;StarOffice.ServiceManager$(QM)"
	@echo --------------------------------------------------------------------------------

%.run: $(OUT_BIN)/DocumentLoader$(EXE_EXT)
	cd $(subst /,$(PS),$(OUT_BIN)) && $(basename $@) $(subst \\,/,$(subst /,$(PS),"$(OO_SDK_HOME)/examples/cpp/DocumentLoader/test.odt"))

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_INC))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_GEN))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_OBJ))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(COMPOENNT_ENV_FLAG)))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(COMPONENT_RDB)))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(OUT_BIN)/DocumentLoader*))
