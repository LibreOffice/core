/*************************************************************************
 *
 *  $RCSfile: SchXMLImport.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: bm $ $Date: 2001-03-04 12:30:53 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef SCH_XMLIMPORT_HXX_
#define SCH_XMLIMPORT_HXX_

#ifndef _XMLOFF_SCH_XMLIMPORTHELPER_HXX_
#include "SchXMLImportHelper.hxx"
#endif
#ifndef _XMLOFF_XMLIMP_HXX
#include "xmlimp.hxx"
#endif
#ifndef _XMLOFF_XMLTKMAP_HXX
#include "xmltkmap.hxx"
#endif
#ifndef _XMLOFF_XMLICTXT_HXX
#include "xmlictxt.hxx"
#endif
#ifndef _XMLOFF_PROPERTYHANDLERFACTORY_HXX
#include "prhdlfac.hxx"
#endif
#ifndef _XMLOFF_FAMILIES_HXX_
#include "families.hxx"
#endif

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
    XML_TOK_PA_FLOOR
};

enum SchXMLSeriesElemTokenMap
{
    XML_TOK_SERIES_DATA_POINT,
    XML_TOK_SERIES_DOMAIN
};

// ----------------------------------------

enum SchXMLChartAttrMap
{
    XML_TOK_CHART_CLASS,
    XML_TOK_CHART_WIDTH,
    XML_TOK_CHART_HEIGHT,
    XML_TOK_CHART_STYLE_NAME,
    XML_TOK_CHART_ADDIN_NAME
};

enum SchXMLPlotAreaAttrTokenMap
{
    XML_TOK_PA_X,
    XML_TOK_PA_Y,
    XML_TOK_PA_WIDTH,
    XML_TOK_PA_HEIGHT,
    XML_TOK_PA_STYLE_NAME,
    XML_TOK_PA_TRANSFORM
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
    XML_TOK_SERIES_CHART_CLASS          // not available yet
};

class SchXMLImport : public SvXMLImport
{
private:
    com::sun::star::uno::Reference< com::sun::star::task::XStatusIndicator > mxStatusIndicator;

    SchXMLImportHelper maImportHelper;

protected:
    virtual SvXMLImportContext *CreateContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList >& xAttrList );

public:
    SchXMLImport( sal_uInt16 nImportFlags = IMPORT_ALL );
    SchXMLImport( com::sun::star::uno::Reference< com::sun::star::frame::XModel > xModel,
                  com::sun::star::uno::Reference< com::sun::star::document::XGraphicObjectResolver > &,
                  sal_Bool bLoadDoc, sal_Bool bShowProgress );
    virtual ~SchXMLImport();
};

#endif  // SCH_XMLIMPORT_HXX_
