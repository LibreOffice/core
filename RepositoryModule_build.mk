# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,cross_toolset))

$(eval $(call gb_Module_add_moduledirs,cross_toolset,\
	basegfx \
	binaryurp \
	bridges \
	cli_ure \
	codemaker \
	comphelper \
	cppu \
	cppuhelper \
	external \
	i18nlangtag \
	i18nutil \
	i18npool \
	idl \
	idlc \
	io \
	javaunohelper \
	jurt \
	jvmaccess \
	jvmfwk \
	l10ntools \
	o3tl \
	offapi \
	officecfg \
	oovbaapi \
	registry \
	remotebridges \
	ridljar \
	rsc \
	sal \
	salhelper \
	sax \
	setup_native \
	shell \
	solenv \
	soltools \
	stoc \
	store \
	tools \
	ucbhelper \
	udkapi \
	unoidl \
	unoil \
	unotest \
	unotools \
	ure \
	xmlreader \
	$(call gb_Helper_optional,BOOST,boost) \
	$(call gb_Helper_optional,CLUCENE,clucene) \
	$(call gb_Helper_optional,CPPUNIT,cppunit) \
	$(call gb_Helper_optional_for_host,DESKTOP, \
		$(if $(filter YES,$(WITH_GALLERY_BUILD)), \
			avmedia \
			basebmp \
			basic \
			canvas \
			configmgr \
			cppcanvas \
			drawinglayer \
			editeng \
			fileaccess \
			framework \
			harfbuzz \
			lcms2 \
			linguistic \
			nss \
			package \
			sfx2 \
			sot \
			svl \
			svtools \
			svx \
			toolkit \
			ucb \
			unoxml \
			vcl \
			$(call gb_Helper_optional,VIGRA,vigra) \
			xmloff \
			xmlscript \
		) \
		helpcompiler \
		xmlhelp \
	) \
	$(call gb_Helper_optional,EXPAT,expat) \
	$(call gb_Helper_optional,ICU,icu) \
	$(call gb_Helper_optional,LIBLANGTAG,liblangtag) \
	$(call gb_Helper_optional,LIBXML2,libxml2) \
	$(call gb_Helper_optional,LIBXSLT,libxslt) \
	$(call gb_Helper_optional,QADEVOOO,qadevOOo) \
	$(call gb_Helper_optional,UCPP,ucpp) \
	$(call gb_Helper_optional,ZLIB,zlib) \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
