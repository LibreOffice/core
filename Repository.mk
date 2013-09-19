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
	$(if $(filter MSC,$(COM)), \
		gcc-wrapper \
		g++-wrapper \
	) \
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
	pocheck \
	propex \
	regsvrex \
	rsc \
	saxparser \
	sp2bv \
	svg2odf \
	svidl \
	stringex \
	transex3 \
	treex \
	uiex \
	ulfex \
	unoidl-check \
	unoidl-read \
	unoidl-write \
	xrmex \
))

$(eval $(call gb_Helper_register_executables_for_install,SDK,sdk, \
	$(if $(filter MSC,$(COM)),climaker) \
	cppumaker \
	idlc \
	javamaker \
	regcompare \
	$(if $(filter UCPP,$(BUILD_TYPE)),ucpp) \
	$(if $(filter ODK,$(BUILD_TYPE)),unoapploader) \
	$(if $(filter ODK,$(BUILD_TYPE)),uno-skeletonmaker) \
))

$(eval $(call gb_Helper_register_executables_for_install,OOO,ooo, \
	gengal \
))

$(eval $(call gb_Helper_register_executables,OOO, \
	$(call gb_Helper_optional,CRASHREP,crashrep) \
	gnome-open-url.bin \
	$(if $(ENABLE_NPAPI_INTO_BROWSER),nsplugin) \
	$(if $(filter-out ANDROID IOS MACOSX WNT,$(OS)),oosplash) \
	$(if $(ENABLE_NPAPI_FROM_BROWSER),pluginapp.bin) \
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
	xpdfimport \
	$(if $(filter WNT,$(OS)), \
		crashrep_com \
		odbcconfig \
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
	) \
))

$(eval $(call gb_Helper_register_executables_for_install,UREBIN,ure,\
	$(if $(and $(ENABLE_JAVA),$(filter-out MACOSX WNT,$(OS)),$(filter DESKTOP,$(BUILD_TYPE))),javaldx) \
	regmerge \
	regview \
	$(if $(filter DESKTOP,$(BUILD_TYPE)),uno) \
))

ifeq ($(ENABLE_NPAPI_INTO_BROWSER),TRUE)
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
	$(if $(ENABLE_OPENCL),scopencl) \
	$(if $(DISABLE_SCRIPTING),,vbaobj) \
	$(if $(ENABLE_TELEPATHY),tubes) \
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
	acc \
	avmedia \
	$(if $(DISABLE_SCRIPTING),,basctl) \
	$(if $(DISABLE_SCRIPTING),,basprov) \
	basegfx \
	bib \
	$(if $(ENABLE_CAIRO_CANVAS),cairocanvas) \
	canvasfactory \
	canvastools \
	chartcore \
	chartcontroller \
	$(if $(filter $(OS),WNT),,cmdmail) \
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
	$(if $(DISABLE_SCRIPTING),,dlgprov) \
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
	expwrap \
	fastsax \
	flat \
	file \
	filterconfig \
	$(if $(filter $(ENABLE_FIREBIRD_SDBC),TRUE),firebird_sdbc) \
	fpicker \
	fps_office \
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
	i18nsearch \
	hatchwindowfactory \
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
	$(if $(ENABLE_JAVA),jdbc) \
	$(if $(ENABLE_KAB),kab1) \
	$(if $(ENABLE_KAB),kabdrv1) \
	ldapbe2 \
	localebe1 \
	log \
	lng \
	lnth \
	$(if $(filter $(OS),MACOSX),macbe1) \
	$(if $(MERGELIBS),merged) \
	migrationoo2 \
	migrationoo3 \
	msfilter \
	$(if $(DISABLE_SCRIPTING),,msforms) \
	mtfrenderer \
	mysql \
	odbc \
	odbcbase \
	odfflatxml \
	offacc \
	passwordcontainer \
	pcr \
	$(if $(ENABLE_NPAPI_FROM_BROWSER),pl) \
	pdffilter \
	$(if $(DISABLE_SCRIPTING),,protocolhandler) \
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
	slideshow \
	sot \
	spell \
	$(if $(ENABLE_HEADLESS),,spl) \
	$(if $(DISABLE_SCRIPTING),,stringresource) \
	svgio \
	svl \
	svt \
	svx \
	svxcore \
	sw \
	syssh \
	$(if $(ENABLE_TDEAB),tdeab1) \
	$(if $(ENABLE_TDEAB),tdeabdrv1) \
	textconversiondlgs \
	textfd \
	tk \
	tl \
	ucpexpand1 \
	ucpext \
	ucpcmis1 \
	ucptdoc1 \
	unordf \
	unoxml \
	updatefeed \
	utl \
	uui \
	$(if $(DISABLE_SCRIPTING),,vbaevents) \
	$(if $(DISABLE_SCRIPTING),,vbahelper) \
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
	$(if $(filter $(OS),WNT), \
		ado \
		$(if $(DISABLE_ATL),,oleautobridge) \
		smplmail \
		wininetbe1 \
		$(if $(filter YES,$(WITH_MOZAB4WIN)), \
			mozab2 \
			mozabdrv \
		) \
		$(if $(filter NO,$(WITH_MOZAB4WIN)),mozbootstrap) \
	) \
	$(if $(filter $(OS),WNT),, \
		mork \
		mozbootstrap \
	) \
	$(if $(filter $(OS),MACOSX), \
		$(if $(ENABLE_MACOSX_SANDBOX),, \
			AppleRemote \
		) \
		fps_aqua \
		MacOSXSpell \
	) \
))

$(eval $(call gb_Helper_register_libraries_for_install,OOOLIBS,writer, \
	hwp \
	$(if $(ENABLE_LWP),lwpft) \
	msword \
	swd \
	swui \
	t602filter \
	$(if $(DISABLE_SCRIPTING),,vbaswobj) \
	wpftwriter \
	writerfilter \
))

$(eval $(call gb_Helper_register_libraries,OOOLIBS, \
	adabas \
	adabasui \
	agg \
	$(if $(filter $(OS),ANDROID),, \
		basebmp \
	) \
	communi \
	oox \
	ooxml \
	sdbc \
	avmediaQuickTime \
	filtertracer \
	rpt \
	rptui \
	rptxml \
	simplecm \
	spa \
	sts \
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

# these libraries are packaged in the "Test" instset, in the "program" dir :-/
$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
	smoketest \
	test \
	unobootstrapprotector \
	unoexceptionprotector \
	unotest \
))

# this is packaged in the "Test" instset, in the "program" dir :-/
$(eval $(call gb_Helper_register_executables,OOO, \
	cppunittester \
))

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_NONE, \
	subsequenttest \
	testtools_cppobj \
	testtools_bridgetest \
	testtools_constructors \
	scqahelper \
	unowinreg \
))

$(eval $(call gb_Helper_register_libraries_for_install,PLAINLIBS_URE,ure, \
	affine_uno_uno \
	$(if $(ENABLE_JAVA),java_uno) \
	$(if $(ENABLE_JAVA),juh) \
	$(if $(ENABLE_JAVA),juhx) \
	log_uno_uno \
	unsafe_uno_uno \
	$(if $(URELIBS),urelibs) \
	$(if $(filter MSC,$(COM)),$(if $(filter INTEL,$(CPUNAME)),msci,mscx),gcc3)_uno \
	$(if $(filter $(OS),WNT), \
		uwinapi \
	) \
))

$(eval $(call gb_Helper_register_libraries_for_install,PRIVATELIBS_URE,ure, \
	acceptor \
	binaryurp \
	bootstrap \
	connector \
	introspection \
	invocadapt \
	invocation \
	$(if $(ENABLE_JAVA),javaloader) \
	$(if $(ENABLE_JAVA),javavm) \
	$(if $(ENABLE_JAVA),jvmaccess) \
	$(if $(ENABLE_JAVA),jvmfwk) \
	namingservice \
	proxyfac \
	reflection \
	reg \
	sal_textenc \
	stocservices \
	store \
	streams \
	$(if $(ENABLE_JAVA),sunjavaplugin) \
	textinstream \
	textoutstream \
	unoidl \
	uuresolver \
	xmlreader \
))

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_URE, \
	$(if $(filter MSC,$(COM)),cli_cppuhelper) \
	$(if $(filter MSC,$(COM)),cli_uno) \
	getuid \
	jpipe \
	$(if $(filter WNT,$(OS)),jpipx) \
))

$(eval $(call gb_Helper_register_libraries_for_install,PLAINLIBS_OOO,ooo, \
	$(if $(ENABLE_VLC),avmediavlc) \
	$(if $(ENABLE_GSTREAMER),avmediagst) \
	$(if $(ENABLE_GSTREAMER_0_10),avmediagst_0_10) \
	$(if $(ENABLE_DIRECTX),avmediawin) \
	cached1 \
	collator_data \
	comphelper \
	dbpool2 \
	deployment \
	deploymentgui \
	dict_ja \
	dict_zh \
	embobj \
	fileacc \
	$(if $(ENABLE_JAVA),hsqldb) \
	i18nlangtag \
	i18nutil \
	index_data \
	localedata_en \
	localedata_es \
	localedata_euro \
	localedata_others \
	mcnttype \
	$(if $(ENABLE_JAVA), \
		$(if $(filter $(OS),MACOSX),,officebean) \
	) \
	$(if $(filter WNT-TRUE,$(OS)-$(DISABLE_ATL)),,emboleobj) \
	package2 \
	$(if $(DISABLE_SCRIPTING),,scriptframe) \
	sdbc2 \
	sofficeapp \
	srtrs1 \
	textconv_dict \
	tvhlp1 \
	ucb1 \
	ucbhelper \
	ucpfile1 \
	ucpftp1 \
	ucpchelp1 \
	ucphier1 \
	ucppkg1 \
	unopkgapp \
	xmlsecurity \
	xsec_fw \
	xstor \
	$(if $(filter $(OS),MACOSX), \
		macab1 \
		macabdrv1 \
	) \
	$(if $(filter WNT,$(OS)), \
		dnd \
		dtrans \
		fps \
		ftransl \
		$(if $(ENABLE_JAVA),java_uno_accessbridge) \
		$(if $(DISABLE_ATL),,\
			inprocserv \
		) \
	) \
))
$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
	bluez_bluetooth \
	libreoffice \
	pyuno \
	pyuno_wrapper \
	recentfile \
	ucpdav1 \
	xsec_xmlsec \
	$(if $(filter $(OS),ANDROID), \
		lo-bootstrap \
	) \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Helper_register_libraries_for_install,PLAINLIBS_OOO,activexbinarytable, \
	$(if $(DISABLE_ACTIVEX),,\
		regactivex \
	) \
))

$(eval $(call gb_Helper_register_libraries_for_install,PLAINLIBS_OOO,ooobinarytable, \
	$(if $(WINDOWS_SDK_HOME),\
		instooofiltmsi \
		qslnkmsi \
		reg4allmsdoc \
		sdqsmsi \
		sellangmsi \
		sn_tools \
	) \
))


$(eval $(call gb_Helper_register_libraries_for_install,PLAINLIBS_OOO,winexplorerextbinarytable, \
	shlxtmsi \
))

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_SHLXTHDL, \
	ooofilt \
	ooofilt_x64 \
	propertyhdl \
	propertyhdl_x64 \
	shlxthdl \
	shlxthdl_x64 \
))

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
	fop \
	jfregca \
	regpatchactivex \
	so_activex \
	so_activex_x64 \
	sysdtrans \
	thidxmsi \
))
endif

$(eval $(call gb_Helper_register_libraries,RTVERLIBS, \
	cppuhelper \
	purpenvhelper \
	salhelper \
))

$(eval $(call gb_Helper_register_libraries,OOOLIBS, \
	OGLTrans \
	pdfimport \
	postgresql-sdbc \
	postgresql-sdbc-impl \
	pythonloader \
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
	xmllint \
	xsltproc \
))

# vim: set noet sw=4 ts=4:
