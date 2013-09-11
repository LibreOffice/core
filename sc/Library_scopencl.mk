# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,scopencl))

$(eval $(call gb_Library_set_include,scopencl,\
	-I$(SRCDIR)/sc/source/core/inc \
	-I$(SRCDIR)/sc/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_set_precompiled_header,scopencl,$(SRCDIR)/sc/inc/pch/precompiled_scopencl))

$(eval $(call gb_Library_use_sdk_api,scopencl))

$(eval $(call gb_Library_use_libraries,scopencl,\
	for \
	sal \
	sc \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_exception_objects,scopencl,\
	sc/source/core/opencl/formulagroupcl \
	sc/source/core/opencl/openclwrapper \
	sc/source/core/opencl/clcc/clew \
))

ifeq ($(OS),LINUX)
$(eval $(call gb_Library_add_libs,scopencl,\
	-ldl \
))
endif

# vim: set noet sw=4 ts=4:
