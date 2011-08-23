/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef SCH_XMLIMPORT_HXX_
#define SCH_XMLIMPORT_HXX_

#ifndef _XMLOFF_SCH_XMLIMPORTHELPER_HXX_
#include <bf_xmloff/SchXMLImportHelper.hxx>
#endif
#ifndef _XMLOFF_XMLIMP_HXX
#include <bf_xmloff/xmlimp.hxx>
#endif
#ifndef _XMLOFF_XMLTKMAP_HXX
#include <bf_xmloff/xmltkmap.hxx>
#endif
#ifndef _XMLOFF_XMLICTXT_HXX 
#include <bf_xmloff/xmlictxt.hxx>
#endif
#ifndef _XMLOFF_PROPERTYHANDLERFACTORY_HXX
#include <bf_xmloff/prhdlfac.hxx>
#endif
#ifndef _XMLOFF_FAMILIES_HXX_
#include <bf_xmloff/families.hxx>
#endif
namespace binfilter {

/* ----------------------------------------
   these enums are used for the
   SvXMLTokenMapEntries to distinguish
   the tokens in switch-directives
   ----------------------------------------*/

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
    XML_TOK_PA_AXIS,
    XML_TOK_PA_SERIES,
    XML_TOK_PA_CATEGORIES,
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

// ----------------------------------------

enum SchXMLChartAttrMap
{
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
    XML_TOK_PA_DS_HAS_LABELS
};

enum SchXMLAxisAttrTokenMap
{
    XML_TOK_AXIS_CLASS,
    XML_TOK_AXIS_NAME,
    XML_TOK_AXIS_STYLE_NAME
};

enum SchXMLLegendAttrMap
{
    XML_TOK_LEGEND_POSITION,
    XML_TOK_LEGEND_X,
    XML_TOK_LEGEND_Y,
    XML_TOK_LEGEND_STYLE_NAME
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
    XML_TOK_SERIES_CHART_CLASS			// not available yet
};

class SchXMLImport : public SvXMLImport
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::task::XStatusIndicator > mxStatusIndicator;

    SchXMLImportHelper maImportHelper;

protected:
    virtual SvXMLImportContext *CreateContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

public:
    // #110680#
    SchXMLImport( 
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
        sal_uInt16 nImportFlags = IMPORT_ALL );

    virtual ~SchXMLImport() throw ();

    // XServiceInfo ( : SvXMLExport )
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException );

    SvXMLImportContext* CreateStylesContext( const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
};

}//end of namespace binfilter
#endif	// SCH_XMLIMPORT_HXX_
