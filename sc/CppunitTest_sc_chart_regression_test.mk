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

$(eval $(call gb_CppunitTest_CppunitTest,sc_chart_regression_test))

$(eval $(call gb_CppunitTest_add_exception_objects,sc_chart_regression_test, \
    sc/qa/extras/regression-test \
))

$(eval $(call gb_CppunitTest_use_libraries,sc_chart_regression_test, \
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
    sc \
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

$(eval $(call gb_CppunitTest_set_include,sc_chart_regression_test,\
    -I$(SRCDIR)/sc/source/ui/inc \
    -I$(SRCDIR)/sc/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sc_chart_regression_test,\
    offapi \
    oovbaapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_ure,sc_chart_regression_test))

$(eval $(call gb_CppunitTest_use_components,sc_chart_regression_test,\
    basic/util/sb \
    chart2/source/controller/chartcontroller \
    chart2/source/chartcore \
    comphelper/util/comphelp \
    configmgr/source/configmgr \
    dbaccess/util/dba \
    embeddedobj/util/embobj \
    eventattacher/source/evtatt \
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
    sc/util/sc \
    sc/util/scd \
    sc/util/scfilt \
    sc/util/vbaobj \
    scaddins/source/analysis/analysis \
    scaddins/source/datefunc/date \
    scripting/source/basprov/basprov \
    scripting/util/scriptframe \
    sfx2/util/sfx \
    sot/util/sot \
    svl/source/fsstor/fsstorage \
    svl/util/svl \
    svx/util/svx \
    toolkit/util/tk \
    ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
    ucb/source/ucp/tdoc/ucptdoc1 \
    unotools/util/utl \
    unoxml/source/rdf/unordf \
    unoxml/source/service/unoxml \
    xmloff/util/xo \
))

$(eval $(call gb_CppunitTest_use_configuration,sc_chart_regression_test))

$(eval $(call gb_CppunitTest_use_filter_configuration,sc_chart_regression_test))

$(eval $(call gb_CppunitTest_use_unittest_configuration,sc_chart_regression_test))

$(eval $(call gb_CppunitTest_use_external,sc_chart_regression_test,libxml2))

# we need to
# - explicitly depend on the sc resource files needed at unit-test runtime
$(call gb_CppunitTest_get_target,sc_chart_regression_test) : \
    $(WORKDIR)/AllLangRes/sc \

# vim: set noet sw=4 ts=4:
