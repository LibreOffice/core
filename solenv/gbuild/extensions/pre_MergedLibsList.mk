# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2012 Matúš Kukan <matus.kukan@gmail.com> (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

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
	fwl \
	graphicfilter \
	$(if $(filter TRUE,$(SOLAR_JAVA)),hsqldb) \
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
	$(if $(filter TRUE,$(SOLAR_JAVA)),jdbc) \
	$(if $(filter-out ANDROID IOS,$(OS)),log) \
	$(if $(filter-out ANDROID IOS,$(OS)),ldapbe2) \
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
	$(call gb_Helper_optional,PYUNO,pyuno) \
	res \
	$(call gb_Helper_optional,DBCONNECTIVITY,rpt) \
	$(if $(filter-out ANDROID IOS,$(OS)),scn) \
	sd \
	$(call gb_Helper_optional,DBCONNECTIVITY,sdbc2) \
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
	cppu \
	cppuhelper \
	$(if $(filter TRUE,$(SOLAR_JAVA)),jvmaccess) \
	$(if $(filter TRUE,$(SOLAR_JAVA)),jvmfwk) \
	purpenvhelper \
	reg \
	sal \
	salhelper \
	store \
	unoidl \
	xmlreader

endif

endif

# vim: set noet sw=4 ts=4:
