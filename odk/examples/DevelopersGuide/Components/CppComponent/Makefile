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

# Builds the C++ component example of the Developers Guide.

PRJ=../../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk

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

COMP_RDB_NAME = $(COMP_NAME).uno.rdb
COMP_RDB = $(SAMPLE_GEN_OUT)/$(COMP_RDB_NAME)
COMP_PACKAGE = $(OUT_BIN)/$(COMP_NAME).$(UNOOXT_EXT)
COMP_PACKAGE_URL = $(subst \\,\,"$(COMP_PACKAGE_DIR)$(PS)$(COMP_NAME).$(UNOOXT_EXT)")
COMP_UNOPKG_MANIFEST = $(SAMPLE_GEN_OUT)/$(COMP_NAME)/META-INF/manifest.xml
COMP_COMPONENTS = $(SAMPLE_GEN_OUT)/$(COMP_NAME).components

COMP_REGISTERFLAG = $(SAMPLE_GEN_OUT)/devguide_$(COMP_NAME)_register_component.flag
COMP_TYPEFLAG = $(SAMPLE_GEN_OUT)/devguide_$(COMP_NAME)_types.flag

IDLFILES = some.idl

CXXFILES = service1_impl.cxx \
	service2_impl.cxx

SLOFILES = $(patsubst %.cxx,$(SAMPLE_SLO_OUT)/%.$(OBJ_EXT),$(CXXFILES))

GENURDFILES = $(patsubst %.idl,$(SAMPLE_GEN_OUT)/%.urd,$(IDLFILES))

TYPELIST=-Tmy_module.XSomething  \
	-Tmy_modules.MyService1 \
	-Tmy_module.MyService2
 
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

$(COMP_TYPEFLAG) : $(COMP_RDB) $(SDKTYPEFLAG)
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(CPPUMAKER) -Gc -O$(SAMPLE_INC_OUT) $(TYPESLIST) $(COMP_RDB) -X$(URE_TYPES) -X$(OFFICE_TYPES)
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
	$(CPPUHELPERLIB) $(CPPULIB) $(SALLIB) msvcprt.lib $(LIBO_SDK_LDFLAGS_STDLIBS)
	$(LINK_MANIFEST)
else
$(SHAREDLIB_OUT)/%.$(SHAREDLIB_EXT) : $(SLOFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D)) && $(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	$(LINK) $(COMP_LINK_FLAGS) $(LINK_LIBS) -o $@ $(SLOFILES) \
	$(CPPUHELPERLIB) $(CPPULIB) $(SALLIB) $(STC++LIB)
ifeq "$(OS)" "MACOSX"
	$(INSTALL_NAME_URELIBS)  $@
endif
endif	

# rule for component package manifest
$(SAMPLE_GEN_OUT)/%/manifest.xml :
	-$(MKDIR) $(subst /,$(PS),$(@D))
	@echo $(OSEP)?xml version="$(QM)1.0$(QM)" encoding="$(QM)UTF-8$(QM)"?$(CSEP) > $@
	@echo $(OSEP)!DOCTYPE manifest:manifest PUBLIC "$(QM)-//OpenOffice.org//DTD Manifest 1.0//EN$(QM)" "$(QM)Manifest.dtd$(QM)"$(CSEP) >> $@
	@echo $(OSEP)manifest:manifest xmlns:manifest="$(QM)http://openoffice.org/2001/manifest$(QM)"$(CSEP) >> $@
	@echo $(SQM)  $(SQM)$(OSEP)manifest:file-entry manifest:media-type="$(QM)application/vnd.sun.star.uno-typelibrary;type=RDB$(QM)" >> $@
	@echo $(SQM)                       $(SQM)manifest:full-path="$(QM)$(subst /META-INF,,$(subst $(SAMPLE_GEN_OUT)/,,$(@D))).uno.rdb$(QM)"/$(CSEP) >> $@
	@echo $(SQM)  $(SQM)$(OSEP)manifest:file-entry manifest:media-type="$(QM)application/vnd.sun.star.uno-components;platform=$(UNOPKG_PLATFORM)$(QM)">> $@
	@echo $(SQM)                       $(SQM)manifest:full-path="$(QM)$(COMP_NAME).components$(QM)"/$(CSEP)>> $@
	@echo $(OSEP)/manifest:manifest$(CSEP) >> $@

$(COMP_COMPONENTS) :
	-$(MKDIR) $(subst /,$(PS),$(@D))
	@echo $(OSEP)?xml version="$(QM)1.0$(QM)" encoding="$(QM)UTF-8$(QM)"?$(CSEP) > $@
	@echo $(OSEP)components xmlns="$(QM)http://openoffice.org/2010/uno-components$(QM)"$(CSEP) >> $@
	@echo $(SQM)  $(SQM)$(OSEP)component loader="$(QM)com.sun.star.loader.SharedLibrary$(QM)" uri="$(QM)$(UNOPKG_PLATFORM)/$(COMP_IMPL_NAME)$(QM)"$(CSEP) >> $@
	@echo $(SQM)    $(SQM)$(OSEP)implementation name="$(QM)my_module.my_sc_implementation.MyService1$(QM)"$(CSEP) >> $@
	@echo $(SQM)      $(SQM)$(OSEP)service name="$(QM)my_module.MyService1$(QM)"/$(CSEP) >> $@
	@echo $(SQM)    $(SQM)$(OSEP)/implementation$(CSEP) >> $@
	@echo $(SQM)    $(SQM)$(OSEP)implementation name="$(QM)my_module.my_sc_implementation.MyService2$(QM)"$(CSEP) >> $@
	@echo $(SQM)      $(SQM)$(OSEP)service name="$(QM)my_module.MyService2$(QM)"/$(CSEP) >> $@
	@echo $(SQM)    $(SQM)$(OSEP)/implementation$(CSEP) >> $@
	@echo $(SQM)  $(SQM)$(OSEP)/component$(CSEP) >> $@
	@echo $(OSEP)/components$(CSEP) >> $@

$(COMP_PACKAGE) : $(SHAREDLIB_OUT)/$(COMP_IMPL_NAME) $(COMP_RDB) $(COMP_UNOPKG_MANIFEST) $(COMP_COMPONENTS)
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))
	-$(MKDIR) $(subst /,$(PS),$(SAMPLE_GEN_OUT)/$(UNOPKG_PLATFORM))	 
	$(COPY) $(subst /,$(PS),$<) $(subst /,$(PS),$(SAMPLE_GEN_OUT)/$(UNOPKG_PLATFORM))
	cd $(subst /,$(PS),$(SAMPLE_GEN_OUT)) && $(SDK_ZIP) ../../bin/$(@F) $(COMP_NAME).components
	cd $(subst /,$(PS),$(SAMPLE_GEN_OUT)) && $(SDK_ZIP) -u ../../bin/$(@F) $(COMP_RDB_NAME) $(UNOPKG_PLATFORM)/$(<F)
	cd $(subst /,$(PS),$(SAMPLE_GEN_OUT)/$(subst .$(UNOOXT_EXT),,$(@F))) && $(SDK_ZIP) -u ../../../bin/$(@F) META-INF/manifest.xml


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
	  $< $(CPPUHELPERLIB) $(CPPULIB) $(SALHELPERLIB) $(SALLIB)
else
	$(LINK) $(EXE_LINK_FLAGS) $(LINK_LIBS) -o $@ $< \
	  $(CPPUHELPERLIB) $(CPPULIB) $(SALHELPERLIB) $(SALLIB) $(STDC++LIB)
ifeq "$(OS)" "MACOSX"
	$(INSTALL_NAME_URELIBS_BIN)  $@
endif
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
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(COMP_COMPONENTS)))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(OUT_BIN)/$(COMP_NAME)*))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(OUT_BIN)/*$(APP1_NAME)*))
