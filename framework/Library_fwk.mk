# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

$(eval $(call gb_Library_Library,fwk))

$(eval $(call gb_Library_set_componentfile,fwk,framework/util/fwk))

$(eval $(call gb_Library_set_include,fwk,\
    -I$(SRCDIR)/framework/source/inc \
    -I$(SRCDIR)/framework/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_set_precompiled_header,fwk,$(SRCDIR)/framework/inc/pch/precompiled_fwk))

$(eval $(call gb_Library_use_external,fwk,boost_headers))

$(eval $(call gb_Library_use_custom_headers,fwk,\
	officecfg/registry \
))

$(eval $(call gb_Library_use_sdk_api,fwk))

$(eval $(call gb_Library_use_libraries,fwk,\
    comphelper \
    cppu \
    cppuhelper \
    fwe \
    fwi \
    i18nlangtag \
    sal \
    sot \
    svl \
    svt \
    tk \
    tl \
    ucbhelper \
    utl \
    vcl \
	$(gb_UWINAPI) \
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
    framework/source/dispatch/closedispatcher \
    framework/source/dispatch/dispatchinformationprovider \
    framework/source/dispatch/dispatchprovider \
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
    framework/source/layoutmanager/toolbarlayoutmanager \
    framework/source/layoutmanager/uielement \
    framework/source/loadenv/loadenv \
    framework/source/loadenv/targethelper \
    framework/source/register/registerservices \
    framework/source/services/ContextChangeEventMultiplexer \
    framework/source/services/autorecovery \
    framework/source/services/desktop \
    framework/source/services/frame \
    framework/source/services/modulemanager \
    framework/source/services/pathsettings \
    framework/source/services/sessionlistener \
    framework/source/services/substitutepathvars \
    framework/source/services/tabwindowservice \
    framework/source/services/taskcreatorsrv \
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
    framework/source/uielement/popuptoolbarcontroller \
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
    framework/source/uielement/toolbarmanager \
    framework/source/uielement/toolbarmerger \
    framework/source/uielement/toolbarwrapper \
    framework/source/uielement/uicommanddescription \
    framework/source/uifactory/addonstoolboxfactory \
    framework/source/uifactory/factoryconfiguration \
    framework/source/uifactory/menubarfactory \
    framework/source/uifactory/statusbarfactory \
    framework/source/uifactory/toolboxfactory \
    framework/source/uifactory/uicontrollerfactory \
    framework/source/uifactory/uielementfactorymanager \
    framework/source/uifactory/windowcontentfactorymanager \
    framework/source/xml/acceleratorconfigurationreader \
    framework/source/xml/acceleratorconfigurationwriter \
    framework/source/xml/imagesconfiguration \
    framework/source/xml/imagesdocumenthandler \
))

# vim: set noet sw=4 ts=4:
