# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# we link all object files from these libraries into one, merged library

MERGE_LIBRARY_LIST := \
	acc \
	$(call gb_Helper_optional,AVMEDIA,avmedia) \
	$(if $(filter MACOSX,$(OS)),\
		avmediaMacAVF \
		$(if $(ENABLE_MACOSX_SANDBOX),,avmediaQuickTime) \
	) \
	$(if $(filter $(OS),ANDROID),,basebmp) \
	basegfx \
	bib \
	canvastools \
	configmgr \
	ctl \
	cppcanvas \
	cui \
	$(call gb_Helper_optional,DBCONNECTIVITY, \
		dba \
		dbase \
		dbmm \
		dbaxml) \
	$(call gb_Helper_optional,DBCONNECTIVITY,dbtools) \
	deployment \
	deploymentmisc \
	$(if $(filter-out MACOSX WNT,$(OS)),desktopbe1) \
	$(if $(USING_X11),desktop_detector) \
	drawinglayer \
	$(call gb_Helper_optional,SCRIPTING,dlgprov) \
	$(if $(ENABLE_DIRECTX),directx9canvas) \
	$(if $(ENABLE_OPENGL_CANVAS),oglcanvas) \
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
	$(if $(filter $(ENABLE_FIREBIRD_SDBC),TRUE),firebird_sdbc) \
	fps_office \
	for \
	forui \
	frm \
	filterconfig \
	fsstorage \
	fwe \
	fwi \
	fwk \
	fwm \
	$(if $(ENABLE_DIRECTX),gdipluscanvas) \
	guesslang \
	$(call gb_Helper_optional,DESKTOP,helplinker) \
	i18npool \
	i18nutil \
	$(if $(filter DESKTOP,$(BUILD_TYPE)),helplinker) \
	i18nsearch \
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
	ldapbe2 \
	log \
	lng \
	lnth \
	hyphen \
	lng \
	localebe1 \
	$(if $(filter $(OS),MACOSX),macbe1) \
	$(if $(MERGELIBS),merged) \
	migrationoo2 \
	migrationoo3 \
	$(call gb_Helper_optional,SCRIPTING,msforms) \
	mtfrenderer \
	$(call gb_Helper_optional,DBCONNECTIVITY,mysql) \
	mcnttype \
	msfilter \
	odbc \
	odfflatxml \
	offacc \
	oox \
	$(call gb_Helper_optional,OPENCL,opencl) \
	package2 \
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
	simplecanvas \
	slideshow \
	sfx \
	sofficeapp \
	sot \
    spell \
	$(if $(ENABLE_HEADLESS),,spl) \
	svl \
	svt \
	svx \
	storagefd \
	$(call gb_Helper_optional,SCRIPTING,stringresource) \
	svgio \
	svxcore \
	$(if $(ENABLE_TDEAB),tdeab1) \
	$(if $(ENABLE_TDEAB),tdeabdrv1) \
	textconversiondlgs \
	textfd \
	tk \
	tl \
	$(if $(filter TRUE,$(ENABLE_TELEPATHY)),tubes) \
	ucb1 \
	ucpexpand1 \
	ucpfile1 \
	unoxml \
	utl \
	uui \
	vcl \
	xmlscript \
	xo \
	xstor \


# allow module-deps.pl to color based on this.
ifneq ($(ENABLE_PRINT_DEPS),)

$(info MergeLibContents: $(MERGE_LIBRARY_LIST))

endif

ifneq ($(MERGELIBS),)

gb_MERGEDLIBS := $(MERGE_LIBRARY_LIST)

endif

# vim: set noet sw=4 ts=4:
