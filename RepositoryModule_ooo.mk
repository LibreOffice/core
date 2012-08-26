# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2000, 2010 Oracle and/or its affiliates.
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org. If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

$(eval $(call gb_Module_Module,ooo))

$(eval $(call gb_Module_add_moduledirs,ooo,\
    accessibility \
    animations \
    autodoc \
    apple_remote \
    avmedia \
    basctl \
    basebmp \
    basegfx \
    basic \
    bean \
    binaryurp \
    bluez_bluetooth \
	boost \
    canvas \
    chart2 \
    clucene \
    codemaker \
    comphelper \
    configmgr \
    connectivity \
    cppcanvas \
    cppu \
    cppuhelper \
    cpputools \
    cui \
    dbaccess \
    desktop \
    drawinglayer \
    dtrans \
    editeng \
    embeddedobj \
    embedserv \
    eventattacher \
    extensions \
	extras \
    fileaccess \
    filter \
    forms \
    formula \
    fpicker \
    framework \
	hsqldb \
    hwpfilter \
    i18npool \
    i18nutil \
    idl \
    idlc \
    io \
    javaunohelper \
    jurt \
    jvmaccess \
    jvmfwk \
    l10ntools \
    lingucomponent \
    linguistic \
    lotuswordpro \
    MathMLDTD \
    Mesa \
    nlpsolver \
    np_sdk \
    o3tl \
    offapi \
    officecfg \
    oovbaapi \
    oox \
    package \
    packimages \
    padmin \
    psprint_config \
	$(if $(strip $(OOO_JUNIT_JAR)),\
		qadevOOo \
	) \
    rdbmaker \
    regexp \
    registry \
    remotebridges \
    reportbuilder \
    reportdesign \
    ridljar \
    rsc \
    sal \
    salhelper \
    sane \
    sax \
    sc \
    scaddins \
    sccomp \
    scp2 \
    scripting \
    sd \
    sdext \
    sfx2 \
    shell \
    slideshow \
    smoketest \
    solenv \
    sot \
    starmath \
    stoc \
    store \
    svl \
    svtools \
    svx \
    sw \
    swext \
    test \
    testtools \
    toolkit \
    tools \
	touch \
	$(if $(filter TRANSLATIONS,$(BUILD_TYPE)),\
		translations \
	) \
	tubes \
    twain \
    ucb \
    ucbhelper \
    udkapi \
    unixODBC \
    UnoControls \
    unodevtools \
    unoil \
    unotest \
    unotools \
    unoxml \
    ure \
    uui \
    vbahelper \
    vcl \
	vigra \
    wizards \
    writerfilter \
    writerperfect \
	$(if $(filter X11_EXTENSIONS,$(BUILD_TYPE)),\
		x11_extensions \
	) \
    xmerge \
    xml2cmp \
    xmlhelp \
    xmloff \
    xmlreader \
    xmlscript \
    xmlsecurity \
))

# vim: set noet ts=4 sw=4:
