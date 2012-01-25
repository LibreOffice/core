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

$(eval $(call gb_CppunitTest_CppunitTest,sd_filters_test))

$(eval $(call gb_CppunitTest_add_exception_objects,sd_filters_test, \
    sd/qa/unit/filters-test \
))

$(eval $(call gb_CppunitTest_add_linked_libs,sd_filters_test, \
    avmedia \
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    drawinglayer \
    editeng \
    fileacc \
    for \
    forui \
    i18nisolang1 \
    msfilter \
    oox \
    sal \
    salhelper \
    sax \
    sd \
    sfx \
    sot \
    svl \
    svt \
    svx \
    svxcore \
	test \
    tl \
    tk \
    ucbhelper \
	unotest \
    utl \
    vcl \
    xo \
	$(gb_STDLIBS) \
))

$(eval $(call gb_CppunitTest_set_include,sd_filters_test,\
    -I$(realpath $(SRCDIR)/sd/source/ui/inc) \
    -I$(realpath $(SRCDIR)/sd/inc) \
    $$(INCLUDE) \
    -I$(OUTDIR)/inc \
))

$(eval $(call gb_CppunitTest_add_api,sd_filters_test,\
    offapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_uses_ure,sd_filters_test))

$(eval $(call gb_CppunitTest_add_type_rdbs,sd_filters_test,\
    types \
))

$(eval $(call gb_CppunitTest_add_components,sd_filters_test,\
    basic/util/sb \
    comphelper/util/comphelp \
    configmgr/source/configmgr \
    dbaccess/util/dba \
    fileaccess/source/fileacc \
    filter/source/config/cache/filterconfig1 \
    forms/util/frm \
    framework/util/fwk \
    i18npool/util/i18npool \
    oox/util/oox \
    package/source/xstor/xstor \
    package/util/package2 \
    sax/source/expatwrap/expwrap \
    sax/source/fastparser/fastsax \
    sd/util/sd \
    sd/util/sdfilt \
    sfx2/util/sfx \
    sot/util/sot \
    toolkit/util/tk \
    ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
    ucb/source/ucp/tdoc/ucptdoc1 \
    unotools/util/utl \
    unoxml/source/rdf/unordf \
    unoxml/source/service/unoxml \
    xmlsecurity/util/xsec_fw \
    xmlsecurity/util/xmlsecurity \
))

$(eval $(call gb_CppunitTest_add_old_components,sd_filters_test,\
    embobj \
))

$(eval $(call gb_CppunitTest_set_args,sd_filters_test,\
    --headless \
    --protector unoexceptionprotector$(gb_Library_DLLEXT) unoexceptionprotector \
    "-env:CONFIGURATION_LAYERS=xcsxcu:$(call gb_CppunitTarget__make_url,$(OUTDIR)/xml/registry) module:$(call gb_CppunitTarget__make_url,$(OUTDIR)/xml/registry/spool)" \
))
    # .../spool is required for the (somewhat strange) filter configuration

# we need to explicitly depend on library sdfilt because it is not implied
# by a link relation
$(call gb_CppunitTest_get_target,sd_filters_test) : $(call gb_Library_get_target,sdfilt)

# vim: set noet sw=4 ts=4:
