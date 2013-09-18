# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,testtools/bridgetest_javamaker))

testtools_JAVADIR := $(call gb_CustomTarget_get_workdir,testtools/bridgetest_javamaker)

$(call gb_CustomTarget_get_target,testtools/bridgetest_javamaker) : $(testtools_JAVADIR)/done

$(testtools_JAVADIR)/done : \
		$(call gb_UnoApiTarget_get_target,bridgetest) \
		$(call gb_UnoApiTarget_get_target,performance) \
		$(call gb_UnoApiTarget_get_target,udkapi) \
		$(call gb_Executable_get_runtime_dependencies,javamaker) \
		| $(testtools_JAVADIR)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),JVM,1)
	$(call gb_Helper_abbreviate_dirs, \
	rm -r $(testtools_JAVADIR) && \
	$(call gb_Helper_execute,javamaker -nD -O$(testtools_JAVADIR) -X$(call gb_UnoApiTarget_get_target,udkapi) $(call gb_UnoApiTarget_get_target,bridgetest) $(call gb_UnoApiTarget_get_target,performance)) && touch $@)

# vim:set shiftwidth=4 tabstop=4 noexpandtab:
