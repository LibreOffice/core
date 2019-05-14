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



$(eval $(call gb_Module_Module,ooo))

$(eval $(call gb_Module_add_moduledirs,ooo,\
    MathMLDTD \
    UnoControls \
    accessibility \
    animations \
    apple_remote \
    autodoc \
    automation \
    avmedia \
    basctl \
    basebmp \
    basegfx \
    basic \
    bean \
    binaryurp \
    bridges \
    canvas \
    codemaker \
    comphelper \
    configmgr \
    cosv \
    cppcanvas \
    cppuhelper \
    cpputools \
    cui \
    dbaccess \
    drawinglayer \
    dtrans \
    editeng \
    embeddedobj \
    eventattacher \
    fileaccess \
    formula \
    fpicker \
    framework \
    i18npool \
    i18nutil \
    idl \
    idlc \
    io \
    javaunohelper \
    jurt \
    jvmaccess \
    lingucomponent \
    linguistic \
    o3tl \
    offapi \
    oovbaapi \
    oox \
    package \
    packimages \
    padmin \
    psprint_config \
    pyuno \
    qadevOOo \
    rdbmaker \
    registry \
    remotebridges \
    reportdesign \
    ridljar \
    rsc \
    sane \
    sax \
    sc \
    scaddins \
    sccomp \
    sd \
    sfx2 \
    slideshow \
    solenv \
    sot \
    starmath \
    store \
    svgio \
    svl \
    svtools \
    svx \
    sw \
    testgraphical \
    toolkit \
    tools \
    twain \
    ucb \
    ucbhelper \
    udkapi \
    udm \
    unixODBC \
    unodevtools \
    unotools \
    unoxml \
    ure \
    uui \
    vbahelper \
    vcl \
    vos \
    wizards \
    writerfilter \
    x11_extensions \
    xmerge \
    xml2cmp \
    xmlhelp \
    xmloff \
    xmlreader \
    xmlscript \
))

# vim: set noet ts=4 sw=4:
