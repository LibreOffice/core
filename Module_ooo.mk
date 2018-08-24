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
    animations \
    autodoc \
    automation \
    avmedia \
    basctl \
    basebmp \
    basegfx \
    binaryurp \
    canvas \
    codemaker \
    comphelper \
    configmgr \
    cosv \
    cppcanvas \
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
    i18nutil \
    idl \
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
    padmin \
    psprint_config \
    rdbmaker \
    registry \
    remotebridges \
    reportdesign \
    ridljar \
    sane \
    sax \
    sc \
    sccomp \
    sd \
    sfx2 \
    slideshow \
    sot \
    starmath \
    store \
    svgio \
    svl \
    svtools \
    svx \
    sw \
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
    uui \
    vbahelper \
    vcl \
    vos \
    wizards \
    writerfilter \
    x11_extensions \
    xml2cmp \
    xmlhelp \
    xmloff \
    xmlreader \
    xmlscript \
))

# vim: set noet ts=4 sw=4:
