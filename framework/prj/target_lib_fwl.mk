#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2009 by Sun Microsystems, Inc.
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.	If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

$(eval $(call gb_Library_Library,fwl))

$(call gb_Library_get_headers_target,fwl) : $(call gb_Package_get_target,framework_inc)

$(eval $(call gb_Library_set_include,fwl,\
    $$(INCLUDE) \
    -I$(WORKDIR)/inc/framework/ \
    -I$(SRCDIR)/framework/inc \
    -I$(SRCDIR)/framework/inc/pch \
    -I$(SRCDIR)/framework/source/inc \
    -I$(OUTDIR)/inc/framework \
    -I$(OUTDIR)/inc/offuh \
))

$(eval $(call gb_Library_add_linked_libs,fwl,\
    comphelper \
    fwe \
    fwi \
    i18nisolang1 \
    stl \
    svl \
    svt \
    tk \
    tl \
    cppu \
    cppuhelper \
    sal \
    utl \
    vcl \
))

$(eval $(call gb_Library_add_exception_objects,fwl,\
    framework/source/register/registertemp \
    framework/source/services/mediatypedetectionhelper \
    framework/source/dispatch/mailtodispatcher \
    framework/source/dispatch/oxt_handler \
    framework/source/uielement/toolbarsmenucontroller \
    framework/source/uielement/newmenucontroller \
    framework/source/uielement/macrosmenucontroller \
    framework/source/uielement/langselectionmenucontroller \
    framework/source/uielement/headermenucontroller \
    framework/source/uielement/footermenucontroller \
    framework/source/uielement/fontsizemenucontroller \
    framework/source/uielement/fontmenucontroller \
    framework/source/services/tabwindowservice \
    framework/source/classes/fwktabwindow \
    framework/source/uielement/logotextstatusbarcontroller \
    framework/source/classes/fwlresid \
    framework/source/uielement/logoimagestatusbarcontroller \
    framework/source/uielement/simpletextstatusbarcontroller \
    framework/source/services/uriabbreviation \
    framework/source/dispatch/servicehandler \
    framework/source/services/license \
    framework/source/recording/dispatchrecorder \
    framework/source/recording/dispatchrecordersupplier \
    framework/source/services/dispatchhelper \
    framework/source/dispatch/popupmenudispatcher \
))
ifeq ($(OS),LINUX)
$(eval $(call gb_Library_add_linked_libs,fwl,\
    dl \
    m \
    pthread \
))
endif
ifeq ($(OS),WNT)
$(eval $(call gb_Library_add_linked_libs,fwl,\
    kernel32 \
    msvcrt \
    uwinapi \
))
endif
# vim: set noet sw=4 ts=4:
