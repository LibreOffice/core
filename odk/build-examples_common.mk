# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

define odk_build-examples_test

$(eval $(call gb_CustomTarget_CustomTarget,$(1)))

$(call gb_CustomTarget_get_target,$(1)): \
	$(call gb_CustomTarget_get_workdir,$(1))/setsdkenv
ifneq ($(gb_SUPPRESS_TESTS),)
	@true
else
	$$(call gb_Output_announce,$(subst $(WORKDIR)/,,$(1)),$(true),CHK,1)
	rm -fr $(call gb_CustomTarget_get_workdir,$(1))/{out,user}
ifeq (MACOSX,$(OS))
	$(eval ODK_BUILD_SHELL := $(shell $(gb_MKTEMP)))
	cp /bin/sh "$(ODK_BUILD_SHELL)"
	chmod 0700 "$(ODK_BUILD_SHELL)"
endif
	(saved_library_path=$$$${$(gb_Helper_LIBRARY_PATH_VAR)} && . $$< \
	$(if $(filter MACOSX,$(OS)),, \
	    && $(gb_Helper_LIBRARY_PATH_VAR)=$$$$saved_library_path) \
	&& export \
	    UserInstallation=$(call gb_Helper_make_url,$(call gb_CustomTarget_get_workdir,$(1))/user) \
	$(foreach my_dir,$(2), \
	    && (cd $(INSTDIR)/$(SDKDIRNAME)/examples/$(my_dir) \
		&& printf 'yes\n' | LC_ALL=C make \
			CC="$(CXX)" LINK="$(CXX)" LIB="$(CXX)" \
		    $(if $(filter MACOSX,$(OS)), SHELL=$(ODK_BUILD_SHELL), )))) \
	    >$(call gb_CustomTarget_get_workdir,$(1))/log 2>&1 \
	|| (RET=$$$$? \
	    $(if $(filter MACOSX,$(OS)), && rm -f $(ODK_BUILD_SHELL) , ) \
	    && cat $(call gb_CustomTarget_get_workdir,$(1))/log \
	    && exit $$$$RET)
ifeq (MACOSX,$(OS))
	-rm -f $(ODK_BUILD_SHELL)
endif
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
