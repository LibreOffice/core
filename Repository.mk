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
    g2g \
    mkunroll \
    rscdep \
    so_checksum \
    svidl \
    sspretty \
))

$(eval $(call gb_Helper_register_executables,OOO, \
    spadmin.bin \
))

$(eval $(call gb_Helper_register_libraries,OOOLIBS, \
    AppleRemote \
    adabas \
    adabasui \
    agg \
    animcore \
    avmedia \
    basebmp \
    basegfx \
    canvastools \
    cppcanvas \
    cui \
    dba \
    dbaxml \
    dbmm \
    dbui \
    dbpool \
    dbtools \
    desktop_detector \
    drawinglayer \
    svgio \
    editeng \
    file \
    fileacc \
    for \
    forui \
    fwe \
    fwi \
    fwk \
    fwl \
    fwm \
    i18npaper \
    lng \
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
    sdbc \
    sdbt \
    sfx \
    sm \
    smd \
    sot \
    spa \
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
    textconversiondlgs \
    ootk \
    tl \
    utl \
    unordf \
    unoxml \
    vbahelper \
    writerfilter \
    vcl \
    vclplug_gen \
    vclplug_gtk \
    vclplug_kde \
    vclplug_kde4 \
    vclplug_svp \
    xcr \
    xo \
    xof \
    xstor \
))

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_URE, \
    xmlreader \
))

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
))


$(eval $(call gb_Helper_register_libraries,RTLIBS, \
    comphelper \
    i18nisolang1 \
    i18nutil \
    jvmaccess \
    ucbhelper \
    vos3 \
))

$(eval $(call gb_Helper_register_libraries,RTVERLIBS, \
    cppuhelper \
    salhelper \
))

$(eval $(call gb_Helper_register_libraries,STLLIBS, \
    stl \
))

$(eval $(call gb_Helper_register_libraries,UNOLIBS_OOO, \
    acceptor \
    cairocanvas \
    canvasfactory \
    connector \
    directx5canvas \
    directx9canvas \
    expwrap \
    fastsax \
    fsstorage \
    gdipluscanvas \
    hatchwindowfactory \
    nullcanvas \
    passwordcontainer \
    simplecanvas \
    streams \
    textinstream \
    textoutstream \
    vbaswobj \
    msforms \
    vclcanvas \
    writerfilter_debug \
))


$(eval $(call gb_Helper_register_libraries,UNOVERLIBS, \
    cppu \
    jvmfwk \
    sal \
))

$(eval $(call gb_Helper_register_static_libraries,PLAINLIBS, \
    basegfx_s \
    ooopathutils \
    salcpprt \
    vclmain \
))

# vim: set noet sw=4 ts=4:
