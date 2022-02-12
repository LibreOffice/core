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

# This makefile needs to be read first because some variables like T_LIBS
# for libmerged are cleared there and then later we append stuff to it
# through e.g. gb_Library_use_external for various libraries in libmerged.
ifneq ($(MERGELIBS),)
$(eval $(call gb_Module_add_targets,libreoffice,\
	Library_merged \
))
endif

build check unitcheck slowcheck subsequentcheck uicheck: \
	$(call gb_Helper_optional,XMLSEC,Module_xmlsec) \
	$(call gb_Helper_optional,ABW,Module_libabw) \
	$(call gb_Helper_optional,BOOST,Module_boost) \
	$(call gb_Helper_optional,BOX2D,Module_box2d) \
	$(call gb_Helper_optional,BREAKPAD,Module_breakpad) \
	$(call gb_Helper_optional,BSH,Module_beanshell) \
	$(call gb_Helper_optional,BZIP2,Module_bzip2) \
	$(call gb_Helper_optional,CAIRO,Module_cairo) \
	$(call gb_Helper_optional,CDR,Module_libcdr) \
	$(call gb_Helper_optional,OPENCL,Module_clew) \
	$(call gb_Helper_optional,CLUCENE,Module_clucene) \
	$(call gb_Helper_optional,LIBCMIS,Module_libcmis) \
	$(call gb_Helper_optional,COINMP,Module_coinmp) \
	$(call gb_Helper_optional,CPPUNIT,Module_cppunit) \
	$(call gb_Helper_optional,CT2N,Module_ct2n) \
	$(call gb_Helper_optional,CURL,Module_curl) \
	Module_dtoa \
	$(call gb_Helper_optional,EBOOK,Module_libebook) \
	$(call gb_Helper_optional,EPM,Module_epm) \
	$(call gb_Helper_optional,EPOXY,Module_epoxy) \
	$(call gb_Helper_optional,EPUBGEN,Module_libepubgen) \
	$(call gb_Helper_optional,ETONYEK,Module_libetonyek) \
	$(call gb_Helper_optional,EXPAT,Module_expat) \
	$(call gb_Helper_optional,FIREBIRD,Module_firebird) \
	$(call gb_Helper_optional,FONTCONFIG,Module_fontconfig) \
	$(call gb_Helper_optional,FREEHAND,Module_libfreehand) \
	$(call gb_Helper_optional,FREETYPE,Module_freetype) \
	$(call gb_Helper_optional,GLM,Module_glm) \
	$(call gb_Helper_optional,GPGMEPP,Module_gpgmepp) \
	$(call gb_Helper_optional,GRAPHITE,Module_graphite) \
	$(call gb_Helper_optional,HARFBUZZ,Module_harfbuzz) \
	$(call gb_Helper_optional,HSQLDB,Module_hsqldb) \
	$(call gb_Helper_optional,HUNSPELL,Module_hunspell) \
	$(call gb_Helper_optional,HYPHEN,Module_hyphen) \
	$(call gb_Helper_optional,ICU,Module_icu) \
	$(call gb_Helper_optional,JFREEREPORT,Module_jfreereport) \
	$(call gb_Helper_optional,LIBJPEG_TURBO,Module_libjpeg-turbo) \
	$(call gb_Helper_optional,LANGUAGETOOL,Module_languagetool) \
	$(call gb_Helper_optional,LCMS2,Module_lcms2) \
	$(call gb_Helper_optional,LIBASSUAN,Module_libassuan) \
	$(call gb_Helper_optional,LIBATOMIC_OPS,Module_libatomic_ops) \
	$(call gb_Helper_optional,LIBEOT,Module_libeot) \
	$(call gb_Helper_optional,LIBEXTTEXTCAT,Module_libexttextcat) \
	$(call gb_Helper_optional,LIBFFI,Module_libffi) \
	$(call gb_Helper_optional,LIBGPGERROR,Module_libgpg-error) \
	$(call gb_Helper_optional,LIBLANGTAG,Module_liblangtag) \
	$(call gb_Helper_optional,LIBNUMBERTEXT,Module_libnumbertext) \
	$(call gb_Helper_optional,LIBPNG,Module_libpng) \
	$(call gb_Helper_optional,LIBWEBP,Module_libwebp) \
	$(call gb_Helper_optional,LIBXML2,Module_libxml2) \
	$(call gb_Helper_optional,LIBXSLT,Module_libxslt) \
	$(call gb_Helper_optional,LPSOLVE,Module_lpsolve) \
	$(call gb_Helper_optional,LIBTOMMATH,Module_libtommath) \
	$(call gb_Helper_optional,LXML,Module_lxml) \
	$(call gb_Helper_optional,MARIADB_CONNECTOR_C,Module_mariadb-connector-c) \
	$(call gb_Helper_optional,MDDS,Module_mdds) \
	$(call gb_Helper_optional,MDNSRESPONDER,Module_mdnsresponder) \
	$(if $(WITH_EXTRA_EXTENSIONS),Module_misc_extensions) \
	$(call gb_Helper_optional,MORE_FONTS,Module_more_fonts) \
	$(call gb_Helper_optional,MSPUB,Module_libmspub) \
	$(call gb_Helper_optional,MWAW,Module_libmwaw) \
	$(call gb_Helper_optional,MYTHES,Module_mythes) \
	$(call gb_Helper_optional,NSS,Module_nss) \
	$(call gb_Helper_optional,ODFGEN,Module_libodfgen) \
	$(call gb_Helper_optional,OPENLDAP,Module_openldap) \
	$(call gb_Helper_optional,OPENSSL,Module_openssl) \
	$(call gb_Helper_optional,ORCUS,Module_liborcus) \
	$(call gb_Helper_optional,PAGEMAKER,Module_libpagemaker) \
	$(call gb_Helper_optional,PDFIUM,Module_pdfium) \
	$(call gb_Helper_optional,POPPLER,Module_poppler) \
	$(call gb_Helper_optional,POSTGRESQL,Module_postgresql) \
	$(call gb_Helper_optional,PYTHON,Module_python3) \
	$(call gb_Helper_optional,QXP,Module_libqxp) \
	$(call gb_Helper_optional,ZXING,Module_zxing) \
	$(call gb_Helper_optional,REDLAND,Module_redland) \
	$(call gb_Helper_optional,REVENGE,Module_librevenge) \
	$(call gb_Helper_optional,RHINO,Module_rhino) \
	$(call gb_Helper_optional,SKIA,Module_skia) \
	$(call gb_Helper_optional,STAROFFICE,Module_libstaroffice) \
	$(if $(filter WNT,$(OS)),Module_twain_dsm) \
	$(call gb_Helper_optional,UCPP,Module_ucpp) \
	$(call gb_Helper_optional,VISIO,Module_libvisio) \
	$(call gb_Helper_optional,WPD,Module_libwpd) \
	$(call gb_Helper_optional,WPG,Module_libwpg) \
	$(call gb_Helper_optional,WPS,Module_libwps) \
	$(call gb_Helper_optional,XSLTML,Module_xsltml) \
	$(call gb_Helper_optional,ZLIB,Module_zlib) \
	$(call gb_Helper_optional,ZMF,Module_libzmf) \
	$(call gb_Helper_optional,CUCKOO,Module_cuckoo) \

# WASM_CHART change
ifneq ($(ENABLE_WASM_STRIP_CHART),TRUE)
$(eval $(call gb_Module_add_moduledirs,libreoffice,\
	chart2 \
))
endif

# WASM_CANVAS change
ifneq ($(ENABLE_WASM_STRIP_CANVAS),TRUE)
$(eval $(call gb_Module_add_moduledirs,libreoffice,\
	canvas \
	cppcanvas \
))
endif

ifneq ($(ENABLE_WASM_STRIP_DBACCESS),TRUE)
$(eval $(call gb_Module_add_moduledirs,libreoffice,\
	dbaccess \
))
endif

ifneq ($(ENABLE_WASM_STRIP_ACCESSIBILITY),TRUE)
$(eval $(call gb_Module_add_moduledirs,libreoffice,\
	accessibility \
	winaccessibility \
))
endif

$(eval $(call gb_Module_add_moduledirs,libreoffice,\
	android \
    $(if $(ENABLE_WASM_STRIP_BASIC_CALC_DRAW_MATH_IMPRESS),, \
	animations \
    ) \
	apple_remote \
	avmedia \
    $(if $(ENABLE_WASM_STRIP_BASIC_CALC_DRAW_MATH_IMPRESS),, \
	basctl \
    ) \
	basegfx \
	basic \
	bean \
	binaryurp \
	bridges \
	cli_ure \
    codemaker \
	comphelper \
	configmgr \
	connectivity \
	cppu \
	cppuhelper \
	cpputools \
	cui \
	desktop \
	$(call gb_Helper_optional,DICTIONARIES,dictionaries) \
	drawinglayer \
	editeng \
	embeddedobj \
	embedserv \
	eventattacher \
	extensions \
	external \
	extras \
	filter \
	$(call gb_Helper_optional,DBCONNECTIVITY,forms) \
	formula \
	$(call gb_Helper_optional,DESKTOP,fpicker) \
	framework \
    $(call gb_Helper_optionals_or,HELPTOOLS XMLHELP,helpcompiler) \
	$(call gb_Helper_optional,HELP,helpcontent2) \
	hwpfilter \
	i18nlangtag \
	i18npool \
	i18nutil \
	idl \
	$(call gb_Helper_optional,DESKTOP,idlc) \
	instsetoo_native \
	io \
	javaunohelper \
	jurt \
	jvmaccess \
	jvmfwk \
	$(call gb_Helper_optional,LIBRELOGO,librelogo) \
	libreofficekit \
	lingucomponent \
	linguistic \
	lotuswordpro \
	$(call gb_Helper_optional,DESKTOP,l10ntools) \
	$(call gb_Helper_optional,NLPSOLVER,nlpsolver) \
	o3tl \
	$(call gb_Helper_optional,ODK,odk) \
	offapi \
	officecfg \
	onlineupdate \
	oovbaapi \
	oox \
	$(call gb_Helper_optional,OPENCL,opencl) \
	package \
	pch \
	postprocess \
	$(call gb_Helper_optional,PYUNO,pyuno) \
	$(call gb_Helper_optional,QADEVOOO,qadevOOo) \
	readlicense_oo \
	registry \
	remotebridges \
	reportbuilder \
	$(call gb_Helper_optional,DBCONNECTIVITY,reportdesign) \
	ridljar \
	sal \
	salhelper \
	sax \
    $(if $(ENABLE_WASM_STRIP_BASIC_CALC_DRAW_MATH_IMPRESS),, \
	sc \
	scaddins \
	sccomp \
    ) \
	$(call gb_Helper_optional,DESKTOP,scp2) \
	scripting \
    $(if $(ENABLE_WASM_STRIP_BASIC_CALC_DRAW_MATH_IMPRESS),, \
	sd \
	sdext \
    ) \
	$(call gb_Helper_optional,DESKTOP,setup_native) \
	sfx2 \
	shell \
    $(if $(ENABLE_WASM_STRIP_BASIC_CALC_DRAW_MATH_IMPRESS),, \
	slideshow \
    ) \
	smoketest \
	solenv \
	soltools \
	sot \
    $(if $(ENABLE_WASM_STRIP_BASIC_CALC_DRAW_MATH_IMPRESS),, \
	starmath \
    ) \
    $(if $(ENABLE_CUSTOMTARGET_COMPONENTS),static) \
	stoc \
	store \
	svl \
	svtools \
	svgio \
	emfio \
	svx \
	sw \
	swext \
	sysui \
	test \
	testtools \
	toolkit \
	tools \
	ucb \
	ucbhelper \
	udkapi \
	$(call gb_Helper_optional,PYUNO,uitest) \
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
	wizards \
	writerfilter \
	writerperfect \
	xmerge \
    $(call gb_Helper_optional,XMLHELP,xmlhelp) \
	xmloff \
	xmlreader \
	xmlscript \
	xmlsecurity \
))

ifeq (,$(DISABLE_DYNLOADING))
# Especially when building everything with symbols, the linking of the largest
# libraries takes enormous amounts of RAM.	To prevent annoying OOM situations
# etc., try to prevent linking these in parallel by adding artificial build
# order dependencies here.
define repositorymodule_serialize1
$(call gb_Library_get_linktarget_target,$(1)) :| $(foreach lib,$(2),$(call gb_Library_get_target,$(lib)))
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
# the default goal is build (see Module.mk)
ifeq (,$(filter-out build check unitcheck slowcheck screenshot subsequentcheck uicheck,$(MAKECMDGOALS)))
$(eval $(call repositorymodule_serialize,\
	scfilt \
	$(call gb_Helper_optional,SCRIPTING,vbaobj) \
	sc msword \
	$(call gb_Helper_optional,DESKTOP,swui) \
	sw sd \
	$(call gb_Helper_optional,DBCONNECTIVITY,dbu) \
	writerfilter cui chartcontroller chartcore oox \
	$(if $(MERGELIBS), merged, \
		svx svxcore xo sfx fwk svt vcl) \
))
endif
endif # !$(DISABLE_DYNLOADING)

# vim: set noet sw=4 ts=4:
