/*************************************************************************
 *
 *  $RCSfile: dlg_ChartType.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: bm $ $Date: 2003-11-20 18:12:22 $
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
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
/*
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#include <sfx2/sfxsids.hrc>
#include <svx/eeitem.hxx>
#pragma hdrstop

#include "schdll.hxx"
#define ITEMID_CHARTSTYLE   CHATTR_DIAGRAM_STYLE
#ifndef _SVX_CHRTITEM_HXX //autogen
#include <svx/chrtitem.hxx>
#endif

#include "strings.hrc"
#include "res_bmp.hrc"
// for high contrast images
#include "res_hc_bmp.hrc"

#include "schresid.hxx"
#include "chtmodel.hxx"
#include "diagrtyp.hxx"
#include "diagrtyp.hrc"

#include "chmod3d.hxx"
#include "schattr.hxx"
// TVM: CHTYPE wurde ins Model verschoben
*/

#include "dlg_ChartType.hxx"
#include "dlg_ChartType.hrc"
#include "ResId.hxx"
#include "Bitmaps.hrc"
#include "Bitmaps_HC.hrc"
#include "Strings.hrc"
#include "DataSeriesTreeHelper.hxx"
#include "InlineContainer.hxx"
#include "macros.hxx"

#include <functional>
#include <algorithm>

#include "SchSfxItemIds.hxx"

// header for class SfxInt32Item
#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif

// header for define RET_OK
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif


#define OFFSET_PYRAMID 1000
#define OFFSET_ROUND   2000
#define OFFSET_CONE    3000

//GetChartShapeStyle()
//#define CHART_SHAPE3D_IGNORE  -2
#define CHART_SHAPE3D_ANY     -1
//#define CHART_SHAPE3D_SQUARE   0
#define CHART_SHAPE3D_CYLINDER 1
#define CHART_SHAPE3D_CONE     2
#define CHART_SHAPE3D_PYRAMID  3
//#define CHART_SHAPE3D_HANOI      4

// base diagram types
//#define CHTYPE_INVALID            0
#define CHTYPE_LINE             1
//#define CHTYPE_LINESYMB           2   // this one has to be removed !! (why?)
#define CHTYPE_AREA             3
#define CHTYPE_COLUMN           4   // is also a bar-type
#define CHTYPE_BAR              5
#define CHTYPE_CIRCLE           6
#define CHTYPE_XY               7
#define CHTYPE_NET              8
//#define CHTYPE_DONUT          9
#define CHTYPE_STOCK           10
//#define CHTYPE_ADDIN         11

using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star;

//.............................................................................
namespace
{
typedef ::std::map< ::rtl::OUString, sal_Int32 > tTemplateServiceMap;

const tTemplateServiceMap & lcl_GetChartTemplateServiceNameMap()
{
    static tTemplateServiceMap aTemplateMap(
        ::comphelper::MakeMap< ::rtl::OUString, sal_Int32 >
        ( C2U( "com.sun.star.chart2.template.Line" ),                           CHSTYLE_2D_LINE)
        ( C2U( "com.sun.star.chart2.template.StackedLine" ),                    CHSTYLE_2D_STACKEDLINE)
        ( C2U( "com.sun.star.chart2.template.PercentStackedLine" ),             CHSTYLE_2D_PERCENTLINE)
        ( C2U( "com.sun.star.chart2.template.LineSymbol" ),                     CHSTYLE_2D_LINESYMBOLS)
        ( C2U( "com.sun.star.chart2.template.StackedLineSymbol" ),              CHSTYLE_2D_STACKEDLINESYM)
        ( C2U( "com.sun.star.chart2.template.PercentStackedLineSymbol" ),       CHSTYLE_2D_PERCENTLINESYM)
        ( C2U( "com.sun.star.chart2.template.CubicSpline" ),                    CHSTYLE_2D_CUBIC_SPLINE)
        ( C2U( "com.sun.star.chart2.template.CubicSplineSymbol" ),              CHSTYLE_2D_CUBIC_SPLINE_SYMBOL)
        ( C2U( "com.sun.star.chart2.template.BSpline" ),                        CHSTYLE_2D_B_SPLINE)
        ( C2U( "com.sun.star.chart2.template.BSplineSymbol" ),                  CHSTYLE_2D_B_SPLINE_SYMBOL)
        ( C2U( "com.sun.star.chart2.template.ThreeDLine" ),                     CHSTYLE_3D_STRIPE)
        ( C2U( "com.sun.star.chart2.template.Column" ),                         CHSTYLE_2D_COLUMN)
        ( C2U( "com.sun.star.chart2.template.StackedColumn" ),                  CHSTYLE_2D_STACKEDCOLUMN)
        ( C2U( "com.sun.star.chart2.template.PercentStackedColumn" ),           CHSTYLE_2D_PERCENTCOLUMN)
        ( C2U( "com.sun.star.chart2.template.Bar" ),                            CHSTYLE_2D_BAR)
        ( C2U( "com.sun.star.chart2.template.StackedBar" ),                     CHSTYLE_2D_STACKEDBAR)
        ( C2U( "com.sun.star.chart2.template.PercentStackedBar" ),              CHSTYLE_2D_PERCENTBAR)
        ( C2U( "com.sun.star.chart2.template.ThreeDColumnDeep" ),               CHSTYLE_3D_COLUMN)
        ( C2U( "com.sun.star.chart2.template.ThreeDColumnFlat" ),               CHSTYLE_3D_FLATCOLUMN)
        ( C2U( "com.sun.star.chart2.template.StackedThreeDColumnFlat" ),        CHSTYLE_3D_STACKEDFLATCOLUMN)
        ( C2U( "com.sun.star.chart2.template.PercentStackedThreeDColumnFlat" ), CHSTYLE_3D_PERCENTFLATCOLUMN)
        ( C2U( "com.sun.star.chart2.template.ThreeDBarDeep" ),                  CHSTYLE_3D_BAR)
        ( C2U( "com.sun.star.chart2.template.ThreeDBarFlat" ),                  CHSTYLE_3D_FLATBAR)
        ( C2U( "com.sun.star.chart2.template.StackedThreeDBarFlat" ),           CHSTYLE_3D_STACKEDFLATBAR)
        ( C2U( "com.sun.star.chart2.template.PercentStackedThreeDBarFlat" ),    CHSTYLE_3D_PERCENTFLATBAR)
        ( C2U( "com.sun.star.chart2.template.ColumnWithLine" ),                 CHSTYLE_2D_LINE_COLUMN)
        ( C2U( "com.sun.star.chart2.template.StackedColumnWithLine" ),          CHSTYLE_2D_LINE_STACKEDCOLUMN)
        ( C2U( "com.sun.star.chart2.template.Area" ),                           CHSTYLE_2D_AREA)
        ( C2U( "com.sun.star.chart2.template.StackedArea" ),                    CHSTYLE_2D_STACKEDAREA)
        ( C2U( "com.sun.star.chart2.template.PercentStackedArea" ),             CHSTYLE_2D_PERCENTAREA)
        ( C2U( "com.sun.star.chart2.template.ThreeDArea" ),                     CHSTYLE_3D_AREA)
        ( C2U( "com.sun.star.chart2.template.StackedThreeDArea" ),              CHSTYLE_3D_STACKEDAREA)
        ( C2U( "com.sun.star.chart2.template.PercentStackedThreeDArea" ),       CHSTYLE_3D_PERCENTAREA)
        ( C2U( "com.sun.star.chart2.template.Pie" ),                            CHSTYLE_2D_PIE)
        ( C2U( "com.sun.star.chart2.template.PieOneExploded" ),                 CHSTYLE_2D_PIE_SEGOF1)
        ( C2U( "com.sun.star.chart2.template.PieAllExploded" ),                 CHSTYLE_2D_PIE_SEGOFALL)
        ( C2U( "com.sun.star.chart2.template.Ring" ),                           CHSTYLE_2D_DONUT1)
//         ( C2U( "com.sun.star.chart2.template.Ring" ),                           CHSTYLE_2D_DONUT2)
        ( C2U( "com.sun.star.chart2.template.ThreeDPie" ),                      CHSTYLE_3D_PIE)
        ( C2U( "com.sun.star.chart2.template.ScatterLineSymbol" ),              CHSTYLE_2D_XY)
        ( C2U( "com.sun.star.chart2.template.CubicSplineScatter" ),             CHSTYLE_2D_CUBIC_SPLINE_XY)
        ( C2U( "com.sun.star.chart2.template.CubicSplineScatterSymbol" ),       CHSTYLE_2D_CUBIC_SPLINE_SYMBOL_XY)
        ( C2U( "com.sun.star.chart2.template.BSplineScatter" ),                 CHSTYLE_2D_B_SPLINE_XY)
        ( C2U( "com.sun.star.chart2.template.BSplineScatterSymbol" ),           CHSTYLE_2D_B_SPLINE_SYMBOL_XY)
        ( C2U( "com.sun.star.chart2.template.ScatterLine" ),                    CHSTYLE_2D_XY_LINE)
        ( C2U( "com.sun.star.chart2.template.ScatterSymbol" ),                  CHSTYLE_2D_XYSYMBOLS)
        ( C2U( "com.sun.star.chart2.template.Net" ),                            CHSTYLE_2D_NET)
        ( C2U( "com.sun.star.chart2.template.NetSymbol" ),                      CHSTYLE_2D_NET_SYMBOLS)
        ( C2U( "com.sun.star.chart2.template.StackedNet" ),                     CHSTYLE_2D_NET_STACK)
        ( C2U( "com.sun.star.chart2.template.StackedNetSymbol" ),               CHSTYLE_2D_NET_SYMBOLS_STACK)
        ( C2U( "com.sun.star.chart2.template.PercentStackedNet" ),              CHSTYLE_2D_NET_PERCENT)
        ( C2U( "com.sun.star.chart2.template.PercentStackedNetSymbol" ),        CHSTYLE_2D_NET_SYMBOLS_PERCENT)
        ( C2U( "com.sun.star.chart2.template.StockLowHighClose" ),              CHSTYLE_2D_STOCK_1)
        ( C2U( "com.sun.star.chart2.template.StockOpenLowHighClose" ),          CHSTYLE_2D_STOCK_2)
        ( C2U( "com.sun.star.chart2.template.StockVolumeLowHighClose" ),        CHSTYLE_2D_STOCK_3)
        ( C2U( "com.sun.star.chart2.template.StockVolumeOpenLowHighClose" ),    CHSTYLE_2D_STOCK_4)
        ( C2U( "com.sun.star.chart2.template.Surface" ),                        CHSTYLE_3D_SURFACE)
        ( C2U( "com.sun.star.chart2.template.ThreeDScatter" ),                  CHSTYLE_3D_XYZ)
        ( C2U( "com.sun.star.chart2.template.ThreeDScatterSymbol" ),            CHSTYLE_3D_XYZSYMBOLS)
        ( C2U( "com.sun.star.chart2.template.Addin" ),                          CHSTYLE_ADDIN)
        );

    return aTemplateMap;
}

SvxChartStyle lcl_GetChartStyleForTemplateServiceName( const ::rtl::OUString & rServiceName )
{
    const tTemplateServiceMap & rMap = lcl_GetChartTemplateServiceNameMap();
    tTemplateServiceMap::const_iterator aIt( rMap.find( rServiceName ));
    if( aIt != rMap.end())
        return static_cast< SvxChartStyle >( (*aIt).second );

    return CHSTYLE_ADDIN;
}

::rtl::OUString lcl_GetTemplateServiceNameForChartStyle( SvxChartStyle eStyle )
{
    const tTemplateServiceMap & rMap = lcl_GetChartTemplateServiceNameMap();
    tTemplateServiceMap::const_iterator aIt(
        ::std::find_if( rMap.begin(), rMap.end(),
                        ::std::compose1( ::std::bind2nd(
                                             ::std::equal_to< tTemplateServiceMap::data_type >(),
                                             static_cast< sal_Int32 >( eStyle ) ),
                                         ::std::select2nd< tTemplateServiceMap::value_type >())));
    if( aIt != rMap.end())
        return (*aIt).first;

    return ::rtl::OUString();
}

bool lcl_IsBSplineChart( SvxChartStyle eStyle )
{
    switch( eStyle )
    {
        case CHSTYLE_2D_B_SPLINE :
        case CHSTYLE_2D_B_SPLINE_SYMBOL :
        case CHSTYLE_2D_B_SPLINE_XY :
        case CHSTYLE_2D_B_SPLINE_SYMBOL_XY :
            return true;

        default:
            return false;
    }
    return false;
}

bool lcl_IsCubicSplineChart( SvxChartStyle eStyle )
{
    switch( eStyle )
    {
        case CHSTYLE_2D_CUBIC_SPLINE :
        case CHSTYLE_2D_CUBIC_SPLINE_SYMBOL :
        case CHSTYLE_2D_CUBIC_SPLINE_XY :
        case CHSTYLE_2D_CUBIC_SPLINE_SYMBOL_XY :
            return true;

        default:
            return false;
    }
    return false;
}

bool lcl_IsShapeStyleChartType( SvxChartStyle eStyle )
{
    switch( eStyle )
    {
        case CHSTYLE_3D_COLUMN:
        case CHSTYLE_3D_FLATCOLUMN:
        case CHSTYLE_3D_STACKEDFLATCOLUMN:
        case CHSTYLE_3D_PERCENTFLATCOLUMN:
        case CHSTYLE_3D_BAR:
        case CHSTYLE_3D_FLATBAR:
        case CHSTYLE_3D_STACKEDFLATBAR:
        case CHSTYLE_3D_PERCENTFLATBAR:
            return true;

        default:
            return false;
    }
    return false;
}

bool lcl_IsCombiChart( SvxChartStyle eStyle )
{
    switch( eStyle )
    {
        case CHSTYLE_2D_LINE_COLUMN:
        case CHSTYLE_2D_LINE_STACKEDCOLUMN:
            return true;
        default:
            return false;
    }
    return false;
}

::rtl::OUString lcl_getTemplateForTree(
    const uno::Reference< chart2::XDiagram > & xDiagram,
    const uno::Reference< lang::XMultiServiceFactory > & xCTManager )
{
    ::rtl::OUString aResult;

    if( ! (xCTManager.is() && xDiagram.is()))
        return aResult;

    uno::Sequence< ::rtl::OUString > aServiceNames( xCTManager->getAvailableServiceNames());
    const sal_Int32 nLength = aServiceNames.getLength();

    for( sal_Int32 i = 0; i < nLength; ++i )
    {
        try
        {
//             OSL_TRACE( U2C( aServiceNames[i] ));
            uno::Reference< chart2::XChartTypeTemplate > xTempl(
                xCTManager->createInstance( aServiceNames[ i ] ), uno::UNO_QUERY_THROW );

            if( xTempl->matchesTemplate( xDiagram ))
            {
                aResult = aServiceNames[ i ];
                break;
            }
        }
        catch( uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }

    return aResult;
}

} // anonymous namespace

//.............................................................................
namespace chart
{
//.............................................................................

// macro for selecting a normal or high contrast bitmap the stack variable
// bIsHighContrast must exist and reflect the correct state
#define SELECT_BITMAP(name) Bitmap( SchResId( bIsHighContrast ? name ## _HC : name ))

SchDiagramTypeDlg::SchDiagramTypeDlg(
    Window* pWindow,
    const uno::Reference< chart2::XDiagram > & xDiagram,
    const uno::Reference< lang::XMultiServiceFactory > & xTemplateManager ) :
        ModalDialog(pWindow, SchResId(DLG_DIAGRAM_TYPE)),

        n3DGeometry(CHART_SHAPE3D_ANY),
        aRbt2D(this, ResId(RBT_2D)),
        aRbt3D(this, ResId(RBT_3D)),
        aFlDimension(this, ResId(FL_DIMENSION)),
        aFtDeep( this, ResId( FT_DEEP ) ),
        aMtrFldDeep( this, ResId( MTR_FLD_DEEP ) ),
        aFtGran( this, ResId( FT_GRAN ) ),
        aMtrFldGran( this, ResId( MTR_FLD_GRAN ) ),
        aFtNumLines( this, ResId( FT_NUM_OF_LINES ) ),
        aMtrFldNumLines( this, ResId( MTR_FLD_NUM_OF_LINES ) ),

        aFtType(this, ResId(FT_TYPE)),
        aCtlType(this, ResId(CTL_TYPE)),
        aFtVariant(this, ResId(FT_VARIANT)),
        aCtlVariant(this, ResId(CTL_VARIANT)),
        aBtnOK(this, ResId(BTN_OK)),
        aBtnCancel(this, ResId(BTN_CANCEL)),
        aBtnHelp(this, ResId(BTN_HELP)),
        eDimension(CHDIM_2D),
        m_xDiagram( xDiagram ),
        m_xTemplateManager( xTemplateManager )
{
    FreeResource();

    aRbt2D.SetClickHdl(LINK(this, SchDiagramTypeDlg,
                               SelectDimensionHdl));
    aRbt3D.SetClickHdl(LINK(this, SchDiagramTypeDlg,
                               SelectDimensionHdl));

    aCtlType.SetStyle(aCtlType.GetStyle() |
                         /*WB_ITEMBORDER | WB_DOUBLEBORDER |*/ WB_NAMEFIELD | WB_VSCROLL );

    aCtlType.SetColCount(4);
    aCtlType.SetLineCount(2);
    aCtlType.SetExtraSpacing(2);

    aCtlType.SetSelectHdl(LINK(this, SchDiagramTypeDlg, SelectTypeHdl));
    aCtlType.SetDoubleClickHdl(LINK(this, SchDiagramTypeDlg,
                                             DoubleClickHdl));

    aCtlVariant.SetStyle(aCtlVariant.GetStyle() |
                            /*WB_ITEMBORDER | WB_DOUBLEBORDER |*/ WB_NAMEFIELD | WB_VSCROLL );

    aCtlVariant.SetColCount(4);
    aCtlVariant.SetLineCount(1);
    aCtlVariant.SetExtraSpacing(2);
    aCtlVariant.SetDoubleClickHdl(LINK(this, SchDiagramTypeDlg, DoubleClickHdl));
    aCtlVariant.SetSelectHdl(LINK(this, SchDiagramTypeDlg, ClickHdl));

    Reset();

    // "- 1": at least one bar should remain
    SetMaximumNumberOfLines( helper::DataSeriesTreeHelper::getDataSeriesFromDiagram( m_xDiagram ).getLength() - 1 );
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

SchDiagramTypeDlg::~SchDiagramTypeDlg()
{
}
/*************************************************************************
|*
|*  Initialisierung
|*
\*************************************************************************/

void SchDiagramTypeDlg::Reset()
{
    if( ! m_xDiagram.is())
        return;

    uno::Reference< chart2::XDataSeriesTreeParent > xTree( m_xDiagram->getTree());
    SvxChartStyle eStyle = lcl_GetChartStyleForTemplateServiceName(
        lcl_getTemplateForTree( m_xDiagram, m_xTemplateManager ));

    USHORT nType;
    ChartDimension eDim;

    switch( eStyle )
    {
        case CHSTYLE_2D_B_SPLINE :
        case CHSTYLE_2D_B_SPLINE_SYMBOL :
        case CHSTYLE_2D_LINE:
        case CHSTYLE_2D_STACKEDLINE:
        case CHSTYLE_2D_PERCENTLINE:
        case CHSTYLE_2D_LINESYMBOLS:
        case CHSTYLE_2D_STACKEDLINESYM:
        case CHSTYLE_2D_PERCENTLINESYM:
        case CHSTYLE_2D_CUBIC_SPLINE :
        case CHSTYLE_2D_CUBIC_SPLINE_SYMBOL :
            nType   = CHTYPE_LINE;
            eDim    = CHDIM_2D;
            break;

        case CHSTYLE_2D_COLUMN:
        case CHSTYLE_2D_STACKEDCOLUMN:
        case CHSTYLE_2D_PERCENTCOLUMN:
        case CHSTYLE_2D_LINE_COLUMN:
        case CHSTYLE_2D_LINE_STACKEDCOLUMN:

            nType   = CHTYPE_COLUMN;
            eDim    = CHDIM_2D;
            break;

        case CHSTYLE_2D_BAR:
        case CHSTYLE_2D_STACKEDBAR:
        case CHSTYLE_2D_PERCENTBAR:
            nType   = CHTYPE_BAR;
            eDim    = CHDIM_2D;
            break;

        case CHSTYLE_2D_AREA:
        case CHSTYLE_2D_STACKEDAREA:
        case CHSTYLE_2D_PERCENTAREA:
            nType   = CHTYPE_AREA;
            eDim    = CHDIM_2D;
            break;

        case CHSTYLE_2D_PIE:
        case CHSTYLE_2D_PIE_SEGOF1:
        case CHSTYLE_2D_PIE_SEGOFALL:
        case CHSTYLE_2D_DONUT1:
        case CHSTYLE_2D_DONUT2:
            nType   = CHTYPE_CIRCLE;
            eDim    = CHDIM_2D;
            break;

        case CHSTYLE_2D_B_SPLINE_XY :
        case CHSTYLE_2D_XY_LINE :
        case CHSTYLE_2D_B_SPLINE_SYMBOL_XY :
        case CHSTYLE_2D_XYSYMBOLS:
        case CHSTYLE_2D_XY:
        case CHSTYLE_2D_CUBIC_SPLINE_XY :
        case CHSTYLE_2D_CUBIC_SPLINE_SYMBOL_XY :
            nType   = CHTYPE_XY;
            eDim    = CHDIM_2D;
            break;

        case CHSTYLE_2D_NET:
        case CHSTYLE_2D_NET_SYMBOLS:
        case CHSTYLE_2D_NET_STACK:
        case CHSTYLE_2D_NET_SYMBOLS_STACK:
        case CHSTYLE_2D_NET_PERCENT:
        case CHSTYLE_2D_NET_SYMBOLS_PERCENT:
            nType   = CHTYPE_NET;
            eDim    = CHDIM_2D;
            break;

        case CHSTYLE_3D_STRIPE:
            nType   = CHTYPE_LINE;
            eDim    = CHDIM_3D;
            break;

        case CHSTYLE_3D_COLUMN:
        case CHSTYLE_3D_FLATCOLUMN:
        case CHSTYLE_3D_STACKEDFLATCOLUMN:
        case CHSTYLE_3D_PERCENTFLATCOLUMN:
            nType   = CHTYPE_COLUMN;
            eDim    = CHDIM_3D;
            break;

        case CHSTYLE_3D_BAR:
        case CHSTYLE_3D_FLATBAR:
        case CHSTYLE_3D_STACKEDFLATBAR:
        case CHSTYLE_3D_PERCENTFLATBAR:
            nType   = CHTYPE_BAR;
            eDim    = CHDIM_3D;
            break;

        case CHSTYLE_3D_AREA:
        case CHSTYLE_3D_STACKEDAREA:
        case CHSTYLE_3D_PERCENTAREA:
            nType   = CHTYPE_AREA;
            eDim    = CHDIM_3D;
            break;

        case CHSTYLE_3D_PIE:
            nType   = CHTYPE_CIRCLE;
            eDim    = CHDIM_3D;
            break;

        case CHSTYLE_2D_STOCK_1:
        case CHSTYLE_2D_STOCK_2:
        case CHSTYLE_2D_STOCK_3:
        case CHSTYLE_2D_STOCK_4:
            nType   = CHTYPE_STOCK;
            eDim    = CHDIM_2D;
            break;

        default:
            DBG_ERROR("Invalid chart style given!");
            return;
    }

    if (eDim == CHDIM_3D)
    {
        aRbt3D.Check(TRUE);
        eDimension = CHDIM_2D;
        SelectDimensionHdl(&aRbt3D);
    }
    else
    {
        aRbt2D.Check(TRUE);
        eDimension = CHDIM_3D;
        SelectDimensionHdl(&aRbt2D);
    }

    aCtlType.SelectItem(nType);
    SelectTypeHdl(&aCtlType);

    SwitchDepth( eStyle );
    SwitchNumLines( eStyle );

    if( lcl_IsCombiChart( eStyle ))
    {
        sal_Int32 nNumLines = helper::DataSeriesTreeHelper::getNumberOfSeriesForChartTypeByIndex(
            xTree, 1 );
        aMtrFldNumLines.SetValue( nNumLines );
    }

    sal_Int32 nReso = 20;
    sal_Int32 nDepth = 3;
    if( lcl_IsBSplineChart( eStyle ) ||
        lcl_IsCubicSplineChart( eStyle ))
    {
        try
        {
            uno::Reference< beans::XPropertySet > xChartTypeProp(
                helper::DataSeriesTreeHelper::getChartTypeByIndex(
                    xTree, 0 ), uno::UNO_QUERY_THROW );

            if( ! (xChartTypeProp->getPropertyValue( C2U( "CurveResolution" )) >>= nReso) )
                nReso = 20;

            if( lcl_IsBSplineChart( eStyle ))
            {
                if( ! (xChartTypeProp->getPropertyValue( C2U( "SplineOrder" )) >>= nDepth) )
                    nDepth = 3;
            }
        }
        catch( uno::Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }

    }
    SetGranularity( nReso );
    SetDepth( nDepth );
}

/*************************************************************************
|*
|* Fuellt das Typ-Set mit Bitmaps
|*
\************************************************************************/

void SchDiagramTypeDlg::FillTypeSet(ChartDimension eDim, bool bForce /* default: false */ )
{
    if (eDim != eDimension )//|| bForce)
    {
        eDimension = eDim;

        USHORT nSelId = aCtlType.IsNoSelection() ?
                        0 : aCtlType.GetSelectItemId();

        aCtlType.Clear();

        bool bIsHighContrast = ( true && GetDisplayBackground().GetColor().IsDark() );

        if (aRbt2D.IsChecked())
        {
            aCtlType.InsertItem(CHTYPE_LINE,
                                SELECT_BITMAP( BMP_LINES_2D ),
                                String(SchResId(STR_LINES)));
            aCtlType.InsertItem(CHTYPE_AREA,
                                SELECT_BITMAP( BMP_AREAS_2D ),
                                String(SchResId(STR_AREAS)));
            aCtlType.InsertItem(CHTYPE_COLUMN,
                                SELECT_BITMAP( BMP_COLUMNS_2D ),
                                String(SchResId(STR_COLUMNS)));
            aCtlType.InsertItem(CHTYPE_BAR,
                                SELECT_BITMAP( BMP_BARS_2D ),
                                String(SchResId(STR_BARS)));
            aCtlType.InsertItem(CHTYPE_CIRCLE,
                                SELECT_BITMAP( BMP_CIRCLES_2D ),
                                String(SchResId(STR_CIRCLES)));
            aCtlType.InsertItem(CHTYPE_XY,
                                SELECT_BITMAP( BMP_XYSYMB_2D ),
                                String(SchResId(STR_XY)));
            aCtlType.InsertItem(CHTYPE_NET,
                                SELECT_BITMAP( BMP_NET_2D ),
                                String(SchResId(STR_NET)));
            aCtlType.InsertItem(CHTYPE_STOCK,
                                SELECT_BITMAP( BMP_STOCK_2 ),
                                String(SchResId(STR_STOCK)));
        }
        else
        {
            aCtlType.InsertItem(CHTYPE_LINE,
                                SELECT_BITMAP( BMP_LINES_3D ),
                                String(SchResId(STR_LINES)));
            aCtlType.InsertItem(CHTYPE_AREA,
                                SELECT_BITMAP( BMP_AREAS_3D_1 ),
                                String(SchResId(STR_AREAS)));
            aCtlType.InsertItem(CHTYPE_BAR,
                                SELECT_BITMAP( BMP_BARS_3D_1 ),
                                String(SchResId(STR_BARS)));
            aCtlType.InsertItem(CHTYPE_COLUMN,
                                SELECT_BITMAP( BMP_COLUMNS_3D_1 ),
                                String(SchResId(STR_COLUMNS)));
            aCtlType.InsertItem(CHTYPE_CIRCLE,
                                SELECT_BITMAP( BMP_CIRCLES_3D ),
                                String(SchResId(STR_CIRCLES)));
        }

        aCtlType.Show();

        if (nSelId > 0)
        {
            if (aCtlType.GetItemPos(nSelId) == VALUESET_ITEM_NOTFOUND) nSelId = aCtlType.GetItemId(0);

            aCtlType.SelectItem(nSelId);
            SelectTypeHdl(&aCtlType);
        }
    }
}

/*************************************************************************
|*
|* Fuellt das Varianten-Set mit Bitmaps
|*
\************************************************************************/

void SchDiagramTypeDlg::FillVariantSet(USHORT nType)
{
    long nDefaultOffset = 0;

    aCtlVariant.Clear();

    bool bIsHighContrast = ( true && GetDisplayBackground().GetColor().IsDark() );

    if (eDimension == CHDIM_3D)
    {
        switch (nType)
        {
            case CHTYPE_LINE:
                aCtlVariant.InsertItem((USHORT)CHSTYLE_3D_STRIPE + 1,
                                       SELECT_BITMAP( BMP_LINES_3D ),
                                       String(SchResId(STR_DEEP)));
                break;

            case CHTYPE_AREA:
                aCtlVariant.InsertItem((USHORT)CHSTYLE_3D_STACKEDAREA + 1,
                                       SELECT_BITMAP( BMP_AREAS_3D_1 ),
                                       String(SchResId(STR_STACKED)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_3D_PERCENTAREA + 1,
                                       SELECT_BITMAP( BMP_AREAS_3D_2 ),
                                       String(SchResId(STR_PERCENT)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_3D_AREA + 1,
                                       SELECT_BITMAP( BMP_AREAS_3D ),
                                       String(SchResId(STR_DEEP)));
                break;

            case CHTYPE_COLUMN:
                aCtlVariant.InsertItem((USHORT)CHSTYLE_3D_FLATCOLUMN + 1,
                                       SELECT_BITMAP( BMP_COLUMNS_3D_1 ),
                                       String(SchResId(STR_NORMAL)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_3D_STACKEDFLATCOLUMN + 1,
                                       SELECT_BITMAP( BMP_COLUMNS_3D_2 ),
                                       String(SchResId(STR_STACKED)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_3D_PERCENTFLATCOLUMN + 1,
                                       SELECT_BITMAP( BMP_COLUMNS_3D_3 ),
                                       String(SchResId(STR_PERCENT)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_3D_COLUMN + 1,
                                       SELECT_BITMAP( BMP_COLUMNS_3D ),
                                       String(SchResId(STR_DEEP)));

                aCtlVariant.InsertItem((USHORT)CHSTYLE_3D_FLATCOLUMN + 1 + OFFSET_ROUND,
                                       SELECT_BITMAP( BMP_SAEULE_3D_1 ),
                                       String(SchResId(STR_SAEULE_3D_1)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_3D_STACKEDFLATCOLUMN + 1 + OFFSET_ROUND,
                                       SELECT_BITMAP( BMP_SAEULE_3D_2 ),
                                       String(SchResId(STR_SAEULE_3D_2)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_3D_PERCENTFLATCOLUMN + 1 + OFFSET_ROUND,
                                       SELECT_BITMAP( BMP_SAEULE_3D_3 ),
                                       String(SchResId(STR_SAEULE_3D_3)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_3D_COLUMN + 1 + OFFSET_ROUND,
                                       SELECT_BITMAP( BMP_SAEULE_3D_4 ),
                                       String(SchResId(STR_SAEULE_3D_4)));

                aCtlVariant.InsertItem((USHORT)CHSTYLE_3D_FLATCOLUMN + 1 + OFFSET_PYRAMID,
                                       SELECT_BITMAP( BMP_PYRAMID_3D_1 ),
                                       String(SchResId(STR_PYRAMID_3D_1)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_3D_STACKEDFLATCOLUMN + 1 + OFFSET_PYRAMID,
                                       SELECT_BITMAP( BMP_PYRAMID_3D_2 ),
                                       String(SchResId(STR_PYRAMID_3D_2)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_3D_PERCENTFLATCOLUMN + 1 + OFFSET_PYRAMID,
                                       SELECT_BITMAP( BMP_PYRAMID_3D_3 ),
                                       String(SchResId(STR_PYRAMID_3D_3)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_3D_COLUMN + 1 + OFFSET_PYRAMID,
                                       SELECT_BITMAP( BMP_PYRAMID_3D_4 ),
                                       String(SchResId(STR_PYRAMID_3D_4)));

                aCtlVariant.InsertItem((USHORT)CHSTYLE_3D_FLATCOLUMN + 1 + OFFSET_CONE,
                                       SELECT_BITMAP( BMP_KEGEL_3D_1 ),
                                       String(SchResId(STR_KEGEL_3D_1)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_3D_STACKEDFLATCOLUMN + 1 + OFFSET_CONE,
                                       SELECT_BITMAP( BMP_KEGEL_3D_2 ),
                                       String(SchResId(STR_KEGEL_3D_2)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_3D_PERCENTFLATCOLUMN + 1 + OFFSET_CONE,
                                       SELECT_BITMAP( BMP_KEGEL_3D_3 ),
                                       String(SchResId(STR_KEGEL_3D_3)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_3D_COLUMN + 1 + OFFSET_CONE,
                                       SELECT_BITMAP( BMP_KEGEL_3D_4 ),
                                       String(SchResId(STR_KEGEL_3D_4)));
                break;

            case CHTYPE_CIRCLE:
                aCtlVariant.InsertItem((USHORT)CHSTYLE_3D_PIE + 1,
                                       SELECT_BITMAP( BMP_CIRCLES_3D ),
                                       String(SchResId(STR_NORMAL)));
                break;

            case CHTYPE_BAR:
                aCtlVariant.InsertItem((USHORT)CHSTYLE_3D_FLATBAR + 1,
                                       SELECT_BITMAP( BMP_BARS_3D_1 ),
                                       String(SchResId(STR_NORMAL)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_3D_STACKEDFLATBAR + 1,
                                       SELECT_BITMAP( BMP_BARS_3D_2 ),
                                       String(SchResId(STR_STACKED)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_3D_PERCENTFLATBAR + 1,
                                       SELECT_BITMAP( BMP_BARS_3D_3 ),
                                       String(SchResId(STR_PERCENT)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_3D_BAR + 1,
                                       SELECT_BITMAP( BMP_BARS_3D ),
                                       String(SchResId(STR_DEEP)));



                aCtlVariant.InsertItem((USHORT)CHSTYLE_3D_FLATBAR + 1 + OFFSET_ROUND,
                                       SELECT_BITMAP( BMP_ROEHRE_3D_1 ),
                                       String(SchResId(STR_ROEHRE_3D_1)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_3D_STACKEDFLATBAR + 1 + OFFSET_ROUND,
                                       SELECT_BITMAP( BMP_ROEHRE_3D_2 ),
                                       String(SchResId(STR_ROEHRE_3D_2)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_3D_PERCENTFLATBAR + 1 + OFFSET_ROUND,
                                       SELECT_BITMAP( BMP_ROEHRE_3D_3 ),
                                       String(SchResId(STR_ROEHRE_3D_3)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_3D_BAR + 1 + OFFSET_ROUND,
                                       SELECT_BITMAP( BMP_ROEHRE_3D_4 ),
                                       String(SchResId(STR_ROEHRE_3D_4)));

                aCtlVariant.InsertItem((USHORT)CHSTYLE_3D_FLATBAR + 1 + OFFSET_PYRAMID,
                                       SELECT_BITMAP( BMP_PYRAMIDQ_3D_1 ),
                                       String(SchResId(STR_PYRAMIDQ_3D_1)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_3D_STACKEDFLATBAR + 1 + OFFSET_PYRAMID,
                                       SELECT_BITMAP( BMP_PYRAMIDQ_3D_2 ),
                                       String(SchResId(STR_PYRAMIDQ_3D_2)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_3D_PERCENTFLATBAR + 1 + OFFSET_PYRAMID,
                                       SELECT_BITMAP( BMP_PYRAMIDQ_3D_3 ),
                                       String(SchResId(STR_PYRAMIDQ_3D_3)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_3D_BAR + 1 + OFFSET_PYRAMID,
                                       SELECT_BITMAP( BMP_PYRAMIDQ_3D_4 ),
                                       String(SchResId(STR_PYRAMIDQ_3D_4)));

                aCtlVariant.InsertItem((USHORT)CHSTYLE_3D_FLATBAR + 1 + OFFSET_CONE,
                                       SELECT_BITMAP( BMP_KEGELQ_3D_1 ),
                                       String(SchResId(STR_KEGELQ_3D_1)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_3D_STACKEDFLATBAR + 1 + OFFSET_CONE,
                                       SELECT_BITMAP( BMP_KEGELQ_3D_2 ),
                                       String(SchResId(STR_KEGELQ_3D_2)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_3D_PERCENTFLATBAR + 1 + OFFSET_CONE,
                                       SELECT_BITMAP( BMP_KEGELQ_3D_3 ),
                                       String(SchResId(STR_KEGELQ_3D_3)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_3D_BAR + 1 + OFFSET_CONE,
                                       SELECT_BITMAP( BMP_KEGELQ_3D_4 ),
                                       String(SchResId(STR_KEGELQ_3D_4)));
                break;

        }
    }
    else        // 2D types
    {
        switch (nType)
        {
            case CHTYPE_LINE:
                aCtlVariant.InsertItem((USHORT)CHSTYLE_2D_LINE + 1,
                                       SELECT_BITMAP( BMP_LINES_2D_1 ),
                                       String(SchResId(STR_NORMAL)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_2D_STACKEDLINE + 1,
                                       SELECT_BITMAP( BMP_LINES_2D_2 ),
                                       String(SchResId(STR_STACKED)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_2D_PERCENTLINE + 1,
                                       SELECT_BITMAP( BMP_LINES_2D_3 ),
                                       String(SchResId(STR_PERCENT)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_2D_LINESYMBOLS + 1,
                                       SELECT_BITMAP( BMP_LINESYMB_2D_1 ),
                                       String(SchResId(STR_SYMBOL)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_2D_STACKEDLINESYM + 1,
                                       SELECT_BITMAP( BMP_LINESYMB_2D_2 ),
                                       String(SchResId(STR_SYMBOL_STACK)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_2D_PERCENTLINESYM + 1,
                                       SELECT_BITMAP( BMP_LINESYMB_2D_3 ),
                                       String(SchResId(STR_SYMBOL_PERCENT)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_2D_CUBIC_SPLINE + 1,
                                       SELECT_BITMAP( BMP_SPLINE_CUBIC ),
                                       String(SchResId(STR_SPLINE_CUBIC)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_2D_CUBIC_SPLINE_SYMBOL + 1,
                                       SELECT_BITMAP( BMP_SPLINE_CUBIC_SYMBOL ),
                                       String(SchResId(STR_SPLINE_CUBIC_SYMBOL)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_2D_B_SPLINE + 1,
                                       SELECT_BITMAP( BMP_SPLINE_B ),
                                       String(SchResId(STR_SPLINE_B)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_2D_B_SPLINE_SYMBOL + 1,
                                       SELECT_BITMAP( BMP_SPLINE_B_SYMBOL ),
                                       String(SchResId(STR_SPLINE_B_SYMBOL)));
                break;

            case CHTYPE_AREA:
                aCtlVariant.InsertItem((USHORT)CHSTYLE_2D_AREA + 1,
                                       SELECT_BITMAP( BMP_AREAS_2D_1 ),
                                       String(SchResId(STR_NORMAL)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_2D_STACKEDAREA + 1,
                                       SELECT_BITMAP( BMP_AREAS_2D ),
                                       String(SchResId(STR_STACKED)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_2D_PERCENTAREA + 1,
                                       SELECT_BITMAP( BMP_AREAS_2D_3 ),
                                       String(SchResId(STR_PERCENT)));
                nDefaultOffset = 1;
                break;

            case CHTYPE_COLUMN:
                aCtlVariant.InsertItem((USHORT)CHSTYLE_2D_COLUMN + 1,
                                       SELECT_BITMAP( BMP_COLUMNS_2D_1 ),
                                       String(SchResId(STR_NORMAL)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_2D_STACKEDCOLUMN + 1,
                                       SELECT_BITMAP( BMP_COLUMNS_2D_2 ),
                                       String(SchResId(STR_STACKED)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_2D_PERCENTCOLUMN + 1,
                                       SELECT_BITMAP( BMP_COLUMNS_2D_3 ),
                                       String(SchResId(STR_PERCENT)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_2D_LINE_COLUMN + 1,
                                       SELECT_BITMAP( BMP_COLUMNS_2D_4 ),
                                       String(SchResId(STR_LINE_COLUMN)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_2D_LINE_STACKEDCOLUMN + 1,
                                       SELECT_BITMAP( BMP_COLUMNS_2D_5 ),
                                       String(SchResId(STR_LINE_STACKEDCOLUMN)));

                break;

            case CHTYPE_BAR:
                aCtlVariant.InsertItem((USHORT)CHSTYLE_2D_BAR + 1,
                                       SELECT_BITMAP( BMP_BARS_2D_1 ),
                                       String(SchResId(STR_NORMAL)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_2D_STACKEDBAR + 1,
                                       SELECT_BITMAP( BMP_BARS_2D_2 ),
                                       String(SchResId(STR_STACKED)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_2D_PERCENTBAR + 1,
                                       SELECT_BITMAP( BMP_BARS_2D_3 ),
                                       String(SchResId(STR_PERCENT)));
                break;

            case CHTYPE_CIRCLE:
            {
                aCtlVariant.InsertItem((USHORT)CHSTYLE_2D_PIE + 1,
                                        SELECT_BITMAP( BMP_CIRCLES_2D ),
                                        String(SchResId(STR_NORMAL)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_2D_DONUT1 + 1,
                                        SELECT_BITMAP( BMP_DONUT1_2D ),
                                        String(SchResId(STR_DONUT1)));
                String aResStr( SchResId( STR_OFS ));
                String aOfsStr = aResStr;

                xub_StrLen nFound = aResStr.SearchAscii( "$(N)" );
                if( nFound != STRING_NOTFOUND )
                    aOfsStr.Replace( nFound, RTL_CONSTASCII_LENGTH( "$(N)" ), String::CreateFromInt32( 1 ));

                aCtlVariant.InsertItem( (USHORT)CHSTYLE_2D_PIE_SEGOF1 + 1,
                                        SELECT_BITMAP( BMP_CIRCLES_2D_1 ),
                                        aOfsStr );

                aOfsStr = aResStr;
                if( nFound != STRING_NOTFOUND )
                    aOfsStr.Replace( nFound, RTL_CONSTASCII_LENGTH( "$(N)" ), String::CreateFromInt32( 2 ));

                aCtlVariant.InsertItem( (USHORT)CHSTYLE_2D_PIE_SEGOFALL + 1,
                                        SELECT_BITMAP( BMP_CIRCLES_2D_2 ),
                                        aOfsStr );
                break;
            }

            case CHTYPE_XY:
                aCtlVariant.InsertItem((USHORT)CHSTYLE_2D_XYSYMBOLS + 1,
                                       SELECT_BITMAP( BMP_XYSYMB_2D ),
                                       String(SchResId(STR_ONLY_SYMBOLS)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_2D_XY + 1,
                                       SELECT_BITMAP( BMP_XY_2D ),
                                       String(SchResId(STR_LINE_SYMBOLS)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_2D_XY_LINE + 1,
                                       SELECT_BITMAP( BMP_LINES_2D_1 ),
                                       String(SchResId(STR_ONLY_LINES)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_2D_CUBIC_SPLINE_XY + 1,
                                       SELECT_BITMAP( BMP_SPLINE_CUBIC ),
                                       String(SchResId(STR_SPLINE_CUBIC)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_2D_CUBIC_SPLINE_SYMBOL_XY + 1,
                                       SELECT_BITMAP( BMP_SPLINE_CUBIC_SYMBOL ),
                                       String(SchResId(STR_SPLINE_CUBIC_SYMBOL)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_2D_B_SPLINE_XY + 1,
                                       SELECT_BITMAP( BMP_SPLINE_B ),
                                       String(SchResId(STR_SPLINE_B)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_2D_B_SPLINE_SYMBOL_XY + 1,
                                       SELECT_BITMAP( BMP_SPLINE_B_SYMBOL ),
                                       String(SchResId(STR_SPLINE_B_SYMBOL)));
                break;
            case CHTYPE_NET:
                aCtlVariant.InsertItem((USHORT)CHSTYLE_2D_NET + 1,
                                       SELECT_BITMAP( BMP_NET_2D ),
                                       String(SchResId(STR_NORMAL)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_2D_NET_STACK + 1,
                                       SELECT_BITMAP( BMP_NET_2D_STACK ),
                                       String(SchResId(STR_STACKED)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_2D_NET_PERCENT + 1,
                                       SELECT_BITMAP( BMP_NET_2D_PERCENT ),
                                       String(SchResId(STR_PERCENT)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_2D_NET_SYMBOLS + 1,
                                       SELECT_BITMAP( BMP_NETSYMB_2D ),
                                       String(SchResId(STR_SYMBOL)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_2D_NET_SYMBOLS_STACK + 1,
                                       SELECT_BITMAP( BMP_NETSYMB_2D_STACK ),
                                       String(SchResId(STR_SYMBOL_STACK)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_2D_NET_SYMBOLS_PERCENT + 1,
                                       SELECT_BITMAP( BMP_NETSYMB_2D_PERCENT ),
                                       String(SchResId(STR_SYMBOL_PERCENT)));
                break;

            case CHTYPE_STOCK:
                aCtlVariant.InsertItem((USHORT)CHSTYLE_2D_STOCK_1 + 1,
                                       SELECT_BITMAP( BMP_STOCK_1 ),
                                       String(SchResId(STR_STOCK_1)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_2D_STOCK_2 + 1,
                                       SELECT_BITMAP( BMP_STOCK_2 ),
                                       String(SchResId(STR_STOCK_2)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_2D_STOCK_3 + 1,
                                       SELECT_BITMAP( BMP_STOCK_3 ),
                                       String(SchResId(STR_STOCK_3)));
                aCtlVariant.InsertItem((USHORT)CHSTYLE_2D_STOCK_4 + 1,
                                       SELECT_BITMAP( BMP_STOCK_4 ),
                                       String(SchResId(STR_STOCK_4)));
                nDefaultOffset = 1;
        }
    }

    aCtlVariant.Show();

    USHORT nSelId = aCtlVariant.GetItemId(0);

    SvxChartStyle eStyle = lcl_GetChartStyleForTemplateServiceName(
        lcl_getTemplateForTree( m_xDiagram, m_xTemplateManager ));

    USHORT nId = static_cast< USHORT >( eStyle ) + 1;
//         (USHORT)((const SvxChartStyleItem*)pPoolItem)->GetValue() + 1;

    if( aCtlVariant.GetItemPos( nId ) != VALUESET_ITEM_NOTFOUND )
    {
        nSelId = nId;
        nDefaultOffset = 0;
    }

    long nShape3dOffset = 0;
    if( lcl_IsShapeStyleChartType( eStyle ))
    {
        nShape3dOffset = OFFSET_CONE;
    }

//      long nId3D = ((const SfxInt32Item*)&pPoolItem)->GetValue();
//      switch( n3DGeometry )
//      {
//          case CHART_SHAPE3D_CYLINDER:
//              nShape3dOffset = OFFSET_ROUND;
//              break;
//          case CHART_SHAPE3D_CONE:
//              nShape3dOffset = OFFSET_CONE;
//              break;
//          case CHART_SHAPE3D_PYRAMID:
//              nShape3dOffset = OFFSET_PYRAMID;
//              break;
//      }

    if( aCtlVariant.GetItemPos( nSelId + nShape3dOffset + nDefaultOffset ) != VALUESET_ITEM_NOTFOUND )
        nSelId += (nShape3dOffset + nDefaultOffset);

    aCtlVariant.SelectItem( nSelId );

    // make selected Item visible
    aCtlVariant.SetFirstLine( aCtlVariant.GetItemPos( aCtlVariant.GetSelectItemId() ) / aCtlVariant.GetColCount() );

    eStyle = static_cast< SvxChartStyle >( nSelId - 1 );
    SwitchDepth( eStyle );
    SwitchNumLines( eStyle );
}

/*************************************************************************
|*
|* Select-Hdl der Dimensions-Buttons
|*
\************************************************************************/

IMPL_LINK( SchDiagramTypeDlg, SelectDimensionHdl, void *, EMPTYARG )
{
    FillTypeSet(aRbt3D.IsChecked() ? CHDIM_3D : CHDIM_2D);
    return 0;
}

/*************************************************************************
|*
|* Select-Hdl des ValueSets
|*
\************************************************************************/

IMPL_LINK( SchDiagramTypeDlg, SelectTypeHdl, void *, EMPTYARG )
{
    FillVariantSet(aCtlType.GetSelectItemId());
    return 0;
}

/*************************************************************************
|*
|* Select-Hdl des ValueSets
|*
\************************************************************************/

IMPL_LINK( SchDiagramTypeDlg, DoubleClickHdl, void *, EMPTYARG )
{
    EndDialog(RET_OK);
    return 0;
}

/*************************************************************************
|*
|*    Fuellt uebergebenen Item-Set mit Dialogbox-Attributen
|*
\*************************************************************************/

// void SchDiagramTypeDlg::GetAttr(SfxItemSet& rOutAttrs)
// {
//  long nId=aCtlVariant.GetSelectItemId() - 1;
//  long nGeo=CHART_SHAPE3D_ANY;
//  if(nId>OFFSET_CONE)
//  {
//      nId-=OFFSET_CONE;
//      nGeo=CHART_SHAPE3D_CONE;
//  }
//  if(nId>OFFSET_ROUND)
//  {
//      nId-=OFFSET_ROUND;
//      nGeo=CHART_SHAPE3D_CYLINDER;
//  }
//  if(nId>OFFSET_PYRAMID)
//  {
//      nId-=OFFSET_PYRAMID;
//      nGeo=CHART_SHAPE3D_PYRAMID;
//  }
//  if(nGeo != n3DGeometry)
//      rOutAttrs.Put(SfxInt32Item(SCHATTR_STYLE_SHAPE,nGeo));
//  else
//      rOutAttrs.ClearItem(SCHATTR_STYLE_SHAPE);

//  rOutAttrs.Put(SvxChartStyleItem((SvxChartStyle) (nId)));

//     rOutAttrs.Put( SfxInt32Item( SCHATTR_NUM_OF_LINES_FOR_BAR, aMtrFldNumLines.GetValue()));
//     rOutAttrs.Put( SfxInt32Item( SCHATTR_SPLINE_RESOLUTION,    aMtrFldGran.GetValue()));
//     rOutAttrs.Put( SfxInt32Item( SCHATTR_SPLINE_ORDER,         aMtrFldDeep.GetValue()));
// }

/*************************************************************************
|*
|* Gibt die eingestellte Intensitaet als double zurueck
|*
\************************************************************************/

sal_Int32 SchDiagramTypeDlg::GetDepth() const
{
    return aMtrFldDeep.GetValue();
}

/*************************************************************************
|*
|* Setzt die uebergebene Intensitaet
|*
\************************************************************************/

void SchDiagramTypeDlg::SetDepth( sal_Int32 nDepth )
{
     aMtrFldDeep.SetValue( nDepth );
}

/*************************************************************************
|*
|* Setzt die uebergebene Intensitaet
|*
\************************************************************************/

void SchDiagramTypeDlg::SwitchDepth( SvxChartStyle eID )
{
    aFtDeep.Hide();
    aMtrFldDeep.Hide();
    aFtGran.Hide();
    aMtrFldGran.Hide();

    if( lcl_IsBSplineChart( eID ))
    {
        aFtDeep.Show ();
        aMtrFldDeep.Show ();
    }

    if( lcl_IsCubicSplineChart( eID ) ||
        lcl_IsBSplineChart( eID ) )
    {
        aFtGran.Show ();
        aMtrFldGran.Show ();
    }
}

/*************************************************************************
|*
|* Select-Hdl des ValueSets
|*
\************************************************************************/

IMPL_LINK( SchDiagramTypeDlg, ClickHdl, void *, EMPTYARG )
{
    SvxChartStyle eStyle = static_cast< SvxChartStyle >(
        aCtlVariant.GetSelectItemId() - 1 );

    SwitchDepth( eStyle );
    SwitchNumLines( eStyle );
    return 0;
}

/*************************************************************************
|*
|* Laedt die Splineaufloesung
|*
\************************************************************************/

sal_Int32 SchDiagramTypeDlg::GetGranularity() const
{
    return aMtrFldGran.GetValue();
}

/*************************************************************************
|*
|* Setzt die Splineaufloesung
|*
\************************************************************************/

void SchDiagramTypeDlg::SetGranularity( sal_Int32 nGranularity )
{
    aMtrFldGran.SetValue( nGranularity );
}

void SchDiagramTypeDlg::FillValueSets()
{
    // type category (2d or 3d)
    FillTypeSet( eDimension, true /* force change */ );

    // sub-type
    USHORT nId = aCtlType.GetSelectItemId();
    if( nId != 0 )
    {
        FillVariantSet( nId );
    }
}

void SchDiagramTypeDlg::DataChanged( const DataChangedEvent& rDCEvt )
{
    ModalDialog::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_STYLE) )
        FillValueSets();
}


void SchDiagramTypeDlg::SwitchNumLines( SvxChartStyle eID )
{
    if( lcl_IsCombiChart( eID ))
    {
        aFtNumLines.Show();
        aMtrFldNumLines.Show();
    }
    else
    {
        aFtNumLines.Hide();
        aMtrFldNumLines.Hide();
    }
}

void SchDiagramTypeDlg::SetMaximumNumberOfLines( long nMaxLines )
{
    aMtrFldNumLines.SetLast( nMaxLines );
    aMtrFldNumLines.SetMax( nMaxLines );
}

uno::Reference< chart2::XChartTypeTemplate > SchDiagramTypeDlg::getTemplate() const
{
    uno::Reference< chart2::XChartTypeTemplate > xResult;

    if( m_xTemplateManager.is())
    {
        SvxChartStyle eStyle = static_cast< SvxChartStyle >(
            aCtlVariant.GetSelectItemId() - 1 );

        xResult.set( m_xTemplateManager->createInstance(
                         lcl_GetTemplateServiceNameForChartStyle( eStyle )),
                     uno::UNO_QUERY );

        if( lcl_IsBSplineChart( eStyle ) ||
            lcl_IsCubicSplineChart( eStyle ))
        {
            try
            {
                uno::Reference< beans::XPropertySet > xProp( xResult, uno::UNO_QUERY_THROW );
                xProp->setPropertyValue( C2U( "CurveResolution" ), uno::makeAny( GetGranularity()));

                if( lcl_IsBSplineChart( eStyle ))
                    xProp->setPropertyValue( C2U( "SplineOrder" ), uno::makeAny( GetDepth()));
            }
            catch( uno::Exception & ex )
            {
                ASSERT_EXCEPTION( ex );
            }
        }

        if( lcl_IsCombiChart( eStyle ))
        {
            try
            {
                uno::Reference< beans::XPropertySet > xProp( xResult, uno::UNO_QUERY_THROW );
                xProp->setPropertyValue( C2U( "NumberOfLines" ), uno::makeAny( GetNumberOfLines()));
            }
            catch( uno::Exception & ex )
            {
                ASSERT_EXCEPTION( ex );
            }
        }
    }

    return xResult;
}

// void SchDiagramTypeDlg::SetNumberOfLines( long nLines )
// {
//     aMtrFldNumLines.SetValue( nLines );
// }

sal_Int32 SchDiagramTypeDlg::GetNumberOfLines() const
{
    return aMtrFldNumLines.GetValue();
}

bool SchDiagramTypeDlg::HasChanged() const
{
    // todo: only return true if really settings have changed
    return true;
}

//.............................................................................
} //namespace chart
//.............................................................................
