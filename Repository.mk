#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



$(eval $(call gb_Helper_register_repository,SRCDIR))


$(eval $(call gb_Helper_register_executables,NONE, \
    bmp \
    bmpsum \
    checksingleton \
    g2g \
    mkunroll \
    rdbedit \
    rdbmaker \
    rsc \
    rsc2 \
    rscdep \
    rscpp \
    so_checksum \
    srvdepy \
    svidl \
    sspretty \
    xml2cmp \
))

$(eval $(call gb_Helper_register_executables,OOO, \
    regsingleton \
    sp2bv \
    spadmin.bin \
))

$(eval $(call gb_Helper_register_executables,SDK, \
    autodoc \
    cppumaker \
    idlc \
    javamaker \
    regcompare \
    uno-skeletonmaker \
))

$(eval $(call gb_Helper_register_executables,UREBIN, \
    javaldx \
    regcomp \
    regmerge \
    regview \
    uno \
))

$(eval $(call gb_Helper_register_libraries,OOOLIBS, \
    AppleRemote \
    MacOSXSpell \
    acc \
    adabas \
    adabasui \
    agg \
    analysis \
    animcore \
    avmedia \
    avmediagst \
    avmediaMacAVF \
    avmediaQuickTime \
    avmediawin \
    basctl \
    basebmp \
    basegfx \
    cached1 \
    canvastools \
    communi \
    cppcanvas \
    ctl \
    cui \
    date \
    dba \
    dbaxml \
    dbmm \
    dbui \
    dbpool \
    dbtools \
    desktop_detector \
    dnd \
    drawinglayer \
    dtrans \
    svgio \
    editeng \
    embobj \
    emboleobj \
    evtatt \
    file \
    fileacc \
    fop \
    for \
    forui \
    fps \
    ftransl \
    fwe \
    fwi \
    fwk \
    fwl \
    fwm \
    guesslang \
    i18npaper \
    java_uno_accessbridge \
    juh \
    juhx \
    lng \
    mcnttype \
    msfilter \
    msword \
    odbc \
    odbcbase \
    oox \
    package2 \
    qstart_gtk \
    rpt \
    rptui \
    rptxml \
    sax \
    sb \
    sc \
    scd \
    scfilt \
    scui \
    sdbc \
    sdbt \
    sfx \
    simplecm \
    sm \
    smd \
    solver \
    sot \
    spa \
    sts \
    svl \
    svt \
    svx \
    svxcore \
    sw \
    swd \
    swui \
    sd \
    sdd \
    sdui \
    sdfilt \
    srtrs1 \
    sysdtran \
    sysdtrans \
    textconversiondlgs \
    ootk \
    tl \
    tvhlp1 \
    ucpchelp1 \
    utl \
    unordf \
    unoxml \
    vbahelper \
    writerfilter \
    ucb1 \
    ucpdav1 \
    ucpfile1 \
    ucpftp1 \
    ucphier1 \
    ucppkg1 \
    vcl \
    vclplug_gen \
    vclplug_gtk \
    vclplug_kde \
    vclplug_kde4 \
    vclplug_svp \
    uui \
    xcr \
    xo \
    xof \
    xstor \
))

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_URE, \
    gcc3_uno \
    java_uno \
    jpipe \
    jpipx \
    msci_uno \
    mscx_uno \
    s5abi_uno \
    sunjavaplugin \
    sunpro5_uno \
    xmlreader \
))

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
    officebean \
))


$(eval $(call gb_Helper_register_libraries,RTLIBS, \
    comphelper \
    i18nisolang1 \
    i18nutil \
    ucbhelper \
    vos3 \
))

$(eval $(call gb_Helper_register_libraries,RTVERLIBS, \
    cppuhelper \
    jvmaccess \
    salhelper \
))

$(eval $(call gb_Helper_register_libraries,STLLIBS, \
    stl \
))

$(eval $(call gb_Helper_register_libraries,UNOLIBS_OOO, \
    OGLTrans \
    acceptor \
    binaryurp \
    cairocanvas \
    canvasfactory \
    configmgr \
    connector \
    directx5canvas \
    directx9canvas \
    expwrap \
    fastsax \
    fpicker \
    fps_aqua \
    fps_gnome \
    fps_kde4 \
    fps_office \
    fsstorage \
    gdipluscanvas \
    hatchwindowfactory \
    hyphen_uno \
    lnth \
    nullcanvas \
    passwordcontainer \
    simplecanvas \
    slideshow \
    spell \
    streams \
    textinstream \
    textoutstream \
    vbaswobj \
    msforms \
    vclcanvas \
    ucpext \
    ucpexpand1 \
    ucpgio1 \
    ucpgvfs1 \
    ucptdoc1 \
    uuresolver \
    vbaobj \
    writerfilter_debug \
))


$(eval $(call gb_Helper_register_libraries,UNOVERLIBS, \
    cppu \
    jvmfwk \
    reg \
    sal \
    store \
))

$(eval $(call gb_Helper_register_static_libraries,PLAINLIBS, \
    basegfx_s \
    codemaker \
    commoncpp \
    commonjava \
    cosv \
    ooopathutils \
    salcpprt \
    sample \
    sldshw_s \
    vclmain \
    udm \
    ulingu \
))

# vim: set noet sw=4 ts=4:
