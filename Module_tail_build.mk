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


$(eval $(call gb_Module_Module,tail_end))

$(eval $(call gb_Module_add_moduledirs,tail_end,\
    accessibility \
    animations \
    apple_remote \
	autodoc \
    avmedia \
    basctl \
    basebmp \
	basegfx \
    basic \
    bean \
	binaryurp \
	bluez_bluetooth \
    canvas \
    chart2 \
    configmgr \
    connectivity \
	cosv \
    cppcanvas \
	cpputools \
    cui \
    dbaccess \
    desktop \
	$(if $(filter DICTIONARIES,$(BUILD_TYPE)),\
		dictionaries \
	) \
    dtrans \
    drawinglayer \
    editeng \
    embeddedobj \
	embedserv \
    eventattacher \
    extensions \
	extras \
    fileaccess \
    filter \
    forms \
    formula \
    fpicker \
    framework \
	$(if $(filter HSQLDB,$(BUILD_TYPE)),\
		hsqldb \
	) \
    hwpfilter \
	i18npool \
	i18nutil \
    idl \
	io \
	javaunohelper \
	libcdr \
	libcmis \
	libmspub \
	libvisio \
	libwpd \
	libwpg \
	libwps \
    lingucomponent \
    linguistic \
    lotuswordpro \
    MathMLDTD \
	mdds \
    Mesa \
	$(if $(filter NLPSOLVER,$(BUILD_TYPE)),\
		nlpsolver \
	) \
    np_sdk \
	o3tl \
    oovbaapi \
    oox \
    package \
    padmin \
    psprint_config \
    $(if $(filter PYUNO,$(BUILD_TYPE)),\
		pyuno \
	) \
	$(if $(filter QADEVOOO,$(BUILD_TYPE)),\
		qadevOOo \
	) \
	regexp \
	remotebridges \
    reportbuilder \
    reportdesign \
    rsc \
    sane \
	sax \
    sc \
    scaddins \
    sccomp \
	$(if $(filter DESKTOP,$(BUILD_TYPE)),\
		scp2 \
	) \
    scripting \
    sd \
    sdext \
    sfx2 \
    shell \
    slideshow \
    smoketest \
    sot \
    starmath \
    svl \
    svtools \
    svx \
    sw \
    swext \
    test \
    testtools \
    toolkit \
	tools \
	touch \
    tubes \
    twain \
    ucb \
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
	$(if $(filter X11_EXTENSIONS,$(BUILD_TYPE)), \
		x11_extensions \
	) \
    xmerge \
	$(if $(filter DESKTOP,$(BUILD_TYPE)), \
	    xmlhelp) \
    xmloff \
    xmlscript \
    xmlsecurity \
))

ifeq ($(MERGELIBS),TRUE)
$(eval $(call gb_Module_add_targets,tail_end,\
	Library_merged \
))
endif

# Especially when building everything with symbols, the linking of the largest
# libraries takes enormous amounts of RAM.  To prevent annoying OOM situations
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
