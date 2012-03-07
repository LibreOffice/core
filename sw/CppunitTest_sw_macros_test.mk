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

$(eval $(call gb_CppunitTest_CppunitTest,sw_macros_test))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_macros_test, \
    sw/qa/core/macros-test \
))

$(eval $(call gb_CppunitTest_add_linked_libs,sw_macros_test, \
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
    sb \
    sw \
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
    vbahelper \
    vcl \
    xo \
	$(gb_STDLIBS) \
))

$(eval $(call gb_CppunitTest_set_include,sw_macros_test,\
    -I$(realpath $(SRCDIR)/sw/source/ui/inc) \
    -I$(realpath $(SRCDIR)/sw/inc) \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_add_api,sw_macros_test,\
    offapi \
    oovbaapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_uses_ure,sw_macros_test))

$(eval $(call gb_CppunitTest_add_type_rdbs,sw_macros_test,\
    oovbaapi \
    types \
))

$(eval $(call gb_CppunitTest_add_components,sw_macros_test,\
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
    sw/util/sw \
    sw/util/swd \
    sw/util/msword \
    sw/util/vbaswobj \
    scripting/source/basprov/basprov \
    scripting/util/scriptframe \
    sfx2/util/sfx \
    sot/util/sot \
    svl/source/fsstor/fsstorage \
    toolkit/util/tk \
    ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
    ucb/source/ucp/tdoc/ucptdoc1 \
    unotools/util/utl \
    unoxml/source/rdf/unordf \
    unoxml/source/service/unoxml \
))

$(eval $(call gb_CppunitTest_set_args,sw_macros_test,\
    --headless \
    --protector unoexceptionprotector$(gb_Library_DLLEXT) unoexceptionprotector \
    "-env:CONFIGURATION_LAYERS=xcsxcu:$(call gb_CppunitTarget__make_url,$(OUTDIR)/xml/registry) module:$(call gb_CppunitTarget__make_url,$(OUTDIR)/xml/registry/spool) xcsxcu:$(call gb_CppunitTarget__make_url,$(OUTDIR)/unittest/registry)" \
))
    # .../spool is required for the (somewhat strange) filter configuration

# we need to
# a) explicitly depend on library msword because it is not implied by a link
#    relation
# b) explicitly depend on the sw resource files needed at unit-test runtime
$(call gb_CppunitTest_get_target,sw_macros_test) : \
    $(call gb_Library_get_target,localedata_en) \
    $(call gb_Library_get_target,msword) \
    $(if $(filter SCRIPTING,$(BUILD_TYPE)),$(call gb_Library_get_target,vbaswobj)) \

# vim: set noet sw=4 ts=4:
