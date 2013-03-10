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
    HelpIndexer \
    HelpLinker \
    bestreversemap \
    bmp \
    bmpsum \
    cfgex \
    cpp \
    cppunit/cppunittester \
    $(call gb_Helper_optional,CRASHREP,crashrep) \
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
    makedepend \
    mork_helper \
    osl_process_child \
    pdf2xml \
    pdfunzip \
    propex \
    reg2unoidl \
    regsvrex \
    rsc \
    rscdep \
    saxparser \
    sp2bv \
    svg2odf \
    svidl \
    stringex \
    transex3 \
    treex \
    $(if $(filter UCPP,$(BUILD_TYPE)),\
            ucpp \
    ) \
    uiex \
    ulfex \
    $(call gb_Helper_optional,ODK,unoapploader) \
    xpdfimport \
    xrmex \
))

$(eval $(call gb_Helper_register_executables,SDK, \
    idlc \
))

ifneq ($(OS),IOS)

$(eval $(call gb_Helper_register_executables,SDK, \
    autodoc \
    climaker \
    cppumaker \
    javamaker \
    regcompare \
    uno-skeletonmaker \
))

endif

$(eval $(call gb_Helper_register_executables,OOO, \
    gnome-open-url.bin \
    spadmin.bin \
	$(if $(filter $(GUIBASE)$(ENABLE_TDE),unxTRUE), \
		tdefilepicker \
	) \
    uri-encode \
	$(if $(filter $(GUIBASE)$(ENABLE_KDE),unxTRUE), \
		kdefilepicker \
	) \
	ui-previewer \
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

ifneq ($(OS),IOS)

$(eval $(call gb_Helper_register_executables,UREBIN,\
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

ifeq ($(ENABLE_NPAPI_FROM_BROWSER),YES)
$(eval $(call gb_Helper_register_executables,OOO,\
    pluginapp.bin \
))
$(eval $(call gb_Helper_register_libraries,OOOLIBS, \
    pl \
))
endif

ifeq ($(ENABLE_NPAPI_INTO_BROWSER),YES)
$(eval $(call gb_Helper_register_executables,OOO,\
    nsplugin \
))
$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
    npsoplugin \
))
endif

ifeq ($(OS),IOS)

$(eval $(call gb_Helper_register_executables,NONE,\
    Viewer \
))

endif

ifeq ($(OS),MACOSX)

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_NONE, \
    OOoSpotlightImporter \
))

endif

ifeq ($(DISABLE_SCRIPTING),)
$(eval $(call gb_Helper_register_libraries_for_install,OOOLIBS,OOO, \
    basctl \
))
endif
ifeq ($(OS),WNT)
ifeq ($(DISABLE_ATL),)
$(eval $(call gb_Helper_register_libraries_for_install,OOOLIBS,OOO, \
    emser \
))
endif
endif


$(eval $(call gb_Helper_register_libraries_for_install,OOOLIBS,OOO, \
    bib \
    chartcore \
    chartcontroller \
    ctl \
    cui \
    dba \
    dbase \
    dbmm \
    dbtools \
    dbaxml \
    drawinglayer \
	file \
    frm \
    fwe \
	fwi \
    fwl \
	fwm \
    lng \
    lwpft \
    sdbt \
    odbcbase \
    t602filter \
    wpftwriter \
    writerfilter \
))

$(eval $(call gb_Helper_register_libraries,OOOLIBS, \
    abp \
    acc \
	adabas \
	adabasui \
	ado \
    agg \
    analysis \
    animcore \
    $(if $(filter $(OS),ANDROID),, \
		basebmp \
    ) \
    canvastools \
    communi \
    date \
    evtatt \
    forui \
    odbc \
    oox \
	ooxml \
    sdbc \
    AppleRemote \
    avmedia \
    avmediaQuickTime \
    basegfx \
    calc \
    cppcanvas \
    dbp \
    dbu \
    deploymentmisc \
    desktop_detector \
    editeng \
    egi \
    eme \
    epb \
    epg \
    epp \
    eps \
    ept \
    era \
    eti \
    evoab \
    exp \
	filterconfig \
	filtertracer \
	flash \
	flat \
    for \
    fwk \
    guesslang \
    helplinker \
    hwp \
    hyphen \
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
    lnth \
    log \
	MacOSXSpell \
    merged \
    mork \
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
    pricing \
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
	svgio \
    svl \
    svt \
    svx \
    svxcore \
    sw \
    swd \
    swui \
	tdeab1 \
	tdeabdrv1 \
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
	cli_cppuhelper \
	cli_uno \
	gcc3_uno \
	getuid \
	java_uno \
    jpipe \
    juh \
    juhx \
    log_uno \
    sal_textenc \
    sunjavaplugin \
	sunpro5_uno \
    unoidl \
    unsafe_uno \
    xmlreader \
))

ifeq ($(OS),WNT)

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_URE, \
    jpipx \
	msci_uno \
	mscx_uno \
))

endif

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
    avmediagst \
    avmediagst_0_10 \
    avmediawin \
    bluez_bluetooth \
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
    instooofiltmsi \
    jfregca \
    ooofilt \
    ooofilt_x64 \
    propertyhdl \
    propertyhdl_x64 \
    qslnkmsi \
    raptor \
    rasqal \
    reg4allmsdoc \
    regactivex \
	regpatchactivex \
    sdqsmsi \
    sellangmsi \
    shlxthdl \
    shlxthdl_x64 \
    shlxtmsi \
    sn_tools \
    so_activex \
    so_activex_x64 \
    sysdtrans \
	thidxmsi \
	unowinreg \
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

$(eval $(call gb_Helper_register_libraries,OOOLIBS, \
    mtfrenderer \
    OGLTrans \
))
$(eval $(call gb_Helper_register_libraries,UNOLIBS_OOO, \
    PresenterScreen \
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
    losessioninstall \
    macbe1 \
    migrationoo2 \
    migrationoo3 \
    msforms \
    nullcanvas \
    passwordcontainer \
    pdfimport \
    pythonloader \
    postgresql-sdbc \
    postgresql-sdbc-impl \
    simplecanvas \
    slideshow \
    stringresource \
    syssh \
    tdebe1 \
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
    $(if $(filter $(OS),MACOSX), \
        fps_aqua \
    ) \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Helper_register_libraries,UNOLIBS_OOO, \
    oleautobridge \
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
	SunPresentationMinimizer \
    mysqlc \
    passive_native \
))

$(eval $(call gb_Helper_register_static_libraries,PLAINLIBS, \
    $(if $(filter $(OS),ANDROID), \
	    basebmp \
	) \
    basegfx_s \
    codemaker \
    codemaker_cpp \
    codemaker_java \
    cosv \
    dtobj \
	findsofficepath \
	gnu_getopt \
	gnu_readdir_r \
    headless \
    libeay32 \
    npsoenv \
    nputils \
    pdfimport_s \
    plugcon \
	quickstarter \
	seterror \
    ssleay32 \
    ooopathutils \
    sample \
    sax_shared \
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
))

ifeq ($(OS),WNT)
$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
    xmlsec1 \
    xmlsec1-nss \
))
ifneq ($(CROSS_COMPILING),YES)
$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
    xmlsec1-mscrypto \
))
endif
else
$(eval $(call gb_Helper_register_static_libraries,PLAINLIBS, \
    xmlsec1 \
    xmlsec1-nss \
))
endif

$(eval $(call gb_Helper_register_jars,URE, \
	java_uno \
	juh \
	jurt \
	ridl \
	testComponent \
	unoil \
	unoloader \
))

$(eval $(call gb_Helper_register_jars,OOO, \
	ConnectivityTools \
	FCFGMerge \
	HelloWorld \
	Highlight \
	MemoryUsage \
	OOoRunner \
	ScriptFramework \
	ScriptProviderForBeanShell \
	ScriptProviderForJava \
	ScriptProviderForJavaScript \
	TestExtension \
	OOoRunnerLight \
	XMergeBridge \
	XSLTFilter \
	XSLTValidate \
	commonwizards \
	agenda \
	aportisdoc \
	fax \
	form \
	htmlsoff \
	java_uno_accessbridge \
	letter \
	officebean \
	pexcel \
	pocketword \
	query \
	report \
	reportbuilderwizard \
	sdbc_hsqldb \
	table \
	test \
	test-tools \
	web \
	xmerge \
))

$(eval $(call gb_Helper_register_jars,OXT, \
	EvolutionarySolver \
	mediawiki \
	nlpsolver \
	passive_java \
	report-builder \
))

# External executables
$(eval $(call gb_ExternalExecutable_register_executables,\
	genbrk \
	genccode \
	gencmn \
	python \
	ucpp \
	xmllint \
	xsltproc \
))

# vim: set noet sw=4 ts=4:
