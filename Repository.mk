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
    $(call gb_Helper_optional,HELPTOOLS, \
	HelpIndexer \
	HelpLinker \
    ) \
	bestreversemap \
	canvasdemo \
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
	helpex \
	idxdict \
	io-testconnection \
	langsupport \
	$(if $(filter iOS,$(OS)),LibreOffice) \
	lngconvex \
	localize \
    $(if $(filter-out ANDROID MACOSX iOS WNT,$(OS)),lockfile) \
	makedepend \
	mbsdiff \
	osl_process_child \
	pdf2xml \
	pdfunzip \
	pdfverify \
	pocheck \
	propex \
	regsvrex \
	saxparser \
	sp2bv \
	svidl \
	$(if $(ENABLE_ONLINE_UPDATE_MAR),\
		test_updater_dialog \
	) \
	treex \
	ulfex \
	unoidl-read \
	unoidl-write \
	xrmex \
	$(if $(filter-out ANDROID iOS WNT,$(OS)), \
        svdemo \
        fftester \
        svptest \
        svpclient ) \
	$(if $(filter LINUX %BSD SOLARIS,$(OS)), tilebench) \
	$(if $(filter LINUX MACOSX SOLARIS WNT %BSD,$(OS)),icontest) \
	vcldemo \
	tiledrendering \
    mtfdemo \
    visualbackendtest \
	$(if $(and $(ENABLE_GTK3), $(filter LINUX %BSD SOLARIS,$(OS))), gtktiledviewer) \
))

$(eval $(call gb_Helper_register_executables_for_install,SDK,sdk, \
	$(if $(filter MSC,$(COM)),$(if $(filter-out AARCH64,$(CPUNAME)),climaker)) \
	cppumaker \
	idlc \
	javamaker \
	$(if $(filter UCPP,$(BUILD_TYPE)),ucpp) \
	$(if $(filter ODK,$(BUILD_TYPE)),unoapploader) \
	unoidl-check \
	$(if $(filter ODK,$(BUILD_TYPE)),uno-skeletonmaker) \
))

$(eval $(call gb_Helper_register_executables_for_install,OOO,brand, \
	$(if $(ENABLE_ONLINE_UPDATE_MAR),\
		mar \
		$(if $(filter WNT,$(OS)), \
			update_service \
		) \
		updater )\
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
	$(call gb_Helper_optional,FUZZERS,pcxfuzzer) \
	$(call gb_Helper_optional,FUZZERS,rasfuzzer) \
	$(call gb_Helper_optional,FUZZERS,tgafuzzer) \
	$(call gb_Helper_optional,FUZZERS,tiffuzzer) \
	$(call gb_Helper_optional,FUZZERS,hwpfuzzer) \
	$(call gb_Helper_optional,FUZZERS,602fuzzer) \
	$(call gb_Helper_optional,FUZZERS,lwpfuzzer) \
	$(call gb_Helper_optional,FUZZERS,olefuzzer) \
	$(call gb_Helper_optional,FUZZERS,pptfuzzer) \
	$(call gb_Helper_optional,FUZZERS,rtffuzzer) \
	$(call gb_Helper_optional,FUZZERS,cgmfuzzer) \
	$(call gb_Helper_optional,FUZZERS,ww2fuzzer) \
	$(call gb_Helper_optional,FUZZERS,ww6fuzzer) \
	$(call gb_Helper_optional,FUZZERS,ww8fuzzer) \
	$(call gb_Helper_optional,FUZZERS,qpwfuzzer) \
	$(call gb_Helper_optional,FUZZERS,slkfuzzer) \
	$(call gb_Helper_optional,FUZZERS,fodtfuzzer) \
	$(call gb_Helper_optional,FUZZERS,fodsfuzzer) \
	$(call gb_Helper_optional,FUZZERS,fodpfuzzer) \
	$(call gb_Helper_optional,FUZZERS,xlsfuzzer) \
	$(call gb_Helper_optional,FUZZERS,scrtffuzzer) \
	$(call gb_Helper_optional,FUZZERS,wksfuzzer) \
	$(call gb_Helper_optional,FUZZERS,diffuzzer) \
	$(call gb_Helper_optional,FUZZERS,docxfuzzer) \
	$(call gb_Helper_optional,FUZZERS,xlsxfuzzer) \
	$(call gb_Helper_optional,FUZZERS,pptxfuzzer) \
	$(call gb_Helper_optional,FUZZERS,mmlfuzzer) \
	$(call gb_Helper_optional,FUZZERS,mtpfuzzer) \
	$(call gb_Helper_optional,FUZZERS,htmlfuzzer) \
	$(call gb_Helper_optional,FUZZERS,sftfuzzer) \
	$(call gb_Helper_optional,FUZZERS,dbffuzzer) \
	$(if $(filter-out ANDROID HAIKU iOS MACOSX WNT,$(OS)),oosplash) \
	soffice_bin \
	$(if $(filter DESKTOP,$(BUILD_TYPE)),unopkg_bin) \
	$(if $(filter WNT,$(OS)), \
		soffice_exe \
		soffice_com \
		soffice_safe \
		unoinfo \
		unopkg \
		unopkg_com \
		$(if $(filter-out AARCH64,$(CPUNAME)),twain32shim) \
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
		$(if $(DISABLE_GUI),, \
			ui-previewer \
		) \
	) \
	$(if $(filter WNT,$(OS)), \
		senddoc \
	) \
	$(if $(filter OPENCL,$(BUILD_TYPE)),opencltest) \
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

ifneq ($(ENABLE_POPPLER),)
$(eval $(call gb_Helper_register_executables_for_install,OOO,pdfimport, \
	xpdfimport \
))
endif

$(eval $(call gb_Helper_register_executables_for_install,UREBIN,ure,\
	$(if $(and $(ENABLE_JAVA),$(filter-out HAIKU MACOSX WNT,$(OS)),$(filter DESKTOP,$(BUILD_TYPE))),javaldx) \
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
	wpftcalc \
	solver \
	$(call gb_Helper_optional,SCRIPTING,vbaobj) \
))

$(eval $(call gb_Helper_register_plugins_for_install,OOOLIBS,calc, \
    scui \
))

$(eval $(call gb_Helper_register_libraries_for_install,OOOLIBS,graphicfilter, \
	svgfilter \
	wpftdraw \
	graphicfilter \
))

$(eval $(call gb_Helper_register_libraries_for_install,OOOLIBS,impress, \
	animcore \
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
	$(if $(ENABLE_GIO),losessioninstall) \
	$(if $(ENABLE_GIO),ucpgio1) \
))

$(eval $(call gb_Helper_register_plugins_for_install,OOOLIBS,gnome, \
    $(if $(ENABLE_GTK3),vclplug_gtk3) \
    $(if $(ENABLE_GTK4),vclplug_gtk4) \
))

$(eval $(call gb_Helper_register_libraries_for_install,OOOLIBS,kde, \
	$(if $(ENABLE_KF5),kf5be1) \
))

ifneq ($(OS),HAIKU)
$(eval $(call gb_Helper_register_plugins_for_install,OOOLIBS,kde, \
    $(if $(ENABLE_KF5),vclplug_kf5) \
    $(if $(ENABLE_QT5),vclplug_qt5) \
    $(if $(ENABLE_QT6),vclplug_qt6) \
    $(if $(ENABLE_GTK3_KDE5),vclplug_gtk3_kde5) \
))

ifneq ($(ENABLE_GTK3_KDE5),)
$(eval $(call gb_Helper_register_executables_for_install,OOO,kde, \
       lo_kde5filepicker \
))
endif

else # HAIKU

$(eval $(call gb_Helper_register_plugins_for_install,OOOLIBS,haiku, \
    $(if $(ENABLE_QT5),vclplug_qt5) \
    $(if $(ENABLE_QT6),vclplug_qt6) \
    $(if $(ENABLE_KF5),vclplug_kf5) \
))
endif

$(eval $(call gb_Helper_register_libraries_for_install,OOOLIBS,math, \
	sm \
	smd \
))

$(eval $(call gb_Helper_register_libraries_for_install,OOOLIBS,ogltrans, \
	OGLTrans \
))

$(eval $(call gb_Helper_register_libraries_for_install,OOOLIBS,ooo, \
    avmedia \
    $(call gb_Helper_optional,AVMEDIA, \
	$(if $(filter MACOSX,$(OS)),\
		avmediaMacAVF \
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
	$(call gb_Helper_optional,OPENCL,clew) \
	$(if $(filter $(OS),WNT),,cmdmail) \
	cppcanvas \
	configmgr \
	ctl \
	dba \
	dbahsql \
	$(call gb_Helper_optional,DBCONNECTIVITY, \
		dbase \
		dbaxml) \
	dbtools \
	deploymentmisc \
	$(if $(filter-out MACOSX WNT,$(OS)),desktopbe1) \
	$(call gb_Helper_optional,SCRIPTING,dlgprov) \
	$(if $(filter WNT,$(OS)),directx9canvas) \
	$(if $(ENABLE_OPENGL_CANVAS),oglcanvas) \
	drawinglayer \
	editeng \
	$(if $(filter WNT,$(OS)),emser) \
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
	fwk \
	$(if $(filter WNT,$(OS)),gdipluscanvas) \
	guesslang \
    $(call gb_Helper_optionals_or,HELPTOOLS XMLHELP,helplinker) \
	i18npool \
	i18nsearch \
	hyphen \
	$(if $(ENABLE_JAVA),jdbc) \
	$(if $(ENABLE_LDAP),ldapbe2) \
	$(if $(filter WNT,$(OS)),WinUserInfoBe) \
	localebe1 \
	log \
	lng \
	lnth \
	$(if $(filter $(OS),MACOSX),macbe1) \
	$(if $(MERGELIBS),merged) \
	migrationoo2 \
	migrationoo3 \
	mozbootstrap \
	msfilter \
	$(call gb_Helper_optional,SCRIPTING,msforms) \
	mtfrenderer \
	$(call gb_Helper_optional,DBCONNECTIVITY,mysql_jdbc) \
	$(call gb_Helper_optional,MARIADBC,$(call gb_Helper_optional,DBCONNECTIVITY,mysqlc)) \
	numbertext \
	odbc \
	odfflatxml \
	offacc \
	oox \
	$(call gb_Helper_optional,OPENCL,opencl) \
	passwordcontainer \
	pcr \
	pdffilter \
	$(call gb_Helper_optional,SCRIPTING,protocolhandler) \
	sax \
	sb \
	$(call gb_Helper_optional,DBCONNECTIVITY,sdbt) \
	scn \
	sd \
	sdd \
	sdfilt \
	sfx \
	simplecanvas \
	slideshow \
	sot \
	spell \
	$(if $(DISABLE_GUI),,spl) \
	storagefd \
	$(call gb_Helper_optional,SCRIPTING,stringresource) \
	svgio \
	emfio \
	svl \
	svt \
	svx \
	svxcore \
	sw \
	syssh \
	textconversiondlgs \
	textfd \
	tk \
	tl \
	ucpexpand1 \
	ucpext \
	ucpimage \
	$(if $(ENABLE_LIBCMIS),ucpcmis1) \
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
		oleautobridge \
		smplmail \
		wininetbe1 \
	) \
	$(if $(filter $(OS),MACOSX), \
		$(if $(ENABLE_MACOSX_SANDBOX),, \
			AppleRemote \
		) \
		fps_aqua \
	) \
	$(if $(filter iOS MACOSX,$(OS)), \
		MacOSXSpell \
	) \
))

$(eval $(call gb_Helper_register_plugins_for_install,OOOLIBS,ooo, \
    acc \
    $(if $(ENABLE_CUSTOMTARGET_COMPONENTS),components) \
    cui \
    $(if $(USING_X11),desktop_detector) \
    icg \
    sdui \
    $(if $(ENABLE_GEN),vclplug_gen) \
    $(if $(filter $(OS),WNT),vclplug_win) \
    $(if $(filter $(OS),MACOSX),vclplug_osx) \
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
	t602filter \
	$(call gb_Helper_optional,SCRIPTING,vbaswobj) \
	wpftwriter \
	writerfilter \
	$(call gb_Helper_optional,DBCONNECTIVITY,writer) \
))

$(eval $(call gb_Helper_register_plugins_for_install,OOOLIBS,writer, \
    swui \
))

# cli_cppuhelper is NONE even though it is actually in URE because it is CliNativeLibrary
$(eval $(call gb_Helper_register_libraries,PLAINLIBS_NONE, \
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
	swqahelper \
	wpftqahelper \
	precompiled_system \
	$(if $(filter MSC,$(COM)),cli_cppuhelper) \
	$(if $(filter $(OS),ANDROID),lo-bootstrap) \
	$(if $(filter $(OS),MACOSX),OOoSpotlightImporter) \
))

$(eval $(call gb_Helper_register_libraries_for_install,PLAINLIBS_URE,ure, \
	affine_uno_uno \
	$(if $(filter MSC,$(COM)),$(if $(filter-out AARCH64,$(CPUNAME)),cli_uno)) \
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
))

$(eval $(call gb_Helper_register_plugins_for_install,PLAINLIBS_URE,ure, \
	$(if $(filter EMSCRIPTEN,$(OS)),, \
		$(if $(filter MSC,$(COM)), \
			$(if $(filter INTEL,$(CPUNAME)),msci_uno) \
			$(if $(filter X86_64,$(CPUNAME)),mscx_uno) \
			$(if $(filter AARCH64,$(CPUNAME)),msca_uno) \
		, gcc3_uno) \
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
	stocservices \
	store \
	unoidl \
	uuresolver \
	xmlreader \
))

$(eval $(call gb_Helper_register_plugins_for_install,PRIVATELIBS_URE,ure, \
    sal_textenc \
))

$(eval $(call gb_Helper_register_libraries_for_install,PLAINLIBS_OOO,ooo, \
    $(call gb_Helper_optional,AVMEDIA, \
        $(if $(ENABLE_GSTREAMER_1_0),avmediagst) \
        $(if $(filter WNT,$(OS)),avmediawin) \
    ) \
	cached1 \
	comphelper \
	$(call gb_Helper_optional,DBCONNECTIVITY,dbpool2) \
	$(call gb_Helper_optional,BREAKPAD,crashreport) \
	deployment \
	deploymentgui \
	embobj \
	$(if $(ENABLE_JAVA),hsqldb) \
	i18nutil \
	$(if $(and $(ENABLE_GTK3), $(filter LINUX %BSD SOLARIS,$(OS))), libreofficekitgtk) \
	$(if $(ENABLE_JAVA), \
		$(if $(filter $(OS),MACOSX),,officebean) \
	) \
	emboleobj \
	package2 \
	$(call gb_Helper_optional,SCRIPTING,scriptframe) \
	sdbc2 \
	sofficeapp \
	srtrs1 \
	ucb1 \
	ucbhelper \
	$(if $(WITH_WEBDAV),ucpdav1) \
	ucpfile1 \
	ucpftp1 \
    $(call gb_Helper_optional,XMLHELP,ucpchelp1) \
	ucphier1 \
	ucppkg1 \
	unopkgapp \
	xmlsecurity \
	xsec_xmlsec \
	xstor \
	$(if $(filter $(OS),MACOSX), \
		macab1 \
		macabdrv1 \
	) \
	$(if $(filter WNT,$(OS)), \
		fps \
		inprocserv \
		UAccCOM \
		winaccessibility \
	) \
))

$(eval $(call gb_Helper_register_plugins_for_install,PLAINLIBS_OOO,ooo, \
    collator_data \
    dict_ja \
    dict_zh \
    index_data \
    localedata_en \
    localedata_es \
    localedata_euro \
    localedata_others \
    textconv_dict \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Helper_register_libraries_for_install,PLAINLIBS_OOO,activexbinarytable, \
	regactivex \
))

$(eval $(call gb_Helper_register_libraries_for_install,PLAINLIBS_OOO,activex, \
	so_activex \
))

ifneq ($(BUILD_X64),)
$(eval $(call gb_Helper_register_libraries_for_install,PLAINLIBS_OOO,activexwin64, \
	so_activex_x64 \
))
endif

$(eval $(call gb_Helper_register_executables_for_install,OOO,spsuppfiles, \
	spsupp_helper \
))

$(eval $(call gb_Helper_register_libraries_for_install,PLAINLIBS_OOO,spsuppfiles, \
	$(if $(CXX_X64_BINARY),spsupp_x64) \
	$(if $(CXX_X86_BINARY),spsupp_x86) \
))

$(eval $(call gb_Helper_register_libraries_for_install,PLAINLIBS_OOO,ooobinarytable, \
	$(if $(WINDOWS_SDK_HOME),\
		instooofiltmsi \
		inst_msu_msi \
		qslnkmsi \
		reg_dlls \
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
	propertyhdl \
	shlxthdl \
))

ifneq ($(BUILD_X64),)
$(eval $(call gb_Helper_register_packages_for_install,winexplorerextwin64, \
	$(if $(filter MSC,$(COM)),msvc_dlls) \
))

$(eval $(call gb_Helper_register_libraries_for_install,PLAINLIBS_SHLXTHDL,winexplorerextwin64, \
	ooofilt_x64 \
	propertyhdl_x64 \
	shlxthdl_x64 \
))
endif # BUILD_X64

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
	passive_native \
))

ifneq ($(ENABLE_JAVA),)
$(eval $(call gb_Helper_register_jars_for_install,URE,ure, \
	java_uno \
	juh \
	jurt \
	libreoffice \
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
	extras_wordbook \
	instsetoo_native_setup \
	$(if $(ENABLE_OOENV),instsetoo_native_ooenv) \
	postprocess_registry \
	readlicense_oo_readmes \
	setup_native_misc \
	sysui_share \
	vcl_fontunxpsprint \
))

$(eval $(call gb_Helper_register_packages_for_install,impress,\
	sd_xml \
))

$(eval $(call gb_Helper_register_packages_for_install,calc,\
	sc_res_xml \
))

$(eval $(call gb_Helper_register_packages_for_install,libreofficekit,\
	$(if $(filter LINUX %BSD SOLARIS, $(OS)),libreofficekit_selectionhandles) \
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
	$(if $(filter WNT,$(OS)),$(if $(filter-out AARCH64,$(CPUNAME)),odk_cli)) \
	odk_config \
	$(if $(filter WNT,$(OS)),odk_config_win) \
	odk_docs \
	$(if $(DOXYGEN),odk_doxygen) \
	odk_examples \
	odk_headers \
	odk_headers_generated \
	odk_html \
	odk_settings \
	odk_settings_generated \
	offapi_idl \
	udkapi_idl \
	$(if $(ENABLE_JAVA), \
		odk_javadoc \
		odk_uno_loader_classes \
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
	extras_gallmytheme \
	extras_gallroot \
	extras_gallsystem \
	extras_gallsystemstr \
	extras_glade \
	extras_labels \
	$(if $(filter WNT,$(OS)),extras_newfiles) \
	extras_palettes \
	extras_personas \
	extras_persona_dark \
	extras_persona_gray \
	extras_persona_green \
	extras_persona_pink \
	extras_persona_sand \
	extras_persona_white \
	extras_tplofficorr \
	extras_tploffimisc \
	extras_tplpresnt \
	extras_tpl_styles \
	extras_tpldraw \
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
	oox_customshapes \
	oox_generated \
	package_dtd \
	sd_web \
	sfx2_emojiconfig \
	$(call gb_Helper_optional,DESKTOP,\
		$(if $(filter-out WNT,$(OS)),$(if $(ENABLE_MACOSX_SANDBOX),,shell_senddoc))) \
	$(call gb_Helper_optional,DESKTOP,$(if $(filter-out EMSCRIPTEN MACOSX WNT,$(OS)),svx_gengal)) \
	$(if $(USING_X11),vcl_fontunxppds) \
	$(if $(filter $(OS),MACOSX),vcl_osxres) \
	xmloff_dtd \
	xmlscript_dtd \
    $(call gb_Helper_optional,XMLHELP,xmlhelp_helpxsl) \
	$(if $(ENABLE_JAVA),\
		scripting_java \
		scripting_java_jars \
		$(if $(ENABLE_SCRIPTING_BEANSHELL),scripting_ScriptsBeanShell) \
		$(if $(ENABLE_SCRIPTING_JAVASCRIPT),scripting_ScriptsJavaScript) \
	) \
	$(call gb_Helper_optional,SCRIPTING,scripting_scriptbindinglib) \
	$(if $(filter $(OS),MACOSX),sysui_osxicons) \
	wizards_basicshare \
	wizards_basicsrvaccess2base \
	wizards_basicsrvdepot \
	wizards_basicsrveuro \
	wizards_basicsrvgimmicks \
	wizards_basicsrvimport \
	wizards_basicsrvform \
	wizards_basicsrvscriptforge \
	wizards_basicsrvsfdatabases \
	wizards_basicsrvsfdialogs \
	wizards_basicsrvsfdocuments \
	wizards_basicsrvsfwidgets \
	wizards_basicsrvstandard \
	wizards_basicsrvtemplate \
	wizards_basicsrvtools \
	wizards_basicsrvtutorials \
	wizards_basicusr \
	wizards_properties \
	wizards_wizardshare \
	tipoftheday_images \
	toolbarmode_images \
	vcl_theme_definitions \
	$(if $(filter WNT,$(OS)), \
		vcl_opengl_denylist \
	) \
	$(if $(ENABLE_OPENGL_CANVAS),canvas_opengl_shader) \
	$(if $(filter SKIA,$(BUILD_TYPE)), \
		vcl_skia_denylist ) \
	$(if $(DISABLE_PYTHON),,$(if $(filter-out AIX,$(OS)), \
		Pyuno/commonwizards \
		Pyuno/fax \
		Pyuno/letter \
		Pyuno/agenda \
		Pyuno/mailmerge \
	)) \
	sfx2_classification \
    $(if $(filter OPENCL,$(BUILD_TYPE)),sc_opencl_runtimetest) \
	$(if $(ENABLE_HTMLHELP),\
		helpcontent2_html_dynamic \
		helpcontent2_html_media \
		helpcontent2_html_icon-themes \
		helpcontent2_html_static \
	) \
	resource_fonts \
	cui \
))

$(eval $(call gb_Helper_register_packages_for_install,ooo_fonts,\
	extras_fonts \
	$(if $(USING_X11)$(DISABLE_GUI)$(filter ANDROID,$(OS)), \
		postprocess_fontconfig) \
	$(call gb_Helper_optional,MORE_FONTS,\
		fonts_alef \
		fonts_amiri \
		fonts_caladea \
		fonts_carlito \
		$(if $(MPL_SUBSET),,fonts_culmus) \
		fonts_dejavu \
		fonts_emojione_color \
		fonts_gentium \
		$(if $(MPL_SUBSET),,fonts_kacst) \
		fonts_liberation \
		fonts_liberation_narrow \
		fonts_libertineg \
		fonts_libre_hebrew \
		fonts_noto \
		fonts_reem \
		fonts_sourcecode \
		fonts_sourcesans \
		fonts_sourceserif \
		fonts_scheherazade \
	) \
))

$(eval $(call gb_Helper_register_packages_for_install,ooo_images,\
	postprocess_images \
	$(call gb_Helper_optional,HELP,helpcontent2_helpimages) \
))

$(eval $(call gb_Helper_register_packages_for_install,ogltrans,\
	sd_opengl \
	slideshow_opengl_shader \
))

ifneq ($(ENABLE_POPPLER),)
$(eval $(call gb_Helper_register_packages_for_install,pdfimport, \
	sdext_pdfimport_pdf \
))
endif

$(eval $(call gb_Helper_register_packages_for_install,reportbuilder,\
	reportbuilder_templates \
))

$(eval $(call gb_Helper_register_packages_for_install,xsltfilter,\
	filter_docbook \
	filter_xhtml \
	filter_xslt \
))

$(eval $(call gb_Helper_register_packages_for_install,brand,\
	desktop_branding \
	$(if $(CUSTOM_BRAND_DIR),desktop_branding_custom) \
	$(if $(filter DESKTOP,$(BUILD_TYPE)),desktop_scripts_install) \
	$(if $(and $(filter-out EMSCRIPTEN HAIKU MACOSX WNT,$(OS)),$(filter DESKTOP,$(BUILD_TYPE))),\
		$(if $(DISABLE_GUI),, \
			desktop_soffice_sh \
		) \
	) \
	readlicense_oo_files \
	readlicense_oo_license \
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
    scripting_scriptproviderforpython \
))

ifeq (LIBRELOGO,$(filter LIBRELOGO,$(BUILD_TYPE)))
$(eval $(call gb_Helper_register_packages_for_install,python_librelogo, \
	librelogo \
	librelogo_properties \
))
endif # LIBRELOGO

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
$(eval $(call gb_Helper_register_mos,\
	acc \
    $(call gb_Helper_optional,AVMEDIA,avmedia) \
	$(call gb_Helper_optional,SCRIPTING,basctl) \
	chart \
	cnr \
	cui \
	dba \
	dkt \
	editeng \
	flt \
	for \
	$(call gb_Helper_optional,DESKTOP,fps) \
	frm \
	fwk \
	oox \
	pcr \
	rpt \
	$(call gb_Helper_optional,SCRIPTING,sb) \
	sc \
	sca \
	scc \
	sd \
	sfx \
	shell \
	sm \
	svl \
	svt \
	svx \
	sw \
	uui \
	vcl \
	wiz \
	wpt \
	$(if $(ENABLE_NSS)$(ENABLE_OPENSSL),xsc) \
))

# UI configuration
$(eval $(call gb_Helper_register_uiconfigs,\
	cui \
	$(call gb_Helper_optional,DBCONNECTIVITY,dbaccess) \
	desktop \
	editeng \
	filter \
	formula \
	fps \
	libreofficekit \
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
	uui \
	vcl \
	writerperfect \
	$(if $(ENABLE_NSS)$(ENABLE_OPENSSL),xmlsec) \
))

ifeq ($(gb_GBUILDSELFTEST),t)
$(eval $(call gb_Helper_register_libraries_for_install,OOOLIBS,ooo, gbuildselftestdep gbuildselftest))
$(eval $(call gb_Helper_register_executables,NONE, gbuildselftestexe))
endif

# vim: set noet sw=4 ts=4:
