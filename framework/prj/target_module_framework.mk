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

include $(foreach targetdef,\
    lib_fwe \
    lib_fwi \
    lib_fwk \
    lib_fwl \
    lib_fwm \
    package_dtd \
    package_inc \
    package_uiconfig \
    package_unotypes \
    res_fwe \
,$(SRCDIR)/framework/prj/target_$(targetdef).mk)


$(eval $(call gb_Module_Module,framework,\
    $(call gb_AllLangResTarget_get_target,fwe) \
    $(call gb_Library_get_target,fwe) \
    $(call gb_Library_get_target,fwi) \
    $(call gb_Library_get_target,fwk) \
    $(call gb_Library_get_target,fwl) \
    $(call gb_Library_get_target,fwm) \
    $(call gb_Library_get_target,fwe) \
    $(call gb_Package_get_target,framework_dtd) \
    $(call gb_Package_get_target,framework_inc) \
    $(call gb_Package_get_target,framework_uiconfig) \
    $(call gb_Package_get_target,framework_unotypes) \
))
# vim: set noet ts=4 sw=4:
