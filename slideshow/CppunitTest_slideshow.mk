# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,slideshow))

$(eval $(call gb_CppunitTest_set_include,slideshow,\
    $$(INCLUDE) \
    -I$(SRCDIR)/slideshow/source/inc \
))

$(eval $(call gb_CppunitTest_use_externals,slideshow,\
	box2d \
))

$(eval $(call gb_CppunitTest_use_sdk_api,slideshow))

$(eval $(call gb_CppunitTest_use_library_objects,slideshow,slideshow))

$(eval $(call gb_CppunitTest_use_libraries,slideshow,\
	$(call gb_Helper_optional,AVMEDIA,avmedia) \
    basegfx \
    canvastools \
    comphelper \
    cppcanvas \
    cppu \
    cppuhelper \
    sal \
    salhelper \
    svl \
    svt \
    svx \
    svxcore \
    tl \
    utl \
    vcl \
))

$(eval $(call gb_CppunitTest_add_exception_objects,slideshow,\
    slideshow/test/slidetest \
    slideshow/test/testshape \
    slideshow/test/testview \
    slideshow/test/views \
))

# vim: set noet sw=4 ts=4:
