# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,test))

$(eval $(call gb_Library_set_include,test,\
    $$(INCLUDE) \
))

$(eval $(call gb_Library_add_defs,test,\
    -DOOO_DLLIMPLEMENTATION_TEST \
))

$(eval $(call gb_Library_use_sdk_api,test))

$(eval $(call gb_Library_use_externals,test,\
	boost_headers \
	cppunit \
	libxml2 \
	valgrind \
))

$(eval $(call gb_Library_use_libraries,test,\
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
	i18nlangtag \
    sal \
    sfx \
    svt \
    test-setupvcl \
	tl \
	utl \
	unotest \
	vcl \
	drawinglayer \
))

$(eval $(call gb_Library_add_exception_objects,test,\
    test/source/bootstrapfixture \
    test/source/diff/diff \
    test/source/callgrind \
    test/source/xmltesttools \
    test/source/htmltesttools \
    test/source/screenshot_test \
    test/source/unoapi_property_testers \
    test/source/lokcallback \
	test/source/helper/form \
	test/source/helper/shape \
	test/source/helper/transferable \
))

# vim: set noet sw=4 ts=4:
