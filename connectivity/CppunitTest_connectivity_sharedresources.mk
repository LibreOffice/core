# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,connectivity_sharedresources))

$(eval $(call gb_CppunitTest_set_include,connectivity_sharedresources,\
	-I$(SRCDIR)/connectivity/inc \
	-I$(SRCDIR)/connectivity/source/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_ure,connectivity_sharedresources))
$(eval $(call gb_CppunitTest_use_vcl,connectivity_sharedresources))

$(eval $(call gb_CppunitTest_use_external,connectivity_sharedresources,boost_headers))

$(eval $(call gb_CppunitTest_use_sdk_api,connectivity_sharedresources))

ifeq ($(COM),GCC)
ifneq ($(COM_IS_CLANG),TRUE)
$(eval $(call gb_CppunitTest_add_cxxflags,connectivity_sharedresources,\
	-fpermissive \
))
endif
endif

# In mergedlibs mode we have multiply-defined symbols, which Visual Studio
# does not like. There is no good solution, so just force it.
ifeq ($(COM),MSC)
ifneq (,$(MERGELIBS))
$(eval $(call gb_CppunitTest_add_ldflags,connectivity_sharedresources,\
	 /FORCE:MULTIPLE \
))
endif
endif

$(eval $(call gb_CppunitTest_add_exception_objects,connectivity_sharedresources, \
	connectivity/qa/connectivity/resource/sharedresources_test \
))

$(eval $(call gb_CppunitTest_use_library_objects,connectivity_sharedresources,dbtools))

$(eval $(call gb_CppunitTest_use_libraries,connectivity_sharedresources, \
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
    svt \
    vcl \
))

$(eval $(call gb_CppunitTest_use_components,connectivity_sharedresources,\
    configmgr/source/configmgr \
    i18npool/util/i18npool \
    ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
))

$(eval $(call gb_CppunitTest_use_configuration,connectivity_sharedresources))

# vim: set noet sw=4 ts=4:
