# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Initial Developer of the Original Code is
#       David Tardon, Red Hat Inc. <dtardon@redhat.com>
# Portions created by the Initial Developer are Copyright (C) 2010 the
# Initial Developer. All Rights Reserved.
#
# Major Contributor(s):
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Package_Package,oox_inc,$(SRCDIR)/oox/inc))

$(eval $(call gb_Package_add_file,oox_inc,inc/oox/core/filterbase.hxx,oox/core/filterbase.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/core/filterdetect.hxx,oox/core/filterdetect.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/core/fragmenthandler2.hxx,oox/core/fragmenthandler2.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/core/relations.hxx,oox/core/relations.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/core/xmlfilterbase.hxx,oox/core/xmlfilterbase.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/dllapi.h,oox/dllapi.h))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/drawingml/chart/chartconverter.hxx,oox/drawingml/chart/chartconverter.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/drawingml/table/tablestylelist.hxx,oox/drawingml/table/tablestylelist.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/export/chartexport.hxx,oox/export/chartexport.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/export/drawingml.hxx,oox/export/drawingml.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/export/shapes.hxx,oox/export/shapes.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/export/utils.hxx,oox/export/utils.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/export/vmlexport.hxx,oox/export/vmlexport.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/helper/binarystreambase.hxx,oox/helper/binarystreambase.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/helper/containerhelper.hxx,oox/helper/containerhelper.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/helper/graphichelper.hxx,oox/helper/graphichelper.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/helper/helper.hxx,oox/helper/helper.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/helper/refmap.hxx,oox/helper/refmap.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/helper/refvector.hxx,oox/helper/refvector.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/helper/storagebase.hxx,oox/helper/storagebase.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/helper/zipstorage.hxx,oox/helper/zipstorage.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/ole/olehelper.hxx,oox/ole/olehelper.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/ole/oleobjecthelper.hxx,oox/ole/oleobjecthelper.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/ole/olestorage.hxx,oox/ole/olestorage.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/ole/vbaproject.hxx,oox/ole/vbaproject.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/vml/vmldrawing.hxx,oox/vml/vmldrawing.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/vml/vmlshape.hxx,oox/vml/vmlshape.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/xls/excelvbaproject.hxx,oox/xls/excelvbaproject.hxx))

# vim: set noet sw=4 ts=4:
