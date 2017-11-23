
# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

include $(SRCDIR)/unoil/climaker/version.txt

$(eval $(call gb_CliUnoApi_CliUnoApi,cli_oootypes))

$(eval $(call gb_CliUnoApi_set_assembly_version,cli_oootypes,$(CLI_OOOTYPES_NEW_VERSION)))

$(eval $(call gb_CliUnoApi_set_configfile,cli_oootypes,unoil/climaker/cli_oootypes_config,unoil/climaker/version.txt))

$(eval $(call gb_CliUnoApi_set_keyfile,cli_oootypes,$(SRCDIR)/cli_ure/source/cliuno.snk))

$(eval $(call gb_CliUnoApi_set_policy,cli_oootypes,$(CLI_OOOTYPES_POLICY_ASSEMBLY),$(CLI_OOOTYPES_POLICY_VERSION)))

$(eval $(call gb_CliUnoApi_wrap_api,cli_oootypes,offapi))

$(eval $(call gb_CliUnoApi_use_api,cli_oootypes,udkapi))

$(eval $(call gb_CliUnoApi_use_assembly,cli_oootypes,cli_uretypes))

# vim: set noet sw=4 ts=4:
