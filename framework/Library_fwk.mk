# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2011 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

$(eval $(call gb_Library_Library,fwk))

$(eval $(call gb_Library_set_componentfile,fwk,framework/util/fwk))

$(eval $(call gb_Library_set_include,fwk,\
    -I$(SRCDIR)/framework/source/inc \
    -I$(SRCDIR)/framework/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_use_sdk_api,fwk))

$(eval $(call gb_Library_use_libraries,fwk,\
    comphelper \
    cppu \
    cppuhelper \
    fwe \
    fwi \
    i18nisolang1 \
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
    framework/source/services/autorecovery \
    framework/source/services/backingcomp \
    framework/source/services/backingwindow \
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
    framework/source/uielement/generictoolbarcontroller \
    framework/source/uielement/imagebuttontoolbarcontroller \
    framework/source/uielement/langselectionstatusbarcontroller \
    framework/source/uielement/menubarmanager \
    framework/source/uielement/menubarmerger \
    framework/source/uielement/menubarwrapper \
    framework/source/uielement/objectmenucontroller \
    framework/source/uielement/progressbarwrapper \
    framework/source/uielement/recentfilesmenucontroller \
    framework/source/uielement/spinfieldtoolbarcontroller \
    framework/source/uielement/statusbar \
    framework/source/uielement/statusbarmanager \
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
