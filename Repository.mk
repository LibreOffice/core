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
	cppunittester \
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
	transex3 \
	treex \
	uiex \
	ulfex \
	unoidl-check \
	unoidl-read \
	unoidl-write \
	xrmex \
	$(if $(filter-out ANDROID IOS WNT,$(OS)), \
        svdemo \
        svptest \
        svpclient \
        pixelctl ) \
	$(if $(and $(ENABLE_GTK), $(filter LINUX,$(OS))), tilebench) \
	$(if $(filter LINUX MACOSX WNT,$(OS)),icontest \
	    outdevgrind) \
	vcldemo \
	tiledrendering \
    mtfdemo \
	$(if $(and $(ENABLE_GTK), $(filter LINUX,$(OS))), gtktiledviewer) \
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

$(eval $(call gb_Helper_register_executables_for_install,OOO,brand, \
	$(if $(filter-out ANDROID IOS MACOSX WNT,$(OS)),oosplash) \
	soffice_bin \
	$(if $(filter DESKTOP,$(BUILD_TYPE)),unopkg_bin) \
	$(if $(filter WNT,$(OS)), \
		soffice \
		unoinfo \
		unopkg \
		unopkg_com \
	) \
))

$(eval $(call gb_Helper_register_executables_for_install,OOO,base_brand, \
	$(if $(filter WNT,$(OS)), \
		sbase \
	) \
))

$(eval $(call gb_Helper_register_executables_for_install,OOO,base, \
	$(if $(filter WNT,$(OS)), \
		odbcconfig \
	) \
))

$(eval $(call gb_Helper_register_executables_for_install,OOO,calc_brand, \
	$(if $(filter WNT,$(OS)), \
		scalc \
	) \
))

$(eval $(call gb_Helper_register_executables_for_install,OOO,draw_brand, \
	$(if $(filter WNT,$(OS)), \
		sdraw \
	) \
))

$(eval $(call gb_Helper_register_executables_for_install,OOO,impress_brand, \
	$(if $(filter WNT,$(OS)), \
		simpress \
	) \
))

$(eval $(call gb_Helper_register_executables_for_install,OOO,math_brand, \
	$(if $(filter WNT,$(OS)), \
		smath \
	) \
))

$(eval $(call gb_Helper_register_executables_for_install,OOO,writer_brand, \
	$(if $(filter WNT,$(OS)), \
		sweb \
		swriter \
	) \
))

$(eval $(call gb_Helper_register_executables_for_install,OOO,ooo, \
	gengal \
	$(if $(filter unx-TRUE,$(GUIBASE)-$(ENABLE_NPAPI_FROM_BROWSER)),pluginapp.bin) \
	$(if $(filter unx-TRUE,$(GUIBASE)-$(ENABLE_TDE)),tdefilepicker) \
	$(if $(filter WNT,$(OS)),,uri-encode) \
	ui-previewer \
	$(if $(filter WNT,$(OS)), \
		senddoc \
	) \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Helper_register_executables_for_install,OOO,quickstart, \
	quickstart \
))
endif

$(eval $(call gb_Helper_register_executables_for_install,OOO,python, \
	$(if $(filter WNT,$(OS)), \
		python \
	) \
))

ifeq ($(GUIBASE),unx)
$(eval $(call gb_Helper_register_executables_for_install,OOO,gnome, \
	$(if $(ENABLE_GTK),\
		xid-fullscreen-on-all-monitors \
	) \
))
endif

ifneq ($(ENABLE_PDFIMPORT),)
$(eval $(call gb_Helper_register_executables_for_install,OOO,pdfimport, \
	xpdfimport \
))
endif

$(eval $(call gb_Helper_register_executables_for_install,UREBIN,ure,\
	$(if $(and $(ENABLE_JAVA),$(filter-out MACOSX WNT,$(OS)),$(filter DESKTOP,$(BUILD_TYPE))),javaldx) \
	regmerge \
	regview \
	$(if $(filter DESKTOP,$(BUILD_TYPE)),uno) \
))

ifneq (,$(filter ANDROID IOS,$(OS)))

# these are in NONE layer because
# a) scp2 is not used on mobile b) layers don't mean anything on mobile
$(eval $(call gb_Helper_register_libraries,PLAINLIBS_NONE, \
	$(if $(filter $(OS),ANDROID), \
		lo-bootstrap \
	) \
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
	$(call gb_Helper_optional,DBCONNECTIVITY,calc) \
	date \
	pricing \
	sc \
	scd \
	scfilt \
	scui \
	wpftcalc \
	$(if $(ENABLE_COINMP)$(ENABLE_LPSOLVE),solver) \
	$(call gb_Helper_optional,SCRIPTING,vbaobj) \
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
	PresentationMinimizer \
	wpftimpress \
))

$(eval $(call gb_Helper_register_libraries_for_install,OOOLIBS,onlineupdate, \
	$(if $(ENABLE_ONLINE_UPDATE), \
		updatecheckui \
		updchk \
	) \
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
	$(if $(and $(filter unx,$(GUIBASE)),$(filter-out MACOSX,$(OS))), \
		$(if $(ENABLE_KDE),vclplug_kde) \
		$(if $(ENABLE_KDE4),vclplug_kde4) \
	) \
))

$(eval $(call gb_Helper_register_executables_for_install,OOO,kde, \
	$(if $(filter $(GUIBASE)$(ENABLE_KDE),unxTRUE), \
		kdefilepicker \
	) \
))

$(eval $(call gb_Helper_register_libraries_for_install,OOOLIBS,math, \
	sm \
	smd \
))

$(eval $(call gb_Helper_register_libraries_for_install,OOOLIBS,ogltrans, \
	OGLTrans \
))

$(eval $(call gb_Helper_register_libraries_for_install,OOOLIBS,ooo, \
	acc \
	$(call gb_Helper_optional,AVMEDIA,avmedia) \
	$(if $(filter MACOSX,$(OS)),\
		avmediaMacAVF \
		$(if $(ENABLE_MACOSX_SANDBOX),,avmediaQuickTime) \
	) \
	$(call gb_Helper_optional,SCRIPTING, \
		basctl \
		basprov \
	) \
	$(if $(filter $(OS),ANDROID),, \
		basebmp \
	) \
	basegfx \
	bib \
	$(if $(ENABLE_CAIRO_CANVAS),cairocanvas) \
	canvasfactory \
	canvastools \
	chartcore \
	chartcontroller \
	chartopengl \
	$(call gb_Helper_optional,OPENCL,clew) \
	$(if $(filter $(OS),WNT),,cmdmail) \
	cppcanvas \
	configmgr \
	ctl \
	cui \
	$(call gb_Helper_optional,DBCONNECTIVITY, \
		dba \
		dbase \
		dbmm \
		dbtools \
		dbaxml) \
	deploymentmisc \
	$(if $(filter-out MACOSX WNT,$(OS)),desktopbe1) \
	$(if $(filter unx,$(GUIBASE)),desktop_detector) \
	$(call gb_Helper_optional,SCRIPTING,dlgprov) \
	$(if $(ENABLE_DIRECTX),directx9canvas) \
	$(if $(ENABLE_OPENGL_CANVAS),oglcanvas) \
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
	$(call gb_Helper_optional,DBCONNECTIVITY, \
		flat \
		file) \
	filterconfig \
	$(if $(filter $(ENABLE_FIREBIRD_SDBC),TRUE),firebird_sdbc) \
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
	$(call gb_Helper_optional,SCRIPTING,msforms) \
	mtfrenderer \
	$(call gb_Helper_optional,DBCONNECTIVITY,mysql) \
	odbc \
	odfflatxml \
	offacc \
	oox \
	$(call gb_Helper_optional,OPENCL,opencl) \
	passwordcontainer \
	pcr \
	$(if $(ENABLE_NPAPI_FROM_BROWSER),pl) \
	pdffilter \
	$(call gb_Helper_optional,SCRIPTING,protocolhandler) \
	res \
	sax \
	sb \
	$(call gb_Helper_optional,DBCONNECTIVITY,sdbt) \
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
	storagefd \
	$(call gb_Helper_optional,SCRIPTING,stringresource) \
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
	$(if $(ENABLE_TELEPATHY),tubes) \
	ucpexpand1 \
	ucpext \
	ucpcmis1 \
	ucptdoc1 \
	unordf \
	unoxml \
	updatefeed \
	utl \
	uui \
	$(call gb_Helper_optional,SCRIPTING, \
		vbaevents \
		vbahelper \
	) \
	vcl \
	vclcanvas \
	$(if $(and $(filter unx,$(GUIBASE)),$(filter-out MACOSX,$(OS))), \
		vclplug_gen \
		$(if $(ENABLE_TDE),vclplug_tde) \
		$(if $(ENABLE_HEADLESS),,vclplug_svp) \
	) \
	writerperfect \
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
		$(if $(WITH_MOZAB4WIN), \
			mozab2 \
			mozabdrv \
		) \
		$(if $(WITH_MOZAB4WIN),,mozbootstrap) \
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

$(eval $(call gb_Helper_register_libraries_for_install,OOOLIBS,postgresqlsdbc, \
	$(if $(BUILD_POSTGRESQL_SDBC), \
		postgresql-sdbc \
		postgresql-sdbc-impl) \
))

$(eval $(call gb_Helper_register_libraries_for_install,OOOLIBS,pdfimport, \
	pdfimport \
))

$(eval $(call gb_Helper_register_libraries_for_install,OOOLIBS,python, \
	pythonloader \
))

$(eval $(call gb_Helper_register_libraries_for_install,PLAINLIBS_OOO,python, \
	pyuno \
	$(if $(filter-out WNT,$(OS)),pyuno_wrapper) \
))

$(eval $(call gb_Helper_register_libraries_for_install,OOOLIBS,reportbuilder, \
	rpt \
	rptui \
	rptxml \
))

$(eval $(call gb_Helper_register_libraries_for_install,OOOLIBS,writer, \
	hwp \
	$(if $(ENABLE_LWP),lwpft) \
	msword \
	swd \
	swui \
	t602filter \
	$(call gb_Helper_optional,SCRIPTING,vbaswobj) \
	wpftwriter \
	writerfilter \
))

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_NONE, \
	getuid \
	smoketest \
	subsequenttest \
	test \
	testtools_cppobj \
	testtools_bridgetest \
	testtools_constructors \
	unobootstrapprotector \
	unoexceptionprotector \
	unotest \
	vclbootstrapprotector \
	scqahelper \
	unowinreg \
))

$(eval $(call gb_Helper_register_libraries_for_install,PLAINLIBS_URE,ure, \
	affine_uno_uno \
	$(if $(filter MSC,$(COM)),cli_uno) \
	$(if $(ENABLE_JAVA), \
		java_uno \
		jpipe \
		$(if $(filter $(OS),WNT),jpipx) \
	    juh \
		juhx \
	) \
	log_uno_uno \
	unsafe_uno_uno \
	$(if $(URELIBS),urelibs) \
	$(if $(filter MSC,$(COM)),$(if $(filter INTEL,$(CPUNAME)),msci,mscx),gcc3)_uno \
	$(if $(filter $(OS),WNT), \
		uwinapi \
	) \
))

$(eval $(call gb_Helper_register_libraries_for_install,PRIVATELIBS_URE,ure, \
	binaryurp \
	bootstrap \
	introspection \
	invocadapt \
	invocation \
	io \
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
	unoidl \
	uuresolver \
	xmlreader \
))

# this is NONE even though it is actually in URE because it is CliNativeLibrary
$(eval $(call gb_Helper_register_libraries,PLAINLIBS_NONE, \
	$(if $(filter MSC,$(COM)),cli_cppuhelper) \
))

$(eval $(call gb_Helper_register_libraries_for_install,PLAINLIBS_OOO,ooo, \
	$(if $(ENABLE_VLC),avmediavlc) \
	$(if $(ENABLE_GSTREAMER_1_0),avmediagst) \
	$(if $(ENABLE_GSTREAMER_0_10),avmediagst_0_10) \
	$(if $(ENABLE_DIRECTX),avmediawin) \
	$(if $(ENABLE_GLTF),avmediaogl) \
	cached1 \
	collator_data \
	comphelper \
	$(call gb_Helper_optional,DBCONNECTIVITY,dbpool2) \
	deployment \
	deploymentgui \
	dict_ja \
	dict_zh \
	embobj \
	$(if $(ENABLE_JAVA),hsqldb) \
	i18nlangtag \
	i18nutil \
	index_data \
	$(if $(and $(ENABLE_GTK), $(filter LINUX,$(OS))), libreofficekitgtk) \
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
	$(if $(and $(filter unx,$(GUIBASE)),$(filter-out MACOSX,$(OS))),recentfile) \
	$(call gb_Helper_optional,SCRIPTING,scriptframe) \
	sdbc2 \
	sofficeapp \
	srtrs1 \
	$(if $(filter $(OS),WNT),sysdtrans) \
	textconv_dict \
	ucb1 \
	ucbhelper \
	$(if $(WITH_WEBDAV),ucpdav1) \
	ucpfile1 \
	ucpftp1 \
	ucpchelp1 \
	ucphier1 \
	ucppkg1 \
	unopkgapp \
	xmlsecurity \
	xsec_fw \
	xsec_xmlsec \
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
		$(if $(DISABLE_ATL),,\
			inprocserv \
			UAccCOM \
			winaccessibility \
		) \
	) \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Helper_register_libraries_for_install,PLAINLIBS_OOO,activexbinarytable, \
	$(if $(DISABLE_ACTIVEX),,\
		regactivex \
	) \
))

$(eval $(call gb_Helper_register_libraries_for_install,PLAINLIBS_OOO,activex, \
	$(if $(DISABLE_ACTIVEX),,\
		so_activex \
	) \
))

ifneq ($(BUILD_X64),)
$(eval $(call gb_Helper_register_libraries_for_install,PLAINLIBS_OOO,activexwin64, \
	$(if $(DISABLE_ACTIVEX),,\
		so_activex_x64 \
	) \
))
endif

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

$(eval $(call gb_Helper_register_libraries_for_install,PLAINLIBS_SHLXTHDL,winexplorerext, \
	ooofilt \
	shlxthdl \
))

$(eval $(call gb_Helper_register_libraries_for_install,PLAINLIBS_SHLXTHDL,winexplorerextnt6, \
	propertyhdl \
))

$(eval $(call gb_Helper_register_libraries_for_install,PLAINLIBS_SHLXTHDL,winexplorerextwin64, \
	ooofilt_x64 \
	shlxthdl_x64 \
))

$(eval $(call gb_Helper_register_libraries_for_install,PLAINLIBS_SHLXTHDL,winexplorerextwin64nt6, \
	propertyhdl_x64 \
))

endif # WNT

$(eval $(call gb_Helper_register_libraries_for_install,RTVERLIBS,ure, \
	cppuhelper \
	purpenvhelper \
	salhelper \
))

$(eval $(call gb_Helper_register_libraries_for_install,UNOVERLIBS,ure, \
	cppu \
	sal \
))

$(eval $(call gb_Helper_register_libraries,EXTENSIONLIBS, \
	active_native \
	mysqlc \
	passive_native \
))

ifneq ($(ENABLE_JAVA),)
$(eval $(call gb_Helper_register_jars_for_install,URE,ure, \
	java_uno \
	juh \
	jurt \
	ridl \
	unoloader \
))

$(eval $(call gb_Helper_register_jars_for_install,OOO,ooo, \
	ScriptFramework \
	ScriptProviderForJava \
	XMergeBridge \
	commonwizards \
	form \
	$(if $(filter-out MACOSX,$(OS)),officebean) \
	query \
	report \
	sdbc_hsqldb \
	smoketest \
	table \
	unoil \
	xmerge \
))

$(eval $(call gb_Helper_register_jars_for_install,OOO,reportbuilder, \
	reportbuilder \
	reportbuilderwizard \
))

ifneq ($(ENABLE_SCRIPTING_BEANSHELL),)
$(eval $(call gb_Helper_register_jars_for_install,OOO,extensions_bsh, \
	ScriptProviderForBeanShell \
))
endif

ifneq ($(ENABLE_SCRIPTING_JAVASCRIPT),)
$(eval $(call gb_Helper_register_jars_for_install,OOO,extensions_rhino, \
	ScriptProviderForJavaScript \
))
endif

ifeq (CT2N,$(filter CT2N,$(BUILD_TYPE)))
$(eval $(call gb_Helper_register_packages_for_install,extensions_ct2n,\
	ct2n \
))
endif

ifeq (NUMBERTEXT,$(filter NUMBERTEXT,$(BUILD_TYPE)))
$(eval $(call gb_Helper_register_packages_for_install,extensions_numbertext,\
	numbertext \
))
endif

$(eval $(call gb_Helper_register_jars,OXT, \
	EvolutionarySolver \
	active_java \
	mediawiki \
	nlpsolver \
	passive_java \
))

$(eval $(call gb_Helper_register_jars,NONE,\
	ConnectivityTools \
	HelloWorld \
	Highlight \
	MemoryUsage \
	OOoRunner \
	TestExtension \
	test \
	test-tools \
	testComponent \
))
endif

$(eval $(call gb_Helper_register_packages_for_install,ure,\
	instsetoo_native_setup_ure \
	ure_install \
	$(if $(ENABLE_JAVA),\
		jvmfwk_jvmfwk3_ini \
		jvmfwk_javavendors \
		jvmfwk_jreproperties \
		$(if $(filter MACOSX,$(OS)),bridges_jnilib_java_uno) \
	) \
))

$(eval $(call gb_Helper_register_packages_for_install,ooo,\
	xmlsec \
	chart2_opengl_shader \
	vcl_opengl_shader \
	$(if $(ENABLE_OPENGL_CANVAS),canvas_opengl_shader) \
))

$(eval $(call gb_Helper_register_packages_for_install,ogltrans,\
	slideshow_opengl_shader \
))

ifeq ($(GUIBASE),unx)
$(eval $(call gb_Helper_register_packages_for_install,base_brand,\
	desktop_sbase_sh \
))

$(eval $(call gb_Helper_register_packages_for_install,calc_brand,\
	desktop_scalc_sh \
))

$(eval $(call gb_Helper_register_packages_for_install,draw_brand,\
	desktop_sdraw_sh \
))

$(eval $(call gb_Helper_register_packages_for_install,impress_brand,\
	desktop_simpress_sh \
))

$(eval $(call gb_Helper_register_packages_for_install,math_brand,\
	desktop_smath_sh \
))

$(eval $(call gb_Helper_register_packages_for_install,writer_brand,\
	desktop_swriter_sh \
))
endif # GUIBASE=unx

ifneq ($(DISABLE_PYTHON),TRUE)
$(eval $(call gb_Helper_register_packages_for_install,python, \
    pyuno_pythonloader_ini \
))
endif

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
