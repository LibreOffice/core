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
# The Initial Developer of the Original Code is
#       Bjoern Michaelsen, Canonical Ltd. <bjoern.michaelsen@canonical.com>
# Portions created by the Initial Developer are Copyright (C) 2010 the
# Initial Developer. All Rights Reserved.
#
# Major Contributor(s):
#       Kohei Yoshida <kohei.yoshida@suse.com>
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.
#*************************************************************************

$(eval $(call gb_CppunitTest_CppunitTest,editeng_core))

$(eval $(call gb_CppunitTest_add_exception_objects,editeng_core, \
    editeng/qa/unit/core-test \
))

$(eval $(call gb_CppunitTest_use_library_objects,editeng_core,editeng))

$(eval $(call gb_CppunitTest_use_libraries,editeng_core, \
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    i18nisolang1 \
    i18nutil \
    lng \
    sal \
    salhelper \
    sot \
    svl \
    svt \
    test \
    tk \
    tl \
    ucbhelper \
    utl \
    vcl \
    xo \
	$(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_add_standard_system_libs,editeng_core))

$(eval $(call gb_CppunitTest_use_externals,editeng_core,\
    icuuc \
))

$(eval $(call gb_CppunitTest_set_include,editeng_core,\
    -I$(SRCDIR)/editeng/source \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,editeng_core,\
    offapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_ure,editeng_core))

$(eval $(call gb_CppunitTest_use_components,editeng_core,\
    configmgr/source/configmgr \
    framework/util/fwk \
    i18npool/util/i18npool \
    sfx2/util/sfx \
    ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
    unoxml/source/service/unoxml \
))

$(eval $(call gb_CppunitTest_use_configuration,editeng_core))

# vim: set noet sw=4 ts=4:
