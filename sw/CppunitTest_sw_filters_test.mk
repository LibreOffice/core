# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2011 Red Hat, Inc., Stephan Bergmann <sbergman@redhat.com>
#  (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.
#*************************************************************************

$(eval $(call gb_CppunitTest_CppunitTest,sw_filters_test))

ifeq ($(SYSTEM_LIBXML),YES)
$(eval $(call gb_CppunitTest_add_cxxflags,sw_filters_test,\
	$(LIBXML_CFLAGS) \
))
endif

$(eval $(call gb_CppunitTest_add_exception_objects,sw_filters_test, \
    sw/qa/core/filters-test \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_filters_test, \
    sw \
    sfx \
    svl \
    svt \
	test \
    tl \
    ucbhelper \
    unotest \
    utl \
    vcl \
    i18nisolang1 \
    comphelper \
    cppu \
    cppuhelper \
    sal \
    $(gb_STDLIBS) \
))

$(eval $(call gb_CppunitTest_set_include,sw_filters_test,\
    -I$(SRCDIR)/sw/source/ui/inc \
    -I$(SRCDIR)/sw/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sw_filters_test,\
    offapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_ure,sw_filters_test))

$(eval $(call gb_CppunitTest_use_components,sw_filters_test,\
    comphelper/util/comphelp \
    configmgr/source/configmgr \
    dbaccess/util/dba \
    fileaccess/source/fileacc \
    forms/util/frm \
    framework/util/fwk \
    i18npool/util/i18npool \
    package/util/package2 \
    sfx2/util/sfx \
    svtools/util/svt \
    sw/util/msword \
    sw/util/sw \
    toolkit/util/tk \
    ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
    unoxml/source/service/unoxml \
	$(if $(filter DESKTOP,$(BUILD_TYPE)),xmlhelp/util/ucpchelp1) \
))

$(eval $(call gb_CppunitTest_use_configuration,sw_filters_test))

# vim: set noet sw=4 ts=4:
