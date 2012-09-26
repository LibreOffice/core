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

$(eval $(call gb_Library_Library,fwm))

$(eval $(call gb_Library_set_componentfile,fwm,framework/util/fwm))

$(eval $(call gb_Library_set_include,fwm,\
    -I$(SRCDIR)/framework/inc \
    -I$(SRCDIR)/framework/source/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_use_sdk_api,fwm))

$(eval $(call gb_Library_use_libraries,fwm,\
    comphelper \
    cppu \
    cppuhelper \
    fwi \
    sal \
    svl \
    svt \
    tk \
    tl \
    utl \
    vcl \
	$(gb_UWINAPI) \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,fwm,\
    framework/source/dispatch/systemexec \
    framework/source/jobs/helponstartup \
    framework/source/jobs/shelljob \
    framework/source/register/register3rdcomponents \
    framework/source/tabwin/tabwindow \
    framework/source/tabwin/tabwinfactory \
))

# vim: set noet sw=4 ts=4:
