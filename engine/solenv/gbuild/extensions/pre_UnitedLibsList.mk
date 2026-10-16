# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# we link all object files from these internal libraries into one united library
gb_UNITED_LIBRARY_LIST := \
	abp \
	$(if $(filter $(OS),WNT), \
		$(call gb_Helper_optional,DBCONNECTIVITY,ado) \
	) \
	analysis \
	animcore \
	avmedia \
	$(call gb_Helper_optional,AVMEDIA, \
		$(if $(filter MACOSX,$(OS)),avmediaMacAVF) \
		$(if $(filter WNT,$(OS)),avmediawin) \
		$(if $(ENABLE_GTK4),avmediagtk) \
		$(if $(ENABLE_QT6_MULTIMEDIA),avmediaqt6) \
		$(if $(filter WNT,$(OS)),avmediawin) \
    ) \
	$(call gb_Helper_optional,SCRIPTING, \
		basctl \
		basprov \
	) \
	basegfx \
	bib \
	binaryurp \
	bootstrap \
	$(call gb_Helper_optional,DBCONNECTIVITY,calc) \
	cached1 \
	$(if $(ENABLE_CAIRO_CANVAS),cairocanvas) \
	canvastools \
	chart2 \
	chart2api \
	$(if $(filter $(OS),WNT),,cmdmail) \
    comphelper \
    $(if $(ENABLE_CUSTOMTARGET_COMPONENTS),components) \
	configmgr \
	cppu \
    cppuhelper \
	ctl \
	cui \
	date \
	$(call gb_Helper_optional,DBCONNECTIVITY, \
		dba \
		dbase \
		dbaxml \
		dbp \
		dbpool2 \
		dbu) \
	$(call gb_Helper_optional,SCRIPTING,dlgprov) \
	dbtools \
	$(if $(filter-out MACOSX WNT,$(OS)),desktopbe1) \
	docmodel \
	drawinglayercore \
	drawinglayer \
	$(if $(ENABLE_CURL),Duden) \
	editeng \
	embobj \
	emboleobj \
	emfio \
	$(if $(ENABLE_EVOAB2),evoab) \
	evtatt \
	filterconfig \
	$(call gb_Helper_optional,DBCONNECTIVITY, \
		flat \
		file) \
	for \
	forui \
	$(if $(filter WNT,$(OS)), fps) \
	$(if $(filter MACOSX,$(OS)), fps_aqua) \
	fps_office \
	$(call gb_Helper_optional,DBCONNECTIVITY,frm) \
	fsstorage \
	fwk \
	guesslang \
	graphicfilter \
	$(if $(ENABLE_CURL),Grammalecte) \
	$(call gb_Helper_optionals_and,DESKTOP XMLHELP,helplinker) \
	hwp \
	hyphen \
	i18nlangtag \
	i18npool \
	i18nsearch \
	i18nutil \
	icg \
	introspection \
	invocadapt \
	invocation \
	io \
	$(if $(ENABLE_JAVA),javaloader) \
	$(if $(ENABLE_JAVA),javavm) \
	$(if $(ENABLE_JAVA),jvmaccess) \
	$(if $(ENABLE_JAVA),jvmfwk) \
	$(if $(ENABLE_JAVA),jdbc) \
	$(call gb_Helper_optional,QUICKJS,jsuno) \
	$(if $(ENABLE_JAVA), \
		jvmaccess \
		jvmfwk) \
	$(if $(filter WNT,$(OS)),jumplist) \
	$(if $(and $(ENABLE_GTK3), $(filter LINUX %BSD SOLARIS,$(OS))), kitgtk) \
	$(if $(ENABLE_CURL),LanguageTool) \
	lightproof \
	lng \
	lnth \
	localebe1 \
	localedata_en \
	localedata_es \
	localedata_euro \
	localedata_others \
	log \
	$(if $(ENABLE_GIO),losessioninstall) \
	$(if $(filter $(OS),MACOSX),macab1) \
	$(if $(filter $(OS),MACOSX),macabdrv1) \
	$(if $(filter $(OS),MACOSX),macbe1) \
	$(if $(filter iOS MACOSX,$(OS)),MacOSXSpell) \
	$(call gb_Helper_optional,DBCONNECTIVITY,mozbootstrap) \
	msfilter \
	$(call gb_Helper_optional,SCRIPTING,msforms) \
	namingservice \
	numbertext \
	$(call gb_Helper_optional,DBCONNECTIVITY,odbc) \
	odfflatxml \
	offacc \
	$(if $(ENABLE_JAVA), \
		$(if $(filter $(OS),MACOSX),,officebean) \
	) \
	oox \
	$(if $(filter OPENCL,$(BUILD_TYPE)),opencl) \
	package2 \
	passwordcontainer \
	$(call gb_Helper_optional,DBCONNECTIVITY,pcr) \
	pdffilter \
    $(if $(ENABLE_PDFIMPORT),pdfimport) \
	pricing \
	$(call gb_Helper_optional,SCRIPTING,protocolhandler) \
	proxyfac \
	purpenvhelper \
	$(call gb_Helper_optional,PYTHON,pythonloader) \
	$(call gb_Helper_optional,PYTHON,pyuno) \
	reflection \
	reg \
    $(if $(ENABLE_RUST_UNO),rust_uno-cpp) \
    sal \
    salhelper \
    $(call gb_CondLibSalTextenc,sal_textenc) \
	sax \
	sb \
	sc \
	scd \
	scn \
	scui \
	$(call gb_Helper_optional,SCRIPTING,scriptinterop) \
	$(call gb_Helper_optional,SCRIPTING,scriptframe) \
	sd \
	sdd \
	sdui \
	$(call gb_Helper_optional,DBCONNECTIVITY,sdbc2) \
	$(call gb_Helper_optional,DBCONNECTIVITY,sdbt) \
	$(if $(filter WNT,$(OS)), smplmail) \
	storagefd \
	sfx \
	sm \
	smd \
	sofficeapp \
	solver \
	sot \
	spell \
	$(if $(DISABLE_GUI),,spl) \
	srtrs1 \
	stocservices \
	store \
	$(call gb_Helper_optional,SCRIPTING,stringresource) \
	svgio \
	svgfilter \
	svl \
	svt \
	svx \
	svxcore \
	sw \
	swd \
	swui \
	sw_writerfilter \
	syssh \
	t602filter \
	textconversiondlgs \
	textfd \
	tk \
	tl \
    ucbhelper \
	ucb1 \
	$(call gb_Helper_optional,XMLHELP,ucpchelp1) \
	$(if $(ENABLE_LIBCMIS),ucpcmis1) \
	$(if $(WITH_WEBDAV),ucpdav1) \
	ucppkg1 \
	ucpexpand1 \
	ucphier1 \
	$(if $(ENABLE_GIO),ucpgio1) \
	ucpimage \
	ucpfile1 \
	ucptdoc1 \
	unoidl \
    $(call gb_CondExeUnopkg,unopkgapp) \
	unordf \
	unoxml \
	utl \
	uui \
	uuresolver \
	$(call gb_Helper_optional,SCRIPTING, \
		vbaevents \
		vbahelper \
		vbaobj \
		vbaswobj \
	) \
	vcl \
	vclcanvas \
    $(if $(ENABLE_GEN),vclplug_gen) \
    $(if $(filter $(OS)-$(USE_HEADLESS_CODE),WNT-),vclplug_win) \
    $(if $(filter $(OS)-$(USE_HEADLESS_CODE),MACOSX-),vclplug_osx) \
	$(if $(ENABLE_GTK3),vclplug_gtk3) \
	$(if $(filter WNT,$(OS)),WinUserInfoBe) \
	wpftcalc \
	wpftdraw \
	wpftimpress \
	wpftwriter \
	$(if $(filter WNT,$(OS)), wininetbe1) \
	$(call gb_Helper_optional,DBCONNECTIVITY,writer) \
	writerperfect \
	xsec_xmlsec \
	xmlfa \
	xmlfd \
	xmlreader \
	xmlsecurity \
	xmlscript \
	xo \
	xof \
	xsltdlg \
	xsltfilter \
	xstor \
	$(if $(filter MSC,$(COM)), \
		$(if $(filter INTEL,$(CPUNAME)),msci_uno) \
		$(if $(filter X86_64,$(CPUNAME)),mscx_uno) \
		$(if $(filter AARCH64,$(CPUNAME)),msca_uno) \
	, gcc3_uno) \




ifneq ($(UNITEDLIBS),)

gb_UNITEDLIBS := $(gb_UNITED_LIBRARY_LIST)

endif

# vim: set noet sw=4 ts=4:
