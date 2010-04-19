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

$(eval $(call gb_Module_Module,svtools,\
    $(call gb_AllLangResTarget_get_target,svt) \
    $(call gb_AllLangResTarget_get_target,productregistration) \
    $(call gb_Executable_get_target,bmp) \
    $(call gb_Executable_get_target,bmpsum) \
    $(call gb_Executable_get_target,g2g) \
    $(call gb_Library_get_target,hatchwindowfactory) \
    $(call gb_Library_get_target,productregistration) \
    $(call gb_Library_get_target,svt) \
    $(call gb_Package_get_target,svtools_inc) \
))

$(eval $(call gb_Module_read_includes,svtools,\
    exe_bmp \
    exe_bmpsum \
    exe_g2g \
    lib_hatchwindowfactory \
    lib_productregistration \
    lib_svt \
    package_inc \
    res_svt \
    res_productregistration \
))

#todo: javapatchres
#todo: jpeg on mac in svtools/util/makefile.mk
#todo: deliver errtxt.src as ehdl.srs
#todo: nooptfiles filter, filterconfigitem, FilterConfigCache, SvFilterOptionsDialog
#todo: map file
# vim: set noet sw=4 ts=4:
