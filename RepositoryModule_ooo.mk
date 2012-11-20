# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org. If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

$(eval $(call gb_Module_Module,ooo))

$(eval $(call gb_Module_add_moduledirs,ooo,\
	accessibility \
	$(call gb_Helper_optional,AFMS,afms) \
	animations \
	$(call gb_Helper_optional,APACHE_COMMONS,apache-commons) \
	apple_remote \
	autodoc \
	avmedia \
	basctl \
	basebmp \
	basegfx \
	basic \
	bean \
	$(call gb_Helper_optional,BSH,beanshell) \
	binaryurp \
	$(call gb_Helper_optional,BLUEZ,bluez_bluetooth) \
	$(call gb_Helper_optional,BOOST,boost) \
	bridges \
	$(call gb_Helper_optional,CAIRO,cairo) \
	canvas \
	chart2 \
	cli_ure \
	$(call gb_Helper_optional,CLUCENE,clucene) \
	$(call gb_Helper_optional,DESKTOP,codemaker) \
	comphelper \
	configmgr \
	connectivity \
	cosv \
	cppcanvas \
	cppu \
	cppuhelper \
	cpputools \
	$(call gb_Helper_optional,CT2N,ct2n) \
	cui \
	$(call gb_Helper_optional,CURL,curl) \
	dbaccess \
	desktop \
	$(call gb_Helper_optional,DICTIONARIES,dictionaries) \
	drawinglayer \
	dtrans \
	editeng \
	embeddedobj \
	embedserv \
	$(call gb_Helper_optional,EPM,epm) \
	eventattacher \
	$(call gb_Helper_optional,EXPAT,expat) \
	extensions \
	extras \
	fileaccess \
	filter \
	$(call gb_Helper_optional,FONTCONFIG,fontconfig) \
	forms \
	formula \
	fpicker \
	framework \
	$(call gb_Helper_optional,FREETYPE,freetype) \
	$(call gb_Helper_optional,GRAPHITE,graphite) \
	$(call gb_Helper_optional,DESKTOP,helpcompiler) \
	$(call gb_Helper_optional,HSQLDB,hsqldb) \
	$(call gb_Helper_optional,HUNSPELL,hunspell) \
	hwpfilter \
	$(call gb_Helper_optional,HYPHEN,hyphen) \
	i18npool \
	i18nutil \
	idl \
	$(call gb_Helper_optional,DESKTOP,idlc) \
	io \
	javaunohelper \
	$(call gb_Helper_optional,JFREEREPORT,jfreereport) \
	jurt \
	$(call gb_Helper_optional,JPEG,jpeg) \
	jvmaccess \
	jvmfwk \
	$(call gb_Helper_optional,DESKTOP,l10ntools) \
	$(call gb_Helper_optional,LANGUAGETOOL,languagetool) \
	$(call gb_Helper_optional,LCMS2,lcms2) \
	libcdr \
	libcmis \
	$(call gb_Helper_optional,LIBEXTTEXTCAT,libexttextcat) \
	$(call gb_Helper_optional,LIBLANGTAG,liblangtag) \
	libmspub \
	liborcus \
	$(call gb_Helper_optional,LIBPNG,libpng) \
	librelogo \
	libvisio \
	libwpd \
	libwpg \
	libwps \
	lingucomponent \
	linguistic \
	lotuswordpro \
	$(call gb_Helper_optional,LPSOLVE,lpsolve) \
	MathMLDTD \
	mdds \
	Mesa \
	$(call gb_Helper_optional,MORE_FONTS,more_fonts) \
	$(call gb_Helper_optional,MYTHES,mythes) \
	$(call gb_Helper_optional,NEON,neon) \
	$(call gb_Helper_optional,NLPSOLVER,nlpsolver) \
	np_sdk \
	o3tl \
	offapi \
	officecfg \
	oovbaapi \
	oox \
	$(call gb_Helper_optional,OPENLDAP,openldap) \
	package \
	packimages \
	padmin \
	$(call gb_Helper_optional,POSTGRESQL,postgresql) \
	psprint_config \
	$(call gb_Helper_optional,PYUNO,pyuno) \
	$(call gb_Helper_optional,QADEVOOO,qadevOOo) \
	readlicense_oo \
	registry \
	remotebridges \
	reportbuilder \
	reportdesign \
	$(call gb_Helper_optional,RHINO,rhino) \
	ridljar \
	rsc \
	sal \
	salhelper \
	sane \
	sax \
	sc \
	scaddins \
	sccomp \
	$(call gb_Helper_optional,DESKTOP,scp2) \
	scripting \
	sd \
	sdext \
	sfx2 \
	shell \
	slideshow \
	smoketest \
	solenv \
	soltools \
	sot \
	starmath \
	stoc \
	store \
	svgio \
	svl \
	svtools \
	svx \
	sw \
	swext \
	test \
	testtools \
	$(call gb_Helper_optional,TOMCAT,tomcat) \
	toolkit \
	tools \
	touch \
	$(call gb_Helper_optional,TRANSLATIONS,translations) \
	tubes \
	twain \
	ucb \
	ucbhelper \
	$(call gb_Helper_optional,UCPP,ucpp) \
	udkapi \
	udm \
	unixODBC \
	UnoControls \
	unodevtools \
	unoil \
	unotest \
	unotools \
	unoxml \
	ure \
	uui \
	vbahelper \
	vcl \
	vigra \
	wizards \
	writerfilter \
	writerperfect \
	$(call gb_Helper_optional,X11_EXTENSIONS,x11_extensions) \
	xmerge \
	$(call gb_Helper_optional,DESKTOP,xmlhelp) \
	xmloff \
	xmlreader \
	xmlscript \
	xmlsecurity \
	xsltml \
	$(call gb_Helper_optional,ZLIB,zlib) \
))

# vim: set noet ts=4 sw=4:
