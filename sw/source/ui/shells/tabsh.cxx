/*************************************************************************
 *
 *  $RCSfile: tabsh.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: jp $ $Date: 2001-02-21 17:38:36 $
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

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef _UIPARAM_HXX
#include <uiparam.hxx>
#endif
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _ZFORLIST_HXX
#define _ZFORLIST_DECLARE_TABLE
#include <svtools/zforlist.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_MODULEOPTIONS_HXX
#include <svtools/moduleoptions.hxx>
#endif
#ifndef _SVX_RULRITEM_HXX //autogen
#include <svx/rulritem.hxx>
#endif
#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif
#ifndef _SVX_SRCHITEM_HXX
#include <svx/srchitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX //autogen
#include <svx/boxitem.hxx>
#endif
#ifndef _SVX_SHADITEM_HXX //autogen
#include <svx/shaditem.hxx>
#endif
#ifndef _SVX_SPLTITEM_HXX //autogen
#include <svx/spltitem.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX //autogen
#include <svx/langitem.hxx>
#endif
#ifndef _SVX_KEEPITEM_HXX //autogen
#include <svx/keepitem.hxx>
#endif
#ifndef _SVX_BOLNITEM_HXX //autogen
#include <svx/bolnitem.hxx>
#endif
#ifndef _SVX_COLRITEM_HXX //autogen
#include <svx/colritem.hxx>
#endif
#ifndef _SVX_NUMINF_HXX //autogen
#include <svx/numinf.hxx>
#endif
#ifndef _ZFORMAT_HXX //autogen
#include <svtools/zformat.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX //autogen
#include <sfx2/bindings.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXOBJFACE_HXX //autogen
#include <sfx2/objface.hxx>
#endif


#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FMTCLDS_HXX //autogen
#include <fmtclds.hxx>
#endif
#ifndef _FMTTSPLT_HXX //autogen
#include <fmtlsplt.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _SWMODULE_HXX
#include <swmodule.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _WVIEW_HXX
#include <wview.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _UITOOL_HXX
#include <uitool.hxx>
#endif
#ifndef _INPUTWIN_HXX
#include <inputwin.hxx>
#endif
#ifndef _UIITEMS_HXX
#include <uiitems.hxx>
#endif
#ifndef _USRPREF_HXX
#include <usrpref.hxx>
#endif
#ifndef _TABSH_HXX
#include <tabsh.hxx>
#endif
#ifndef _TABLEDLG_HXX
#include <tabledlg.hxx>
#endif
#ifndef _TABLEMGR_HXX
#include <tablemgr.hxx>
#endif
#ifndef _TAUTOFMT_HXX
#include <tautofmt.hxx>
#endif
#ifndef _CELLATR_HXX
#include <cellatr.hxx>
#endif
#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>
#endif
#ifndef _ROWHT_HXX
#include <rowht.hxx>
#endif
#ifndef _SPLIT_HXX
#include <split.hxx>
#endif
#ifndef _INSRC_HXX
#include <insrc.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _TBLNUMFM_HXX
#include <tblnumfm.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _SPLITTBL_HXX
#include <splittbl.hxx>
#endif
#ifndef _MERGETBL_HXX
#include <mergetbl.hxx>
#endif
#ifndef _TBLSEL_HXX
#include <tblsel.hxx>
#endif

#ifndef _POPUP_HRC
#include <popup.hrc>
#endif
#ifndef _SHELLS_HRC
#include <shells.hrc>
#endif
#ifndef _TABLE_HRC
#include <table.hrc>
#endif
#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _GLOBALS_HRC
#include <globals.hrc>
#endif
#ifndef _HELPID_H
#include <helpid.h>
#endif

#define SwTableShell
#ifndef _ITEMDEF_HXX
#include <itemdef.hxx>
#endif
#ifndef _SWSLOTS_HXX
#include <swslots.hxx>
#endif

//-----------------------------------------------------------------------------
BOOL lcl_IsNumeric(const String& rStr)
{
    for(xub_StrLen i = 0; i < rStr.Len(); i++)
    {
        sal_Unicode c = rStr.GetChar(i);
         if((c < '0') || (c > '9'))
             return FALSE;
    }
    return TRUE;

}

SFX_IMPL_INTERFACE(SwTableShell, SwBaseShell, SW_RES(STR_SHELLNAME_TABLE))
{
    SFX_POPUPMENU_REGISTRATION(SW_RES(MN_TAB_POPUPMENU));
    SFX_OBJECTBAR_REGISTRATION(SFX_OBJECTBAR_OBJECT, SW_RES(RID_TABLE_TOOLBOX));
    SFX_OBJECTMENU_REGISTRATION(SID_OBJECTMENU0, SW_RES(MN_OBJECTMENU_TABLE));
}


TYPEINIT1(SwTableShell,SwBaseShell)

/************************************************************************/


const USHORT __FAR_DATA aUITableAttrRange[] =
{
    FN_PARAM_TABLE_NAME,            FN_PARAM_TABLE_NAME,
    FN_PARAM_TABLE_HEADLINE,        FN_PARAM_TABLE_HEADLINE,
    FN_PARAM_TABLE_SPACE,           FN_PARAM_TABLE_SPACE,
    FN_TABLE_REP,                   FN_TABLE_REP,
    SID_RULER_BORDERS,              SID_RULER_BORDERS,
    RES_LR_SPACE,                   RES_UL_SPACE,
    SID_ATTR_BORDER_INNER,          SID_ATTR_BORDER_SHADOW,
    RES_BOX,                        RES_SHADOW,
    RES_BACKGROUND,                 RES_BACKGROUND,
    SID_BACKGRND_DESTINATION,       SID_BACKGRND_DESTINATION,
    SID_HTML_MODE,                  SID_HTML_MODE,
    SID_ATTR_BRUSH_ROW,             SID_ATTR_BRUSH_TABLE,
    RES_PAGEDESC,                   RES_BREAK,
    RES_KEEP,                       RES_KEEP,
    RES_LAYOUT_SPLIT,               RES_LAYOUT_SPLIT,
    FN_TABLE_SET_VERT_ALIGN,        FN_TABLE_SET_VERT_ALIGN,
    0
};




void lcl_SetAttr( SwWrtShell &rSh, const SfxPoolItem &rItem )
{
    SfxItemSet aSet( rSh.GetView().GetPool(), rItem.Which(), rItem.Which(), 0);
    aSet.Put( rItem );
    rSh.SetTblAttr( aSet );
}

/************************************************************************/


SwTableRep*  lcl_TableParamToItemSet( SfxItemSet& rSet, SwWrtShell &rSh )
{
    SwFrmFmt *pFmt = rSh.GetTableFmt();
    SwTabCols aCols;
    rSh.GetTabCols( aCols );

    //Ersteinmal die einfachen Attribute besorgen.
    rSet.Put( SfxStringItem( FN_PARAM_TABLE_NAME, pFmt->GetName()));
    rSet.Put( SfxBoolItem( FN_PARAM_TABLE_HEADLINE, rSh.IsHeadlineRepeat()) );
    rSet.Put( pFmt->GetShadow() );
    rSet.Put(SfxUInt16Item(FN_TABLE_SET_VERT_ALIGN, rSh.GetBoxAlign()));
    SvxULSpaceItem aULSpace( pFmt->GetULSpace() );
    rSet.Put( aULSpace );

    USHORT  nBackgroundDestination = rSh.GetViewOptions()->GetTblDest();
    rSet.Put(SwBackgroundDestinationItem(SID_BACKGRND_DESTINATION, nBackgroundDestination ));
    SvxBrushItem aBrush( RES_BACKGROUND );
//      rSh.GetBoxBackground(aBrush);
//  rSet.Put( aBrush );
            rSh.GetRowBackground(aBrush);
    rSet.Put( aBrush, SID_ATTR_BRUSH_ROW );
        rSh.GetTabBackground(aBrush);
    rSet.Put( aBrush, SID_ATTR_BRUSH_TABLE );
    FASTBOOL bTableSel = rSh.IsTableMode();
    if(!bTableSel)
    {
        rSh.StartAllAction();
        rSh.Push();
        rSh.GetView().GetViewFrame()->GetDispatcher()->Execute( FN_TABLE_SELECT_ALL, FALSE );
    }
    SvxBoxInfoItem aBoxInfo;

        // Tabellenvariante, wenn mehrere Tabellenzellen selektiert
    rSh.GetCrsr();                  //Damit GetCrsrCnt() auch das Richtige liefert
    aBoxInfo.SetTable          (rSh.IsTableMode() && rSh.GetCrsrCnt() > 1 ||
                                    !bTableSel);
        // Abstandsfeld immer anzeigen
    aBoxInfo.SetDist           ((BOOL) TRUE);
        // Minimalgroesse in Tabellen und Absaetzen setzen
    aBoxInfo.SetMinDist( !bTableSel || rSh.IsTableMode() ||
                            rSh.GetSelectionType() &
                            (SwWrtShell::SEL_TXT | SwWrtShell::SEL_TBL));
        // Default-Abstand immer setzen
    aBoxInfo.SetDefDist        (MIN_BORDER_DIST);
        // Einzelne Linien koennen nur in Tabellen DontCare-Status haben
    aBoxInfo.SetValid( VALID_DISABLE, !bTableSel || !rSh.IsTableMode() );

    rSet.Put(aBoxInfo);
    rSh.GetTabBorders( rSet );

    if(!bTableSel)
    {
        rSh.ClearMark();
        rSh.Pop(FALSE);
        rSh.EndAllAction();
    }

    SwTabCols aTabCols;
    rSh.GetTabCols( aTabCols );
    SvxColumnItem aColItem;


    // Pointer wird nach der Dialogausfuehrung geloescht
    SwTableRep* pRep = new SwTableRep( aTabCols, rSh.IsTblComplex());
    pRep->SetSpace(aCols.GetRightMax());

    USHORT nPercent = 0;
    long nWidth = ::GetTableWidth(pFmt, aCols, &nPercent, &rSh );
    // Die Tabellenbreite ist fuer relative Angaben nicht korrekt
    if(nPercent)
        nWidth = pRep->GetSpace() * nPercent / 100;
    USHORT nAlign = pFmt->GetHoriOrient().GetHoriOrient();
    pRep->SetAlign(nAlign);
    SvxLRSpaceItem aLRSpace( pFmt->GetLRSpace() );
    SwTwips nLeft = aLRSpace.GetLeft();
    SwTwips nRight = aLRSpace.GetRight();
    SwTwips nDiff = pRep->GetSpace() - nRight - nLeft - nWidth;
    if(nAlign != HORI_FULL && Abs(nDiff) > 2)
    {
        SwTwips nLR = pRep->GetSpace() - nWidth;
        switch ( nAlign )
        {
            case HORI_CENTER: nLeft = nRight = nLR / 2;
            break;
            case HORI_LEFT: nRight = nLR; nLeft = 0;
            break;
            case HORI_RIGHT: nLeft = nLR, nRight = 0;
            break;
            case HORI_LEFT_AND_WIDTH:
                nRight = nLR - nLeft;
            break;
            case HORI_NONE:
                if(!nPercent)
                    nWidth = pRep->GetSpace() - nLeft - nRight;
            break;
        }
    }
    pRep->SetLeftSpace(nLeft);
    pRep->SetRightSpace(nRight);

    pRep->SetWidth(nWidth);
    pRep->SetWidthPercent(nPercent);
    // sind einzelne Zeilen/Zellen selektiert, wird die Spaltenbearbeitung veraendert
    pRep->SetLineSelected(bTableSel && ! rSh.HasWholeTabSelection());
    rSet.Put(SwPtrItem(FN_TABLE_REP, pRep));
    return pRep;
}

/************************************************************************/


void lcl_ItemSetToTableParam( const SfxItemSet& rSet,
                                SwWrtShell &rSh )
{
    rSh.StartAllAction();
    rSh.StartUndo( UNDO_TABLE_ATTR );
    const SfxPoolItem* pItem = 0;

    SwViewOption aUsrPref( *rSh.GetViewOptions() );
    USHORT nBackgroundDestination = aUsrPref.GetTblDest();
    if(SFX_ITEM_SET == rSet.GetItemState(SID_BACKGRND_DESTINATION, FALSE, &pItem))
    {
        nBackgroundDestination = ((SfxUInt16Item*)pItem)->GetValue();
        aUsrPref.SetTblDest((BYTE)nBackgroundDestination);
        SW_MOD()->ApplyUsrPref(aUsrPref, &rSh.GetView());
    }
    FASTBOOL bBorder = ( SFX_ITEM_SET == rSet.GetItemState( RES_BOX ) ||
            SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_BORDER_INNER ) );
    pItem = 0;
    BOOL bBackground = SFX_ITEM_SET == rSet.GetItemState( RES_BACKGROUND, FALSE, &pItem );
    const SfxPoolItem* pRowItem = 0, *pTableItem = 0;
    bBackground |= SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_BRUSH_ROW, FALSE, &pRowItem );
    bBackground |= SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_BRUSH_TABLE, FALSE, &pTableItem );
    if( bBackground || bBorder)
    {
        /*
         Die Umrandung wird auf die vorliegende Selektion angewendet
         Liegt keine Selektion vor, wird die Tabelle vollstaendig selektiert.
         Der Hintergrund wird immer auf den aktuellen Zustand angewendet.
         */

        FASTBOOL bTableSel = rSh.IsTableMode();
        rSh.StartAllAction();

        if(bBackground)
        {
            if(pItem)
                rSh.SetBoxBackground( *(const SvxBrushItem*)pItem );
            if(pRowItem)
            {
                SvxBrushItem aBrush(*(const SvxBrushItem*)pRowItem);
                aBrush.SetWhich(RES_BACKGROUND);
                rSh.SetRowBackground(aBrush);
            }
            if(pTableItem)
            {
                SvxBrushItem aBrush(*(const SvxBrushItem*)pTableItem);
                aBrush.SetWhich(RES_BACKGROUND);
                rSh.SetTabBackground( aBrush );
            }
        }

        if(bBorder)
        {
            rSh.Push();
            if(!bTableSel)
            {
                rSh.GetView().GetViewFrame()->GetDispatcher()->Execute( FN_TABLE_SELECT_ALL );
            }
            rSh.SetTabBorders( rSet );

            if(!bTableSel)
            {
                rSh.ClearMark();
            }
            rSh.Pop(FALSE);
        }
        rSh.EndAllAction();
    }

    SwTabCols aTabCols;
    BOOL bTabCols = FALSE;
    BOOL bSingleLine = FALSE;
    SwTableRep* pRep = 0;
    SwFrmFmt *pFmt = rSh.GetTableFmt();
    SfxItemSet aSet( rSh.GetAttrPool(), RES_FRMATR_BEGIN, RES_FRMATR_END-1 );
    if(SFX_ITEM_SET == rSet.GetItemState( FN_TABLE_REP, FALSE, &pItem ))
    {
        pRep = (SwTableRep*)((const SwPtrItem*)pItem)->GetValue();

        const SwTwips nWidth = pRep->GetWidth();
        if ( HORI_FULL == (SwHoriOrient)pRep->GetAlign() )
        {
            SwFmtHoriOrient aAttr( pFmt->GetHoriOrient() );
            aAttr.SetHoriOrient( HORI_FULL );
            aSet.Put( aAttr );
        }
        else
        {
            SwFmtFrmSize aSz( ATT_VAR_SIZE, nWidth );
            if(pRep->GetWidthPercent())
            {
                aSz.SetWidthPercent( (BYTE)pRep->GetWidthPercent() );
            }
            aSet.Put(aSz);
        }

        SvxLRSpaceItem aLRSpace( RES_LR_SPACE );
        aLRSpace.SetLeft(pRep->GetLeftSpace());
        aLRSpace.SetRight(pRep->GetRightSpace());
        aSet.Put( aLRSpace );

        SwHoriOrient eOrient = (SwHoriOrient)pRep->GetAlign();
        SwFmtHoriOrient aAttr( 0, eOrient );
        aSet.Put( aAttr );
    // Damit beim recording die Ausrichtung nicht durch die Abstaende ueberschrieben
    // wird, darf das Item nur bei manueller Ausrichtung aufgez. werden
        if(eOrient != HORI_NONE)
            ((SfxItemSet&)rSet).ClearItem( SID_ATTR_LRSPACE );


        if(pRep->HasColsChanged())
        {
            bTabCols = TRUE;
        }
    }


    if( SFX_ITEM_SET == rSet.GetItemState( FN_PARAM_TABLE_HEADLINE, FALSE, &pItem))
        rSh.SetHeadlineRepeat( ((SfxBoolItem*)pItem)->GetValue() );

    if( SFX_ITEM_SET == rSet.GetItemState( FN_TABLE_SET_VERT_ALIGN, FALSE, &pItem))
        rSh.SetBoxAlign(((SfxUInt16Item*)(pItem))->GetValue());

    if( SFX_ITEM_SET == rSet.GetItemState( FN_PARAM_TABLE_NAME, FALSE, &pItem ))
        rSh.SetTableName( *pFmt, ((const SfxStringItem*)pItem)->GetValue() );

    // kopiere die ausgesuchten Attribute in den ItemSet
    static USHORT __READONLY_DATA aIds[] =
        {
            RES_PAGEDESC,       RES_BREAK,
            RES_KEEP,           RES_LAYOUT_SPLIT,
            RES_UL_SPACE,       RES_SHADOW,
            0
        };
    for( const USHORT* pIds = aIds; *pIds; ++pIds )
        if( SFX_ITEM_SET == rSet.GetItemState( *pIds, FALSE, &pItem))
            aSet.Put( *pItem );

    if( aSet.Count() )
        rSh.SetTblAttr( aSet );

    if(bTabCols)
    {
        rSh.GetTabCols( aTabCols );
        bSingleLine = pRep->FillTabCols( aTabCols );
        rSh.SetTabCols( aTabCols, bSingleLine );
    }

    rSh.EndUndo( UNDO_TABLE_ATTR );
    rSh.EndAllAction();
}

/************************************************************************/


void lcl_TabGetMaxLineWidth(const SvxBorderLine* pBorderLine, SvxBorderLine& rBorderLine)
{
    if(pBorderLine->GetInWidth() > rBorderLine.GetInWidth())
        rBorderLine.SetInWidth(pBorderLine->GetInWidth());

    if(pBorderLine->GetOutWidth() > rBorderLine.GetOutWidth())
        rBorderLine.SetOutWidth(pBorderLine->GetOutWidth());

    if(pBorderLine->GetDistance() > rBorderLine.GetDistance())
        rBorderLine.SetDistance(pBorderLine->GetDistance());

    rBorderLine.SetColor(pBorderLine->GetColor());
}


void SwTableShell::Execute(SfxRequest &rReq)
{

    const SfxItemSet* pArgs = rReq.GetArgs();
    SwWrtShell &rSh = GetShell();

    //Erstmal die Slots, die keinen FrmMgr benoetigen.
    BOOL bMore = FALSE;
    const SfxPoolItem* pItem = 0;
    USHORT nSlot = rReq.GetSlot();
    if(pArgs)
        pArgs->GetItemState(GetPool().GetWhich(nSlot), FALSE, &pItem);
    switch ( nSlot )
    {
        case SID_ATTR_BORDER:
        {
/*          BOOL bPopCrsr = FALSE;
            if ( !rReq.IsAPI() )
            {
                //Keine Tabellenselektion -> Aenderung wird auf die gesamte
                //Tabelle.
                if ( !rSh.IsTableMode() )
                {
                    rSh.StartAction();
                    bPopCrsr = TRUE;
                    rSh.Push();
                    rSh.EnterStdMode();
                    rSh.MoveTable( fnTableCurr, fnTableStart );
                    rSh.SttSelect();
                    rSh.MoveTable( fnTableCurr, fnTableEnd );
                    rSh.EndSelect();
                }
            }*/
            if(!pArgs)
                break;
            //Items erzeugen, weil wir sowieso nacharbeiten muessen
            SvxBoxItem     aBox;
            SfxItemSet aCoreSet( GetPool(),
                            RES_BOX, RES_BOX,
                            SID_ATTR_BORDER_INNER, SID_ATTR_BORDER_INNER, 0);
            SvxBoxInfoItem aCoreInfo;
            aCoreSet.Put(aCoreInfo);
            rSh.GetTabBorders( aCoreSet );
            const SvxBoxItem& rCoreBox = (const SvxBoxItem&)
                                                    aCoreSet.Get(RES_BOX);
            const SfxPoolItem *pItem = 0;
            if ( pArgs->GetItemState(RES_BOX, TRUE, &pItem) == SFX_ITEM_SET )
            {
                aBox = *(SvxBoxItem*)pItem;
                if ( !rReq.IsAPI() )
                    aBox.SetDistance( Max(rCoreBox.GetDistance(),USHORT(55)) );
                else if ( aBox.GetDistance() < MIN_BORDER_DIST )
                    aBox.SetDistance( Max(rCoreBox.GetDistance(),(USHORT)MIN_BORDER_DIST)  );
            }
            else
                {ASSERT( !this, "Wo ist das Box-Item?" )}

            SvxBoxInfoItem aInfo;
            if (pArgs->GetItemState(SID_ATTR_BORDER_INNER, TRUE, &pItem) == SFX_ITEM_SET)
                aInfo = *(SvxBoxInfoItem*)pItem;
            aInfo.SetTable( TRUE );
            aInfo.SetValid( VALID_DISABLE, FALSE );


// Die Attribute aller Linien werden gelesen und das staerkste gewinnt
            const SvxBorderLine* pBorderLine;
            SvxBorderLine aBorderLine;
            if ((pBorderLine = rCoreBox.GetTop()) != NULL)
                lcl_TabGetMaxLineWidth(pBorderLine, aBorderLine);
            if ((pBorderLine = rCoreBox.GetBottom()) != NULL)
                lcl_TabGetMaxLineWidth(pBorderLine, aBorderLine);
            if ((pBorderLine = rCoreBox.GetLeft()) != NULL)
                lcl_TabGetMaxLineWidth(pBorderLine, aBorderLine);
            if ((pBorderLine = rCoreBox.GetRight()) != NULL)
                lcl_TabGetMaxLineWidth(pBorderLine, aBorderLine);
            if ((pBorderLine = aCoreInfo.GetHori()) != NULL)
                lcl_TabGetMaxLineWidth(pBorderLine, aBorderLine);
            if ((pBorderLine = aCoreInfo.GetVert()) != NULL)
                lcl_TabGetMaxLineWidth(pBorderLine, aBorderLine);

            if(aBorderLine.GetOutWidth() == 0)
            {
                aBorderLine.SetInWidth(0);
                aBorderLine.SetOutWidth(DEF_LINE_WIDTH_0);
            }

            BOOL bLine = FALSE;
            if ( (pBorderLine = aBox.GetTop()) != NULL)
                aBox.SetLine(&aBorderLine, BOX_LINE_TOP), bLine |= TRUE;
            if ((pBorderLine = aBox.GetBottom()) != NULL)
                aBox.SetLine(&aBorderLine, BOX_LINE_BOTTOM), bLine |= TRUE;
            if ((pBorderLine = aBox.GetLeft()) != NULL)
                aBox.SetLine(&aBorderLine, BOX_LINE_LEFT), bLine |= TRUE;
            if ((pBorderLine = aBox.GetRight()) != NULL)
                aBox.SetLine(&aBorderLine, BOX_LINE_RIGHT), bLine |= TRUE;
            if ((pBorderLine = aInfo.GetHori()) != NULL)
                aInfo.SetLine(&aBorderLine, BOXINFO_LINE_HORI), bLine |= TRUE;
            if ((pBorderLine = aInfo.GetVert()) != NULL)
                aInfo.SetLine(&aBorderLine, BOXINFO_LINE_VERT), bLine |= TRUE;

//          if ( bPopCrsr && !bLine )
//              aBox.SetDistance( 0 );

            aCoreSet.Put( aBox  );
            aCoreSet.Put( aInfo );
            rSh.SetTabBorders( aCoreSet );

/*          if ( bPopCrsr )
            {
                rSh.KillPams();
                rSh.Pop(FALSE);
                rSh.EndAction();
            }*/
        }
        break;
        case SID_ATTR_BRUSH:
            if(pItem)
                rSh.SetBoxBackground( *(SvxBrushItem*)pItem );
        break;
        case FN_INSERT_TABLE:
        case FN_FORMAT_TABLE_DLG:
        {
            SfxItemSet aCoreSet( GetPool(), aUITableAttrRange);

            SwTableRep* pTblRep = ::lcl_TableParamToItemSet( aCoreSet, rSh );
            SwTableTabDlg* pDlg = new SwTableTabDlg( GetView().GetWindow(),
                                                     GetPool(), &aCoreSet, &rSh);
            aCoreSet.Put(SfxUInt16Item(SID_HTML_MODE, ::GetHtmlMode(GetView().GetDocShell())));
            rSh.GetTblAttr(aCoreSet);
            // GetTblAttr buegelt den Background ueber!
            SvxBrushItem aBrush( RES_BACKGROUND );
                rSh.GetBoxBackground(aBrush);
            aCoreSet.Put( aBrush );

            if ( pDlg->Execute() == RET_OK )
            {
                const SfxItemSet* pOutSet = pDlg->GetOutputItemSet();
                ::lcl_ItemSetToTableParam( *pOutSet, rSh );
            }
            delete pDlg;
            delete pTblRep;
            SfxBindings& rBindings = GetView().GetViewFrame()->GetBindings();
            rBindings.Update(SID_RULER_BORDERS);
            rBindings.Update(SID_ATTR_TABSTOP);
        }
        break;
        case FN_NUM_FORMAT_TABLE_DLG:
        {
            SwView* pView = GetActiveView();
            if(pView)
            {
                FieldUnit eMetric = ::GetDfltMetric(0 != PTR_CAST(SwWebView, pView));
                SW_MOD()->PutItem(SfxUInt16Item(SID_ATTR_METRIC, eMetric));
                SvNumberFormatter* pFormatter = rSh.GetNumberFormatter();
                SfxItemSet aCoreSet( GetPool(),
                                 SID_ATTR_NUMBERFORMAT_VALUE, SID_ATTR_NUMBERFORMAT_VALUE,
                                 SID_ATTR_NUMBERFORMAT_INFO, SID_ATTR_NUMBERFORMAT_INFO,
                                 0 );

                SfxItemSet aBoxSet( *aCoreSet.GetPool(),
                                    RES_BOXATR_FORMAT, RES_BOXATR_FORMAT,
                                    RES_BOXATR_VALUE, RES_BOXATR_VALUE,
                                    0 );
                rSh.GetTblBoxFormulaAttrs( aBoxSet );

                SfxItemState eState = aBoxSet.GetItemState(RES_BOXATR_FORMAT);
                if(eState == SFX_ITEM_DEFAULT)
                {
                    aCoreSet.Put( SfxUInt32Item( SID_ATTR_NUMBERFORMAT_VALUE,
                    pFormatter->GetFormatIndex(NF_TEXT, LANGUAGE_SYSTEM)));
                }
                else
                    aCoreSet.Put( SfxUInt32Item( SID_ATTR_NUMBERFORMAT_VALUE,
                                    ((SwTblBoxNumFormat&)aBoxSet.Get(
                                    RES_BOXATR_FORMAT )).GetValue() ));

                aCoreSet.Put( SvxNumberInfoItem( pFormatter,
                                    ((SwTblBoxValue&)aBoxSet.Get(
                                        RES_BOXATR_VALUE)).GetValue(),
                                    SID_ATTR_NUMBERFORMAT_INFO ));

                SwNumFmtDlg* pDlg = new SwNumFmtDlg(GetView().GetWindow(), aCoreSet);

                if (RET_OK == pDlg->Execute())
                {
                    const SfxPoolItem* pItem = GetView().GetDocShell()->
                                    GetItem( SID_ATTR_NUMBERFORMAT_INFO );

                    if( pItem && 0 != ((SvxNumberInfoItem*)pItem)->GetDelCount() )
                    {
                        const ULONG* pDelArr = ((SvxNumberInfoItem*)
                                                        pItem)->GetDelArray();

                        for ( USHORT i = 0; i < ((SvxNumberInfoItem*)pItem)->GetDelCount(); i++ )
                            ((SvxNumberInfoItem*)pItem)->
                            GetNumberFormatter()->DeleteEntry( pDelArr[i] );
                    }

                    if( SFX_ITEM_SET == pDlg->GetOutputItemSet()->GetItemState(
                        SID_ATTR_NUMBERFORMAT_VALUE, FALSE, &pItem ))
                    {
                        SfxItemSet aBoxSet( *aCoreSet.GetPool(),
                                    RES_BOXATR_FORMAT, RES_BOXATR_FORMAT );
                        aBoxSet.Put( SwTblBoxNumFormat(
                                ((SfxUInt32Item*)pItem)->GetValue() ));
                        rSh.SetTblBoxFormulaAttrs( aBoxSet );

                    }
                }
                delete pDlg;
            }
        }
        break;
        case FN_CALC_TABLE:
            rSh.UpdateTable();
        break;
        case FN_TABLE_OPTIMAL_HEIGHT:
        {
            const SwFmtFrmSize aSz;
            rSh.SetRowHeight( aSz );
        }
        break;
        case FN_TABLE_DELETE_COL:
            if ( rSh.DeleteCol() && rSh.HasSelection() )
                rSh.EnterStdMode();
        break;
        case FN_END_TABLE:
            rSh.MoveTable( fnTableCurr, fnTableEnd );
        break;
        case FN_START_TABLE:
            rSh.MoveTable( fnTableCurr, fnTableStart );
        break;

        case FN_TABLE_DELETE_ROW:
            if ( rSh.DeleteRow() && rSh.HasSelection() )
                rSh.EnterStdMode();
        break;
        case FN_TABLE_MERGE_CELLS:
            if ( rSh.IsTableMode() )
                switch ( rSh.MergeTab() )
                {
                    case TBLMERGE_OK:
                    case TBLMERGE_NOSELECTION:  break;
                    case TBLMERGE_TOOCOMPLEX:
                    {
                        InfoBox aInfoBox( GetView().GetWindow(),
                                    SW_RES( MSG_ERR_TABLE_MERGE ) );
                        aInfoBox.Execute();
                        break;
                    }
                    default: ASSERT( !this, "unbekannter Returnwert MergeTab.");
                }
        break;
        case FN_TABLE_ADJUST_CELLS:
            if ( rSh.IsAdjustCellWidthAllowed() )
                rSh.AdjustCellWidth();
        break;
        case FN_TABLE_BALANCE_CELLS:
            if ( rSh.IsAdjustCellWidthAllowed(TRUE) )
                rSh.AdjustCellWidth(TRUE);
        break;
        case FN_TABLE_BALANCE_ROWS:
            if ( rSh.BalanceRowHeight(TRUE) )
                rSh.BalanceRowHeight(FALSE);
        break;
        case FN_TABLE_SELECT_ALL:
            rSh.EnterStdMode();
            rSh.MoveTable( fnTableCurr, fnTableStart );
            rSh.SttSelect();
            rSh.MoveTable( fnTableCurr, fnTableEnd );
            rSh.EndSelect();
        break;
        case FN_TABLE_SELECT_COL:
            rSh.EnterStdMode();
            rSh.SelectTableCol();
        break;
        case FN_TABLE_SELECT_ROW:
            rSh.EnterStdMode();
            rSh.SelectTableRow();
        break;
        case FN_TABLE_SET_READ_ONLY_CELLS:
            rSh.ProtectCells();
            rSh.ResetSelect( 0, FALSE );
        break;
        case FN_TABLE_UNSET_READ_ONLY_CELLS:
            rSh.UnProtectCells();
        break;
        case SID_AUTOFORMAT:
        {
            SwAutoFormatDlg* pDlg = new SwAutoFormatDlg(
                            &GetView().GetViewFrame()->GetWindow(), &rSh );
            pDlg->Execute();
            delete pDlg;
        }
        break;
        case FN_TABLE_SET_ROW_HEIGHT:
        {
            SwTableHeightDlg *pDlg = new SwTableHeightDlg(
                                    GetView().GetWindow(), rSh );
            pDlg->Execute();
            delete pDlg;
        }
        break;
        case FN_NUMBER_BULLETS:
        case FN_NUM_BULLET_ON:
            ASSERT( !this, "Funktion darf zur Zeit nicht gerufen werden." );
        break;

        case FN_TABLE_INSERT_COL:
        case FN_TABLE_INSERT_ROW:
        {
            BOOL bColumn = rReq.GetSlot() == FN_TABLE_INSERT_COL;
            USHORT nCount = 0;
            BOOL bAfter = TRUE;
            if (pItem)
            {
                nCount = ((const SfxInt16Item* )pItem)->GetValue();
                if(SFX_ITEM_SET == pArgs->GetItemState(FN_PARAM_INSERT_AFTER, TRUE, &pItem))
                    bAfter = ((const SfxBoolItem* )pItem)->GetValue();
            }
            else if( !rReq.IsAPI() )
                ++nCount;

            if( nCount )
            {
                if( bColumn )
                    rSh.InsertCol( nCount, bAfter );
                else if ( !rSh.IsInRepeatedHeadline() )
                    rSh.InsertRow( nCount, bAfter );
                break;
            }

            nSlot = bColumn ? FN_TABLE_INSERT_COL_DLG : FN_TABLE_INSERT_ROW_DLG;
        }
        // kein break;  bei Count = 0 kommt der Dialog
        case FN_TABLE_INSERT_COL_DLG:
        case FN_TABLE_INSERT_ROW_DLG:
        {
            if ( FN_TABLE_INSERT_ROW_DLG != nSlot || !rSh.IsInRepeatedHeadline())
            {
                SwInsRowColDlg *pDlg = new SwInsRowColDlg( GetView(),
                                        FN_TABLE_INSERT_COL_DLG == nSlot );
                pDlg->Execute();
                delete pDlg;
            }
        }
        break;
        case FN_TABLE_SPLIT_CELLS:
        {
            SwSplitTableDlg *pDlg = new SwSplitTableDlg( GetView().GetWindow(), rSh );
            pDlg->Execute();
            delete pDlg;
        }
        break;

        case FN_TABLE_SPLIT_TABLE:
        {
            SwSplitTblDlg *pDlg = new SwSplitTblDlg( GetView().GetWindow(), rSh );
            pDlg->Execute();
            delete pDlg;
        }
        break;

        case FN_TABLE_MERGE_TABLE:
        {
            BOOL bPrev = rSh.CanMergeTable( TRUE );
            BOOL bNext = rSh.CanMergeTable( FALSE );

            if( bPrev && bNext )
            {
                // Dialog: welche denn?
                SwMergeTblDlg* pDlg = new SwMergeTblDlg(
                    GetView().GetWindow(), bPrev );
                if( RET_OK != pDlg->Execute() )
                    bPrev = bNext = FALSE;
                delete pDlg;
            }

            if( bPrev || bNext )
                rSh.MergeTable( bPrev );
        }
        break;

        case FN_TABLE_MODE_FIX       :
        case FN_TABLE_MODE_FIX_PROP  :
        case FN_TABLE_MODE_VARIABLE  :
        {
            rSh.SetTblChgMode( ( FN_TABLE_MODE_FIX == nSlot ? TBLFIX_CHGABS
                                    : FN_TABLE_MODE_FIX_PROP == nSlot
                                        ? TBLFIX_CHGPROP
                                        : TBLVAR_CHGABS ) );

            SfxBindings& rBind = GetView().GetViewFrame()->GetBindings();
            static USHORT __READONLY_DATA aInva[] =
                            {   FN_TABLE_MODE_FIX,
                                FN_TABLE_MODE_FIX_PROP,
                                FN_TABLE_MODE_VARIABLE,
                                0
                            };
            rBind.Invalidate( aInva );
        }
        break;
        case FN_TABLE_AUTOSUM:
        {
            SfxViewFrame* pVFrame = GetView().GetViewFrame();
            pVFrame->GetDispatcher()->Execute(FN_EDIT_FORMULA, SFX_CALLMODE_SYNCHRON);
            const USHORT nId = SwInputChild::GetChildWindowId();
            SwInputChild* pChildWin = (SwInputChild*)pVFrame->
                                                GetChildWindow( nId );
            String sSum;
            GetShell().GetAutoSum(sSum);
            if( pChildWin )
                pChildWin->SetFormula( sSum );

        }
        break;
        default:
            bMore = TRUE;
    }

    if ( !bMore )
    {
        return;
    }
    else
        bMore = FALSE;
    //Jetzt die Slots, die direkt auf dem TableFmt arbeiten.
    SwFrmFmt *pFmt = rSh.GetTableFmt();
    switch ( nSlot )
    {
        case SID_ATTR_ULSPACE:
        {
            if(pItem)
            {
                SvxULSpaceItem aULSpace( *(const SvxULSpaceItem*)pItem );
                aULSpace.SetWhich( RES_UL_SPACE );
                ::lcl_SetAttr( rSh, aULSpace );
            }
        }
            break;

        case SID_ATTR_LRSPACE:
        {
            if(pItem)
            {
                SfxItemSet aSet( GetPool(), RES_LR_SPACE, RES_LR_SPACE,
                                            RES_HORI_ORIENT, RES_HORI_ORIENT, 0 );
                SvxLRSpaceItem aLRSpace( *(const SvxLRSpaceItem*)pItem );
                aLRSpace.SetWhich( RES_LR_SPACE );
                aSet.Put( aLRSpace );
                SwFmtHoriOrient aHori( pFmt->GetHoriOrient() );
                aHori.SetHoriOrient( HORI_NONE );
                aSet.Put( aLRSpace );
                rSh.SetTblAttr( aSet );
            }
        }
        break;
// der letzte case-Zweig der noch einen TabellenManager braucht!!
        case FN_TABLE_SET_COL_WIDTH:
        {
            SwTableFUNC aMgr( &rSh, FALSE);
            aMgr.ColWidthDlg(GetView().GetWindow());
        }
        break;
        case FN_TABLE_VERT_NONE:
        case FN_TABLE_VERT_CENTER:
        case FN_TABLE_VERT_BOTTOM:
        {
            USHORT nAlign = nSlot == FN_TABLE_VERT_NONE ?
                                VERT_NONE :
                                    nSlot == FN_TABLE_VERT_CENTER ?
                                        VERT_CENTER : VERT_BOTTOM;
            rSh.SetBoxAlign(nAlign);
        }
        break;

        case SID_ATTR_PARA_SPLIT:
            if ( pItem )
            {
                SwFmtLayoutSplit aSplit( ((const SvxFmtSplitItem*)pItem)->GetValue());
                SfxItemSet aSet(GetPool(),  RES_LAYOUT_SPLIT, RES_LAYOUT_SPLIT, 0 );
                aSet.Put(aSplit);
                rSh.SetTblAttr(aSet);
            }
        break;

        case SID_ATTR_PARA_KEEP:
            if ( pItem )
            {
                SvxFmtKeepItem aKeep( *(const SvxFmtKeepItem*)pItem );
                aKeep.SetWhich( RES_KEEP );
                SfxItemSet aSet(GetPool(),  RES_KEEP, RES_KEEP, 0 );
                aSet.Put(aKeep);
                rSh.SetTblAttr(aSet);
            }
        break;

        default:
            ASSERT( !this, "falscher Dispatcher" );
            return;
    }
}
/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwTableShell::GetState(SfxItemSet &rSet)
{
    SfxWhichIter aIter( rSet );
    SwWrtShell &rSh = GetShell();
    SwFrmFmt *pFmt = rSh.GetTableFmt();
    USHORT nSlot = aIter.FirstWhich();
    while ( nSlot )
    {
        switch ( nSlot )
        {
            case FN_TABLE_MERGE_CELLS:
                if ( !rSh.IsTableMode() )
                    rSet.DisableItem(FN_TABLE_MERGE_CELLS);
            break;
            case FN_TABLE_ADJUST_CELLS:
                if ( !rSh.IsAdjustCellWidthAllowed() )
                    rSet.DisableItem(FN_TABLE_ADJUST_CELLS);
            break;

            case FN_TABLE_BALANCE_CELLS:
                if ( !rSh.IsAdjustCellWidthAllowed(TRUE) )
                    rSet.DisableItem(FN_TABLE_BALANCE_CELLS);
            break;

            case FN_TABLE_BALANCE_ROWS:
                if ( !rSh.BalanceRowHeight(TRUE) )
                    rSet.DisableItem(FN_TABLE_BALANCE_ROWS);
            break;

            case SID_INSERT_DIAGRAM:
                {
                    SvtModuleOptions aMOpt;
                    if ( !aMOpt.IsMath() || rSh.IsTblComplexForChart() )
                        rSet.DisableItem(nSlot);
                }
                break;

            case FN_INSERT_TABLE:
                // Irgendeinen Wert "putten", damit Controller enabled bleibt.
                // Statt "Insert:Table" erscheint dann "Format:Table".
                rSet.Put(SfxUInt16Item(nSlot, 1));
                break;

            case FN_TABLE_OPTIMAL_HEIGHT:
            {
                //Disablen wenn bereits auto-Hoehe eingestellt ist.
                SwFmtFrmSize *pSz;
                rSh.GetRowHeight( pSz );
                if ( pSz )
                {
                    if ( ATT_VAR_SIZE == pSz->GetSizeType() )
                        rSet.DisableItem( nSlot );
                    delete pSz;
                }
            }
            break;
            case FN_TABLE_INSERT_ROW:
            case FN_TABLE_INSERT_ROW_DLG:
            {
                if ( rSh.IsInRepeatedHeadline() )
                    rSet.DisableItem( nSlot );
            }
            break;
            case RES_LR_SPACE:
                rSet.Put(pFmt->GetLRSpace());
            break;
            case RES_UL_SPACE:
                rSet.Put(pFmt->GetULSpace());
            break;
            case FN_TABLE_VERT_NONE:
            case FN_TABLE_VERT_CENTER:
            case FN_TABLE_VERT_BOTTOM:
            {
                USHORT nAlign = rSh.GetBoxAlign();
                BOOL bSet = nSlot == FN_TABLE_VERT_NONE && nAlign == VERT_NONE||
                            nSlot == FN_TABLE_VERT_CENTER && nAlign == VERT_CENTER ||
                            nSlot == FN_TABLE_VERT_BOTTOM && nAlign == VERT_BOTTOM;
                rSet.Put(SfxBoolItem(nSlot, bSet));
            }
            break;

            case FN_TABLE_MODE_FIX       :
            case FN_TABLE_MODE_FIX_PROP  :
            case FN_TABLE_MODE_VARIABLE  :
                {
                    USHORT nMode = rSh.GetTblChgMode();
                    BOOL bSet = nSlot == FN_TABLE_MODE_FIX && nMode == TBLFIX_CHGABS ||
                            nSlot == FN_TABLE_MODE_FIX_PROP && nMode == TBLFIX_CHGPROP ||
                            nSlot == FN_TABLE_MODE_VARIABLE && nMode == TBLVAR_CHGABS;
                    rSet.Put(SfxBoolItem(nSlot, bSet));
                }
            break;

            case SID_ATTR_PARA_SPLIT:
                rSet.Put( pFmt->GetKeep() );
            break;

            case SID_ATTR_PARA_KEEP:
                rSet.Put( pFmt->GetLayoutSplit() );
            break;
            case FN_TABLE_SPLIT_TABLE:
            {
                if ( rSh.IsInHeadline() )
                    rSet.DisableItem( nSlot );
            }
            break;
            case FN_TABLE_MERGE_TABLE:
            {
                BOOL bAsk;
                if( !rSh.CanMergeTable( TRUE, &bAsk ))
                    rSet.DisableItem( nSlot );
            }
            break;

        case FN_TABLE_DELETE_ROW:
            {
                SwSelBoxes aBoxes;
                ::GetTblSel( rSh, aBoxes, TBLSEARCH_ROW );
                if( ::HasProtectedCells( aBoxes ))
                    rSet.DisableItem( nSlot );
            }
            break;
        case FN_TABLE_DELETE_COL:
            {
                SwSelBoxes aBoxes;
                ::GetTblSel( rSh, aBoxes, TBLSEARCH_COL );
                if( ::HasProtectedCells( aBoxes ))
                    rSet.DisableItem( nSlot );
            }
            break;
        }
    nSlot = aIter.NextWhich();
    }
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SwTableShell::SwTableShell(SwView &rView) :
    SwBaseShell(rView)
{
    SetName(String::CreateFromAscii("Table"));
    SetHelpId(SW_TABSHELL);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwTableShell::GetFrmBorderState(SfxItemSet &rSet)
{
    SfxItemSet aCoreSet( GetPool(),
                         RES_BOX, RES_BOX,
                         SID_ATTR_BORDER_INNER, SID_ATTR_BORDER_INNER, 0 );
    SvxBoxInfoItem aBoxInfo;
    aCoreSet.Put( aBoxInfo );
    GetShell().GetTabBorders( aCoreSet );
    rSet.Put( aCoreSet );
    rSet.Put(SfxUInt16Item(SID_ATTR_BORDER, FALSE));    // Rahmencontroller: letzte Zeile ausklappen
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwTableShell::ExecTableStyle(SfxRequest& rReq)
{
    SwWrtShell &rSh = GetShell();
    const SfxItemSet *pArgs = rReq.GetArgs();
    if(pArgs)
        switch ( rReq.GetSlot() )
        {
            case SID_FRAME_LINESTYLE:
            case SID_FRAME_LINECOLOR:
            {
/*  Tabellenselektion ist abgeschafft
                BOOL bPopCrsr = FALSE;
                if ( !rReq.IsAPI() )
                {
                    //Keine Tabellenselektion -> Aenderung wird auf die gesamte
                    //Tabelle.
                    if ( !rSh.IsTableMode() )
                    {
                        bPopCrsr = TRUE;
                        rSh.Push();
                        rSh.StartAction();
                        rSh.EnterStdMode();
                        rSh.MoveTable( fnTableCurr, fnTableStart );
                        rSh.SttSelect();
                        rSh.MoveTable( fnTableCurr, fnTableEnd );
                        rSh.EndSelect();
                    }
                }*/
                if ( rReq.GetSlot() == SID_FRAME_LINESTYLE )
                {
                    const SvxLineItem &rLineItem = (const SvxLineItem&)pArgs->
                                                            Get( SID_FRAME_LINESTYLE );
                    const SvxBorderLine* pBorderLine = rLineItem.GetLine();
                    rSh.SetTabLineStyle( 0, TRUE, pBorderLine);
                }
                else
                {
                    const SvxColorItem &rNewColorItem = (const SvxColorItem&)pArgs->
                                                            Get( SID_FRAME_LINECOLOR );
                    rSh.SetTabLineStyle( &rNewColorItem.GetValue() );
                }
/*              if ( bPopCrsr )
                {
                    rSh.KillPams();
                    rSh.Pop(FALSE);
                    rSh.EndAction();
                }*/
                break;
            }
        }
}


void SwTableShell::GetLineStyleState(SfxItemSet &rSet)
{
    SfxItemSet aCoreSet( GetPool(),
                            RES_BOX, RES_BOX,
                            SID_ATTR_BORDER_INNER, SID_ATTR_BORDER_INNER, 0);
    SvxBoxInfoItem aCoreInfo;
    aCoreSet.Put(aCoreInfo);
    GetShell().GetTabBorders( aCoreSet );

    const SvxBoxItem& rBoxItem = (const SvxBoxItem&)aCoreSet.Get( RES_BOX );
    const SvxBorderLine* pLine = rBoxItem.GetTop();

    rSet.Put( SvxColorItem( pLine ? pLine->GetColor() : Color(), SID_FRAME_LINECOLOR ) );
    SvxLineItem aLine( SID_FRAME_LINESTYLE );
    aLine.SetLine(pLine);
    rSet.Put( aLine );
}

void SwTableShell::ExecNumberFormat(SfxRequest& rReq)
{
    const SfxItemSet* pArgs = rReq.GetArgs();
    SwWrtShell &rSh = GetShell();

    //Erstmal die Slots, die keinen FrmMgr benoetigen.
    const SfxPoolItem* pItem = 0;
    USHORT nSlot = rReq.GetSlot();
    if(pArgs)
        pArgs->GetItemState(GetPool().GetWhich(nSlot), FALSE, &pItem);

    //  Sprache immer von Cursorposition besorgen
    LanguageType eLang = rSh.GetCurLang();
    SvNumberFormatter* pFormatter = rSh.GetNumberFormatter();
    ULONG nNumberFormat = NUMBERFORMAT_ENTRY_NOT_FOUND;
    USHORT nFmtType = 0, nOffset = 0;

    switch ( nSlot )
    {
    case FN_NUMBER_FORMAT:
        if( pItem )
        {
            //  Index fuer String bestimmen
            String aCode( ((const SfxStringItem*)pItem)->GetValue() );
            nNumberFormat = pFormatter->GetEntryKey( aCode, eLang );
            if( NUMBERFORMAT_ENTRY_NOT_FOUND == nNumberFormat )
            {
                //  neu eintragen
                xub_StrLen nErrPos;
                short nType;
                if( !pFormatter->PutEntry( aCode, nErrPos, nType,
                                            nNumberFormat, eLang ))
                    nNumberFormat = NUMBERFORMAT_ENTRY_NOT_FOUND;
            }
        }
        break;
    case FN_NUMBER_STANDARD:        nFmtType = NUMBERFORMAT_NUMBER; break;
    case FN_NUMBER_SCIENTIFIC:      nFmtType = NUMBERFORMAT_SCIENTIFIC; break;
    case FN_NUMBER_DATE:            nFmtType = NUMBERFORMAT_DATE; break;
    case FN_NUMBER_TIME:            nFmtType = NUMBERFORMAT_TIME; break;
    case FN_NUMBER_CURRENCY:        nFmtType = NUMBERFORMAT_CURRENCY; break;
    case FN_NUMBER_PERCENT:         nFmtType = NUMBERFORMAT_PERCENT; break;

    case FN_NUMBER_TWODEC:          // #.##0,00
        nFmtType = NUMBERFORMAT_NUMBER;
        nOffset = NF_NUMBER_1000DEC2;
        break;

    default:
        ASSERT(FALSE, falscher Dispatcher);
        return;
    }

    if( nFmtType )
        nNumberFormat = pFormatter->GetStandardFormat( nFmtType, eLang ) + nOffset;

    if( NUMBERFORMAT_ENTRY_NOT_FOUND != nNumberFormat )
    {
        SfxItemSet aBoxSet( GetPool(), RES_BOXATR_FORMAT, RES_BOXATR_FORMAT );
        aBoxSet.Put( SwTblBoxNumFormat( nNumberFormat ));
        rSh.SetTblBoxFormulaAttrs( aBoxSet );
    }

}
/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.3  2000/11/14 18:32:27  jp
    use moduleoptions

    Revision 1.2  2000/10/06 13:36:37  jp
    should changes: don't use IniManager

    Revision 1.1.1.1  2000/09/18 17:14:47  hr
    initial import

    Revision 1.246  2000/09/18 16:06:05  willem.vandorp
    OpenOffice header added.

    Revision 1.245  2000/09/08 08:12:52  os
    Change: Set/Toggle/Has/Knows/Show/GetChildWindow

    Revision 1.244  2000/09/07 15:59:30  os
    change: SFX_DISPATCHER/SFX_BINDINGS removed

    Revision 1.243  2000/08/25 09:02:38  os
    negative margins enabled

    Revision 1.242  2000/07/03 20:02:16  jp
    Bug #70403#: dont delete protected cells

    Revision 1.241  2000/05/26 07:21:32  os
    old SW Basic API Slots removed

    Revision 1.240  2000/05/18 08:37:25  os
    lcl_IsNumeric

    Revision 1.239  2000/05/16 14:32:51  jp
    Changes for Unicode

    Revision 1.238  2000/05/10 11:53:02  os
    Basic API removed

    Revision 1.237  2000/04/20 12:51:52  os
    GetName() returns String&

    Revision 1.236  2000/04/18 14:58:24  os
    UNICODE

    Revision 1.235  2000/02/11 14:57:39  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.234  2000/02/03 15:10:42  os
    #72601# record deleting of rows or column prior to execution

    Revision 1.233  1999/08/23 07:49:02  OS
    #61218# correct handling of left_and_width oriented tables


      Rev 1.232   23 Aug 1999 09:49:02   OS
   #61218# correct handling of left_and_width oriented tables

      Rev 1.231   21 Jul 1999 13:29:58   JP
   WhichId-Ranges of SfxItemSets: EndId is inclusive

      Rev 1.230   09 Jun 1999 19:33:58   JP
   have to change: no cast from GetpApp to SfxApp/OffApp, SfxShell only subclass of SfxApp

      Rev 1.229   11 May 1999 21:51:40   JP
   Task #66127#: ueberfluessigen Code entfernt

      Rev 1.228   05 May 1999 11:23:18   OS
   #63790# Tabellen-Update nur bei Bedarf rufen

      Rev 1.227   28 Apr 1999 22:41:34   JP
   Bug #65545#: lcl_TableParamToItemSet - umgebene spaltige Bereiche beachten

      Rev 1.226   18 Mar 1999 14:40:48   OS
   #61169# #61489# Masseinheiten fuer Text u. HTML am Module setzen, nicht an der App

      Rev 1.225   22 Feb 1999 11:50:32   JP
   fuer Bug #61545#: Code optimiert

      Rev 1.224   17 Feb 1999 21:14:20   JP
   Task #61764#: neu: Tabellen zusammenfassen

      Rev 1.223   27 Nov 1998 14:52:54   AMA
   Fix #59951#59825#: Unterscheiden zwischen Rahmen-,Seiten- und Bereichsspalten

      Rev 1.222   05 Oct 1998 15:36:02   OM
   Vernuenftiges Standardformat verwenden

      Rev 1.221   15 Sep 1998 11:20:36   JP
   Bug #56201#: GoNextCell - per Flag das anhaengen von Lines steuern

      Rev 1.220   08 Sep 1998 17:03:00   OS
   #56134# Metric fuer Text und HTML getrennt

      Rev 1.219   30 Jul 1998 10:58:18   OS
   Tabellenbreit ergibt sich fuer HORI-NONE aus verfuegbarer Breite - Raender #54030#

      Rev 1.218   20 Jul 1998 10:44:52   MA
   #53013# Nach Anweisung abgeklemmt

      Rev 1.217   26 Jun 1998 17:36:06   OS
   SwBGDestItem mit Which #51751#

      Rev 1.216   18 Jun 1998 18:18:46   OS
   SwBackgroundDestinationItem

      Rev 1.215   02 Jun 1998 10:53:16   JP
   aUITableAttrRange sollte ein const Array sein

      Rev 1.214   29 May 1998 19:08:26   JP
   SS vom SwTableReq geaendert

      Rev 1.213   12 May 1998 23:44:08   JP
   neu: InserTable/TextToTable mit optionalen AutoFormat

      Rev 1.212   28 Apr 1998 09:16:18   OS
   GetTableWidth() verschoben

      Rev 1.211   18 Mar 1998 19:44:20   MA
   State fuer Split

      Rev 1.210   17 Mar 1998 16:20:42   OS
   Tabelle auftrennen

      Rev 1.209   17 Mar 1998 14:12:06   OS
   Border-Slot auch ohne Argumente ueberleben #48169#

      Rev 1.208   16 Mar 1998 23:21:04   JP
   SplitTable: zusaetzlich einen Modus angeben - Headline kopieren/Attr. Kopieren/Border korrigieren

      Rev 1.207   02 Mar 1998 14:19:34   OS
   Zahlenformat statt Default auf Text einstellen#46574#

      Rev 1.206   25 Feb 1998 12:53:52   MA
   new: SplitTable

      Rev 1.205   29 Nov 1997 15:53:00   MA
   includes

      Rev 1.204   24 Nov 1997 09:47:06   MA
   includes

      Rev 1.203   11 Nov 1997 10:49:08   OS
   Parameter fuer Split und ColumnWidth richtig pruefen

      Rev 1.202   03 Nov 1997 13:55:48   MA
   precomp entfernt

      Rev 1.201   01 Sep 1997 13:23:08   OS
   DLL-Umstellung

      Rev 1.200   15 Aug 1997 11:48:50   OS
   chartar/frmatr/txtatr aufgeteilt

      Rev 1.199   12 Aug 1997 14:43:10   MH
   chg: header

      Rev 1.198   11 Aug 1997 15:42:10   JP
   Bugfix #42478#: ueber die DocShell die geloeschten Formate besorgen

      Rev 1.198   11 Aug 1997 15:41:44   JP
   Bugfix #42478#: ueber die DocShell die geloeschten Formate besorgen

      Rev 1.197   11 Aug 1997 08:48:54   OS
   paraitem/frmitems/textitem aufgeteilt

      Rev 1.196   08 Aug 1997 17:28:52   OM
   Headerfile-Umstellung


------------------------------------------------------------------------*/


