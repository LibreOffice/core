#*************************************************************************
#
#  $RCSfile: Makefile,v $
#
#  $Revision: 1.5 $
#
#  last change: $Author: rt $ $Date: 2004-05-18 13:27:51 $
#
#  The Contents of this file are made available subject to the terms of
#  the BSD license.
#  
#  Copyright (c) 2003 by Sun Microsystems, Inc.
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
include $(SETTINGS)/dk.mk

# Define non-platform/compiler specific settings
COMPONENT_NAME=ProfUnoCppBinding
COMPONENT_RDB=$(OUT_BIN)/office_connect.rdb
OUT_COMP_INC = $(OUT_INC)/$(COMPONENT_NAME)
OUT_COMP_GEN = $(OUT_MISC)/$(COMPONENT_NAME)
OUT_COMP_OBJ=$(OUT_OBJ)/$(COMPONENT_NAME)

COMPONENT_ENV_FLAG = $(OUT_MISC)/cpp_$(COMPONENT_NAME)_prepare_env.flag

ifeq "$(OS)" "WIN"
INI_EXTENSION=.ini
else
INI_EXTENSION=rc
endif

CXXFILES = \
	 office_connect.cxx \
	 string_samples.cxx

OBJFILES = $(patsubst %.cxx,$(OUT_SLO_COMP)/%.$(OBJ_EXT),$(CXXFILES))

TYPES := \
	com.sun.star.uno.XNamingService \
	com.sun.star.uno.XComponentContext \
	com.sun.star.uno.XWeak \
	com.sun.star.uno.XAggregation \
	com.sun.star.lang.XMain \
	com.sun.star.lang.XMultiServiceFactory \
	com.sun.star.lang.XSingleComponentFactory \
	com.sun.star.lang.XTypeProvider \
	com.sun.star.lang.XComponent \
	com.sun.star.registry.XSimpleRegistry \
	com.sun.star.registry.XImplementationRegistration \
	com.sun.star.bridge.XBridgeFactory \
	com.sun.star.bridge.XUnoUrlResolver \
	com.sun.star.container.XHierarchicalNameAccess

TYPESLIST = $(foreach t,$(TYPES),-T$(t))
GENHPPFILES = $(foreach t,$(TYPES),$(OUT_COMP_INC)/$(subst .,/,$(t)).hpp)


# Targets
.PHONY: ALL
ALL : \
    ProUNOCppBindingExample

include $(SETTINGS)/stdtarget.mk

$(OUT_BIN)/%$(INI_EXTENSION) : %$(INI_EXTENSION)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(COPY) $< $(subst /,$(PS),$@)

$(OUT_BIN)/%.rdb :
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(REGMERGE) $@ / $(DKREGISTRYNAME)

$(GENHPPFILES) :
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(CPPUMAKER) -Gc -BUCR -O$(OUT_COMP_INC) $(TYPESLIST) $(DKREGISTRYNAME)

$(OUT_COMP_OBJ)/%.$(OBJ_EXT) : %.cxx $(GENHPPFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(CC) $(CC_FLAGS) $(CC_INCLUDES) -I$(OUT_COMP_INC) $(CC_DEFINES) $(CC_OUTPUT_SWITCH)$(subst /,$(PS),$@) $<

$(OUT_BIN)/%$(EXE_EXT) : $(OUT_COMP_OBJ)/%.$(OBJ_EXT)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	-$(MKDIR) $(subst /,$(PS),$(OUT_COMP_GEN))
ifeq "$(OS)" "WIN"
	$(LINK) $(EXE_LINK_FLAGS) /OUT:$@ /MAP:$(OUT_COMP_GEN)/$(subst $(EXE_EXT),.map,$(@F)) \
	  $< $(CPPUHELPERLIB) $(CPPULIB) $(SALHELPERLIB) $(SALLIB) $(STLPORTLIB)
else
	$(LINK) $(EXE_LINK_FLAGS) $(LINK_LIBS) -o $@ $< \
	  $(CPPUHELPERLIB) $(CPPULIB) $(SALHELPERLIB) $(SALLIB) $(STLPORTLIB) $(STDC++LIB)
endif

$(OUT_BIN)/office_connect$(EXE_EXT) : $(OUT_COMP_OBJ)/office_connect.$(OBJ_EXT) 

$(OUT_BIN)/string_samples$(EXE_EXT) : $(OUT_COMP_OBJ)/string_samples.$(OBJ_EXT)

$(COMPONENT_ENV_FLAG) : $(COMPONENT_RDB)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	-$(DEL) $@
	@echo --------------------------------------------------------------------------------
	@echo   Register necessary runtime components in $(COMPONENT_RDB)
	@echo --------------------------------------------------------------------------------
	$(REGCOMP) -register -r $(COMPONENT_RDB) -c connector.uno.$(SHAREDLIB_EXT)
	$(REGCOMP) -register -r $(COMPONENT_RDB) -c remotebridge.uno.$(SHAREDLIB_EXT)
	$(REGCOMP) -register -r $(COMPONENT_RDB) -c bridgefac.uno.$(SHAREDLIB_EXT)
	$(REGCOMP) -register -r $(COMPONENT_RDB) -c uuresolver.uno.$(SHAREDLIB_EXT)
	@echo bla > $@


ProUNOCppBindingExample : $(OUT_BIN)/office_connect$(EXE_EXT) $(OUT_BIN)/string_samples$(EXE_EXT) $(COMPONENT_ENV_FLAG) $(OUT_BIN)/office_connect$(INI_EXTENSION)
	@echo --------------------------------------------------------------------------------
	@echo Please use one of the following commands to execute the examples!
	@echo -
	@echo make office_connect.run
	@echo make string_samples.run
	@echo --------------------------------------------------------------------------------

%.run: $(OUT_BIN)/%$(EXE_EXT) 
	cd $(subst /,$(PS),$(OUT_BIN)) && $(basename $@)

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_INC))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_GEN))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_OBJ))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(OUT_BIN)/office_connect*))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(OUT_BIN)/string_samples.*))
