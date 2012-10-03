# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

include $(SRCDIR)/cli_ure/version/version.txt

$(eval $(call gb_CliUnoApi_CliUnoApi,cli_uretypes))

$(eval $(call gb_CliUnoApi_set_assembly_version,cli_uretypes,$(CLI_URETYPES_NEW_VERSION)))

$(eval $(call gb_CliUnoApi_set_configfile,cli_uretypes,cli_ure/unotypes/cli_uretypes_config))

$(eval $(call gb_CliUnoApi_set_keyfile,cli_uretypes,$(OUTDIR)/bin/cliuno.snk))

$(eval $(call gb_CliUnoApi_set_policy,cli_uretypes,$(CLI_URETYPES_POLICY_ASSEMBLY),$(CLI_URETYPES_POLICY_VERSION)))

$(eval $(call gb_CliUnoApi_wrap_api,cli_uretypes,udkapi))

# vim: set noet sw=4 ts=4:
