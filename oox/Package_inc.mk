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

$(eval $(call gb_Package_Package,oox_inc,$(SRCDIR)/oox/inc))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/dllapi.h,oox/dllapi.h))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/core/filterbase.hxx,oox/core/filterbase.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/core/filterdetect.hxx,oox/core/filterdetect.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/core/relations.hxx,oox/core/relations.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/core/xmlfilterbase.hxx,oox/core/xmlfilterbase.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/drawingml/chart/chartconverter.hxx,oox/drawingml/chart/chartconverter.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/drawingml/table/tablestylelist.hxx,oox/drawingml/table/tablestylelist.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/helper/binarystreambase.hxx,oox/helper/binarystreambase.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/helper/helper.hxx,oox/helper/helper.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/helper/refmap.hxx,oox/helper/refmap.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/helper/refvector.hxx,oox/helper/refvector.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/helper/storagebase.hxx,oox/helper/storagebase.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/helper/zipstorage.hxx,oox/helper/zipstorage.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/ole/vbaproject.hxx,oox/ole/vbaproject.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/vml/vmldrawing.hxx,oox/vml/vmldrawing.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/vml/vmlshape.hxx,oox/vml/vmlshape.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/xls/excelvbaproject.hxx,oox/xls/excelvbaproject.hxx))
