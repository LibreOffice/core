# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,basebmp))

$(eval $(call gb_Library_use_sdk_api,basebmp))

$(eval $(call gb_Library_use_externals,basebmp,\
	boost_headers \
	vigra_headers \
))

$(eval $(call gb_Library_use_libraries,basebmp,\
    basegfx \
    sal \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_defs,basebmp,\
	-DBASEBMP_DLLIMPLEMENTATION \
))

# TODO(davido): This is failing only in release mode with:
#
# Compilation of bitmapdevice.cxx is failing in release mode:
# 
# Fatal Error C1128: number of sections exceeded object file
# format limit : compile with /bigobj
#
ifeq ($(CPUNAME),X86_64)
$(eval $(call gb_Library_add_cxxflags,basebmp, \
    -bigobj \
))
endif

$(eval $(call gb_Library_add_exception_objects,basebmp,\
	basebmp/source/bitmapdevice \
	basebmp/source/debug \
	basebmp/source/polypolygonrenderer \
))

# vim: set noet sw=4 ts=4:
