# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,testtools/bridgetest_climaker))

testtools_CLIDIR := $(gb_CustomTarget_workdir)/testtools/bridgetest_climaker

$(call gb_CustomTarget_get_target,testtools/bridgetest_climaker) : \
	$(testtools_CLIDIR)/cli_types_bridgetest.dll

$(testtools_CLIDIR)/cli_types_bridgetest.dll : \
		$(call gb_UnoApiTarget_get_target,bridgetest) \
		$(call gb_UnoApiTarget_get_target,udkapi) \
		$(call gb_CliUnoApi_get_target,cli_uretypes) \
		$(call gb_Executable_get_runtime_dependencies,climaker) \
		| $(testtools_CLIDIR)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),CLM,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),CLM)
	$(call gb_Helper_abbreviate_dirs, \
	$(call gb_Helper_execute,climaker) \
		$(if $(filter -s,$(MAKEFLAGS)),,--verbose) \
		--out $@ -r $(call gb_CliUnoApi_get_target,cli_uretypes) \
		-X $(call gb_UnoApiTarget_get_target,udkapi) \
		$(call gb_UnoApiTarget_get_target,bridgetest) > /dev/null)
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),CLM)

# vim:set shiftwidth=4 tabstop=4 noexpandtab:
