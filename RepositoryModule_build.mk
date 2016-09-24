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
	instsetoo_native \
	idl \
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
	$(call gb_Helper_optional_for_host,DESKTOP, \
		$(if $(WITH_GALLERY_BUILD), \
			avmedia \
			basic \
			canvas \
			configmgr \
			connectivity \
			cppcanvas \
			drawinglayer \
			editeng \
			extensions \
			filter \
			framework \
			linguistic \
			mysqlc \
			officecfg \
			package \
			postprocess \
			sfx2 \
			sot \
			svgio \
			svl \
			svtools \
			svx \
			swext \
			toolkit \
			ucb \
			unoxml \
			vcl \
			xmloff \
			xmlscript \
		) \
		helpcompiler \
		xmlhelp \
	) \
	$(call gb_Helper_optional,QADEVOOO,qadevOOo) \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
