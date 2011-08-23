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

#include "SchXMLChartContext.hxx"
#include "contexts.hxx"

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLSTYLE_HXX
#include "xmlstyle.hxx"
#endif

#ifndef _COM_SUN_STAR_TASK_XSTATUSINDICATORSUPPLIER_HPP_
#include <com/sun/star/task/XStatusIndicatorSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_XCHARTDATAARRAY_HPP_ 
#include <com/sun/star/chart/XChartDataArray.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_CHARTDATAROWSOURCE_HPP_
#include <com/sun/star/chart/ChartDataRowSource.hpp>
#endif
namespace binfilter {

using namespace rtl;
using namespace ::com::sun::star;
using namespace ::binfilter::xmloff::token;

/* ----------------------------------------
   TokenMaps for distinguishing different
   tokens in different contexts
   ----------------------------------------*/

// ----------------------------------------
// element maps
// ----------------------------------------

static __FAR_DATA SvXMLTokenMapEntry aDocElemTokenMap[] =
{
    { XML_NAMESPACE_OFFICE, XML_AUTOMATIC_STYLES,	XML_TOK_DOC_AUTOSTYLES	},
    { XML_NAMESPACE_OFFICE, XML_STYLES,			    XML_TOK_DOC_STYLES	},
    { XML_NAMESPACE_OFFICE, XML_META, 				XML_TOK_DOC_META	},
    { XML_NAMESPACE_OFFICE, XML_BODY, 				XML_TOK_DOC_BODY	},
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aTableElemTokenMap[] =
{
    { XML_NAMESPACE_TABLE,	XML_TABLE_HEADER_COLUMNS,	XML_TOK_TABLE_HEADER_COLS	},
    { XML_NAMESPACE_TABLE,	XML_TABLE_COLUMNS,			XML_TOK_TABLE_COLUMNS		},
    { XML_NAMESPACE_TABLE,	XML_TABLE_COLUMN,			XML_TOK_TABLE_COLUMN		},
    { XML_NAMESPACE_TABLE,	XML_TABLE_HEADER_ROWS,		XML_TOK_TABLE_HEADER_ROWS	},
    { XML_NAMESPACE_TABLE,	XML_TABLE_ROWS,			    XML_TOK_TABLE_ROWS 			},
    { XML_NAMESPACE_TABLE,	XML_TABLE_ROW,				XML_TOK_TABLE_ROW 			},
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aChartElemTokenMap[] =
{
    { XML_NAMESPACE_CHART,	XML_PLOT_AREA,				XML_TOK_CHART_PLOT_AREA		},
    { XML_NAMESPACE_CHART,	XML_TITLE,					XML_TOK_CHART_TITLE			},
    { XML_NAMESPACE_CHART,	XML_SUBTITLE,				XML_TOK_CHART_SUBTITLE		},
    { XML_NAMESPACE_CHART,	XML_LEGEND,				XML_TOK_CHART_LEGEND		},
    { XML_NAMESPACE_TABLE,	XML_TABLE,					XML_TOK_CHART_TABLE			},
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aPlotAreaElemTokenMap[] =
{
    { XML_NAMESPACE_CHART,	XML_AXIS,					XML_TOK_PA_AXIS				},
    { XML_NAMESPACE_CHART,	XML_SERIES,				    XML_TOK_PA_SERIES			},
    { XML_NAMESPACE_CHART,	XML_CATEGORIES,			    XML_TOK_PA_CATEGORIES		},
    { XML_NAMESPACE_CHART,	XML_WALL,					XML_TOK_PA_WALL				},
    { XML_NAMESPACE_CHART,	XML_FLOOR,					XML_TOK_PA_FLOOR			},
    { XML_NAMESPACE_DR3D,	XML_LIGHT,					XML_TOK_PA_LIGHT_SOURCE		},
    { XML_NAMESPACE_CHART,  XML_STOCK_GAIN_MARKER,      XML_TOK_PA_STOCK_GAIN       },
    { XML_NAMESPACE_CHART,  XML_STOCK_LOSS_MARKER,      XML_TOK_PA_STOCK_LOSS       },
    { XML_NAMESPACE_CHART,  XML_STOCK_RANGE_LINE,       XML_TOK_PA_STOCK_RANGE      },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aSeriesElemTokenMap[] =
{
    { XML_NAMESPACE_CHART,	XML_DATA_POINT,	      XML_TOK_SERIES_DATA_POINT       },
    { XML_NAMESPACE_CHART,	XML_DOMAIN,		      XML_TOK_SERIES_DOMAIN	          },
    { XML_NAMESPACE_CHART,	XML_MEAN_VALUE,       XML_TOK_SERIES_MEAN_VALUE_LINE  },
    { XML_NAMESPACE_CHART,	XML_REGRESSION_CURVE, XML_TOK_SERIES_REGRESSION_CURVE },
    { XML_NAMESPACE_CHART,	XML_ERROR_INDICATOR,  XML_TOK_SERIES_ERROR_INDICATOR  },
    XML_TOKEN_MAP_END
};

// ----------------------------------------
// attribute maps
// ----------------------------------------

static __FAR_DATA SvXMLTokenMapEntry aChartAttrTokenMap[] =
{
    { XML_NAMESPACE_CHART,	XML_CLASS,					XML_TOK_CHART_CLASS			},
    { XML_NAMESPACE_SVG,	XML_WIDTH,					XML_TOK_CHART_WIDTH			},
    { XML_NAMESPACE_SVG,	XML_HEIGHT,				    XML_TOK_CHART_HEIGHT		},
    { XML_NAMESPACE_CHART,	XML_STYLE_NAME,			    XML_TOK_CHART_STYLE_NAME	},
    { XML_NAMESPACE_CHART,	XML_ADD_IN_NAME,			XML_TOK_CHART_ADDIN_NAME	},
    { XML_NAMESPACE_CHART,  XML_COLUMN_MAPPING,         XML_TOK_CHART_COL_MAPPING   },
    { XML_NAMESPACE_CHART,  XML_ROW_MAPPING,            XML_TOK_CHART_ROW_MAPPING   },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aPlotAreaAttrTokenMap[] =
{
    { XML_NAMESPACE_SVG,	XML_X,						XML_TOK_PA_X                 },
    { XML_NAMESPACE_SVG,	XML_Y,						XML_TOK_PA_Y                 },
    { XML_NAMESPACE_SVG,	XML_WIDTH,					XML_TOK_PA_WIDTH             },
    { XML_NAMESPACE_SVG,	XML_HEIGHT,				    XML_TOK_PA_HEIGHT            },
    { XML_NAMESPACE_CHART,	XML_STYLE_NAME,			    XML_TOK_PA_STYLE_NAME        },
    { XML_NAMESPACE_DR3D,	XML_TRANSFORM,				XML_TOK_PA_TRANSFORM         },
    { XML_NAMESPACE_TABLE,  XML_CELL_RANGE_ADDRESS,    XML_TOK_PA_CHART_ADDRESS     },
    { XML_NAMESPACE_CHART,  XML_TABLE_NUMBER_LIST,     XML_TOK_PA_TABLE_NUMBER_LIST },
    { XML_NAMESPACE_CHART,  XML_DATA_SOURCE_HAS_LABELS,XML_TOK_PA_DS_HAS_LABELS     },
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aAxisAttrTokenMap[] =
{
    { XML_NAMESPACE_CHART,	XML_CLASS,					XML_TOK_AXIS_CLASS			},
    { XML_NAMESPACE_CHART,	XML_NAME,					XML_TOK_AXIS_NAME			},
    { XML_NAMESPACE_CHART,	XML_STYLE_NAME,		    	XML_TOK_AXIS_STYLE_NAME		},
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aLegendAttrTokenMap[] =
{
    { XML_NAMESPACE_CHART,	XML_LEGEND_POSITION,		XML_TOK_LEGEND_POSITION		},
    { XML_NAMESPACE_SVG,	XML_X,						XML_TOK_LEGEND_X			},
    { XML_NAMESPACE_SVG,	XML_Y,						XML_TOK_LEGEND_Y			},
    { XML_NAMESPACE_CHART,	XML_STYLE_NAME,			    XML_TOK_LEGEND_STYLE_NAME	},
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aCellAttrTokenMap[] =
{
    { XML_NAMESPACE_TABLE,	XML_VALUE_TYPE,			    XML_TOK_CELL_VAL_TYPE		},
    { XML_NAMESPACE_TABLE,	XML_VALUE,					XML_TOK_CELL_VALUE			},
    XML_TOKEN_MAP_END
};

static __FAR_DATA SvXMLTokenMapEntry aSeriesAttrTokenMap[] =
{
    { XML_NAMESPACE_CHART,	XML_VALUES_CELL_RANGE_ADDRESS,	XML_TOK_SERIES_CELL_RANGE	 },
    { XML_NAMESPACE_CHART,	XML_LABEL_CELL_ADDRESS,		    XML_TOK_SERIES_LABEL_ADDRESS },
    { XML_NAMESPACE_CHART,	XML_ATTACHED_AXIS,				XML_TOK_SERIES_ATTACHED_AXIS },
    { XML_NAMESPACE_CHART,	XML_STYLE_NAME, 				XML_TOK_SERIES_STYLE_NAME	 },
    { XML_NAMESPACE_CHART,	XML_CLASS, 					    XML_TOK_SERIES_CHART_CLASS	 },
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
    sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName,
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

// -1 means don't change
void SchXMLImportHelper::ResizeChartData( sal_Int32 nSeries, sal_Int32 nDataPoints )
{
    if( mxChartDoc.is())
    {
        sal_Bool bWasChanged = sal_False;

        sal_Bool bDataInColumns = sal_True;
        uno::Reference< beans::XPropertySet > xDiaProp( mxChartDoc->getDiagram(), uno::UNO_QUERY );
        if( xDiaProp.is())
        {
            chart::ChartDataRowSource eRowSource;
            xDiaProp->getPropertyValue( ::rtl::OUString::createFromAscii( "DataRowSource" )) >>= eRowSource;
            bDataInColumns = ( eRowSource == chart::ChartDataRowSource_COLUMNS );

            // the chart core treats donut chart with interchanged rows/columns
            uno::Reference< chart::XDiagram > xDiagram( xDiaProp, uno::UNO_QUERY );            
            if( xDiagram.is())
            {
                ::rtl::OUString sChartType = xDiagram->getDiagramType();
                if( 0 == sChartType.reverseCompareToAsciiL( RTL_CONSTASCII_STRINGPARAM( "com.sun.star.chart.DonutDiagram" )))
                {
                    bDataInColumns = ! bDataInColumns;
                }
            }
        }
        sal_Int32 nColCount = bDataInColumns ? nSeries : nDataPoints;
        sal_Int32 nRowCount = bDataInColumns ? nDataPoints : nSeries;

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
            }
        }
    }
}

// ========================================

// #110680#
SchXMLImport::SchXMLImport( 
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory,
    sal_uInt16 nImportFlags ) 
:	SvXMLImport( xServiceFactory, nImportFlags )
{
}

SchXMLImport::~SchXMLImport() throw ()
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
SvXMLImportContext *SchXMLImport::CreateContext( USHORT nPrefix, const ::rtl::OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    SvXMLImportContext* pContext = 0;

    // accept <office:document>
    if( XML_NAMESPACE_OFFICE == nPrefix &&
        ( IsXMLToken( rLocalName, XML_DOCUMENT ) ||
          IsXMLToken( rLocalName, XML_DOCUMENT_META) ||
          IsXMLToken( rLocalName, XML_DOCUMENT_STYLES) ||
          IsXMLToken( rLocalName, XML_DOCUMENT_CONTENT) ))
    {
        pContext = new SchXMLDocContext( maImportHelper, *this, nPrefix, rLocalName );
    }
    else
    {
        pContext = SvXMLImport::CreateContext( nPrefix, rLocalName, xAttrList );
    }

    return pContext;
}

SvXMLImportContext* SchXMLImport::CreateStylesContext(
    const ::rtl::OUString& rLocalName,
    const uno::Reference<xml::sax::XAttributeList>& xAttrList )
{
    SvXMLStylesContext* pStylesCtxt =
        new SvXMLStylesContext( *(this), XML_NAMESPACE_OFFICE, rLocalName, xAttrList );

    // set context at base class, so that all auto-style classes are imported
    SetAutoStyles( pStylesCtxt );
    maImportHelper.SetAutoStylesContext( pStylesCtxt );

    return pStylesCtxt;
}

// export components ========================================

// first version: everything comes from one storage

uno::Sequence< OUString > SAL_CALL SchXMLImport_getSupportedServiceNames() throw()
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.Chart.XMLImporter" ) );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

OUString SAL_CALL SchXMLImport_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SchXMLImport" ) );
}

uno::Reference< uno::XInterface > SAL_CALL SchXMLImport_createInstance(const uno::Reference< lang::XMultiServiceFactory > & rSMgr) throw( uno::Exception )
{
    // #110680#
    // return (cppu::OWeakObject*)new SchXMLImport();
    return (cppu::OWeakObject*)new SchXMLImport(rSMgr);
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
    // #110680#
    // return (cppu::OWeakObject*)new SchXMLImport( IMPORT_STYLES );
    return (cppu::OWeakObject*)new SchXMLImport( rSMgr, IMPORT_STYLES );
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
    // #110680#
    // return (cppu::OWeakObject*)new SchXMLImport( IMPORT_CONTENT | IMPORT_AUTOSTYLES | IMPORT_FONTDECLS );
    return (cppu::OWeakObject*)new SchXMLImport( rSMgr, IMPORT_CONTENT | IMPORT_AUTOSTYLES | IMPORT_FONTDECLS );
}

// ------------------------------------------------------------

OUString SAL_CALL SchXMLImport_Meta_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( "SchXMLImport.Meta" ) );
}

// XServiceInfo
OUString SAL_CALL SchXMLImport::getImplementationName() throw( uno::RuntimeException )
{
    switch( getImportFlags())
    {
        case IMPORT_ALL:
            return SchXMLImport_getImplementationName();
        case IMPORT_STYLES:
            return SchXMLImport_Styles_getImplementationName();
        case ( IMPORT_CONTENT | IMPORT_AUTOSTYLES | IMPORT_FONTDECLS ):
            return SchXMLImport_Content_getImplementationName();
        case IMPORT_META:
            return SchXMLImport_Meta_getImplementationName();

        case IMPORT_SETTINGS:
        // there is no settings component in chart
        default:
            return OUString::createFromAscii( "SchXMLImport" );
    }
}
}//end of namespace binfilter
