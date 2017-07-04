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
    orcus \
    orcus-parser \
    boost_filesystem \
    boost_system \
    boost_iostreams \
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
    liblangtag \
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

fuzzer_core_libraries = \
    basctl \
    avmedia \
    basegfx \
    canvastools \
    comphelper \
    configmgr \
    cppcanvas \
    dbtools \
    deployment \
    deploymentmisc \
    drawinglayer \
    editeng \
    filterconfig \
    fsstorage \
    fwe \
    fwi \
    fwk \
    i18npool \
    i18nutil \
    lng \
    localebe1 \
    mcnttype \
    msfilter \
    package2 \
    sax \
    sb \
    spell \
    sfx \
    sofficeapp \
    sot \
    svl \
    svt \
    svx \
    svxcore \
    emboleobj \
    svgfilter \
    svgio \
    animcore \
    tk \
    tl \
    ucb1 \
    ucbhelper \
    ucpexpand1 \
    ucpfile1 \
    unoxml \
    utl \
    uui \
    vcl \
    opencl \
    xmlscript \
    xo \
    xstor \
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

fuzzer_calc_libraries = \
    analysis \
    date \
    pricing \
    scfilt \
    scd \
    vbaevents \
    sc \
    for \
    forui \

fuzzer_writer_libraries = \
    msword \
    sw \
    swd \
    writerfilter \
    textfd \

fuzzer_draw_libraries = \
    sdfilt \
    sd \
    sdd \
    icg \
