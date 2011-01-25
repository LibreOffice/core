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

$(eval $(call gb_Package_Package,sot_inc,$(SRCDIR)/sot/inc))
$(eval $(call gb_Package_add_file,sot_inc,inc/sot/absdev.hxx,sot/absdev.hxx))
$(eval $(call gb_Package_add_file,sot_inc,inc/sot/agg.hxx,sot/agg.hxx))
$(eval $(call gb_Package_add_file,sot_inc,inc/sot/clsids.hxx,sot/clsids.hxx))
$(eval $(call gb_Package_add_file,sot_inc,inc/sot/exchange.hxx,sot/exchange.hxx))
$(eval $(call gb_Package_add_file,sot_inc,inc/sot/factory.hxx,sot/factory.hxx))
$(eval $(call gb_Package_add_file,sot_inc,inc/sot/filelist.hxx,sot/filelist.hxx))
$(eval $(call gb_Package_add_file,sot_inc,inc/sot/formats.hxx,sot/formats.hxx))
$(eval $(call gb_Package_add_file,sot_inc,inc/sot/object.hxx,sot/object.hxx))
$(eval $(call gb_Package_add_file,sot_inc,inc/sot/sotdata.hxx,sot/sotdata.hxx))
$(eval $(call gb_Package_add_file,sot_inc,inc/sot/sotdllapi.h,sot/sotdllapi.h))
$(eval $(call gb_Package_add_file,sot_inc,inc/sot/sotref.hxx,sot/sotref.hxx))
$(eval $(call gb_Package_add_file,sot_inc,inc/sot/stg.hxx,sot/stg.hxx))
$(eval $(call gb_Package_add_file,sot_inc,inc/sot/storage.hxx,sot/storage.hxx))
$(eval $(call gb_Package_add_file,sot_inc,inc/sot/storinfo.hxx,sot/storinfo.hxx))
