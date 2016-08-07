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



$(eval $(call gb_Library_Library,fwl))

$(eval $(call gb_Library_set_componentfile,fwl,framework/util/fwl))

$(eval $(call gb_Library_set_include,fwl,\
    -I$(SRCDIR)/framework/inc \
    -I$(SRCDIR)/framework/inc/pch \
    -I$(SRCDIR)/framework/source/inc \
    -I$(WORKDIR)/inc/framework/ \
    $$(INCLUDE) \
    -I$(OUTDIR)/inc/framework \
))

$(eval $(call gb_Library_add_api,fwl,\
    udkapi \
    offapi \
))

$(eval $(call gb_Library_add_linked_libs,fwl,\
    comphelper \
    cppu \
    cppuhelper \
    fwe \
    fwi \
    i18nisolang1 \
    sal \
    stl \
    svl \
    svt \
    ootk \
    tl \
    utl \
    vcl \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,fwl,\
    framework/source/classes/fwlresid \
    framework/source/dispatch/mailtodispatcher \
    framework/source/dispatch/oxt_handler \
    framework/source/dispatch/popupmenudispatcher \
    framework/source/dispatch/servicehandler \
    framework/source/recording/dispatchrecorder \
    framework/source/recording/dispatchrecordersupplier \
    framework/source/register/registertemp \
    framework/source/services/dispatchhelper \
    framework/source/services/license \
    framework/source/services/mediatypedetectionhelper \
    framework/source/services/uriabbreviation \
    framework/source/uielement/fontmenucontroller \
    framework/source/uielement/fontsizemenucontroller \
    framework/source/uielement/footermenucontroller \
    framework/source/uielement/headermenucontroller \
    framework/source/uielement/langselectionmenucontroller \
    framework/source/uielement/macrosmenucontroller \
    framework/source/uielement/newmenucontroller \
    framework/source/uielement/toolbarsmenucontroller \
))

# vim: set noet sw=4 ts=4:
