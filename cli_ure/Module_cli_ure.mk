# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,cli_ure))

$(eval $(call gb_Module_add_targets,cli_ure,\
    Package_version \
))

# common stuff 
ifeq ($(BUILD_CLI),YES)
$(eval $(call gb_Module_add_targets,cli_ure,\
	CliLibrary_cli_basetypes \
	CliLibrary_cli_ure \
	CustomTarget_cli_ure_assemblies \
	Package_config \
))

ifeq ($(COM),MSC)

# Original implementation for Windows
$(eval $(call gb_Module_add_targets,cli_ure,\
	CliNativeLibrary_cli_cppuhelper \
	CliUnoApi_cli_uretypes \
	Executable_climaker \
	Library_cli_cppuhelper_native \
	Library_cli_uno \
))

else

# Alternative implementation
# FIXME: tested only on Linux
$(eval $(call gb_Module_add_targets,cli_ure,\
        CliLibrary_cli_cppuhelper_mono \
        CliLibrary_cli_uno_bridge \
        Library_cli_uno_unix \
        Library_cli_uno_glue \
        Library_mono_loader \
))

ifeq ($(ENABLE_MONO_CLIMAKER),YES)
$(eval $(call gb_Module_add_targets,cli_ure,\
        CliUnoApi_cli_uretypes \
        Executable_climaker_mono \
        Library_climaker \
))
else
$(eval $(call gb_Module_add_targets,cli_ure,\
        CustomTarget_cli_uretypes \
        Package_cli_uretypes \
))
endif

endif

endif

# vim: set noet sw=4 ts=4:
