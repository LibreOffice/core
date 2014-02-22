/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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
    SfxItemSet* pItemSet = 0;
    if( nSelectionType & (nsSelectionType::SEL_FRM | nsSelectionType::SEL_OLE | nsSelectionType::SEL_GRF) )
    {
        pItemSet = new SfxItemSet(rPool,
                        FORMAT_PAINTBRUSH_FRAME_IDS
                        0);
    }
    else if( nSelectionType & nsSelectionType::SEL_DRW )
    {
        
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
    bool bBorder = ( SFX_ITEM_SET == rSet.GetItemState( RES_BOX ) ||
            SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_BORDER_INNER ) );
    bool bBackground = SFX_ITEM_SET == rSet.GetItemState( RES_BACKGROUND, false, &pItem );
    const SfxPoolItem* pRowItem = 0, *pTableItem = 0;
    bBackground |= SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_BRUSH_ROW, false, &pRowItem );
    bBackground |= SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_BRUSH_TABLE, false, &pTableItem );

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

    if( SFX_ITEM_SET == rSet.GetItemState( FN_PARAM_TABLE_HEADLINE, false, &pItem) )
        rSh.SetRowsToRepeat( ((SfxUInt16Item*)pItem)->GetValue() );

    SwFrmFmt* pFrmFmt = rSh.GetTableFmt();
    if(pFrmFmt)
    {
        
        pItem=0;
        rSet.GetItemState(rSet.GetPool()->GetWhich(RES_SHADOW), false, &pItem);
        if(pItem)
            pFrmFmt->SetFmtAttr( *pItem );

        
        pItem=0;
        rSet.GetItemState(rSet.GetPool()->GetWhich(RES_BREAK), false, &pItem);
        if(pItem)
            pFrmFmt->SetFmtAttr( *pItem );

        
        pItem=0;
        rSet.GetItemState(rSet.GetPool()->GetWhich(RES_PAGEDESC), false, &pItem);
        if(pItem)
            pFrmFmt->SetFmtAttr( *pItem );

        
        pItem=0;
        rSet.GetItemState(rSet.GetPool()->GetWhich(RES_LAYOUT_SPLIT), false, &pItem);
        if(pItem)
            pFrmFmt->SetFmtAttr( *pItem );

        
        pItem=0;
        rSet.GetItemState(rSet.GetPool()->GetWhich(RES_KEEP), false, &pItem);
        if(pItem)
            pFrmFmt->SetFmtAttr( *pItem );

        
        pItem=0;
        rSet.GetItemState(rSet.GetPool()->GetWhich(RES_FRAMEDIR), false, &pItem);
        if(pItem)
            pFrmFmt->SetFmtAttr( *pItem );
    }

    if( SFX_ITEM_SET == rSet.GetItemState( FN_TABLE_BOX_TEXTORIENTATION, false, &pItem) )
    {
        SvxFrameDirectionItem aDirection( FRMDIR_ENVIRONMENT, RES_FRAMEDIR );
        aDirection.SetValue(static_cast< const SvxFrameDirectionItem* >(pItem)->GetValue());
        rSh.SetBoxDirection(aDirection);
    }

    if( SFX_ITEM_SET == rSet.GetItemState( FN_TABLE_SET_VERT_ALIGN, false, &pItem))
        rSh.SetBoxAlign(((SfxUInt16Item*)(pItem))->GetValue());

    if( SFX_ITEM_SET == rSet.GetItemState( RES_ROW_SPLIT, false, &pItem) )
        rSh.SetRowSplit(*static_cast<const SwFmtRowSplit*>(pItem));
}
}

SwFormatClipboard::SwFormatClipboard()
        : m_nSelectionType(0)
        , m_pItemSet_TxtAttr(0)
        , m_pItemSet_ParAttr(0)
        , m_pTableItemSet(0)
        , m_bPersistentCopy(false)
{
}
SwFormatClipboard::~SwFormatClipboard()
{
    delete m_pItemSet_TxtAttr;
    delete m_pItemSet_ParAttr;
    delete m_pTableItemSet;
}

bool SwFormatClipboard::HasContent() const
{
    return m_pItemSet_TxtAttr!=0
        || m_pItemSet_ParAttr!=0
        || m_pTableItemSet != 0
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
    SfxItemSet* pItemSet_TxtAttr = lcl_CreateEmptyItemSet( nSelectionType, rPool, true );
    SfxItemSet* pItemSet_ParAttr = lcl_CreateEmptyItemSet( nSelectionType, rPool, false );

    rWrtShell.StartAction();
    rWrtShell.Push();

    
    
    
    if( nSelectionType == nsSelectionType::SEL_TXT )
    {
        
        
        
        SwPaM* pCrsr = rWrtShell.GetCrsr();

        sal_Bool bHasSelection = pCrsr->HasMark();
        bool bForwardSelection = false;

        if(!bHasSelection && pCrsr->GetPrev() != pCrsr && pCrsr->GetPrev() != 0)
        {
            

            
            rWrtShell.KillPams();

            
            pCrsr = rWrtShell.GetCrsr();
            bHasSelection = true;
        }

        bool dontMove = false;
        if (bHasSelection)
        {
            bForwardSelection = (*pCrsr->GetPoint()) > (*pCrsr->GetMark());

            
            pCrsr->DeleteMark();
            pCrsr->SetMark();
        }
        else
        {
            bool rightToLeft = rWrtShell.IsInRightToLeftText();
            
            
            if ( rWrtShell.IsEndPara() && !rightToLeft )
                dontMove = true;

            
            if ( rightToLeft )
            {
                if (pCrsr->GetPoint()->nContent == 0)
                    dontMove = true;
                else
                    bForwardSelection = !bForwardSelection;
            }
        }

        
        if (!dontMove)
            pCrsr->Move( bForwardSelection ? fnMoveBackward : fnMoveForward );
    }

    if(pItemSet_TxtAttr)
    {
        if( nSelectionType & (nsSelectionType::SEL_FRM | nsSelectionType::SEL_OLE | nsSelectionType::SEL_GRF) )
            rWrtShell.GetFlyFrmAttr(*pItemSet_TxtAttr);
        else
        {
            
            rWrtShell.GetCurAttr(*pItemSet_TxtAttr);

            if( nSelectionType & nsSelectionType::SEL_TXT )
            {
                
                
                rWrtShell.GetCurParAttr(*pItemSet_ParAttr);
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
                pItemSet_TxtAttr = new SfxItemSet( pDrawView->GetAttrFromMarked(bOnlyHardAttr) );
                
                pItemSet_TxtAttr->ClearItem(SDRATTR_CUSTOMSHAPE_ENGINE);
                pItemSet_TxtAttr->ClearItem(SDRATTR_CUSTOMSHAPE_DATA);
                pItemSet_TxtAttr->ClearItem(SDRATTR_CUSTOMSHAPE_GEOMETRY);
                pItemSet_TxtAttr->ClearItem(SDRATTR_CUSTOMSHAPE_REPLACEMENT_URL);
            }
        }
    }

    if( nSelectionType & nsSelectionType::SEL_TBL_CELLS )
    {
        m_pTableItemSet = new SfxItemSet(rPool,
                        SID_ATTR_BORDER_INNER,  SID_ATTR_BORDER_SHADOW, 
                        RES_BACKGROUND,         RES_SHADOW, 
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
    m_pItemSet_TxtAttr = pItemSet_TxtAttr;
    m_pItemSet_ParAttr = pItemSet_ParAttr;

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

static void lcl_AppendSetItems( ItemVector& rItemVector, const SfxItemSet& rStyleAttrSet )
{
    const sal_uInt16*  pRanges = rStyleAttrSet.GetRanges();
    while( *pRanges )
    {
        for ( sal_uInt16 nWhich = *pRanges; nWhich <= *(pRanges+1); ++nWhich )
        {
            const SfxPoolItem* pItem;
            if( SFX_ITEM_SET == rStyleAttrSet.GetItemState( nWhich, false, &pItem ) )
            {
                rItemVector.push_back( SfxPoolItemSharedPtr( pItem->Clone() ) );
            }
        }
        pRanges += 2;
    }
}

static void lcl_RemoveEqualItems( SfxItemSet& rTemplateItemSet, const ItemVector& rItemVector )
{
    ItemVector::const_iterator aEnd = rItemVector.end();
    ItemVector::const_iterator aIter = rItemVector.begin();
    while( aIter != aEnd )
    {
        const SfxPoolItem* pItem;
        if( SFX_ITEM_SET == rTemplateItemSet.GetItemState( (*aIter)->Which(), true, &pItem ) &&
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
        
        if( pPool )
        {
            
            if(!m_aCharStyle.isEmpty() && !bNoCharacterFormats )
            {
                
                SwDocStyleSheet* pStyle = static_cast<SwDocStyleSheet*>(pPool->Find(m_aCharStyle, SFX_STYLE_FAMILY_CHAR));

                
                if( pStyle )
                {
                    SwFmtCharFmt aFmt(pStyle->GetCharFmt());
                    
                    
                    lcl_AppendSetItems( aItemVector, aFmt.GetCharFmt()->GetAttrSet());

                    
                    rWrtShell.SetAttrItem( aFmt );
                }
            }

            
            if(!m_aParaStyle.isEmpty() && !bNoParagraphFormats )
            {
                
                SwDocStyleSheet* pStyle = static_cast<SwDocStyleSheet*>(pPool->Find(m_aParaStyle, SFX_STYLE_FAMILY_PARA));
                if( pStyle )
                {
                    
                    
                    lcl_AppendSetItems( aItemVector, pStyle->GetCollection()->GetAttrSet());

                    
                    rWrtShell.SetTxtFmtColl( pStyle->GetCollection() );
                }
            }
        }

        
        if ( m_pItemSet_ParAttr && m_pItemSet_ParAttr->Count() != 0 && !bNoParagraphFormats )
        {
            
            SfxItemSet* pTemplateItemSet = lcl_CreateEmptyItemSet(
                    nSelectionType, *m_pItemSet_ParAttr->GetPool(), false);
            
            

            pTemplateItemSet->Put( *m_pItemSet_ParAttr );

            
            lcl_RemoveEqualItems( *pTemplateItemSet, aItemVector );

            
            rWrtShell.SetAttrSet(*pTemplateItemSet);

            
            
            lcl_AppendSetItems( aItemVector, *pTemplateItemSet);

            delete pTemplateItemSet;
        }
    }

    if(m_pItemSet_TxtAttr)
    {
        if( nSelectionType & nsSelectionType::SEL_DRW )
        {
            SdrView* pDrawView = rWrtShell.GetDrawView();
            if(pDrawView)
            {
                sal_Bool bReplaceAll = sal_True;
                pDrawView->SetAttrToMarked(*m_pItemSet_TxtAttr, bReplaceAll);
            }
        }
        else
        {
            
            SfxItemSet* pTemplateItemSet = lcl_CreateEmptyItemSet(
                    nSelectionType, *m_pItemSet_TxtAttr->GetPool(), true );

            if(pTemplateItemSet)
            {
                
                pTemplateItemSet->Put( *m_pItemSet_TxtAttr );

                
                
                lcl_RemoveEqualItems( *pTemplateItemSet, aItemVector );

                
                if( nSelectionType & (nsSelectionType::SEL_FRM | nsSelectionType::SEL_OLE | nsSelectionType::SEL_GRF) )
                    rWrtShell.SetFlyFrmAttr(*pTemplateItemSet);
                else if ( !bNoCharacterFormats )
                    rWrtShell.SetAttrSet(*pTemplateItemSet);

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

    delete m_pItemSet_TxtAttr;
    m_pItemSet_TxtAttr = 0;

    delete m_pItemSet_ParAttr;
    m_pItemSet_ParAttr = 0;

    delete m_pTableItemSet;
    m_pTableItemSet = 0;

    if( !m_aCharStyle.isEmpty() )
        m_aCharStyle = "";
    if( !m_aParaStyle.isEmpty() )
        m_aParaStyle = "";

    m_bPersistentCopy = false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
