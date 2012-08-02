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

$(eval $(call gb_Helper_register_executables,NONE, \
    bestreversemap \
    bmp \
    bmpsum \
    checksingleton \
    cfgex \
    cppunit/cppunittester \
    g2g \
    gencoll_rule \
    genconv_dict \
    gendict \
    genindex_data \
    gsicheck \
    helpex \
    idxdict \
    lngconvex \
    localize \
    mkunroll \
    osl_process_child \
    pdf2xml \
    pdfunzip \
    rdbmaker \
    regsingleton \
    regsvrex \
    rsc \
    rscdep \
    saxparser \
    so_checksum \
    sp2bv \
    svg2odf \
    svidl \
    transex3 \
    typesconfig \
    ulfconv \
    ulfex \
    xml2cmp \
    xrmex \
    xpdfimport \
))

$(eval $(call gb_Helper_register_executables,SDK, \
    idlc \
))

ifneq ($(OS),IOS)

$(eval $(call gb_Helper_register_executables,SDK, \
    autodoc \
    javamaker \
    cppumaker \
    regcompare \
    uno-skeletonmaker \
))

endif

$(eval $(call gb_Helper_register_executables,OOO, \
    gnome-open-url.bin \
    HelpLinker \
    HelpIndexer \
    spadmin.bin \
	$(if $(filter $(GUIBASE)$(ENABLE_TDE),unxTRUE), \
		tdefilepicker \
	) \
    uri-encode \
	$(if $(filter $(GUIBASE)$(ENABLE_KDE),unxTRUE), \
		kdefilepicker \
	) \
))

ifeq ($(OS),WNT)

$(eval $(call gb_Helper_register_executables,OOO,\
	crashrep_com \
	gengal \
	guiloader \
	guistdio \
	langsupport \
	odbcconfig \
	officeloader \
	pyuno/python \
	quickstart \
	sbase \
	scalc \
	sdraw \
    senddoc \
	simpress \
	soffice \
	soffice_bin \
	smath \
	sweb \
	swriter \
	unoinfo \
	unopkg \
	unopkg_bin \
	unopkg_com \
	unopkgio \
))

$(eval $(call gb_Helper_register_static_libraries,PLAINLIBS, \
	directxcanvas \
	winextendloaderenv \
	winlauncher \
))

else

$(eval $(call gb_Helper_register_executables,OOO,\
    langsupport \
    soffice.bin \
    unopkg.bin \
    gengal.bin \
    liboapprover \
))

ifeq ($(OS),MACOSX)

$(eval $(call gb_Helper_register_executables,OOO,\
    officeloader \
))

else

$(eval $(call gb_Helper_register_executables,OOO,\
    oosplash \
))

endif

endif

ifeq ($(OS),IOS)

$(eval $(call gb_Helper_register_executables,OOO,\
	basegfx_cppunittester_all \
	basebmp_cppunittester_all \
    i18npool_cppunittester_all \
    sax_cppunittester_all \
	tools_cppunittester_all \
))

endif

ifneq ($(OS),IOS)

$(eval $(call gb_Helper_register_executables,UREBIN,\
	regcomp \
	regmerge \
	regview \
	uno \
))

endif

ifneq ($(OS),MACOSX)

$(eval $(call gb_Helper_register_executables,UREBIN,\
	javaldx \
))

endif

ifeq ($(ENABLE_NSPLUGIN),YES)
$(eval $(call gb_Helper_register_executables,OOO,\
    nsplugin \
))
$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
    npsoplugin \
))
endif

ifeq ($(WITH_MOZILLA),YES)
$(eval $(call gb_Helper_register_executables,OOO,\
    pluginapp.bin \
))
$(eval $(call gb_Helper_register_libraries,OOOLIBS, \
    pl \
))
endif # WITH_MOZILLA


ifeq ($(OS),MACOSX)

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_NONE, \
    OOoSpotlightImporter \
))

endif

$(eval $(call gb_Helper_register_libraries,OOOLIBS, \
    PptImporter \
    abp \
    acc \
	adabas \
	adabasui \
	ado \
    agg \
    analysis \
    animcore \
    basctl \
    basebmp \
    bib \
    canvastools \
    communi \
    date \
    evtatt \
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
    calc \
    chartcontroller \
    chartmodel \
    charttools \
    chartview \
    cppcanvas \
    ctl \
    cui \
    dba \
    dbase \
    dbaxml \
    dbmm \
    dbp \
    dbtools \
    dbu \
    deploymentmisc \
    desktop_detector \
    drawinglayer \
    editeng \
    egi \
    eme \
    emser \
    epb \
    epg \
    epp \
    eps \
    ept \
    era \
    eti \
    evoab \
    exp \
	file \
	filterconfig \
	filtertracer \
	flash \
	flat \
    for \
    frm \
    fwe \
    fwi \
    fwk \
    fwl \
    fwm \
    guesslang \
    helplinker \
    hwp \
    hyphen \
    i18nregexp \
	icd \
	icg \
	idx \
	ime \
	ipb \
	ipd \
	ips \
	ipt \
	ipx \
	ira \
	itg \
	iti \
	jdbc \
	kab1 \
	kabdrv1 \
    lng \
    lnth \
    log \
    lwpft \
	MacOSXSpell \
    merged \
    mozab2 \
    mozabdrv \
    msfilter \
    msword \
	mysql \
    odfflatxml \
    offacc \
    pcr \
    pdffilter \
    placeware \
    protocolhandler \
    qstart_gtk \
    res \
    rpt \
    rptui \
    rptxml \
    sax \
    sb \
    sc \
    scd \
    scfilt \
    scn \
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
    spell \
    spl \
    sts \
    svgfilter \
    svl \
    svt \
    svx \
    svxcore \
    sw \
    swd \
    swui \
    t602filter \
    textconversiondlgs \
    textfd \
    tk \
    tl \
	tubes \
    unordf \
    unoxml \
    updatecheckui \
    utl \
    uui \
    vbahelper \
    vcl \
    vclplug_gen \
    vclplug_gtk \
    vclplug_gtk3 \
    vclplug_tde \
    vclplug_kde \
    vclplug_kde4 \
    vclplug_svp \
    wpftdraw \
    wpftwriter \
    writerfilter \
    xmlscript \
    xmlfa \
    xmlfd \
    xo \
    xof \
    xsltdlg \
    xsltfilter \
))

ifneq (,$(filter ANDROID IOS,$(OS)))

$(eval $(call gb_Helper_register_libraries,OOOLIBS, \
	libotouch \
))

endif

ifeq ($(filter MACOSX WNT,$(OS)),)

$(eval $(call gb_Helper_register_libraries,OOOLIBS, \
    spl_unx \
))

endif

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_URE, \
    affine_uno \
    jpipe \
    juh \
    juhx \
    log_uno \
    sal_textenc \
    sunjavaplugin \
    unsafe_uno \
    xmlreader \
))

ifeq ($(OS),WNT)

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_URE, \
    jpipx \
))

endif

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
    avmediagst \
    avmediawin \
    cached1 \
    collator_data \
    dbpool2 \
    deployment \
    deploymentgui \
    dict_ja \
    dict_zh \
    embobj \
    emboleobj \
    fileacc \
    hsqldb \
    index_data \
    java_uno_accessbridge \
    localedata_en \
    localedata_es \
    localedata_euro \
    localedata_others \
    macab1 \
    macabdrv1 \
    mcnttype \
    mozbootstrap \
    officebean \
    package2 \
    pyuno \
    pyuno_wrapper \
    recentfile \
    scriptframe \
    sdbc2 \
    smoketest \
    sofficeapp \
    srtrs1 \
    subsequenttest \
    test \
    textconv_dict \
    tvhlp1 \
    ucb1 \
    ucpdav1 \
    ucpfile1 \
    ucpftp1 \
    ucpchelp1 \
    ucphier1 \
    ucppkg1 \
    unobootstrapprotector \
    unoexceptionprotector \
    unopkgapp \
    unotest \
    updchk \
    xmlsecurity \
    xsec_fw \
    xsec_xmlsec \
    xstor \
    $(if $(filter $(OS),ANDROID), \
        lo-bootstrap \
    ) \
    $(if $(filter $(OS),WNT), \
        ucpodma1 \
        uwinapi \
    ) \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
    dnd \
    dtrans \
    fop \
    fps \
    ftransl \
    inprocserv \
    ooofilt \
    ooofilt_x64 \
    propertyhdl \
    propertyhdl_x64 \
    shlxthdl \
    shlxthdl_x64 \
    so_activex \
    so_activex_x64 \
    sysdtrans \
))
endif

