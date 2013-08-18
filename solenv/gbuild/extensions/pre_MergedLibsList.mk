# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

ifneq ($(MERGELIBS),)
# set of libraries to link even more stuff into one merged library
gb_EXTRAMERGEDLIBS := \
	$(if $(filter-out ANDROID IOS,$(OS)),abp) \
	$(if $(filter unx,$(GUIBASE)),basebmp) \
	$(call gb_Helper_optional,DBCONNECTIVITY,bib) \
	$(call gb_Helper_optional,DBCONNECTIVITY,calc) \
	chartcore \
	$(call gb_Helper_optional,DBCONNECTIVITY,dba) \
	$(call gb_Helper_optional,DBCONNECTIVITY,dbase) \
	$(call gb_Helper_optional,DBCONNECTIVITY,dbp) \
	$(call gb_Helper_optional,DBCONNECTIVITY,dbtools) \
	$(call gb_Helper_optional,DBCONNECTIVITY,dbu) \
	deploymentgui \
	$(call gb_Helper_optional,EXPORT,egi) \
	embobj \
	emboleobj \
	$(call gb_Helper_optional,EXPORT,eme) \
	$(call gb_Helper_optional,EXPORT,epb) \
	$(call gb_Helper_optional,EXPORT,epg) \
	$(call gb_Helper_optional,EXPORT,epp) \
	$(call gb_Helper_optional,EXPORT,eps) \
	$(call gb_Helper_optional,EXPORT,ept) \
	$(call gb_Helper_optional,EXPORT,era) \
	$(call gb_Helper_optional,EXPORT,eti) \
	$(call gb_Helper_optional,EXPORT,exp) \
	$(call gb_Helper_optional,DBCONNECTIVITY,file) \
	$(call gb_Helper_optional,DBCONNECTIVITY,flat) \
	flash \
	for \
	forui \
	fpicker \
	fps_office \
	$(call gb_Helper_optional,DBCONNECTIVITY,frm) \
	fwl \
	fwm \
	graphicfilter \
	guesslang \
	$(if $(ENABLE_JAVA),hsqldb) \
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
	$(if $(filter-out ANDROID IOS,$(OS)),ldapbe2) \
	lnth \
	$(if $(filter-out ANDROID IOS,$(OS)),log) \
	$(if $(ENABLE_LWP),lwpft) \
	$(call gb_Helper_optional,DESKTOP,migrationoo2) \
	$(call gb_Helper_optional,DESKTOP,migrationoo3) \
	$(call gb_Helper_optional,DBCONNECTIVITY,mork) \
	$(call gb_Helper_optional,DBCONNECTIVITY,mysql) \
	$(if $(filter-out ANDROID IOS,$(OS)),odbc) \
	$(if $(filter-out ANDROID IOS,$(OS)),odbcbase) \
	odfflatxml \
	offacc \
	oox \
	$(call gb_Helper_optional,DBCONNECTIVITY,pcr) \
	pdffilter \
	placeware \
	res \
	$(call gb_Helper_optional,DBCONNECTIVITY,rpt) \
	$(call gb_Helper_optional,DBCONNECTIVITY,rptui) \
	$(call gb_Helper_optional,DBCONNECTIVITY,rptxml) \
	$(if $(filter-out ANDROID IOS,$(OS)),scn) \
	sd \
	$(call gb_Helper_optional,DBCONNECTIVITY,sdbc2) \
	$(if $(filter unx,$(GUIBASE)),spa) \
	spell \
	$(if $(filter-out WNT MACOSX ANDROID IOS,$(OS)),spl_unx) \
	svgfilter \
	swd \
	t602filter \
	test \
	textfd \
	$(call gb_Helper_optional,DESKTOP,unopkgapp) \
	unotest \
	$(call gb_Helper_optional,DESKTOP,updatefeed) \
	$(if $(DISABLE_SCRIPTING),,vbahelper) \
	xmlfa \
	xmlfd \
	xmlsecurity \
	xsltfilter

# FIXME: just retaining these for now - they currently crash & need thought.
#gb_EXTRAMERGEDLIBS := \
	comphelper \
	sc \
	sw \


# we link all object files from these libraries into one, merged library
gb_MERGEDLIBS := \
	avmedia \
	basegfx \
	canvastools \
	configmgr \
	cppcanvas \
	deployment \
	deploymentmisc \
	$(if $(filter-out MACOSX WNT,$(OS)),desktopbe1) \
	$(if $(filter unx,$(GUIBASE)),desktop_detector) \
	drawinglayer \
	editeng \
	fileacc \
	filterconfig \
	fsstorage \
	fwe \
	fwi \
	fwk \
	$(call gb_Helper_optional,DESKTOP,helplinker) \
	i18npool \
	i18nutil \
	lng \
	localebe1 \
	mcnttype \
	msfilter \
	package2 \
	sax \
	sb \
	sfx \
	sofficeapp \
	sot \
	$(if $(ENABLE_HEADLESS),,spl) \
	svl \
	svt \
	svx \
	svxcore \
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
	$(if $(filter ALL,$(MERGELIBS)),$(gb_EXTRAMERGEDLIBS))

ifneq (,$(URELIBS))
gb_URELIBS := \
	bootstrap \
	cppu \
	cppuhelper \
	introspection \
	$(if $(ENABLE_JAVA),jvmaccess) \
	$(if $(ENABLE_JAVA),jvmfwk) \
	reflection \
	reg \
	sal \
	salhelper \
	stocservices \
	store \
	unoidl \
	xmlreader \
	$(if $(filter ALL,$(MERGELIBS)), \
		purpenvhelper \
	)

endif

endif

# vim: set noet sw=4 ts=4:
