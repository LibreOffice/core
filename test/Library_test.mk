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
	-I$(SRCDIR)/test/inc \
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
	test/source/helper/form \
))

# vim: set noet sw=4 ts=4:
