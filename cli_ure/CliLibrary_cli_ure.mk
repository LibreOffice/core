# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

include $(SRCDIR)/cli_ure/version/version.txt

$(eval $(call gb_CliLibrary_CliLibrary,cli_ure))

$(eval $(call gb_CliLibrary_set_configfile,cli_ure,cli_ure/source/ure/cli_ure_config))

$(eval $(call gb_CliLibrary_set_keyfile,cli_ure,$(SRCDIR)/cli_ure/source/cliuno.snk))

$(eval $(call gb_CliLibrary_set_policy,cli_ure,$(CLI_URE_POLICY_ASSEMBLY),$(CLI_URE_POLICY_VERSION)))

$(eval $(call gb_CliLibrary_use_assemblies,cli_ure,\
	cli_uretypes \
))

$(eval $(call gb_CliLibrary_add_csfiles,cli_ure,\
    cli_ure/source/ure/uno/util/DisposeGuard \
    cli_ure/source/ure/uno/util/WeakAdapter \
    cli_ure/source/ure/uno/util/WeakBase \
    cli_ure/source/ure/uno/util/WeakComponentBase \
))

$(eval $(call gb_CliLibrary_add_generated_csfiles,cli_ure,\
	CustomTarget/cli_ure/source/ure/assembly \
))

# vim: set noet sw=4 ts=4:
