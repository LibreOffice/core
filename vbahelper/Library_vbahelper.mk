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



$(eval $(call gb_Library_Library,vbahelper))

$(eval $(call gb_Library_add_package_headers,vbahelper,vbahelper_inc))

# for platforms supporting PCH: declare the location of the pch file
# this is the name of the cxx file (without extension)
#$(eval $(call gb_Library_add_precompiled_header,vbahelper,$(SRCDIR)/vbahelper/PCH_FILE))

# in case UNO services are exported: declare location of component file
#$(eval $(call gb_Library_set_componentfile,vbahelper,vbahelper/COMPONENT_FILE))

# add any additional include paths for this library here
$(eval $(call gb_Library_set_include,vbahelper,\
    $$(INCLUDE) \
))

$(eval $(call gb_Library_add_defs,vbahelper,\
    -DVBAHELPER_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_add_api,vbahelper,\
    udkapi \
    offapi \
    oovbaapi \
))

# add libraries to be linked to vbahelper; again these names need to be given as
# specified in Repository.mk
$(eval $(call gb_Library_add_linked_libs,vbahelper,\
    comphelper \
    cppu \
    cppuhelper \
    msfilter \
    sal \
    sb \
    sfx \
    stl \
    svl \
    svt \
    svxcore \
    ootk \
    tl \
    utl \
    vcl \
    $(gb_STDLIBS) \
))

# add all source files that shall be compiled with exceptions enabled
# the name is relative to $(SRCROOT) and must not contain an extension
$(eval $(call gb_Library_add_exception_objects,vbahelper,\
    vbahelper/source/vbahelper/collectionbase \
    vbahelper/source/vbahelper/vbaapplicationbase \
    vbahelper/source/vbahelper/vbacolorformat \
    vbahelper/source/vbahelper/vbacommandbar \
    vbahelper/source/vbahelper/vbacommandbarcontrol \
    vbahelper/source/vbahelper/vbacommandbarcontrols \
    vbahelper/source/vbahelper/vbacommandbarhelper \
    vbahelper/source/vbahelper/vbacommandbars \
    vbahelper/source/vbahelper/vbadialogbase \
    vbahelper/source/vbahelper/vbadialogsbase \
    vbahelper/source/vbahelper/vbadocumentbase \
    vbahelper/source/vbahelper/vbadocumentsbase \
    vbahelper/source/vbahelper/vbaeventshelperbase \
    vbahelper/source/vbahelper/vbafillformat \
    vbahelper/source/vbahelper/vbafontbase \
    vbahelper/source/vbahelper/vbaglobalbase \
    vbahelper/source/vbahelper/vbahelper \
    vbahelper/source/vbahelper/vbalineformat \
    vbahelper/source/vbahelper/vbapagesetupbase \
    vbahelper/source/vbahelper/vbapictureformat \
    vbahelper/source/vbahelper/vbapropvalue \
    vbahelper/source/vbahelper/vbashape \
    vbahelper/source/vbahelper/vbashaperange \
    vbahelper/source/vbahelper/vbashapes \
    vbahelper/source/vbahelper/vbatextframe \
    vbahelper/source/vbahelper/vbawindowbase \
))

# vim: set noet sw=4 ts=4:

