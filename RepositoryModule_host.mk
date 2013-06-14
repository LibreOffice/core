# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#


$(eval $(call gb_Module_Module,libreoffice))

$(eval $(call gb_Module_add_moduledirs,libreoffice,\
	accessibility \
	$(call gb_Helper_optional,AFMS,afms) \
	android \
	animations \
	$(call gb_Helper_optional,APACHE_COMMONS,apache-commons) \
	apple_remote \
	avmedia \
	basctl \
	basebmp \
	basegfx \
	basic \
	bean \
	$(call gb_Helper_optional,BOOST,boost) \
	$(call gb_Helper_optional,BSH,beanshell) \
	binaryurp \
	bridges \
	$(call gb_Helper_optional,CAIRO,cairo) \
	canvas \
	chart2 \
	cli_ure \
	$(call gb_Helper_optional,CLUCENE,clucene) \
	$(call gb_Helper_optional,DESKTOP,codemaker) \
	comphelper \
	configmgr \
	$(call gb_Helper_optional,DBCONNECTIVITY,connectivity) \
	cppcanvas \
	cppu \
	cppuhelper \
	$(call gb_Helper_optional,CPPUNIT,cppunit) \
	cpputools \
	$(call gb_Helper_optional,CRASHREP,crashrep) \
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
	$(call gb_Helper_optional,EXPAT,expat) \
	extensions \
	external \
	extras \
	fileaccess \
	filter \
	$(call gb_Helper_optional,LIBATOMIC_OPS,libatomic_ops) \
	$(call gb_Helper_optional,FIREBIRD,firebird) \
	$(call gb_Helper_optional,FONTCONFIG,fontconfig) \
	$(call gb_Helper_optional,DBCONNECTIVITY,forms) \
	formula \
	fpicker \
	framework \
	$(call gb_Helper_optional,FREETYPE,freetype) \
	$(call gb_Helper_optional,GRAPHITE,graphite) \
	$(call gb_Helper_optional,HARFBUZZ,harfbuzz) \
	$(call gb_Helper_optional,DESKTOP,helpcompiler) \
	$(call gb_Helper_optional,HELP,helpcontent2) \
	$(call gb_Helper_optional,HSQLDB,hsqldb) \
	$(call gb_Helper_optional,HUNSPELL,hunspell) \
	hwpfilter \
	$(call gb_Helper_optional,HYPHEN,hyphen) \
	i18nlangtag \
	i18npool \
	i18nutil \
	$(call gb_Helper_optional,ICU,icu) \
	idl \
	$(call gb_Helper_optional,DESKTOP,idlc) \
	instsetoo_native \
	io \
	javaunohelper \
	$(call gb_Helper_optional,JFREEREPORT,jfreereport) \
	$(call gb_Helper_optional,JPEG,jpeg) \
	jurt \
	jvmaccess \
	jvmfwk \
	$(call gb_Helper_optional,LANGUAGETOOL,languagetool) \
	$(call gb_Helper_optional,LCMS2,lcms2) \
	$(call gb_Helper_optional,CDR,libcdr) \
	$(call gb_Helper_optional,CMIS,libcmis) \
	$(call gb_Helper_optional,LIBEXTTEXTCAT,libexttextcat) \
	$(call gb_Helper_optional,LIBLANGTAG,liblangtag) \
	$(call gb_Helper_optional,LIBPNG,libpng) \
	$(call gb_Helper_optional,MSPUB,libmspub) \
	$(call gb_Helper_optional,MWAW,libmwaw) \
	$(call gb_Helper_optional,ODFGEN,libodfgen) \
	$(call gb_Helper_optional,ORCUS,liborcus) \
	librelogo \
	$(call gb_Helper_optional,VISIO,libvisio) \
	$(call gb_Helper_optional,WPD,libwpd) \
	$(call gb_Helper_optional,WPG,libwpg) \
	$(call gb_Helper_optional,WPS,libwps) \
	$(call gb_Helper_optional,LIBXML2,libxml2) \
	libxmlsec \
	$(call gb_Helper_optional,LIBXSLT,libxslt) \
	lingucomponent \
	linguistic \
	lotuswordpro \
	$(call gb_Helper_optional,LPSOLVE,lpsolve) \
	$(call gb_Helper_optional,DESKTOP,l10ntools) \
	$(call gb_Helper_optional,MARIADB,libmariadb) \
	$(call gb_Helper_optional,MDDS,mdds) \
	$(call gb_Helper_optional,MORE_FONTS,more_fonts) \
	$(call gb_Helper_optional,MOZ,moz) \
	$(call gb_Helper_optional,MARIADBC,mysqlc) \
	$(call gb_Helper_optional,MYSQLCPPCONN,mysqlcppconn) \
	$(call gb_Helper_optional,MYTHES,mythes) \
	$(call gb_Helper_optional,NEON,neon) \
	$(call gb_Helper_optional,NLPSOLVER,nlpsolver) \
	np_sdk \
	$(call gb_Helper_optional,NSS,nss) \
	o3tl \
	$(call gb_Helper_optional,ODK,odk) \
	offapi \
	officecfg \
	oovbaapi \
	oox \
	$(call gb_Helper_optional,OPENLDAP,openldap) \
	$(call gb_Helper_optional,OPENSSL,openssl) \
	package \
	padmin \
	$(call gb_Helper_optional,POPPLER,poppler) \
	$(call gb_Helper_optional,POSTGRESQL,postgresql) \
	postprocess \
	psprint_config \
	$(call gb_Helper_optional,PYTHON,python3) \
	$(call gb_Helper_optional,PYUNO,pyuno) \
	$(call gb_Helper_optional,QADEVOOO,qadevOOo) \
	readlicense_oo \
	$(call gb_Helper_optional,REDLAND,redland) \
	registry \
	remotebridges \
	reportbuilder \
	$(call gb_Helper_optional,DBCONNECTIVITY,reportdesign) \
	$(call gb_Helper_optional,RHINO,rhino) \
	ridljar \
	rsc \
	sal \
	salhelper \
	sax \
	sc \
	scaddins \
	sccomp \
	$(call gb_Helper_optional,DESKTOP,scp2) \
	scripting \
	sd \
	sdext \
	$(call gb_Helper_optional,DESKTOP,setup_native) \
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
	svl \
	svtools \
	svgio \
	svx \
	sw \
	swext \
	sysui \
	test \
	testtools \
	$(call gb_Helper_optional,TOMCAT,tomcat) \
	toolkit \
	tools \
	touch \
	tubes \
	ucb \
	ucbhelper \
	$(call gb_Helper_optional,UCPP,ucpp) \
	udkapi \
	UnoControls \
	unodevtools \
	unoil \
	unoidl \
	unotest \
	unotools \
	unoxml \
	ure \
	uui \
	vbahelper \
	vcl \
	$(call gb_Helper_optional,VIGRA,vigra) \
	wizards \
	writerfilter \
	writerperfect \
	xmerge \
	$(call gb_Helper_optional,DESKTOP,xmlhelp) \
	xmloff \
	xmlreader \
	xmlscript \
	xmlsecurity \
	$(call gb_Helper_optional,XSLTML,xsltml) \
	$(call gb_Helper_optional,ZLIB,zlib) \
))

ifneq ($(MERGELIBS),)
$(eval $(call gb_Module_add_targets,libreoffice,\
	Library_merged \
	$(if $(URELIBS),Library_urelibs) \
))
endif

# Especially when building everything with symbols, the linking of the largest
# libraries takes enormous amounts of RAM.	To prevent annoying OOM situations
# etc., try to prevent linking these in parallel by adding artificial build
# order dependencies here.
define repositorymodule_serialize1
$(call gb_LinkTarget_get_target,$(call gb_Library_get_linktargetname,$(1))) \
	:| $(foreach lib,$(2),$(call gb_Library_get_target,$(lib)))
endef

define repositorymodule_serialize
$(if $(filter-out 0 1,$(words $(1))),\
$(call repositorymodule_serialize1,$(firstword $(1)),$(wordlist 2,$(words $(1)),$(1))))
$(if $(strip $(1)),\
$(call repositorymodule_serialize,$(wordlist 2,$(words $(1)),$(1))))
endef

# DO NOT SORT ALPHABETICALLY: the libraries must be listed in dependency order,
# otherwise cyclic dependencies ruin everything.
# do not serialize on a partial build as that may fail due to missing deps.
# the default goal is all (see Module.mk)
ifeq (,$(filter-out all,$(MAKECMDGOALS)))
$(eval $(call repositorymodule_serialize,\
	scfilt \
	$(if $(filter SCRIPTING,$(BUILD_TYPE)),vbaobj) \
	sc msword swui sw sd \
	$(if $(filter DBCONNECTIVITY,$(BUILD_TYPE)),dbu) \
	writerfilter cui chartcontroller chartcore oox svx \
	$(if $(MERGELIBS),merged,svxcore) \
	xo sfx fwk svt vcl \
))
endif

# vim: set noet sw=4 ts=4:
