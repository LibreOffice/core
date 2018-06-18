# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#*************************************************************************

$(eval $(call gb_CppunitTest_CppunitTest,desktop_lok_init))

$(eval $(call gb_CppunitTest_add_exception_objects,desktop_lok_init, \
    desktop/qa/unit/desktop-lok-init \
))

$(eval $(call gb_CppunitTest_use_external,desktop_lok_init,boost_headers))

$(eval $(call gb_CppunitTest_use_sdk_api,desktop_lok_init))

$(eval $(call gb_CppunitTest_use_ure,desktop_lok_init))

$(eval $(call gb_CppunitTest_set_include,desktop_lok_init,\
    -I$(SRCDIR)/desktop/source/inc \
    -I$(SRCDIR)/desktop/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_libraries,desktop_lok_init, \
	comphelper \
	cppu \
	sal \
	sofficeapp \
	vcl \
	$(gb_UWINAPI) \
))

ifeq ($(OS),LINUX)
$(eval $(call gb_CppunitTest_add_libs,desktop_lok_init,\
    -lm \
    -ldl \
    -lpthread \
))
endif

$(eval $(call gb_CppunitTest_use_configuration,desktop_lok_init))

# vim: set noet sw=4 ts=4:
