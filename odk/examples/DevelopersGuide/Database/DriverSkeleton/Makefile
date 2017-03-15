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

# Builds the Database SDBC driver skeleton example of the Developers Guide.

PRJ=../../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk

# Define non-platform/compiler specific settings
COMP_NAME=DatabaseSDBCDriverSkeleton
COMP_IMPL_NAME=$(COMP_NAME).uno.$(SHAREDLIB_EXT)
COMP_LIBRARY=$(SHAREDLIB_OUT)/$(COMP_IMPL_NAME)
OUT_COMP_INC=$(OUT_INC)/$(COMP_NAME)
OUT_COMP_MISC=$(OUT_MISC)/$(COMP_NAME)
OUT_COMP_SLO=$(OUT_SLO)/$(COMP_NAME)
COMP_PACKAGE = $(OUT_BIN)/$(COMP_NAME).$(UNOOXT_EXT)
COMP_PACKAGE_URL = $(subst \\,\,"$(COMP_PACKAGE_DIR)$(PS)$(COMP_NAME).$(UNOOXT_EXT)")
COMP_UNOPKG_MANIFEST = $(OUT_COMP_MISC)/$(COMP_NAME)/META-INF/manifest.xml
COMP_COMPONENTS = $(OUT_COMP_MISC)/$(COMP_NAME).components

COMP_REGISTERFLAG = $(OUT_COMP_MISC)/devguide_$(COMP_NAME)_register_component.flag

CXXFILES = SConnection.cxx \
	SDatabaseMetaData.cxx \
	propertyids.cxx \
	SDriver.cxx \
	SPreparedStatement.cxx \
	SResultSet.cxx \
	SResultSetMetaData.cxx \
	SServices.cxx \
	SStatement.cxx

SLOFILES = $(patsubst %.cxx,$(OUT_COMP_SLO)/%.$(OBJ_EXT),$(CXXFILES))

# Targets
.PHONY: ALL
ALL : \
	DatabaseSDBCDriverSkeletonExample

include $(SETTINGS)/stdtarget.mk

$(OUT_COMP_SLO)/%.$(OBJ_EXT) : %.cxx $(SDKTYPEFLAG)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(CC) $(CC_FLAGS) $(CC_INCLUDES) $(STL_INCLUDES) -I$(OUT_COMP_INC) $(CC_DEFINES) $(CC_OUTPUT_SWITCH)$(subst /,$(PS),$@) $<

ifeq "$(OS)" "WIN"
$(SHAREDLIB_OUT)/%.$(SHAREDLIB_EXT) : $(SLOFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	-$(MKDIR) $(subst /,$(PS),$(OUT_COMP_MISC))
	$(LINK) $(COMP_LINK_FLAGS) /OUT:$@ \
	/MAP:$(OUT_COMP_MISC)/$(COMP_NAME).map $(SLOFILES) \
	 $(CPPUHELPERLIB) $(CPPULIB) $(SALLIB) msvcprt.lib $(LIBO_SDK_LDFLAGS_STDLIBS)
	$(LINK_MANIFEST)
else
$(SHAREDLIB_OUT)/%.$(SHAREDLIB_EXT) : $(SLOFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(LINK) $(COMP_LINK_FLAGS) $(LINK_LIBS) -o $@ $(SLOFILES) \
	$(CPPUHELPERLIB) $(CPPULIB) $(SALLIB) $(STC++LIB)
ifeq "$(OS)" "MACOSX"
	$(INSTALL_NAME_URELIBS)  $@
endif
endif

# rule for component package manifest
$(OUT_COMP_MISC)/%/manifest.xml :
	-$(MKDIR) $(subst /,$(PS),$(@D))
	@echo $(OSEP)?xml version="$(QM)1.0$(QM)" encoding="$(QM)UTF-8$(QM)"?$(CSEP) > $@
	@echo $(OSEP)!DOCTYPE manifest:manifest PUBLIC "$(QM)-//OpenOffice.org//DTD Manifest 1.0//EN$(QM)" "$(QM)Manifest.dtd$(QM)"$(CSEP) >> $@
	@echo $(OSEP)manifest:manifest xmlns:manifest="$(QM)http://openoffice.org/2001/manifest$(QM)"$(CSEP) >> $@
	@echo $(SQM)  $(SQM)$(OSEP)manifest:file-entry manifest:media-type="$(QM)application/vnd.sun.star.uno-components;platform=$(UNOPKG_PLATFORM)$(QM)">> $@
	@echo $(SQM)                       $(SQM)manifest:full-path="$(QM)$(COMP_NAME).components$(QM)"/$(CSEP)>> $@
	@echo $(OSEP)/manifest:manifest$(CSEP) >> $@

$(COMP_COMPONENTS) :
	-$(MKDIR) $(subst /,$(PS),$(@D))
	@echo $(OSEP)?xml version="$(QM)1.0$(QM)" encoding="$(QM)UTF-8$(QM)"?$(CSEP) > $@
	@echo $(OSEP)components xmlns="$(QM)http://openoffice.org/2010/uno-components$(QM)"$(CSEP) >> $@
	@echo $(SQM)  $(SQM)$(OSEP)component loader="$(QM)com.sun.star.loader.SharedLibrary$(QM)" uri="$(QM)$(UNOPKG_PLATFORM)/$(COMP_IMPL_NAME)$(QM)"$(CSEP) >> $@
	@echo $(SQM)    $(SQM)$(OSEP)implementation name="$(QM)com.sun.star.comp.sdbc.SkeletonDriver$(QM)"$(CSEP) >> $@
	@echo $(SQM)      $(SQM)$(OSEP)service name="$(QM)com.sun.star.sdbc.Driver$(QM)"/$(CSEP) >> $@
	@echo $(SQM)    $(SQM)$(OSEP)/implementation$(CSEP) >> $@
	@echo $(SQM)  $(SQM)$(OSEP)/component$(CSEP) >> $@
	@echo $(OSEP)/components$(CSEP) >> $@

$(COMP_PACKAGE) : $(COMP_LIBRARY) $(COMP_UNOPKG_MANIFEST) $(COMP_COMPONENTS)
	-$(MKDIR) $(subst /,$(PS),$(@D)) && $(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(OUT_COMP_MISC)/$(UNOPKG_PLATFORM))
	$(COPY) $(subst /,$(PS),$<) $(subst /,$(PS),$(OUT_COMP_MISC)/$(UNOPKG_PLATFORM))
	cd $(subst /,$(PS),$(OUT_COMP_MISC)) && $(SDK_ZIP) ../../bin/$(@F) $(COMP_NAME).components
	cd $(subst /,$(PS),$(OUT_COMP_MISC)) && $(SDK_ZIP) -u ../../bin/$(@F) $(UNOPKG_PLATFORM)/$(<F)
	cd $(subst /,$(PS),$(OUT_COMP_MISC)/$(subst .$(UNOOXT_EXT),,$(@F))) && $(SDK_ZIP) -u ../../../bin/$(@F) META-INF/manifest.xml

$(COMP_REGISTERFLAG) : $(COMP_PACKAGE)
ifeq "$(SDK_AUTO_DEPLOYMENT)" "YES"
	-$(MKDIR) $(subst /,$(PS),$(@D))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	$(DEPLOYTOOL) $(COMP_PACKAGE_URL)
	@echo flagged > $(subst /,$(PS),$@)
else
	@echo --------------------------------------------------------------------------------
	@echo  If you want to install your component automatically, please set the environment
	@echo  variable SDK_AUTO_DEPLOYMENT = YES. But note that auto deployment is only
	@echo  possible if no office instance is running.
	@echo --------------------------------------------------------------------------------
endif

DatabaseSDBCDriverSkeletonExample : $(COMP_REGISTERFLAG)
	@echo --------------------------------------------------------------------------------
	@echo The Database SDBC skeleton driver component was installed if SDK_AUTO_DEPLOYMENT = YES.
	@echo You can use this component inside your office installation, see the example
	@echo description and the howto of implementing a sdbc driver.
	@echo --------------------------------------------------------------------------------

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_INC))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_MISC))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_SLO))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(COMP_LIBRARY)))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(COMP_COMPONENTS)))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(COMP_REGISTERFLAG)))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(COMP_PACKAGE_URL)))
