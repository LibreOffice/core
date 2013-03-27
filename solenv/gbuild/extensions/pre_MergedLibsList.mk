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

# we link all object files from these libraries into one, merged library
gb_MERGEDLIBS := \
	avmedia \
	$(if $(filter unx,$(GUIBASE)),basebmp) \
	$(call gb_Helper_optional,DBCONNECTIVITY,calc) \
	canvastools \
	chartcore \
	configmgr \
	cppcanvas \
	cui \
	$(call gb_Helper_optional,DBCONNECTIVITY,dba) \
	$(call gb_Helper_optional,DBCONNECTIVITY,dbase) \
	$(call gb_Helper_optional,DBCONNECTIVITY,dbtools) \
	$(call gb_Helper_optional,DBCONNECTIVITY,dbu) \
	deploymentmisc \
	$(if $(filter unx,$(GUIBASE)),desktop_detector) \
	desktopbe1 \
	drawinglayer \
	editeng \
	$(call gb_Helper_optional,DBCONNECTIVITY,file) \
	filterconfig \
	$(call gb_Helper_optional,DBCONNECTIVITY,flat) \
	for \
	forui \
	$(call gb_Helper_optional,DBCONNECTIVITY,frm) \
	fsstorage \
	fwe \
	fwi \
	fwk \
	fwl \
	$(call gb_Helper_optional,DESKTOP,helplinker) \
	i18npool \
	i18nutil \
	$(if $(filter TRUE,$(SOLAR_JAVA)),javavm) \
	jvmfwk \
	lng \
	localebe1 \
	localedata_en \
	localedata_es \
	localedata_euro \
	localedata_others \
	msfilter \
	$(if $(filter-out ANDROID IOS,$(OS)),odbc) \
	$(if $(filter-out ANDROID IOS,$(OS)),odbcbase) \
	oox \
	package2 \
	$(call gb_Helper_optional,DBCONNECTIVITY,rpt) \
	$(call gb_Helper_optional,DBCONNECTIVITY,rptui) \
	$(call gb_Helper_optional,DBCONNECTIVITY,rptxml) \
	sax \
	sb \
	sc \
	scui \
	scfilt \
	sd \
	sdui \
	sfx \
	sofficeapp \
	sot \
	spl \
	subsequenttest \
	svl \
	svt \
	svx \
	svxcore \
	sw \
	swui \
	swd \
	test \
	tk \
	$(if $(filter TRUE,$(ENABLE_TELEPATHY)),tubes) \
	ucb1 \
	ucpfile1 \
	unotest \
	utl \
	uui \
	$(if $(DISABLE_SCRIPTING),,vbahelper) \
	vcl \
	$(if $(filter unx,$(GUIBASE)),vclplug_svp) \
	$(if $(filter-out IOS,$(OS)),wpftdraw) \
	$(if $(filter-out IOS,$(OS)),wpftwriter) \
	writerfilter \
	xmlscript \
	xmlsecurity \
	xo

ifeq ($(OS),ANDROID)
gb_MERGEDLIBS += \
	analysis \
	ctl \
	date \
	embobj \
	evtatt \
	fastsax \
	fileacc \
	for \
	forui \
	fwl \
	fwm \
	hwp \
	hyphen \
	lnth \
	pricing \
	spell \
	xstor \

endif

endif

# vim: set noet sw=4 ts=4:
