#*************************************************************************
#
#  $RCSfile: Makefile,v $
#
#  $Revision: 1.6 $
#
#  last change: $Author: rt $ $Date: 2005-01-31 16:20:44 $
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

# Builds the Databse SDBC driver skeleton example of the Developers Guide.

PRJ=../../../..
SETTINGS=$(PRJ)/settings

include $(SETTINGS)/settings.mk
include $(SETTINGS)/std.mk
include $(SETTINGS)/dk.mk

# Define non-platform/compiler specific settings
COMPONENT_NAME=DatabaseSDBCDriverSkeleton
COMPONENT_IMPL_NAME=$(COMPONENT_NAME).uno.$(SHAREDLIB_EXT) 
COMPONENT_LIBRARY=$(SHAREDLIB_OUT)/$(COMPONENT_IMPL_NAME)
OUT_COMP_INC=$(OUT_INC)/$(COMPONENT_NAME)
OUT_COMP_MISC=$(OUT_MISC)/$(COMPONENT_NAME)
OUT_COMP_SLO=$(OUT_SLO)/$(COMPONENT_NAME)
COMPONENT_XML=skeleton.xml
COMPONENT_RDB_NAME = $(COMPONENT_NAME).uno.rdb
COMPONENT_RDB = $(OUT_COMP_MISC)/$(COMPONENT_RDB_NAME)
COMPONENT_PACKAGE = $(OUT_BIN)/$(COMPONENT_NAME).$(UNOPKG_EXT)
COMPONENT_PACKAGE_URL = $(subst \\,\,"$(COMP_PACKAGE_DIR)$(PS)$(COMPONENT_NAME).$(UNOPKG_EXT)")
COMPONENT_UNOPKG_MANIFEST = $(OUT_COMP_MISC)/$(COMPONENT_NAME)/META-INF/manifest.xml

REGISTERFLAG = $(OUT_COMP_MISC)/devguide_$(COMPONENT_NAME)_register_component.flag
COMPONENT_TYPEFLAG = $(OUT_COMP_MISC)/devguide_$(COMPONENT_NAME)_types.flag

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

TYPES = $(shell $(XML2CMP) -types stdout $(COMPONENT_XML))
TYPESLIST = $(foreach t,$(TYPES),-T$(t))
GENHPPFILES = $(foreach t,$(TYPES),$(OUT_COMP_INC)/$(subst .,/,$(t)).hpp)

# Targets
.PHONY: ALL
ALL : \
	DatabaseSDBCDriverSkeletonExample

include $(SETTINGS)/stdtarget.mk

$(COMPONENT_TYPEFLAG) : $(COMPONENT_XML)
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	-$(MKDIR) $(subst /,$(PS),$(@D))	 
	$(CPPUMAKER) -Gc -BUCR -O$(OUT_COMP_INC) $(TYPESLIST) $(OFFICE_TYPE_LIBRARY)
	echo flagged > $@

$(OUT_COMP_SLO)/%.$(OBJ_EXT) : %.cxx $(COMPONENT_TYPEFLAG)
	-$(MKDIR) $(subst /,$(PS),$(@D))	 
	$(CC) $(CC_FLAGS) $(CC_INCLUDES) $(STL_INCLUDES) -I$(OUT_COMP_INC) $(CC_DEFINES) $(CC_OUTPUT_SWITCH)$(subst /,$(PS),$@) $<

ifeq "$(OS)" "WIN"
$(SHAREDLIB_OUT)/%.$(SHAREDLIB_EXT) : $(SLOFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))
	$(LINK) $(COMP_LINK_FLAGS) /OUT:$@ \
	/MAP:$(OUT_COMP_MISC)/$(COMPONENT_NAME).map $(SLOFILES) \
	 $(CPPUHELPERLIB) $(CPPULIB) $(SALLIB) $(STLPORTLIB) msvcrt.lib kernel32.lib
else
$(SHAREDLIB_OUT)/%.$(SHAREDLIB_EXT) : $(SLOFILES)
	-$(MKDIR) $(subst /,$(PS),$(@D))	 
	$(LINK) $(COMP_LINK_FLAGS) $(LINK_LIBS) -o $@ $^ \
	 $(CPPUHELPERLIB) $(CPPULIB) $(SALLIB) $(STLPORTLIB)
endif

# rule for component package manifest
$(OUT_COMP_MISC)/%/manifest.xml :
	-$(MKDIR) $(subst /,$(PS),$(@D))
	@echo $(OSEP)?xml version="$(QM)1.0$(QM)" encoding="$(QM)UTF-8$(QM)"?$(CSEP) > $@
	@echo $(OSEP)!DOCTYPE manifest:manifest PUBLIC "$(QM)-//OpenOffice.org//DTD Manifest 1.0//EN$(QM)" "$(QM)Manifest.dtd$(QM)"$(CSEP) >> $@
	@echo $(OSEP)manifest:manifest xmlns:manifest="$(QM)http://openoffice.org/2001/manifest$(QM)"$(CSEP) >> $@
	@echo $(SQM)  $(SQM)$(OSEP)manifest:file-entry manifest:media-type="$(QM)application/vnd.sun.star.uno-component;type=native;platform=$(UNOPKG_PLATFORM)$(QM)" >> $@
	@echo $(SQM)                       $(SQM)manifest:full-path="$(QM)$(subst /META-INF,,$(subst $(OUT_COMP_MISC)/,,$(UNOPKG_PLATFORM)/$(@D))).uno.$(SHAREDLIB_EXT)$(QM)"/$(CSEP) >> $@
	@echo $(OSEP)/manifest:manifest$(CSEP) >> $@

$(COMPONENT_PACKAGE) : $(COMPONENT_LIBRARY) $(COMPONENT_UNOPKG_MANIFEST)
	-$(MKDIR) $(subst /,$(PS),$(@D)) && $(DEL) $(subst \\,\,$(subst /,$(PS),$@))	 
	-$(MKDIR) $(subst /,$(PS),$(OUT_COMP_MISC)/$(UNOPKG_PLATFORM))	 
	$(COPY) $(subst /,$(PS),$<) $(subst /,$(PS),$(OUT_COMP_MISC)/$(UNOPKG_PLATFORM))
	cd $(subst /,$(PS),$(OUT_COMP_MISC)) && $(SDK_ZIP) ../../bin/$(@F) $(UNOPKG_PLATFORM)/$(<F)
	cd $(subst /,$(PS),$(OUT_COMP_MISC)/$(subst .$(UNOPKG_EXT),,$(@F))) && $(SDK_ZIP) -u ../../../bin/$(@F) META-INF/manifest.xml

$(REGISTERFLAG) : $(COMPONENT_PACKAGE)
ifeq "$(SDK_AUTO_DEPLOYMENT)" "YES"
	-$(MKDIR) $(subst /,$(PS),$(@D))	 
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$@))
	$(DEPLOYTOOL) $(COMPONENT_PACKAGE_URL)
	@echo flagged > $(subst /,$(PS),$@)
else
	@echo --------------------------------------------------------------------------------
	@echo  If you want to install your component automatically, please set the environment
	@echo  variable SDK_AUTO_DEPLOYMENT = YES. But note that auto deployment is only 
	@echo  possible if no office instance is running. 
	@echo --------------------------------------------------------------------------------
endif

DatabaseSDBCDriverSkeletonExample : $(REGISTERFLAG)
	@echo --------------------------------------------------------------------------------
	@echo The Databse SDBC skeleton driver component was installed if SDK_AUTO_DEPLOYMENT = YES.
	@echo You can use this component inside your office installation, see the example
	@echo description and the howto of implementing a sdbc driver.
	@echo --------------------------------------------------------------------------------

.PHONY: clean
clean :
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_INC))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_MISC))
	-$(DELRECURSIVE) $(subst /,$(PS),$(OUT_COMP_SLO))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(COMPONENT_LIBRARY)))
	-$(DEL) $(subst \\,\,$(subst /,$(PS),$(COMPONENT_PACKAGE_URL)))
