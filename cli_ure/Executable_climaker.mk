# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,climaker))

$(eval $(call gb_Executable_use_package,climaker,\
	cli_basetypes_copy \
))

# -analyze- to turn that off due to internal compiler error
$(eval $(call gb_Executable_add_cxxclrflags,climaker,\
	-LN \
	-wd4715 \
	-analyze- \
))

$(eval $(call gb_Executable_add_ldflags,climaker,\
		-ignore:4248 \
))

$(eval $(call gb_Executable_use_internal_bootstrap_api,climaker,\
	udkapi \
))

$(eval $(call gb_Executable_use_libraries,climaker,\
	cppu \
	cppuhelper \
	sal \
	salhelper \
	unoidl \
))

$(eval $(call gb_Executable_use_system_win32_libs,climaker,\
	mscoree \
	msvcmrt \
))

$(eval $(call gb_Executable_add_cxxclrobjects,climaker,\
	cli_ure/source/climaker/climaker_app \
	cli_ure/source/climaker/climaker_emit \
))

$(call gb_Executable_get_headers_target,climaker) : \
    $(call gb_CliLibrary_get_target,cli_basetypes)

# vim: set noet sw=4 ts=4:
