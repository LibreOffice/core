# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,writerperfect_importtestbase))

$(eval $(call gb_StaticLibrary_set_include,writerperfect_importtestbase,\
    -I$(SRCDIR)/writerperfect/qa/unit \
    $$(INCLUDE) \
))

$(eval $(call gb_StatiLibrary_use_externals,writerperfect_importtestbase,\
	boost_headers \
    cppunit \
))

$(eval $(call gb_StaticLibrary_use_sdk_api,writerperfect_importtestbase))

$(eval $(call gb_StaticLibrary_use_libraries,writerperfect_importtestbase,\
    cppu \
    sal \
    test \
    unotest \
))

$(eval $(call gb_StaticLibrary_add_exception_objects,writerperfect_importtestbase,\
    writerperfect/qa/unit/WpftImportTestBase \
))

# vim: set noet sw=4 ts=4:
