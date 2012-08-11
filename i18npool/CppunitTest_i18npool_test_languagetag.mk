# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,i18npool_test_languagetag))

$(eval $(call gb_CppunitTest_use_api,i18npool_test_languagetag,\
	udkapi \
	offapi \
))

$(eval $(call gb_CppunitTest_use_library_objects,i18npool_test_languagetag,i18nisolang1))

$(eval $(call gb_CppunitTest_use_libraries,i18npool_test_languagetag,\
	cppu \
	cppuhelper \
	sal \
	$(gb_STDLIBS) \
))

ifeq ($(ENABLE_LIBLANGTAG),YES)
$(eval $(call gb_CppunitTest_use_externals,i18npool_test_languagetag,\
	liblangtag \
	glib \
	libxml2 \
))
$(eval $(call gb_CppunitTest_add_defs,i18npool_test_languagetag,-DENABLE_LIBLANGTAG))

ifeq ($(SYSTEM_LIBLANGTAG),YES)
$(eval $(call gb_CppunitTest_add_defs,i18npool_test_languagetag,-DSYSTEM_LIBLANGTAG))
endif
endif

$(eval $(call gb_CppunitTest_set_include,i18npool_test_languagetag,\
	-I$(SRCDIR)/i18npool/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_add_exception_objects,i18npool_test_languagetag,\
	i18npool/qa/cppunit/test_languagetag \
))

# vim: set noet sw=4 ts=4:
