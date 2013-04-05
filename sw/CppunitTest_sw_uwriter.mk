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

$(eval $(call gb_CppunitTest_CppunitTest,sw_uwriter))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_uwriter, \
    sw/qa/core/uwriter \
    sw/qa/core/Test-BigPtrArray \
))

$(eval $(call gb_CppunitTest_use_library_objects,sw_uwriter,sw))

$(eval $(call gb_CppunitTest_use_libraries,sw_uwriter, \
    avmedia \
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    drawinglayer \
    editeng \
    i18nlangtag \
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
	xmlreader \
    xo \
	$(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_use_externals,sw_uwriter,\
	boost_headers \
	icuuc \
	libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sw_uwriter,\
    -I$(SRCDIR)/sw/source/ui/inc \
    -I$(SRCDIR)/sw/source/core/inc \
    -I$(SRCDIR)/sw/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sw_uwriter,\
    offapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_ure,sw_uwriter))

$(eval $(call gb_CppunitTest_use_components,sw_uwriter,\
    comphelper/util/comphelp \
    configmgr/source/configmgr \
    fileaccess/source/fileacc \
    framework/util/fwk \
    i18npool/util/i18npool \
    package/util/package2 \
    package/source/xstor/xstor \
    sfx2/util/sfx \
    ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
    unotools/util/utl \
    unoxml/source/service/unoxml \
))

$(eval $(call gb_CppunitTest_use_configuration,sw_uwriter))

# vim: set noet sw=4 ts=4:
