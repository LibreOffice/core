# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,cli_ure))

ifeq ($(COM),MSC)
$(eval $(call gb_Module_add_targets,cli_ure,\
	CliLibrary_cli_basetypes \
	CliLibrary_cli_ure \
	CliNativeLibrary_cli_cppuhelper \
	CliUnoApi_cli_uretypes \
	CustomTarget_cli_ure_assemblies \
	Executable_climaker \
	Library_cli_cppuhelper_native \
	Library_cli_uno \
	Package_cli_basetypes_copy \
))
endif

# vim: set noet sw=4 ts=4:
