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


#include <algorithm>
#include <editeng/eeitem.hxx>
#include <com/sun/star/i18n/WordType.hpp>

#include <svl/itemset.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>
#include <editeng/unoedhlp.hxx>
#include <editeng/editdata.hxx>
#include <editeng/outliner.hxx>
#include <editeng/editobj.hxx>      // only for the GetText crutch

#include <editeng/unofored.hxx>

using namespace ::com::sun::star;

//------------------------------------------------------------------------

SvxEditEngineForwarder::SvxEditEngineForwarder( EditEngine& rEngine ) :
    rEditEngine( rEngine )
{
}

SvxEditEngineForwarder::~SvxEditEngineForwarder()
{
    // the EditEngine may need to be deleted from the outside
}

sal_Int32 SvxEditEngineForwarder::GetParagraphCount() const
{
    return rEditEngine.GetParagraphCount();
}

sal_uInt16 SvxEditEngineForwarder::GetTextLen( sal_Int32 nParagraph ) const
{
    return rEditEngine.GetTextLen( nParagraph );
}

OUString SvxEditEngineForwarder::GetText( const ESelection& rSel ) const
{
    return convertLineEnd(rEditEngine.GetText(rSel, LINEEND_LF), GetSystemLineEnd());
}

SfxItemSet SvxEditEngineForwarder::GetAttribs( const ESelection& rSel, sal_Bool bOnlyHardAttrib ) const
{
    if( rSel.nStartPara == rSel.nEndPara )
    {
        sal_uInt8 nFlags = 0;
        switch( bOnlyHardAttrib )
        {
        case EditEngineAttribs_All:
            nFlags = GETATTRIBS_ALL;
            break;
        case EditEngineAttribs_HardAndPara:
            nFlags = GETATTRIBS_PARAATTRIBS|GETATTRIBS_CHARATTRIBS;
            break;
        case EditEngineAttribs_OnlyHard:
            nFlags = GETATTRIBS_CHARATTRIBS;
            break;
        default:
            OSL_FAIL("unknown flags for SvxOutlinerForwarder::GetAttribs");
        }

        return rEditEngine.GetAttribs( rSel.nStartPara, rSel.nStartPos, rSel.nEndPos, nFlags );
    }
    else
    {
        return rEditEngine.GetAttribs( rSel, bOnlyHardAttrib );
    }
}

SfxItemSet SvxEditEngineForwarder::GetParaAttribs( sal_Int32 nPara ) const
{
    SfxItemSet aSet( rEditEngine.GetParaAttribs( nPara ) );

    sal_uInt16 nWhich = EE_PARA_START;
    while( nWhich <= EE_PARA_END )
    {
        if( aSet.GetItemState( nWhich, sal_True ) != SFX_ITEM_ON )
        {
            if( rEditEngine.HasParaAttrib( nPara, nWhich ) )
                aSet.Put( rEditEngine.GetParaAttrib( nPara, nWhich ) );
        }
        nWhich++;
    }

    return aSet;
}

void SvxEditEngineForwarder::SetParaAttribs( sal_Int32 nPara, const SfxItemSet& rSet )
{
    rEditEngine.SetParaAttribs( nPara, rSet );
}

void SvxEditEngineForwarder::RemoveAttribs( const ESelection& rSelection, sal_Bool bRemoveParaAttribs, sal_uInt16 nWhich )
{
    rEditEngine.RemoveAttribs( rSelection, bRemoveParaAttribs, nWhich );
}

SfxItemPool* SvxEditEngineForwarder::GetPool() const
{
    return rEditEngine.GetEmptyItemSet().GetPool();
}

void SvxEditEngineForwarder::GetPortions( sal_Int32 nPara, std::vector<sal_uInt16>& rList ) const
{
    rEditEngine.GetPortions( nPara, rList );
}

void SvxEditEngineForwarder::QuickInsertText( const OUString& rText, const ESelection& rSel )
{
    rEditEngine.QuickInsertText( rText, rSel );
}

void SvxEditEngineForwarder::QuickInsertLineBreak( const ESelection& rSel )
{
    rEditEngine.QuickInsertLineBreak( rSel );
}

void SvxEditEngineForwarder::QuickInsertField( const SvxFieldItem& rFld, const ESelection& rSel )
{
    rEditEngine.QuickInsertField( rFld, rSel );
}

void SvxEditEngineForwarder::QuickSetAttribs( const SfxItemSet& rSet, const ESelection& rSel )
{
    rEditEngine.QuickSetAttribs( rSet, rSel );
}

sal_Bool SvxEditEngineForwarder::IsValid() const
{
    // cannot reliably query EditEngine state
    // while in the middle of an update
    return rEditEngine.GetUpdateMode();
}

OUString SvxEditEngineForwarder::CalcFieldValue( const SvxFieldItem& rField, sal_Int32 nPara, sal_uInt16 nPos, Color*& rpTxtColor, Color*& rpFldColor )
{
    return rEditEngine.CalcFieldValue( rField, nPara, nPos, rpTxtColor, rpFldColor );
}

void SvxEditEngineForwarder::FieldClicked( const SvxFieldItem& rField, sal_Int32 nPara, xub_StrLen nPos )
{
    rEditEngine.FieldClicked( rField, nPara, nPos );
}

sal_uInt16 GetSvxEditEngineItemState( EditEngine& rEditEngine, const ESelection& rSel, sal_uInt16 nWhich )
{
    std::vector<EECharAttrib> aAttribs;

    const SfxPoolItem*  pLastItem = NULL;

    SfxItemState eState = SFX_ITEM_DEFAULT;

    // check all paragraphs inside the selection
    for( sal_Int32 nPara = rSel.nStartPara; nPara <= rSel.nEndPara; nPara++ )
    {
        SfxItemState eParaState = SFX_ITEM_DEFAULT;

        // calculate start and endpos for this paragraph
        sal_uInt16 nPos = 0;
        if( rSel.nStartPara == nPara )
            nPos = rSel.nStartPos;

        sal_uInt16 nEndPos = rSel.nEndPos;
        if( rSel.nEndPara != nPara )
            nEndPos = rEditEngine.GetTextLen( nPara );


        // get list of char attribs
        rEditEngine.GetCharAttribs( nPara, aAttribs );

        sal_Bool bEmpty = sal_True;     // we found no item inside the selektion of this paragraph
        sal_Bool bGaps  = sal_False;    // we found items but theire gaps between them
        sal_uInt16 nLastEnd = nPos;

        const SfxPoolItem* pParaItem = NULL;

        for(std::vector<EECharAttrib>::const_iterator i = aAttribs.begin(); i < aAttribs.end(); ++i)
        {
            DBG_ASSERT(i->pAttr, "GetCharAttribs gives corrupt data");

            const sal_Bool bEmptyPortion = i->nStart == i->nEnd;
            if((!bEmptyPortion && i->nStart >= nEndPos) ||
               (bEmptyPortion && i->nStart > nEndPos))
                break;  // break if we are already behind our selektion

            if((!bEmptyPortion && i->nEnd <= nPos) ||
               (bEmptyPortion && i->nEnd < nPos))
                continue;   // or if the attribute ends before our selektion

            if(i->pAttr->Which() != nWhich)
                continue; // skip if is not the searched item

            // if we already found an item
            if( pParaItem )
            {
                // ... and its different to this one than the state is dont care
                if(*pParaItem != *(i->pAttr))
                    return SFX_ITEM_DONTCARE;
            }
            else
                pParaItem = i->pAttr;

            if( bEmpty )
                bEmpty = sal_False;

            if(!bGaps && i->nStart > nLastEnd)
                bGaps = sal_True;

            nLastEnd = i->nEnd;
        }

        if( !bEmpty && !bGaps && nLastEnd < ( nEndPos - 1 ) )
            bGaps = sal_True;

        if( bEmpty )
            eParaState = SFX_ITEM_DEFAULT;
        else if( bGaps )
            eParaState = SFX_ITEM_DONTCARE;
        else
            eParaState = SFX_ITEM_SET;

        // if we already found an item check if we found the same
        if( pLastItem )
        {
            if( (pParaItem == NULL) || (*pLastItem != *pParaItem) )
                return SFX_ITEM_DONTCARE;
        }
        else
        {
            pLastItem = pParaItem;
            eState = eParaState;
        }
    }

    return eState;
}

sal_uInt16 SvxEditEngineForwarder::GetItemState( const ESelection& rSel, sal_uInt16 nWhich ) const
{
    return GetSvxEditEngineItemState( rEditEngine, rSel, nWhich );
}

sal_uInt16 SvxEditEngineForwarder::GetItemState( sal_Int32 nPara, sal_uInt16 nWhich ) const
{
    const SfxItemSet& rSet = rEditEngine.GetParaAttribs( nPara );
    return rSet.GetItemState( nWhich );
}

LanguageType SvxEditEngineForwarder::GetLanguage( sal_Int32 nPara, sal_uInt16 nIndex ) const
{
    return rEditEngine.GetLanguage(nPara, nIndex);
}

sal_uInt16 SvxEditEngineForwarder::GetFieldCount( sal_Int32 nPara ) const
{
    return rEditEngine.GetFieldCount(nPara);
}

EFieldInfo SvxEditEngineForwarder::GetFieldInfo( sal_Int32 nPara, sal_uInt16 nField ) const
{
    return rEditEngine.GetFieldInfo( nPara, nField );
}

EBulletInfo SvxEditEngineForwarder::GetBulletInfo( sal_Int32 ) const
{
    return EBulletInfo();
}

Rectangle SvxEditEngineForwarder::GetCharBounds( sal_Int32 nPara, sal_uInt16 nIndex ) const
{
    // #101701#
    // EditEngine's 'internal' methods like GetCharacterBounds()
    // don't rotate for vertical text.
    Size aSize( rEditEngine.CalcTextWidth(), rEditEngine.GetTextHeight() );
    ::std::swap( aSize.Width(), aSize.Height() );
    bool bIsVertical( rEditEngine.IsVertical() == sal_True );

    // #108900# Handle virtual position one-past-the end of the string
    if( nIndex >= rEditEngine.GetTextLen(nPara) )
    {
        Rectangle aLast;

        if( nIndex )
        {
            // use last character, if possible
            aLast = rEditEngine.GetCharacterBounds( EPosition(nPara, nIndex-1) );

            // move at end of this last character, make one pixel wide
            aLast.Move( aLast.Right() - aLast.Left(), 0 );
            aLast.SetSize( Size(1, aLast.GetHeight()) );

            // take care for CTL
            aLast = SvxEditSourceHelper::EEToUserSpace( aLast, aSize, bIsVertical );
        }
        else
        {
            // #109864# Bounds must lie within the paragraph
            aLast = GetParaBounds( nPara );

            // #109151# Don't use paragraph height, but line height
            // instead. aLast is already CTL-correct
            if( bIsVertical)
                aLast.SetSize( Size( rEditEngine.GetLineHeight(nPara,0), 1 ) );
            else
                aLast.SetSize( Size( 1, rEditEngine.GetLineHeight(nPara,0) ) );
        }

        return aLast;
    }
    else
    {
        return SvxEditSourceHelper::EEToUserSpace( rEditEngine.GetCharacterBounds( EPosition(nPara, nIndex) ),
                                                   aSize, bIsVertical );
    }
}

Rectangle SvxEditEngineForwarder::GetParaBounds( sal_Int32 nPara ) const
{
    const Point aPnt = rEditEngine.GetDocPosTopLeft( nPara );
    sal_uLong nWidth;
    sal_uLong nHeight;
    sal_uLong nTextWidth;

    if( rEditEngine.IsVertical() )
    {
        // #101701#
        // Hargl. EditEngine's 'external' methods return the rotated
        // dimensions, 'internal' methods like GetTextHeight( n )
        // don't rotate.
        nWidth = rEditEngine.GetTextHeight( nPara );
        nHeight = rEditEngine.GetTextHeight();
        nTextWidth = rEditEngine.GetTextHeight();

        return Rectangle( nTextWidth - aPnt.Y() - nWidth, 0, nTextWidth - aPnt.Y(), nHeight );
    }
    else
    {
        nWidth = rEditEngine.CalcTextWidth();
        nHeight = rEditEngine.GetTextHeight( nPara );

        return Rectangle( 0, aPnt.Y(), nWidth, aPnt.Y() + nHeight );
    }
}

MapMode SvxEditEngineForwarder::GetMapMode() const
{
    return rEditEngine.GetRefMapMode();
}

OutputDevice* SvxEditEngineForwarder::GetRefDevice() const
{
    return rEditEngine.GetRefDevice();
}

sal_Bool SvxEditEngineForwarder::GetIndexAtPoint( const Point& rPos, sal_Int32& nPara, sal_uInt16& nIndex ) const
{
    Size aSize( rEditEngine.CalcTextWidth(), rEditEngine.GetTextHeight() );
    ::std::swap( aSize.Width(), aSize.Height() );
    Point aEEPos( SvxEditSourceHelper::UserSpaceToEE( rPos,
                                                      aSize,
                                                      rEditEngine.IsVertical() == sal_True ));

    EPosition aDocPos = rEditEngine.FindDocPosition( aEEPos );

    nPara = aDocPos.nPara;
    nIndex = aDocPos.nIndex;

    return sal_True;
}

sal_Bool SvxEditEngineForwarder::GetWordIndices( sal_Int32 nPara, sal_uInt16 nIndex, sal_uInt16& nStart, sal_uInt16& nEnd ) const
{
    ESelection aRes = rEditEngine.GetWord( ESelection(nPara, nIndex, nPara, nIndex), com::sun::star::i18n::WordType::DICTIONARY_WORD );

    if( aRes.nStartPara == nPara &&
        aRes.nStartPara == aRes.nEndPara )
    {
        nStart = aRes.nStartPos;
        nEnd = aRes.nEndPos;

        return sal_True;
    }

    return sal_False;
}

sal_Bool SvxEditEngineForwarder::GetAttributeRun( sal_uInt16& nStartIndex, sal_uInt16& nEndIndex, sal_Int32 nPara, sal_uInt16 nIndex ) const
{
    return SvxEditSourceHelper::GetAttributeRun( nStartIndex, nEndIndex, rEditEngine, nPara, nIndex );
}

sal_uInt16 SvxEditEngineForwarder::GetLineCount( sal_Int32 nPara ) const
{
    return rEditEngine.GetLineCount(nPara);
}

sal_uInt16 SvxEditEngineForwarder::GetLineLen( sal_Int32 nPara, sal_uInt16 nLine ) const
{
    return rEditEngine.GetLineLen(nPara, nLine);
}

void SvxEditEngineForwarder::GetLineBoundaries( /*out*/sal_uInt16 &rStart, /*out*/sal_uInt16 &rEnd, sal_Int32 nPara, sal_uInt16 nLine ) const
{
    rEditEngine.GetLineBoundaries(rStart, rEnd, nPara, nLine);
}

sal_uInt16 SvxEditEngineForwarder::GetLineNumberAtIndex( sal_Int32 nPara, sal_uInt16 nIndex ) const
{
    return rEditEngine.GetLineNumberAtIndex(nPara, nIndex);
}


sal_Bool SvxEditEngineForwarder::QuickFormatDoc( sal_Bool )
{
    rEditEngine.QuickFormatDoc();

    return sal_True;
}

sal_Bool SvxEditEngineForwarder::Delete( const ESelection& rSelection )
{
    rEditEngine.QuickDelete( rSelection );
    rEditEngine.QuickFormatDoc();

    return sal_True;
}

sal_Bool SvxEditEngineForwarder::InsertText( const OUString& rStr, const ESelection& rSelection )
{
    rEditEngine.QuickInsertText( rStr, rSelection );
    rEditEngine.QuickFormatDoc();

    return sal_True;
}

sal_Int16 SvxEditEngineForwarder::GetDepth( sal_Int32 ) const
{
    // EditEngine does not support outline depth
    return -1;
}

sal_Bool SvxEditEngineForwarder::SetDepth( sal_Int32, sal_Int16 nNewDepth )
{
    // EditEngine does not support outline depth
    return nNewDepth == -1 ? sal_True : sal_False;
}

const SfxItemSet * SvxEditEngineForwarder::GetEmptyItemSetPtr()
{
    return &rEditEngine.GetEmptyItemSet();
}

void SvxEditEngineForwarder::AppendParagraph()
{
    rEditEngine.InsertParagraph( rEditEngine.GetParagraphCount(), OUString() );
}

sal_uInt16 SvxEditEngineForwarder::AppendTextPortion( sal_Int32 nPara, const OUString &rText, const SfxItemSet & /*rSet*/ )
{
    sal_uInt16 nLen = 0;

    sal_Int32 nParaCount = rEditEngine.GetParagraphCount();
    DBG_ASSERT( nPara < nParaCount, "paragraph index out of bounds" );
    if (0 <= nPara && nPara < nParaCount)
    {
        nLen = rEditEngine.GetTextLen( nPara );
        rEditEngine.QuickInsertText( rText, ESelection( nPara, nLen, nPara, nLen ) );
    }

    return nLen;
}

void SvxEditEngineForwarder::CopyText(const SvxTextForwarder& rSource)
{
    const SvxEditEngineForwarder* pSourceForwarder = dynamic_cast< const SvxEditEngineForwarder* >( &rSource );
    if( !pSourceForwarder )
        return;
    EditTextObject* pNewTextObject = pSourceForwarder->rEditEngine.CreateTextObject();
    rEditEngine.SetText( *pNewTextObject );
    delete pNewTextObject;
}

//------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
