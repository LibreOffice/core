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

$(eval $(call gb_Library_Library,vbahelper))

# in case UNO services are exported: declare location of component file
#$(eval $(call gb_Library_set_componentfile,vbahelper,vbahelper/COMPONENT_FILE,services))

$(eval $(call gb_Library_add_defs,vbahelper,\
    -DVBAHELPER_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_use_external,vbahelper,boost_headers))

$(eval $(call gb_Library_set_precompiled_header,vbahelper,vbahelper/inc/pch/precompiled_vbahelper))

$(eval $(call gb_Library_use_api,vbahelper,\
	udkapi \
	offapi \
	oovbaapi \
))

# add libraries to be linked to vbahelper; again these names need to be given as
# specified in Repository.mk
$(eval $(call gb_Library_use_libraries,vbahelper,\
    comphelper \
    cppu \
    cppuhelper \
    msfilter \
    sal \
    sb \
    sfx \
    svl \
    svt \
	svxcore \
    tk \
    tl \
    utl \
    vcl \
))

# add all source files that shall be compiled with exceptions enabled
# the name is relative to $(SRCROOT) and must not contain an extension
$(eval $(call gb_Library_add_exception_objects,vbahelper,\
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
