# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

$(eval $(call gb_Helper_register_executables,NONE, \
	HelpIndexer \
	HelpLinker \
	bestreversemap \
	cfgex \
	concat-deps \
	cpp \
	cppunit/cppunittester \
	$(call gb_Helper_optional,CRASHREP,crashrep) \
	gencoll_rule \
	genconv_dict \
	gendict \
	genindex_data \
	helpex \
	idxdict \
	langsupport \
	$(if $(ENABLE_TELEPATHY),liboapprover) \
	$(if $(filter IOS,$(OS)),LibreOffice) \
	libtest \
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
	saxparser \
	sp2bv \
	svg2odf \
	svidl \
	stringex \
	transex3 \
	treex \
	$(if $(filter UCPP,$(BUILD_TYPE)),ucpp) \
	uiex \
	ulfex \
	$(call gb_Helper_optional,ODK,unoapploader) \
	xpdfimport \
	xrmex \
))

$(eval $(call gb_Helper_register_executables,SDK, \
	idlc \
	$(if $(filter-out IOS,$(OS)), \
		climaker \
		cppumaker \
		javamaker \
		regcompare \
		uno-skeletonmaker \
	) \
))

$(eval $(call gb_Helper_register_executables,OOO, \
	$(if $(filter DESKTOP,$(BUILD_TYPE)),gengal.bin) \
	gnome-open-url.bin \
	$(if $(filter YES,$(ENABLE_NPAPI_INTO_BROWSER)),nsplugin) \
	$(if $(filter MACOSX,$(OS)),officeloader) \
	$(if $(filter-out ANDROID IOS MACOSX WNT,$(OS)),oosplash) \
	$(if $(filter YES,$(ENABLE_NPAPI_FROM_BROWSER)),pluginapp.bin) \
	$(if $(filter-out WNT,$(OS)),soffice.bin) \
	spadmin.bin \
	$(if $(filter $(GUIBASE)$(ENABLE_TDE),unxTRUE), \
		tdefilepicker \
	) \
	uri-encode \
	$(if $(filter $(GUIBASE)$(ENABLE_KDE),unxTRUE), \
		kdefilepicker \
	) \
	ui-previewer \
	$(if $(filter DESKTOP,$(BUILD_TYPE)),unopkg.bin) \
	$(if $(filter WNT,$(OS)), \
		crashrep_com \
		gengal \
		gcc-wrapper \
		g++-wrapper \
		guiloader \
		guistdio \
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
	) \
))

$(eval $(call gb_Helper_register_executables_for_install,UREBIN,ure,\
	$(if $(filter-out IOS,$(OS)), \
		regmerge \
		regview \
	) \
	$(if $(and $(SOLAR_JAVA),$(filter-out MACOSX WNT,$(OS)),$(filter DESKTOP,$(BUILD_TYPE))),javaldx) \
))
$(eval $(call gb_Helper_register_executables,UREBIN,\
	$(if $(filter DESKTOP,$(BUILD_TYPE)),uno) \
))

ifeq ($(ENABLE_NPAPI_FROM_BROWSER),YES)
$(eval $(call gb_Helper_register_libraries,OOOLIBS, \
	pl \
))
endif

ifeq ($(ENABLE_NPAPI_INTO_BROWSER),YES)
$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
	npsoplugin \
))
endif

ifeq ($(OS),MACOSX)

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_NONE, \
	OOoSpotlightImporter \
))

endif

$(eval $(call gb_Helper_register_libraries_for_install,OOOLIBS,base, \
	abp \
	dbp \
	dbu \
))

$(eval $(call gb_Helper_register_libraries_for_install,OOOLIBS,calc, \
	analysis \
	calc \
	date \
	pricing \
	sc \
	scd \
	scfilt \
	scui \
	solver \
	$(if $(DISABLE_SCRIPTING),,vbaobj) \
))

$(eval $(call gb_Helper_register_libraries_for_install,OOOLIBS,graphicfilter, \
	svgfilter \
	flash \
	wpftdraw \
	graphicfilter \
))

$(eval $(call gb_Helper_register_libraries_for_install,OOOLIBS,tde, \
	$(if $(ENABLE_TDE),tdebe1) \
))

$(eval $(call gb_Helper_register_libraries_for_install,OOOLIBS,impress, \
	animcore \
	placeware \
	PresenterScreen \
))

$(eval $(call gb_Helper_register_libraries_for_install,OOOLIBS,onlineupdate, \
	updatecheckui \
	updchk \
))

$(eval $(call gb_Helper_register_libraries_for_install,OOOLIBS,gnome, \
	$(if $(ENABLE_EVOAB2),evoab) \
	$(if $(ENABLE_GTK),vclplug_gtk) \
	$(if $(ENABLE_GTK3),vclplug_gtk3) \
	$(if $(ENABLE_GCONF),gconfbe1) \
	$(if $(ENABLE_SYSTRAY_GTK),qstart_gtk) \
	$(if $(ENABLE_GIO),losessioninstall) \
	$(if $(ENABLE_GIO),ucpgio1) \
	$(if $(ENABLE_GNOMEVFS),ucpgvfs1) \
))

$(eval $(call gb_Helper_register_libraries_for_install,OOOLIBS,kde, \
	$(if $(ENABLE_KDE),kdebe1) \
	$(if $(ENABLE_KDE4),kde4be1) \
))

$(eval $(call gb_Helper_register_libraries_for_install,OOOLIBS,math, \
	sm \
	smd \
))

$(eval $(call gb_Helper_register_libraries_for_install,OOOLIBS,ooo, \
	avmedia \
	$(if $(DISABLE_SCRIPTING),,basctl) \
	basegfx \
	bib \
	$(if $(ENABLE_CAIRO_CANVAS),cairocanvas) \
	canvasfactory \
	canvastools \
	chartcore \
	chartcontroller \
	cppcanvas \
	configmgr \
	ctl \
	cui \
	dba \
	dbase \
	dbmm \
	$(if $(DISABLE_DBCONNECTIVITY),,dbtools) \
	dbaxml \
	deploymentmisc \
	$(if $(filter-out MACOSX WNT,$(OS)),desktopbe1) \
	$(if $(filter unx,$(GUIBASE)),desktop_detector) \
	$(if $(ENABLE_DIRECTX),directx9canvas) \
	drawinglayer \
	editeng \
	egi \
	eme \
	$(if $(filter WNT,$(OS)),$(if $(DISABLE_ATL),,emser)) \
	epb \
	epg \
	epp \
	eps \
	ept \
	era \
	eti \
	evtatt \
	exp \
	flat \
	file \
	filterconfig \
	for \
	forui \
	frm \
	fsstorage \
	fwe \
	fwi \
	fwk \
	fwl \
	fwm \
	$(if $(ENABLE_DIRECTX),gdipluscanvas) \
	guesslang \
	$(if $(filter DESKTOP,$(BUILD_TYPE)),helplinker) \
	i18npool \
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
	$(if $(SOLAR_JAVA),jdbc) \
	$(if $(ENABLE_KAB),kab1) \
	$(if $(ENABLE_KAB),kabdrv1) \
	localebe1 \
	lng \
	lnth \
	$(if $(MERGELIBS),merged) \
	msfilter \
	mtfrenderer \
	mysql \
	odbc \
	odbcbase \
	odfflatxml \
	offacc \
	pcr \
	pdffilter \
	res \
	sax \
	sb \
	sdbt \
	scn \
	sd \
	sdd \
	sdfilt \
	sdui \
	sfx \
	simplecanvas \
	sot \
	$(if $(ENABLE_HEADLESS),,spl) \
	svgio \
	svl \
	svt \
	svx \
	svxcore \
	sw \
	$(if $(ENABLE_TDEAB),tdeab1) \
	$(if $(ENABLE_TDEAB),tdeabdrv1) \
	textconversiondlgs \
	tk \
	tl \
	$(if $(ENABLE_TELEPATHY),tubes) \
	ucpexpand1 \
	unordf \
	unoxml \
	utl \
	uui \
	vcl \
	vclcanvas \
	$(if $(and $(filter unx,$(GUIBASE)),$(filter-out MACOSX,$(OS))),vclplug_gen) \
	xmlscript \
	xmlfa \
	xmlfd \
	xo \
	xof \
	xsltdlg \
	xsltfilter \
))

$(eval $(call gb_Helper_register_libraries_for_install,OOOLIBS,writer, \
	hwp \
	$(if $(ENABLE_LWP),lwpft) \
	msword \
	swd \
	swui \
	t602filter \
	wpftwriter \
	writerfilter \
))

$(eval $(call gb_Helper_register_libraries,OOOLIBS, \
	acc \
	adabas \
	adabasui \
	ado \
	agg \
	$(if $(filter $(OS),ANDROID),, \
		basebmp \
	) \
	communi \
	oox \
	ooxml \
	sdbc \
	AppleRemote \
	avmediaQuickTime \
	filtertracer \
	log \
	MacOSXSpell \
	mork \
	mozab2 \
	mozabdrv \
	protocolhandler \
	rpt \
	rptui \
	rptxml \
	simplecm \
	spa \
	spell \
	sts \
	textfd \
	$(if $(DISABLE_SCRIPTING),,vbahelper) \
	vclplug_tde \
	vclplug_kde \
	vclplug_kde4 \
	vclplug_svp \
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

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_NONE, \
	smoketest \
	subsequenttest \
	test \
	unobootstrapprotector \
	unoexceptionprotector \
	unotest \
))

$(eval $(call gb_Helper_register_libraries_for_install,PLAINLIBS_URE,ure, \
	affine_uno \
	$(if $(SOLAR_JAVA),java_uno) \
	$(if $(SOLAR_JAVA),juh) \
	$(if $(SOLAR_JAVA),juhx) \
	$(if $(SOLAR_JAVA),jvmaccess) \
	$(if $(SOLAR_JAVA),jvmfwk) \
	log_uno \
	reg \
	sal_textenc \
	store \
	$(if $(SOLAR_JAVA),sunjavaplugin) \
	unoidl \
	unsafe_uno \
	$(if $(URELIBS),urelibs) \
	xmlreader \
	$(if $(filter MSC,$(COM)),$(if $(filter INTEL,$(CPUNAME)),msci,mscx),gcc3)_uno \
))

$(eval $(call gb_Helper_register_libraries_for_install,UNOLIBS_URE,ure, \
	acceptor \
	binaryurp \
	bootstrap \
	connector \
	introspection \
	invocadapt \
	invocation \
	$(if $(SOLAR_JAVA),javaloader) \
	$(if $(SOLAR_JAVA),javavm) \
	namingservice \
	proxyfac \
	reflection \
	stocservices \
	streams \
	textinstream \
	textoutstream \
	uuresolver \
))

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_URE, \
	$(if $(filter MSC,$(COM)),cli_cppuhelper) \
	$(if $(filter MSC,$(COM)),cli_uno) \
	getuid \
	jpipe \
	$(if $(filter WNT,$(OS)),jpipx) \
))

$(eval $(call gb_Helper_register_libraries_for_install,PLAINLIBS_OOO,ooo, \
	comphelper \
	deployment \
	fileacc \
	i18nlangtag \
	i18nutil \
	mcnttype \
	package2 \
	sofficeapp \
	ucb1 \
	ucbhelper \
	ucpfile1 \
	xstor \
))
$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
	avmediagst \
	avmediagst_0_10 \
	avmediawin \
	bluez_bluetooth \
	cached1 \
	collator_data \
	dbpool2 \
	deploymentgui \
	dict_ja \
	dict_zh \
	embobj \
	emboleobj \
	hsqldb \
	index_data \
	java_uno_accessbridge \
	libreoffice \
	localedata_en \
	localedata_es \
	localedata_euro \
	localedata_others \
	macab1 \
	macabdrv1 \
	mozbootstrap \
	officebean \
	pyuno \
	pyuno_wrapper \
	recentfile \
	scriptframe \
	sdbc2 \
	srtrs1 \
	textconv_dict \
	tvhlp1 \
	ucpdav1 \
	ucpftp1 \
	ucpchelp1 \
	ucphier1 \
	ucppkg1 \
	unopkgapp \
	xmlsecurity \
	xsec_fw \
	xsec_xmlsec \
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

$(eval $(call gb_Helper_register_libraries,RTVERLIBS, \
	cppuhelper \
	purpenvhelper \
	salhelper \
))

$(eval $(call gb_Helper_register_libraries,OOOLIBS, \
	basprov \
	cmdmail \
	dlgprov \
	expwrap \
	fastsax \
	fpicker \
	fps_office \
	hatchwindowfactory \
	i18nsearch \
	ldapbe2 \
	macbe1 \
	migrationoo2 \
	migrationoo3 \
	msforms \
	OGLTrans \
	passwordcontainer \
	pdfimport \
	postgresql-sdbc \
	postgresql-sdbc-impl \
	pythonloader \
	slideshow \
	stringresource \
	syssh \
	ucpcmis1 \
	ucpext \
	ucptdoc1 \
	updatefeed \
	vbaevents \
	vbaswobj \
	$(if $(filter $(OS),MACOSX), \
		fps_aqua \
	) \
	$(if $(filter $(OS),WNT), \
		oleautobridge \
		smplmail \
		wininetbe1 \
	) \
))

$(eval $(call gb_Helper_register_libraries,UNOLIBS_URE, \
	testtools_cppobj \
	testtools_bridgetest \
	testtools_constructors \
))

$(eval $(call gb_Helper_register_libraries,UNOVERLIBS, \
	cppu \
	sal \
))

$(eval $(call gb_Helper_register_libraries,EXTENSIONLIBS, \
	SunPresentationMinimizer \
	mysqlc \
	passive_native \
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
endif

$(eval $(call gb_Helper_register_jars,URE, \
	java_uno \
	juh \
	jurt \
	ridl \
	testComponent \
	unoloader \
))

$(eval $(call gb_Helper_register_jars,OOO, \
	ConnectivityTools \
	ScriptFramework \
	ScriptProviderForBeanShell \
	ScriptProviderForJava \
	ScriptProviderForJavaScript \
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
	reportbuilder \
	reportbuilderwizard \
	sdbc_hsqldb \
	smoketest \
	table \
	unoil \
	web \
	xmerge \
))

$(eval $(call gb_Helper_register_jars,OXT, \
	EvolutionarySolver \
	mediawiki \
	nlpsolver \
	passive_java \
))

$(eval $(call gb_Helper_register_jars,NONE,\
	HelloWorld \
	Highlight \
	MemoryUsage \
	OOoRunner \
	OOoRunnerLight \
	TestExtension \
	test \
	test-tools \
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
