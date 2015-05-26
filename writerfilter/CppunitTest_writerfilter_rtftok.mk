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

$(eval $(call gb_CppunitTest_CppunitTest,writerfilter_rtftok))

$(eval $(call gb_CppunitTest_use_external,writerfilter_rtftok,boost_headers))

$(eval $(call gb_CppunitTest_add_exception_objects,writerfilter_rtftok, \
	writerfilter/qa/cppunittests/rtftok/testrtftok \
))

ifeq ($(DISABLE_CVE_TESTS),TRUE)
$(eval $(call gb_CppunitTest_add_defs,writerfilter_rtftok,\
    -DDISABLE_CVE_TESTS \
))
endif

$(eval $(call gb_CppunitTest_use_libraries,writerfilter_rtftok, \
	comphelper \
	cppu \
	cppuhelper \
	sal \
	test \
	unotest \
	vcl \
	writerfilter \
	$(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_use_api,writerfilter_rtftok,\
	offapi \
	udkapi \
))

$(eval $(call gb_CppunitTest_use_ure,writerfilter_rtftok))
$(eval $(call gb_CppunitTest_use_vcl,writerfilter_rtftok))

$(eval $(call gb_CppunitTest_use_components,writerfilter_rtftok,\
	configmgr/source/configmgr \
	framework/util/fwk \
	i18npool/util/i18npool \
	svtools/util/svt \
	ucb/source/core/ucb1 \
	ucb/source/ucp/file/ucpfile1 \
	writerfilter/util/writerfilter \
))

$(eval $(call gb_CppunitTest_use_configuration,writerfilter_rtftok))

# we need to explicitly depend on library writerfilter because it is not implied
# by a link relation
$(call gb_CppunitTest_get_target,writerfilter_rtftok) : $(call gb_Library_get_target,writerfilter)

# vim: set noet sw=4 ts=4:
