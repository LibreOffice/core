# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,i18nlangtag_test_languagetag))

$(eval $(call gb_CppunitTest_use_external,i18nlangtag_test_languagetag,boost_headers))

$(eval $(call gb_CppunitTest_use_sdk_api,i18nlangtag_test_languagetag))

$(eval $(call gb_CppunitTest_use_libraries,i18nlangtag_test_languagetag,\
	i18nlangtag \
	cppu \
	cppuhelper \
	sal \
))

$(eval $(call gb_CppunitTest_use_externals,i18nlangtag_test_languagetag,\
	liblangtag \
	libxml2 \
))

ifneq ($(SYSTEM_LIBLANGTAG),)
$(eval $(call gb_CppunitTest_add_defs,i18nlangtag_test_languagetag,-DSYSTEM_LIBLANGTAG))
else
$(eval $(call gb_CppunitTest_use_package,i18nlangtag_test_languagetag,liblangtag_data))
endif

$(eval $(call gb_CppunitTest_add_exception_objects,i18nlangtag_test_languagetag,\
	i18nlangtag/qa/cppunit/test_languagetag \
))

# vim: set noet sw=4 ts=4:
