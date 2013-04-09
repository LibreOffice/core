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

ifeq ($(MERGELIBS),TRUE)

# FIXME: just retaining these for now - they currently crash & need thought.
# set of libraries to link even more stuff into one merged library
gb_EXTRAMERGEDLIBS := \
	analysis \
	$(if $(filter unx,$(GUIBASE)),basebmp) \
	$(call gb_Helper_optional,DBCONNECTIVITY,calc) \
	chartcore \
	ctl \
	cui \
	date \
	$(call gb_Helper_optional,DBCONNECTIVITY,dba) \
	$(call gb_Helper_optional,DBCONNECTIVITY,dbase) \
	$(call gb_Helper_optional,DBCONNECTIVITY,dbtools) \
	$(call gb_Helper_optional,DBCONNECTIVITY,dbu) \
	embobj \
	evtatt \
	fastsax \
	$(call gb_Helper_optional,DBCONNECTIVITY,file) \
	$(call gb_Helper_optional,DBCONNECTIVITY,flat) \
	for \
	forui \
	$(call gb_Helper_optional,DBCONNECTIVITY,frm) \
	fwl \
	fwm \
	hwp \
	hyphen \
	$(if $(filter TRUE,$(SOLAR_JAVA)),javaloader) \
	$(if $(filter TRUE,$(SOLAR_JAVA)),javavm) \
	$(if $(filter TRUE,$(SOLAR_JAVA)),java_uno) \
	$(if $(filter TRUE,$(SOLAR_JAVA)),juh) \
	$(if $(filter-out IOS,$(OS)),jvmaccess) \
	jvmfwk \
	lnth \
	localedata_en \
	localedata_es \
	localedata_euro \
	localedata_others \
	$(if $(filter-out ANDROID IOS,$(OS)),odbc) \
	$(if $(filter-out ANDROID IOS,$(OS)),odbcbase) \
	oox \
	pricing \
	$(call gb_Helper_optional,PYUNO,pythonloader) \
	$(call gb_Helper_optional,PYUNO,pyuno) \
	$(call gb_Helper_optional,DBCONNECTIVITY,rpt) \
	$(call gb_Helper_optional,DBCONNECTIVITY,rptui) \
	$(call gb_Helper_optional,DBCONNECTIVITY,rptxml) \
	sc \
	$(if $(DISABLE_SCRIPTING),,scriptframe) \
	scui \
	scfilt \
	sd \
	sdui \
	$(if $(filter-out IOS,$(OS)),solver) \
	spell \
	$(if $(DISABLE_SCRIPTING),,stringresource) \
	subsequenttest \
	sw \
	swui \
	swd \
	test \
	textfd \
	ucpcmis1 \
	unotest \
	$(if $(DISABLE_SCRIPTING),,vbahelper) \
	$(if $(filter-out IOS,$(OS)),wpftdraw) \
	$(if $(filter-out IOS,$(OS)),wpftwriter) \
	writerfilter \
	xmlfa \
	xmlfd \
	xmlsecurity

# we link all object files from these libraries into one, merged library
gb_MERGEDLIBS := \
	avmedia \
	canvastools \
	configmgr \
	cppcanvas \
	deploymentmisc \
	desktopbe1 \
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
	msfilter \
	package2 \
	sax \
	sb \
	sfx \
	sofficeapp \
	sot \
	spl \
	svl \
	svt \
	svx \
	svxcore \
	tk \
	$(if $(filter TRUE,$(ENABLE_TELEPATHY)),tubes) \
	ucb1 \
	ucpfile1 \
	utl \
	uui \
	vcl \
	xmlscript \
	xo \
	xstor \

ifneq (,$(URELIBS))
gb_URELIBS := \
	cppu \
	cppuhelper \
	$(if $(filter TRUE,$(SOLAR_JAVA)),jvmaccess) \
	$(if $(filter TRUE,$(SOLAR_JAVA)),jvmfwk) \
	reg \
	sal \
	salhelper \
	store \
	unoidl \
	xmlreader \

endif

endif

# vim: set noet sw=4 ts=4:
