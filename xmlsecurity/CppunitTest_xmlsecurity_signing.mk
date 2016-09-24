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

$(eval $(call gb_CppunitTest_CppunitTest,xmlsecurity_signing))

$(eval $(call gb_CppunitTest_add_exception_objects,xmlsecurity_signing, \
	xmlsecurity/qa/unit/signing/signing \
))

$(eval $(call gb_CppunitTest_use_libraries,xmlsecurity_signing, \
	comphelper \
	cppuhelper \
	cppu \
	sal \
	sax \
	sfx \
	test \
	tl \
	unotest \
	utl \
	xmlsecurity \
	$(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_use_externals,xmlsecurity_signing,\
    boost_headers \
))

$(eval $(call gb_CppunitTest_set_include,xmlsecurity_signing,\
	-I$(SRCDIR)/xmlsecurity/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_sdk_api,xmlsecurity_signing))

$(eval $(call gb_CppunitTest_use_ure,xmlsecurity_signing))
$(eval $(call gb_CppunitTest_use_vcl,xmlsecurity_signing))

$(eval $(call gb_CppunitTest_use_rdb,xmlsecurity_signing,services))

$(eval $(call gb_CppunitTest_use_configuration,xmlsecurity_signing))

# vim: set noet sw=4 ts=4:
