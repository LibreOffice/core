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
# Copyright (C) 2011 Red Hat, Inc., David Tardon <dtardon@redhat.com>
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

$(eval $(call gb_Pagein_Pagein,common))

# sorted in approx. reverse load order (ld.so.1)
$(eval $(call gb_Pagein_add_objects,common,\
    $(if $(MERGELIBS),merged) \
    $(if $(URELIBS),urelibs) \
    i18nlangtag \
    $(if $(findstring YES,$(SYSTEM_ICU)),,\
        icui18n \
        icule \
        icuuc \
    ) \
    lng \
    xo \
    fwe \
    fwk \
    fwi \
    fwl \
    package2 \
    ucpfile1 \
    ucb1 \
    configmgr \
    vclplug_gen \
    $(if $(findstring TRUE,$(ENABLE_GTK)),vclplug_gtk) \
    $(if $(findstring TRUE,$(ENABLE_TDE)),vclplug_tde) \
    $(if $(findstring TRUE,$(ENABLE_KDE)),vclplug_kde) \
    basegfx \
    sot \
    xmlscript \
    sb \
    stocservices \
    bootstrap \
    reg \
    store \
    reflection \
    cppuhelper \
    cppu \
    sal \
    ucbhelper \
    comphelper \
    tl \
    utl \
    svl \
    vcl \
    tk \
    ../ure-link/share/misc/types.rdb \
    services.rdb \
    oovbaapi.rdb \
    deployment \
    deploymentmisc \
    xstor \
    filterconfig \
    uui \
    svt \
    spl \
    avmedia \
    helplinker \
    sax \
    gconfbe1 \
    fsstorage \
    desktopbe1 \
    localebe1 \
    ucpexpand1 \
    sfx \
    sofficeapp \
))

# vim: set ts=4 sw=4 et:
