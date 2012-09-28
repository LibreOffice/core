# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
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

$(eval $(call gb_Library_Library,sot))

$(eval $(call gb_Library_use_package,sot,sot_inc))

$(eval $(call gb_Library_set_componentfile,sot,sot/util/sot))

$(eval $(call gb_Library_set_include,sot,\
    $$(INCLUDE) \
))

$(eval $(call gb_Library_use_sdk_api,sot))

$(eval $(call gb_Library_add_defs,sot,\
    -DSOT_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_use_libraries,sot,\
    comphelper \
    cppu \
    cppuhelper \
    sal \
    tl \
    ucbhelper \
    utl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_exception_objects,sot,\
    sot/source/unoolestorage/xolesimplestorage \
    sot/source/unoolestorage/register \
    sot/source/base/formats \
    sot/source/base/object \
    sot/source/base/exchange \
    sot/source/base/filelist \
    sot/source/base/factory \
    sot/source/sdstor/stg \
    sot/source/sdstor/stgavl \
    sot/source/sdstor/stgcache \
    sot/source/sdstor/stgdir \
    sot/source/sdstor/stgelem \
    sot/source/sdstor/stgio \
    sot/source/sdstor/stgole \
    sot/source/sdstor/stgstrms \
    sot/source/sdstor/storage \
    sot/source/sdstor/storinfo \
    sot/source/sdstor/ucbstorage \
))

# vim: set noet sw=4 ts=4:
