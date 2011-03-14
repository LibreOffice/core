/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include "formatclipboard.hxx"


#include <hintids.hxx>
#include <svx/svxids.hrc>
#include <cmdid.h>
#include <format.hxx>
#include <charfmt.hxx>
#include <fmtcol.hxx>
#include <frmfmt.hxx>
#include <docstyle.hxx>
#include <fchrfmt.hxx>
#include <pam.hxx>
// header for class SdrView
#include <svx/svdview.hxx>
//SvxBrushItem
#include <editeng/brshitem.hxx>
#include <editeng/shaditem.hxx>
#include <frmatr.hxx>
// header for class SvxBoxInfoItem
#include <editeng/boxitem.hxx>
// header for class SvxFmtBreakItem
#include <editeng/brkitem.hxx>
// header for class SwFmtLayoutSplit
#include <fmtlsplt.hxx>
// header for class SvxFmtKeepItem
#include <editeng/keepitem.hxx>
// header for class SvxFrameDirectionItem
#include <editeng/frmdiritem.hxx>
#include <paratr.hxx>
#include <fmtpdsc.hxx>
#include <fmtrowsplt.hxx>
#include <swundo.hxx>           // fuer die UndoIds
#include <boost/shared_ptr.hpp>

/*--------------------------------------------------------------------
 --------------------------------------------------------------------*/

namespace
{
#define FORMAT_PAINTBRUSH_FRAME_IDS \
RES_FRMATR_BEGIN, RES_FILL_ORDER, \
/* no RES_FRM_SIZE */ \
RES_PAPER_BIN, RES_SURROUND, \
/* no RES_VERT_ORIENT */ \
/* no RES_HORI_ORIENT */ \
/* no RES_ANCHOR */ \
RES_BACKGROUND, RES_SHADOW, \
/* no RES_FRMMACRO */ \
RES_COL, RES_KEEP, \
/* no RES_URL */ \
RES_EDIT_IN_READONLY, RES_LAYOUT_SPLIT, \
/* no RES_CHAIN */ \
RES_TEXTGRID, RES_FRMATR_END-1,

#define FORMAT_PAINTBRUSH_PARAGRAPH_IDS \
RES_PARATR_BEGIN, RES_PARATR_END -1, \
RES_PARATR_LIST_BEGIN, RES_PARATR_LIST_END -1, \
FORMAT_PAINTBRUSH_FRAME_IDS \
FN_NUMBER_NEWSTART, FN_NUMBER_NEWSTART, \
FN_NUMBER_NEWSTART_AT, FN_NUMBER_NEWSTART_AT,

SfxItemSet* lcl_CreateEmptyItemSet( int nSelectionType, SfxItemPool& rPool
            , bool bNoCharacterFormats = false, bool bNoParagraphFormats = false )
{
    SfxItemSet* pItemSet = 0;
    if( nSelectionType & (nsSelectionType::SEL_FRM | nsSelectionType::SEL_OLE | nsSelectionType::SEL_GRF) )
    {
        pItemSet = new SfxItemSet(rPool,
                        FORMAT_PAINTBRUSH_FRAME_IDS
                        0);
    }
    else if( nSelectionType & nsSelectionType::SEL_DRW )
    {
        //is handled different
    }
    else if( nSelectionType == nsSelectionType::SEL_TBL )
    {
        pItemSet = new SfxItemSet(rPool,
                        SID_ATTR_BORDER_INNER,  SID_ATTR_BORDER_SHADOW, //SID_ATTR_BORDER_OUTER is inbetween
                        RES_BACKGROUND,         RES_SHADOW, //RES_BOX is inbetween
                        SID_ATTR_BRUSH_ROW,     SID_ATTR_BRUSH_TABLE,
                        RES_BREAK,              RES_BREAK,
                        RES_PAGEDESC,           RES_PAGEDESC,
                        RES_LAYOUT_SPLIT,       RES_LAYOUT_SPLIT,
                        RES_ROW_SPLIT,          RES_ROW_SPLIT,
                        RES_KEEP,               RES_KEEP,
                        RES_FRAMEDIR,           RES_FRAMEDIR,
                        FN_PARAM_TABLE_HEADLINE, FN_PARAM_TABLE_HEADLINE,
                        FN_TABLE_BOX_TEXTDIRECTION, FN_TABLE_BOX_TEXTDIRECTION,
                        FN_TABLE_SET_VERT_ALIGN, FN_TABLE_SET_VERT_ALIGN,
                        0);
    }
    else if( nSelectionType & nsSelectionType::SEL_TXT )
    {
        if( bNoCharacterFormats )
            pItemSet = new SfxItemSet(rPool,
                    FORMAT_PAINTBRUSH_PARAGRAPH_IDS
                    0);
        else if( bNoParagraphFormats )
            pItemSet = new SfxItemSet(rPool,
                    RES_CHRATR_BEGIN, RES_CHRATR_END - 1,
                    0);
        else
            pItemSet = new SfxItemSet(rPool,
                    RES_CHRATR_BEGIN, RES_CHRATR_END - 1,
                    FORMAT_PAINTBRUSH_PARAGRAPH_IDS
                    0);
    }
    return pItemSet;
}

void lcl_getTableAttributes( SfxItemSet& rSet, SwWrtShell &rSh )
{
    SvxBrushItem aBrush( RES_BACKGROUND );
     rSh.GetBoxBackground(aBrush);
    rSet.Put( aBrush );
    if(rSh.GetRowBackground(aBrush))
        rSet.Put( aBrush, SID_ATTR_BRUSH_ROW );
    else
        rSet.InvalidateItem(SID_ATTR_BRUSH_ROW);
    rSh.GetTabBackground(aBrush);
    rSet.Put( aBrush, SID_ATTR_BRUSH_TABLE );

    SvxBoxInfoItem aBoxInfo( SID_ATTR_BORDER_INNER );
    rSet.Put(aBoxInfo);
    rSh.GetTabBorders( rSet );

    SvxFrameDirectionItem aBoxDirection( FRMDIR_ENVIRONMENT, RES_FRAMEDIR );
    if(rSh.GetBoxDirection( aBoxDirection ))
        rSet.Put(aBoxDirection, FN_TABLE_BOX_TEXTDIRECTION);

    rSet.Put(SfxUInt16Item(FN_TABLE_SET_VERT_ALIGN, rSh.GetBoxAlign()));

    rSet.Put( SfxUInt16Item( FN_PARAM_TABLE_HEADLINE, rSh.GetRowsToRepeat() ) );

    SwFrmFmt *pFrmFmt = rSh.GetTableFmt();
    if(pFrmFmt)
    {
        rSet.Put( pFrmFmt->GetShadow() );
        rSet.Put( pFrmFmt->GetBreak() );
        rSet.Put( pFrmFmt->GetPageDesc() );
        rSet.Put( pFrmFmt->GetLayoutSplit() );
        rSet.Put( pFrmFmt->GetKeep() );
        rSet.Put( pFrmFmt->GetFrmDir() );
    }

    SwFmtRowSplit* pSplit = 0;
    rSh.GetRowSplit(pSplit);
    if(pSplit)
        rSet.Put(*pSplit);
}

void lcl_setTableAttributes( const SfxItemSet& rSet, SwWrtShell &rSh )
{
    const SfxPoolItem* pItem = 0;
    sal_Bool bBorder = ( SFX_ITEM_SET == rSet.GetItemState( RES_BOX ) ||
            SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_BORDER_INNER ) );
    pItem = 0;
    sal_Bool bBackground = SFX_ITEM_SET == rSet.GetItemState( RES_BACKGROUND, sal_False, &pItem );
    const SfxPoolItem* pRowItem = 0, *pTableItem = 0;
    bBackground |= SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_BRUSH_ROW, sal_False, &pRowItem );
    bBackground |= SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_BRUSH_TABLE, sal_False, &pTableItem );

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
        rSh.SetTabBorders( rSet );

    if( SFX_ITEM_SET == rSet.GetItemState( FN_PARAM_TABLE_HEADLINE, sal_False, &pItem) )
        rSh.SetRowsToRepeat( ((SfxUInt16Item*)pItem)->GetValue() );

    SwFrmFmt* pFrmFmt = rSh.GetTableFmt();
    if(pFrmFmt)
    {
        //RES_SHADOW
        pItem=0;
        rSet.GetItemState(rSet.GetPool()->GetWhich(RES_SHADOW), sal_False, &pItem);
        if(pItem)
            pFrmFmt->SetFmtAttr( *pItem );

        //RES_BREAK
        pItem=0;
        rSet.GetItemState(rSet.GetPool()->GetWhich(RES_BREAK), sal_False, &pItem);
        if(pItem)
            pFrmFmt->SetFmtAttr( *pItem );

        //RES_PAGEDESC
        pItem=0;
        rSet.GetItemState(rSet.GetPool()->GetWhich(RES_PAGEDESC), sal_False, &pItem);
        if(pItem)
            pFrmFmt->SetFmtAttr( *pItem );

        //RES_LAYOUT_SPLIT
        pItem=0;
        rSet.GetItemState(rSet.GetPool()->GetWhich(RES_LAYOUT_SPLIT), sal_False, &pItem);
        if(pItem)
            pFrmFmt->SetFmtAttr( *pItem );

        //RES_KEEP
        pItem=0;
        rSet.GetItemState(rSet.GetPool()->GetWhich(RES_KEEP), sal_False, &pItem);
        if(pItem)
            pFrmFmt->SetFmtAttr( *pItem );

        //RES_FRAMEDIR
        pItem=0;
        rSet.GetItemState(rSet.GetPool()->GetWhich(RES_FRAMEDIR), sal_False, &pItem);
        if(pItem)
            pFrmFmt->SetFmtAttr( *pItem );
    }

    if( SFX_ITEM_SET == rSet.GetItemState( FN_TABLE_BOX_TEXTDIRECTION, sal_False, &pItem) )
    {
        SvxFrameDirectionItem aDirection( FRMDIR_ENVIRONMENT, RES_FRAMEDIR );
        aDirection.SetValue(static_cast< const SvxFrameDirectionItem* >(pItem)->GetValue());
        rSh.SetBoxDirection(aDirection);
    }

    if( SFX_ITEM_SET == rSet.GetItemState( FN_TABLE_SET_VERT_ALIGN, sal_False, &pItem))
        rSh.SetBoxAlign(((SfxUInt16Item*)(pItem))->GetValue());

    if( SFX_ITEM_SET == rSet.GetItemState( RES_ROW_SPLIT, sal_False, &pItem) )
        rSh.SetRowSplit(*static_cast<const SwFmtRowSplit*>(pItem));
}
}//end anonymous namespace

SwFormatClipboard::SwFormatClipboard()
        : m_nSelectionType(0)
        , m_pItemSet(0)
        , m_pTableItemSet(0)
        , m_bPersistentCopy(false)
{
}
SwFormatClipboard::~SwFormatClipboard()
{
    if(m_pItemSet)
        delete m_pItemSet;
    if(m_pTableItemSet)
        delete m_pTableItemSet;
}

bool SwFormatClipboard::HasContent() const
{
    return m_pItemSet!=0
        || m_pTableItemSet != 0
        || m_aCharStyle.Len()
        || m_aParaStyle.Len()
        ;
}
bool SwFormatClipboard::HasContentForThisType( int nSelectionType ) const
{
    if( !HasContent() )
        return false;

    if( m_nSelectionType == nSelectionType )
        return true;

    if(   ( nSelectionType & (nsSelectionType::SEL_FRM | nsSelectionType::SEL_OLE | nsSelectionType::SEL_GRF) )
        &&
        ( m_nSelectionType & (nsSelectionType::SEL_FRM | nsSelectionType::SEL_OLE | nsSelectionType::SEL_GRF) )
        )
        return true;

    if( nSelectionType & nsSelectionType::SEL_TXT && m_nSelectionType & nsSelectionType::SEL_TXT )
        return true;

    return false;
}

bool SwFormatClipboard::CanCopyThisType( int nSelectionType ) const
{
    if( nSelectionType & (nsSelectionType::SEL_FRM | nsSelectionType::SEL_OLE | nsSelectionType::SEL_GRF
         | nsSelectionType::SEL_TXT | nsSelectionType::SEL_DRW | nsSelectionType::SEL_TBL | nsSelectionType::SEL_TBL_CELLS ) )
         return true;
    return false;
}

void SwFormatClipboard::Copy( SwWrtShell& rWrtShell, SfxItemPool& rPool, bool bPersistentCopy )
{
    this->Erase();
    m_bPersistentCopy = bPersistentCopy;

    int nSelectionType = rWrtShell.GetSelectionType();
    SfxItemSet* pItemSet = lcl_CreateEmptyItemSet( nSelectionType, rPool );

    rWrtShell.StartAction();
    rWrtShell.Push();
    if( nSelectionType == nsSelectionType::SEL_TXT )
    {
        SwPaM* pCrsr = rWrtShell.GetCrsr();
        //select one character only to get the attributes of this single character only
        sal_Bool bHasSelection = pCrsr->HasMark();
        sal_Bool bForwardSelection = sal_False;

        if(!bHasSelection) //check for and handle multiselections
        {
            if( pCrsr->GetPrev() != pCrsr && pCrsr->GetPrev() != 0)
            {
                pCrsr = (SwPaM*)pCrsr->GetPrev();
                bForwardSelection = (*pCrsr->GetPoint()) > (*pCrsr->GetMark());
                bHasSelection = true;
                pCrsr->DeleteMark();
                pCrsr->SetMark();
                rWrtShell.KillPams();
                pCrsr = rWrtShell.GetCrsr();
            }
        }
        else
            bForwardSelection = (*pCrsr->GetPoint()) > (*pCrsr->GetMark());
        pCrsr->DeleteMark();
        pCrsr->SetMark();

        if( !bHasSelection && rWrtShell.IsInRightToLeftText() )
            bForwardSelection = !bForwardSelection;

        if( !( !bHasSelection && rWrtShell.IsEndPara() ) )
            pCrsr->Move( bForwardSelection ? fnMoveBackward : fnMoveForward );
    }

    if(pItemSet)
    {
        if( nSelectionType & (nsSelectionType::SEL_FRM | nsSelectionType::SEL_OLE | nsSelectionType::SEL_GRF) )
            rWrtShell.GetFlyFrmAttr(*pItemSet);
        else
        {
            rWrtShell.GetCurAttr(*pItemSet);

            // additional numbering properties for paragraph styles
            if( nSelectionType & nsSelectionType::SEL_TXT && rWrtShell.GetCurNumRule() )
            {
                SfxBoolItem aStart(FN_NUMBER_NEWSTART, rWrtShell.IsNumRuleStart());
                pItemSet->Put(aStart);
                SfxUInt16Item aStartAt(FN_NUMBER_NEWSTART_AT, rWrtShell.GetNodeNumStart());
                pItemSet->Put(aStartAt);
            }
        }
    }
    else if ( nSelectionType & nsSelectionType::SEL_DRW )
    {
        SdrView* pDrawView = rWrtShell.GetDrawView();
        if(pDrawView)
        {
            sal_Bool bOnlyHardAttr = sal_True;
            if( pDrawView->AreObjectsMarked() )
            {
                pItemSet = new SfxItemSet( pDrawView->GetAttrFromMarked(bOnlyHardAttr) );
                //remove attributes defining the type/data of custom shapes
                pItemSet->ClearItem(SDRATTR_CUSTOMSHAPE_ENGINE);
                pItemSet->ClearItem(SDRATTR_CUSTOMSHAPE_DATA);
                pItemSet->ClearItem(SDRATTR_CUSTOMSHAPE_GEOMETRY);
                pItemSet->ClearItem(SDRATTR_CUSTOMSHAPE_REPLACEMENT_URL);
            }
        }
    }
    if( nSelectionType & nsSelectionType::SEL_TBL_CELLS )//only copy table attributes if really cells are selected (not only text in tables)
    {
        m_pTableItemSet = lcl_CreateEmptyItemSet( nsSelectionType::SEL_TBL, rPool );
        lcl_getTableAttributes( *m_pTableItemSet, rWrtShell );
    }

    m_nSelectionType = nSelectionType;
    m_pItemSet = pItemSet;

    if( nSelectionType & nsSelectionType::SEL_TXT )
    {
        SwFmt* pFmt = rWrtShell.GetCurCharFmt();
        if( pFmt )
            m_aCharStyle = pFmt->GetName();

        pFmt = rWrtShell.GetCurTxtFmtColl();
        if( pFmt )
            m_aParaStyle = pFmt->GetName();
    }
    rWrtShell.Pop(sal_False);
    rWrtShell.EndAction();
}
typedef boost::shared_ptr< SfxPoolItem > SfxPoolItemSharedPtr;
typedef std::vector< SfxPoolItemSharedPtr > ItemVector;
// collect all PoolItems from the applied styles
void lcl_AppendSetItems( ItemVector& rItemVector, const SfxItemSet& rStyleAttrSet )
{
    const sal_uInt16*  pRanges = rStyleAttrSet.GetRanges();
    while( *pRanges )
    {
        for ( sal_uInt16 nWhich = *pRanges; nWhich <= *(pRanges+1); ++nWhich )
        {
            const SfxPoolItem* pItem;
            if( SFX_ITEM_SET == rStyleAttrSet.GetItemState( nWhich, sal_False, &pItem ) )
            {
                rItemVector.push_back( SfxPoolItemSharedPtr( pItem->Clone() ) );
            }
        }
        pRanges += 2;
    }
}
// remove all items that are inherited from the styles
void lcl_RemoveEqualItems( SfxItemSet& rTemplateItemSet, ItemVector& rItemVector )
{
    ItemVector::iterator aEnd = rItemVector.end();
    ItemVector::iterator aIter = rItemVector.begin();
    while( aIter != aEnd )
    {
        const SfxPoolItem* pItem;
        if( SFX_ITEM_SET == rTemplateItemSet.GetItemState( (*aIter)->Which(), sal_True, &pItem ) &&
            *pItem == *(*aIter) )
        {
            rTemplateItemSet.ClearItem( (*aIter)->Which() );
        }
        ++aIter;
    }
}

void SwFormatClipboard::Paste( SwWrtShell& rWrtShell, SfxStyleSheetBasePool* pPool
                              , bool bNoCharacterFormats, bool bNoParagraphFormats )
{
    int nSelectionType = rWrtShell.GetSelectionType();
    if( !this->HasContentForThisType(nSelectionType) )
    {
        if(!m_bPersistentCopy)
            this->Erase();
        return;
    }

    rWrtShell.StartAction();
    rWrtShell.StartUndo(UNDO_INSATTR);

    ItemVector aItemVector;
    if(pPool) //to find the styles we need the pool
    {
        if( nSelectionType & nsSelectionType::SEL_TXT )
        {
            if(m_aCharStyle.Len() && !bNoCharacterFormats )
            {
                SwDocStyleSheet* pStyle = (SwDocStyleSheet*)pPool->Find(m_aCharStyle, SFX_STYLE_FAMILY_CHAR);
                if( pStyle )
                {
                    SwFmtCharFmt aFmt(pStyle->GetCharFmt());
                    // collect items from character style
                    lcl_AppendSetItems( aItemVector, aFmt.GetCharFmt()->GetAttrSet());
                    sal_uInt16 nFlags=0;
                    rWrtShell.SetAttr( aFmt, nFlags );
                }
            }
            if(m_aParaStyle.Len() && !bNoParagraphFormats )
            {
                SwDocStyleSheet* pStyle = (SwDocStyleSheet*)pPool->Find(m_aParaStyle, SFX_STYLE_FAMILY_PARA);
                if( pStyle )
                {
                    // collect items from paragraph style
                    lcl_AppendSetItems( aItemVector, pStyle->GetCollection()->GetAttrSet());
                    rWrtShell.SetTxtFmtColl( pStyle->GetCollection() );
                }
            }
        }
    }
    if(m_pItemSet)
    {
        if( nSelectionType & nsSelectionType::SEL_DRW )
        {
            SdrView* pDrawView = rWrtShell.GetDrawView();
            if(pDrawView)
            {
                sal_Bool bReplaceAll = sal_True;
                pDrawView->SetAttrToMarked(*m_pItemSet, bReplaceAll);
            }
        }
        else
        {
            SfxItemSet* pTemplateItemSet = lcl_CreateEmptyItemSet(
                              nSelectionType, *m_pItemSet->GetPool()
                              , bNoCharacterFormats, bNoParagraphFormats );
            if(pTemplateItemSet)
            {
                pTemplateItemSet->Put( *m_pItemSet );
                // only _set_ attributes that differ from style attributes should be applied - the style is applied anyway
                lcl_RemoveEqualItems( *pTemplateItemSet, aItemVector );

                if( nSelectionType & (nsSelectionType::SEL_FRM | nsSelectionType::SEL_OLE | nsSelectionType::SEL_GRF) )
                    rWrtShell.SetFlyFrmAttr(*pTemplateItemSet);
                else
                {
                    rWrtShell.SetAttr(*pTemplateItemSet);

                    // additional numbering properties for paragraph styles
                    if( nSelectionType & nsSelectionType::SEL_TXT && rWrtShell.GetCurNumRule() )
                    {
                        if( SFX_ITEM_SET == pTemplateItemSet->GetItemState(FN_NUMBER_NEWSTART) )
                        {
                            sal_Bool bStart = ((SfxBoolItem&)pTemplateItemSet->Get(FN_NUMBER_NEWSTART)).GetValue();
                            sal_uInt16 nNumStart = USHRT_MAX;
                            if( SFX_ITEM_SET == pTemplateItemSet->GetItemState(FN_NUMBER_NEWSTART_AT) )
                            {
                                nNumStart = ((SfxUInt16Item&)pTemplateItemSet->Get(FN_NUMBER_NEWSTART_AT)).GetValue();
                                if(USHRT_MAX != nNumStart)
                                    bStart = sal_False;
                            }
                            rWrtShell.SetNumRuleStart(bStart);
                            rWrtShell.SetNodeNumStart(nNumStart);
                        }
                        else if( SFX_ITEM_SET == pTemplateItemSet->GetItemState(FN_NUMBER_NEWSTART_AT) )
                        {
                            sal_uInt16 nNumStart = ((SfxUInt16Item&)pTemplateItemSet->Get(FN_NUMBER_NEWSTART_AT)).GetValue();
                            rWrtShell.SetNodeNumStart(nNumStart);
                            rWrtShell.SetNumRuleStart(sal_False);
                        }
                    }
                }
                delete pTemplateItemSet;
            }
        }
    }

    if( m_pTableItemSet && nSelectionType & (nsSelectionType::SEL_TBL | nsSelectionType::SEL_TBL_CELLS) )
        lcl_setTableAttributes( *m_pTableItemSet, rWrtShell );

    rWrtShell.EndUndo(UNDO_INSATTR);
    rWrtShell.EndAction();

    if(!m_bPersistentCopy)
        this->Erase();
}

void SwFormatClipboard::Erase()
{
    m_nSelectionType = 0;
    if(m_pItemSet)
    {
        delete m_pItemSet;
        m_pItemSet = 0;
    }
    if(m_pTableItemSet)
    {
        delete m_pTableItemSet;
        m_pTableItemSet = 0;
    }
    if( m_aCharStyle.Len() )
        m_aCharStyle.Erase();
    if( m_aParaStyle.Len() )
        m_aParaStyle.Erase();

    m_bPersistentCopy = false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
