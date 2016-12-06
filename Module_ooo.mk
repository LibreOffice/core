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
    animations \
    basebmp \
    basegfx \
    binaryurp \
    canvas \
    comphelper \
    configmgr \
    cppcanvas \
    dbaccess \
    drawinglayer \
    editeng \
    eventattacher \
    fileaccess \
    formula \
    framework \
    idl \
    io \
    linguistic \
    o3tl \
    offapi \
    oovbaapi \
    oox \
    padmin \
    package \
    reportdesign \
    remotebridges \
    sax \
    sd \
    sfx2 \
    sot \
    starmath \
    svgio \
    svl \
    svtools \
    svx \
    sw \
    toolkit \
    tools \
    ucbhelper \
    udkapi \
    unotools \
    unoxml \
    uui \
    vbahelper \
    vcl \
    wizards \
    writerfilter \
    x11_extensions \
    xmloff \
    xmlreader \
    xmlscript \
))

# vim: set noet ts=4 sw=4:
