# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Pagein_Pagein,common))

ifneq ($(ENABLE_WASM_STRIP_CLUCENE),TRUE)
$(eval $(call gb_Pagein_add_objects,common,\
    helplinker \
))
endif

# sorted in approx. reverse load order (ld.so.1)
$(eval $(call gb_Pagein_add_objects,common,\
    $(if $(MERGELIBS),merged) \
    i18nlangtag \
    $(if $(SYSTEM_ICU),,\
        libicui18n$(gb_Library_DLLEXT).$(ICU_MAJOR) \
        libicuuc$(gb_Library_DLLEXT).$(ICU_MAJOR) \
    ) \
    lng \
    xo \
    fwk \
    package2 \
    ucpfile1 \
    ucb1 \
    configmgr \
    vclplug_gen \
    $(if $(findstring TRUE,$(ENABLE_GTK3)),vclplug_gtk3) \
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
    types.rdb \
    services/services.rdb \
    types/oovbaapi.rdb \
    deployment \
    deploymentmisc \
    xstor \
    filterconfig \
    uui \
    svt \
    spl \
    avmedia \
    sax \
    fsstorage \
    desktopbe1 \
    localebe1 \
    ucpexpand1 \
    sfx \
    sofficeapp \
))

# vim: set ts=4 sw=4 et:
