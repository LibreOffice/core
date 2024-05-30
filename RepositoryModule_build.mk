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
	net_ure \
	o3tl \
	offapi \
	officecfg \
	oovbaapi \
	pch \
	registry \
	remotebridges \
	ridljar \
	sal \
	salhelper \
	sax \
	setup_native \
	shell \
	solenv \
	soltools \
	$(if $(filter EMSCRIPTEN,$(BUILD_TYPE_FOR_HOST)),static) \
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
	$(if $(WITH_GALLERY_BUILD), \
		avmedia \
		basic \
		canvas \
		configmgr \
		connectivity \
		cppcanvas \
		docmodel \
		drawinglayer \
		editeng \
		emfio \
		filter \
		framework \
		linguistic \
		officecfg \
		oox \
		package \
		postprocess \
		sfx2 \
		shell \
		sot \
		svgio \
		svl \
		svtools \
		svx \
		toolkit \
		ucb \
		unoxml \
		uui \
		vcl \
		xmloff \
		xmlscript \
	) \
    $(call gb_Helper_optionals_or,HELPTOOLS XMLHELP,helpcompiler) \
    $(call gb_Helper_optional,XMLHELP,xmlhelp) \
	$(call gb_Helper_optional,QADEVOOO,qadevOOo) \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
