# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

$(eval $(call gb_Library_Library,xo))

$(eval $(call gb_Library_set_componentfile,xo,xmloff/util/xo))

$(eval $(call gb_Library_set_precompiled_header,xo,$(SRCDIR)/xmloff/inc/pch/precompiled_xo))

$(eval $(call gb_Library_set_include,xo,\
    -I$(SRCDIR)/xmloff/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_add_defs,xo,\
    -DXMLOFF_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_use_external,xo,boost_headers))

$(eval $(call gb_Library_use_custom_headers,xo,\
	officecfg/registry \
))

$(eval $(call gb_Library_use_sdk_api,xo))

$(eval $(call gb_Library_use_libraries,xo,\
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    i18nlangtag \
    sal \
    salhelper \
	sax \
    svl \
    tl \
    utl \
    vcl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_exception_objects,xo,\
    xmloff/source/chart/ColorPropertySet \
    xmloff/source/chart/PropertyMaps \
    xmloff/source/chart/SchXMLAutoStylePoolP \
    xmloff/source/chart/SchXMLAxisContext \
    xmloff/source/chart/SchXMLCalculationSettingsContext \
    xmloff/source/chart/SchXMLChartContext \
    xmloff/source/chart/SchXMLEnumConverter \
    xmloff/source/chart/SchXMLExport \
    xmloff/source/chart/SchXMLImport \
    xmloff/source/chart/SchXMLLegendContext \
    xmloff/source/chart/SchXMLParagraphContext \
    xmloff/source/chart/SchXMLPlotAreaContext \
    xmloff/source/chart/SchXMLRegressionCurveObjectContext \
    xmloff/source/chart/SchXMLSeries2Context \
    xmloff/source/chart/SchXMLSeriesHelper \
    xmloff/source/chart/SchXMLTableContext \
    xmloff/source/chart/SchXMLTextListContext \
    xmloff/source/chart/SchXMLTools \
    xmloff/source/chart/XMLAxisPositionPropertyHdl \
    xmloff/source/chart/XMLChartPropertyContext \
    xmloff/source/chart/XMLChartStyleContext \
    xmloff/source/chart/XMLErrorBarStylePropertyHdl \
    xmloff/source/chart/XMLErrorIndicatorPropertyHdl \
    xmloff/source/chart/XMLLabelSeparatorContext \
    xmloff/source/chart/XMLSymbolImageContext \
    xmloff/source/chart/XMLSymbolTypePropertyHdl \
    xmloff/source/chart/XMLTextOrientationHdl \
    xmloff/source/chart/contexts \
    xmloff/source/chart/transporttypes \
    xmloff/source/core/DocumentSettingsContext \
    xmloff/source/core/DomBuilderContext \
    xmloff/source/core/DomExport \
    xmloff/source/core/ProgressBarHelper \
    xmloff/source/core/PropertySetMerger \
    xmloff/source/core/RDFaExportHelper \
    xmloff/source/core/RDFaImportHelper \
    xmloff/source/core/SettingsExportHelper \
    xmloff/source/core/SvXMLAttr \
    xmloff/source/core/SvXMLAttrCollection \
    xmloff/source/core/XMLBase64Export \
    xmloff/source/core/XMLBase64ImportContext \
    xmloff/source/core/XMLBasicExportFilter \
    xmloff/source/core/XMLEmbeddedObjectExportFilter \
    xmloff/source/core/XMLEmbeddedObjectImportContext \
    xmloff/source/core/attrlist \
    xmloff/source/core/facreg \
    xmloff/source/core/i18nmap \
    xmloff/source/core/nmspmap \
    xmloff/source/core/unoatrcn \
    xmloff/source/core/unointerfacetouniqueidentifiermapper \
    xmloff/source/core/xmlcnimp \
    xmloff/source/core/xmlerror \
    xmloff/source/core/xmlexp \
    xmloff/source/core/xmlictxt \
    xmloff/source/core/xmlimp \
    xmloff/source/core/xmltkmap \
    xmloff/source/core/xmltoken \
    xmloff/source/core/xmluconv \
	xmloff/source/core/xmlmultiimagehelper \
    xmloff/source/draw/EnhancedCustomShapeToken \
    xmloff/source/draw/XMLGraphicsDefaultStyle \
    xmloff/source/draw/XMLImageMapContext \
    xmloff/source/draw/XMLImageMapExport \
    xmloff/source/draw/XMLNumberStyles \
    xmloff/source/draw/XMLReplacementImageContext \
    xmloff/source/draw/XMLShapePropertySetContext \
    xmloff/source/draw/XMLShapeStyleContext \
    xmloff/source/draw/animationexport \
    xmloff/source/draw/animationimport \
    xmloff/source/draw/animexp \
    xmloff/source/draw/animimp \
    xmloff/source/draw/descriptionimp \
    xmloff/source/draw/eventimp \
    xmloff/source/draw/layerexp \
    xmloff/source/draw/layerimp \
    xmloff/source/draw/numithdl \
    xmloff/source/draw/propimp0 \
    xmloff/source/draw/sdpropls \
    xmloff/source/draw/sdxmlexp \
    xmloff/source/draw/sdxmlimp \
    xmloff/source/draw/shapeexport \
    xmloff/source/draw/shapeexport2 \
    xmloff/source/draw/shapeexport3 \
    xmloff/source/draw/shapeexport4 \
    xmloff/source/draw/shapeimport \
    xmloff/source/draw/xexptran \
    xmloff/source/draw/ximp3dobject \
    xmloff/source/draw/ximp3dscene \
    xmloff/source/draw/ximpbody \
    xmloff/source/draw/ximpcustomshape \
    xmloff/source/draw/ximpgrp \
    xmloff/source/draw/ximplink \
    xmloff/source/draw/ximpnote \
    xmloff/source/draw/ximppage \
    xmloff/source/draw/ximpshap \
    xmloff/source/draw/ximpshow \
    xmloff/source/draw/ximpstyl \
    xmloff/source/forms/attriblistmerge \
    xmloff/source/forms/controlelement \
    xmloff/source/forms/controlpropertyhdl \
    xmloff/source/forms/controlpropertymap \
    xmloff/source/forms/elementexport \
    xmloff/source/forms/elementimport \
    xmloff/source/forms/eventexport \
    xmloff/source/forms/eventimport \
    xmloff/source/forms/formattributes \
    xmloff/source/forms/formcellbinding \
    xmloff/source/forms/formenums \
    xmloff/source/forms/formevents \
    xmloff/source/forms/formlayerexport \
    xmloff/source/forms/formlayerimport \
    xmloff/source/forms/gridcolumnproptranslator \
    xmloff/source/forms/layerexport \
    xmloff/source/forms/layerimport \
    xmloff/source/forms/logging \
    xmloff/source/forms/officeforms \
    xmloff/source/forms/propertyexport \
    xmloff/source/forms/propertyimport \
    xmloff/source/forms/property_meta_data \
    xmloff/source/forms/valueproperties \
    xmloff/source/forms/handler/form_handler_factory \
    xmloff/source/forms/handler/property_handler_base \
    xmloff/source/forms/handler/vcl_date_handler \
    xmloff/source/forms/handler/vcl_time_handler \
    xmloff/source/meta/MetaExportComponent \
    xmloff/source/meta/MetaImportComponent \
    xmloff/source/meta/xmlmetae \
    xmloff/source/meta/xmlmetai \
    xmloff/source/meta/xmlversion \
    xmloff/source/script/XMLEventExport \
    xmloff/source/script/XMLEventImportHelper \
    xmloff/source/script/XMLEventsImportContext \
    xmloff/source/script/XMLScriptContextFactory \
    xmloff/source/script/XMLScriptExportHandler \
    xmloff/source/script/XMLStarBasicContextFactory \
    xmloff/source/script/XMLStarBasicExportHandler \
    xmloff/source/script/xmlbasici \
    xmloff/source/script/xmlscripti \
    xmloff/source/style/AttributeContainerHandler \
    xmloff/source/style/DashStyle \
    xmloff/source/style/DrawAspectHdl \
    xmloff/source/style/EnumPropertyHdl \
    xmloff/source/style/FillStyleContext \
    xmloff/source/style/GradientStyle \
    xmloff/source/style/HatchStyle \
    xmloff/source/style/ImageStyle \
    xmloff/source/style/MarkerStyle \
    xmloff/source/style/MultiPropertySetHelper \
    xmloff/source/style/NamedBoolPropertyHdl \
    xmloff/source/style/PageHeaderFooterContext \
    xmloff/source/style/PageMasterExportPropMapper \
    xmloff/source/style/PageMasterImportContext \
    xmloff/source/style/PageMasterImportPropMapper \
    xmloff/source/style/PageMasterPropHdl \
    xmloff/source/style/PageMasterPropHdlFactory \
    xmloff/source/style/PageMasterPropMapper \
    xmloff/source/style/PageMasterStyleMap \
    xmloff/source/style/PagePropertySetContext \
    xmloff/source/style/SinglePropertySetInfoCache \
    xmloff/source/style/StyleMap \
    xmloff/source/style/TransGradientStyle \
    xmloff/source/style/WordWrapPropertyHdl \
    xmloff/source/style/XMLBackgroundImageContext \
    xmloff/source/style/XMLBackgroundImageExport \
    xmloff/source/style/XMLBitmapLogicalSizePropertyHandler \
    xmloff/source/style/XMLBitmapRepeatOffsetPropertyHandler \
    xmloff/source/style/XMLClipPropertyHandler \
    xmloff/source/style/XMLConstantsPropertyHandler \
    xmloff/source/style/XMLElementPropertyContext \
    xmloff/source/style/XMLFillBitmapSizePropertyHandler \
    xmloff/source/style/XMLFontAutoStylePool \
    xmloff/source/style/XMLFontStylesContext \
    xmloff/source/style/XMLFootnoteSeparatorExport \
    xmloff/source/style/XMLFootnoteSeparatorImport \
    xmloff/source/style/XMLIsPercentagePropertyHandler \
    xmloff/source/style/XMLPageExport \
    xmloff/source/style/XMLPercentOrMeasurePropertyHandler \
    xmloff/source/style/XMLRectangleMembersHandler \
    xmloff/source/style/adjushdl \
    xmloff/source/style/backhdl \
    xmloff/source/style/bordrhdl \
    xmloff/source/style/breakhdl \
    xmloff/source/style/cdouthdl \
    xmloff/source/style/chrhghdl \
    xmloff/source/style/chrlohdl \
    xmloff/source/style/csmaphdl \
    xmloff/source/style/durationhdl \
    xmloff/source/style/escphdl \
    xmloff/source/style/fonthdl \
    xmloff/source/style/impastpl \
    xmloff/source/style/impastp2 \
    xmloff/source/style/impastp3 \
    xmloff/source/style/impastp4 \
    xmloff/source/style/kernihdl \
    xmloff/source/style/lspachdl \
    xmloff/source/style/numehelp \
    xmloff/source/style/opaquhdl \
    xmloff/source/style/postuhdl \
    xmloff/source/style/prhdlfac \
    xmloff/source/style/prstylei \
    xmloff/source/style/shadwhdl \
    xmloff/source/style/shdwdhdl \
    xmloff/source/style/styleexp \
    xmloff/source/style/tabsthdl \
    xmloff/source/style/undlihdl \
    xmloff/source/style/uniref \
    xmloff/source/style/weighhdl \
    xmloff/source/style/xmlaustp \
    xmloff/source/style/xmlbahdl \
    xmloff/source/style/xmlexppr \
    xmloff/source/style/xmlimppr \
    xmloff/source/style/xmlnume \
    xmloff/source/style/xmlnumfe \
    xmloff/source/style/xmlnumfi \
    xmloff/source/style/xmlnumi \
    xmloff/source/style/xmlprcon \
    xmloff/source/style/xmlprhdl \
    xmloff/source/style/xmlprmap \
    xmloff/source/style/xmlstyle \
    xmloff/source/style/xmltabe \
    xmloff/source/style/xmltabi \
    xmloff/source/table/XMLTableExport \
    xmloff/source/table/XMLTableImport \
    xmloff/source/text/XMLAutoMarkFileContext \
    xmloff/source/text/XMLAutoTextContainerEventImport \
    xmloff/source/text/XMLAutoTextEventExport \
    xmloff/source/text/XMLAutoTextEventImport \
    xmloff/source/text/XMLCalculationSettingsContext \
    xmloff/source/text/XMLChangeElementImportContext \
    xmloff/source/text/XMLChangeImportContext \
    xmloff/source/text/XMLChangeInfoContext \
    xmloff/source/text/XMLChangedRegionImportContext \
    xmloff/source/text/XMLFootnoteBodyImportContext \
    xmloff/source/text/XMLFootnoteConfigurationImportContext \
    xmloff/source/text/XMLFootnoteImportContext \
    xmloff/source/text/XMLIndexAlphabeticalSourceContext \
    xmloff/source/text/XMLIndexBibliographyConfigurationContext \
    xmloff/source/text/XMLIndexBibliographyEntryContext \
    xmloff/source/text/XMLIndexBibliographySourceContext \
    xmloff/source/text/XMLIndexBodyContext \
    xmloff/source/text/XMLIndexChapterInfoEntryContext \
    xmloff/source/text/XMLIndexIllustrationSourceContext \
    xmloff/source/text/XMLIndexMarkExport \
    xmloff/source/text/XMLIndexObjectSourceContext \
    xmloff/source/text/XMLIndexSimpleEntryContext \
    xmloff/source/text/XMLIndexSourceBaseContext \
    xmloff/source/text/XMLIndexSpanEntryContext \
    xmloff/source/text/XMLIndexTOCContext \
    xmloff/source/text/XMLIndexTOCSourceContext \
    xmloff/source/text/XMLIndexTOCStylesContext \
    xmloff/source/text/XMLIndexTabStopEntryContext \
    xmloff/source/text/XMLIndexTableSourceContext \
    xmloff/source/text/XMLIndexTemplateContext \
    xmloff/source/text/XMLIndexTitleTemplateContext \
    xmloff/source/text/XMLIndexUserSourceContext \
    xmloff/source/text/XMLLineNumberingExport \
    xmloff/source/text/XMLLineNumberingImportContext \
    xmloff/source/text/XMLLineNumberingSeparatorImportContext \
    xmloff/source/text/XMLPropertyBackpatcher \
    xmloff/source/text/XMLRedlineExport \
    xmloff/source/text/XMLSectionExport \
    xmloff/source/text/XMLSectionFootnoteConfigExport \
    xmloff/source/text/XMLSectionFootnoteConfigImport \
    xmloff/source/text/XMLSectionImportContext \
    xmloff/source/text/XMLSectionSourceDDEImportContext \
    xmloff/source/text/XMLSectionSourceImportContext \
    xmloff/source/text/XMLStringBufferImportContext \
    xmloff/source/text/XMLTextCharStyleNamesElementExport \
    xmloff/source/text/XMLTextColumnsContext \
    xmloff/source/text/XMLTextColumnsExport \
    xmloff/source/text/XMLTextFrameContext \
    xmloff/source/text/XMLTextFrameHyperlinkContext \
    xmloff/source/text/XMLTextHeaderFooterContext \
    xmloff/source/text/XMLTextListAutoStylePool \
    xmloff/source/text/XMLTextListBlockContext \
    xmloff/source/text/XMLTextListItemContext \
    xmloff/source/text/XMLTextMarkImportContext \
    xmloff/source/text/XMLTextMasterPageContext \
    xmloff/source/text/XMLTextMasterPageExport \
    xmloff/source/text/XMLTextMasterStylesContext \
    xmloff/source/text/XMLTextNumRuleInfo \
    xmloff/source/text/XMLTextPropertySetContext \
    xmloff/source/text/XMLTextShapeImportHelper \
    xmloff/source/text/XMLTextShapeStyleContext \
    xmloff/source/text/XMLTextTableContext \
    xmloff/source/text/XMLTrackedChangesImportContext \
    xmloff/source/text/txtdrope \
    xmloff/source/text/txtdropi \
    xmloff/source/text/txtexppr \
    xmloff/source/text/txtflde \
    xmloff/source/text/txtfldi \
    xmloff/source/text/txtftne \
    xmloff/source/text/txtimp \
    xmloff/source/text/txtimppr \
    xmloff/source/text/txtlists \
    xmloff/source/text/txtparae \
    xmloff/source/text/txtparai \
    xmloff/source/text/txtprhdl \
    xmloff/source/text/txtprmap \
    xmloff/source/text/txtsecte \
    xmloff/source/text/txtstyle \
    xmloff/source/text/txtstyli \
    xmloff/source/text/txtvfldi \
    xmloff/source/xforms/SchemaContext \
    xmloff/source/xforms/SchemaRestrictionContext \
    xmloff/source/xforms/SchemaSimpleTypeContext \
    xmloff/source/xforms/TokenContext \
    xmloff/source/xforms/XFormsBindContext \
    xmloff/source/xforms/XFormsInstanceContext \
    xmloff/source/xforms/XFormsModelContext \
    xmloff/source/xforms/XFormsSubmissionContext \
    xmloff/source/xforms/xformsapi \
    xmloff/source/xforms/xformsexport \
    xmloff/source/xforms/xformsimport \
))

# vim: set noet sw=4 ts=4:
