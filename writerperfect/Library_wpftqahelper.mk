# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,wpftqahelper))

$(eval $(call gb_Library_set_include,wpftqahelper,\
    -I$(SRCDIR)/writerperfect/qa/unit \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_add_defs,wpftqahelper,\
	-DWPFTQAHELPER_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_use_externals,wpftqahelper,\
    cppunit \
))

$(eval $(call gb_Library_use_sdk_api,wpftqahelper))

$(eval $(call gb_Library_use_libraries,wpftqahelper,\
    cppu \
    sal \
    test \
    tl \
    ucbhelper \
    unotest \
))

$(eval $(call gb_Library_add_exception_objects,wpftqahelper,\
    writerperfect/qa/unit/WpftFilterFixture \
    writerperfect/qa/unit/WpftFilterTestBase \
    writerperfect/qa/unit/WpftLoader \
))

# vim: set noet sw=4 ts=4:
