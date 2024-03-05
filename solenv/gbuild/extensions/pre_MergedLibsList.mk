# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# we link all object files from these libraries into one, merged library
gb_MERGE_LIBRARY_LIST := \
	avmedia \
	$(if $(filter WNT,$(OS)),avmediawin) \
	$(call gb_Helper_optional,SCRIPTING, \
		basctl \
		basprov \
	) \
	basegfx \
	canvasfactory \
	canvastools \
	chartcontroller \
	chartcore \
	comphelper \
	configmgr \
	cppcanvas \
	$(call gb_Helper_optional,BREAKPAD,crashreport) \
	ctl \
	dbtools \
	deployment \
	deploymentmisc \
	$(if $(filter-out MACOSX WNT,$(OS)),desktopbe1) \
	$(if $(filter WNT,$(OS)),directx9canvas) \
	docmodel \
	drawinglayercore \
	drawinglayer \
	editeng \
	embobj \
	emboleobj \
	emfio \
	$(if $(filter WNT,$(OS)),emser) \
	evtatt \
	filterconfig \
	for \
	forui \
	fps_office \
	frm \
	fsstorage \
	fwk \
	$(if $(filter WNT,$(OS)),gdipluscanvas) \
	guesslang \
	$(call gb_Helper_optional,DESKTOP,helplinker) \
	hyphen \
	i18nsearch \
	i18npool \
	i18nutil \
	icg \
	lng \
	lnth \
	localebe1 \
	$(if $(filter iOS MACOSX,$(OS)),MacOSXSpell) \
	msfilter \
	mtfrenderer \
	numbertext \
	odfflatxml \
	offacc \
	$(if $(ENABLE_OPENGL_CANVAS),oglcanvas) \
	oox \
	$(if $(filter OPENCL,$(BUILD_TYPE)),opencl) \
	package2 \
	passwordcontainer \
	sax \
	sb \
	simplecanvas \
	sfx \
	sofficeapp \
	sot \
	spell \
	$(if $(or $(DISABLE_GUI),$(ENABLE_WASM_STRIP_SPLASH)),,spl) \
	srtrs1 \
	$(call gb_Helper_optional,SCRIPTING,stringresource) \
	svgio \
	svl \
	svt \
	svx \
	svxcore \
	syssh \
	textfd \
	tk \
	tl \
	ucb1 \
	ucbhelper \
	ucpexpand1 \
	ucpext \
	ucphier1 \
	ucpimage \
	ucpfile1 \
	ucptdoc1 \
	unordf \
	unoxml \
	updatefeed \
	utl \
	uui \
	vbaevents \
	vbahelper \
	vcl \
	vclcanvas \
	xsec_xmlsec \
	xmlfa \
	xmlfd \
	xmlscript \
	xo \
	xof \
	xsltdlg \
	xsltfilter \
	xstor \

# if we have --enable-mergelibs=more
ifneq ($(MERGELIBS_MORE),)

gb_MERGE_LIBRARY_LIST += \
	acc \
	$(if $(filter $(OS),WNT), \
		ado \
	) \
	analysis \
	animcore \
	$(call gb_Helper_optional,AVMEDIA, \
		$(if $(filter MACOSX,$(OS)),\
			avmediaMacAVF \
		) \
	) \
	bib \
	cached1 \
	$(if $(ENABLE_CAIRO_CANVAS),cairocanvas) \
	cui \
	date \
	dba \
	dbahsql \
	$(call gb_Helper_optional,DBCONNECTIVITY, \
		dbase \
		dbaxml) \
	$(call gb_Helper_optional,DBCONNECTIVITY,dbpool2) \
	$(call gb_Helper_optional,DBCONNECTIVITY,dbu) \
	dlgprov \
	$(if $(ENABLE_EVOAB2),evoab) \
	$(call gb_Helper_optional,DBCONNECTIVITY, \
		flat \
		file) \
	$(if $(ENABLE_FIREBIRD_SDBC),firebird_sdbc) \
	$(if $(filter WNT,$(OS)), \
		fps \
	) \
	$(if $(filter MACOSX,$(OS)),\
		fps_aqua \
	) \
	graphicfilter \
	hwp \
	log \
	$(if $(ENABLE_LWP),lwpft) \
	$(if $(filter $(OS),MACOSX), \
		macab1 \
		macabdrv1 \
	) \
	mozbootstrap \
	msforms \
	$(call gb_Helper_optional,DBCONNECTIVITY,mysql_jdbc) \
	$(call gb_Helper_optional,MARIADBC,$(call gb_Helper_optional,DBCONNECTIVITY,mysqlc)) \
	OGLTrans \
	odbc \
	pdffilter \
	$(if $(BUILD_POSTGRESQL_SDBC), \
		postgresql-sdbc \
		postgresql-sdbc-impl) \
	pricing \
	$(call gb_Helper_optional,SCRIPTING,protocolhandler) \
	$(call gb_Helper_optional,SCRIPTING,scriptframe) \
	sdbc2 \
	slideshow \
	$(if $(filter WNT,$(OS)), \
		smplmail \
	) \
	solver \
	storagefd \
	svgfilter \
	t602filter \
	textconversiondlgs \
	$(call gb_Helper_optional,XMLHELP,ucpchelp1) \
	$(if $(ENABLE_LIBCMIS),ucpcmis1) \
	$(if $(WITH_WEBDAV),ucpdav1) \
	ucppkg1 \
	wpftcalc \
	wpftdraw \
	wpftimpress \
	wpftwriter \
	$(if $(filter WNT,$(OS)), \
		wininetbe1 \
	) \
	writerfilter \
	writerperfect \
	xmlsecurity \

endif

# allow module-deps.pl to color based on this.
ifneq ($(ENABLE_PRINT_DEPS),)

$(info MergeLibContents: $(gb_MERGE_LIBRARY_LIST))

endif

ifneq ($(MERGELIBS),)

gb_MERGEDLIBS := $(gb_MERGE_LIBRARY_LIST)

endif

# vim: set noet sw=4 ts=4:
