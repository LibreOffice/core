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
# Copyright (C) 2011 Red Hat, Inc., Caolán McNamara <caolanm@redhat.com>
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

$(eval $(call gb_CppunitTest_CppunitTest,sw_swdoc_test))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_swdoc_test, \
    sw/qa/core/swdoc-test \
    sw/qa/core/Test-BigPtrArray \
))

$(eval $(call gb_CppunitTest_use_library_objects,sw_swdoc_test,sw))

$(eval $(call gb_CppunitTest_use_libraries,sw_swdoc_test, \
    avmedia \
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    drawinglayer \
    editeng \
    i18nisolang1 \
    i18nutil \
    lng \
    oox \
    sal \
    salhelper \
    sax \
    sb \
    sfx \
    sot \
    svl \
    svt \
    svx \
    svxcore \
    swd \
	test \
    tk \
    tl \
    ucbhelper \
    utl \
    vbahelper \
    vcl \
    xo \
    $(gb_STDLIBS) \
))

$(eval $(call gb_CppunitTest_use_externals,sw_swdoc_test,\
	icuuc \
	libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sw_swdoc_test,\
    -I$(SRCDIR)/sw/source/ui/inc \
    -I$(SRCDIR)/sw/source/core/inc \
    -I$(SRCDIR)/sw/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sw_swdoc_test,\
    offapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_ure,sw_swdoc_test))

$(eval $(call gb_CppunitTest_use_components,sw_swdoc_test,\
    comphelper/util/comphelp \
    configmgr/source/configmgr \
    fileaccess/source/fileacc \
    framework/util/fwk \
    i18npool/util/i18npool \
    sfx2/util/sfx \
    ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
    unoxml/source/service/unoxml \
))

$(eval $(call gb_CppunitTest_use_configuration,sw_swdoc_test))

# we need to explicitly depend on the sw resource files needed at unit-test
# runtime
$(call gb_CppunitTest_get_target,sw_swdoc_test) : \
    $(WORKDIR)/AllLangRes/sw \
    $(call gb_AllLangResTarget_get_target,sw) \

# vim: set noet sw=4 ts=4:
