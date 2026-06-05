# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# we link all object files from these libraries into one, merged library
gb_MERGE_LIBRARY_LIST := \
	$(if $(filter $(OS),WNT), \
		$(call gb_Helper_optional,DBCONNECTIVITY,ado) \
	) \
	analysis \
	animcore \
	avmedia \
	$(call gb_Helper_optional,AVMEDIA, \
		$(if $(filter MACOSX,$(OS)),\
			avmediaMacAVF \
		) \
	) \
	$(call gb_Helper_optional,AVMEDIA, \
		$(if $(filter WNT,$(OS)),avmediawin) \
	) \
	$(call gb_Helper_optional,SCRIPTING, \
		basctl \
		basprov \
	) \
	basegfx \
	bib \
	cached1 \
	$(if $(ENABLE_CAIRO_CANVAS),cairocanvas) \
	canvasfactory \
	canvastools \
	chart2 \
	chart2api \
	comphelper \
	configmgr \
	cppcanvas \
	ctl \
	cui \
	date \
	$(call gb_Helper_optional,DBCONNECTIVITY, \
		dba \
		dbase \
		dbaxml \
		dbpool2 \
		dbu) \
	$(call gb_Helper_optional,SCRIPTING,dlgprov) \
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
	$(if $(ENABLE_EVOAB2),evoab) \
	evtatt \
	filterconfig \
	$(call gb_Helper_optional,DBCONNECTIVITY, \
		flat \
		file) \
	for \
	forui \
	$(if $(filter WNT,$(OS)), \
		fps \
	) \
	$(if $(filter MACOSX,$(OS)),\
		fps_aqua \
	) \
	fps_office \
	$(call gb_Helper_optional,DBCONNECTIVITY,frm) \
	fsstorage \
	fwk \
	$(if $(filter WNT,$(OS)),gdipluscanvas) \
	guesslang \
	graphicfilter \
	$(call gb_Helper_optionals_and,DESKTOP XMLHELP,helplinker) \
	hwp \
	hyphen \
	i18nsearch \
	i18npool \
	i18nutil \
	icg \
	$(call gb_Helper_optional,QUICKJS,jsuno) \
	lng \
	lnth \
	localebe1 \
	log \
	$(if $(filter $(OS),MACOSX), \
		macab1 \
	) \
	$(if $(filter iOS MACOSX,$(OS)),MacOSXSpell) \
	$(call gb_Helper_optional,DBCONNECTIVITY,mozbootstrap) \
	$(call gb_Helper_optional,SCRIPTING,msforms) \
	msfilter \
	mtfrenderer \
	$(call gb_Helper_optional,DBCONNECTIVITY,mysql_jdbc) \
	$(call gb_Helper_optional,MARIADBC,$(call gb_Helper_optional,DBCONNECTIVITY,mysqlc)) \
	numbertext \
	$(call gb_Helper_optional,DBCONNECTIVITY,odbc) \
	odfflatxml \
	offacc \
	oox \
	$(if $(filter OPENCL,$(BUILD_TYPE)),opencl) \
	package2 \
	passwordcontainer \
	pdffilter \
	pricing \
	$(call gb_Helper_optional,SCRIPTING,protocolhandler) \
	sax \
	sb \
	$(call gb_Helper_optional,SCRIPTING,scriptframe) \
	$(call gb_Helper_optional,DBCONNECTIVITY,sdbc2) \
	$(call gb_Helper_optional,DBCONNECTIVITY,sdbt) \
	simplecanvas \
	slideshow \
	$(if $(filter WNT,$(OS)), \
		smplmail \
	) \
	storagefd \
	svgfilter \
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
	t602filter \
	textconversiondlgs \
	textfd \
	tk \
	tl \
	ucb1 \
	ucbhelper \
	$(call gb_Helper_optional,XMLHELP,ucpchelp1) \
	$(if $(ENABLE_LIBCMIS),ucpcmis1) \
	$(if $(WITH_WEBDAV),ucpdav1) \
	ucppkg1 \
	ucpexpand1 \
	ucpext \
	ucphier1 \
	ucpimage \
	ucpfile1 \
	ucptdoc1 \
	unordf \
	unoxml \
	utl \
	uui \
	$(call gb_Helper_optional,SCRIPTING, \
		vbaevents \
		vbahelper \
	) \
	vcl \
	vclcanvas \
	wpftcalc \
	wpftdraw \
	wpftimpress \
	wpftwriter \
	$(if $(filter WNT,$(OS)), \
		wininetbe1 \
	) \
	writerperfect \
	xsec_xmlsec \
	xmlfa \
	xmlfd \
	xmlsecurity \
	xmlscript \
	xo \
	xof \
	xsltdlg \
	xsltfilter \
	xstor \


# allow module-deps.pl to color based on this.
ifneq ($(ENABLE_PRINT_DEPS),)

$(info MergeLibContents: $(gb_MERGE_LIBRARY_LIST))

endif

ifneq ($(MERGELIBS),)

gb_MERGEDLIBS := $(gb_MERGE_LIBRARY_LIST)

endif

# vim: set noet sw=4 ts=4:
