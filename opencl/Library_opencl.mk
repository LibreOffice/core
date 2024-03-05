# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,opencl))

$(eval $(call gb_Library_set_include,opencl,\
    -I$(SRCDIR)/opencl/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_add_defs,opencl,\
    -DOPENCL_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_use_externals,opencl,\
    clew \
    icu_headers \
    icui18n \
    icuuc \
))

$(eval $(call gb_Library_use_custom_headers,opencl,\
    officecfg/registry \
))

$(eval $(call gb_Library_use_sdk_api,opencl))

$(eval $(call gb_Library_use_libraries,opencl,\
    comphelper \
    cppu \
    sal \
    salhelper \
    tl \
))

$(eval $(call gb_Library_add_exception_objects,opencl,\
    opencl/source/openclconfig \
    opencl/source/openclwrapper \
    opencl/source/opencl_device \
    opencl/source/platforminfo \
    opencl/source/OpenCLZone \
))

ifeq ($(OS),LINUX)
$(eval $(call gb_Library_add_libs,opencl,\
    -lrt \
))
endif

# vim: set noet sw=4 ts=4:
