# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

fuzzer_externals = \
    boost_headers \
    curl \
    harfbuzz \
    graphite \
    cairo \
    fontconfig \
    freetype \
    icui18n \
    icuuc \
    icudata \
    lcms2 \
    librdf \
    libxslt \
    libxml2 \
    jpeg \
    png \
    clew \
    openssl \
    expat \
    mythes \
    hyphen \
    hunspell \
    zlib \

fuzzer_libraries = \
    basctl \
    merged \
    cui \
    chartcontroller \
    chartcore \
    sm \
    gie \
    oox \
    reflection \
    odfflatxml \
    invocadapt \
    bootstrap \
    introspection \
    stocservices \
    lnth \
    hyphen \
    i18nsearch \
    embobj \
    evtatt \
    unordf \
    ucphier1 \
    ucptdoc1 \
    srtrs1 \
    storagefd \
    mtfrenderer \
    canvasfactory \
    vclcanvas \
    xof \
    xmlfa \
    xmlfd \
    cppu \
    cppuhelper \
    comphelper \
    i18nlangtag \
    xmlreader \
    unoidl \
    reg \
    store \
    expwrap \
    gcc3_uno \
    salhelper \
    sal \
