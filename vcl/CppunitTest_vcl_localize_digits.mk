# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,vcl_localize_digits))

$(eval $(call gb_CppunitTest_set_include,vcl_localize_digits,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
))

$(eval $(call gb_CppunitTest_add_exception_objects,vcl_localize_digits, \
    vcl/qa/cppunit/LocalizeDigits \
))

$(eval $(call gb_CppunitTest_use_libraries,vcl_localize_digits, \
    sal \
    vcl \
))

# vim: set noet sw=4 ts=4:
