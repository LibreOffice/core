# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

include $(SRCDIR)/cli_ure/version/version.txt

$(eval $(call gb_CliLibrary_CliLibrary,cli_basetypes))

$(eval $(call gb_CliLibrary_set_configfile,cli_basetypes,cli_ure/source/basetypes/cli_basetypes_config))

$(eval $(call gb_CliLibrary_set_keyfile,cli_basetypes,$(OUTDIR)/bin/cliuno.snk))

$(eval $(call gb_CliLibrary_set_policy,cli_basetypes,$(CLI_BASETYPES_POLICY_ASSEMBLY),$(CLI_BASETYPES_POLICY_VERSION)))

$(eval $(call gb_CliLibrary_add_csfiles,cli_basetypes,\
    cli_ure/source/basetypes/uno/Any \
    cli_ure/source/basetypes/uno/BoundAttribute \
    cli_ure/source/basetypes/uno/ExceptionAttribute \
    cli_ure/source/basetypes/uno/OnewayAttribute \
    cli_ure/source/basetypes/uno/ParameterizedTypeAttribute \
    cli_ure/source/basetypes/uno/PolymorphicType \
    cli_ure/source/basetypes/uno/TypeArgumentsAttribute \
    cli_ure/source/basetypes/uno/TypeParametersAttribute \
))

$(eval $(call gb_CliLibrary_add_generated_csfiles,cli_basetypes,\
	CustomTarget/cli_ure/source/basetypes/assembly \
))

# vim: set noet sw=4 ts=4:
