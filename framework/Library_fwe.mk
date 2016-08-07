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



$(eval $(call gb_Library_Library,fwe))

$(eval $(call gb_Library_set_include,fwe,\
    -I$(SRCDIR)/framework/inc/pch \
    -I$(SRCDIR)/framework/source/inc \
    -I$(SRCDIR)/framework/inc \
    -I$(WORKDIR)/inc/framework/ \
    $$(INCLUDE) \
    -I$(OUTDIR)/inc/framework \
))

$(eval $(call gb_Library_add_defs,fwe,\
    -DFWE_DLLIMPLEMENTATION\
))

$(eval $(call gb_Library_add_api,fwe,\
    udkapi \
    offapi \
))

$(eval $(call gb_Library_add_linked_libs,fwe,\
    comphelper \
    cppu \
    cppuhelper \
    fwi \
    sal \
    stl \
    svl \
    svt \
    tl \
    utl \
    vcl \
    vos3 \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,fwe,\
    framework/source/fwe/classes/actiontriggercontainer \
    framework/source/fwe/classes/actiontriggerpropertyset \
    framework/source/fwe/classes/actiontriggerseparatorpropertyset \
    framework/source/fwe/classes/addonmenu \
    framework/source/fwe/classes/addonsoptions \
    framework/source/fwe/classes/bmkmenu \
    framework/source/fwe/classes/framelistanalyzer \
    framework/source/fwe/classes/fwkresid \
    framework/source/fwe/classes/imagewrapper \
    framework/source/fwe/classes/menuextensionsupplier \
    framework/source/fwe/classes/rootactiontriggercontainer \
    framework/source/fwe/classes/sfxhelperfunctions \
    framework/source/fwe/dispatch/interaction \
    framework/source/fwe/helper/acceleratorinfo \
    framework/source/fwe/helper/actiontriggerhelper \
    framework/source/fwe/helper/configimporter \
    framework/source/fwe/helper/imageproducer \
    framework/source/fwe/helper/propertysetcontainer \
    framework/source/fwe/helper/titlehelper \
    framework/source/fwe/helper/documentundoguard \
    framework/source/fwe/helper/undomanagerhelper \
    framework/source/fwe/interaction/preventduplicateinteraction \
    framework/source/fwe/xml/eventsconfiguration \
    framework/source/fwe/xml/eventsdocumenthandler \
    framework/source/fwe/xml/menuconfiguration \
    framework/source/fwe/xml/menudocumenthandler \
    framework/source/fwe/xml/saxnamespacefilter \
    framework/source/fwe/xml/statusbarconfiguration \
    framework/source/fwe/xml/statusbardocumenthandler \
    framework/source/fwe/xml/toolboxconfiguration \
    framework/source/fwe/xml/toolboxdocumenthandler \
    framework/source/fwe/xml/xmlnamespaces \
))

# vim: set noet sw=4 ts=4:
