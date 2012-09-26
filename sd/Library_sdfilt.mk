# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2010 Red Hat, Inc., David Tardon <dtardon@redhat.com>
#  (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Library_Library,sdfilt))

$(eval $(call gb_Library_set_include,sdfilt,\
    $$(INCLUDE) \
    -I$(SRCDIR)/sd/inc \
))

$(eval $(call gb_Library_add_defs,sdfilt,\
    -DSD_DLLIMPLEMENTATION \
))

ifneq ($(strip $(dbg_anim_log)$(DBG_ANIM_LOG)),)
$(eval $(call gb_Library_add_defs,sdfilt,\
    -DDBG_ANIM_LOG \
))
endif

$(eval $(call gb_Library_use_sdk_api,sdfilt))

$(eval $(call gb_Library_use_libraries,sdfilt,\
    comphelper \
    cppu \
    cppuhelper \
    editeng \
    i18nisolang1 \
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
	$(gb_UWINAPI) \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_set_componentfile,sdfilt,sd/util/sdfilt))

$(eval $(call gb_Library_add_exception_objects,sdfilt,\
    sd/source/filter/eppt/eppt \
    sd/source/filter/eppt/epptso \
    sd/source/filter/eppt/escherex \
    sd/source/filter/eppt/pptexanimations \
    sd/source/filter/eppt/pptexsoundcollection \
    sd/source/filter/eppt/pptx-epptbase \
    sd/source/filter/eppt/pptx-epptooxml \
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
