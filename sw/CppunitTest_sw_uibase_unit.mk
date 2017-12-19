# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,sw_uibase_unit))

$(eval $(call gb_CppunitTest_use_external,sw_uibase_unit,boost_headers))

$(eval $(call gb_CppunitTest_use_sdk_api,sw_uibase_unit))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_uibase_unit, \
    sw/qa/unit/uibase \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_uibase_unit, \
	sal \
	sw \
	tl \
))

$(eval $(call gb_CppunitTest_set_include,sw_uibase_unit,\
    -I$(SRCDIR)/sw/inc \
    $$(INCLUDE) \
))

# vim: set noet sw=4 ts=4:
