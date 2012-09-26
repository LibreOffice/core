# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
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
# Copyright (C) 2010 Red Hat, Inc., David Tardon <dtardon@redhat.com>
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

$(eval $(call gb_CppunitTest_CppunitTest,sd_uimpress))

$(eval $(call gb_CppunitTest_set_include,sd_uimpress,\
    $$(INCLUDE) \
    -I$(SRCDIR)/sd/inc \
))

$(eval $(call gb_CppunitTest_add_defs,sd_uimpress,\
    -DSD_DLLIMPLEMENTATION \
))

$(eval $(call gb_CppunitTest_use_api,sd_uimpress,\
    offapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_library_objects,sd_uimpress,sd))

$(eval $(call gb_CppunitTest_use_libraries,sd_uimpress,\
    avmedia \
    basegfx \
    canvastools \
    comphelper \
    cppcanvas \
    cppu \
    cppuhelper \
    drawinglayer \
    editeng \
    i18nisolang1 \
    msfilter \
    sal \
    sax \
    salhelper \
    sb \
    sfx \
    sot \
    svl \
    svt \
    svxcore \
    svx \
    tk \
    tl \
    ucbhelper \
    utl \
    vcl \
    xo \
	$(gb_UWINAPI) \
    $(gb_STDLIBS) \
))

ifeq ($(OS),WNT)
$(eval $(call gb_CppunitTest_use_libraries,sd_uimpress,\
	ws2_32 \
))
endif

$(eval $(call gb_CppunitTest_use_externals,sd_uimpress,\
    gtk \
    dbus \
))

$(eval $(call gb_CppunitTest_add_exception_objects,sd_uimpress,\
    sd/qa/unit/uimpress \
))

$(eval $(call gb_CppunitTest_use_ure,sd_uimpress))

$(eval $(call gb_CppunitTest_use_components,sd_uimpress,\
    configmgr/source/configmgr \
    framework/util/fwk \
    i18npool/util/i18npool \
    ucb/source/core/ucb1 \
))

$(eval $(call gb_CppunitTest_use_configuration,sd_uimpress))

# vim: set noet sw=4 ts=4:
