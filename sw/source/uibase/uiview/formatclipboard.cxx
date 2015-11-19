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

#include "formatclipboard.hxx"

#include <svx/svxids.hrc>
#include <cmdid.h>
#include <charfmt.hxx>
#include <frmfmt.hxx>
#include <docstyle.hxx>
#include <fchrfmt.hxx>
#include <svx/svdview.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/formatbreakitem.hxx>
#include <fmtlsplt.hxx>
#include <editeng/keepitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <fmtpdsc.hxx>
#include <fmtrowsplt.hxx>

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
FORMAT_PAINTBRUSH_FRAME_IDS

SfxItemSet* lcl_CreateEmptyItemSet( int nSelectionType, SfxItemPool& rPool, bool bNoParagraphFormats = false )
{
    SfxItemSet* pItemSet = nullptr;
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
    else if( nSelectionType & nsSelectionType::SEL_TXT )
    {
        if( bNoParagraphFormats )
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
        rSet.Put(aBoxDirection, FN_TABLE_BOX_TEXTORIENTATION);

    rSet.Put(SfxUInt16Item(FN_TABLE_SET_VERT_ALIGN, rSh.GetBoxAlign()));

    rSet.Put( SfxUInt16Item( FN_PARAM_TABLE_HEADLINE, rSh.GetRowsToRepeat() ) );

    SwFrameFormat *pFrameFormat = rSh.GetTableFormat();
    if(pFrameFormat)
    {
        rSet.Put( pFrameFormat->GetShadow() );
        rSet.Put( pFrameFormat->GetBreak() );
        rSet.Put( pFrameFormat->GetPageDesc() );
        rSet.Put( pFrameFormat->GetLayoutSplit() );
        rSet.Put( pFrameFormat->GetKeep() );
        rSet.Put( pFrameFormat->GetFrameDir() );
    }

    SwFormatRowSplit* pSplit = nullptr;
    rSh.GetRowSplit(pSplit);
    if(pSplit)
        rSet.Put(*pSplit);
}

void lcl_setTableAttributes( const SfxItemSet& rSet, SwWrtShell &rSh )
{
    const SfxPoolItem* pItem = nullptr;
    bool bBorder = ( SfxItemState::SET == rSet.GetItemState( RES_BOX ) ||
            SfxItemState::SET == rSet.GetItemState( SID_ATTR_BORDER_INNER ) );
    bool bBackground = SfxItemState::SET == rSet.GetItemState( RES_BACKGROUND, false, &pItem );
    const SfxPoolItem* pRowItem = nullptr, *pTableItem = nullptr;
    bBackground |= SfxItemState::SET == rSet.GetItemState( SID_ATTR_BRUSH_ROW, false, &pRowItem );
    bBackground |= SfxItemState::SET == rSet.GetItemState( SID_ATTR_BRUSH_TABLE, false, &pTableItem );

    if(bBackground)
    {
        if(pItem)
            rSh.SetBoxBackground( *static_cast<const SvxBrushItem*>(pItem) );
        if(pRowItem)
        {
            SvxBrushItem aBrush(*static_cast<const SvxBrushItem*>(pRowItem));
            aBrush.SetWhich(RES_BACKGROUND);
            rSh.SetRowBackground(aBrush);
        }
        if(pTableItem)
        {
            SvxBrushItem aBrush(*static_cast<const SvxBrushItem*>(pTableItem));
            aBrush.SetWhich(RES_BACKGROUND);
            rSh.SetTabBackground( aBrush );
        }
    }
    if(bBorder)
        rSh.SetTabBorders( rSet );

    if( SfxItemState::SET == rSet.GetItemState( FN_PARAM_TABLE_HEADLINE, false, &pItem) )
        rSh.SetRowsToRepeat( static_cast<const SfxUInt16Item*>(pItem)->GetValue() );

    SwFrameFormat* pFrameFormat = rSh.GetTableFormat();
    if(pFrameFormat)
    {
        //RES_SHADOW
        pItem=nullptr;
        rSet.GetItemState(rSet.GetPool()->GetWhich(RES_SHADOW), false, &pItem);
        if(pItem)
            pFrameFormat->SetFormatAttr( *pItem );

        //RES_BREAK
        pItem=nullptr;
        rSet.GetItemState(rSet.GetPool()->GetWhich(RES_BREAK), false, &pItem);
        if(pItem)
            pFrameFormat->SetFormatAttr( *pItem );

        //RES_PAGEDESC
        pItem=nullptr;
        rSet.GetItemState(rSet.GetPool()->GetWhich(RES_PAGEDESC), false, &pItem);
        if(pItem)
            pFrameFormat->SetFormatAttr( *pItem );

        //RES_LAYOUT_SPLIT
        pItem=nullptr;
        rSet.GetItemState(rSet.GetPool()->GetWhich(RES_LAYOUT_SPLIT), false, &pItem);
        if(pItem)
            pFrameFormat->SetFormatAttr( *pItem );

        //RES_KEEP
        pItem=nullptr;
        rSet.GetItemState(rSet.GetPool()->GetWhich(RES_KEEP), false, &pItem);
        if(pItem)
            pFrameFormat->SetFormatAttr( *pItem );

        //RES_FRAMEDIR
        pItem=nullptr;
        rSet.GetItemState(rSet.GetPool()->GetWhich(RES_FRAMEDIR), false, &pItem);
        if(pItem)
            pFrameFormat->SetFormatAttr( *pItem );
    }

    if( SfxItemState::SET == rSet.GetItemState( FN_TABLE_BOX_TEXTORIENTATION, false, &pItem) )
    {
        SvxFrameDirectionItem aDirection( FRMDIR_ENVIRONMENT, RES_FRAMEDIR );
        aDirection.SetValue(static_cast< const SvxFrameDirectionItem* >(pItem)->GetValue());
        rSh.SetBoxDirection(aDirection);
    }

    if( SfxItemState::SET == rSet.GetItemState( FN_TABLE_SET_VERT_ALIGN, false, &pItem))
        rSh.SetBoxAlign(static_cast<const SfxUInt16Item*>((pItem))->GetValue());

    if( SfxItemState::SET == rSet.GetItemState( RES_ROW_SPLIT, false, &pItem) )
        rSh.SetRowSplit(*static_cast<const SwFormatRowSplit*>(pItem));
}
}//end anonymous namespace

SwFormatClipboard::SwFormatClipboard()
        : m_nSelectionType(0)
        , m_pItemSet_TextAttr(nullptr)
        , m_pItemSet_ParAttr(nullptr)
        , m_pTableItemSet(nullptr)
        , m_bPersistentCopy(false)
{
}
SwFormatClipboard::~SwFormatClipboard()
{
    delete m_pItemSet_TextAttr;
    delete m_pItemSet_ParAttr;
    delete m_pTableItemSet;
}

bool SwFormatClipboard::HasContent() const
{
    return m_pItemSet_TextAttr!=nullptr
        || m_pItemSet_ParAttr!=nullptr
        || m_pTableItemSet != nullptr
        || !m_aCharStyle.isEmpty()
        || !m_aParaStyle.isEmpty()
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

bool SwFormatClipboard::CanCopyThisType( int nSelectionType )
{
    if( nSelectionType & (nsSelectionType::SEL_FRM | nsSelectionType::SEL_OLE | nsSelectionType::SEL_GRF
         | nsSelectionType::SEL_TXT | nsSelectionType::SEL_DRW | nsSelectionType::SEL_TBL | nsSelectionType::SEL_TBL_CELLS ) )
         return true;
    return false;
}

void SwFormatClipboard::Copy( SwWrtShell& rWrtShell, SfxItemPool& rPool, bool bPersistentCopy )
{
    // first clear the previously stored attributes
    this->Erase();
    m_bPersistentCopy = bPersistentCopy;

    int nSelectionType = rWrtShell.GetSelectionType();
    SfxItemSet* pItemSet_TextAttr = lcl_CreateEmptyItemSet( nSelectionType, rPool, true );
    SfxItemSet* pItemSet_ParAttr = lcl_CreateEmptyItemSet( nSelectionType, rPool );

    rWrtShell.StartAction();
    rWrtShell.Push();

    // modify the "Point and Mark" of the cursor
    // in order to select only the last character of the
    // selection(s) and then to get the attributes of this single character
    if( nSelectionType == nsSelectionType::SEL_TXT )
    {
        // get the current PaM, the cursor
        // if there several selection it currently point
        // on the last (sort by there creation time) selection
        SwPaM* pCursor = rWrtShell.GetCursor();

        bool bHasSelection = pCursor->HasMark();
        bool bForwardSelection = false;

        if(!bHasSelection && pCursor->IsMultiSelection())
        {
            // if cursor has multiple selections

            // clear all the selections except the last
            rWrtShell.KillPams();

            // reset the cursor to the remaining selection
            pCursor = rWrtShell.GetCursor();
            bHasSelection = true;
        }

        bool dontMove = false;
        if (bHasSelection)
        {
            bForwardSelection = (*pCursor->GetPoint()) > (*pCursor->GetMark());

            // clear the selection leaving just the cursor
            pCursor->DeleteMark();
            pCursor->SetMark();
        }
        else
        {
            bool rightToLeft = rWrtShell.IsInRightToLeftText();
            // if there were no selection (only a cursor) and the cursor was at
            // the end of the paragraph then don't move
            if ( rWrtShell.IsEndPara() && !rightToLeft )
                dontMove = true;

            // revert left and right
            if ( rightToLeft )
            {
                if (pCursor->GetPoint()->nContent == 0)
                    dontMove = true;
                else
                    bForwardSelection = !bForwardSelection;
            }
        }

        // move the cursor in order to select one character
        if (!dontMove)
            pCursor->Move( bForwardSelection ? fnMoveBackward : fnMoveForward );
    }

    if(pItemSet_TextAttr)
    {
        if( nSelectionType & (nsSelectionType::SEL_FRM | nsSelectionType::SEL_OLE | nsSelectionType::SEL_GRF) )
            rWrtShell.GetFlyFrameAttr(*pItemSet_TextAttr);
        else
        {
            // get the text attributes from named and automatic formatting
            rWrtShell.GetCurAttr(*pItemSet_TextAttr);

            if( nSelectionType & nsSelectionType::SEL_TXT )
            {
                // get the paragraph attributes (could be character properties)
                // from named and automatic formatting
                rWrtShell.GetCurParAttr(*pItemSet_ParAttr);
            }
        }
    }
    else if ( nSelectionType & nsSelectionType::SEL_DRW )
    {
        SdrView* pDrawView = rWrtShell.GetDrawView();
        if(pDrawView)
        {
            if( pDrawView->AreObjectsMarked() )
            {
                bool bOnlyHardAttr = true;
                pItemSet_TextAttr = new SfxItemSet( pDrawView->GetAttrFromMarked(bOnlyHardAttr) );
                //remove attributes defining the type/data of custom shapes
                pItemSet_TextAttr->ClearItem(SDRATTR_CUSTOMSHAPE_ENGINE);
                pItemSet_TextAttr->ClearItem(SDRATTR_CUSTOMSHAPE_DATA);
                pItemSet_TextAttr->ClearItem(SDRATTR_CUSTOMSHAPE_GEOMETRY);
                pItemSet_TextAttr->ClearItem(SDRATTR_CUSTOMSHAPE_REPLACEMENT_URL);
            }
        }
    }

    if( nSelectionType & nsSelectionType::SEL_TBL_CELLS )//only copy table attributes if really cells are selected (not only text in tables)
    {
        m_pTableItemSet = new SfxItemSet(rPool,
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
                        FN_TABLE_BOX_TEXTORIENTATION, FN_TABLE_BOX_TEXTORIENTATION,
                        FN_TABLE_SET_VERT_ALIGN, FN_TABLE_SET_VERT_ALIGN,
                        0);
        lcl_getTableAttributes( *m_pTableItemSet, rWrtShell );
    }

    m_nSelectionType = nSelectionType;
    m_pItemSet_TextAttr = pItemSet_TextAttr;
    m_pItemSet_ParAttr = pItemSet_ParAttr;

    if( nSelectionType & nsSelectionType::SEL_TXT )
    {
        // if text is selected save the named character format
        SwFormat* pFormat = rWrtShell.GetCurCharFormat();
        if( pFormat )
            m_aCharStyle = pFormat->GetName();

        // and the named paragraph format
        pFormat = rWrtShell.GetCurTextFormatColl();
        if( pFormat )
            m_aParaStyle = pFormat->GetName();
    }

    rWrtShell.Pop(false);
    rWrtShell.EndAction();
}

typedef std::shared_ptr< SfxPoolItem > SfxPoolItemSharedPtr;
typedef std::vector< SfxPoolItemSharedPtr > ItemVector;
// collect all PoolItems from the applied styles
static void lcl_AppendSetItems( ItemVector& rItemVector, const SfxItemSet& rStyleAttrSet )
{
    const sal_uInt16*  pRanges = rStyleAttrSet.GetRanges();
    while( *pRanges )
    {
        for ( sal_uInt16 nWhich = *pRanges; nWhich <= *(pRanges+1); ++nWhich )
        {
            const SfxPoolItem* pItem;
            if( SfxItemState::SET == rStyleAttrSet.GetItemState( nWhich, false, &pItem ) )
            {
                rItemVector.push_back( SfxPoolItemSharedPtr( pItem->Clone() ) );
            }
        }
        pRanges += 2;
    }
}
// remove all items that are inherited from the styles
static void lcl_RemoveEqualItems( SfxItemSet& rTemplateItemSet, const ItemVector& rItemVector )
{
    ItemVector::const_iterator aEnd = rItemVector.end();
    ItemVector::const_iterator aIter = rItemVector.begin();
    while( aIter != aEnd )
    {
        const SfxPoolItem* pItem;
        if( SfxItemState::SET == rTemplateItemSet.GetItemState( (*aIter)->Which(), true, &pItem ) &&
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

    if( nSelectionType & nsSelectionType::SEL_TXT )
    {
        // apply the named text and paragraph formatting
        if( pPool )
        {
            // if there is a named text format recorded and the user wants to apply it
            if(!m_aCharStyle.isEmpty() && !bNoCharacterFormats )
            {
                // look for the named text format in the pool
                SwDocStyleSheet* pStyle = static_cast<SwDocStyleSheet*>(pPool->Find(m_aCharStyle, SFX_STYLE_FAMILY_CHAR));

                // if the style is found
                if( pStyle )
                {
                    SwFormatCharFormat aFormat(pStyle->GetCharFormat());
                    // store the attributes from this style in aItemVector in order
                    // not to apply them as automatic formatting attributes later in the code
                    lcl_AppendSetItems( aItemVector, aFormat.GetCharFormat()->GetAttrSet());

                    // apply the named format
                    rWrtShell.SetAttrItem( aFormat );
                }
            }

            // if there is a named paragraph format recorded and the user wants to apply it
            if(!m_aParaStyle.isEmpty() && !bNoParagraphFormats )
            {
                // look for the named pragraph format in the pool
                SwDocStyleSheet* pStyle = static_cast<SwDocStyleSheet*>(pPool->Find(m_aParaStyle, SFX_STYLE_FAMILY_PARA));
                if( pStyle )
                {
                    // store the attributes from this style in aItemVector in order
                    // not to apply them as automatic formatting attributes later in the code
                    lcl_AppendSetItems( aItemVector, pStyle->GetCollection()->GetAttrSet());

                    // apply the named format
                    rWrtShell.SetTextFormatColl( pStyle->GetCollection() );
                }
            }
        }

        // apply the paragraph automatic attributes
        if ( m_pItemSet_ParAttr && m_pItemSet_ParAttr->Count() != 0 && !bNoParagraphFormats )
        {
            // temporary SfxItemSet
            std::unique_ptr<SfxItemSet> pTemplateItemSet(lcl_CreateEmptyItemSet(
                    nSelectionType, *m_pItemSet_ParAttr->GetPool()));
            // no need to verify the existence of pTemplateItemSet as we
            // know that here the selection type is SEL_TXT

            pTemplateItemSet->Put( *m_pItemSet_ParAttr );

            // remove attribute that were applied by named text and paragraph formatting
            lcl_RemoveEqualItems( *pTemplateItemSet, aItemVector );

            // apply the paragraph automatic attributes to all the nodes in the selection
            rWrtShell.SetAttrSet(*pTemplateItemSet);

            // store the attributes in aItemVector in order not to apply them as
            // text automatic formatting attributes later in the code
            lcl_AppendSetItems( aItemVector, *pTemplateItemSet);
        }
    }

    if(m_pItemSet_TextAttr)
    {
        if( nSelectionType & nsSelectionType::SEL_DRW )
        {
            SdrView* pDrawView = rWrtShell.GetDrawView();
            if(pDrawView)
            {
                bool bReplaceAll = true;
                pDrawView->SetAttrToMarked(*m_pItemSet_TextAttr, bReplaceAll);
            }
        }
        else
        {
            // temporary SfxItemSet
            std::unique_ptr<SfxItemSet> pTemplateItemSet(lcl_CreateEmptyItemSet(
                    nSelectionType, *m_pItemSet_TextAttr->GetPool(), true ));

            if(pTemplateItemSet)
            {
                // copy the stored automatic text attributes in a temporary SfxItemSet
                pTemplateItemSet->Put( *m_pItemSet_TextAttr );

                // only attributes that were not apply by named style attributes and automatic
                // paragraph attributes should be applied
                lcl_RemoveEqualItems( *pTemplateItemSet, aItemVector );

                // apply the character automatic attributes
                if( nSelectionType & (nsSelectionType::SEL_FRM | nsSelectionType::SEL_OLE | nsSelectionType::SEL_GRF) )
                    rWrtShell.SetFlyFrameAttr(*pTemplateItemSet);
                else if ( !bNoCharacterFormats )
                    rWrtShell.SetAttrSet(*pTemplateItemSet);
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

    delete m_pItemSet_TextAttr;
    m_pItemSet_TextAttr = nullptr;

    delete m_pItemSet_ParAttr;
    m_pItemSet_ParAttr = nullptr;

    delete m_pTableItemSet;
    m_pTableItemSet = nullptr;

    if( !m_aCharStyle.isEmpty() )
        m_aCharStyle.clear();
    if( !m_aParaStyle.isEmpty() )
        m_aParaStyle.clear();

    m_bPersistentCopy = false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
