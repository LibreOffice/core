#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

# Builds the C++ Convertor example of the SDK.

PRJ=../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk

# Define non-platform/compiler specific settings
COMPONENT_NAME=Convertor

OUT_COMP_INC = $(OUT_INC)/$(COMPONENT_NAME)
OUT_COMP_GEN = $(OUT_MISC)/$(COMPONENT_NAME)
OUT_COMP_OBJ=$(OUT_OBJ)/$(COMPONENT_NAME)

CXXFILES = Convertor.cxx

OBJFILES = $(patsubst %.cxx,$(OUT_SLO_COMP)/%.$(OBJ_EXT),$(CXXFILES))

ENV_OFFICE_TYPES=-env:URE_MORE_TYPES=$(URLPREFIX)$(OFFICE_TYPES)

# Targets
.PHONY: ALL
ALL : \
	CppConvertorExample

include $(SETTINGS)/stdtarget.mk

$(OUT_COMP_OBJ)/%.$(OBJ_EXT) : %.cxx $(SDKTYPEFLAG)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(CC) $(CC_FLAGS) $(CC_INCLUDES) -I$(OUT_COMP_INC) $(CC_DEFINES) $(CC_OUTPUT_SWITCH)$(subst /,$(PS),$@) $<

$(OUT_BIN)/Convertor$(EXE_EXT) : $(OUT_COMP_OBJ)/Convertor.$(OBJ_EXT)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	-$(MKDIR) $(subst /,$(PS),$(OUT_COMP_GEN))
ifeq "$(OS)" "WIN"
	$(LINK) $(EXE_LINK_FLAGS) /OUT:$@ /MAP:$(OUT_COMP_GEN)/$(basename $(@F)).map \
	  $< $(CPPUHELPERLIB) $(CPPULIB) $(SALHELPERLIB) $(SALLIB)
else
	$(LINK) $(EXE_LINK_FLAGS) $(LINK_LIBS) -o $@ $< \
	  $(CPPUHELPERLIB) $(CPPULIB) $(SALHELPERLIB) $(SALLIB) $(STDC++LIB)
ifeq "$(OS)" "MACOSX"
	$(INSTALL_NAME_URELIBS_BIN)  $@
endif
endif

CppConvertorExample : $(OUT_BIN)/Convertor$(EXE_EXT)
	@echo --------------------------------------------------------------------------------
	@echo The example loads the "$(QM)test.odt$(QM)" document in the Convertor example directory
	@echo   and convert it to test.pdf
	@echo -
	@echo Use the following command to execute the example!
	@echo -
	@echo $(MAKE) Convertor.run
	@echo -
	@echo NOTE: This example does not use the new UNO bootstrap mechanism, it uses still a socket
	@echo $(SQM)      $(SQM)connection. The example use the defaultBootstrap_InitialComponentContext method and provides
	@echo $(SQM)      $(SQM)the additional office types via the UNO environment variable -env:URE_MORE_TYPES=...
	@echo $(SQM)      $(SQM)Before you can run this example you have to start your office in listening mode.
	@echo -
	@echo $(SQM)  $(SQM)soffice "$(QM)--accept=socket,host=localhost,port=2083;urp;StarOffice.ServiceManager$(QM)"
	@echo --------------------------------------------------------------------------------

%.run: $(OUT_BIN)/Convertor$(EXE_EXT)
	cd $(subst /,$(PS),$(OUT_BIN)) && $(basename $@) $(ENV_OFFICE_TYPES) $(subst \\,/,$(subst /,$(PS),"$(OO_SDK_HOME)/examples/cpp/Convertor/test.odt"))

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_INC))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_GEN))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_OBJ))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(OUT_BIN)/Convertor*))
