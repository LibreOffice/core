/*************************************************************************
 *
 *  $RCSfile: SchXMLImport.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: cl $ $Date: 2001-03-04 17:48:54 $
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

#include "SchXMLImport.hxx"
#include "SchXMLChartContext.hxx"
#include "contexts.hxx"
#include "XMLChartPropertySetMapper.hxx"

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif
#ifndef _XMLOFF_XMLICTXT_HXX
#include "xmlictxt.hxx"
#endif
#ifndef _XMLOFF_XMLSTYLE_HXX
#include "xmlstyle.hxx"
#endif

#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATORSUPPLIER_HPP_
#include <com/sun/star/task/XStatusIndicatorSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XSTRINGMAPPING_HPP_
#include <com/sun/star/util/XStringMapping.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_XCHARTDOCUMENT_HPP_
#include <com/sun/star/chart/XChartDocument.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_XCHARTDATAARRAY_HPP_
#include <com/sun/star/chart/XChartDataArray.hpp>
#endif

using namespace rtl;
using namespace com::sun::star;

/* ----------------------------------------
   TokenMaps for distinguishing different
   tokens in different contexts
   ----------------------------------------*/

// ----------------------------------------
// element maps
// ----------------------------------------

static __FAR_DATA SvXMLTokenMapEntry aDocElemTokenMap[] =
{
    { XML_NAMESPACE_OFFICE, sXML_automatic_styles,  XML_TOK_DOC_AUTOSTYLES  },
    { XML_NAMESPACE_OFFICE, sXML_styles,            XML_TOK_DOC_STYLES  },
    { XML_NAMESPACE_OFFICE, sXML_meta,              XML_TOK_DOC_META    },
    { XML_NAMESPACE_OFFICE, sXML_body,              XML_TOK_DOC_BODY    },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aTableElemTokenMap[] =
{
    { XML_NAMESPACE_TABLE,  sXML_table_header_columns,  XML_TOK_TABLE_HEADER_COLS   },
    { XML_NAMESPACE_TABLE,  sXML_table_columns,         XML_TOK_TABLE_COLUMNS       },
    { XML_NAMESPACE_TABLE,  sXML_table_column,          XML_TOK_TABLE_COLUMN        },
    { XML_NAMESPACE_TABLE,  sXML_table_header_rows,     XML_TOK_TABLE_HEADER_ROWS   },
    { XML_NAMESPACE_TABLE,  sXML_table_rows,            XML_TOK_TABLE_ROWS          },
    { XML_NAMESPACE_TABLE,  sXML_table_row,             XML_TOK_TABLE_ROW           },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aChartElemTokenMap[] =
{
    { XML_NAMESPACE_CHART,  sXML_plot_area,             XML_TOK_CHART_PLOT_AREA     },
    { XML_NAMESPACE_CHART,  sXML_title,                 XML_TOK_CHART_TITLE         },
    { XML_NAMESPACE_CHART,  sXML_subtitle,              XML_TOK_CHART_SUBTITLE      },
    { XML_NAMESPACE_CHART,  sXML_legend,                XML_TOK_CHART_LEGEND        },
    { XML_NAMESPACE_TABLE,  sXML_table,                 XML_TOK_CHART_TABLE         },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aPlotAreaElemTokenMap[] =
{
    { XML_NAMESPACE_CHART,  sXML_axis,                  XML_TOK_PA_AXIS             },
    { XML_NAMESPACE_CHART,  sXML_series,                XML_TOK_PA_SERIES           },
    { XML_NAMESPACE_CHART,  sXML_categories,            XML_TOK_PA_CATEGORIES       },
    { XML_NAMESPACE_CHART,  sXML_wall,                  XML_TOK_PA_WALL             },
    { XML_NAMESPACE_CHART,  sXML_floor,                 XML_TOK_PA_FLOOR            },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aSeriesElemTokenMap[] =
{
    { XML_NAMESPACE_CHART,  sXML_data_point,    XML_TOK_SERIES_DATA_POINT   },
    { XML_NAMESPACE_CHART,  sXML_domain,        XML_TOK_SERIES_DOMAIN       },
    XML_TOKEN_MAP_END
};

// ----------------------------------------
// attribute maps
// ----------------------------------------

static __FAR_DATA SvXMLTokenMapEntry aChartAttrTokenMap[] =
{
    { XML_NAMESPACE_CHART,  sXML_class,                 XML_TOK_CHART_CLASS         },
    { XML_NAMESPACE_SVG,    sXML_width,                 XML_TOK_CHART_WIDTH         },
    { XML_NAMESPACE_SVG,    sXML_height,                XML_TOK_CHART_HEIGHT        },
    { XML_NAMESPACE_CHART,  sXML_style_name,            XML_TOK_CHART_STYLE_NAME    },
    { XML_NAMESPACE_CHART,  sXML_add_in_name,           XML_TOK_CHART_ADDIN_NAME    },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aPlotAreaAttrTokenMap[] =
{
    { XML_NAMESPACE_SVG,    sXML_x,                     XML_TOK_PA_X            },
    { XML_NAMESPACE_SVG,    sXML_y,                     XML_TOK_PA_Y            },
    { XML_NAMESPACE_SVG,    sXML_width,                 XML_TOK_PA_WIDTH        },
    { XML_NAMESPACE_SVG,    sXML_height,                XML_TOK_PA_HEIGHT       },
    { XML_NAMESPACE_CHART,  sXML_style_name,            XML_TOK_PA_STYLE_NAME   },
    { XML_NAMESPACE_DR3D,   sXML_transform,             XML_TOK_PA_TRANSFORM    },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aAxisAttrTokenMap[] =
{
    { XML_NAMESPACE_CHART,  sXML_class,                 XML_TOK_AXIS_CLASS          },
    { XML_NAMESPACE_CHART,  sXML_name,                  XML_TOK_AXIS_NAME           },
    { XML_NAMESPACE_CHART,  sXML_style_name,            XML_TOK_AXIS_STYLE_NAME     },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aLegendAttrTokenMap[] =
{
    { XML_NAMESPACE_CHART,  sXML_legend_position,       XML_TOK_LEGEND_POSITION     },
    { XML_NAMESPACE_SVG,    sXML_x,                     XML_TOK_LEGEND_X            },
    { XML_NAMESPACE_SVG,    sXML_y,                     XML_TOK_LEGEND_Y            },
    { XML_NAMESPACE_CHART,  sXML_style_name,            XML_TOK_LEGEND_STYLE_NAME   },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aAutoStyleAttrTokenMap[] =
{
    { XML_NAMESPACE_STYLE,  sXML_family,                XML_TOK_AS_FAMILY           },
    { XML_NAMESPACE_STYLE,  sXML_name,                  XML_TOK_AS_NAME             },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aCellAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE,  sXML_value_type,            XML_TOK_CELL_VAL_TYPE       },
    { XML_NAMESPACE_TABLE,  sXML_value,                 XML_TOK_CELL_VALUE          },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aSeriesAttrTokenMap[] =
{
    { XML_NAMESPACE_CHART,  sXML_values_cell_range_address, XML_TOK_SERIES_CELL_RANGE    },
    { XML_NAMESPACE_CHART,  sXML_label_cell_address,        XML_TOK_SERIES_LABEL_ADDRESS },
    { XML_NAMESPACE_CHART,  sXML_attached_axis,             XML_TOK_SERIES_ATTACHED_AXIS },
    { XML_NAMESPACE_CHART,  sXML_style_name,                XML_TOK_SERIES_STYLE_NAME    },
    { XML_NAMESPACE_CHART,  sXML_class,                     XML_TOK_SERIES_CHART_CLASS   },
    XML_TOKEN_MAP_END
};

// ========================================

SchXMLImportHelper::SchXMLImportHelper() :
        mpAutoStyles( 0 ),

        mpDocElemTokenMap( 0 ),
        mpTableElemTokenMap( 0 ),
        mpChartElemTokenMap( 0 ),
        mpPlotAreaElemTokenMap( 0 ),
        mpSeriesElemTokenMap( 0 ),

        mpChartAttrTokenMap( 0 ),
        mpPlotAreaAttrTokenMap( 0 ),
        mpAxisAttrTokenMap( 0 ),
        mpLegendAttrTokenMap( 0 ),
        mpAutoStyleAttrTokenMap( 0 ),
        mpCellAttrTokenMap( 0 ),
        mpSeriesAttrTokenMap( 0 )
{
}

SchXMLImportHelper::~SchXMLImportHelper()
{
        // delete token maps
    if( mpDocElemTokenMap )
        delete mpDocElemTokenMap;
    if( mpTableElemTokenMap )
        delete mpTableElemTokenMap;
    if( mpChartElemTokenMap )
        delete mpChartElemTokenMap;
    if( mpPlotAreaElemTokenMap )
        delete mpPlotAreaElemTokenMap;
    if( mpSeriesElemTokenMap )
        delete mpSeriesElemTokenMap;

    if( mpChartAttrTokenMap )
        delete mpChartAttrTokenMap;
    if( mpPlotAreaAttrTokenMap )
        delete mpPlotAreaAttrTokenMap;
    if( mpAxisAttrTokenMap )
        delete mpAxisAttrTokenMap;
    if( mpLegendAttrTokenMap )
        delete mpLegendAttrTokenMap;
    if( mpAutoStyleAttrTokenMap )
        delete mpAutoStyleAttrTokenMap;
    if( mpCellAttrTokenMap )
        delete mpCellAttrTokenMap;
    if( mpSeriesAttrTokenMap )
        delete mpSeriesAttrTokenMap;
}

SvXMLImportContext* SchXMLImportHelper::CreateChartContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrefix, const rtl::OUString& rLocalName,
    const uno::Reference< frame::XModel > xChartModel,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    SvXMLImportContext* pContext = 0;

    uno::Reference< chart::XChartDocument > xDoc( xChartModel, uno::UNO_QUERY );
    if( xDoc.is())
    {
        mxChartDoc = xDoc;
        pContext = new SchXMLChartContext( *this, rImport, rLocalName );
    }
    else
    {
        DBG_ERROR( "No valid XChartDocument given as XModel" );
        pContext = new SvXMLImportContext( rImport, nPrefix, rLocalName );
    }

    return pContext;
}

/* ----------------------------------------
   get various token maps
   ----------------------------------------*/

const SvXMLTokenMap& SchXMLImportHelper::GetDocElemTokenMap()
{
    if( ! mpDocElemTokenMap )
        mpDocElemTokenMap = new SvXMLTokenMap( aDocElemTokenMap );
    return *mpDocElemTokenMap;
}

const SvXMLTokenMap& SchXMLImportHelper::GetTableElemTokenMap()
{
    if( ! mpTableElemTokenMap )
        mpTableElemTokenMap = new SvXMLTokenMap( aTableElemTokenMap );
    return *mpTableElemTokenMap;
}

const SvXMLTokenMap& SchXMLImportHelper::GetChartElemTokenMap()
{
    if( ! mpChartElemTokenMap )
        mpChartElemTokenMap = new SvXMLTokenMap( aChartElemTokenMap );
    return *mpChartElemTokenMap;
}

const SvXMLTokenMap& SchXMLImportHelper::GetPlotAreaElemTokenMap()
{
    if( ! mpPlotAreaElemTokenMap )
        mpPlotAreaElemTokenMap = new SvXMLTokenMap( aPlotAreaElemTokenMap );
    return *mpPlotAreaElemTokenMap;
}

const SvXMLTokenMap& SchXMLImportHelper::GetSeriesElemTokenMap()
{
    if( ! mpSeriesElemTokenMap )
        mpSeriesElemTokenMap = new SvXMLTokenMap( aSeriesElemTokenMap );
    return *mpSeriesElemTokenMap;
}

// ----------------------------------------

const SvXMLTokenMap& SchXMLImportHelper::GetChartAttrTokenMap()
{
    if( ! mpChartAttrTokenMap )
        mpChartAttrTokenMap = new SvXMLTokenMap( aChartAttrTokenMap );
    return *mpChartAttrTokenMap;
}

const SvXMLTokenMap& SchXMLImportHelper::GetPlotAreaAttrTokenMap()
{
    if( ! mpPlotAreaAttrTokenMap )
        mpPlotAreaAttrTokenMap = new SvXMLTokenMap( aPlotAreaAttrTokenMap );
    return *mpPlotAreaAttrTokenMap;
}

const SvXMLTokenMap& SchXMLImportHelper::GetAxisAttrTokenMap()
{
    if( ! mpAxisAttrTokenMap )
        mpAxisAttrTokenMap = new SvXMLTokenMap( aAxisAttrTokenMap );
    return *mpAxisAttrTokenMap;
}

const SvXMLTokenMap& SchXMLImportHelper::GetLegendAttrTokenMap()
{
    if( ! mpLegendAttrTokenMap )
        mpLegendAttrTokenMap = new SvXMLTokenMap( aLegendAttrTokenMap );
    return *mpLegendAttrTokenMap;
}

const SvXMLTokenMap& SchXMLImportHelper::GetAutoStyleAttrTokenMap()
{
    if( ! mpAutoStyleAttrTokenMap )
        mpAutoStyleAttrTokenMap = new SvXMLTokenMap( aAutoStyleAttrTokenMap );
    return *mpAutoStyleAttrTokenMap;
}

const SvXMLTokenMap& SchXMLImportHelper::GetCellAttrTokenMap()
{
    if( ! mpCellAttrTokenMap )
        mpCellAttrTokenMap = new SvXMLTokenMap( aCellAttrTokenMap );
    return *mpCellAttrTokenMap;
}

const SvXMLTokenMap& SchXMLImportHelper::GetSeriesAttrTokenMap()
{
    if( ! mpSeriesAttrTokenMap )
        mpSeriesAttrTokenMap = new SvXMLTokenMap( aSeriesAttrTokenMap );
    return *mpSeriesAttrTokenMap;
}

// ----------------------------------------

// -1 means don't change
void SchXMLImportHelper::ResizeChartData( sal_Int32 nSeries, sal_Int32 nDataPoints )
{
    if( mxChartDoc.is())
    {
        sal_Bool bWasChanged = sal_False;

        // chart data is an array of rows,
        // a series represents always a column
        sal_Int32 nColCount = nSeries;
        sal_Int32 nRowCount = nDataPoints;

        uno::Reference< chart::XChartDataArray > xData( mxChartDoc->getData(), uno::UNO_QUERY );
        if( xData.is())
        {
            uno::Sequence< uno::Sequence< double > > xArray = xData->getData();

            // increase number of rows
            if( xArray.getLength() < nRowCount )
            {
                sal_Int32 nOldLen = xArray.getLength();
                xArray.realloc( nRowCount );
                if( nColCount == -1 )
                {
                    sal_Int32 nSize = xArray[ 0 ].getLength();
                    for( sal_Int32 i = nOldLen; i < nRowCount; i++ )
                        xArray[ i ].realloc( nSize );
                }
                bWasChanged = sal_True;
            }

            if( nSeries == -1 &&
                nRowCount > 0 )
                nColCount = xArray[ 0 ].getLength();

            // columns
            if( nColCount > 0 &&
                xArray[ 0 ].getLength() < nColCount )
            {
                if( nDataPoints == -1 )
                    nRowCount = xArray.getLength();

                for( sal_Int32 i = 0; i < nRowCount; i++ )
                    xArray[ i ].realloc( nColCount );
                bWasChanged = sal_True;
            }

            if( bWasChanged )
            {
                xData->setData( xArray );

                uno::Reference< chart::XChartData > xChData( xData, uno::UNO_QUERY );
                if( xChData.is())
                    mxChartDoc->attachData( xChData );
            }
        }
    }
}

// ========================================

SchXMLImport::SchXMLImport( sal_uInt16 nImportFlags ) :
        SvXMLImport( nImportFlags )
{
}

SchXMLImport::SchXMLImport( uno::Reference< frame::XModel > xModel,
                            uno::Reference< com::sun::star::document::XGraphicObjectResolver >& rGrfContainer,
                            sal_Bool bLoadDoc, sal_Bool bShowProgress ) :
        SvXMLImport( xModel, rGrfContainer )
{
    // get status indicator (if requested)
    if( bShowProgress )
    {
        uno::Reference< frame::XController > xController( xModel->getCurrentController());
        if( xController.is())
        {
            uno::Reference< frame::XFrame > xFrame( xController->getFrame());
            if( xFrame.is())
            {
                uno::Reference< task::XStatusIndicatorSupplier > xFactory( xFrame, uno::UNO_QUERY );
                if( xFactory.is())
                {
                    mxStatusIndicator = xFactory->getStatusIndicator();
                }
            }
        }
    }

    // add progress view
    if( mxStatusIndicator.is())
    {
        const rtl::OUString aText( RTL_CONSTASCII_USTRINGPARAM( "XML Import" ));
        mxStatusIndicator->start( aText, 100 );     // use percentage as values
    }
}

SchXMLImport::~SchXMLImport()
{
    // stop progress view
    if( mxStatusIndicator.is())
    {
        mxStatusIndicator->end();
        mxStatusIndicator->reset();
    }
}

// create the main context (subcontexts are created
// by the one created here)
SvXMLImportContext *SchXMLImport::CreateContext( USHORT nPrefix, const rtl::OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    SvXMLImportContext* pContext = 0;

    // accept <office:document>
    if( XML_NAMESPACE_OFFICE == nPrefix &&
        ( 0 == rLocalName.compareToAscii(sXML_document) ||
          0 == rLocalName.compareToAscii(sXML_document_meta) ||
          0 == rLocalName.compareToAscii(sXML_document_styles) ||
          0 == rLocalName.compareToAscii(sXML_document_content) ))
    {
        pContext = new SchXMLDocContext( maImportHelper, *this, nPrefix, rLocalName );
    }
    else
    {
        pContext = SvXMLImport::CreateContext( nPrefix, rLocalName, xAttrList );
    }

    return pContext;
}

// export components ========================================

// first version: everything comes from one storage

uno::Sequence< OUString > SAL_CALL SchXMLImport_getSupportedServiceNames() throw()
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.office.sax.importer.Chart" ) );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

OUString SAL_CALL SchXMLImport_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SchXMLImport" ) );
}

uno::Reference< uno::XInterface > SAL_CALL SchXMLImport_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception )
{
    return (cppu::OWeakObject*)new SchXMLImport();
}

// ============================================================

// multiple storage version: one for content / styles / meta

uno::Sequence< OUString > SAL_CALL SchXMLImport_Styles_getSupportedServiceNames() throw()
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Chart.XMLStylesImporter" ) );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

OUString SAL_CALL SchXMLImport_Styles_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SchXMLImport.Styles" ) );
}

uno::Reference< uno::XInterface > SAL_CALL SchXMLImport_Styles_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception )
{
    return (cppu::OWeakObject*)new SchXMLImport( IMPORT_STYLES );
}

// ------------------------------------------------------------

uno::Sequence< OUString > SAL_CALL SchXMLImport_Content_getSupportedServiceNames() throw()
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Chart.XMLContentImporter" ) );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

OUString SAL_CALL SchXMLImport_Content_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SchXMLImport.Content" ) );
}

uno::Reference< uno::XInterface > SAL_CALL SchXMLImport_Content_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception )
{
    return (cppu::OWeakObject*)new SchXMLImport( IMPORT_CONTENT | IMPORT_AUTOSTYLES | IMPORT_FONTDECLS );
}

// ------------------------------------------------------------

uno::Sequence< OUString > SAL_CALL SchXMLImport_Meta_getSupportedServiceNames() throw()
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Chart.XMLMetaImporter" ) );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

OUString SAL_CALL SchXMLImport_Meta_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SchXMLImport.Meta" ) );
}

uno::Reference< uno::XInterface > SAL_CALL SchXMLImport_Meta_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception )
{
    return (cppu::OWeakObject*)new SchXMLImport( IMPORT_META );
}
