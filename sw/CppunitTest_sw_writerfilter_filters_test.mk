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

$(eval $(call gb_CppunitTest_CppunitTest,sw_writerfilter_filters_test))

$(eval $(call gb_CppunitTest_use_external,sw_writerfilter_filters_test,boost_headers))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_writerfilter_filters_test, \
	sw/qa/writerfilter/filters-test/filters-test \
))

ifeq ($(DISABLE_CVE_TESTS),TRUE)
$(eval $(call gb_CppunitTest_add_defs,sw_writerfilter_filters_test,\
    -DDISABLE_CVE_TESTS \
))
endif

$(eval $(call gb_CppunitTest_use_libraries,sw_writerfilter_filters_test, \
	comphelper \
	cppu \
	cppuhelper \
	sal \
	test \
	unotest \
	vcl \
	sw_writerfilter \
))

$(eval $(call gb_CppunitTest_use_sdk_api,sw_writerfilter_filters_test))

$(eval $(call gb_CppunitTest_use_ure,sw_writerfilter_filters_test))
$(eval $(call gb_CppunitTest_use_vcl,sw_writerfilter_filters_test))

$(eval $(call gb_CppunitTest_use_components,sw_writerfilter_filters_test,\
	configmgr/source/configmgr \
	framework/util/fwk \
	i18npool/util/i18npool \
	svtools/util/svt \
	ucb/source/core/ucb1 \
	ucb/source/ucp/file/ucpfile1 \
	sw/util/sw_writerfilter \
	vcl/vcl.common \
))



$(eval $(call gb_CppunitTest_use_configuration,sw_writerfilter_filters_test))

# we need to explicitly depend on library sw_writerfilter because it is not implied
# by a link relation
$(call gb_CppunitTest_get_target,sw_writerfilter_filters_test) : $(call gb_Library_get_target,sw_writerfilter)

# vim: set noet sw=4 ts=4:
