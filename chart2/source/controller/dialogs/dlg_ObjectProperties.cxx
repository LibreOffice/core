/*************************************************************************
 *
 *  $RCSfile: dlg_ObjectProperties.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: hr $ $Date: 2004-05-12 15:24:22 $
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
#ifndef _ZFORLIST_HXX
#ifndef _ZFORLIST_DECLARE_TABLE
#define _ZFORLIST_DECLARE_TABLE
#endif
#include <svtools/zforlist.hxx>
#endif

#include "dlg_ObjectProperties.hxx"
#include "dlg_ObjectProperties.hrc"
#include "TabPages.hrc"
#include "tp_AxisLabel.hxx"
#include "tp_DataLabel.hxx"
#include "tp_LegendPosition.hxx"
#include "tp_PointGeometry.hxx"
#include "tp_Scale.hxx"
#include "tp_SeriesStatistic.hxx"
#include "tp_SeriesToAxis.hxx"
#include "tp_TitleRotation.hxx"
#include "SchSlotIds.hxx"
#include "ResId.hxx"
#include "ViewElementListProvider.hxx"
#include "macros.hxx"
#include "ChartModelHelper.hxx"
#include "ChartTypeHelper.hxx"

#ifndef _COM_SUN_STAR_CHART2_XCHARTTYPE_HPP_
#include <com/sun/star/chart2/XChartType.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XDATASERIES_HPP_
#include <com/sun/star/chart2/XDataSeries.hpp>
#endif

#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif

#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif

#define ITEMID_COLOR_TABLE      SID_COLOR_TABLE
#define ITEMID_GRADIENT_LIST    SID_GRADIENT_LIST
#define ITEMID_HATCH_LIST       SID_HATCH_LIST
#define ITEMID_BITMAP_LIST      SID_BITMAP_LIST
#define ITEMID_DASH_LIST        SID_DASH_LIST
#define ITEMID_LINEEND_LIST     SID_LINEEND_LIST

#ifndef _SVX_DRAWITEM_HXX
#include <svx/drawitem.hxx>
#endif

#ifndef _OFF_OFAITEM_HXX
#include <svx/ofaitem.hxx>
#endif
#ifndef _SVX_GRAPHICITEM_HXX
#include <svx/svxgrahicitem.hxx>
#endif

#ifndef _SVX_DIALOGS_HRC
#include <svx/dialogs.hrc>
#endif
#ifndef _SVX_FLSTITEM_HXX
#include <svx/flstitem.hxx>
#endif
#ifndef _SVX_TAB_LINE_HXX
#include <svx/tabline.hxx>
#endif

#include <svx/flagsdef.hxx>

#define ITEMID_NUMBERINFO   SID_ATTR_NUMBERFORMAT_INFO
#ifndef _SVX_NUMINF_HXX
#include <svx/numinf.hxx>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

using namespace ::com::sun::star;
using namespace ::com::sun::star::chart2;

namespace
{

sal_Int32 lcl_getDimensionCount( const uno::Reference< frame::XModel >& xChartModel )
{
    sal_Int32 nDimensionCount = ChartTypeHelper::getDimensionCount(
        ChartModelHelper::getFirstChartType(
            ChartModelHelper::findDiagram( xChartModel ) ) );
    return nDimensionCount;
}

}//end anonymous namespace

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

ObjectPropertiesDialogParameter::ObjectPropertiesDialogParameter( const rtl::OUString& rObjectCID )
        : m_aObjectCID( rObjectCID )
        , m_eObjectType( ObjectIdentifier::getObjectType( m_aObjectCID ) )
        , m_bAffectsMultipleObjects(false)
        , m_bHasGeometryProperties(false)
        , m_bHasStatisticProperties(false)
        , m_bHasRegressionProperties(false)
        , m_bProvidesSecondaryYAxis(false)
        , m_bHasAreaProperties(false)
        , m_bHasLineProperties(false)
        , m_bHasSymbolProperties(false)
        , m_bHasScaleProperties(false)
        , m_bCanAxisLabelsBeStaggered(false)
{
    rtl::OUString aParticleID = ObjectIdentifier::getParticleID( m_aObjectCID );
    m_bAffectsMultipleObjects = aParticleID.equals(C2U("ALLELEMENTS"));
}
ObjectPropertiesDialogParameter::~ObjectPropertiesDialogParameter()
{
}
ObjectType ObjectPropertiesDialogParameter::getObjectType() const
{
    return m_eObjectType;
}

void ObjectPropertiesDialogParameter::init( const uno::Reference< frame::XModel >& xChartModel )
{
    uno::Reference< XDataSeries > xSeries = ObjectIdentifier::getDataSeriesForCID( m_aObjectCID, xChartModel );
    uno::Reference< XChartType > xChartType = ChartModelHelper::getChartTypeOfSeries( xChartModel, xSeries );

    m_bHasGeometryProperties = ChartTypeHelper::isSupportingGeometryProperties( xChartType );
    m_bHasAreaProperties     = ChartTypeHelper::isSupportingAreaProperties( xChartType );
    m_bHasSymbolProperties   = ChartTypeHelper::isSupportingSymbolProperties( xChartType );
    m_bHasLineProperties     = true; //@todo ask object

    if(    OBJECTTYPE_DATA_SERIES==m_eObjectType
        || OBJECTTYPE_DATA_LABELS==m_eObjectType )
    {
        m_bHasStatisticProperties =  ChartTypeHelper::isSupportingStatisticProperties( xChartType );
        m_bHasRegressionProperties = ChartTypeHelper::isSupportingRegressionProperties( xChartType );
        m_bProvidesSecondaryYAxis =  ChartTypeHelper::isSupportingSecondaryAxis( xChartType, 1 );
    }

    if( OBJECTTYPE_AXIS == m_eObjectType )
    {
        //show scale properties only for a single axis not for multiselection
        m_bHasScaleProperties = !m_bAffectsMultipleObjects;

        //no staggering of labels for 3D axis
        sal_Int32 nDimensionCount = lcl_getDimensionCount( xChartModel );
        m_bCanAxisLabelsBeStaggered = nDimensionCount==2;
    }
}

bool ObjectPropertiesDialogParameter::HasGeometryProperties() const
{
    return m_bHasGeometryProperties;
}
bool ObjectPropertiesDialogParameter::HasStatisticProperties() const
{
    return m_bHasStatisticProperties;
}
bool ObjectPropertiesDialogParameter::HasRegressionProperties() const
{
    return m_bHasRegressionProperties;
}
bool ObjectPropertiesDialogParameter::ProvidesSecondaryYAxis() const
{
    return m_bProvidesSecondaryYAxis;
}
bool ObjectPropertiesDialogParameter::HasAreaProperties() const
{
    return m_bHasAreaProperties;
}
bool ObjectPropertiesDialogParameter::HasLineProperties() const
{
    return m_bHasLineProperties;
}
bool ObjectPropertiesDialogParameter::HasSymbolProperties() const
{
    return m_bHasSymbolProperties;
}
bool ObjectPropertiesDialogParameter::HasScaleProperties() const
{
    return m_bHasScaleProperties;
}
bool ObjectPropertiesDialogParameter::CanAxisLabelsBeStaggered() const
{
    return m_bCanAxisLabelsBeStaggered;
}

//const USHORT nNoArrowDlg          = 1100;
const USHORT nNoArrowNoShadowDlg    = 1101;

//-------------------------------------------------------------------
//-------------------------------------------------------------------
//-------------------------------------------------------------------

//provide Resource-Id of a tab dialog for the given ObjectType
USHORT SchAttribTabDlg::GetResId(ObjectType eObjectType)
{
    USHORT nRet=0;
    switch( eObjectType )
    {
        case OBJECTTYPE_PAGE:
                nRet=TAB_DIAGRAM_AREA;
                break;
        case OBJECTTYPE_TITLE:
                nRet=TAB_TITLE;
                break;
        case OBJECTTYPE_LEGEND:
                nRet=TAB_LEGEND;
                break;
        case OBJECTTYPE_LEGEND_ENTRY:
                nRet=0;//@todo there is not tab dialog so far
                break;
        case OBJECTTYPE_DIAGRAM:
                nRet=TAB_DIAGRAM_WALL;//@todo ... maybe size,pos,rotation depth ...?
                break;
        case OBJECTTYPE_DIAGRAM_WALL:
                nRet=TAB_DIAGRAM_WALL;
                break;
        case OBJECTTYPE_DIAGRAM_FLOOR:
                nRet=TAB_DIAGRAM_FLOOR;
                break;
        case OBJECTTYPE_AXIS:
                nRet=TAB_AXIS;//@todo need further specialisation for titles (x,y,z axis)
                //and for z axis hide pages PAGE_AXISSCALE and PAGE_NUMBERFORMAT
                break;
        case OBJECTTYPE_AXIS_UNITLABEL:
                nRet=0;//@todo there is not tab dialog so far
                break;
        case OBJECTTYPE_GRID:
                nRet=TAB_GRID;
                break;
        case OBJECTTYPE_DATA_LABELS:
        case OBJECTTYPE_DATA_SERIES:
                nRet=TAB_DATA_ROW;
                //@todo ( old case ATTR_DATA_LINE - TAB_DATA_LINE )
                //for line oriented series hide folloing pages: PAGE_AREA PAGE_TRANSPARENCE and PAGE_LAYOUT
                break;
        case OBJECTTYPE_DATA_LABEL:
        case OBJECTTYPE_DATA_POINT:
                nRet=TAB_DATA_POINT;
                break;
        case OBJECTTYPE_DATA_ERRORS:
                nRet=TAB_LINE;
                break;
        case OBJECTTYPE_DATA_ERRORS_X:
                nRet=TAB_LINE;
                break;
        case OBJECTTYPE_DATA_ERRORS_Y:
                nRet=TAB_LINE;
                break;
        case OBJECTTYPE_DATA_ERRORS_Z:
                nRet=TAB_LINE;
                break;
        case OBJECTTYPE_DATA_CURVE:
                nRet=TAB_LINE;
                break;
        case OBJECTTYPE_DATA_STOCK_RANGE:
                nRet=TAB_LINE;
                break;
        case OBJECTTYPE_DATA_STOCK_LOSS:
                nRet=TAB_DIAGRAM_STOCK_LOSS;
                break;
        case OBJECTTYPE_DATA_STOCK_GAIN:
                nRet=TAB_DIAGRAM_STOCK_PLUS;
                break;
        default: //OBJECTTYPE_UNKNOWN
                nRet=0;
    }
    if(nRet==0)
    {
        DBG_ERROR("Do not have an associated tab dialog for this ObjectType");
        nRet=TAB_LINE;
    }
    return nRet;
}

void SchAttribTabDlg::setSymbolInformation( SfxItemSet* pSymbolShapeProperties,
                Graphic* pAutoSymbolGraphic )
{
    m_pSymbolShapeProperties = pSymbolShapeProperties;
    m_pAutoSymbolGraphic = pAutoSymbolGraphic;
}

SchAttribTabDlg::SchAttribTabDlg(Window* pParent,
                                 const SfxItemSet* pAttr,
                                 const ObjectPropertiesDialogParameter* pDialogParameter,
                                 const ViewElementListProvider* pViewElementListProvider )
    : SfxTabDialog(pParent, SchResId(GetResId(pDialogParameter->getObjectType())), pAttr)
    , eObjectType(pDialogParameter->getObjectType())
    , nDlgType(nNoArrowNoShadowDlg)
    , nPageType(0)
    , m_pParameter( pDialogParameter )
    , m_pViewElementListProvider( pViewElementListProvider )
    , m_pSymbolShapeProperties(NULL)
    , m_pAutoSymbolGraphic(NULL)
    , nColorTableState(CT_NONE)
    , nGradientListState(CT_NONE)
    , nHatchingListState(CT_NONE)
    , nBitmapListState(CT_NONE)
{
    FreeResource();
    switch (eObjectType)
    {
        case OBJECTTYPE_TITLE://ATTR_TITLE:
            AddTabPage(RID_SVXPAGE_LINE); //, SvxLineTabPage::Create, NULL);
            AddTabPage(RID_SVXPAGE_AREA); //, SvxAreaTabPage::Create, NULL);
            AddTabPage(RID_SVXPAGE_TRANSPARENCE); //, SvxTransparenceTabPage::Create, NULL);
            AddTabPage(RID_SVXPAGE_CHAR_NAME );//CHINA001  AddTabPage(RID_SVXPAGE_CHAR_NAME, SvxCharNamePage::Create, NULL);
            AddTabPage(RID_SVXPAGE_CHAR_EFFECTS );//CHINA001 AddTabPage(RID_SVXPAGE_CHAR_EFFECTS, SvxCharEffectsPage::Create, NULL);
            AddTabPage(TP_ALIGNMENT, SchAlignmentTabPage::Create, NULL);
            break;

        case OBJECTTYPE_LEGEND://ATTR_LEGEND:
            AddTabPage(RID_SVXPAGE_LINE); //, SvxLineTabPage::Create, NULL);
            AddTabPage(RID_SVXPAGE_AREA); //, SvxAreaTabPage::Create, NULL);
            AddTabPage(RID_SVXPAGE_TRANSPARENCE); //, SvxTransparenceTabPage::Create, NULL);
            AddTabPage(RID_SVXPAGE_CHAR_NAME ); //CHINA001 AddTabPage(RID_SVXPAGE_CHAR_NAME, SvxCharNamePage::Create, NULL);
            AddTabPage(RID_SVXPAGE_CHAR_EFFECTS );//CHINA001 AddTabPage(RID_SVXPAGE_CHAR_EFFECTS, SvxCharEffectsPage::Create, NULL);
            AddTabPage(TP_LEGEND_POS, SchLegendPosTabPage::Create, NULL);
            break;

        case OBJECTTYPE_DATA_SERIES://ATTR_DATA_ROW:
        case OBJECTTYPE_DATA_POINT://ATTR_DATA_POINT:
        case OBJECTTYPE_DATA_LABEL:
        case OBJECTTYPE_DATA_LABELS:
            AddTabPage(RID_SVXPAGE_LINE); //, SvxLineTabPage::Create, NULL);
            AddTabPage(RID_SVXPAGE_AREA); //, SvxAreaTabPage::Create, NULL);
            AddTabPage(RID_SVXPAGE_TRANSPARENCE); //, SvxTransparenceTabPage::Create, NULL);
            if(!m_pParameter->HasAreaProperties())
            {
                RemoveTabPage(RID_SVXPAGE_AREA);
                RemoveTabPage(RID_SVXPAGE_TRANSPARENCE);
            }
            AddTabPage(RID_SVXPAGE_CHAR_NAME ); //CHINA001 AddTabPage(RID_SVXPAGE_CHAR_NAME, SvxCharNamePage::Create, NULL);
            AddTabPage(RID_SVXPAGE_CHAR_EFFECTS );//CHINA001 AddTabPage(RID_SVXPAGE_CHAR_EFFECTS, SvxCharEffectsPage::Create, NULL);
            AddTabPage(TP_DATA_DESCR, SchDataDescrTabPage::Create, NULL);
            AddTabPage(TP_STAT, SchStatisticTabPage::Create, NULL);
            if( !m_pParameter->HasStatisticProperties() )
                RemoveTabPage (TP_STAT);
            AddTabPage(TP_LAYOUT,SchLayoutTabPage::Create, NULL);
            if( !m_pParameter->HasGeometryProperties() )
                RemoveTabPage(TP_LAYOUT);

            AddTabPage(TP_OPTIONS,SchOptionTabPage::Create, NULL);
            if( !m_pParameter->ProvidesSecondaryYAxis() )
                RemoveTabPage(TP_OPTIONS);
            break;

            /*
        case OBJECTTYPE_DATA_SERIES://ATTR_DATA_LINE:

            AddTabPage(RID_SVXPAGE_LINE); //, SvxLineTabPage::Create, NULL);
            AddTabPage(RID_SVXPAGE_CHAR_NAME, SvxCharNamePage::Create, NULL);
            AddTabPage(RID_SVXPAGE_CHAR_EFFECTS, SvxCharEffectsPage::Create, NULL);
            AddTabPage(TP_DATA_DESCR, SchDataDescrTabPage::Create, NULL);
            //if (m_pParameter->HasStatisticProperties())
            AddTabPage(TP_STAT, SchStatisticTabPage::Create, NULL);
            AddTabPage(TP_OPTIONS,SchOptionTabPage::Create, NULL);
            if (!m_pParameter->HasStatisticProperties())
                RemoveTabPage (TP_STAT);
            if(!m_pParameter->ProvidesSecondaryYAxis())
                RemoveTabPage(TP_OPTIONS);
            break;
            */


        /*
        case OBJECTTYPE_AXIS://ATTR_AXIS:
            AddTabPage(RID_SVXPAGE_LINE); //, SvxLineTabPage::Create, NULL);
            AddTabPage(RID_SVXPAGE_CHAR_NAME, SvxCharNamePage::Create, NULL);
            AddTabPage(RID_SVXPAGE_CHAR_EFFECTS, SvxCharEffectsPage::Create, NULL);
            AddTabPage(TP_AXIS_LABEL, SchAxisLabelTabPage::Create, NULL);
            break;

        case ATTR_X_AXIS_2D:
        */
        case OBJECTTYPE_AXIS:
        {
            AddTabPage(RID_SVXPAGE_LINE); //, SvxLineTabPage::Create, NULL);
            AddTabPage(RID_SVXPAGE_CHAR_NAME ); //CHINA001 AddTabPage(RID_SVXPAGE_CHAR_NAME, SvxCharNamePage::Create, NULL);
            AddTabPage(RID_SVXPAGE_CHAR_EFFECTS );//CHINA001 AddTabPage(RID_SVXPAGE_CHAR_EFFECTS, SvxCharEffectsPage::Create, NULL);
            AddTabPage(TP_AXIS_LABEL, SchAxisLabelTabPage::Create, NULL);
            AddTabPage(TP_SCALE_Y, SchScaleYAxisTabPage::Create, NULL);
            AddTabPage(RID_SVXPAGE_NUMBERFORMAT); //CHINA001 AddTabPage(RID_SVXPAGE_NUMBERFORMAT, SvxNumberFormatTabPage::Create, NULL);

            if( !m_pParameter->HasScaleProperties() )
            {
                RemoveTabPage (TP_SCALE_Y);
                RemoveTabPage (RID_SVXPAGE_NUMBERFORMAT);
            }
////            ((SfxItemSet * const) pAttr)->ClearItem (SCHATTR_AXISTYPE);
////            ((SfxItemSet *) pAttr)->Put (SfxInt32Item (SCHATTR_AXISTYPE, CHART_AXIS_X));

////            nAxisType = 1;
            break;
        }

        /*
        case ATTR_Y_AXIS_2D:
        case ATTR_Y_AXIS_3D:
            AddTabPage(RID_SVXPAGE_LINE); //, SvxLineTabPage::Create, NULL);
            AddTabPage(RID_SVXPAGE_CHAR_NAME, SvxCharNamePage::Create, NULL);
            AddTabPage(RID_SVXPAGE_CHAR_EFFECTS, SvxCharEffectsPage::Create, NULL);
            AddTabPage(TP_SCALE_Y, SchScaleYAxisTabPage::Create, NULL);
            AddTabPage(RID_SVXPAGE_NUMBERFORMAT, SvxNumberFormatTabPage::Create, NULL);
            AddTabPage(TP_AXIS_LABEL, SchAxisLabelTabPage::Create, NULL);
////            ((SfxItemSet * const) pAttr)->ClearItem (SCHATTR_AXISTYPE);
////            ((SfxItemSet *) pAttr)->Put (SfxInt32Item (SCHATTR_AXISTYPE, CHART_AXIS_Y));

////            nAxisType = 2;
            break;

        case ATTR_Z_AXIS:
            AddTabPage(RID_SVXPAGE_LINE); //, SvxLineTabPage::Create, NULL);
            AddTabPage(RID_SVXPAGE_CHAR_NAME, SvxCharNamePage::Create, NULL);
            AddTabPage(RID_SVXPAGE_CHAR_EFFECTS, SvxCharEffectsPage::Create, NULL);
            AddTabPage(TP_AXIS_LABEL, SchAxisLabelTabPage::Create, NULL);
////            ((SfxItemSet * const) pAttr)->ClearItem (SCHATTR_AXISTYPE);
////            ((SfxItemSet *) pAttr)->Put (SfxInt32Item (SCHATTR_AXISTYPE, CHART_AXIS_Z));

////            nAxisType = 3;
            break;
        */
        case OBJECTTYPE_GRID: //ATTR_GRID
        case OBJECTTYPE_DATA_ERRORS:
        case OBJECTTYPE_DATA_ERRORS_X:
        case OBJECTTYPE_DATA_ERRORS_Y:
        case OBJECTTYPE_DATA_ERRORS_Z:
        case OBJECTTYPE_DATA_CURVE:
        case OBJECTTYPE_DATA_STOCK_RANGE://ATTR_LINE
            AddTabPage(RID_SVXPAGE_LINE); //, SvxLineTabPage::Create, NULL);
            break;

        case OBJECTTYPE_DATA_STOCK_LOSS://ATTR_DIAGRAM_STOCK_LOSS:
        case OBJECTTYPE_DATA_STOCK_GAIN://ATTR_DIAGRAM_STOCK_PLUS:
        case OBJECTTYPE_PAGE://ATTR_DIAGRAM_AREA:
        case OBJECTTYPE_DIAGRAM_FLOOR://ATTR_DIAGRAM_FLOOR:
        case OBJECTTYPE_DIAGRAM_WALL://ATTR_DIAGRAM_WALL:
        case OBJECTTYPE_DIAGRAM://ATTR_DIAGRAM_WALL: //@todo
            AddTabPage(RID_SVXPAGE_LINE); //, SvxLineTabPage::Create, NULL);
            AddTabPage(RID_SVXPAGE_AREA); //, SvxAreaTabPage::Create, NULL);
            AddTabPage(RID_SVXPAGE_TRANSPARENCE); //, SvxTransparenceTabPage::Create, NULL);
            break;

        case OBJECTTYPE_LEGEND_ENTRY:
        case OBJECTTYPE_AXIS_UNITLABEL:
        case OBJECTTYPE_UNKNOWN:
            // nothing
            break;
    }
}

SchAttribTabDlg::~SchAttribTabDlg()
{
    delete m_pSymbolShapeProperties;
    delete m_pAutoSymbolGraphic;
}

void SchAttribTabDlg::PageCreated(USHORT nId, SfxTabPage &rPage)
{
    SfxAllItemSet aSet(*(GetInputSetImpl()->GetPool()));
    switch (nId)
    {
        case TP_LAYOUT:
        break;
        case RID_SVXPAGE_LINE:
//          ((SvxLineTabPage&)rPage).SetColorTable(m_pViewElementListProvider->GetColorTable());
//          ((SvxLineTabPage&)rPage).SetDashList(m_pViewElementListProvider->GetDashList());
//          ((SvxLineTabPage&)rPage).SetLineEndList(m_pViewElementListProvider->GetLineEndList());
//          ((SvxLineTabPage&)rPage).SetPageType(&nPageType);
//          ((SvxLineTabPage&)rPage).SetDlgType(&nDlgType);
//          ((SvxLineTabPage&)rPage).Construct();
//          ((SvxLineTabPage&)rPage).ActivatePage(*GetInputSetImpl());
            aSet.Put (SvxColorTableItem(m_pViewElementListProvider->GetColorTable(),SID_COLOR_TABLE));
            aSet.Put (SvxDashListItem(m_pViewElementListProvider->GetDashList(),SID_DASH_LIST));
            aSet.Put (SvxLineEndListItem(m_pViewElementListProvider->GetLineEndList(),SID_LINEEND_LIST));
            aSet.Put (SfxUInt16Item(SID_PAGE_TYPE,nPageType));
            aSet.Put (SfxUInt16Item(SID_DLG_TYPE,nDlgType));

            if( m_pParameter->HasSymbolProperties() )
            {
                aSet.Put(OfaPtrItem(SID_OBJECT_LIST,m_pViewElementListProvider->GetSymbolList()));
                if( m_pSymbolShapeProperties )
                    aSet.Put(SfxTabDialogItem(SID_ATTR_SET,*m_pSymbolShapeProperties));
                if( m_pAutoSymbolGraphic )
                    aSet.Put(SvxGraphicItem(SID_GRAPHIC,*m_pAutoSymbolGraphic));
//              ((SvxLineTabPage&)rPage).ShowSymbolControls(TRUE);
//              ((SvxLineTabPage&)rPage).SetSymbolList();
//                 if( m_pSymbolShapeProperties )
//                  ((SvxLineTabPage&)rPage).SetSymbolAttr(m_pSymbolShapeProperties);
//                 if( m_pAutoSymbolGraphic )
//                  ((SvxLineTabPage&)rPage).SetAutoSymbolGraphic(m_pAutoSymbolGraphic);
            }
            rPage.PageCreated(aSet);
            break;

        case RID_SVXPAGE_AREA:
//          ((SvxAreaTabPage&)rPage).SetColorTable(m_pViewElementListProvider->GetColorTable());
//          ((SvxAreaTabPage&)rPage).SetGradientList(m_pViewElementListProvider->GetGradientList());
//          ((SvxAreaTabPage&)rPage).SetHatchingList(m_pViewElementListProvider->GetHatchList());
//          ((SvxAreaTabPage&)rPage).SetBitmapList(m_pViewElementListProvider->GetBitmapList());
//          ((SvxAreaTabPage&)rPage).SetPageType(&nPageType);
//          ((SvxAreaTabPage&)rPage).SetDlgType(&nDlgType);
//          ((SvxAreaTabPage&)rPage).SetGrdChgd(&nGradientListState);
//          ((SvxAreaTabPage&)rPage).SetHtchChgd(&nHatchingListState);
//          ((SvxAreaTabPage&)rPage).SetBmpChgd(&nBitmapListState);
//          ((SvxAreaTabPage&)rPage).SetColorChgd(&nColorTableState);
//          ((SvxAreaTabPage&)rPage).Construct();
//          ((SvxAreaTabPage&)rPage).ActivatePage(*GetInputSetImpl());
            aSet.Put(SvxColorTableItem(m_pViewElementListProvider->GetColorTable(),SID_COLOR_TABLE));
            aSet.Put(SvxGradientListItem(m_pViewElementListProvider->GetGradientList(),SID_GRADIENT_LIST));
            aSet.Put(SvxHatchListItem(m_pViewElementListProvider->GetHatchList(),SID_HATCH_LIST));
            aSet.Put(SvxBitmapListItem(m_pViewElementListProvider->GetBitmapList(),SID_BITMAP_LIST));
            aSet.Put(SfxUInt16Item(SID_PAGE_TYPE,nPageType));
            aSet.Put(SfxUInt16Item(SID_DLG_TYPE,nDlgType));
            rPage.PageCreated(aSet);
            rPage.ActivatePage(*GetInputSetImpl());
            break;

        case RID_SVXPAGE_TRANSPARENCE:
//          ( (SvxTransparenceTabPage&) rPage ).SetPageType( &nPageType );
//          ( (SvxTransparenceTabPage&) rPage ).SetDlgType( &nDlgType );
//          ( (SvxTransparenceTabPage&) rPage ).Construct();
            aSet.Put (SfxUInt16Item(SID_PAGE_TYPE,nPageType));
            aSet.Put (SfxUInt16Item(SID_DLG_TYPE,nDlgType));
            rPage.PageCreated(aSet);
            break;

        case RID_SVXPAGE_CHAR_NAME:

            //CHINA001 ((SvxCharNamePage&)rPage).
            //CHINA001     SetFontList(SvxFontListItem(m_pViewElementListProvider->getFontList()));
            aSet.Put (SvxFontListItem(m_pViewElementListProvider->getFontList(), SID_ATTR_CHAR_FONTLIST)); //CHINA001
            rPage.PageCreated(aSet); //CHINA001
            break;

        case RID_SVXPAGE_CHAR_EFFECTS:
            //CHINA001 ((SvxCharEffectsPage&) rPage).DisableControls( DISABLE_CASEMAP );
            aSet.Put (SfxUInt16Item(SID_DISABLE_CTL,DISABLE_CASEMAP)); //CHINA001
            rPage.PageCreated(aSet);
            break;

        case TP_AXIS_LABEL:
        {
            bool bShowStaggeringControls = m_pParameter->CanAxisLabelsBeStaggered();
            ((SchAxisLabelTabPage&)rPage).ShowStaggeringControls( bShowStaggeringControls );
            break;
        }

        case TP_ALIGNMENT:
            break;

        case TP_SCALE_Y:
            {
                SchScaleYAxisTabPage & rAxisTabPage = static_cast< SchScaleYAxisTabPage & >( rPage );

                // #101318# use own number formatter with higher precision for rendering the values in the dialog.
                rAxisTabPage.SetNumFormatter( m_pViewElementListProvider->GetOwnNumberFormatter() );
            }
            break;

        case RID_SVXPAGE_NUMBERFORMAT:
           //CHINA001  ((SvxNumberFormatTabPage&)rPage).SetNumberFormatList(SvxNumberInfoItem(m_pViewElementListProvider->GetNumFormatter()));
            aSet.Put (SvxNumberInfoItem( m_pViewElementListProvider->GetNumFormatter(), (const USHORT)SID_ATTR_NUMBERFORMAT_INFO)); //CHINA001
            rPage.PageCreated(aSet);
            break;

        case TP_STAT:
            static_cast< SchStatisticTabPage & >( rPage ).EnableRegression(
                m_pParameter->HasRegressionProperties() );
            break;
    }
}

//.............................................................................
} //namespace chart
//.............................................................................
