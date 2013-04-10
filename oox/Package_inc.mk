# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
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
# Major Contributor(s):
# Copyright (C) 2010 Red Hat, Inc., David Tardon <dtardon@redhat.com>
#  (initial developer)
#
# All Rights Reserved.
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
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/core/binarycodec.hxx,oox/core/binarycodec.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/core/contexthandler.hxx,oox/core/contexthandler.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/core/contexthandler2.hxx,oox/core/contexthandler2.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/core/fragmenthandler.hxx,oox/core/fragmenthandler.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/dllapi.h,oox/dllapi.h))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/drawingml/chart/chartconverter.hxx,oox/drawingml/chart/chartconverter.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/drawingml/table/tablestylelist.hxx,oox/drawingml/table/tablestylelist.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/drawingml/color.hxx,oox/drawingml/color.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/drawingml/theme.hxx,oox/drawingml/theme.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/drawingml/clrscheme.hxx,oox/drawingml/clrscheme.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/drawingml/shape.hxx,oox/drawingml/shape.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/drawingml/drawingmltypes.hxx,oox/drawingml/drawingmltypes.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/drawingml/customshapeproperties.hxx,oox/drawingml/customshapeproperties.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/drawingml/textliststyle.hxx,oox/drawingml/textliststyle.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/drawingml/colorchoicecontext.hxx,oox/drawingml/colorchoicecontext.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/drawingml/textparagraphproperties.hxx,oox/drawingml/textparagraphproperties.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/drawingml/fillproperties.hxx,oox/drawingml/fillproperties.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/drawingml/fillpropertiesgroupcontext.hxx,oox/drawingml/fillpropertiesgroupcontext.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/drawingml/shapepropertiescontext.hxx,oox/drawingml/shapepropertiescontext.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/drawingml/shapegroupcontext.hxx,oox/drawingml/shapegroupcontext.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/drawingml/shapecontext.hxx,oox/drawingml/shapecontext.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/drawingml/embeddedwavaudiofile.hxx,oox/drawingml/embeddedwavaudiofile.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/drawingml/textcharacterproperties.hxx,oox/drawingml/textcharacterproperties.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/drawingml/textspacing.hxx,oox/drawingml/textspacing.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/drawingml/shape3dproperties.hxx,oox/drawingml/shape3dproperties.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/drawingml/connectorshapecontext.hxx,oox/drawingml/connectorshapecontext.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/drawingml/graphicshapecontext.hxx,oox/drawingml/graphicshapecontext.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/drawingml/textfont.hxx,oox/drawingml/textfont.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/drawingml/lineproperties.hxx,oox/drawingml/lineproperties.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/drawingml/shapepropertymap.hxx,oox/drawingml/shapepropertymap.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/drawingml/themefragmenthandler.hxx,oox/drawingml/themefragmenthandler.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/drawingml/chart/datasourcemodel.hxx,oox/drawingml/chart/datasourcemodel.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/drawingml/chart/modelbase.hxx,oox/drawingml/chart/modelbase.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/export/chartexport.hxx,oox/export/chartexport.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/export/drawingml.hxx,oox/export/drawingml.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/export/shapes.hxx,oox/export/shapes.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/export/utils.hxx,oox/export/utils.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/export/vmlexport.hxx,oox/export/vmlexport.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/helper/binarystreambase.hxx,oox/helper/binarystreambase.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/helper/binaryinputstream.hxx,oox/helper/binaryinputstream.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/helper/binaryoutputstream.hxx,oox/helper/binaryoutputstream.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/helper/containerhelper.hxx,oox/helper/containerhelper.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/helper/graphichelper.hxx,oox/helper/graphichelper.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/helper/helper.hxx,oox/helper/helper.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/helper/refmap.hxx,oox/helper/refmap.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/helper/refvector.hxx,oox/helper/refvector.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/helper/storagebase.hxx,oox/helper/storagebase.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/helper/zipstorage.hxx,oox/helper/zipstorage.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/helper/propertymap.hxx,oox/helper/propertymap.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/helper/propertyset.hxx,oox/helper/propertyset.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/helper/attributelist.hxx,oox/helper/attributelist.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/helper/progressbar.hxx,oox/helper/progressbar.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/helper/modelobjecthelper.hxx,oox/helper/modelobjecthelper.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/mathml/export.hxx,oox/mathml/export.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/mathml/import.hxx,oox/mathml/import.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/mathml/importutils.hxx,oox/mathml/importutils.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/ole/olehelper.hxx,oox/ole/olehelper.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/ole/oleobjecthelper.hxx,oox/ole/oleobjecthelper.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/ole/olestorage.hxx,oox/ole/olestorage.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/ole/vbaproject.hxx,oox/ole/vbaproject.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/ole/axcontrol.hxx,oox/ole/axcontrol.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/ole/axbinaryreader.hxx,oox/ole/axbinaryreader.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/ole/axfontdata.hxx,oox/ole/axfontdata.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/vml/vmldrawing.hxx,oox/vml/vmldrawing.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/vml/vmlshape.hxx,oox/vml/vmlshape.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/vml/vmlformatting.hxx,oox/vml/vmlformatting.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/vml/vmldrawingfragment.hxx,oox/vml/vmldrawingfragment.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/vml/vmltextbox.hxx,oox/vml/vmltextbox.hxx))
$(eval $(call gb_Package_add_file,oox_inc,inc/oox/vml/vmlshapecontainer.hxx,oox/vml/vmlshapecontainer.hxx))

# vim: set noet sw=4 ts=4:
