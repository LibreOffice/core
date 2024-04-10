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
    boost_locale \
    boost_filesystem \
    boost_system \
    boost_iostreams \
    curl \
    dtoa \
    harfbuzz \
    graphite \
    cairo \
    fontconfig \
    freetype \
    gpgmepp \
    icui18n \
    icuuc \
    icudata \
    lcms2 \
    librdf \
    libexttextcat \
    liblangtag \
    libxslt \
    libxml2 \
    libjpeg \
    libpng \
    libtiff \
    libwebp \
    openssl \
    expat \
    mythes \
    hyphen \
    hunspell \
    zlib \
    zxcvbn-c \
    argon2 \

fuzzer_statics = \
     findsofficepath \
     ulingu \

fuzzer_core_libraries = \
    avmedia \
    basegfx \
    $(call gb_Helper_optional,SCRIPTING, \
        basctl \
        basprov \
    ) \
    canvastools \
    configmgr \
    cppcanvas \
    ctl \
    dba \
    dbase \
    dbtools \
    deployment \
    deploymentmisc \
    docmodel \
    drawinglayer \
    drawinglayercore \
    editeng \
    emfio \
    file \
    filterconfig \
    fsstorage \
    fwk \
    i18npool \
    i18nutil \
    io \
    lng \
    localebe1 \
    msfilter \
    package2 \
    sax \
    sb \
    sdbc2 \
    $(call gb_Helper_optional,SCRIPTING, \
        scriptframe) \
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
    ucppkg1 \
    unoxml \
    utl \
    uui \
    vcl \
    xmlscript \
    xo \
    xstor \
    chartcontroller \
    chartcore \
    sm \
    oox \
    proxyfac \
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
    gcc3_uno \
    salhelper \
    sal \
    localedata_en \
    localedata_others \

fuzzer_calc_libraries = \
    analysis \
    date \
    pricing \
    scfilt \
    scd \
    $(call gb_Helper_optional,SCRIPTING, \
        vbaevents \
        vbahelper \
        vbaobj \
    ) \
    sc \
    for \
    forui \
    guesslang \

fuzzer_writer_libraries = \
    msword \
    sw \
    swd \
    sw_writerfilter \
    wpftwriter \
    textfd \
    guesslang \

fuzzer_draw_libraries = \
    sd \
    sdd \
    icg \
    guesslang \

fuzzer_math_libraries = \
    sm \
    guesslang \
