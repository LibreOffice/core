#*************************************************************************
#
#  $RCSfile: Makefile,v $
#
#  $Revision: 1.3 $
#
#  last change: $Author: hr $ $Date: 2003-06-30 15:12:55 $
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

# Builds the C++ component example of the Developers Guide.

PRJ=../../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk
include $(SETTINGS)/dk.mk

# Define non-platform/compiler specific settings
COMPONENT_NAME=CppComponent
COMPONENT_IMPL_NAME=$(COMPONENT_NAME).uno.$(SHAREDLIB_EXT) 
OUT_COMP_INC = $(OUT_INC)/$(COMPONENT_NAME)
OUT_COMP_GEN = $(OUT_MISC)/$(COMPONENT_NAME)
OUT_COMP_SLO=$(OUT_SLO)/$(COMPONENT_NAME)
COMPONENT_XML=$(COMPONENT_NAME).uno.xml
COMPONENT_RDB_NAME = $(COMPONENT_NAME).uno.rdb
COMPONENT_RDB = $(OUT_COMP_GEN)/$(COMPONENT_RDB_NAME)
COMPONENT_PACKAGE = $(OUT_BIN)/$(COMPONENT_NAME).uno.zip
COMPONENT_PACKAGE_URL = $(subst \\,\,"$(COMPONENT_PACKAGE_DIR)$(PS)$(COMPONENT_NAME).uno.zip")


REGISTERFLAG = $(OUT_MISC)/devguide_$(COMPONENT_NAME)_register_component.flag
COMPONENT_TYPEFLAG = $(OUT_MISC)/devguide_$(COMPONENT_NAME)_types.flag

IDLFILES = some.idl

CXXFILES = service1_impl.cxx \
	service2_impl.cxx

SLOFILES = $(patsubst %.cxx,$(OUT_COMP_SLO)/%.$(OBJ_EXT),$(CXXFILES))

GENURDFILES = $(patsubst %.idl,$(OUT_COMP_GEN)/%.urd,$(IDLFILES))

TYPES = $(shell xml2cmp -types stdout $(COMPONENT_XML))
TYPESLIST = $(foreach t,$(TYPES),-T$(t))
GENHPPFILES = $(foreach t,$(TYPES),$(OUT_COMP_INC)/$(subst .,/,$(t)).hpp)

# Targets
.PHONY: ALL
ALL : \
	CppComponentExample

include $(SETTINGS)/stdtarget.mk

$(OUT_COMP_GEN)/%.urd : %.idl
	-$(MKDIR) $(subst /,$(PS),$(@D))
	idlc -I. -I$(IDL_DIR) -O$(OUT_COMP_GEN) $<

$(OUT_COMP_GEN)/%.rdb : $(GENURDFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	-$(DEL) $(subst /,$(PS),$@)
	regmerge $@ /UCR $(GENURDFILES)

$(COMPONENT_TYPEFLAG) : $(COMPONENT_RDB) $(COMPONENT_XML)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	-$(DEL) $(subst /,$(PS),$(COMPONENT_TYPEFLAG))
	cppumaker -Gc -BUCR -O$(OUT_COMP_INC) $(TYPESLIST) $(COMPONENT_RDB) $(DKREGISTRYNAME)
	echo flagged > $@

$(OUT_COMP_SLO)/%.$(OBJ_EXT) : %.cxx $(COMPONENT_TYPEFLAG)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(CC) $(CC_FLAGS) $(CC_INCLUDES) -I$(OUT_COMP_INC) $(CC_DEFINES) $(CC_OUTPUT_SWITCH)$(subst /,$(PS),$@) $<

$(OUT_COMP_GEN)/%.def : exports.dxp
ifeq "$(OS)" "WIN"
	-$(MKDIR) $(subst /,$(PS),$(@D))
	@echo EXPORTS > $@
	$(CAT) exports.dxp >> $@
endif

ifeq "$(OS)" "WIN"
$(SHAREDLIB_OUT)/%.$(SHAREDLIB_EXT) : $(SLOFILES) $(OUT_COMP_GEN)/%.def
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(LINK) $(LIBRARY_LINK_FLAGS) /OUT:$@ /MAP:$(OUT_COMP_GEN)/$(subst $(SHAREDLIB_EXT),map,$(@F)) \
	/DEF:$(OUT_COMP_GEN)/$(subst $(SHAREDLIB_EXT),def,$(@F)) $(SLOFILES) \
	$(CPPUHELPERLIB) $(CPPULIB) $(SALLIB) $(STLPORTLIB) msvcrt.lib kernel32.lib
else
$(SHAREDLIB_OUT)/%.$(SHAREDLIB_EXT) : $(SLOFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(LINK) $(LIBRARY_LINK_FLAGS) $(LINK_LIBS) -o $@ $^\
	$(CPPUHELPERLIB) $(CPPULIB) $(SALLIB) $(STLPORTLIB) $(STC++LIB)
endif	

$(OUT_BIN)/%.zip : $(SHAREDLIB_OUT)/%.$(SHAREDLIB_EXT) $(OUT_COMP_GEN)/%.rdb
	-$(MKDIR) $(subst /,$(PS),$(@D))
	-$(MKDIR) $(subst /,$(PS),$(OUT_COMP_GEN)/$(PACKAGE_LIB_DIR))	 
	$(COPY) $(subst /,$(PS),$<) $(subst /,$(PS),$(OUT_COMP_GEN)/$(PACKAGE_LIB_DIR))
	cd $(OUT_COMP_GEN) && jar cvfM ../../bin/$(@F) $(basename $(@F)).rdb $(PACKAGE_LIB_DIR)/$(<F)

$(REGISTERFLAG) : $(COMPONENT_PACKAGE)
ifneq "$(SDK_AUTO_DEPLOYMENT)" ""
	-$(MKDIR) $(subst /,$(PS),$(@D))
	-$(DEL) $(subst /,$(PS),$@)
	$(DEPLOYTOOL) $(COMPONENT_PACKAGE_URL)
	@echo flagged > $(subst /,$(PS),$@)
else
	@echo --------------------------------------------------------------------------------
	@echo  If you want to install your component automatically, please set the environment
	@echo  variable SDK_AUTO_DEPLOYMENT = YES. But note that auto deployment is only 
	@echo  possible if no office instance is running. 
	@echo --------------------------------------------------------------------------------
endif

CppComponentExample : $(REGISTERFLAG) 
	@echo --------------------------------------------------------------------------------
	@echo The simple C++ component was installed if SDK_AUTO_DEPLOYMENT = YES.
	@echo You can use this component inside your office installation, see the example
	@echo description. You can also load the "$(QM)SimpleComponent.sxw$(QM)" document containing
	@echo a StarBasic macro which uses this component.
	@echo --------------------------------------------------------------------------------

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_INC))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_GEN))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_SLO))
	-$(DEL) $(subst /,$(PS),$(COMPONENT_PACKAGE_URL))
	-$(DEL) $(subst /,$(PS),$(REGISTERFLAG))
	-$(DEL) $(subst /,$(PS),$(COMPONENT_TYPEFLAG))
	-$(DEL) $(subst /,$(PS),$(SHAREDLIB_OUT)/$(COMPONENT_NAME).*)
