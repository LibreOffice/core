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

$(eval $(call gb_Library_Library,fwl))

$(eval $(call gb_Library_set_componentfile,fwl,framework/util/fwl))

$(eval $(call gb_Library_set_include,fwl,\
    -I$(SRCDIR)/framework/inc \
    -I$(SRCDIR)/framework/source/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_use_sdk_api,fwl))

$(eval $(call gb_Library_use_libraries,fwl,\
    comphelper \
    cppu \
    cppuhelper \
    fwe \
    fwi \
    i18nisolang1 \
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
    framework/source/uielement/logoimagestatusbarcontroller \
    framework/source/uielement/logotextstatusbarcontroller \
    framework/source/uielement/macrosmenucontroller \
    framework/source/uielement/newmenucontroller \
    framework/source/uielement/popupmenucontroller \
    framework/source/uielement/simpletextstatusbarcontroller \
    framework/source/uielement/toolbarsmenucontroller \
))

# vim: set noet sw=4 ts=4:
