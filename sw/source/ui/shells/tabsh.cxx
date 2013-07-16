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

#include <hintids.hxx>
#include <svl/zforlist.hxx>
#include <svl/stritem.hxx>
#include <svl/whiter.hxx>
#include <unotools/moduleoptions.hxx>
#include <svx/rulritem.hxx>
#include <svl/srchitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/spltitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/keepitem.hxx>
#include <editeng/lineitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/frmdiritem.hxx>
#include <svx/numinf.hxx>
#include <svx/svddef.hxx>
#include <svx/svxdlg.hxx>
#include <svl/zformat.hxx>
#include <sfx2/bindings.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/request.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/sidebar/EnumContext.hxx>

#include <fmtornt.hxx>
#include <fmtclds.hxx>
#include <fmtlsplt.hxx>
#include <fmtrowsplt.hxx>
#include <fmtfsize.hxx>
#include <swmodule.hxx>
#include <wrtsh.hxx>
#include <wview.hxx>
#include <frmatr.hxx>
#include <uitool.hxx>
#include <inputwin.hxx>
#include <uiitems.hxx>
#include <usrpref.hxx>
#include <tabsh.hxx>
#include "swtablerep.hxx"
#include <tablemgr.hxx>
#include <cellatr.hxx>
#include <frmfmt.hxx>
#include <swundo.hxx>
#include <swtable.hxx>
#include <docsh.hxx>
#include <tblsel.hxx>

#include <dialog.hrc>
#include <popup.hrc>
#include <shells.hrc>
#include <cmdid.h>
#include <globals.hrc>
#include <helpid.h>
#include <unobaseclass.hxx>

#define SwTableShell
#include <sfx2/msg.hxx>
#include <swslots.hxx>

#include "swabstdlg.hxx"
#include <table.hrc>

using ::editeng::SvxBorderLine;
using namespace ::com::sun::star;

SFX_IMPL_INTERFACE(SwTableShell, SwBaseShell, SW_RES(STR_SHELLNAME_TABLE))
{
    SFX_POPUPMENU_REGISTRATION(SW_RES(MN_TAB_POPUPMENU));
    SFX_OBJECTBAR_REGISTRATION(SFX_OBJECTBAR_OBJECT, SW_RES(RID_TABLE_TOOLBOX));
}

TYPEINIT1(SwTableShell,SwBaseShell)

const sal_uInt16 aUITableAttrRange[] =
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
    RES_FRAMEDIR,                   RES_FRAMEDIR,
    RES_ROW_SPLIT,                  RES_ROW_SPLIT,
    FN_TABLE_BOX_TEXTORIENTATION,   FN_TABLE_BOX_TEXTORIENTATION,
// #i29550#
    RES_COLLAPSING_BORDERS,         RES_COLLAPSING_BORDERS,
// <-- collapsing borders
    0
};

const sal_uInt16* SwuiGetUITableAttrRange()
{
    return aUITableAttrRange;
}

static void lcl_SetAttr( SwWrtShell &rSh, const SfxPoolItem &rItem )
{
    SfxItemSet aSet( rSh.GetView().GetPool(), rItem.Which(), rItem.Which(), 0);
    aSet.Put( rItem );
    rSh.SetTblAttr( aSet );
}

static SwTableRep*  lcl_TableParamToItemSet( SfxItemSet& rSet, SwWrtShell &rSh )
{
    SwFrmFmt *pFmt = rSh.GetTableFmt();
    SwTabCols aCols;
    rSh.GetTabCols( aCols );

    //At first get the simple attributes.
    rSet.Put( SfxStringItem( FN_PARAM_TABLE_NAME, pFmt->GetName()));
    rSet.Put( SfxUInt16Item( FN_PARAM_TABLE_HEADLINE, rSh.GetRowsToRepeat() ) );
    rSet.Put( pFmt->GetShadow() );
    rSet.Put(SfxUInt16Item(FN_TABLE_SET_VERT_ALIGN, rSh.GetBoxAlign()));
    rSet.Put( pFmt->GetFrmDir() );

    SvxULSpaceItem aULSpace( pFmt->GetULSpace() );
    rSet.Put( aULSpace );

    sal_uInt16  nBackgroundDestination = rSh.GetViewOptions()->GetTblDest();
    rSet.Put(SwBackgroundDestinationItem(SID_BACKGRND_DESTINATION, nBackgroundDestination ));
    SvxBrushItem aBrush( RES_BACKGROUND );
    if(rSh.GetRowBackground(aBrush))
        rSet.Put( aBrush, SID_ATTR_BRUSH_ROW );
    else
        rSet.InvalidateItem(SID_ATTR_BRUSH_ROW);
    rSh.GetTabBackground(aBrush);
    rSet.Put( aBrush, SID_ATTR_BRUSH_TABLE );

    // text direction in boxes
    SvxFrameDirectionItem aBoxDirection( FRMDIR_ENVIRONMENT, RES_FRAMEDIR );
    if(rSh.GetBoxDirection( aBoxDirection ))
        rSet.Put(aBoxDirection, FN_TABLE_BOX_TEXTORIENTATION);

    sal_Bool bTableSel = rSh.IsTableMode();
    if(!bTableSel)
    {
        rSh.StartAllAction();
        rSh.Push();
        rSh.GetView().GetViewFrame()->GetDispatcher()->Execute( FN_TABLE_SELECT_ALL, sal_False );
    }
    SvxBoxInfoItem aBoxInfo( SID_ATTR_BORDER_INNER );

        // Table variant: If multiple table cells are selected.
    rSh.GetCrsr();                  //Thus GetCrsrCnt() returns the right thing
    aBoxInfo.SetTable          ((rSh.IsTableMode() && rSh.GetCrsrCnt() > 1) ||
                                    !bTableSel);
        // Always show distance field.
    aBoxInfo.SetDist           ((sal_Bool) sal_True);
        // Set minimum size in tables and paragraphs.
    aBoxInfo.SetMinDist( !bTableSel || rSh.IsTableMode() ||
                            rSh.GetSelectionType() &
                            (nsSelectionType::SEL_TXT | nsSelectionType::SEL_TBL));
        // Always set the default spacing.
    aBoxInfo.SetDefDist        (MIN_BORDER_DIST);
        // Individual lines can have DontCare status only in tables.
    aBoxInfo.SetValid( VALID_DISABLE, !bTableSel || !rSh.IsTableMode() );

    rSet.Put(aBoxInfo);
    rSh.GetTabBorders( rSet );

    //row split
    SwFmtRowSplit* pSplit = 0;
    rSh.GetRowSplit(pSplit);
    if(pSplit)
    {
        rSet.Put(*pSplit);
        delete pSplit;
    }

    if(!bTableSel)
    {
        rSh.ClearMark();
        rSh.Pop(sal_False);
        rSh.EndAllAction();
    }

    SwTabCols aTabCols;
    rSh.GetTabCols( aTabCols );
    SvxColumnItem aColItem;

    // Pointer will be deleted after the dialogue execution.
    SwTableRep* pRep = new SwTableRep( aTabCols );
    pRep->SetSpace(aCols.GetRightMax());

    sal_uInt16 nPercent = 0;
    long nWidth = ::GetTableWidth(pFmt, aCols, &nPercent, &rSh );
    // The table width is wrong for relative values.
    if(nPercent)
        nWidth = pRep->GetSpace() * nPercent / 100;
    sal_uInt16 nAlign = pFmt->GetHoriOrient().GetHoriOrient();
    pRep->SetAlign(nAlign);
    SvxLRSpaceItem aLRSpace( pFmt->GetLRSpace() );
    SwTwips nLeft = aLRSpace.GetLeft();
    SwTwips nRight = aLRSpace.GetRight();
    SwTwips nDiff = pRep->GetSpace() - nRight - nLeft - nWidth;
    if(nAlign != text::HoriOrientation::FULL && std::abs(nDiff) > 2)
    {
        SwTwips nLR = pRep->GetSpace() - nWidth;
        switch ( nAlign )
        {
            case text::HoriOrientation::CENTER: nLeft = nRight = nLR / 2;
            break;
            case text::HoriOrientation::LEFT: nRight = nLR; nLeft = 0;
            break;
            case text::HoriOrientation::RIGHT: nLeft = nLR, nRight = 0;
            break;
            case text::HoriOrientation::LEFT_AND_WIDTH:
                nRight = nLR - nLeft;
            break;
            case text::HoriOrientation::NONE:
                if(!nPercent)
                    nWidth = pRep->GetSpace() - nLeft - nRight;
            break;
        }
    }
    pRep->SetLeftSpace(nLeft);
    pRep->SetRightSpace(nRight);

    pRep->SetWidth(nWidth);
    pRep->SetWidthPercent(nPercent);
    // Are individual rows / cells are selected, the column processing will be changed.
    pRep->SetLineSelected(bTableSel && ! rSh.HasWholeTabSelection());
    rSet.Put(SwPtrItem(FN_TABLE_REP, pRep));
    return pRep;
}

void ItemSetToTableParam( const SfxItemSet& rSet,
                                SwWrtShell &rSh )
{
    rSh.StartAllAction();
    rSh.StartUndo( UNDO_TABLE_ATTR );
    const SfxPoolItem* pItem = 0;

    SwViewOption aUsrPref( *rSh.GetViewOptions() );
    sal_uInt16 nBackgroundDestination = aUsrPref.GetTblDest();
    if(SFX_ITEM_SET == rSet.GetItemState(SID_BACKGRND_DESTINATION, sal_False, &pItem))
    {
        nBackgroundDestination = ((SfxUInt16Item*)pItem)->GetValue();
        aUsrPref.SetTblDest((sal_uInt8)nBackgroundDestination);
        SW_MOD()->ApplyUsrPref(aUsrPref, &rSh.GetView());
    }
    bool bBorder = ( SFX_ITEM_SET == rSet.GetItemState( RES_BOX ) ||
            SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_BORDER_INNER ) );
    pItem = 0;
    sal_Bool bBackground = SFX_ITEM_SET == rSet.GetItemState( RES_BACKGROUND, sal_False, &pItem );
    const SfxPoolItem* pRowItem = 0, *pTableItem = 0;
    bBackground |= SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_BRUSH_ROW, sal_False, &pRowItem );
    bBackground |= SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_BRUSH_TABLE, sal_False, &pTableItem );
    const SfxPoolItem* pSplit = 0;
    bool bRowSplit = SFX_ITEM_SET == rSet.GetItemState( RES_ROW_SPLIT, sal_False, &pSplit );
    const SfxPoolItem* pBoxDirection = 0;
    bool bBoxDirection = SFX_ITEM_SET == rSet.GetItemState( FN_TABLE_BOX_TEXTORIENTATION, sal_False, &pBoxDirection );
    if( bBackground || bBorder || bRowSplit || bBoxDirection)
    {
        // The border will be applied to the present selection.
        // If there is no selection, the table will be completely selected.
        // The background will always be applied to the current state.
        sal_Bool bTableSel = rSh.IsTableMode();
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

        if(bBoxDirection)
        {
            SvxFrameDirectionItem aDirection( FRMDIR_ENVIRONMENT, RES_FRAMEDIR );
            aDirection.SetValue(static_cast< const SvxFrameDirectionItem* >(pBoxDirection)->GetValue());
            rSh.SetBoxDirection(aDirection);
        }

        if(bBorder || bRowSplit)
        {
            rSh.Push();
            if(!bTableSel)
            {
                rSh.GetView().GetViewFrame()->GetDispatcher()->Execute( FN_TABLE_SELECT_ALL );
            }
            if(bBorder)
                rSh.SetTabBorders( rSet );

            if(bRowSplit)
            {
                rSh.SetRowSplit(*static_cast<const SwFmtRowSplit*>(pSplit));
            }

            if(!bTableSel)
            {
                rSh.ClearMark();
            }
            rSh.Pop(sal_False);
        }

        rSh.EndAllAction();
    }

    SwTabCols aTabCols;
    bool bTabCols = false;
    sal_Bool bSingleLine = sal_False;
    SwTableRep* pRep = 0;
    SwFrmFmt *pFmt = rSh.GetTableFmt();
    SfxItemSet aSet( rSh.GetAttrPool(), RES_FRMATR_BEGIN, RES_FRMATR_END-1 );
    if(SFX_ITEM_SET == rSet.GetItemState( FN_TABLE_REP, sal_False, &pItem ))
    {
        pRep = (SwTableRep*)((const SwPtrItem*)pItem)->GetValue();

        const SwTwips nWidth = pRep->GetWidth();
        if ( text::HoriOrientation::FULL == pRep->GetAlign() )
        {
            SwFmtHoriOrient aAttr( pFmt->GetHoriOrient() );
            aAttr.SetHoriOrient( text::HoriOrientation::FULL );
            aSet.Put( aAttr );
        }
        else
        {
            SwFmtFrmSize aSz( ATT_VAR_SIZE, nWidth );
            if(pRep->GetWidthPercent())
            {
                aSz.SetWidthPercent( (sal_uInt8)pRep->GetWidthPercent() );
            }
            aSet.Put(aSz);
        }

        SvxLRSpaceItem aLRSpace( RES_LR_SPACE );
        aLRSpace.SetLeft(pRep->GetLeftSpace());
        aLRSpace.SetRight(pRep->GetRightSpace());
        aSet.Put( aLRSpace );

        sal_Int16 eOrient = pRep->GetAlign();
        SwFmtHoriOrient aAttr( 0, eOrient );
        aSet.Put( aAttr );
    // The item must only be recorded while manual alignment, so that the
    // alignment is not overwritten by the distances while recording.
        if(eOrient != text::HoriOrientation::NONE)
            ((SfxItemSet&)rSet).ClearItem( SID_ATTR_LRSPACE );


        if(pRep->HasColsChanged())
        {
            bTabCols = true;
        }
    }


    if( SFX_ITEM_SET == rSet.GetItemState( FN_PARAM_TABLE_HEADLINE, sal_False, &pItem))
        rSh.SetRowsToRepeat( ((SfxUInt16Item*)pItem)->GetValue() );

    if( SFX_ITEM_SET == rSet.GetItemState( FN_TABLE_SET_VERT_ALIGN, sal_False, &pItem))
        rSh.SetBoxAlign(((SfxUInt16Item*)(pItem))->GetValue());

    if( SFX_ITEM_SET == rSet.GetItemState( FN_PARAM_TABLE_NAME, sal_False, &pItem ))
        rSh.SetTableName( *pFmt, ((const SfxStringItem*)pItem)->GetValue() );

    // Copy the chosen attributes in the ItemSet.
    static const sal_uInt16 aIds[] =
        {
            RES_PAGEDESC,
            RES_BREAK,
            RES_KEEP,
            RES_LAYOUT_SPLIT,
            RES_UL_SPACE,
            RES_SHADOW,
            RES_FRAMEDIR,
            // #i29550#
            RES_COLLAPSING_BORDERS,
            // <-- collapsing borders
            0
        };
    for( const sal_uInt16* pIds = aIds; *pIds; ++pIds )
        if( SFX_ITEM_SET == rSet.GetItemState( *pIds, sal_False, &pItem))
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

static void lcl_TabGetMaxLineWidth(const SvxBorderLine* pBorderLine, SvxBorderLine& rBorderLine)
{
    if(pBorderLine->GetWidth() > rBorderLine.GetWidth())
        rBorderLine.SetWidth(pBorderLine->GetWidth());

    rBorderLine.SetBorderLineStyle(pBorderLine->GetBorderLineStyle());
    rBorderLine.SetColor(pBorderLine->GetColor());
}

void SwTableShell::Execute(SfxRequest &rReq)
{
    const SfxItemSet* pArgs = rReq.GetArgs();
    SwWrtShell &rSh = GetShell();

    // At first the slots which doesn't need a FrmMgr.
    bool bMore = false;
    const SfxPoolItem* pItem = 0;
    sal_uInt16 nSlot = rReq.GetSlot();
    if(pArgs)
        pArgs->GetItemState(GetPool().GetWhich(nSlot), sal_False, &pItem);
    bool bCallDone = false;
    switch ( nSlot )
    {
        case SID_ATTR_BORDER:
        {
            if(!pArgs)
                break;
            // Create items, because we have to rework anyway.
            SvxBoxItem     aBox( RES_BOX );
            SfxItemSet aCoreSet( GetPool(),
                            RES_BOX, RES_BOX,
                            SID_ATTR_BORDER_INNER, SID_ATTR_BORDER_INNER,
                            0);
            SvxBoxInfoItem aCoreInfo( SID_ATTR_BORDER_INNER );
            aCoreSet.Put(aCoreInfo);
            rSh.GetTabBorders( aCoreSet );
            const SvxBoxItem& rCoreBox = (const SvxBoxItem&)
                                                    aCoreSet.Get(RES_BOX);
            const SfxPoolItem *pBoxItem = 0;
            if ( pArgs->GetItemState(RES_BOX, sal_True, &pBoxItem) == SFX_ITEM_SET )
            {
                aBox = *(SvxBoxItem*)pBoxItem;
                if ( !rReq.IsAPI() )
                    aBox.SetDistance( std::max(rCoreBox.GetDistance(),sal_uInt16(55)) );
                else if ( aBox.GetDistance() < MIN_BORDER_DIST )
                    aBox.SetDistance( std::max(rCoreBox.GetDistance(),(sal_uInt16)MIN_BORDER_DIST)  );
            }
            else
                OSL_ENSURE( !this, "where is BoxItem?" );

            //since the drawing layer also supports borders the which id might be a different one
            SvxBoxInfoItem aInfo( SID_ATTR_BORDER_INNER );
            if (pArgs->GetItemState(SID_ATTR_BORDER_INNER, sal_True, &pBoxItem) == SFX_ITEM_SET)
                aInfo = *(SvxBoxInfoItem*)pBoxItem;
            else if( pArgs->GetItemState(SDRATTR_TABLE_BORDER_INNER, sal_True, &pBoxItem) == SFX_ITEM_SET )
            {
                aInfo = *(SvxBoxInfoItem*)pBoxItem;
                aInfo.SetWhich(SID_ATTR_BORDER_INNER);
            }

            aInfo.SetTable( sal_True );
            aInfo.SetValid( VALID_DISABLE, sal_False );


// The attributes of all lines will be read and the strongest wins.
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
                aBorderLine.SetBorderLineStyle(table::BorderLineStyle::SOLID);
                aBorderLine.SetWidth( DEF_LINE_WIDTH_0 );
            }

            sal_Bool bLine = sal_False;
            if ( (pBorderLine = aBox.GetTop()) != NULL)
                aBox.SetLine(&aBorderLine, BOX_LINE_TOP), bLine |= sal_True;
            if ((pBorderLine = aBox.GetBottom()) != NULL)
                aBox.SetLine(&aBorderLine, BOX_LINE_BOTTOM), bLine |= sal_True;
            if ((pBorderLine = aBox.GetLeft()) != NULL)
                aBox.SetLine(&aBorderLine, BOX_LINE_LEFT), bLine |= sal_True;
            if ((pBorderLine = aBox.GetRight()) != NULL)
                aBox.SetLine(&aBorderLine, BOX_LINE_RIGHT), bLine |= sal_True;
            if ((pBorderLine = aInfo.GetHori()) != NULL)
                aInfo.SetLine(&aBorderLine, BOXINFO_LINE_HORI), bLine |= sal_True;
            if ((pBorderLine = aInfo.GetVert()) != NULL)
                aInfo.SetLine(&aBorderLine, BOXINFO_LINE_VERT), bLine |= sal_True;

            aCoreSet.Put( aBox  );
            aCoreSet.Put( aInfo );
            rSh.SetTabBorders( aCoreSet );

            // we must record the "real" values because otherwise the lines can't be reconstructed on playtime
            // the coding style of the controller (setting lines with width 0) is not transportable via Query/PutValue in
            // the SvxBoxItem
            rReq.AppendItem( aBox );
            rReq.AppendItem( aInfo );
            bCallDone = true;

        }
        break;
        case FN_INSERT_TABLE:
            InsertTable( rReq );
        break;
        case FN_FORMAT_TABLE_DLG:
        {
            //#127012# get the bindings before the dialog is called
            // it might happen that this shell is removed after closing the dialog
            SfxBindings& rBindings = GetView().GetViewFrame()->GetBindings();
            SfxItemSet aCoreSet( GetPool(), aUITableAttrRange);

            FieldUnit eMetric = ::GetDfltMetric(0 != PTR_CAST(SwWebView, &rSh.GetView()));
            SW_MOD()->PutItem(SfxUInt16Item(SID_ATTR_METRIC, static_cast< sal_uInt16 >(eMetric)));
            SwTableRep* pTblRep = ::lcl_TableParamToItemSet( aCoreSet, rSh );
            SfxAbstractTabDialog * pDlg = NULL;
            {
                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

                pDlg = pFact->CreateSwTableTabDlg(GetView().GetWindow(), GetPool(), &aCoreSet, &rSh);
                OSL_ENSURE(pDlg, "Dialogdiet fail!");

                if (pItem)
                    pDlg->SetCurPageId(OUStringToOString(((SfxStringItem *)pItem)->GetValue(), RTL_TEXTENCODING_UTF8));
            }
            aCoreSet.Put(SfxUInt16Item(SID_HTML_MODE, ::GetHtmlMode(GetView().GetDocShell())));
            rSh.GetTblAttr(aCoreSet);
            // GetTblAttr overwrites the background!
            SvxBrushItem aBrush( RES_BACKGROUND );
            if(rSh.GetBoxBackground(aBrush))
                aCoreSet.Put( aBrush );
            else
                aCoreSet.InvalidateItem( RES_BACKGROUND );

            if ( (!pDlg && rReq.GetArgs()) || pDlg->Execute() == RET_OK )
            {
                const SfxItemSet* pOutSet = pDlg ? pDlg->GetOutputItemSet() : rReq.GetArgs();
                if ( pDlg )
                {
                    //to record FN_INSERT_TABLE correctly
                    rReq.SetSlot(FN_FORMAT_TABLE_DLG);
                    rReq.Done( *pOutSet );
                }
                ItemSetToTableParam( *pOutSet, rSh );
            }

            delete pDlg;
            delete pTblRep;
            rBindings.Update(SID_RULER_BORDERS);
            rBindings.Update(SID_ATTR_TABSTOP);
            rBindings.Update(SID_RULER_BORDERS_VERTICAL);
            rBindings.Update(SID_ATTR_TABSTOP_VERTICAL);
        }
        break;
        case SID_ATTR_BRUSH:
        case SID_ATTR_BRUSH_ROW :
        case SID_ATTR_BRUSH_TABLE :
            if(rReq.GetArgs())
                ItemSetToTableParam(*rReq.GetArgs(), rSh);
        break;
        case FN_NUM_FORMAT_TABLE_DLG:
        {
            SwView* pView = GetActiveView();
            if(pView)
            {
                FieldUnit eMetric = ::GetDfltMetric(0 != PTR_CAST(SwWebView, pView));
                SW_MOD()->PutItem(SfxUInt16Item(SID_ATTR_METRIC, static_cast< sal_uInt16 >(eMetric)));
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

                String sCurText( rSh.GetTableBoxText() );
                aCoreSet.Put( SvxNumberInfoItem( pFormatter,
                                    ((SwTblBoxValue&)aBoxSet.Get(
                                        RES_BOXATR_VALUE)).GetValue(),
                                    sCurText, SID_ATTR_NUMBERFORMAT_INFO ));

                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

                SfxAbstractDialog* pDlg = pFact->CreateSfxDialog( GetView().GetWindow(),aCoreSet,
                    pView->GetViewFrame()->GetFrame().GetFrameInterface(),
                    RC_DLG_SWNUMFMTDLG );
                OSL_ENSURE(pDlg, "Dialogdiet fail!");

                if (RET_OK == pDlg->Execute())
                {
                    const SfxPoolItem* pNumberFormatItem = GetView().GetDocShell()->
                                    GetItem( SID_ATTR_NUMBERFORMAT_INFO );

                    if( pNumberFormatItem && 0 != ((SvxNumberInfoItem*)pNumberFormatItem)->GetDelCount() )
                    {
                        const sal_uInt32* pDelArr = ((SvxNumberInfoItem*)
                                                        pNumberFormatItem)->GetDelArray();

                        for ( sal_uInt16 i = 0; i < ((SvxNumberInfoItem*)pNumberFormatItem)->GetDelCount(); i++ )
                            ((SvxNumberInfoItem*)pNumberFormatItem)->
                            GetNumberFormatter()->DeleteEntry( pDelArr[i] );
                    }

                    if( SFX_ITEM_SET == pDlg->GetOutputItemSet()->GetItemState(
                        SID_ATTR_NUMBERFORMAT_VALUE, sal_False, &pNumberFormatItem ))
                    {
                        SfxItemSet aBoxFormatSet( *aCoreSet.GetPool(),
                                    RES_BOXATR_FORMAT, RES_BOXATR_FORMAT );
                        aBoxFormatSet.Put( SwTblBoxNumFormat(
                                ((SfxUInt32Item*)pNumberFormatItem)->GetValue() ));
                        rSh.SetTblBoxFormulaAttrs( aBoxFormatSet );

                    }
                }
                delete pDlg;
            }
        }
        break;
        case FN_CALC_TABLE:
            rSh.UpdateTable();
            bCallDone = true;
        break;
        case FN_TABLE_OPTIMAL_HEIGHT:
        {
            const SwFmtFrmSize aSz;
            rSh.SetRowHeight( aSz );
            bCallDone = true;
        }
        break;
        case FN_TABLE_DELETE_COL:
            if ( rSh.DeleteCol() && rSh.HasSelection() )
                rSh.EnterStdMode();
            bCallDone = true;
        break;
        case FN_END_TABLE:
            rSh.MoveTable( fnTableCurr, fnTableEnd );
            bCallDone = true;
        break;
        case FN_START_TABLE:
            rSh.MoveTable( fnTableCurr, fnTableStart );
            bCallDone = true;
        break;
        case FN_GOTO_NEXT_CELL:
        {
            sal_Bool bAppendLine = sal_True;
            if( pItem )
                bAppendLine = ((SfxBoolItem*)pItem)->GetValue();
            rReq.SetReturnValue( SfxBoolItem( nSlot,
                                    rSh.GoNextCell( bAppendLine ) ) );
            bCallDone = true;
        }
        break;
        case FN_GOTO_PREV_CELL:
            rReq.SetReturnValue( SfxBoolItem( nSlot, rSh.GoPrevCell() ) );
            bCallDone = true;
        break;
        case FN_TABLE_DELETE_ROW:
            if ( rSh.DeleteRow() && rSh.HasSelection() )
                rSh.EnterStdMode();
            bCallDone = true;
        break;
        case FN_TABLE_MERGE_CELLS:
            if ( rSh.IsTableMode() )
                switch ( rSh.MergeTab() )
                {
                    case TBLMERGE_OK:
                         bCallDone = true;
                    //no break;
                    case TBLMERGE_NOSELECTION:  break;
                    case TBLMERGE_TOOCOMPLEX:
                    {
                        InfoBox aInfoBox( GetView().GetWindow(),
                                    SW_RES( MSG_ERR_TABLE_MERGE ) );
                        aInfoBox.Execute();
                        break;
                    }
                    default: OSL_ENSURE( !this, "unknown return value MergeTab.");
                }
        break;
        case FN_TABLE_ADJUST_CELLS:
        case FN_TABLE_BALANCE_CELLS:
        {
            sal_Bool bBalance = (FN_TABLE_BALANCE_CELLS == nSlot);
            if ( rSh.IsAdjustCellWidthAllowed(bBalance) )
            {
                {
                    // remove actions to make a valid table selection
                    UnoActionRemoveContext aRemoveContext(rSh.GetDoc());
                }
                rSh.AdjustCellWidth(bBalance);
            }
            bCallDone = true;
        }
        break;
        case FN_TABLE_BALANCE_ROWS:
            if ( rSh.BalanceRowHeight(sal_True) )
                rSh.BalanceRowHeight(sal_False);
            bCallDone = true;
        break;
        case FN_TABLE_SELECT_ALL:
            rSh.EnterStdMode();
            rSh.MoveTable( fnTableCurr, fnTableStart );
            rSh.SttSelect();
            rSh.MoveTable( fnTableCurr, fnTableEnd );
            rSh.EndSelect();
            bCallDone = true;
        break;
        case FN_TABLE_SELECT_COL:
            rSh.EnterStdMode();
            rSh.SelectTableCol();
            bCallDone = true;
        break;
        case FN_TABLE_SELECT_ROW:
            rSh.EnterStdMode();
            rSh.SelectTableRow();
            bCallDone = true;
        break;
        case FN_TABLE_SET_READ_ONLY_CELLS:
            rSh.ProtectCells();
            rSh.ResetSelect( 0, sal_False );
            bCallDone = true;
        break;
        case FN_TABLE_UNSET_READ_ONLY_CELLS:
            rSh.UnProtectCells();
            bCallDone = true;
        break;
        case SID_AUTOFORMAT:
        {
            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

            AbstractSwAutoFormatDlg* pDlg = pFact->CreateSwAutoFormatDlg(&GetView().GetViewFrame()->GetWindow(), &rSh);
            OSL_ENSURE(pDlg, "Dialogdiet fail!");
            pDlg->Execute();
            delete pDlg;
        }
        break;
        case FN_TABLE_SET_ROW_HEIGHT:
        {
            SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
            OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

            VclAbstractDialog* pDlg = pFact->CreateVclAbstractDialog( GetView().GetWindow(), rSh, DLG_ROW_HEIGHT );
            OSL_ENSURE(pDlg, "Dialogdiet fail!");
            pDlg->Execute();
            delete pDlg;
        }
        break;
        case FN_NUMBER_BULLETS:
        case FN_NUM_BULLET_ON:
            OSL_ENSURE( !this, "function may not be called now." );
        break;

        case FN_TABLE_INSERT_COL:
        case FN_TABLE_INSERT_ROW:
        {
            bool bColumn = rReq.GetSlot() == FN_TABLE_INSERT_COL;
            sal_uInt16 nCount = 0;
            sal_Bool bAfter = sal_True;
            if (pItem)
            {
                nCount = ((const SfxInt16Item* )pItem)->GetValue();
                if(SFX_ITEM_SET == pArgs->GetItemState(FN_PARAM_INSERT_AFTER, sal_True, &pItem))
                    bAfter = ((const SfxBoolItem* )pItem)->GetValue();
            }
            else if( !rReq.IsAPI() )
                ++nCount;

            if( nCount )
            {
                // i74180: Table border patch submitted by chensuchun:
                // -->get the SvxBoxInfoItem of the table before insert
                SfxItemSet aCoreSet( GetPool(), aUITableAttrRange);
                ::lcl_TableParamToItemSet( aCoreSet, rSh );
                bool bSetInnerBorders = false;
                SwUndoId nUndoId = UNDO_EMPTY;
                // <--End

                if( bColumn )
                {
                    rSh.StartUndo( UNDO_TABLE_INSCOL );
                    rSh.InsertCol( nCount, bAfter );
                    bSetInnerBorders = true;
                    nUndoId = UNDO_TABLE_INSCOL;
                }
                else if ( !rSh.IsInRepeatedHeadline() )
                {
                    rSh.StartUndo( UNDO_TABLE_INSROW );
                    rSh.InsertRow( nCount, bAfter );
                    bSetInnerBorders = true;
                    nUndoId = UNDO_TABLE_INSROW;
                }

                // -->after inserting,reset the inner table borders
                if ( bSetInnerBorders )
                {
                    const SvxBoxInfoItem aBoxInfo((const SvxBoxInfoItem&)
                        aCoreSet.Get(SID_ATTR_BORDER_INNER));
                    SfxItemSet aSet( GetPool(), SID_ATTR_BORDER_INNER,
                                                SID_ATTR_BORDER_INNER, 0);
                    aSet.Put( aBoxInfo );
                    ItemSetToTableParam( aSet, rSh );
                    rSh.EndUndo( nUndoId );
                }

                bCallDone = true;
                break;
            }

            nSlot = bColumn ? FN_TABLE_INSERT_COL_DLG : FN_TABLE_INSERT_ROW_DLG;
        }
        // No break;  on Count = 0 appears the dialog
        case FN_TABLE_INSERT_COL_DLG:
        case FN_TABLE_INSERT_ROW_DLG:
        {
            const SfxSlot* pSlot = GetStaticInterface()->GetSlot(nSlot);
            if ( FN_TABLE_INSERT_ROW_DLG != nSlot || !rSh.IsInRepeatedHeadline())
            {
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                ::std::auto_ptr<SvxAbstractInsRowColDlg> pDlg( pFact ? pFact->CreateSvxInsRowColDlg( GetView().GetWindow(), nSlot == FN_TABLE_INSERT_COL_DLG, pSlot->GetCommand() ) : 0);

                if( pDlg.get() && (pDlg->Execute() == 1) )
                {
                    sal_uInt16 nDispatchSlot = (nSlot == FN_TABLE_INSERT_COL_DLG) ? FN_TABLE_INSERT_COL : FN_TABLE_INSERT_ROW;
                    SfxUInt16Item aCountItem( nDispatchSlot, static_cast< sal_uInt16 >(pDlg->getInsertCount()) );
                    SfxBoolItem  aAfter( FN_PARAM_INSERT_AFTER, !pDlg->isInsertBefore() );
                       SfxViewFrame* pVFrame = GetView().GetViewFrame();
                       if( pVFrame )
                        pVFrame->GetDispatcher()->Execute( nDispatchSlot, SFX_CALLMODE_SYNCHRON|SFX_CALLMODE_RECORD, &aCountItem, &aAfter, 0L);
                }
            }
        }
        break;
        case FN_TABLE_SPLIT_CELLS:
        {
            long nCount=0;
            sal_Bool bHorizontal=sal_True;
            sal_Bool bProportional = sal_False;
            SFX_REQUEST_ARG( rReq, pSplit, SfxInt32Item, FN_TABLE_SPLIT_CELLS, sal_False );
            SFX_REQUEST_ARG( rReq, pHor, SfxBoolItem, FN_PARAM_1, sal_False );
            SFX_REQUEST_ARG( rReq, pProp, SfxBoolItem, FN_PARAM_2, sal_False );
            if ( pSplit )
            {
                nCount = pSplit->GetValue();
                if ( pHor )
                    bHorizontal = pHor->GetValue();
                if ( pProp )
                    bProportional = pProp->GetValue();
            }
            else
            {
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                if( pFact )
                {
                    const long nMaxVert = rSh.GetAnyCurRect( RECT_FRM ).Width() / MINLAY;
                    SvxAbstractSplittTableDialog* pDlg = pFact->CreateSvxSplittTableDialog( GetView().GetWindow(), rSh.IsTableVertical(), nMaxVert, 99 );
                    if( pDlg && (pDlg->Execute() == RET_OK) )
                    {
                        nCount = pDlg->GetCount();
                        bHorizontal = pDlg->IsHorizontal();
                        bProportional = pDlg->IsProportional();
                        rReq.AppendItem( SfxInt32Item( FN_TABLE_SPLIT_CELLS, nCount ) );
                        rReq.AppendItem( SfxBoolItem( FN_PARAM_1, bHorizontal ) );
                        rReq.AppendItem( SfxBoolItem( FN_PARAM_2, bProportional ) );
                    }
                    delete pDlg;
                }
            }

            if ( nCount>1 )
            {
                rSh.SplitTab(!bHorizontal, static_cast< sal_uInt16 >( nCount-1 ), bProportional );
                bCallDone = true;
            }
            else
                rReq.Ignore();
        }
        break;

        case FN_TABLE_SPLIT_TABLE:
        {
            SFX_REQUEST_ARG( rReq, pType, SfxUInt16Item, FN_PARAM_1, sal_False );
            if( pType )
            {
                switch( pType->GetValue() )
                {
                    case HEADLINE_NONE    :
                    case HEADLINE_BORDERCOPY:
                    case HEADLINE_CNTNTCOPY:
                    case HEADLINE_BOXATTRCOPY:
                    case HEADLINE_BOXATRCOLLCOPY:
                        rSh.SplitTable(pType->GetValue()) ;
                    default: ;//wrong parameter, do nothing
                }
            }
            else
            {
                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");

                AbstractSplitTableDialog* pDlg = pFact->CreateSplitTblDialog( GetView().GetWindow(), rSh );
                OSL_ENSURE(pDlg, "Dialogdiet fail!");
                pDlg->Execute();
                rReq.AppendItem( SfxUInt16Item( FN_PARAM_1, pDlg->GetSplitMode() ) );
                delete pDlg;
                bCallDone = true;
            }
        }
        break;

        case FN_TABLE_MERGE_TABLE:
        {
            sal_Bool bPrev = rSh.CanMergeTable( sal_True );
            sal_Bool bNext = rSh.CanMergeTable( sal_False );

            if( bPrev && bNext )
            {
                SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
                OSL_ENSURE(pFact, "SwAbstractDialogFactory fail!");
                VclAbstractDialog* pDlg = pFact->CreateTblMergeDialog(GetView().GetWindow(), bPrev);
                OSL_ENSURE(pDlg, "dialogdiet pDlg fail!");
                if( RET_OK != pDlg->Execute())
                    bPrev = bNext = sal_False;
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
            static sal_uInt16 aInva[] =
                            {   FN_TABLE_MODE_FIX,
                                FN_TABLE_MODE_FIX_PROP,
                                FN_TABLE_MODE_VARIABLE,
                                0
                            };
            rBind.Invalidate( aInva );
            bCallDone = true;
        }
        break;
        case FN_TABLE_AUTOSUM:
        {
            SfxViewFrame* pVFrame = GetView().GetViewFrame();
            pVFrame->GetDispatcher()->Execute(FN_EDIT_FORMULA, SFX_CALLMODE_SYNCHRON);
            const sal_uInt16 nId = SwInputChild::GetChildWindowId();
            SwInputChild* pChildWin = (SwInputChild*)pVFrame->
                                                GetChildWindow( nId );
            String sSum;
            GetShell().GetAutoSum(sSum);
            if( pChildWin )
                pChildWin->SetFormula( sSum );

        }
        break;
        case FN_TABLE_HEADLINE_REPEAT:
            if(0 != rSh.GetRowsToRepeat())
                rSh.SetRowsToRepeat( 0 );
            else
                rSh.SetRowsToRepeat(rSh.GetRowSelectionFromTop());
        break;
        case FN_TABLE_SELECT_CELL   :
            rSh.SelectTableCell();
        break;
        case FN_TABLE_DELETE_TABLE  :
        {
            rSh.StartAction();
            rSh.StartUndo();
            rSh.GetView().GetViewFrame()->GetDispatcher()->Execute(FN_TABLE_SELECT_ALL);
            rSh.DeleteRow();
            rSh.EndUndo();
            rSh.EndAction();
        }
        //'this' is already destroyed
        return;
        //break;
        default:
            bMore = true;
    }

    if ( !bMore )
    {
        if(bCallDone)
            rReq.Done();
        return;
    }
    else
        bMore = false;
    // Now the slots which are working directly on the TableFmt.
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
                aHori.SetHoriOrient( text::HoriOrientation::NONE );
                aSet.Put( aLRSpace );
                rSh.SetTblAttr( aSet );
            }
        }
        break;
// The last case branch which needs a table manager!!
        case FN_TABLE_SET_COL_WIDTH:
        {
            SwTableFUNC aMgr( &rSh, sal_False);
            aMgr.ColWidthDlg(GetView().GetWindow());
        }
        break;
        case SID_TABLE_VERT_NONE:
        case SID_TABLE_VERT_CENTER:
        case SID_TABLE_VERT_BOTTOM:
        {
                sal_uInt16 nAlign = nSlot == SID_TABLE_VERT_NONE ?
                                text::VertOrientation::NONE :
                                    nSlot == SID_TABLE_VERT_CENTER ?
                                        text::VertOrientation::CENTER : text::VertOrientation::BOTTOM;
            rSh.SetBoxAlign(nAlign);
            bCallDone = true;
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
        case FN_TABLE_ROW_SPLIT :
        {
            const SfxBoolItem* pBool = static_cast<const SfxBoolItem*>(pItem);
            SwFmtRowSplit* pSplit = 0;
            if(!pBool)
            {
                rSh.GetRowSplit(pSplit);
                if(pSplit)
                    pSplit->SetValue(!pSplit->GetValue());
                else
                   pSplit = new SwFmtRowSplit(sal_True);
            }
            else
            {
                pSplit = new SwFmtRowSplit(pBool->GetValue());
            }
            rSh.SetRowSplit( *pSplit );
            delete pSplit;
        }
        break;

        default:
            OSL_ENSURE( !this, "wrong Dispatcher" );
            return;
    }
    if(bCallDone)
        rReq.Done();
}

void SwTableShell::GetState(SfxItemSet &rSet)
{
    SfxWhichIter aIter( rSet );
    SwWrtShell &rSh = GetShell();
    SwFrmFmt *pFmt = rSh.GetTableFmt();
    // os #124829# crash report: in case of an invalid shell selection return immediately
    if(!pFmt)
        return;
    sal_uInt16 nSlot = aIter.FirstWhich();
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
                if ( !rSh.IsAdjustCellWidthAllowed(sal_True) )
                    rSet.DisableItem(FN_TABLE_BALANCE_CELLS);
            break;

            case FN_TABLE_BALANCE_ROWS:
                if ( !rSh.BalanceRowHeight(sal_True) )
                    rSet.DisableItem(FN_TABLE_BALANCE_ROWS);
            break;
            case FN_OPTIMIZE_TABLE:
                if ( !rSh.IsTableMode() &&
                        !rSh.IsAdjustCellWidthAllowed() &&
                        !rSh.IsAdjustCellWidthAllowed(sal_True) &&
                        !rSh.BalanceRowHeight(sal_True) )
                    rSet.DisableItem(FN_OPTIMIZE_TABLE);
            break;
            case SID_INSERT_DIAGRAM:
                {
                    SvtModuleOptions aMOpt;
                    if ( !aMOpt.IsMath() || rSh.IsTblComplexForChart() )
                        rSet.DisableItem(nSlot);
                }
                break;

            case FN_INSERT_TABLE:
                // Put any value, so that the controller remains enabled.
                // Instead "Insert:Table" appears "Format:Table".
                break;

            case FN_TABLE_OPTIMAL_HEIGHT:
            {
                // Disable if auto height already is enabled.
                SwFmtFrmSize *pSz;
                rSh.GetRowHeight( pSz );
                if ( pSz )
                {
                    if ( ATT_VAR_SIZE == pSz->GetHeightSizeType() )
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

            case SID_TABLE_VERT_NONE:
            case SID_TABLE_VERT_CENTER:
            case SID_TABLE_VERT_BOTTOM:
            {
                sal_uInt16 nAlign = rSh.GetBoxAlign();
                sal_Bool bSet = nSlot == (SID_TABLE_VERT_NONE && nAlign == text::VertOrientation::NONE) ||
                            (nSlot == SID_TABLE_VERT_CENTER && nAlign == text::VertOrientation::CENTER) ||
                            (nSlot == SID_TABLE_VERT_BOTTOM && nAlign == text::VertOrientation::BOTTOM);
                rSet.Put(SfxBoolItem(nSlot, bSet));
            }
            break;

            case FN_TABLE_MODE_FIX       :
            case FN_TABLE_MODE_FIX_PROP  :
            case FN_TABLE_MODE_VARIABLE  :
                {
                    TblChgMode nMode = rSh.GetTblChgMode();
                    sal_Bool bSet = (nSlot == FN_TABLE_MODE_FIX && nMode == TBLFIX_CHGABS) ||
                            (nSlot == FN_TABLE_MODE_FIX_PROP && nMode == TBLFIX_CHGPROP) ||
                            (nSlot == FN_TABLE_MODE_VARIABLE && nMode == TBLVAR_CHGABS);
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
                sal_Bool bAsk;
                if( !rSh.CanMergeTable( sal_True, &bAsk ))
                    rSet.DisableItem( nSlot );
            }
            break;

            case FN_TABLE_DELETE_ROW:
                {
                    SwSelBoxes aBoxes;
                    ::GetTblSel( rSh, aBoxes, nsSwTblSearchType::TBLSEARCH_ROW );
                    if( ::HasProtectedCells( aBoxes ))
                        rSet.DisableItem( nSlot );
                }
                break;
            case FN_TABLE_DELETE_COL:
                {
                    SwSelBoxes aBoxes;
                    ::GetTblSel( rSh, aBoxes, nsSwTblSearchType::TBLSEARCH_COL );
                    if( ::HasProtectedCells( aBoxes ))
                        rSet.DisableItem( nSlot );
                }
                break;

            case FN_TABLE_UNSET_READ_ONLY_CELLS:
                // disable in readonly sections, but enable in protected cells
                if( !rSh.CanUnProtectCells() )
                    rSet.DisableItem( nSlot );
                break;
            case RES_ROW_SPLIT:
            {
                const SwFmtLayoutSplit& rTabSplit = pFmt->GetLayoutSplit();
                if ( 0 == rTabSplit.GetValue() )
                {
                    rSet.DisableItem( nSlot );
                }
                else
                {
                    SwFmtRowSplit* pSplit = 0;
                    rSh.GetRowSplit(pSplit);
                    if(pSplit)
                        rSet.Put(*pSplit);
                    else
                        rSet.InvalidateItem( nSlot );
                    delete pSplit;
                }
            }
            break;
            case FN_TABLE_HEADLINE_REPEAT:
                if(0 != rSh.GetRowsToRepeat())
                    rSet.Put(SfxBoolItem(nSlot, sal_True));
                else if(!rSh.GetRowSelectionFromTop())
                    rSet.DisableItem( nSlot );
                else
                    rSet.Put(SfxBoolItem(nSlot, sal_False));
            break;
            case FN_TABLE_SELECT_CELL   :
                if(rSh.HasBoxSelection())
                    rSet.DisableItem( nSlot );
            break;
        }
    nSlot = aIter.NextWhich();
    }
}

SwTableShell::SwTableShell(SwView &_rView) :
    SwBaseShell(_rView)
{
    SetName(OUString("Table"));
    SetHelpId(SW_TABSHELL);
    SfxShell::SetContextName(sfx2::sidebar::EnumContext::GetContextName(sfx2::sidebar::EnumContext::Context_Table));
}

void SwTableShell::GetFrmBorderState(SfxItemSet &rSet)
{
    SfxItemSet aCoreSet( GetPool(),
                         RES_BOX, RES_BOX,
                         SID_ATTR_BORDER_INNER, SID_ATTR_BORDER_INNER, 0 );
    SvxBoxInfoItem aBoxInfo( SID_ATTR_BORDER_INNER );
    aCoreSet.Put( aBoxInfo );
    GetShell().GetTabBorders( aCoreSet );
    rSet.Put( aCoreSet );
}

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
                if ( rReq.GetSlot() == SID_FRAME_LINESTYLE )
                {
                    const SvxLineItem &rLineItem = (const SvxLineItem&)pArgs->
                                                            Get( SID_FRAME_LINESTYLE );
                    const SvxBorderLine* pBorderLine = rLineItem.GetLine();
                    rSh.SetTabLineStyle( 0, sal_True, pBorderLine);
                }
                else
                {
                    const SvxColorItem &rNewColorItem = (const SvxColorItem&)pArgs->
                                                            Get( SID_FRAME_LINECOLOR );
                    rSh.SetTabLineStyle( &rNewColorItem.GetValue() );
                }

                rReq.Done();

                break;
            }
        }
}

void SwTableShell::GetLineStyleState(SfxItemSet &rSet)
{
    SfxItemSet aCoreSet( GetPool(),
                            RES_BOX, RES_BOX,
                            SID_ATTR_BORDER_INNER, SID_ATTR_BORDER_INNER, 0);
    SvxBoxInfoItem aCoreInfo( SID_ATTR_BORDER_INNER );
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

    // At first the slots, which doesn't need a FrmMgr.
    const SfxPoolItem* pItem = 0;
    sal_uInt16 nSlot = rReq.GetSlot();
    if(pArgs)
        pArgs->GetItemState(GetPool().GetWhich(nSlot), sal_False, &pItem);

    // Always aquire the language from the current cursor position.
    LanguageType eLang = rSh.GetCurLang();
    SvNumberFormatter* pFormatter = rSh.GetNumberFormatter();
    sal_uInt32 nNumberFormat = NUMBERFORMAT_ENTRY_NOT_FOUND;
    sal_uInt16 nFmtType = 0, nOffset = 0;

    switch ( nSlot )
    {
    case FN_NUMBER_FORMAT:
        if( pItem )
        {
            // Determine index for string.
            OUString aCode( ((const SfxStringItem*)pItem)->GetValue() );
            nNumberFormat = pFormatter->GetEntryKey( aCode, eLang );
            if( NUMBERFORMAT_ENTRY_NOT_FOUND == nNumberFormat )
            {
                // Re-enter
                sal_Int32 nErrPos;
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
        OSL_FAIL("wrong dispatcher");
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
