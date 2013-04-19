# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,slideshow))

$(eval $(call gb_CppunitTest_use_package,slideshow,sd_qa_unit))

$(eval $(call gb_CppunitTest_set_include,slideshow,\
    $$(INCLUDE) \
    -I$(SRCDIR)/slideshow/source/inc \
))

$(eval $(call gb_CppunitTest_set_defs,slideshow,\
    $$(DEFS) \
    $(if $(filter TRUE,$(VERBOSE)),-DVERBOSE) \
))

ifneq ($(strip $(debug)$(DEBUG)),)
$(eval $(call gb_CppunitTest_set_defs,slideshow,\
    $$(DEFS) \
    -DBOOST_SP_ENABLE_DEBUG_HOOKS \
))
endif

$(eval $(call gb_CppunitTest_use_api,slideshow,\
    offapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_library_objects,slideshow,slideshow))

$(eval $(call gb_CppunitTest_use_libraries,slideshow,\
    avmedia \
    basegfx \
    canvastools \
    comphelper \
    cppcanvas \
    cppu \
    cppuhelper \
    sal \
    svt \
    tl \
    utl \
    vcl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_add_exception_objects,slideshow,\
    slideshow/test/slidetest \
    slideshow/test/testshape \
    slideshow/test/testview \
    slideshow/test/views \
))

# vim: set noet sw=4 ts=4:
