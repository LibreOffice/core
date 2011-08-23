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

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <algorithm>

#include <com/sun/star/i18n/WordType.hpp>

#include <unoedhlp.hxx>
#include <outliner.hxx>
#include <editobj.hxx>		// nur fuer die GetText-Kruecke

#include "unofored.hxx"
namespace binfilter {

using namespace ::com::sun::star;

//------------------------------------------------------------------------

SvxEditEngineForwarder::SvxEditEngineForwarder( EditEngine& rEngine ) :
    rEditEngine( rEngine )
{
}

SvxEditEngineForwarder::~SvxEditEngineForwarder()
{
    //	die EditEngine muss ggf. von aussen geloescht werden
}

USHORT SvxEditEngineForwarder::GetParagraphCount() const
{
    return rEditEngine.GetParagraphCount();
}

USHORT SvxEditEngineForwarder::GetTextLen( USHORT nParagraph ) const
{
    return rEditEngine.GetTextLen( nParagraph );
}

String SvxEditEngineForwarder::GetText( const ESelection& rSel ) const
{
    String aRet = rEditEngine.GetText( rSel, LINEEND_LF );
    aRet.ConvertLineEnd();
    return aRet;
}

SfxItemSet SvxEditEngineForwarder::GetAttribs( const ESelection& rSel, BOOL bOnlyHardAttrib ) const
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
            DBG_ERROR("unknown flags for SvxOutlinerForwarder::GetAttribs");
        }

        return rEditEngine.GetAttribs( rSel.nStartPara, rSel.nStartPos, rSel.nEndPos, nFlags );
    }
    else
    {
        return rEditEngine.GetAttribs( rSel, bOnlyHardAttrib );
    }
}

SfxItemSet SvxEditEngineForwarder::GetParaAttribs( USHORT nPara ) const
{
    SfxItemSet aSet( rEditEngine.GetParaAttribs( nPara ) );

    USHORT nWhich = EE_PARA_START;
    while( nWhich <= EE_PARA_END )
    {
        if( aSet.GetItemState( nWhich, TRUE ) != SFX_ITEM_ON )
        {
            if( rEditEngine.HasParaAttrib( nPara, nWhich ) )
                aSet.Put( rEditEngine.GetParaAttrib( nPara, nWhich ) );
        }
        nWhich++;
    }

    return aSet;
}

void SvxEditEngineForwarder::SetParaAttribs( USHORT nPara, const SfxItemSet& rSet )
{
    rEditEngine.SetParaAttribs( nPara, rSet );
}

SfxItemPool* SvxEditEngineForwarder::GetPool() const
{
    return rEditEngine.GetEmptyItemSet().GetPool();
}

void SvxEditEngineForwarder::GetPortions( USHORT nPara, SvUShorts& rList ) const
{
    rEditEngine.GetPortions( nPara, rList );
}

void SvxEditEngineForwarder::QuickInsertText( const String& rText, const ESelection& rSel )
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

BOOL SvxEditEngineForwarder::IsValid() const
{ 
    // cannot reliably query EditEngine state 
    // while in the middle of an update
    return rEditEngine.GetUpdateMode();
}

XubString SvxEditEngineForwarder::CalcFieldValue( const SvxFieldItem& rField, USHORT nPara, USHORT nPos, Color*& rpTxtColor, Color*& rpFldColor )
{
    return rEditEngine.CalcFieldValue( rField, nPara, nPos, rpTxtColor, rpFldColor );
}

USHORT GetSvxEditEngineItemState( EditEngine& rEditEngine, const ESelection& rSel, USHORT nWhich )
{
    EECharAttribArray aAttribs;

    const SfxPoolItem*	pLastItem = NULL;

    SfxItemState eState = SFX_ITEM_DEFAULT;

    // check all paragraphs inside the selection
    for( USHORT nPara = rSel.nStartPara; nPara <= rSel.nEndPara; nPara++ )
    {
        SfxItemState eParaState = SFX_ITEM_DEFAULT;

        // calculate start and endpos for this paragraph
        USHORT nPos = 0;
        if( rSel.nStartPara == nPara )
            nPos = rSel.nStartPos;

        USHORT nEndPos = rSel.nEndPos;
        if( rSel.nEndPara != nPara )
            nEndPos = rEditEngine.GetTextLen( nPara );


        // get list of char attribs
        rEditEngine.GetCharAttribs( nPara, aAttribs );

        BOOL bEmpty = TRUE;		// we found no item inside the selektion of this paragraph
        BOOL bGaps  = FALSE;	// we found items but theire gaps between them
        USHORT nLastEnd = nPos;

        const SfxPoolItem* pParaItem = NULL;

        for( USHORT nAttrib = 0; nAttrib < aAttribs.Count(); nAttrib++ )
        {
            struct EECharAttrib aAttrib = aAttribs.GetObject( nAttrib );
            DBG_ASSERT( aAttrib.pAttr, "GetCharAttribs gives corrupt data" );

            const sal_Bool bEmptyPortion = aAttrib.nStart == aAttrib.nEnd;
            if( (!bEmptyPortion && (aAttrib.nStart >= nEndPos)) || (bEmptyPortion && (aAttrib.nStart > nEndPos)) )
                break;	// break if we are already behind our selektion

            if( (!bEmptyPortion && (aAttrib.nEnd <= nPos)) || (bEmptyPortion && (aAttrib.nEnd < nPos)) )
                continue;	// or if the attribute ends before our selektion

            if( aAttrib.pAttr->Which() != nWhich )
                continue; // skip if is not the searched item

            // if we already found an item
            if( pParaItem )
            {
                // ... and its different to this one than the state is dont care
                if( *pParaItem != *aAttrib.pAttr )
                    return SFX_ITEM_DONTCARE;
            }
            else
            {
                pParaItem = aAttrib.pAttr;
            }

            if( bEmpty )
                bEmpty = FALSE;

            if( !bGaps && aAttrib.nStart > nLastEnd )
                bGaps = TRUE;

            nLastEnd = aAttrib.nEnd;
        }

        if( !bEmpty && !bGaps && nLastEnd < ( nEndPos - 1 ) )
            bGaps = TRUE;
/*
        // since we have no portion with our item or if there were gaps
        if( bEmpty || bGaps )
        {
            // we need to check the paragraph item
            const SfxItemSet& rParaSet = rEditEngine.GetParaAttribs( nPara );
            if( rParaSet.GetItemState( nWhich ) == SFX_ITEM_SET )
            {
                eState = SFX_ITEM_SET;
                // get item from the paragraph
                const SfxPoolItem* pTempItem = rParaSet.GetItem( nWhich );
                if( pParaItem )
                {
                    if( *pParaItem != *pTempItem )
                        return SFX_ITEM_DONTCARE;
                } 
                else
                {
                    pParaItem = pTempItem;
                }

                // set if theres no last item or if its the same
                eParaState = SFX_ITEM_SET;
            }
            else if( bEmpty )
            {
                eParaState = SFX_ITEM_DEFAULT;
            }
            else if( bGaps )
            {
                // gaps and item not set in paragraph, thats a dont care
                return SFX_ITEM_DONTCARE;
            }
        }
        else
        {
            eParaState = SFX_ITEM_SET;
        }
*/
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

USHORT SvxEditEngineForwarder::GetItemState( const ESelection& rSel, USHORT nWhich ) const
{
    return GetSvxEditEngineItemState( rEditEngine, rSel, nWhich );
}

USHORT SvxEditEngineForwarder::GetItemState( USHORT nPara, USHORT nWhich ) const
{
    const SfxItemSet& rSet = rEditEngine.GetParaAttribs( nPara );
    return rSet.GetItemState( nWhich );
}

LanguageType SvxEditEngineForwarder::GetLanguage( USHORT nPara, USHORT nIndex ) const
{
    return rEditEngine.GetLanguage(nPara, nIndex);
}

USHORT SvxEditEngineForwarder::GetFieldCount( USHORT nPara ) const
{
    return rEditEngine.GetFieldCount(nPara); 
}

EFieldInfo SvxEditEngineForwarder::GetFieldInfo( USHORT nPara, USHORT nField ) const
{
    return rEditEngine.GetFieldInfo( nPara, nField );
}

EBulletInfo SvxEditEngineForwarder::GetBulletInfo( USHORT nPara ) const
{
    return EBulletInfo();
}

Rectangle SvxEditEngineForwarder::GetCharBounds( USHORT nPara, USHORT nIndex ) const
{
    // #101701#
    // EditEngine's 'internal' methods like GetCharacterBounds()
    // don't rotate for vertical text.
    Size aSize( rEditEngine.CalcTextWidth(), rEditEngine.GetTextHeight() );
    ::std::swap( aSize.Width(), aSize.Height() );
    bool bIsVertical( rEditEngine.IsVertical() == TRUE );

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

Rectangle SvxEditEngineForwarder::GetParaBounds( USHORT nPara ) const
{
    const Point aPnt = rEditEngine.GetDocPosTopLeft( nPara );
    ULONG nWidth; 
    ULONG nHeight;
    ULONG nTextWidth;

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

sal_Bool SvxEditEngineForwarder::GetIndexAtPoint( const Point& rPos, USHORT& nPara, USHORT& nIndex ) const
{
    // #101701#
    Size aSize( rEditEngine.CalcTextWidth(), rEditEngine.GetTextHeight() );
    ::std::swap( aSize.Width(), aSize.Height() );
    Point aEEPos( SvxEditSourceHelper::UserSpaceToEE( rPos, 
                                                      aSize,
                                                      rEditEngine.IsVertical() == TRUE ));

    EPosition aDocPos = rEditEngine.FindDocPosition( aEEPos );

    nPara = aDocPos.nPara;
    nIndex = aDocPos.nIndex;

    return sal_True;
}

sal_Bool SvxEditEngineForwarder::GetWordIndices( USHORT nPara, USHORT nIndex, USHORT& nStart, USHORT& nEnd ) const
{
    ESelection aRes = rEditEngine.GetWord( ESelection(nPara, nIndex, nPara, nIndex), ::com::sun::star::i18n::WordType::DICTIONARY_WORD );

    if( aRes.nStartPara == nPara &&
        aRes.nStartPara == aRes.nEndPara )
    {
        nStart = aRes.nStartPos;
        nEnd = aRes.nEndPos;

        return sal_True;
    }

    return sal_False;
}

sal_Bool SvxEditEngineForwarder::GetAttributeRun( USHORT& nStartIndex, USHORT& nEndIndex, USHORT nPara, USHORT nIndex ) const
{
    return SvxEditSourceHelper::GetAttributeRun( nStartIndex, nEndIndex, rEditEngine, nPara, nIndex );
}

USHORT SvxEditEngineForwarder::GetLineCount( USHORT nPara ) const
{
    return rEditEngine.GetLineCount(nPara);
}

USHORT SvxEditEngineForwarder::GetLineLen( USHORT nPara, USHORT nLine ) const
{
    return rEditEngine.GetLineLen(nPara, nLine);
}

sal_Bool SvxEditEngineForwarder::QuickFormatDoc( BOOL bFull )
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

sal_Bool SvxEditEngineForwarder::InsertText( const String& rStr, const ESelection& rSelection )
{
    rEditEngine.QuickInsertText( rStr, rSelection );
    rEditEngine.QuickFormatDoc();

    return sal_True;
}

USHORT SvxEditEngineForwarder::GetDepth( USHORT nPara ) const
{
    // EditEngine does not support outline depth
    return 0;
}

sal_Bool SvxEditEngineForwarder::SetDepth( USHORT nPara, USHORT nNewDepth )
{
    // EditEngine does not support outline depth
    return nNewDepth == 0 ? sal_True : sal_False;
}

//------------------------------------------------------------------------
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
