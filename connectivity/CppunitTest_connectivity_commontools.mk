# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,connectivity_commontools))

$(eval $(call gb_CppunitTest_set_include,connectivity_commontools,\
	-I$(SRCDIR)/connectivity/source/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_ure,connectivity_commontools))

$(eval $(call gb_CppunitTest_use_external,connectivity_commontools,boost_headers))

$(eval $(call gb_CppunitTest_use_sdk_api,connectivity_commontools))

ifeq ($(COM),GCC)
ifneq ($(COM_GCC_IS_CLANG),TRUE)
$(eval $(call gb_CppunitTest_add_cxxflags,connectivity_commontools,\
	-fpermissive \
))
endif
endif

ifeq ($(WINDOWS_SDK_VERSION),80)
$(eval $(call gb_CppunitTest_add_defs,connectivity_commontools,\
	-DNTDDI_VERSION=0x0601 \
))
endif

$(eval $(call gb_CppunitTest_add_exception_objects,connectivity_commontools, \
	connectivity/qa/connectivity/commontools/FValue_test \
))

$(eval $(call gb_CppunitTest_use_library_objects,connectivity_commontools,dbtools))

$(eval $(call gb_CppunitTest_use_libraries,connectivity_commontools, \
    comphelper \
    cppu \
    cppuhelper \
    i18nlangtag \
    $(if $(ENABLE_JAVA), \
        jvmaccess) \
    sal \
    salhelper \
    test \
    unotest \
    utl \
    tl \
    $(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_use_components,connectivity_commontools,\
    configmgr/source/configmgr \
    i18npool/util/i18npool \
    ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
))

$(eval $(call gb_CppunitTest_use_configuration,connectivity_commontools))

# vim: set noet sw=4 ts=4:
