# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2009 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.	If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

$(eval $(call gb_Helper_register_repository,SRCDIR))


$(eval $(call gb_Helper_register_executables,NONE, \
    bmp \
    bmpsum \
    g2g \
    mkunroll \
    rscdep \
    so_checksum \
    xml2cmp \
))

$(eval $(call gb_Helper_register_executables,OOO, \
    spadmin.bin \
    testtool \
))

$(eval $(call gb_Helper_register_libraries,OOOLIBS, \
	adabas \
	adabasui \
	agg \
	analysis \
	animcore \
	basctl \
	basebmp \
	canvastools \
	communi \
	date \
	forui \
	odbc \
	odbcbase \
	oox \
	ooxml \
	sdbc \
    AppleRemote \
    avmedia \
	avmediaQuickTime \
    basegfx \
    chartcontroller \
    chartmodel \
    charttools \
    chartview \
    cppcanvas \
    cui \
    dba \
    dbaxml \
    dbmm \
    dbtools \
    dbu \
    desktop_detector \
    doctok \
    drawinglayer \
    editeng \
    for \
    frm \
    fwe \
    fwi \
    fwk \
    fwl \
    fwm \
    hwp \
    i18npaper \
    i18nregexp \
    lng \
    lwpft \
    merged \
    msfilter \
    msword \
	msworks \
    qstart_gtk \
    resourcemodel \
    rpt \
    rptui \
    rptxml \
	rtftok \
    sax \
    sb \
    sc \
    scd \
    scfilt \
    scui \
    sd \
    sdbt \
    sdd \
    sdfilt \
    sdui \
    sfx \
    simplecm \
    sm \
    smd \
	solver \
    sot \
    spa \
    sts \
    svl \
    svt \
    svx \
    svxcore \
    sw \
    swd \
    swui \
    textconversiondlgs \
    tk \
    tl \
    unordf \
    unoxml \
    utl \
	uui \
    vbahelper \
    vcl \
    vclplug_gen \
    vclplug_gtk \
    vclplug_gtk3 \
    vclplug_kde \
    vclplug_kde4 \
    vclplug_svp \
	visioimport \
	wpft \
	wpgimport \
    writerfilter \
    xcr \
    xo \
    xof \
))

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_URE, \
	xmlreader \
))

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
	avmediagst \
	avmediawin \
	collator_data \
	dict_ja \
	dict_zh \
	fileacc \
	index_data \
	localedata_en \
	localedata_es \
	localedata_euro \
	localedata_others \
	mcnttype \
	neon \
	package2 \
    reg \
    store \
	test \
	textconv_dict \
	xmlsecurity \
	xsec_fw \
	xsec_xmlsec \
	xstor \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
	dnd \
	dtrans \
	ftransl \
	sysdtrans \
))
endif

ifeq ($(OS),IOS)
$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
    gcc3_uno \
    sal_textenc \
))
endif

$(eval $(call gb_Helper_register_libraries,RTLIBS, \
    comphelper \
    i18nisolang1 \
    i18nutil \
    jvmaccess \
    ucbhelper \
))

$(eval $(call gb_Helper_register_libraries,RTVERLIBS, \
    cppuhelper \
    salhelper \
))

$(eval $(call gb_Helper_register_libraries,UNOLIBS_OOO, \
	cairocanvas \
	canvasfactory \
	directx5canvas \
	directx9canvas \
	expwrap \
	fastsax \
    fsstorage \
	gdipluscanvas \
    hatchwindowfactory \
	i18npool \
	i18nsearch \
    lomenubar \
    msforms \
	mtfrenderer \
	nullcanvas \
    OGLTrans \
    passwordcontainer \
	simplecanvas \
    slideshow \
    vbaobj \
    vbaswobj \
	vclcanvas \
    writerfilter_uno \
	writerfilter_debug \
))

ifeq ($(OS),IOS)
$(eval $(call gb_Helper_register_libraries,UNOLIBS_OOO, \
    bootstrap \
))
endif

$(eval $(call gb_Helper_register_libraries,UNOVERLIBS, \
    cppu \
    jvmfwk \
    sal \
))

$(eval $(call gb_Helper_register_static_libraries,PLAINLIBS, \
	app \
	basegfx_s \
    dtobj \
    libeay32 \
    ssleay32 \
    ooopathutils \
	salmain \
	sample \
    salcpprt \
	vclmain \
	writerperfect \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
	xmlsec1 \
	xmlsec1-nss \
	xmlsec1-mscrypto \
))
else
$(eval $(call gb_Helper_register_static_libraries,PLAINLIBS, \
	xmlsec1 \
	xmlsec1-nss \
	xmlsec1-mscrypto \
))
endif

# vim: set noet sw=4 ts=4:
