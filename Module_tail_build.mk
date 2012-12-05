# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Initial Developer of the Original Code is
# Norbert Thiebaud <nthiebaud@gmail.com> (C) 2011, All Rights Reserved.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.


$(eval $(call gb_Module_Module,tail_build))

$(eval $(call gb_Module_add_moduledirs,tail_build,\
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
	$(call gb_Helper_optional,BOOST,boost) \
	$(call gb_Helper_optional,BSH,beanshell) \
	binaryurp \
	$(call gb_Helper_optional,BLUEZ,bluez_bluetooth) \
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
	$(call gb_Helper_optional,CPPUNIT,cppunit) \
	cpputools \
	$(call gb_Helper_optional,CT2N,ct2n) \
	cui \
	$(call gb_Helper_optional,CURL,curl) \
	dbaccess \
	desktop \
	$(call gb_Helper_optional,DICTIONARIES,dictionaries) \
	dtrans \
	drawinglayer \
	editeng \
	embeddedobj \
	embedserv \
	$(call gb_Helper_optional,EPM,epm) \
	eventattacher \
	extensions \
	extras \
	fileaccess \
	filter \
	forms \
	formula \
	fpicker \
	framework \
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
	$(call gb_Helper_optional,JPEG,jpeg) \
	jurt \
	jvmaccess \
	jvmfwk \
	$(call gb_Helper_optional,LANGUAGETOOL,languagetool) \
	$(call gb_Helper_optional,LCMS2,lcms2) \
	libcdr \
	libcmis \
	$(call gb_Helper_optional,LIBEXTTEXTCAT,libexttextcat) \
	$(call gb_Helper_optional,LIBLANGTAG,liblangtag) \
	libmspub \
	liborcus \
	librelogo \
	libvisio \
	libwpd \
	libwpg \
	libwps \
	libxmlsec \
	lingucomponent \
	linguistic \
	lotuswordpro \
	$(call gb_Helper_optional,LPSOLVE,lpsolve) \
	$(call gb_Helper_optional,DESKTOP,l10ntools) \
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
	sot \
	starmath \
	stoc \
	store \
	svl \
	svtools \
	svgio \
	svx \
	sw \
	swext \
	test \
	testtools \
	$(call gb_Helper_optional,TOMCAT,tomcat) \
	toolkit \
	tools \
	touch \
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
))

ifeq ($(MERGELIBS),TRUE)
$(eval $(call gb_Module_add_targets,tail_build,\
	Library_merged \
))
endif

# Especially when building everything with symbols, the linking of the largest
# libraries takes enormous amounts of RAM.	To prevent annoying OOM situations
# etc., try to prevent linking these in parallel by adding artificial build
# order dependencies here.
define tailbuild_serialize1
$(call gb_LinkTarget_get_target,$(call gb_Library_get_linktargetname,$(1))) \
	:| $(foreach lib,$(2),$(call gb_Library_get_target,$(lib)))
endef

define tailbuild_serialize
$(if $(filter-out 0 1,$(words $(1))),\
$(call tailbuild_serialize1,$(firstword $(1)),$(wordlist 2,$(words $(1)),$(1))))
$(if $(strip $(1)),\
$(call tailbuild_serialize,$(wordlist 2,$(words $(1)),$(1))))
endef

ifeq (all,$(filter all,$(MAKECMDGOALS)))
$(eval $(call tailbuild_serialize,\
	scfilt \
	$(if $(filter SCRIPTING,$(BUILD_TYPE)),vbaobj) \
	sc msword swui sw sd \
	$(if $(filter DBCONNECTIVITY,$(BUILD_TYPE)),dbu) \
	writerfilter cui chartcontroller oox \
	$(if $(filter TRUE,$(MERGELIBS)),merged,svxcore) \
	vcl xo \
))
endif

# vim: set noet sw=4 ts=4:
