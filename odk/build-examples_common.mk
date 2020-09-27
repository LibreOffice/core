# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# System-provided shells on macOS are known to unset DYLD_LIBRARY_PATH, so we need some sort of hack
# to provided the invocation of make below with a shell that preserves DYLD_LIBRARY_PATH.  Until
# macOS 10.14, what apparently worked is to use a copy of /bin/sh in some temp dir.  However, with
# macOS 10.15, that hack appears to no longer work, the only known workaround is to use e.g. bash
# built from upstream source and pass it into the toplevel make invocation as SHELL=... command line
# argument (which is also needed when building external/firebird and needing to preserve a global
# DYLD_LIBRARY_PATH, see comment 17 at
# <https://bugs.documentfoundation.org/show_bug.cgi?id=101789#c17> "FIREBIRD 3 - fails to build on
# OSX 10.11.6 with clang").  When building on macOS <= 10.14 and not passing SHELL=... on the
# command line, MACOSX_SHELL_HACK will kick in and do the "use a copy of /bin/sh" workaround.  When
# passing in a self-built SHELL=... (which is necessary now on macOS >= 10.15), the
# MACOSX_SHELL_HACK will not kick in, but the below invocation of make will automatically inherit
# SHELL from the currently running invocation of make.  (At least with GNU Make 4.2.1, the origin of
# a default SHELL setting is, somewhat unintuitively, reported as "file" rather than "default".  To
# avoid surprises, use an explicit check for an origin of "command line" here.)
MACOSX_SHELL_HACK := $(and $(filter MACOSX,$(OS)),$(filter-out command line,$(origin SHELL)))

define odk_build-examples_test

$(eval $(call gb_CustomTarget_CustomTarget,$(1)))

$(call gb_CustomTarget_get_target,$(1)): \
	$(call gb_CustomTarget_get_workdir,$(1))/setsdkenv
ifneq ($(gb_SUPPRESS_TESTS),)
	@true
else
	$$(call gb_Output_announce,$(subst $(WORKDIR)/,,$(1)),$(true),CHK,1)
	rm -fr $(call gb_CustomTarget_get_workdir,$(1))/{out,user}
	$(if $(MACOSX_SHELL_HACK), \
	    ODK_BUILD_SHELL=$$$$($(gb_MKTEMP)) && \
	    cp /bin/sh "$$$$ODK_BUILD_SHELL" && \
	    chmod 0700 "$$$$ODK_BUILD_SHELL" &&) \
	(saved_library_path=$$$${$(gb_Helper_LIBRARY_PATH_VAR)} && . $$< \
	$(if $(filter MACOSX,$(OS)),, \
	    && $(gb_Helper_LIBRARY_PATH_VAR)=$$$$saved_library_path) \
	&& export \
	    UserInstallation=$(call gb_Helper_make_url,$(call gb_CustomTarget_get_workdir,$(1))/user) \
	$(foreach my_dir,$(2), \
	    && (cd $(INSTDIR)/$(SDKDIRNAME)/examples/$(my_dir) \
		&& printf 'yes\n' | LC_ALL=C make -j1 \
			CC="$(CXX)" LINK="$(CXX)" LIB="$(CXX)" \
		    $(if $(MACOSX_SHELL_HACK), SHELL="$$$$ODK_BUILD_SHELL", ))) \
	$(if $(MACOSX_SHELL_HACK),&& rm -f "$$$$ODK_BUILD_SHELL")) \
	    >$(call gb_CustomTarget_get_workdir,$(1))/log 2>&1 \
	|| (RET=$$$$? \
	    $(if $(MACOSX_SHELL_HACK), && rm -f "$$$$ODK_BUILD_SHELL" , ) \
	    && cat $(call gb_CustomTarget_get_workdir,$(1))/log \
	    && exit $$$$RET)
endif

$(call gb_CustomTarget_get_workdir,$(1))/setsdkenv: \
	$(SRCDIR)/odk/config/setsdkenv_unix.sh.in \
	$(BUILDDIR)/config_$(gb_Side).mk | \
	$(call gb_CustomTarget_get_workdir,$(1))/.dir
	$$(call gb_Output_announce,$(subst $(WORKDIR)/,,$(1)),$(true),SED,1)
	sed -e 's!@OO_SDK_NAME@!sdk!' \
	-e 's!@OO_SDK_HOME@!$(INSTDIR)/$(SDKDIRNAME)!' \
	-e 's!@OFFICE_HOME@!$(INSTROOTBASE)!' -e 's!@OO_SDK_MAKE_HOME@!!' \
	-e 's!@OO_SDK_ZIP_HOME@!!' -e 's!@OO_SDK_CAT_HOME@!!' \
	-e 's!@OO_SDK_SED_HOME@!!' -e 's!@OO_SDK_CPP_HOME@!!' \
	-e 's!@OO_SDK_JAVA_HOME@!$(JAVA_HOME)!' \
	-e 's!@OO_SDK_OUTPUT_DIR@!$(call gb_CustomTarget_get_workdir,$(1))/out!' \
	-e 's!@SDK_AUTO_DEPLOYMENT@!YES!' $$< > $$@

.PHONY: $(call gb_CustomTarget_get_target,$(1))

endef

# vim: set noet sw=4 ts=4:
