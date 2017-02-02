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
	gbuildtojson \
	$(if $(filter MSC,$(COM)), \
		gcc-wrapper \
		g++-wrapper \
	) \
	gencoll_rule \
	genconv_dict \
	gendict \
	genindex_data \
	genlang \
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
	pdfverify \
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
	unoidl-read \
	unoidl-write \
	xrmex \
	$(if $(filter-out ANDROID IOS WNT,$(OS)), \
        svdemo \
        fftester \
        svptest \
        svpclient \
        pixelctl ) \
	$(if $(and $(ENABLE_GTK3), $(filter LINUX %BSD SOLARIS,$(OS))), tilebench) \
	$(if $(filter LINUX MACOSX SOLARIS WNT %BSD,$(OS)),icontest \
	    outdevgrind) \
	vcldemo \
	tiledrendering \
    mtfdemo \
    visualbackendtest \
	$(if $(and $(ENABLE_GTK3), $(filter LINUX %BSD SOLARIS,$(OS))), gtktiledviewer) \
))

$(eval $(call gb_Helper_register_executables_for_install,SDK,sdk, \
	$(if $(filter MSC,$(COM)),climaker) \
	cppumaker \
	idlc \
	javamaker \
	$(if $(filter UCPP,$(BUILD_TYPE)),ucpp) \
	$(if $(filter ODK,$(BUILD_TYPE)),unoapploader) \
	unoidl-check \
	$(if $(filter ODK,$(BUILD_TYPE)),uno-skeletonmaker) \
))

$(eval $(call gb_Helper_register_executables_for_install,OOO,brand, \
	$(call gb_Helper_optional,BREAKPAD,minidump_upload) \
	$(call gb_Helper_optional,FUZZERS,wmffuzzer) \
	$(call gb_Helper_optional,FUZZERS,jpgfuzzer) \
	$(call gb_Helper_optional,FUZZERS,giffuzzer) \
	$(call gb_Helper_optional,FUZZERS,xbmfuzzer) \
	$(call gb_Helper_optional,FUZZERS,xpmfuzzer) \
	$(call gb_Helper_optional,FUZZERS,pngfuzzer) \
	$(call gb_Helper_optional,FUZZERS,bmpfuzzer) \
	$(call gb_Helper_optional,FUZZERS,svmfuzzer) \
	$(call gb_Helper_optional,FUZZERS,pcdfuzzer) \
	$(call gb_Helper_optional,FUZZERS,dxffuzzer) \
	$(call gb_Helper_optional,FUZZERS,metfuzzer) \
	$(call gb_Helper_optional,FUZZERS,ppmfuzzer) \
	$(call gb_Helper_optional,FUZZERS,psdfuzzer) \
	$(call gb_Helper_optional,FUZZERS,epsfuzzer) \
	$(call gb_Helper_optional,FUZZERS,pctfuzzer) \
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
	$(if $(filter WNT,$(OS)),,uri-encode) \
	$(if $(ENABLE_MACOSX_SANDBOX),, \
		ui-previewer \
	) \
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

ifeq ($(USING_X11), TRUE)
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

ifneq ($(ENABLE_ONLINE_UPDATE_MAR),)
$(eval $(call gb_Helper_register_executables_for_install,OOO,updater,\
		mar \
		updater \
))
endif

$(eval $(call gb_Helper_register_executables_for_install,UREBIN,ure,\
	$(if $(and $(ENABLE_JAVA),$(filter-out MACOSX WNT,$(OS)),$(filter DESKTOP,$(BUILD_TYPE))),javaldx) \
	$(if $(ENABLE_MACOSX_SANDBOX),, \
		regmerge \
		regview \
	) \
	$(if $(filter DESKTOP,$(BUILD_TYPE)),uno) \
))

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
	$(if $(USING_X11), \
		$(if $(ENABLE_TDE),vclplug_tde) \
	) \
))

$(eval $(call gb_Helper_register_executables_for_install,OOO,tde, \
       $(if $(ENABLE_TDE),tdefilepicker) \
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
	$(if $(ENABLE_SYSTRAY_GTK),qstart_gtk) \
	$(if $(ENABLE_GIO),losessioninstall) \
	$(if $(ENABLE_GIO),ucpgio1) \
))

$(eval $(call gb_Helper_register_libraries_for_install,OOOLIBS,kde, \
	$(if $(ENABLE_KDE4),kde4be1) \
	$(if $(USING_X11), \
		$(if $(ENABLE_KDE4),vclplug_kde4) \
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
		$(if $(ENABLE_MACOSX_SANDBOX),,\
			$(if $(shell test $(MACOSX_SDK_VERSION) -ge 101200 || echo not),avmediaQuickTime) \
		) \
	) \
	$(call gb_Helper_optional,SCRIPTING, \
		basctl \
		basprov \
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
		dbaxml) \
	dbtools \
	deploymentmisc \
	$(if $(filter-out MACOSX WNT,$(OS)),desktopbe1) \
	$(if $(USING_X11),desktop_detector) \
	$(call gb_Helper_optional,SCRIPTING,dlgprov) \
	$(if $(ENABLE_DIRECTX),directx9canvas) \
	$(if $(ENABLE_OPENGL_CANVAS),oglcanvas) \
	drawinglayer \
	editeng \
	$(if $(filter WNT,$(OS)),$(if $(DISABLE_ATL),,emser)) \
	evtatt \
	expwrap \
	$(call gb_Helper_optional,DBCONNECTIVITY, \
		flat \
		file) \
	filterconfig \
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
    gie \
	hyphen \
    icg \
	$(if $(ENABLE_JAVA),jdbc) \
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
	ucpimage \
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
	$(if $(USING_X11), \
		vclplug_gen \
	) \
	writerperfect \
	xmlscript \
	xmlfa \
	xmlfd \
	xo \
	xof \
	xsltdlg \
	xsltfilter \
	mork \
	mozbootstrap \
	$(if $(filter $(OS),WNT), \
		ado \
		$(if $(DISABLE_ATL),,oleautobridge) \
		smplmail \
		wininetbe1 \
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

$(eval $(call gb_Helper_register_libraries_for_install,OOOLIBS,firebirdsdbc, \
	$(if $(ENABLE_FIREBIRD_SDBC),firebird_sdbc) \
))

ifneq ($(ENABLE_PDFIMPORT),)
$(eval $(call gb_Helper_register_libraries_for_install,OOOLIBS,pdfimport, \
	pdfimport \
))
endif

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

# cli_cppuhelper is NONE even though it is actually in URE because it is CliNativeLibrary
$(eval $(call gb_Helper_register_libraries,PLAINLIBS_NONE, \
	getuid \
	smoketest \
	subsequenttest \
	test \
	test-setupvcl \
	testtools_cppobj \
	testtools_bridgetest \
	testtools_bridgetest-common \
	testtools_constructors \
	unobootstrapprotector \
	unoexceptionprotector \
	unotest \
	vclbootstrapprotector \
	scqahelper \
	unowinreg \
	wpftqahelper \
	$(if $(filter MSC,$(COM)),cli_cppuhelper) \
	$(if $(filter $(OS),ANDROID),lo-bootstrap) \
	$(if $(filter $(OS),MACOSX),OOoSpotlightImporter) \
))

$(eval $(call gb_Helper_register_libraries_for_install,PLAINLIBS_URE,ure, \
	affine_uno_uno \
	$(if $(filter MSC,$(COM)),cli_uno) \
	i18nlangtag \
	$(if $(ENABLE_JAVA), \
		java_uno \
		jpipe \
		$(if $(filter $(OS),WNT),jpipx) \
	    juh \
		juhx \
	) \
	log_uno_uno \
	unsafe_uno_uno \
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

$(eval $(call gb_Helper_register_libraries_for_install,PLAINLIBS_OOO,ooo, \
	$(if $(ENABLE_VLC),avmediavlc) \
	$(if $(ENABLE_GSTREAMER_1_0),avmediagst) \
	$(if $(ENABLE_GSTREAMER_0_10),avmediagst_0_10) \
	$(if $(ENABLE_DIRECTX),avmediawin) \
	cached1 \
	collator_data \
	comphelper \
	$(call gb_Helper_optional,DBCONNECTIVITY,dbpool2) \
	$(call gb_Helper_optional,BREAKPAD,crashreport) \
	deployment \
	deploymentgui \
	dict_ja \
	dict_zh \
	embobj \
	$(if $(ENABLE_JAVA),hsqldb) \
	i18nutil \
	index_data \
	$(if $(and $(ENABLE_GTK3), $(filter LINUX %BSD SOLARIS,$(OS))), libreofficekitgtk) \
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
	$(if $(USING_X11),recentfile) \
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
	xsec_gpg \
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

ifeq ($(ENABLE_GLTF),TRUE)
$(eval $(call gb_Helper_register_libraries_for_install,PLAINLIBS_OOO,ooo, \
    avmediaogl \
))
endif

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

# 'test_unittest' is only package delivering to workdir.
# Other packages could be potentially autoinstalled.
$(eval $(call gb_Helper_register_packages, \
	test_unittest \
	cli_basetypes_copy \
	desktop_install \
	$(if $(filter DESKTOP,$(BUILD_TYPE)),desktop_scripts_install) \
	extras_fonts \
	extras_wordbook \
	$(if $(filter WNTGCC,$(OS)$(COM)), \
		mingw_dlls \
		mingw_gccdlls \
	) \
	$(if $(filter MSC,$(COM)),msvc_dlls) \
	more_fonts_conf \
	instsetoo_native_setup \
	$(if $(ENABLE_OOENV),instsetoo_native_ooenv) \
	odk_headers_generated \
	oox_customshapes \
	oox_generated \
	postprocess_images \
	postprocess_registry \
	python_zipcore \
	readlicense_oo_readmes \
	setup_native_misc \
	share \
	vcl_fontunxpsprint \
))

$(eval $(call gb_Helper_register_packages_for_install,impress,\
	sd_xml \
))

$(eval $(call gb_Helper_register_packages_for_install,calc,\
	sc_res_xml \
))

$(eval $(call gb_Helper_register_packages_for_install,libreofficekit,\
	$(if $(filter LINUX %BSD SOLARIS, $(OS)),$(if $(ENABLE_GTK3),libreofficekit_selectionhandles)) \
))

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

$(eval $(call gb_Helper_register_packages_for_install,postgresqlsdbc,\
	$(if $(BUILD_POSTGRESQL_SDBC),connectivity_postgresql-sdbc) \
))

$(eval $(call gb_Helper_register_packages_for_install,sdk,\
	odk_share_readme \
	odk_share_readme_generated \
	$(if $(filter WNT,$(OS)),odk_cli) \
	odk_config \
	$(if $(filter WNT,$(OS)),odk_config_win) \
	odk_docs \
	$(if $(DOXYGEN),odk_doxygen) \
	odk_examples \
	odk_headers \
	odk_html \
	odk_settings \
	odk_settings_generated \
	offapi_idl \
	udkapi_idl \
	$(if $(ENABLE_JAVA), \
		odk_javadoc \
		odk_uno_loader_classes \
		odk_unowinreg \
	) \
))

$(eval $(call gb_Helper_register_packages_for_install,ooo,\
	$(if $(SYSTEM_LIBEXTTEXTCAT),,libexttextcat_fingerprint) \
	officecfg_misc \
	$(if $(filter $(OS),MACOSX), \
		extensions_mdibundle \
		extensions_OOoSpotlightImporter \
	) \
	extras_autocorr \
	extras_autotextuser \
	extras_cfgsrvnolang \
	extras_cfgusr \
	extras_database \
	extras_databasebiblio \
	extras_gallbullets \
	extras_gallhtmlexpo \
	extras_gallmytheme \
	extras_gallroot \
	extras_gallsystem \
	extras_gallwwwback \
	extras_gallwwwgraf \
	extras_glade \
	extras_labels \
	$(if $(filter WNT,$(OS)),extras_newfiles) \
	extras_palettes \
	extras_tplofficorr \
	extras_tploffimisc \
	extras_tplpresnt \
	extras_tpl_styles \
	extras_tplpersonal \
	extras_tplwizbitmap \
	extras_tplwizdesktop \
	extras_tplwizletter \
	extras_tplwizfax \
	extras_tplwizagenda \
	extras_tplwizreport \
	extras_tplwizstyles \
	framework_dtd \
	$(if $(filter $(OS),MACOSX),infoplist) \
	package_dtd \
	sd_web \
	sfx2_emojiconfig \
	$(call gb_Helper_optional,DESKTOP,\
		$(if $(filter-out WNT,$(OS)),$(if $(ENABLE_MACOSX_SANDBOX),,shell_senddoc))) \
	$(call gb_Helper_optional,DESKTOP,$(if $(filter-out MACOSX WNT,$(OS)),svx_gengal)) \
	$(if $(USING_X11),vcl_fontunxppds) \
	$(if $(filter $(OS),MACOSX),vcl_osxres) \
	xmloff_dtd \
	xmlscript_dtd \
	xmlhelp_helpxsl \
	$(if $(ENABLE_JAVA),\
		scripting_java \
		scripting_java_jars \
		$(if $(ENABLE_SCRIPTING_BEANSHELL),scripting_ScriptsBeanShell) \
		$(if $(ENABLE_SCRIPTING_JAVASCRIPT),scripting_ScriptsJavaScript) \
	) \
	$(if $(DISABLE_SCRIPTING),,scripting_scriptbindinglib) \
	$(if $(filter $(OS),MACOSX),sysui_osxicons) \
	wizards_basicshare \
	wizards_basicsrvaccess2base \
	wizards_basicsrvdepot \
	wizards_basicsrveuro \
	wizards_basicsrvgimmicks \
	wizards_basicsrvimport \
	wizards_basicsrvform \
	wizards_basicsrvstandard \
	wizards_basicsrvtemplate \
	wizards_basicsrvtools \
	wizards_basicsrvtutorials \
	wizards_basicusr \
	xmlsec \
	chart2_opengl_shader \
	vcl_opengl_shader \
	$(if $(filter WNT,$(OS)), \
		vcl_opengl_blacklist \
	) \
	$(if $(ENABLE_OPENGL_CANVAS),canvas_opengl_shader) \
	$(if $(DISABLE_PYTHON),,$(if $(filter-out AIX,$(OS)), \
		Pyuno/commonwizards \
		Pyuno/fax \
		Pyuno/letter \
		Pyuno/agenda \
		Pyuno/web \
		Pyuno/mailmerge \
	)) \
	sfx2_classification \
    $(if $(filter OPENCL,$(BUILD_TYPE)),sc_opencl_runtimetest) \
))

$(eval $(call gb_Helper_register_packages_for_install,ogltrans,\
	sd_opengl \
	slideshow_opengl_shader \
))

ifneq ($(ENABLE_PDFIMPORT),)
$(eval $(call gb_Helper_register_packages_for_install,pdfimport, \
	sdext_pdfimport_pdf \
))
endif

$(eval $(call gb_Helper_register_packages_for_install,reportbuilder,\
	reportbuilder-templates \
))

$(eval $(call gb_Helper_register_packages_for_install,xsltfilter,\
	filter_docbook \
	filter_xhtml \
	filter_xslt \
))

$(eval $(call gb_Helper_register_packages_for_install,brand,\
	desktop_branding \
	$(if $(CUSTOM_BRAND_DIR),desktop_branding_custom) \
	$(if $(and $(filter-out MACOSX WNT,$(OS)),$(filter DESKTOP,$(BUILD_TYPE))),desktop_soffice_sh) \
	readlicense_oo_files \
	$(if $(filter WNT,$(OS)),readlicense_oo_license) \
	$(call gb_Helper_optional,DESKTOP,setup_native_packinfo) \
))

ifeq ($(USING_X11), TRUE)
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
endif # USING_X11=TRUE

$(eval $(call gb_Helper_register_packages_for_install,onlineupdate,\
	$(if $(ENABLE_ONLINE_UPDATE),$(if $(filter LINUX SOLARIS,$(OS)),setup_native_scripts)) \
))

ifneq ($(DISABLE_PYTHON),TRUE)
$(eval $(call gb_Helper_register_packages_for_install,python, \
    pyuno_pythonloader_ini \
	pyuno_python_scripts \
	$(if $(SYSTEM_PYTHON),,$(if $(filter-out WNT,$(OS)),python_shell)) \
	scripting_ScriptsPython \
))

$(eval $(call gb_Helper_register_packages_for_install,python_scriptprovider, \
    scriptproviderforpython \
))

$(eval $(call gb_Helper_register_packages_for_install,python_librelogo, \
	librelogo \
	librelogo_properties \
))
endif # DISABLE_PYTHON

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

# Resources
$(eval $(call gb_Helper_register_resources,\
	abp \
	acc \
	analysis \
	avmedia \
	$(call gb_Helper_optional,SCRIPTING,basctl) \
	$(call gb_Helper_optional,DBCONNECTIVITY,bib) \
	chartcontroller \
	cnr \
	cui \
	date \
	$(call gb_Helper_optional,DBCONNECTIVITY,\
		dba \
		dbmm \
		dbp \
		dbu \
	) \
	dbw \
	deployment \
	deploymentgui \
	dkt \
	editeng \
	eps \
	eur \
	for \
	forui \
	$(call gb_Helper_optional,DESKTOP,fps_office) \
	frm \
	fwe \
	gal \
	imp \
	ofa \
	$(call gb_Helper_optional,DBCONNECTIVITY,pcr) \
	pdffilter \
	pricing \
	rpt \
	rptui \
	$(call gb_Helper_optional,SCRIPTING,sb) \
	sc \
	scn \
	sd \
	sdbcl \
	sdberr \
	$(call gb_Helper_optional,DBCONNECTIVITY,sdbt) \
	sfx \
	sm \
	$(if $(ENABLE_COINMP)$(ENABLE_LPSOLVE),solver) \
	svl \
	svt \
	svx \
	sw \
	t602filter \
	tpl \
	upd \
	$(if $(ENABLE_ONLINE_UPDATE),updchk) \
	uui \
	vcl \
	writerperfect \
	wzi \
	$(if $(ENABLE_NSS),xmlsec) \
	xsltdlg \
))

# UI configuration
$(eval $(call gb_Helper_register_uiconfigs,\
	cui \
	$(call gb_Helper_optional,DBCONNECTIVITY,dbaccess) \
	desktop \
	filter \
	formula \
	fps \
	$(call gb_Helper_optional,SCRIPTING,modules/BasicIDE) \
	$(call gb_Helper_optional,DBCONNECTIVITY,\
		modules/dbapp \
		modules/dbbrowser \
		modules/dbquery \
		modules/dbrelation \
	) \
	modules/dbreport \
	$(call gb_Helper_optional,DBCONNECTIVITY,\
		modules/dbtable \
		modules/dbtdata \
	) \
	modules/sabpilot \
	$(call gb_Helper_optional,DBCONNECTIVITY,modules/sbibliography) \
	modules/scalc \
	modules/scanner \
	modules/schart \
	modules/sdraw \
	modules/sglobal \
	modules/simpress \
	modules/smath \
	$(call gb_Helper_optional,DBCONNECTIVITY,modules/spropctrlr) \
	modules/StartModule \
	modules/sweb \
	modules/swform \
	modules/swreport \
	modules/swriter \
	modules/swxform \
	sfx \
	svt \
	svx \
	$(if $(ENABLE_TELEPATHY),tubes) \
	uui \
	vcl \
	writerperfect \
	$(if $(ENABLE_NSS),xmlsec) \
))

ifeq ($(gb_GBUILDSELFTEST),t)
$(eval $(call gb_Helper_register_libraries_for_install,OOOLIBS,ooo, gbuildselftestdep gbuildselftest))
$(eval $(call gb_Helper_register_executables,NONE, gbuildselftestexe))
endif

# vim: set noet sw=4 ts=4:
