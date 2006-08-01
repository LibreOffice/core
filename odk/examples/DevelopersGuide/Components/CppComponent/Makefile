#*************************************************************************
#
#  $RCSfile: Makefile,v $
#
#  $Revision: 1.8 $
#
#  last change: $Author: ihi $ $Date: 2006-08-01 16:31:40 $
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
SAMPLE_NAME=CppComponentSample
SAMPLE_INC_OUT=$(OUT_INC)/$(SAMPLE_NAME)
SAMPLE_GEN_OUT=$(OUT_MISC)/$(SAMPLE_NAME)
SAMPLE_SLO_OUT=$(OUT_SLO)/$(SAMPLE_NAME)
SAMPLE_OBJ_OUT=$(OUT_OBJ)/$(SAMPLE_NAME)

COMP_NAME=CppComponent
COMP_IMPL_NAME=$(COMP_NAME).uno.$(SHAREDLIB_EXT) 

APP1_NAME= TestCppComponent
APP1_BINARY= $(OUT_BIN)/$(APP1_NAME)$(EXE_EXT)

COMP_XML=$(COMP_NAME).uno.xml
COMP_RDB_NAME = $(COMP_NAME).uno.rdb
COMP_RDB = $(SAMPLE_GEN_OUT)/$(COMP_RDB_NAME)
COMP_PACKAGE = $(OUT_BIN)/$(COMP_NAME).$(UNOPKG_EXT)
COMP_PACKAGE_URL = $(subst \\,\,"$(COMP_PACKAGE_DIR)$(PS)$(COMP_NAME).$(UNOPKG_EXT)")
COMP_UNOPKG_MANIFEST = $(SAMPLE_GEN_OUT)/$(COMP_NAME)/META-INF/manifest.xml

COMP_REGISTERFLAG = $(SAMPLE_GEN_OUT)/devguide_$(COMP_NAME)_register_component.flag
COMP_TYPEFLAG = $(SAMPLE_GEN_OUT)/devguide_$(COMP_NAME)_types.flag

IDLFILES = some.idl

CXXFILES = service1_impl.cxx \
	service2_impl.cxx

SLOFILES = $(patsubst %.cxx,$(SAMPLE_SLO_OUT)/%.$(OBJ_EXT),$(CXXFILES))

GENURDFILES = $(patsubst %.idl,$(SAMPLE_GEN_OUT)/%.urd,$(IDLFILES))

TYPES = $(shell $(XML2CMP) -types stdout $(COMP_XML))

# some more types necessary for the demo application
TYPES += \
	com.sun.star.lang.XComponent \
	com.sun.star.bridge.XUnoUrlResolver \
	com.sun.star.frame.XComponentLoader \
	com.sun.star.container.XHierarchicalNameAccess \
	com.sun.star.uno.DeploymentException \
	com.sun.star.registry.XSimpleRegistry


TYPESLIST = $(foreach t,$(TYPES),-T$(t))
GENHPPFILES = $(foreach t,$(TYPES),$(SAMPLE_INC_OUT)/$(subst .,/,$(t)).hpp)

# Targets
.PHONY: ALL
ALL : \
	$(SAMPLE_NAME)

include $(SETTINGS)/stdtarget.mk

$(SAMPLE_GEN_OUT)/%.urd : %.idl
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(IDLC) -I. -I$(IDL_DIR) -O$(SAMPLE_GEN_OUT) $<

$(SAMPLE_GEN_OUT)/%.rdb : $(GENURDFILES)
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(REGMERGE) $@ /UCR $(GENURDFILES)

$(COMP_TYPEFLAG) : $(COMP_RDB) $(COMP_XML)
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(CPPUMAKER) -Gc -BUCR -O$(SAMPLE_INC_OUT) $(TYPESLIST) $(COMP_RDB) $(OFFICE_TYPE_LIBRARY)
	echo flagged > $@

$(SAMPLE_SLO_OUT)/%.$(OBJ_EXT) : %.cxx $(COMP_TYPEFLAG)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(CC) $(CC_FLAGS) $(CC_INCLUDES) -I$(SAMPLE_INC_OUT) $(CC_DEFINES) $(CC_OUTPUT_SWITCH)$(subst /,$(PS),$@) $<

ifeq "$(OS)" "WIN"
$(SHAREDLIB_OUT)/%.$(SHAREDLIB_EXT) : $(SLOFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	-$(MKDIR) $(subst /,$(PS),$(SAMPLE_GEN_OUT))
	$(LINK) $(COMP_LINK_FLAGS) /OUT:$@ \
	/MAP:$(SAMPLE_GEN_OUT)/$(subst $(SHAREDLIB_EXT),map,$(@F)) $(SLOFILES) \
	$(CPPUHELPERLIB) $(CPPULIB) $(SALLIB) $(STLPORTLIB) msvcrt.lib kernel32.lib
else
$(SHAREDLIB_OUT)/%.$(SHAREDLIB_EXT) : $(SLOFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D)) && $(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	$(LINK) $(COMP_LINK_FLAGS) $(LINK_LIBS) -o $@ $^ \
	$(CPPUHELPERLIB) $(CPPULIB) $(SALLIB) $(STLPORTLIB) $(STC++LIB)
endif	

# rule for component package manifest
$(SAMPLE_GEN_OUT)/%/manifest.xml :
	-$(MKDIR) $(subst /,$(PS),$(@D))
	@echo $(OSEP)?xml version="$(QM)1.0$(QM)" encoding="$(QM)UTF-8$(QM)"?$(CSEP) > $@
	@echo $(OSEP)!DOCTYPE manifest:manifest PUBLIC "$(QM)-//OpenOffice.org//DTD Manifest 1.0//EN$(QM)" "$(QM)Manifest.dtd$(QM)"$(CSEP) >> $@
	@echo $(OSEP)manifest:manifest xmlns:manifest="$(QM)http://openoffice.org/2001/manifest$(QM)"$(CSEP) >> $@
	@echo $(SQM)  $(SQM)$(OSEP)manifest:file-entry manifest:media-type="$(QM)application/vnd.sun.star.uno-typelibrary;type=RDB$(QM)" >> $@
	@echo $(SQM)                       $(SQM)manifest:full-path="$(QM)$(subst /META-INF,,$(subst $(SAMPLE_GEN_OUT)/,,$(@D))).uno.rdb$(QM)"/$(CSEP) >> $@
	@echo $(SQM)  $(SQM)$(OSEP)manifest:file-entry manifest:media-type="$(QM)application/vnd.sun.star.uno-component;type=native;platform=$(UNOPKG_PLATFORM)$(QM)" >> $@
	@echo $(SQM)                       $(SQM)manifest:full-path="$(QM)$(subst /META-INF,,$(subst $(SAMPLE_GEN_OUT)/,,$(UNOPKG_PLATFORM)/$(@D))).uno.$(SHAREDLIB_EXT)$(QM)"/$(CSEP) >> $@
	@echo $(OSEP)/manifest:manifest$(CSEP) >> $@

$(COMP_PACKAGE) : $(SHAREDLIB_OUT)/$(COMP_IMPL_NAME) $(COMP_RDB) $(COMP_UNOPKG_MANIFEST)
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	-$(MKDIR) $(subst /,$(PS),$(SAMPLE_GEN_OUT)/$(UNOPKG_PLATFORM))	 
	$(COPY) $(subst /,$(PS),$<) $(subst /,$(PS),$(SAMPLE_GEN_OUT)/$(UNOPKG_PLATFORM))
	cd $(subst /,$(PS),$(SAMPLE_GEN_OUT)) && $(SDK_ZIP) ../../bin/$(@F) $(basename $(@F)).rdb $(UNOPKG_PLATFORM)/$(<F)
	cd $(subst /,$(PS),$(SAMPLE_GEN_OUT)/$(subst .$(UNOPKG_EXT),,$(@F))) && $(SDK_ZIP) -u ../../../bin/$(@F) META-INF/manifest.xml


$(COMP_REGISTERFLAG) : $(COMP_PACKAGE)
ifeq "$(SDK_AUTO_DEPLOYMENT)" "YES"
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(DEPLOYTOOL) $(COMP_PACKAGE_URL)
	@echo flagged > $(subst /,$(PS),$@)
else
	@echo --------------------------------------------------------------------------------
	@echo  If you want to install your component automatically, please set the environment
	@echo  variable SDK_AUTO_DEPLOYMENT = YES. But note that auto deployment is only 
	@echo  possible if no office instance is running. 
	@echo --------------------------------------------------------------------------------
endif

$(SAMPLE_OBJ_OUT)/$(APP1_NAME).$(OBJ_EXT) : $(APP1_NAME).cxx $(COMP_TYPEFLAG)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(CC) $(CC_FLAGS) $(CC_INCLUDES) -I$(SAMPLE_INC_OUT) $(CC_DEFINES) $(CC_OUTPUT_SWITCH)$(subst /,$(PS),$@) $<

$(OUT_BIN)/_$(APP1_NAME)$(EXE_EXT) : $(SAMPLE_OBJ_OUT)/$(APP1_NAME).$(OBJ_EXT)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	-$(MKDIR) $(subst /,$(PS),$(SAMPLE_GEN_OUT))
ifeq "$(OS)" "WIN"
	$(LINK) $(EXE_LINK_FLAGS) /OUT:$@ /MAP:$(SAMPLE_GEN_OUT)/$(basename $(@F)).map \
	  $< $(CPPUHELPERLIB) $(CPPULIB) $(SALHELPERLIB) $(SALLIB) $(STLPORTLIB)
else
	$(LINK) $(EXE_LINK_FLAGS) $(LINK_LIBS) -o $@ $< \
	  $(CPPUHELPERLIB) $(CPPULIB) $(SALHELPERLIB) $(SALLIB) $(STLPORTLIB) $(STDC++LIB)
endif

$(OUT_BIN)/$(APP1_NAME)$(EXE_EXT) : $(OUT_BIN)/_$(APP1_NAME)$(EXE_EXT)
	$(COPY) $(subst /,$(PS),$(BIN_DIR)/unoapploader$(EXE_EXT)) $(subst /,$(PS),$@)
# touch the target to renew the date for correct dependencies.
# Note: no touch under windows! The unoapploader.exe is copied always.
ifneq "$(OS)" "WIN"
	touch $@
endif

$(SAMPLE_NAME) : $(COMP_REGISTERFLAG) $(APP1_BINARY)
	@echo --------------------------------------------------------------------------------
	@echo Please use the following command to execute the example!
	@echo -
	@echo $(MAKE) $(APP1_NAME).run
	@echo --------
	@echo The simple C++ component was installed if SDK_AUTO_DEPLOYMENT = YES.
	@echo You can use this component inside your office installation, see the example
	@echo description. You can also load the "$(QM)SimpleComponent.odt$(QM)" document containing
	@echo a StarBasic macro which uses this component.
	@echo -
	@echo $(MAKE) SimpleComponent.odt.load
	@echo --------------------------------------------------------------------------------

%.run: $(OUT_BIN)/%$(EXE_EXT)
	$(subst /,$(PS),$(OUT_BIN))$(PS)$(basename $@)
#	cd $(subst /,$(PS),$(OUT_BIN)) && $(basename $@)

SimpleComponent.odt.load : $(COMP_REGISTERFLAG)
	"$(OFFICE_PROGRAM_PATH)$(PS)soffice" $(basename $@)

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(SAMPLE_INC_OUT))
	-$(DELRECURSIVE) $(subst /,$(PS),$(SAMPLE_GEN_OUT))
	-$(DELRECURSIVE) $(subst /,$(PS),$(SAMPLE_SLO_OUT))
	-$(DELRECURSIVE) $(subst /,$(PS),$(SAMPLE_OBJ_OUT))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(OUT_BIN)/$(COMP_NAME)*))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(OUT_BIN)/*$(APP1_NAME)*))
