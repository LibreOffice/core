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
	$(call gb_Helper_optional,AFMS,afms) \
    animations \
	$(call gb_Helper_optional,APACHE_COMMONS,apache-commons) \
    apple_remote \
    autodoc \
    avmedia \
    basctl \
    basebmp \
    basegfx \
    basic \
    bean \
	$(call gb_Helper_optional,BSH,beanshell) \
    binaryurp \
	$(call gb_Helper_optional,BLUEZ,bluez_bluetooth) \
	boost \
	bridges \
    canvas \
    chart2 \
	cli_ure \
	$(call gb_Helper_optional,CLUCENE,clucene) \
	$(call gb_Helper_optional,DESKTOP,codemaker) \
    comphelper \
    configmgr \
    connectivity \
    cosv \
    cppcanvas \
    cppu \
    cppuhelper \
    cpputools \
        $(call gb_Helper_optional,CT2N,ct2n) \
    cui \
    dbaccess \
    desktop \
	$(call gb_Helper_optional,DICTIONARIES,dictionaries) \
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
	$(call gb_Helper_optional,FONTCONFIG,fontconfig) \
    forms \
    formula \
    fpicker \
    framework \
	$(call gb_Helper_optional,DESKTOP,helpcompiler) \
	$(call gb_Helper_optional,HSQLDB,hsqldb) \
    hwpfilter \
    i18npool \
    i18nutil \
    idl \
	$(call gb_Helper_optional,DESKTOP,idlc) \
    io \
    javaunohelper \
	$(call gb_Helper_optional,JFREEREPORT,jfreereport) \
    jurt \
    $(call gb_Helper_optional,JPEG,jpeg) \
    jvmaccess \
    jvmfwk \
	$(call gb_Helper_optional,DESKTOP,l10ntools) \
	$(call gb_Helper_optional,LANGUAGETOOL,languagetool) \
	libcdr \
	libcmis \
	libmspub \
	liborcus \
	libpng \
	$(call gb_Helper_optional,LIBRSVG,librsvg) \
	libvisio \
	libwpd \
	libwpg \
	libwps \
    lingucomponent \
    linguistic \
    lotuswordpro \
    MathMLDTD \
	mdds \
    Mesa \
	$(call gb_Helper_optional,MORE_FONTS,more_fonts) \
	$(call gb_Helper_optional,NEON,neon) \
	$(call gb_Helper_optional,NLPSOLVER,nlpsolver) \
    np_sdk \
    o3tl \
    offapi \
    officecfg \
    oovbaapi \
    oox \
    package \
    packimages \
    padmin \
    $(call gb_Helper_optional,POSTGRESQL,postgresql) \
    psprint_config \
	$(call gb_Helper_optional,PYUNO,pyuno) \
	$(if $(strip $(OOO_JUNIT_JAR)),\
		qadevOOo \
	) \
	readlicense_oo \
    regexp \
    registry \
    remotebridges \
    reportbuilder \
    reportdesign \
	$(call gb_Helper_optional,RHINO,rhino) \
    ridljar \
    rsc \
    sal \
    salhelper \
    sane \
    sax \
	$(call gb_Helper_optional,SAXON,saxon) \
    sc \
    scaddins \
    sccomp \
	$(call gb_Helper_optional,DESKTOP,scp2) \
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
	$(call gb_Helper_optional,TOMCAT,tomcat) \
    toolkit \
    tools \
	touch \
	$(call gb_Helper_optional,TRANSLATIONS,translations) \
	tubes \
    twain \
    ucb \
    ucbhelper \
	$(call gb_Helper_optional,UCPP,ucpp) \
    udkapi \
	udm \
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
	$(call gb_Helper_optional,X11_EXTENSIONS,x11_extensions) \
    xmerge \
	$(call gb_Helper_optional,DESKTOP,xmlhelp) \
    xmloff \
    xmlreader \
    xmlscript \
    xmlsecurity \
	xsltml \
))

# vim: set noet ts=4 sw=4:
