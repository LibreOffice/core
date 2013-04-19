# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,jvmfwk))

$(eval $(call gb_Library_use_package,jvmfwk,\
))

$(eval $(call gb_Library_add_defs,jvmfwk,\
    -DJVMFWK_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_use_api,jvmfwk,\
    udkapi \
))

$(eval $(call gb_Library_use_libraries,jvmfwk,\
    cppuhelper \
    sal \
	$(gb_UWINAPI) \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Library_use_system_win32_libs,jvmfwk,\
	advapi32 \
))
endif

$(eval $(call gb_Library_use_externals,jvmfwk,\
    boost_headers \
    libxml2 \
))

$(eval $(call gb_Library_add_exception_objects,jvmfwk,\
    jvmfwk/source/elements \
    jvmfwk/source/framework \
    jvmfwk/source/fwkbase \
    jvmfwk/source/fwkutil \
    jvmfwk/source/libxmlutil \
))

# vim:set noet sw=4 ts=4:
