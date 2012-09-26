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
#       David Tardon, Red Hat Inc. <dtardon@redhat.com>
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.
#*************************************************************************

$(eval $(call gb_CppunitTest_CppunitTest,sc_ucalc))

$(eval $(call gb_CppunitTest_add_exception_objects,sc_ucalc, \
    sc/qa/unit/ucalc \
))

$(eval $(call gb_CppunitTest_use_library_objects,sc_ucalc,sc))

$(eval $(call gb_CppunitTest_use_external,sc_ucalc,mdds_headers))

ifeq ($(ENABLE_TELEPATHY),TRUE)
$(eval $(call gb_CppunitTest_use_libraries,sc_ucalc,tubes))
endif

$(eval $(call gb_CppunitTest_use_externals,sc_ucalc,\
	orcus \
))

$(eval $(call gb_CppunitTest_use_libraries,sc_ucalc, \
    avmedia \
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    drawinglayer \
    editeng \
    for \
    forui \
    i18nisolang1 \
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
	test \
    tk \
    tl \
    ucbhelper \
    utl \
    vbahelper \
    vcl \
    xo \
	$(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_add_standard_system_libs,sc_ucalc))

$(eval $(call gb_CppunitTest_set_include,sc_ucalc,\
    -I$(SRCDIR)/sc/source/ui/inc \
    -I$(SRCDIR)/sc/source/core/inc \
    -I$(SRCDIR)/sc/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sc_ucalc,\
    offapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_ure,sc_ucalc))

$(eval $(call gb_CppunitTest_use_components,sc_ucalc,\
    configmgr/source/configmgr \
    framework/util/fwk \
    i18npool/util/i18npool \
    sfx2/util/sfx \
    ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
    unoxml/source/service/unoxml \
))

$(eval $(call gb_CppunitTest_use_configuration,sc_ucalc))

# vim: set noet sw=4 ts=4:
