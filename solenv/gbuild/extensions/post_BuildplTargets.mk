# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#  
#   Version: MPL 1.1 / GPLv3+ / LGPLv3+
#  
#   The contents of this file are subject to the Mozilla Public License Version
#   1.1 (the "License"); you may not use this file except in compliance with
#   the License or as specified alternatively below. You may obtain a copy of
#   the License at http://www.mozilla.org/MPL/
#  
#   Software distributed under the License is distributed on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
#   for the specific language governing rights and limitations under the
#   License.
#  
#   Major Contributor(s):
#   [ Copyright (C) 2011 Bjoern Michaelsen <bjoern.michaelsen@canonical.com> (initial developer) ]
#  
#   All Rights Reserved.
#  
#   For minor contributions see the git repository.
#  
#   Alternatively, the contents of this file may be used under the terms of
#   either the GNU General Public License Version 3 or later (the "GPLv3+"), or
#   the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
#   in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
#   instead of those above.

.PHONY: cross-build-toolset dev-install build findunusedcode

ifeq ($(gb_SourceEnvAndRecurse_STAGE),buildpl)

.DEFAULT_GOAL=all
#include $(GBUILDDIR)/Module.mk

.PHONY: build all

all: build
	@true

# fake targets -- whatever is requested from gbuild requires a full build before (dev-install for JunitTests)
$(call gb_Package_get_target,%): build
	@true

$(call gb_Executable_get_target,%): build
	@true

$(call gb_Extension_get_target,%): build
	@true

$(call gb_ComponentsTarget_get_target,%): build
	@true

$(call gb_Jar_get_target,%): build
	@true

$(call gb_RdbTarget_get_target,%): build
	@true

$(call gb_Pyuno_get_target,%): build
	@true

$(call gb_WinResTarget_get_target,%): build
	@true

$(call gb_CppunitTest_get_target,%): build
	@true

$(call gb_Configuration_get_target,%): build
	@true

#$(call gb_StaticLibrary_get_target,%): build
#	@true

$(call gb_AllLangResTarget_get_target,%): build
	@true

$(call gb_ExternalLib_get_target,%): build
	@true

#$(call gb_Library_get_target,%): build
#	@true

$(call gb_Package_get_target,%): build
	@true

$(call gb_UnoApiTarget_get_target,%): build
	@true

$(call gb_Zip_get_target,%): build
	@true

$(call gb_JunitTest_get_target,%): dev-install
	@true

gb_MAKETARGET=all
# if we have only build as target use build instead of all
ifneq ($(strip $(MAKECMDGOALS)),)
ifeq ($(filter-out build,$(MAKECMDGOALS)),)
gb_MAKETARGET=build
endif
endif

define gb_BuildplTarget_command
cd $(SRCDIR)/$(1) && unset MAKEFLAGS && export gb_SourceEnvAndRecurse_STAGE=gbuild && $(SOLARENV)/bin/build.pl $(if $(findstring s,$(MAKEFLAGS)),,VERBOSE=T) -P$(BUILD_NCPUS) $(2) -- -P$(GMAKE_PARALLELISM) gb_MAKETARGET=$(gb_MAKETARGET)
endef

# the build order dependencies are rather ugly...
dev-install: \
			$(WORKDIR)/bootstrap \
			$(SRCDIR)/src.downloaded \
			$(if $(filter $(INPATH),$(INPATH_FOR_BUILD)),,cross_toolset) \
		|   $(filter build,$(MAKECMDGOALS)) \
			$(if $(filter check,$(MAKECMDGOALS)),build)
	$(call gb_BuildplTarget_command,smoketestoo_native,)

build: $(WORKDIR)/bootstrap $(SRCDIR)/src.downloaded $(if $(filter $(INPATH),$(INPATH_FOR_BUILD)),,cross_toolset)
	$(call gb_BuildplTarget_command,instsetoo_native,--all)

cross_toolset: $(WORKDIR)/bootstrap $(SRCDIR)/src.downloaded
	source $(SRCDIR)/config_build.mk && source $(SRCDIR)/Env.Build.sh && $(call gb_BuildplTarget_command,$@,--all)

# experimental callcatcher target
# http://www.skynet.ie/~caolan/Packages/callcatcher.html
findunusedcode:
	@which callcatcher > /dev/null 2>&1 || \
	    (echo "callcatcher not installed" && false)
	@sed -e s,$(INPATH),callcatcher,g $(SRCDIR)/config_host.mk > $(SRCDIR)/config_host_callcatcher.mk
	@sed -e s,config_host,config_host_callcatcher,g $(SRCDIR)/Env.Host.sh > $(SRCDIR)/Env.callcatcher.sh
	@mkdir -p $(SRCDIR)/solenv/callcatcher/bin && \
	    ln -sf $(SRCDIR)/solenv/$(INPATH)/bin/dmake \
		$(SRCDIR)/solenv/callcatcher/bin/dmake && \
	    source $(SRCDIR)/config_host_callcatcher.mk && \
	    source $(SRCDIR)/Env.callcatcher.sh && \
	    source $(SRCDIR)/solenv/bin/callcatchEnv.Set.sh && \
	    $(call gb_BuildplTarget_command,instsetoo_native,--all)
	@source $(SRCDIR)/config_host_callcatcher.mk && \
            source $(SRCDIR)/Env.callcatcher.sh && \
	    callanalyse \
		$$WORKDIR/LinkTarget/*/* \
		*/$$OUTPATH/bin/* \
		*/$$OUTPATH/lib/* > unusedcode.all
#because non-c++ symbols could be dlsymed lets make a list of class level
#unused methods which don't require much effort to determine if they need
#to be just removed, or put behind appropiate platform or debug level ifdefs
#
#filter out cppu:: because there's a concern they might be required for the
#extensions abi
#
#filter out boost:: and Icc*:: because there are external libraries we
#build but don't include into our install sets
#
#filter out Atom*:: from libcmis, because its too painful to customize
#it to only build the methods we directly call
	@grep ::.*\( unusedcode.all \
		| grep -v ^cppu:: \
		| grep -v ^boost:: \
		| grep -v ^CIcc \
		| grep -v ^CAtom \
		> unusedcode.easy

subsequentcheck: dev-install

clean:
	@true

check: subsequentcheck
	@true

unitcheck: build
	@true

debugrun:
	@true

endif # gb_SourceEnvAndRecurse_STAGE=buildpl

ifeq ($(gb_SourceEnvAndRecurse_STAGE),gbuild)

clean: clean-host clean-build

dev-install: $(WORKDIR)/bootstrap  $(SRCDIR)/src.downloaded $(if $(filter $(INPATH),$(INPATH_FOR_BUILD)),,cross_toolset) | $(filter build,$(MAKECMDGOALS))

build: $(WORKDIR)/bootstrap $(SRCDIR)/src.downloaded $(if $(filter $(INPATH),$(INPATH_FOR_BUILD)),,cross_toolset)

cross_toolset: $(WORKDIR)/bootstrap $(SRCDIR)/src.downloaded

findunusedcode:

endif


# vim: set noet sw=4:
