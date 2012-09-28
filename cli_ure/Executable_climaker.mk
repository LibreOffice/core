# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,climaker))

$(eval $(call gb_Executable_add_cxxflags,climaker,\
	-AI $(gb_Helper_OUTDIRLIBDIR) \
	-clr \
	-LN \
	-wd4339 \
	-wd4715 \
))

$(eval $(call gb_Executable_use_internal_bootstrap_api,climaker,\
	udkapi \
))

$(eval $(call gb_Executable_use_libraries,climaker,\
	cppu \
	cppuhelper \
	sal \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Executable_use_system_win32_libs,climaker,\
	mscoree \
	$(if $(USE_DEBUG_RUNTIME) \
		,msvcmrtd \
		,msvcmrt \
	) \
))

$(eval $(call gb_Executable_add_standard_system_libs,climaker))

$(eval $(call gb_Executable_add_exception_objects,climaker,\
	cli_ure/source/climaker/climaker_app \
	cli_ure/source/climaker/climaker_emit \
))

# vim: set noet sw=4 ts=4:
