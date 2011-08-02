# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

include $(SRCDIR)/cli_ure/version/version.txt

$(eval $(call gb_CliLibrary_CliLibrary,cli_uno_bridge))

# FIXME: The original code did not use config file at all; How to do this in gbuild
$(eval $(call gb_CliLibrary_set_configfile,cli_uno_bridge,cli_ure/source/basetypes/cli_basetypes_config))

$(eval $(call gb_CliLibrary_set_keyfile,cli_uno_bridge,$(SRCDIR)/cli_ure/source/cliuno.snk))

# FIXME: the original code did not use the policy assembly; there is someting similar in cli_ure/source/mono_bridge/assemblyinfo
#        How to do this in gbuild?
$(eval $(call gb_CliLibrary_set_policy,cli_uno_bridge,$(CLI_BASETYPES_POLICY_ASSEMBLY),$(CLI_BASETYPES_POLICY_VERSION)))

$(eval $(call gb_CliLibrary_add_csfiles,cli_uno_bridge,\
    cli_ure/source/mono_bridge/assemblyinfo \
    cli_ure/source/mono_bridge/binaryuno \
    cli_ure/source/mono_bridge/bridge \
    cli_ure/source/mono_bridge/cli_environment \
    cli_ure/source/mono_bridge/managed_proxy \
    cli_ure/source/mono_bridge/rtl_ustring \
    cli_ure/source/mono_bridge/typeclass \
    cli_ure/source/mono_bridge/typedescription \
    cli_ure/source/mono_bridge/uik \
    cli_ure/source/mono_bridge/uno_proxy \
))

$(eval $(call gb_CliLibrary_add_csflags,cli_uno_bridge,\
	-unsafe \
))

$(eval $(call gb_CliLibrary_use_assemblies,cli_uno_bridge,\
	cli_basetypes \
	cli_uretypes \
))


# vim: set noet sw=4 ts=4:
