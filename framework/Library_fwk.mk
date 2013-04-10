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



$(eval $(call gb_Library_Library,fwk))

$(eval $(call gb_Library_add_precompiled_header,fwk,$(SRCDIR)/framework/inc/pch/precompiled_framework))

$(eval $(call gb_Library_set_componentfile,fwk,framework/util/fwk))

$(eval $(call gb_Library_set_include,fwk,\
    -I$(SRCDIR)/framework/inc/pch \
    -I$(SRCDIR)/framework/source/inc \
    -I$(SRCDIR)/framework/inc \
    -I$(WORKDIR)/inc/framework/ \
    $$(INCLUDE) \
    -I$(OUTDIR)/inc/framework \
    -I$(OUTDIR)/inc/offuh \
))

$(eval $(call gb_Library_add_linked_libs,fwk,\
    comphelper \
    cppu \
    cppuhelper \
    fwe \
    fwi \
    i18nisolang1 \
    sal \
    sot \
    stl \
    svl \
    svt \
    tk \
    tl \
    ucbhelper \
    utl \
    vcl \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,fwk,\
    framework/source/accelerators/acceleratorcache \
    framework/source/accelerators/acceleratorconfiguration \
    framework/source/accelerators/documentacceleratorconfiguration \
    framework/source/accelerators/globalacceleratorconfiguration \
    framework/source/accelerators/keymapping \
    framework/source/accelerators/moduleacceleratorconfiguration \
    framework/source/accelerators/presethandler \
    framework/source/accelerators/storageholder \
    framework/source/classes/droptargetlistener \
    framework/source/classes/framecontainer \
    framework/source/classes/fwktabwindow \
    framework/source/classes/menumanager \
    framework/source/classes/taskcreator \
    framework/source/constant/containerquery \
    framework/source/constant/contenthandler \
    framework/source/constant/frameloader \
    framework/source/dispatch/closedispatcher \
    framework/source/dispatch/dispatchinformationprovider \
    framework/source/dispatch/dispatchprovider \
    framework/source/dispatch/helpagentdispatcher \
    framework/source/dispatch/interceptionhelper \
    framework/source/dispatch/loaddispatcher \
    framework/source/dispatch/menudispatcher \
    framework/source/dispatch/startmoduledispatcher \
    framework/source/dispatch/windowcommanddispatch \
    framework/source/helper/dockingareadefaultacceptor \
    framework/source/helper/ocomponentaccess \
    framework/source/helper/ocomponentenumeration \
    framework/source/helper/oframes \
    framework/source/helper/persistentwindowstate \
    framework/source/helper/statusindicator \
    framework/source/helper/statusindicatorfactory \
    framework/source/helper/tagwindowasmodified \
    framework/source/helper/titlebarupdate \
    framework/source/helper/uiconfigelementwrapperbase \
    framework/source/helper/uielementwrapperbase \
    framework/source/helper/vclstatusindicator \
    framework/source/helper/wakeupthread \
    framework/source/interaction/quietinteraction \
    framework/source/jobs/job \
    framework/source/jobs/jobdata \
    framework/source/jobs/jobdispatch \
    framework/source/jobs/jobexecutor \
    framework/source/jobs/jobresult \
    framework/source/jobs/joburl \
    framework/source/layoutmanager/helpers \
    framework/source/layoutmanager/layoutmanager \
    framework/source/layoutmanager/panel \
    framework/source/layoutmanager/panelmanager \
    framework/source/layoutmanager/toolbarlayoutmanager \
    framework/source/layoutmanager/uielement \
    framework/source/loadenv/loadenv \
    framework/source/loadenv/targethelper \
    framework/source/register/registerservices \
    framework/source/services/autorecovery \
    framework/source/services/backingcomp \
    framework/source/services/backingwindow \
    framework/source/services/desktop \
    framework/source/services/ContextChangeEventMultiplexer \
    framework/source/services/frame \
    framework/source/services/modelwinservice \
    framework/source/services/modulemanager \
    framework/source/services/pathsettings \
    framework/source/services/sessionlistener \
    framework/source/services/substitutepathvars \
    framework/source/services/tabwindowservice \
    framework/source/services/taskcreatorsrv \
    framework/source/services/uriabbreviation \
    framework/source/services/urltransformer \
    framework/source/uiconfiguration/globalsettings \
    framework/source/uiconfiguration/graphicnameaccess \
    framework/source/uiconfiguration/imagemanager \
    framework/source/uiconfiguration/imagemanagerimpl \
    framework/source/uiconfiguration/moduleimagemanager \
    framework/source/uiconfiguration/moduleuicfgsupplier \
    framework/source/uiconfiguration/moduleuiconfigurationmanager \
    framework/source/uiconfiguration/uicategorydescription \
    framework/source/uiconfiguration/uiconfigurationmanager \
    framework/source/uiconfiguration/uiconfigurationmanagerimpl \
    framework/source/uiconfiguration/windowstateconfiguration \
    framework/source/uielement/addonstoolbarmanager \
    framework/source/uielement/addonstoolbarwrapper \
    framework/source/uielement/buttontoolbarcontroller \
    framework/source/uielement/comboboxtoolbarcontroller \
    framework/source/uielement/complextoolbarcontroller \
    framework/source/uielement/controlmenucontroller \
    framework/source/uielement/dropdownboxtoolbarcontroller \
    framework/source/uielement/edittoolbarcontroller \
    framework/source/uielement/genericstatusbarcontroller \
    framework/source/uielement/generictoolbarcontroller \
    framework/source/uielement/imagebuttontoolbarcontroller \
    framework/source/uielement/langselectionstatusbarcontroller \
    framework/source/uielement/menubarmanager \
    framework/source/uielement/menubarmerger \
    framework/source/uielement/menubarwrapper \
    framework/source/uielement/objectmenucontroller \
    framework/source/uielement/panelwindow \
    framework/source/uielement/panelwrapper \
    framework/source/uielement/progressbarwrapper \
    framework/source/uielement/recentfilesmenucontroller \
    framework/source/uielement/spinfieldtoolbarcontroller \
    framework/source/uielement/statusbar \
    framework/source/uielement/statusbaritem \
    framework/source/uielement/statusbarmanager \
    framework/source/uielement/statusbarmerger \
    framework/source/uielement/statusbarwrapper \
    framework/source/uielement/statusindicatorinterfacewrapper \
    framework/source/uielement/togglebuttontoolbarcontroller \
    framework/source/uielement/toolbar \
    framework/source/uielement/toolbarmanager \
    framework/source/uielement/toolbarmerger \
    framework/source/uielement/toolbarwrapper \
    framework/source/uielement/uicommanddescription \
    framework/source/uifactory/addonstoolboxfactory \
    framework/source/uifactory/factoryconfiguration \
    framework/source/uifactory/menubarfactory \
    framework/source/uifactory/popupmenucontrollerfactory \
    framework/source/uifactory/statusbarcontrollerfactory \
    framework/source/uifactory/statusbarfactory \
    framework/source/uifactory/toolbarcontrollerfactory \
    framework/source/uifactory/toolboxfactory \
    framework/source/uifactory/uielementfactorymanager \
    framework/source/uifactory/windowcontentfactorymanager \
    framework/source/xml/acceleratorconfigurationreader \
    framework/source/xml/acceleratorconfigurationwriter \
    framework/source/xml/imagesconfiguration \
    framework/source/xml/imagesdocumenthandler \
))

# vim: set noet sw=4 ts=4:
