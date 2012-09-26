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

$(eval $(call gb_Library_Library,vbahelper))

$(eval $(call gb_Library_use_package,vbahelper,vbahelper_inc))

# in case UNO services are exported: declare location of component file
#$(eval $(call gb_Library_set_componentfile,vbahelper,vbahelper/COMPONENT_FILE))

# add any additional include paths for this library here
$(eval $(call gb_Library_set_include,vbahelper,\
    $$(INCLUDE) \
))

$(eval $(call gb_Library_add_defs,vbahelper,\
    -DVBAHELPER_DLLIMPLEMENTATION \
))

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
	$(gb_UWINAPI) \
    $(gb_STDLIBS) \
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
