/*************************************************************************
 *
 *  $RCSfile: dlg_ObjectProperties.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-06 09:58:26 $
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

#ifndef _SVX_CHARDLG_HXX
#include <svx/chardlg.hxx>
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


#ifndef _SVX_NUMFMT_HXX
#include <svx/numfmt.hxx>
#endif

#define ITEMID_NUMBERINFO   SID_ATTR_NUMBERFORMAT_INFO
#ifndef _SVX_NUMINF_HXX
#include <svx/numinf.hxx>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

/************************************************************************/

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
        case OBJECTTYPE_DATA_FUNCTION:
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
/*
USHORT SchAttribTabDlg::GetResId(AttrType eType)
{
    switch (eType)
    {
        case ATTR_TITLE:            return TAB_TITLE;
        case ATTR_LEGEND:           return TAB_LEGEND;
        case ATTR_DATA_ROW:         return TAB_DATA_ROW;
        case ATTR_DATA_LINE:        return TAB_DATA_LINE;
        case ATTR_DATA_POINT:       return TAB_DATA_POINT;
        case ATTR_AXIS:             return TAB_AXIS;
        case ATTR_X_AXIS_2D:
        case ATTR_X_AXIS_3D:        return TAB_X_AXIS;
        case ATTR_Y_AXIS_2D:
        case ATTR_Y_AXIS_3D:        return TAB_Y_AXIS;
        case ATTR_Z_AXIS:           return TAB_Z_AXIS;
        case ATTR_GRID:             return TAB_GRID;
        case ATTR_LINE:             return TAB_LINE;
        case ATTR_DIAGRAM_AREA:     return TAB_DIAGRAM_AREA;
        case ATTR_DIAGRAM_WALL:     return TAB_DIAGRAM_WALL;
        case ATTR_DIAGRAM_FLOOR:    return TAB_DIAGRAM_FLOOR;
        case ATTR_DIAGRAM_STOCK_LOSS: return TAB_DIAGRAM_STOCK_LOSS;
        case ATTR_DIAGRAM_STOCK_PLUS: return TAB_DIAGRAM_STOCK_PLUS;
        default:                    return 0;
    }
}
*/
//-------------------------------------------------------------------

OldModelWrapper::OldModelWrapper()
{
}

OldModelWrapper::~OldModelWrapper()
{
}
/*
BOOL OldModelWrapper::IsXYChart() const
{
    return false;
}
*/
BOOL OldModelWrapper::IsBar() const
{
    return true;
}
BOOL OldModelWrapper::Is3DChart() const
{
    return false;
}
/*
BOOL OldModelWrapper::Is3DPie() const
{
    return false;
}
*/
BOOL OldModelWrapper::IsStatisticChart() const
{
    return true;
}

BOOL OldModelWrapper::HasAreaProperties() const
{
    return true;
}
BOOL OldModelWrapper::HasScaleProperties() const
{
    return true;
}

BOOL OldModelWrapper::ProvidesSecondaryYAxis() const
{
    return true;
}
BOOL OldModelWrapper::IsCol( long nRow ) const
{
    return false;
}

/*************************************************************************
|*
|* Konstruktor des Tab-Dialogs: Fuegt die Seiten zum Dialog hinzu
|*
\************************************************************************/

//const USHORT nNoArrowDlg          = 1100;
const USHORT nNoArrowNoShadowDlg    = 1101;

SchAttribTabDlg::SchAttribTabDlg(Window* pParent, ObjectType eType,
                                 const SfxItemSet* pAttr,
                                 const ViewElementListProvider* pViewElementListProvider,
                                 const OldModelWrapper* pModel,
                                 bool  AffectsMultipleObjects,
                                 const SfxItemSet* pSymbolAttr,
                                 Graphic aGraphic)
    : SfxTabDialog(pParent, SchResId(GetResId(eType)), pAttr)
    , eObjectType(eType)
    , bAffectsMultipleObjects(AffectsMultipleObjects)
    , nDlgType(nNoArrowNoShadowDlg)
    , nPageType(0)
    , m_pModelWrapper( pModel )
    , m_pViewElementListProvider( pViewElementListProvider )
    , mpSymbolAttr(pSymbolAttr)
    , maSymbolGraphic(aGraphic)
/*
    , pColorTab(pViewElementListProvider->GetColorTable())
    , pGradientList(pViewElementListProvider->GetGradientList())
    , pHatchingList(pViewElementListProvider->GetHatchList())
    , pBitmapList(pViewElementListProvider->GetBitmapList())
    , pDashList(pViewElementListProvider->GetDashList())
    , pLineEndList(pViewElementListProvider->GetLineEndList())
*/
    , nColorTableState(CT_NONE)
    , nGradientListState(CT_NONE)
    , nHatchingListState(CT_NONE)
    , nBitmapListState(CT_NONE)
{
    FreeResource();
    switch (eObjectType)
    {
        case OBJECTTYPE_TITLE://ATTR_TITLE:
            AddTabPage(RID_SVXPAGE_LINE, SvxLineTabPage::Create, NULL);
            AddTabPage(RID_SVXPAGE_AREA, SvxAreaTabPage::Create, NULL);
            AddTabPage(RID_SVXPAGE_TRANSPARENCE, SvxTransparenceTabPage::Create, NULL);
            AddTabPage(RID_SVXPAGE_CHAR_NAME, SvxCharNamePage::Create, NULL);
            AddTabPage(RID_SVXPAGE_CHAR_EFFECTS, SvxCharEffectsPage::Create, NULL);
            AddTabPage(TP_ALIGNMENT, SchAlignmentTabPage::Create, NULL);
            /*if (//(nID == CHOBJID_DIAGRAM_TITLE_X_AXIS) || (nID == CHOBJID_DIAGRAM_TITLE_Z_AXIS) ||
                (nID == CHOBJID_TITLE_MAIN) || (nID == CHOBJID_TITLE_SUB))
                RemoveTabPage (TP_ALIGNMENT);*/
////            nDlgType = nNoArrowDlg;
            break;

        case OBJECTTYPE_LEGEND://ATTR_LEGEND:
            AddTabPage(RID_SVXPAGE_LINE, SvxLineTabPage::Create, NULL);
            AddTabPage(RID_SVXPAGE_AREA, SvxAreaTabPage::Create, NULL);
            AddTabPage(RID_SVXPAGE_TRANSPARENCE, SvxTransparenceTabPage::Create, NULL);
            AddTabPage(RID_SVXPAGE_CHAR_NAME, SvxCharNamePage::Create, NULL);
            AddTabPage(RID_SVXPAGE_CHAR_EFFECTS, SvxCharEffectsPage::Create, NULL);
            AddTabPage(TP_LEGEND_POS, SchLegendPosTabPage::Create, NULL);
////            nDlgType = nNoArrowDlg;
            break;

        case OBJECTTYPE_DATA_SERIES://ATTR_DATA_ROW:
        case OBJECTTYPE_DATA_POINT://ATTR_DATA_POINT:
        case OBJECTTYPE_DATA_LABEL:
        case OBJECTTYPE_DATA_LABELS:
            AddTabPage(RID_SVXPAGE_LINE, SvxLineTabPage::Create, NULL);
            AddTabPage(RID_SVXPAGE_AREA, SvxAreaTabPage::Create, NULL);
            AddTabPage(RID_SVXPAGE_TRANSPARENCE, SvxTransparenceTabPage::Create, NULL);
            if(!m_pModelWrapper->HasAreaProperties())
            {
                RemoveTabPage(RID_SVXPAGE_AREA);
                RemoveTabPage(RID_SVXPAGE_TRANSPARENCE);
            }
            AddTabPage(RID_SVXPAGE_CHAR_NAME, SvxCharNamePage::Create, NULL);
            AddTabPage(RID_SVXPAGE_CHAR_EFFECTS, SvxCharEffectsPage::Create, NULL);
            AddTabPage(TP_DATA_DESCR, SchDataDescrTabPage::Create, NULL);
            AddTabPage(TP_STAT, SchStatisticTabPage::Create, NULL);
            if(    OBJECTTYPE_DATA_POINT == eObjectType
                || OBJECTTYPE_DATA_LABEL == eObjectType
                || !m_pModelWrapper->IsStatisticChart())
            {
                RemoveTabPage (TP_STAT);
            }
            AddTabPage(TP_LAYOUT,SchLayoutTabPage::Create, NULL);
            //  Layout tab only for 3D bar and column charts.
            if( !(m_pModelWrapper->Is3DChart() && (m_pModelWrapper->IsBar() || m_pModelWrapper->IsCol(0))))
                RemoveTabPage(TP_LAYOUT);
            /*
            if(m_pModelWrapper->Is3DPie() )
                RemoveTabPage(TP_LAYOUT);
            */

            AddTabPage(TP_OPTIONS,SchOptionTabPage::Create, NULL);
            //if ((((const SfxInt32Item &)pAttr->Get (SCHATTR_DUMMY1)).GetValue () == CHOBJID_DIAGRAM_DATA) ||

            if(    OBJECTTYPE_DATA_POINT == eObjectType
                || OBJECTTYPE_DATA_LABEL == eObjectType
                || !m_pModelWrapper->ProvidesSecondaryYAxis() )
                RemoveTabPage(TP_OPTIONS);

////            nDlgType = nNoArrowNoShadowDlg;
            break;

            /*
        case OBJECTTYPE_DATA_SERIES://ATTR_DATA_LINE:

            AddTabPage(RID_SVXPAGE_LINE, SvxLineTabPage::Create, NULL);
            AddTabPage(RID_SVXPAGE_CHAR_NAME, SvxCharNamePage::Create, NULL);
            AddTabPage(RID_SVXPAGE_CHAR_EFFECTS, SvxCharEffectsPage::Create, NULL);
            AddTabPage(TP_DATA_DESCR, SchDataDescrTabPage::Create, NULL);
            //if (m_pModelWrapper->IsStatisticChart())
            AddTabPage(TP_STAT, SchStatisticTabPage::Create, NULL);
            AddTabPage(TP_OPTIONS,SchOptionTabPage::Create, NULL);
            if (!m_pModelWrapper->IsStatisticChart())
                RemoveTabPage (TP_STAT);
            if(!m_pModelWrapper->ProvidesSecondaryYAxis())
                RemoveTabPage(TP_OPTIONS);
            nDlgType = nNoArrowDlg;
            break;
            */


        /*
        case OBJECTTYPE_AXIS://ATTR_AXIS:
            AddTabPage(RID_SVXPAGE_LINE, SvxLineTabPage::Create, NULL);
            AddTabPage(RID_SVXPAGE_CHAR_NAME, SvxCharNamePage::Create, NULL);
            AddTabPage(RID_SVXPAGE_CHAR_EFFECTS, SvxCharEffectsPage::Create, NULL);
            AddTabPage(TP_AXIS_LABEL, SchAxisLabelTabPage::Create, NULL);
            nDlgType = nNoArrowDlg;
            break;

        case ATTR_X_AXIS_2D:
        */
        case OBJECTTYPE_AXIS:
        {
            AddTabPage(RID_SVXPAGE_LINE, SvxLineTabPage::Create, NULL);
            AddTabPage(RID_SVXPAGE_CHAR_NAME, SvxCharNamePage::Create, NULL);
            AddTabPage(RID_SVXPAGE_CHAR_EFFECTS, SvxCharEffectsPage::Create, NULL);
            AddTabPage(TP_AXIS_LABEL, SchAxisLabelTabPage::Create, NULL);
            AddTabPage(TP_SCALE_Y, SchScaleYAxisTabPage::Create, NULL);
            AddTabPage(RID_SVXPAGE_NUMBERFORMAT, SvxNumberFormatTabPage::Create, NULL);

            if( bAffectsMultipleObjects || !m_pModelWrapper->HasScaleProperties() )
            {
                RemoveTabPage (TP_SCALE_Y);
                RemoveTabPage (RID_SVXPAGE_NUMBERFORMAT);
            }
////            nDlgType = nNoArrowDlg;
////            ((SfxItemSet * const) pAttr)->ClearItem (SCHATTR_AXISTYPE);
////            ((SfxItemSet *) pAttr)->Put (SfxInt32Item (SCHATTR_AXISTYPE, CHART_AXIS_X));

////            nAxisType = 1;
            break;
        }

        /*
        case ATTR_Y_AXIS_2D:
        case ATTR_Y_AXIS_3D:
            AddTabPage(RID_SVXPAGE_LINE, SvxLineTabPage::Create, NULL);
            AddTabPage(RID_SVXPAGE_CHAR_NAME, SvxCharNamePage::Create, NULL);
            AddTabPage(RID_SVXPAGE_CHAR_EFFECTS, SvxCharEffectsPage::Create, NULL);
            AddTabPage(TP_SCALE_Y, SchScaleYAxisTabPage::Create, NULL);
            AddTabPage(RID_SVXPAGE_NUMBERFORMAT, SvxNumberFormatTabPage::Create, NULL);
            AddTabPage(TP_AXIS_LABEL, SchAxisLabelTabPage::Create, NULL);
            nDlgType = nNoArrowDlg;
////            ((SfxItemSet * const) pAttr)->ClearItem (SCHATTR_AXISTYPE);
////            ((SfxItemSet *) pAttr)->Put (SfxInt32Item (SCHATTR_AXISTYPE, CHART_AXIS_Y));

////            nAxisType = 2;
            break;

        case ATTR_Z_AXIS:
            AddTabPage(RID_SVXPAGE_LINE, SvxLineTabPage::Create, NULL);
            AddTabPage(RID_SVXPAGE_CHAR_NAME, SvxCharNamePage::Create, NULL);
            AddTabPage(RID_SVXPAGE_CHAR_EFFECTS, SvxCharEffectsPage::Create, NULL);
            AddTabPage(TP_AXIS_LABEL, SchAxisLabelTabPage::Create, NULL);
            nDlgType = nNoArrowDlg;
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
        case OBJECTTYPE_DATA_FUNCTION:
        case OBJECTTYPE_DATA_STOCK_RANGE://ATTR_LINE
            AddTabPage(RID_SVXPAGE_LINE, SvxLineTabPage::Create, NULL);
////            nDlgType = nNoArrowDlg;
            break;

        case OBJECTTYPE_DATA_STOCK_LOSS://ATTR_DIAGRAM_STOCK_LOSS:
        case OBJECTTYPE_DATA_STOCK_GAIN://ATTR_DIAGRAM_STOCK_PLUS:
        case OBJECTTYPE_PAGE://ATTR_DIAGRAM_AREA:
        case OBJECTTYPE_DIAGRAM_FLOOR://ATTR_DIAGRAM_FLOOR:
        case OBJECTTYPE_DIAGRAM_WALL://ATTR_DIAGRAM_WALL:
        case OBJECTTYPE_DIAGRAM://ATTR_DIAGRAM_WALL: //@todo
            AddTabPage(RID_SVXPAGE_LINE, SvxLineTabPage::Create, NULL);
            AddTabPage(RID_SVXPAGE_AREA, SvxAreaTabPage::Create, NULL);
            AddTabPage(RID_SVXPAGE_TRANSPARENCE, SvxTransparenceTabPage::Create, NULL);
////            nDlgType = nNoArrowNoShadowDlg;
            break;
    }
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

SchAttribTabDlg::~SchAttribTabDlg()
{
}

/*************************************************************************
|*
|*
|*
\************************************************************************/

void SchAttribTabDlg::PageCreated(USHORT nId, SfxTabPage &rPage)
{
    switch (nId)
    {
        case TP_LAYOUT:
        break;
        case RID_SVXPAGE_LINE:
            ((SvxLineTabPage&)rPage).SetColorTable(m_pViewElementListProvider->GetColorTable());
            ((SvxLineTabPage&)rPage).SetDashList(m_pViewElementListProvider->GetDashList());
            ((SvxLineTabPage&)rPage).SetLineEndList(m_pViewElementListProvider->GetLineEndList());
            ((SvxLineTabPage&)rPage).SetPageType(&nPageType);
            ((SvxLineTabPage&)rPage).SetDlgType(&nDlgType);
            ((SvxLineTabPage&)rPage).Construct();
            ((SvxLineTabPage&)rPage).ActivatePage(*GetInputSetImpl());
            if(mpSymbolAttr)//unfein, aber wenn man schon die Attribute setzt, wird es wohl ne Linie
            {               //mit Symbolen sein.
                ((SvxLineTabPage&)rPage).ShowSymbolControls(TRUE);
                ((SvxLineTabPage&)rPage).SetSymbolList(m_pViewElementListProvider->GetSymbolList());
                ((SvxLineTabPage&)rPage).SetSymbolAttr(mpSymbolAttr);
                ((SvxLineTabPage&)rPage).SetAutoSymbolGraphic(&maSymbolGraphic);
            }
            break;

        case RID_SVXPAGE_AREA:
            ((SvxAreaTabPage&)rPage).SetColorTable(m_pViewElementListProvider->GetColorTable());
            ((SvxAreaTabPage&)rPage).SetGradientList(m_pViewElementListProvider->GetGradientList());
            ((SvxAreaTabPage&)rPage).SetHatchingList(m_pViewElementListProvider->GetHatchList());
            ((SvxAreaTabPage&)rPage).SetBitmapList(m_pViewElementListProvider->GetBitmapList());
            ((SvxAreaTabPage&)rPage).SetPageType(&nPageType);
            ((SvxAreaTabPage&)rPage).SetDlgType(&nDlgType);
            ((SvxAreaTabPage&)rPage).SetGrdChgd(&nGradientListState);
            ((SvxAreaTabPage&)rPage).SetHtchChgd(&nHatchingListState);
            ((SvxAreaTabPage&)rPage).SetBmpChgd(&nBitmapListState);
            ((SvxAreaTabPage&)rPage).SetColorChgd(&nColorTableState);
            ((SvxAreaTabPage&)rPage).Construct();
            ((SvxAreaTabPage&)rPage).ActivatePage(*GetInputSetImpl());
            break;

        case RID_SVXPAGE_TRANSPARENCE:
            ( (SvxTransparenceTabPage&) rPage ).SetPageType( &nPageType );
            ( (SvxTransparenceTabPage&) rPage ).SetDlgType( &nDlgType );
            ( (SvxTransparenceTabPage&) rPage ).Construct();
            break;

        case RID_SVXPAGE_CHAR_NAME:
            ((SvxCharNamePage&)rPage).
                SetFontList(SvxFontListItem(m_pViewElementListProvider->getFontList()));
            break;

        case RID_SVXPAGE_CHAR_EFFECTS:
            ((SvxCharEffectsPage&) rPage).DisableControls( DISABLE_CASEMAP );
            break;

        case TP_AXIS_LABEL:
        {
            //for 3D axis staggering is disabled
            BOOL bShowStaggeringControls = !m_pModelWrapper->Is3DChart();
            ((SchAxisLabelTabPage&)rPage).ShowStaggeringControls( bShowStaggeringControls );
////            ((SchAxisLabelTabPage&)rPage).AllowTextOverlap( TRUE );
            break;
        }

        case TP_ALIGNMENT:
        {
            /*
            OrderMode eMode;
            switch (eObjectType)
            {
                case ATTR_X_AXIS_2D:
                    eMode = CHORDMODE_X_AXIS;
                    break;

                case ATTR_Y_AXIS_2D:
                    eMode = CHORDMODE_Y_AXIS;
                    break;

                default:
                    eMode = CHORDMODE_NONE;
                    break;
            }
            ((SchAlignmentTabPage&)rPage).SetOrderMode(eMode);
            xxx
            */
            ((SchAlignmentTabPage&)rPage).SetTextCanOverlap(/*m_pModelWrapper->IsReal3D()*/TRUE);
            break;
        }

        case TP_SCALE_Y:
            {
                SchScaleYAxisTabPage & rAxisTabPage = static_cast< SchScaleYAxisTabPage & >( rPage );

                // #101318# use own number formatter with higher precision for
                // rendering the values in the dialog.
                rAxisTabPage.SetNumFormatter( m_pViewElementListProvider->GetOwnNumberFormatter() );
            }
            break;

        case RID_SVXPAGE_NUMBERFORMAT:
            ((SvxNumberFormatTabPage&)rPage).SetNumberFormatList(SvxNumberInfoItem(m_pViewElementListProvider->GetNumFormatter()));
            break;
    }
}

//.............................................................................
} //namespace chart
//.............................................................................
