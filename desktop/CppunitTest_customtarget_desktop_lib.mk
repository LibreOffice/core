# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#*************************************************************************

$(eval $(call gb_CppunitTest_CppunitTest,customtarget_desktop_lib))

$(eval $(call gb_CppunitTest_add_exception_objects,customtarget_desktop_lib, \
	desktop/qa/custom_target/test_customtarget_desktop_lib \
))

$(eval $(call gb_CppunitTest_use_libraries,customtarget_desktop_lib, \
	comphelper \
	cppu \
	cppuhelper \
	sal \
	sfx \
	sofficeapp \
	subsequenttest \
	sw \
	test \
	unotest \
	vcl \
    $(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_use_external,customtarget_desktop_lib,boost_headers))

$(eval $(call gb_CppunitTest_use_api,customtarget_desktop_lib,\
	offapi \
	udkapi \
))

$(eval $(call gb_CppunitTest_use_ure,customtarget_desktop_lib))

$(eval $(call gb_CppunitTest_use_vcl,customtarget_desktop_lib))

$(eval $(call gb_CppunitTest_use_components,customtarget_desktop_lib,\
    comphelper/util/comphelp \
    configmgr/source/configmgr \
    filter/source/config/cache/filterconfig1 \
    filter/source/storagefilterdetect/storagefd \
    framework/util/fwk \
    i18npool/util/i18npool \
    package/source/xstor/xstor \
    package/util/package2 \
    sax/source/expatwrap/expwrap \
    sfx2/util/sfx \
    svl/source/fsstor/fsstorage \
    svtools/util/svt \
    sw/util/sw \
    sw/util/swd \
    toolkit/util/tk \
    ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
    unoxml/source/service/unoxml \
    xmloff/util/xo \
))

$(eval $(call gb_CppunitTest_use_configuration,customtarget_desktop_lib))

# vim: set noet sw=4 ts=4:
