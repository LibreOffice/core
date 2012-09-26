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

# MODULE is the name of the module the makefile is located in

# declare a library
# LIB is the name of the library as it is found in Repository.mk
$(eval $(call gb_Library_Library,msforms))

# in case UNO services are exported: declare location of component file
$(eval $(call gb_Library_set_componentfile,msforms,vbahelper/util/msforms))

# add any additional include paths for this library here
$(eval $(call gb_Library_set_include,msforms,\
    $$(INCLUDE) \
))

$(eval $(call gb_Library_use_api,msforms,\
	udkapi \
	offapi \
	oovbaapi \
))

# add libraries to be linked to LIB; again these names need to be given as
# specified in Repository.mk
$(eval $(call gb_Library_use_libraries,msforms,\
    comphelper \
    cppu \
    cppuhelper \
    sal \
    sb \
    sfx \
    svl \
    svt \
    svx \
    tk \
    tl \
    vbahelper \
    vcl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_standard_system_libs,msforms))

# add all source files that shall be compiled with exceptions enabled
# the name is relative to $(SRCROOT) and must not contain an extension
$(eval $(call gb_Library_add_exception_objects,msforms,\
    vbahelper/source/msforms/service \
    vbahelper/source/msforms/vbabutton \
    vbahelper/source/msforms/vbacheckbox \
    vbahelper/source/msforms/vbacombobox \
    vbahelper/source/msforms/vbacontrol \
    vbahelper/source/msforms/vbacontrols \
    vbahelper/source/msforms/vbaframe \
    vbahelper/source/msforms/vbaimage \
    vbahelper/source/msforms/vbalabel \
    vbahelper/source/msforms/vbalistbox \
    vbahelper/source/msforms/vbalistcontrolhelper \
    vbahelper/source/msforms/vbamultipage \
    vbahelper/source/msforms/vbanewfont \
    vbahelper/source/msforms/vbapages \
    vbahelper/source/msforms/vbaprogressbar \
    vbahelper/source/msforms/vbaradiobutton \
    vbahelper/source/msforms/vbascrollbar \
    vbahelper/source/msforms/vbaspinbutton \
    vbahelper/source/msforms/vbasystemaxcontrol \
    vbahelper/source/msforms/vbatextbox \
    vbahelper/source/msforms/vbatogglebutton \
    vbahelper/source/msforms/vbauserform \
))

# vim: set noet sw=4 ts=4:
