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

$(eval $(call gb_Module_add_moduledirs,libreoffice,\
	accessibility \
	android \
	animations \
	apple_remote \
	$(call gb_Helper_optional,AVMEDIA,avmedia) \
	basctl \
	basegfx \
	basic \
	bean \
	binaryurp \
	bridges \
	canvas \
	chart2 \
	cli_ure \
	$(call gb_Helper_optional,DESKTOP,codemaker) \
	comphelper \
	configmgr \
	connectivity \
	cppcanvas \
	cppu \
	cppuhelper \
	cpputools \
	cui \
	$(call gb_Helper_optional,DBCONNECTIVITY,dbaccess) \
	desktop \
	$(call gb_Helper_optional,DICTIONARIES,dictionaries) \
	dtrans \
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
	$(call gb_Helper_optional,DESKTOP,helpcompiler) \
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
	librelogo \
	libreofficekit \
	lingucomponent \
	linguistic \
	lotuswordpro \
	$(call gb_Helper_optional,DESKTOP,l10ntools) \
	$(call gb_Helper_optional,MARIADBC,mysqlc) \
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
	postprocess \
	$(call gb_Helper_optional,PYUNO,pyuno) \
	$(call gb_Helper_optional,QADEVOOO,qadevOOo) \
	readlicense_oo \
	registry \
	remotebridges \
	reportbuilder \
	$(call gb_Helper_optional,DBCONNECTIVITY,reportdesign) \
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
	winaccessibility \
	wizards \
	writerfilter \
	writerperfect \
	xmerge \
	$(call gb_Helper_optional,DESKTOP,xmlhelp) \
	xmloff \
	xmlreader \
	xmlscript \
	xmlsecurity \
))

# Especially when building everything with symbols, the linking of the largest
# libraries takes enormous amounts of RAM.	To prevent annoying OOM situations
# etc., try to prevent linking these in parallel by adding artificial build
# order dependencies here.
define repositorymodule_serialize1
$(call gb_LinkTarget_get_target,$(call gb_Library_get_linktarget,$(1))) \
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
ifeq (,$(filter-out all build check unitcheck slowcheck screenshot subsequentcheck uicheck,$(MAKECMDGOALS)))
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

# vim: set noet sw=4 ts=4:
