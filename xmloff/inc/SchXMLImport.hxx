/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef SCH_XMLIMPORT_HXX_
#define SCH_XMLIMPORT_HXX_

#include <xmloff/SchXMLImportHelper.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmltkmap.hxx>
#include <xmloff/xmlictxt.hxx>
#include <xmloff/prhdlfac.hxx>
#include <xmloff/families.hxx>

/*
   these enums are used for the
   SvXMLTokenMapEntries to distinguish
   the tokens in switch-directives
*/

enum SchXMLDocElemTokenMap
{
    XML_TOK_DOC_AUTOSTYLES,
    XML_TOK_DOC_STYLES,
    XML_TOK_DOC_META,
    XML_TOK_DOC_BODY
};

enum SchXMLTableElemTokenMap
{
    XML_TOK_TABLE_HEADER_COLS,
    XML_TOK_TABLE_COLUMNS,
    XML_TOK_TABLE_COLUMN,
    XML_TOK_TABLE_HEADER_ROWS,
    XML_TOK_TABLE_ROWS,
    XML_TOK_TABLE_ROW
};

enum SchXMLChartElemTokenMap
{
    XML_TOK_CHART_PLOT_AREA,
    XML_TOK_CHART_TITLE,
    XML_TOK_CHART_SUBTITLE,
    XML_TOK_CHART_LEGEND,
    XML_TOK_CHART_TABLE
};

enum SchXMLPlotAreaElemTokenMap
{
    XML_TOK_PA_COORDINATE_REGION_EXT,
    XML_TOK_PA_COORDINATE_REGION,
    XML_TOK_PA_AXIS,
    XML_TOK_PA_SERIES,
    XML_TOK_PA_WALL,
    XML_TOK_PA_FLOOR,
    XML_TOK_PA_LIGHT_SOURCE,
    XML_TOK_PA_STOCK_GAIN,
    XML_TOK_PA_STOCK_LOSS,
    XML_TOK_PA_STOCK_RANGE
};

enum SchXMLSeriesElemTokenMap
{
    XML_TOK_SERIES_DATA_POINT,
    XML_TOK_SERIES_DOMAIN,
    XML_TOK_SERIES_MEAN_VALUE_LINE,
    XML_TOK_SERIES_REGRESSION_CURVE,
    XML_TOK_SERIES_ERROR_INDICATOR
};

enum SchXMLChartAttrMap
{
    XML_TOK_CHART_HREF,
    XML_TOK_CHART_CLASS,
    XML_TOK_CHART_WIDTH,
    XML_TOK_CHART_HEIGHT,
    XML_TOK_CHART_STYLE_NAME,
    XML_TOK_CHART_ADDIN_NAME,
    XML_TOK_CHART_COL_MAPPING,
    XML_TOK_CHART_ROW_MAPPING
};

enum SchXMLPlotAreaAttrTokenMap
{
    XML_TOK_PA_X,
    XML_TOK_PA_Y,
    XML_TOK_PA_WIDTH,
    XML_TOK_PA_HEIGHT,
    XML_TOK_PA_STYLE_NAME,
    XML_TOK_PA_TRANSFORM,
    XML_TOK_PA_CHART_ADDRESS,
    XML_TOK_PA_TABLE_NUMBER_LIST,
    XML_TOK_PA_DS_HAS_LABELS,
    XML_TOK_PA_VRP,
    XML_TOK_PA_VPN,
    XML_TOK_PA_VUP,
    XML_TOK_PA_PROJECTION,
    XML_TOK_PA_DISTANCE,
    XML_TOK_PA_FOCAL_LENGTH,
    XML_TOK_PA_SHADOW_SLANT,
    XML_TOK_PA_SHADE_MODE,
    XML_TOK_PA_AMBIENT_COLOR,
    XML_TOK_PA_LIGHTING_MODE
};

enum SchXMLAutoStyleAttrMap
{
    XML_TOK_AS_FAMILY,
    XML_TOK_AS_NAME
};

enum SchXMLCellAttrMap
{
    XML_TOK_CELL_VAL_TYPE,
    XML_TOK_CELL_VALUE
};

enum SchXMLSeriesAttrMap
{
    XML_TOK_SERIES_CELL_RANGE,
    XML_TOK_SERIES_LABEL_ADDRESS,
    XML_TOK_SERIES_ATTACHED_AXIS,
    XML_TOK_SERIES_STYLE_NAME,
    XML_TOK_SERIES_CHART_CLASS
};

enum SchXMLRegEquationAttrMap
{
    XML_TOK_REGEQ_STYLE_NAME,
    XML_TOK_REGEQ_DISPLAY_EQUATION,
    XML_TOK_REGEQ_DISPLAY_R_SQUARE,
    XML_TOK_REGEQ_POS_X,
    XML_TOK_REGEQ_POS_Y
};

class SchXMLImport : public SvXMLImport
{
private:
    com::sun::star::uno::Reference< com::sun::star::task::XStatusIndicator > mxStatusIndicator;

    SchXMLImportHelper maImportHelper;

protected:
    virtual SvXMLImportContext *CreateContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );

public:
    // #110680#
    SchXMLImport(
        const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext,
        sal_uInt16 nImportFlags = IMPORT_ALL );

    virtual ~SchXMLImport() throw ();

    // XServiceInfo ( : SvXMLExport )
    virtual OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException );

    SvXMLImportContext* CreateStylesContext( const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );

    // XImporter
    virtual void SAL_CALL setTargetDocument( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& xDoc ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
};

#endif  // SCH_XMLIMPORT_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
