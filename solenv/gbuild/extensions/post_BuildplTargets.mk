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

ifeq ($(strip $(gb_PARTIALBUILD)),)

clean: clean-host clean-build

subsequentcheck: smoketestoo_native
	@$(MAKE) -f $(realpath $(firstword $(MAKEFILE_LIST))) $@ gb_PARTIALBUILD=T

# instsetoo_native via build.pl already runs unittests
unitcheck: instsetoo_native
	@true

all:

gb_MAKETARGET=all
# if we have only build as target use build instead of all
ifneq ($(strip $(MAKECMDGOALS)),)
ifeq ($(filter-out build,$(MAKECMDGOALS)),)
gb_MAKETARGET=build
endif
endif

gb_BuildplTarget_COMPLETEDTARGETS=
define gb_BuildplTarget_command
cd $(SRCDIR)/$(1) && unset MAKEFLAGS && $(SOLARENV)/bin/build.pl $(if $(findstring s,$(MAKEFLAGS)),,VERBOSE=T) -P$(BUILD_NCPUS) $(2) -P$(GMAKE_PARALLELISM) gb_MAKETARGET=$(gb_MAKETARGET)
$(eval gb_BuildplTarget_COMPLETEDTARGETS+=$(1))
endef

.PHONY: smoketestoo_native instsetoo_native cross-build-toolset dev-install all build findunusedcode

smoketestoo_native: $(WORKDIR)/bootstrap  $(SRCDIR)/src.downloaded $(if $(filter $(INPATH),$(INPATH_FOR_BUILD)),,cross_toolset) | instsetoo_native
	$(call gb_BuildplTarget_command,$@,$(if $(filter instsetoo_native,$(gb_BuildplTarget_COMPLETEDTARGETS)),--from instsetoo_native,--all))

instsetoo_native: $(WORKDIR)/bootstrap $(SRCDIR)/src.downloaded $(if $(filter $(INPATH),$(INPATH_FOR_BUILD)),,cross_toolset)
	$(call gb_BuildplTarget_command,$@,--all)

cross_toolset: $(WORKDIR)/bootstrap $(SRCDIR)/src.downloaded
	source $(SRCDIR)/Env.Build.sh && $(call gb_BuildplTarget_command,$@,--all)

# experimental callcatcher target
# http://www.skynet.ie/~caolan/Packages/callcatcher.html
findunusedcode:
	@which callcatcher > /dev/null 2>&1 || \
	    (echo "callcatcher not installed" && false)
	@sed -e s,$(INPATH),callcatcher,g $(SRCDIR)/Env.Host.sh > $(SRCDIR)/Env.callcatcher.sh
	@mkdir -p $(SRCDIR)/solenv/callcatcher/bin && \
	    ln -sf $(SRCDIR)/solenv/$(INPATH)/bin/dmake \
		$(SRCDIR)/solenv/callcatcher/bin/dmake && \
	    source $(SRCDIR)/Env.callcatcher.sh && \
	    source $(SRCDIR)/solenv/bin/callcatchEnv.Set.sh && \
	    $(call gb_BuildplTarget_command,instsetoo_native,--all)
	@source $(SRCDIR)/Env.callcatcher.sh && \
	    callanalyse \
		$$WORKDIR/LinkTarget/*/* \
		*/$$OUTPATH/bin/* \
		*/$$OUTPATH/lib/* > unusedcode.all
#because non-c++ symbols could be dlsymed lets make a list of class level
#unused methods which don't require much effort to determine if they need
#to be just removed, or put behind appropiate platform or debug level ifdefs
	@grep ::.*\( unusedcode.all | grep -v ^cppu:: > unusedcode.easy

dev-install: smoketestoo_native

all: instsetoo_native

build: instsetoo_native

endif # gb_PARTIALBUILD
    
# vim: set noet sw=4:
