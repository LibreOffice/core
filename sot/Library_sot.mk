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

$(eval $(call gb_Library_add_package_headers,sot,sot_inc))

$(eval $(call gb_Library_add_precompiled_header,sot,$(SRCDIR)/sot/inc/pch/precompiled_sot))

$(eval $(call gb_Library_set_componentfile,sot,sot/util/sot))

$(eval $(call gb_Library_set_include,sot,\
    -I$(realpath $(SRCDIR)/sot/inc/pch) \
    $$(INCLUDE) \
    -I$(OUTDIR)/inc/offuh \
))

$(eval $(call gb_Library_set_defs,sot,\
    $$(DEFS) \
    -DSOT_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_add_linked_libs,sot,\
    comphelper \
    cppu \
    cppuhelper \
    sal \
    tl \
    ucbhelper \
    utl \
    $(gb_STDLIBS) \
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
    sot/source/sdstor/unostorageholder \
))

# vim: set noet sw=4 ts=4:
