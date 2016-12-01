# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,writerperfect_filtertestbase))

$(eval $(call gb_StaticLibrary_set_include,writerperfect_filtertestbase,\
    -I$(SRCDIR)/writerperfect/qa/unit \
    $$(INCLUDE) \
))

$(eval $(call gb_StaticLibrary_use_externals,writerperfect_filtertestbase,\
	boost_headers \
    cppunit \
))

$(eval $(call gb_StaticLibrary_use_sdk_api,writerperfect_filtertestbase))

$(eval $(call gb_StaticLibrary_use_libraries,writerperfect_filtertestbase,\
    cppu \
    sal \
    test \
    unotest \
))

$(eval $(call gb_StaticLibrary_add_exception_objects,writerperfect_filtertestbase,\
    writerperfect/qa/unit/WpftFilterTestBase \
))

# vim: set noet sw=4 ts=4:
