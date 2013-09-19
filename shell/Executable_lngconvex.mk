# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,lngconvex))

$(eval $(call gb_Executable_add_defs,lngconvex,\
	-D_NTSDK \
))

$(eval $(call gb_Executable_use_external,lngconvex,boost_headers))

$(eval $(call gb_Executable_use_sdk_api,lngconvex))

$(eval $(call gb_Executable_use_libraries,lngconvex,\
	comphelper \
	i18nlangtag \
	sal \
	tl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Executable_add_exception_objects,lngconvex,\
    shell/source/tools/lngconvex/cmdline \
    shell/source/tools/lngconvex/lngconvex \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
