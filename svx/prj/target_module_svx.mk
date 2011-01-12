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

$(eval $(call gb_Module_Module,svx,\
    $(call gb_AllLangResTarget_get_target,svx) \
    $(call gb_Library_get_target,svxcore) \
    $(call gb_Library_get_target,svx) \
))

$(eval $(call gb_Module_read_includes,svx,\
    lib_svx \
    lib_svxcore \
    res_svx \
))

#todo: noopt for EnhanceCustomShapesFunctionParser.cxx on Solaris Sparc and MacOSX
#todo: -DBOOST_SPIRIT_USE_OLD_NAMESPACE only in CustomShapes
#todo: -DUNICODE and -D_UNICODE on WNT for source/dialog
#todo: noopt for viewpt3d.cxx with MSVC and ICC
#todo: set -DENABLE_VBA in case $(ENABLE_VBA)="YES"; can we do it in gbuild.mk?
#todo: gengal
#todo: set -D... for ENABLE_GTK/KDE/KDE4 -> in gbuild.mk?
#todo: ibrwimp.lib in util/makefile.mk on Windows ?!
#todo: GENHID
#todo: uiconfig
#todo: exports from gallery (util/gal.dxp) and svx (svx.dxp)
#todo: resources, textconversion, packages

