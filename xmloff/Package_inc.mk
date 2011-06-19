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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

$(eval $(call gb_Package_Package,xmloff_inc,$(SRCDIR)/xmloff/inc))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/DashStyle.hxx,xmloff/DashStyle.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/DocumentSettingsContext.hxx,xmloff/DocumentSettingsContext.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/EnumPropertyHdl.hxx,xmloff/EnumPropertyHdl.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/GradientStyle.hxx,xmloff/GradientStyle.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/HatchStyle.hxx,xmloff/HatchStyle.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/ImageStyle.hxx,xmloff/ImageStyle.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/MarkerStyle.hxx,xmloff/MarkerStyle.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/NamedBoolPropertyHdl.hxx,xmloff/NamedBoolPropertyHdl.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/PageMasterStyleMap.hxx,xmloff/PageMasterStyleMap.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/ProgressBarHelper.hxx,xmloff/ProgressBarHelper.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/PropertySetInfoHash.hxx,xmloff/PropertySetInfoHash.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/PropertySetInfoKey.hxx,xmloff/PropertySetInfoKey.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/SchXMLExportHelper.hxx,xmloff/SchXMLExportHelper.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/SchXMLImportHelper.hxx,xmloff/SchXMLImportHelper.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/SettingsExportHelper.hxx,xmloff/SettingsExportHelper.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/SinglePropertySetInfoCache.hxx,xmloff/SinglePropertySetInfoCache.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/VisAreaContext.hxx,xmloff/VisAreaContext.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/VisAreaExport.hxx,xmloff/VisAreaExport.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/WordWrapPropertyHdl.hxx,xmloff/WordWrapPropertyHdl.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/XMLBase64ImportContext.hxx,xmloff/XMLBase64ImportContext.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/XMLCharContext.hxx,xmloff/XMLCharContext.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/XMLConstantsPropertyHandler.hxx,xmloff/XMLConstantsPropertyHandler.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/XMLEmbeddedObjectExportFilter.hxx,xmloff/XMLEmbeddedObjectExportFilter.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/XMLEventExport.hxx,xmloff/XMLEventExport.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/XMLEventsImportContext.hxx,xmloff/XMLEventsImportContext.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/XMLFilterServiceNames.h,xmloff/XMLFilterServiceNames.h))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/XMLFontAutoStylePool.hxx,xmloff/XMLFontAutoStylePool.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/XMLFontStylesContext.hxx,xmloff/XMLFontStylesContext.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/XMLGraphicsDefaultStyle.hxx,xmloff/XMLGraphicsDefaultStyle.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/XMLPageExport.hxx,xmloff/XMLPageExport.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/XMLSettingsExportContext.hxx,xmloff/XMLSettingsExportContext.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/XMLShapeStyleContext.hxx,xmloff/XMLShapeStyleContext.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/XMLStringVector.hxx,xmloff/XMLStringVector.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/XMLTextListAutoStylePool.hxx,xmloff/XMLTextListAutoStylePool.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/XMLTextMasterPageContext.hxx,xmloff/XMLTextMasterPageContext.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/XMLTextMasterPageExport.hxx,xmloff/XMLTextMasterPageExport.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/XMLTextMasterStylesContext.hxx,xmloff/XMLTextMasterStylesContext.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/XMLTextShapeImportHelper.hxx,xmloff/XMLTextShapeImportHelper.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/XMLTextShapeImportHelper.hxx,xmloff/XMLTextShapeImportHelper.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/XMLTextShapeStyleContext.hxx,xmloff/XMLTextShapeStyleContext.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/XMLTextTableContext.hxx,xmloff/XMLTextTableContext.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/animexp.hxx,xmloff/animexp.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/attrlist.hxx,xmloff/attrlist.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/contextid.hxx,xmloff/contextid.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/controlpropertyhdl.hxx,xmloff/controlpropertyhdl.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/dllapi.h,xmloff/dllapi.h))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/families.hxx,xmloff/families.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/formlayerexport.hxx,xmloff/formlayerexport.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/formlayerimport.hxx,xmloff/formlayerimport.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/formsimp.hxx,xmloff/formsimp.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/functional.hxx,xmloff/functional.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/i18nmap.hxx,xmloff/i18nmap.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/maptype.hxx,xmloff/maptype.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/nmspmap.hxx,xmloff/nmspmap.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/numehelp.hxx,xmloff/numehelp.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/odffields.hxx,xmloff/odffields.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/prhdlfac.hxx,xmloff/prhdlfac.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/prstylei.hxx,xmloff/prstylei.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/shapeexport.hxx,xmloff/shapeexport.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/shapeimport.hxx,xmloff/shapeimport.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/styleexp.hxx,xmloff/styleexp.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/table/XMLTableExport.hxx,xmloff/table/XMLTableExport.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/table/XMLTableImport.hxx,xmloff/table/XMLTableImport.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/txtimp.hxx,xmloff/txtimp.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/txtimppr.hxx,xmloff/txtimppr.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/txtparae.hxx,xmloff/txtparae.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/txtprmap.hxx,xmloff/txtprmap.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/txtstyle.hxx,xmloff/txtstyle.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/txtstyli.hxx,xmloff/txtstyli.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/uniref.hxx,xmloff/uniref.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/unoatrcn.hxx,xmloff/unoatrcn.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/xformsexport.hxx,xmloff/xformsexport.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/xformsimport.hxx,xmloff/xformsimport.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/xmlaustp.hxx,xmloff/xmlaustp.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/xmlcnimp.hxx,xmloff/xmlcnimp.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/xmlcnitm.hxx,xmloff/xmlcnitm.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/xmlement.hxx,xmloff/xmlement.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/xmlerror.hxx,xmloff/xmlerror.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/xmlevent.hxx,xmloff/xmlevent.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/xmlexp.hxx,xmloff/xmlexp.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/xmlexppr.hxx,xmloff/xmlexppr.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/xmlictxt.hxx,xmloff/xmlictxt.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/xmlimp.hxx,xmloff/xmlimp.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/xmlimppr.hxx,xmloff/xmlimppr.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/xmlkywd.hxx,xmloff/xmlkywd.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/xmlmetae.hxx,xmloff/xmlmetae.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/xmlmetai.hxx,xmloff/xmlmetai.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/xmlnmspe.hxx,xmloff/xmlnmspe.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/xmlnume.hxx,xmloff/xmlnume.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/xmlnumfe.hxx,xmloff/xmlnumfe.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/xmlnumfi.hxx,xmloff/xmlnumfi.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/xmlnumi.hxx,xmloff/xmlnumi.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/xmlprcon.hxx,xmloff/xmlprcon.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/xmlprhdl.hxx,xmloff/xmlprhdl.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/xmlprmap.hxx,xmloff/xmlprmap.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/xmlreg.hxx,xmloff/xmlreg.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/xmlscripti.hxx,xmloff/xmlscripti.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/xmlstyle.hxx,xmloff/xmlstyle.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/xmltabe.hxx,xmloff/xmltabe.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/xmltkmap.hxx,xmloff/xmltkmap.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/xmltoken.hxx,xmloff/xmltoken.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/xmltypes.hxx,xmloff/xmltypes.hxx))
$(eval $(call gb_Package_add_file,xmloff_inc,inc/xmloff/xmluconv.hxx,xmloff/xmluconv.hxx))
