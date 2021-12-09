# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,sdfilt))

$(eval $(call gb_Library_set_include,sdfilt,\
    $$(INCLUDE) \
    -I$(SRCDIR)/sd/inc \
    -I$(SRCDIR)/sd/source/ui/inc \
))

ifneq ($(strip $(dbg_anim_log)$(DBG_ANIM_LOG)),)
$(eval $(call gb_Library_add_defs,sdfilt,\
    -DDBG_ANIM_LOG \
))
endif

$(eval $(call gb_Library_use_external,sdfilt,boost_headers))

$(eval $(call gb_Library_use_custom_headers,sdfilt,\
	oox/generated \
))

$(eval $(call gb_Library_use_sdk_api,sdfilt))

$(eval $(call gb_Library_use_libraries,sdfilt,\
    comphelper \
    cppu \
    cppuhelper \
    editeng \
    i18nlangtag \
    i18nutil \
    msfilter \
    oox \
    sal \
    sax \
    sd \
    sfx \
    sot \
    svl \
    svt \
    svxcore \
    tl \
    ucbhelper \
    utl \
    vcl \
    basegfx \
))

$(eval $(call gb_Library_set_componentfile,sdfilt,sd/util/sdfilt,services))

$(eval $(call gb_Library_use_common_precompiled_header,sdfilt))

$(eval $(call gb_Library_add_exception_objects,sdfilt,\
    sd/source/filter/eppt/eppt \
    sd/source/filter/eppt/epptso \
    sd/source/filter/eppt/escherex \
    sd/source/filter/eppt/pptexanimations \
    sd/source/filter/eppt/pptexsoundcollection \
    sd/source/filter/eppt/pptx-epptbase \
    sd/source/filter/eppt/pptx-epptooxml \
    sd/source/filter/eppt/pptx-animations \
    sd/source/filter/eppt/pptx-grouptable \
    sd/source/filter/eppt/pptx-stylesheet \
    sd/source/filter/eppt/pptx-text \
    sd/source/filter/ppt/ppt97animations \
    sd/source/filter/ppt/pptatom \
    sd/source/filter/ppt/pptin \
    sd/source/filter/ppt/pptinanimations \
    sd/source/filter/ppt/propread \
))

# vim: set noet sw=4 ts=4:
