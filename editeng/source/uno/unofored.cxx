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
#include <osl/diagnose.h>
#include <tools/debug.hxx>
#include <editeng/eeitem.hxx>
#include <com/sun/star/i18n/WordType.hpp>

#include <svl/itemset.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>
#include <editeng/unoedhlp.hxx>
#include <editeng/editdata.hxx>
#include <editeng/outliner.hxx>
#include <editeng/editobj.hxx>

#include <editeng/unofored.hxx>
#include "unofored_internal.hxx"

using namespace ::com::sun::star;


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

sal_Int32 SvxEditEngineForwarder::GetTextLen( sal_Int32 nParagraph ) const
{
    return rEditEngine.GetTextLen( nParagraph );
}

OUString SvxEditEngineForwarder::GetText( const ESelection& rSel ) const
{
    return convertLineEnd(rEditEngine.GetText(rSel), GetSystemLineEnd());
}

SfxItemSet SvxEditEngineForwarder::GetAttribs( const ESelection& rSel, EditEngineAttribs nOnlyHardAttrib ) const
{
    if( rSel.nStartPara == rSel.nEndPara )
    {
        GetAttribsFlags nFlags = GetAttribsFlags::NONE;
        switch( nOnlyHardAttrib )
        {
        case EditEngineAttribs::All:
            nFlags = GetAttribsFlags::ALL;
            break;
        case EditEngineAttribs::OnlyHard:
            nFlags = GetAttribsFlags::CHARATTRIBS;
            break;
        default:
            OSL_FAIL("unknown flags for SvxOutlinerForwarder::GetAttribs");
        }

        return rEditEngine.GetAttribs( rSel.nStartPara, rSel.nStartPos, rSel.nEndPos, nFlags );
    }
    else
    {
        return rEditEngine.GetAttribs( rSel, nOnlyHardAttrib );
    }
}

SfxItemSet SvxEditEngineForwarder::GetParaAttribs( sal_Int32 nPara ) const
{
    SfxItemSet aSet( rEditEngine.GetParaAttribs( nPara ) );

    sal_uInt16 nWhich = EE_PARA_START;
    while( nWhich <= EE_PARA_END )
    {
        if( aSet.GetItemState( nWhich ) != SfxItemState::SET )
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

void SvxEditEngineForwarder::RemoveAttribs( const ESelection& rSelection )
{
    rEditEngine.RemoveAttribs( rSelection, false/*bRemoveParaAttribs*/, 0 );
}

SfxItemPool* SvxEditEngineForwarder::GetPool() const
{
    return rEditEngine.GetEmptyItemSet().GetPool();
}

void SvxEditEngineForwarder::GetPortions( sal_Int32 nPara, std::vector<sal_Int32>& rList ) const
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

bool SvxEditEngineForwarder::IsValid() const
{
    // cannot reliably query EditEngine state
    // while in the middle of an update
    return rEditEngine.GetUpdateMode();
}

OUString SvxEditEngineForwarder::CalcFieldValue( const SvxFieldItem& rField, sal_Int32 nPara, sal_Int32 nPos, boost::optional<Color>& rpTxtColor, boost::optional<Color>& rpFldColor )
{
    return rEditEngine.CalcFieldValue( rField, nPara, nPos, rpTxtColor, rpFldColor );
}

void SvxEditEngineForwarder::FieldClicked( const SvxFieldItem& rField, sal_Int32 nPara, sal_Int32 nPos )
{
    rEditEngine.FieldClicked( rField, nPara, nPos );
}

SfxItemState GetSvxEditEngineItemState( EditEngine const & rEditEngine, const ESelection& rSel, sal_uInt16 nWhich )
{
    std::vector<EECharAttrib> aAttribs;

    const SfxPoolItem*  pLastItem = nullptr;

    SfxItemState eState = SfxItemState::DEFAULT;

    // check all paragraphs inside the selection
    for( sal_Int32 nPara = rSel.nStartPara; nPara <= rSel.nEndPara; nPara++ )
    {
        SfxItemState eParaState = SfxItemState::DEFAULT;

        // calculate start and endpos for this paragraph
        sal_Int32 nPos = 0;
        if( rSel.nStartPara == nPara )
            nPos = rSel.nStartPos;

        sal_Int32 nEndPos = rSel.nEndPos;
        if( rSel.nEndPara != nPara )
            nEndPos = rEditEngine.GetTextLen( nPara );


        // get list of char attribs
        rEditEngine.GetCharAttribs( nPara, aAttribs );

        bool bEmpty = true;     // we found no item inside the selection of this paragraph
        bool bGaps  = false;    // we found items but theire gaps between them
        sal_Int32 nLastEnd = nPos;

        const SfxPoolItem* pParaItem = nullptr;

        for (auto const& attrib : aAttribs)
        {
            DBG_ASSERT(attrib.pAttr, "GetCharAttribs gives corrupt data");

            const bool bEmptyPortion = attrib.nStart == attrib.nEnd;
            if((!bEmptyPortion && attrib.nStart >= nEndPos) ||
               (bEmptyPortion && attrib.nStart > nEndPos))
                break;  // break if we are already behind our selection

            if((!bEmptyPortion && attrib.nEnd <= nPos) ||
               (bEmptyPortion && attrib.nEnd < nPos))
                continue;   // or if the attribute ends before our selection

            if(attrib.pAttr->Which() != nWhich)
                continue; // skip if is not the searched item

            // if we already found an item
            if( pParaItem )
            {
                // ... and its different to this one than the state is don't care
                if(*pParaItem != *(attrib.pAttr))
                    return SfxItemState::DONTCARE;
            }
            else
                pParaItem = attrib.pAttr;

            if( bEmpty )
                bEmpty = false;

            if(!bGaps && attrib.nStart > nLastEnd)
                bGaps = true;

            nLastEnd = attrib.nEnd;
        }

        if( !bEmpty && !bGaps && nLastEnd < ( nEndPos - 1 ) )
            bGaps = true;

        if( bEmpty )
            eParaState = SfxItemState::DEFAULT;
        else if( bGaps )
            eParaState = SfxItemState::DONTCARE;
        else
            eParaState = SfxItemState::SET;

        // if we already found an item check if we found the same
        if( pLastItem )
        {
            if( (pParaItem == nullptr) || (*pLastItem != *pParaItem) )
                return SfxItemState::DONTCARE;
        }
        else
        {
            pLastItem = pParaItem;
            eState = eParaState;
        }
    }

    return eState;
}

SfxItemState SvxEditEngineForwarder::GetItemState( const ESelection& rSel, sal_uInt16 nWhich ) const
{
    return GetSvxEditEngineItemState( rEditEngine, rSel, nWhich );
}

SfxItemState SvxEditEngineForwarder::GetItemState( sal_Int32 nPara, sal_uInt16 nWhich ) const
{
    const SfxItemSet& rSet = rEditEngine.GetParaAttribs( nPara );
    return rSet.GetItemState( nWhich );
}

LanguageType SvxEditEngineForwarder::GetLanguage( sal_Int32 nPara, sal_Int32 nIndex ) const
{
    return rEditEngine.GetLanguage(nPara, nIndex);
}

sal_Int32 SvxEditEngineForwarder::GetFieldCount( sal_Int32 nPara ) const
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

tools::Rectangle SvxEditEngineForwarder::GetCharBounds( sal_Int32 nPara, sal_Int32 nIndex ) const
{
    // EditEngine's 'internal' methods like GetCharacterBounds()
    // don't rotate for vertical text.
    Size aSize( rEditEngine.CalcTextWidth(), rEditEngine.GetTextHeight() );
    // swap width and height
    long tmp = aSize.Width();
    aSize.setWidth(aSize.Height());
    aSize.setHeight(tmp);
    bool bIsVertical( rEditEngine.IsVertical() );

    // #108900# Handle virtual position one-past-the end of the string
    if( nIndex >= rEditEngine.GetTextLen(nPara) )
    {
        tools::Rectangle aLast;

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
                aLast.SetSize( Size( rEditEngine.GetLineHeight(nPara), 1 ) );
            else
                aLast.SetSize( Size( 1, rEditEngine.GetLineHeight(nPara) ) );
        }

        return aLast;
    }
    else
    {
        return SvxEditSourceHelper::EEToUserSpace( rEditEngine.GetCharacterBounds( EPosition(nPara, nIndex) ),
                                                   aSize, bIsVertical );
    }
}

tools::Rectangle SvxEditEngineForwarder::GetParaBounds( sal_Int32 nPara ) const
{
    const Point aPnt = rEditEngine.GetDocPosTopLeft( nPara );
    sal_uLong nWidth;
    sal_uLong nHeight;
    sal_uLong nTextWidth;

    if( rEditEngine.IsVertical() )
    {
        // Hargl. EditEngine's 'external' methods return the rotated
        // dimensions, 'internal' methods like GetTextHeight( n )
        // don't rotate.
        nWidth = rEditEngine.GetTextHeight( nPara );
        nHeight = rEditEngine.GetTextHeight();
        nTextWidth = rEditEngine.GetTextHeight();

        return tools::Rectangle( nTextWidth - aPnt.Y() - nWidth, 0, nTextWidth - aPnt.Y(), nHeight );
    }
    else
    {
        nWidth = rEditEngine.CalcTextWidth();
        nHeight = rEditEngine.GetTextHeight( nPara );

        return tools::Rectangle( 0, aPnt.Y(), nWidth, aPnt.Y() + nHeight );
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

bool SvxEditEngineForwarder::GetIndexAtPoint( const Point& rPos, sal_Int32& nPara, sal_Int32& nIndex ) const
{
    Size aSize( rEditEngine.CalcTextWidth(), rEditEngine.GetTextHeight() );
    // swap width and height
    long tmp = aSize.Width();
    aSize.setWidth(aSize.Height());
    aSize.setHeight(tmp);
    Point aEEPos( SvxEditSourceHelper::UserSpaceToEE( rPos,
                                                      aSize,
                                                      rEditEngine.IsVertical() ));

    EPosition aDocPos = rEditEngine.FindDocPosition( aEEPos );

    nPara = aDocPos.nPara;
    nIndex = aDocPos.nIndex;

    return true;
}

bool SvxEditEngineForwarder::GetWordIndices( sal_Int32 nPara, sal_Int32 nIndex, sal_Int32& nStart, sal_Int32& nEnd ) const
{
    ESelection aRes = rEditEngine.GetWord( ESelection(nPara, nIndex, nPara, nIndex), css::i18n::WordType::DICTIONARY_WORD );

    if( aRes.nStartPara == nPara &&
        aRes.nStartPara == aRes.nEndPara )
    {
        nStart = aRes.nStartPos;
        nEnd = aRes.nEndPos;

        return true;
    }

    return false;
}

bool SvxEditEngineForwarder::GetAttributeRun( sal_Int32& nStartIndex, sal_Int32& nEndIndex, sal_Int32 nPara, sal_Int32 nIndex, bool bInCell ) const
{
    SvxEditSourceHelper::GetAttributeRun( nStartIndex, nEndIndex, rEditEngine, nPara, nIndex, bInCell );
    return true;
}

sal_Int32 SvxEditEngineForwarder::GetLineCount( sal_Int32 nPara ) const
{
    return rEditEngine.GetLineCount(nPara);
}

sal_Int32 SvxEditEngineForwarder::GetLineLen( sal_Int32 nPara, sal_Int32 nLine ) const
{
    return rEditEngine.GetLineLen(nPara, nLine);
}

void SvxEditEngineForwarder::GetLineBoundaries( /*out*/sal_Int32 &rStart, /*out*/sal_Int32 &rEnd, sal_Int32 nPara, sal_Int32 nLine ) const
{
    rEditEngine.GetLineBoundaries(rStart, rEnd, nPara, nLine);
}

sal_Int32 SvxEditEngineForwarder::GetLineNumberAtIndex( sal_Int32 nPara, sal_Int32 nIndex ) const
{
    return rEditEngine.GetLineNumberAtIndex(nPara, nIndex);
}


bool SvxEditEngineForwarder::QuickFormatDoc( bool )
{
    rEditEngine.QuickFormatDoc();

    return true;
}

bool SvxEditEngineForwarder::Delete( const ESelection& rSelection )
{
    rEditEngine.QuickDelete( rSelection );
    rEditEngine.QuickFormatDoc();

    return true;
}

bool SvxEditEngineForwarder::InsertText( const OUString& rStr, const ESelection& rSelection )
{
    rEditEngine.QuickInsertText( rStr, rSelection );
    rEditEngine.QuickFormatDoc();

    return true;
}

sal_Int16 SvxEditEngineForwarder::GetDepth( sal_Int32 ) const
{
    // EditEngine does not support outline depth
    return -1;
}

bool SvxEditEngineForwarder::SetDepth( sal_Int32, sal_Int16 nNewDepth )
{
    // EditEngine does not support outline depth
    return nNewDepth == -1;
}

const SfxItemSet * SvxEditEngineForwarder::GetEmptyItemSetPtr()
{
    return &rEditEngine.GetEmptyItemSet();
}

void SvxEditEngineForwarder::AppendParagraph()
{
    rEditEngine.InsertParagraph( rEditEngine.GetParagraphCount(), OUString() );
}

sal_Int32 SvxEditEngineForwarder::AppendTextPortion( sal_Int32 nPara, const OUString &rText, const SfxItemSet & /*rSet*/ )
{
    sal_Int32 nLen = 0;

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
    std::unique_ptr<EditTextObject> pNewTextObject = pSourceForwarder->rEditEngine.CreateTextObject();
    rEditEngine.SetText( *pNewTextObject );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