ifeq ($(OS),IOS)
$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
    gcc3_uno \
))
endif

$(eval $(call gb_Helper_register_libraries,RTLIBS, \
    comphelper \
    i18nisolang1 \
    i18nutil \
    ucbhelper \
))

$(eval $(call gb_Helper_register_libraries,RTVERLIBS, \
    cppuhelper \
    jvmaccess \
    purpenvhelper \
    salhelper \
))

$(eval $(call gb_Helper_register_libraries,UNOLIBS_OOO, \
    basprov \
    cairocanvas \
    canvasfactory \
    cmdmail \
    configmgr \
    desktopbe1 \
    directx5canvas \
    directx9canvas \
    dlgprov \
    expwrap \
    fastsax \
    fpicker \
    fps_office \
    fsstorage \
    gconfbe1 \
    gdipluscanvas \
    hatchwindowfactory \
    i18npool \
    i18nsearch \
    kdebe1 \
    kde4be1 \
    ldapbe2 \
    localebe1 \
    lomenubar \
    macbe1 \
    migrationoo2 \
    migrationoo3 \
    msforms \
    mtfrenderer \
    nullcanvas \
    OGLTrans \
    passwordcontainer \
    pythonloader \
    postgresql-sdbc \
    postgresql-sdbc-impl \
    simplecanvas \
    slideshow \
    stringresource \
    syssh \
    ucpcmis1 \
    ucpexpand1 \
    ucpext \
    ucpgio1 \
    ucpgvfs1 \
    ucptdoc1 \
    updatefeed \
    vbaevents \
    vbaobj \
    vbaswobj \
    vclcanvas \
    writerfilter_uno \
    writerfilter_debug \
    $(if $(filter $(GUIBASE),aqua), \
        fps_aqua \
    ) \
    $(if $(filter $(OS),WNT), \
        fps_odma \
    ) \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Helper_register_libraries,UNOLIBS_OOO, \
    oleautobridge \
    oleautobridge2 \
    smplmail \
    wininetbe1 \
))
endif

$(eval $(call gb_Helper_register_libraries,UNOLIBS_URE, \
    acceptor \
    binaryurp \
    bootstrap \
    connector \
    introspection \
    invocadapt \
    invocation \
    javaloader \
    javavm \
    namingservice \
    proxyfac \
    reflection \
    stocservices \
    streams \
    testtools_cppobj \
    testtools_bridgetest \
    testtools_constructors \
    textinstream \
    textoutstream \
    uuresolver \
))

$(eval $(call gb_Helper_register_libraries,UNOVERLIBS, \
    cppu \
    jvmfwk \
    reg \
    sal \
    store \
))

$(eval $(call gb_Helper_register_libraries,EXTENSIONLIBS, \
    pdfimport \
    PresenterScreen \
	SunPresentationMinimizer \
))

$(eval $(call gb_Helper_register_static_libraries,PLAINLIBS, \
    basegfx_s \
    codemaker \
    codemaker_cpp \
    codemaker_java \
    cosv \
    dtobj \
    headless \
    libeay32 \
    npsoenv \
    nputils \
    pdfimport_s \
    plugcon \
    ssleay32 \
    ooopathutils \
    sample \
    simplemapi \
    shell_xmlparser \
    shell_xmlparser_x64 \
    shlxthandler_common \
    shlxthandler_common_x64 \
    salcpprt \
    testtools_bridgetest_s \
    transex \
    udm \
    ulingu \
    vclmain \
    writerperfect \
    $(if $(filter-out $(OS),IOS), \
        registry_helper \
    ) \
    $(if $(filter $(OS),IOS), \
        uno \
    ) \
    $(if $(filter $(OS),WNT), \
        odma_lib \
    ) \
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
