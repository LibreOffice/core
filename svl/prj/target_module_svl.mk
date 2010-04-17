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

$(eval $(call gb_Module_read_includes,svl,\
    lib_svl \
    lib_fsstorage \
    lib_passwordcontainer \
    res_svl \
    package_inc \
))

$(eval $(call gb_Module_Module,svl,\
    $(call gb_AllLangResTarget_get_target,svl) \
    $(call gb_Library_get_target,fsstorage) \
    $(call gb_Library_get_target,passwordcontainer) \
    $(call gb_Library_get_target,svl) \
    $(call gb_Package_get_target,svl_inc) \
))


#todo: dde platform dependent
#todo: package_inc
#todo: map file
