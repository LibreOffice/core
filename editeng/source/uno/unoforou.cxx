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
#include <svl/style.hxx>
#include <com/sun/star/i18n/WordType.hpp>

#include <svl/itemset.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editdata.hxx>
#include <editeng/outliner.hxx>
#include <editeng/unoedhlp.hxx>
#include <svl/poolitem.hxx>
#include <vcl/wrkwin.hxx>
#include <editeng/eeitem.hxx>

#include <editeng/unoforou.hxx>
#include <editeng/unofored.hxx>
#include <editeng/outlobj.hxx>
#include <unofored_internal.hxx>

using namespace ::com::sun::star;


SvxOutlinerForwarder::SvxOutlinerForwarder( const std::shared_ptr< Outliner >& pOutl, bool bOutlText /* = false */ ) :
    mpOutliner( pOutl ),
    mbOutlinerText( bOutlText ),
    mpAttribsCache( nullptr ),
    mpParaAttribsCache( nullptr ),
    mnParaAttribsCache( 0 )
{
}

SvxOutlinerForwarder::~SvxOutlinerForwarder()
{
    flushCache();
}

sal_Int32 SvxOutlinerForwarder::GetParagraphCount() const
{
    return mpOutliner->GetParagraphCount();
}

sal_Int32 SvxOutlinerForwarder::GetTextLen( sal_Int32 nParagraph ) const
{
    return mpOutliner->GetEditEngine().GetTextLen( nParagraph );
}

OUString SvxOutlinerForwarder::GetText( const ESelection& rSel ) const
{
    //! GetText (ESelection) should probably also be in the Outliner
    // in the time being use as the hack for the EditEngine:
    EditEngine* pEditEngine = const_cast<EditEngine*>(&mpOutliner->GetEditEngine());
    return pEditEngine->GetText( rSel );
}

static SfxItemSet ImplOutlinerForwarderGetAttribs( const ESelection& rSel, EditEngineAttribs nOnlyHardAttrib, EditEngine& rEditEngine )
{
    if( rSel.nStartPara == rSel.nEndPara )
    {
        GetAttribsFlags nFlags = GetAttribsFlags::NONE;

        switch( nOnlyHardAttrib )
        {
        case EditEngineAttribs_All:
            nFlags = GetAttribsFlags::ALL;
            break;
        case EditEngineAttribs_HardAndPara:
            nFlags = GetAttribsFlags::PARAATTRIBS|GetAttribsFlags::CHARATTRIBS;
            break;
        case EditEngineAttribs_OnlyHard:
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

SfxItemSet SvxOutlinerForwarder::GetAttribs( const ESelection& rSel, EditEngineAttribs nOnlyHardAttrib ) const
{
    if( mpAttribsCache && ( EditEngineAttribs_All == nOnlyHardAttrib ) )
    {
        // have we the correct set in cache?
        if( const_cast<SvxOutlinerForwarder*>(this)->maAttribCacheSelection.IsEqual(rSel) )
        {
            // yes! just return the cache
            return *mpAttribsCache;
        }
        else
        {
            // no, we need delete the old cache
            delete mpAttribsCache;
            mpAttribsCache = nullptr;
        }
    }

    //! Does it not exist on the Outliner?
    //! and why is the GetAttribs on the EditEngine not a const?
    EditEngine& rEditEngine = (EditEngine&)mpOutliner->GetEditEngine();

    SfxItemSet aSet( ImplOutlinerForwarderGetAttribs( rSel, nOnlyHardAttrib, rEditEngine ) );

    if( EditEngineAttribs_All == nOnlyHardAttrib )
    {
        mpAttribsCache = new SfxItemSet( aSet );
        maAttribCacheSelection = rSel;
    }

    SfxStyleSheet* pStyle = rEditEngine.GetStyleSheet( rSel.nStartPara );
    if( pStyle )
        aSet.SetParent( &(pStyle->GetItemSet() ) );

    return aSet;
}

SfxItemSet SvxOutlinerForwarder::GetParaAttribs( sal_Int32 nPara ) const
{
    if( mpParaAttribsCache )
    {
        // have we the correct set in cache?
        if( nPara == mnParaAttribsCache )
        {
            // yes! just return the cache
            return *mpParaAttribsCache;
        }
        else
        {
            // no, we need delete the old cache
            delete mpParaAttribsCache;
            mpParaAttribsCache = nullptr;
        }
    }

    mpParaAttribsCache = new SfxItemSet( mpOutliner->GetParaAttribs( nPara ) );
    mnParaAttribsCache = nPara;

    EditEngine& rEditEngine = (EditEngine&)mpOutliner->GetEditEngine();

    SfxStyleSheet* pStyle = rEditEngine.GetStyleSheet( nPara );
    if( pStyle )
        mpParaAttribsCache->SetParent( &(pStyle->GetItemSet() ) );

    return *mpParaAttribsCache;
}

void SvxOutlinerForwarder::SetParaAttribs( sal_Int32 nPara, const SfxItemSet& rSet )
{
    flushCache();

    const SfxItemSet* pOldParent = rSet.GetParent();
    if( pOldParent )
        const_cast<SfxItemSet*>(&rSet)->SetParent( nullptr );

    mpOutliner->SetParaAttribs( nPara, rSet );

    if( pOldParent )
        const_cast<SfxItemSet*>(&rSet)->SetParent( pOldParent );
}

void SvxOutlinerForwarder::RemoveAttribs( const ESelection& rSelection, bool bRemoveParaAttribs, sal_uInt16 nWhich )
{
    mpOutliner->RemoveAttribs( rSelection, bRemoveParaAttribs, nWhich );
}

SfxItemPool* SvxOutlinerForwarder::GetPool() const
{
    return mpOutliner->GetEmptyItemSet().GetPool();
}

void SvxOutlinerForwarder::GetPortions( sal_Int32 nPara, std::vector<sal_Int32>& rList ) const
{
    ((EditEngine&)mpOutliner->GetEditEngine()).GetPortions( nPara, rList );
}

void SvxOutlinerForwarder::QuickInsertText( const OUString& rText, const ESelection& rSel )
{
    flushCache();
    if( rText.isEmpty() )
    {
        mpOutliner->QuickDelete( rSel );
    }
    else
    {
        mpOutliner->QuickInsertText( rText, rSel );
    }
}

void SvxOutlinerForwarder::QuickInsertLineBreak( const ESelection& rSel )
{
    flushCache();
    mpOutliner->QuickInsertLineBreak( rSel );
}

void SvxOutlinerForwarder::QuickInsertField( const SvxFieldItem& rFld, const ESelection& rSel )
{
    flushCache();
    mpOutliner->QuickInsertField( rFld, rSel );
}

void SvxOutlinerForwarder::QuickSetAttribs( const SfxItemSet& rSet, const ESelection& rSel )
{
    flushCache();
    mpOutliner->QuickSetAttribs( rSet, rSel );
}

OUString SvxOutlinerForwarder::CalcFieldValue( const SvxFieldItem& rField, sal_Int32 nPara, sal_Int32 nPos, Color*& rpTxtColor, Color*& rpFldColor )
{
    return mpOutliner->CalcFieldValue( rField, nPara, nPos, rpTxtColor, rpFldColor );
}

void SvxOutlinerForwarder::FieldClicked( const SvxFieldItem& rField, sal_Int32 nPara, sal_Int32 nPos )
{
    mpOutliner->FieldClicked( rField, nPara, nPos );
}

bool SvxOutlinerForwarder::IsValid() const
{
    // cannot reliably query outliner state
    // while in the middle of an update
    return mpOutliner->GetUpdateMode();
}

SfxItemState SvxOutlinerForwarder::GetItemState( const ESelection& rSel, sal_uInt16 nWhich ) const
{
    return GetSvxEditEngineItemState( (EditEngine&)mpOutliner->GetEditEngine(), rSel, nWhich );
}

SfxItemState SvxOutlinerForwarder::GetItemState( sal_Int32 nPara, sal_uInt16 nWhich ) const
{
    const SfxItemSet& rSet = mpOutliner->GetParaAttribs( nPara );
    return rSet.GetItemState( nWhich );
}


void SvxOutlinerForwarder::flushCache()
{
    if( mpAttribsCache )
    {
        delete mpAttribsCache;
        mpAttribsCache = nullptr;
    }

    if( mpParaAttribsCache )
    {
        delete mpParaAttribsCache;
        mpParaAttribsCache = nullptr;
    }
}

LanguageType SvxOutlinerForwarder::GetLanguage( sal_Int32 nPara, sal_Int32 nIndex ) const
{
    return mpOutliner->GetLanguage(nPara, nIndex);
}

sal_Int32 SvxOutlinerForwarder::GetFieldCount( sal_Int32 nPara ) const
{
    return mpOutliner->GetEditEngine().GetFieldCount(nPara);
}

EFieldInfo SvxOutlinerForwarder::GetFieldInfo( sal_Int32 nPara, sal_uInt16 nField ) const
{
    return mpOutliner->GetEditEngine().GetFieldInfo( nPara, nField );
}

EBulletInfo SvxOutlinerForwarder::GetBulletInfo( sal_Int32 nPara ) const
{
    return mpOutliner->GetBulletInfo( nPara );
}

Rectangle SvxOutlinerForwarder::GetCharBounds( sal_Int32 nPara, sal_Int32 nIndex ) const
{
    // EditEngine's 'internal' methods like GetCharacterBounds()
    // don't rotate for vertical text.
    Size aSize( mpOutliner->CalcTextSize() );
    ::std::swap( aSize.Width(), aSize.Height() );
    bool bIsVertical( mpOutliner->IsVertical() );

    // #108900# Handle virtual position one-past-the end of the string
    if( nIndex >= GetTextLen(nPara) )
    {
        Rectangle aLast;

        if( nIndex )
        {
            // use last character, if possible
            aLast = mpOutliner->GetEditEngine().GetCharacterBounds( EPosition(nPara, nIndex-1) );

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
                aLast.SetSize( Size( mpOutliner->GetLineHeight(nPara), 1 ) );
            else
                aLast.SetSize( Size( 1, mpOutliner->GetLineHeight(nPara) ) );
        }

        return aLast;
    }
    else
    {
        return SvxEditSourceHelper::EEToUserSpace( mpOutliner->GetEditEngine().GetCharacterBounds( EPosition(nPara, nIndex) ),
                                                   aSize, bIsVertical );
    }
}

Rectangle SvxOutlinerForwarder::GetParaBounds( sal_Int32 nPara ) const
{
    Point aPnt = mpOutliner->GetDocPosTopLeft( nPara );
    Size aSize = mpOutliner->CalcTextSize();

    if( mpOutliner->IsVertical() )
    {
        // Hargl. Outliner's 'external' methods return the rotated
        // dimensions, 'internal' methods like GetTextHeight( n )
        // don't rotate.
        sal_uLong nWidth = mpOutliner->GetTextHeight( nPara );

        return Rectangle( aSize.Width() - aPnt.Y() - nWidth, 0, aSize.Width() - aPnt.Y(), aSize.Height() );
    }
    else
    {
        sal_uLong nHeight = mpOutliner->GetTextHeight( nPara );

        return Rectangle( 0, aPnt.Y(), aSize.Width(), aPnt.Y() + nHeight );
    }
}

MapMode SvxOutlinerForwarder::GetMapMode() const
{
    return mpOutliner->GetRefMapMode();
}

OutputDevice* SvxOutlinerForwarder::GetRefDevice() const
{
    return mpOutliner->GetRefDevice();
}

bool SvxOutlinerForwarder::GetIndexAtPoint( const Point& rPos, sal_Int32& nPara, sal_Int32& nIndex ) const
{
    Size aSize( mpOutliner->CalcTextSize() );
    ::std::swap( aSize.Width(), aSize.Height() );
    Point aEEPos( SvxEditSourceHelper::UserSpaceToEE( rPos,
                                                      aSize,
                                                      mpOutliner->IsVertical() ));

    EPosition aDocPos = mpOutliner->GetEditEngine().FindDocPosition( aEEPos );

    nPara = aDocPos.nPara;
    nIndex = aDocPos.nIndex;

    return true;
}

bool SvxOutlinerForwarder::GetWordIndices( sal_Int32 nPara, sal_Int32 nIndex, sal_Int32& nStart, sal_Int32& nEnd ) const
{
    ESelection aRes = mpOutliner->GetEditEngine().GetWord( ESelection(nPara, nIndex, nPara, nIndex), css::i18n::WordType::DICTIONARY_WORD );

    if( aRes.nStartPara == nPara &&
        aRes.nStartPara == aRes.nEndPara )
    {
        nStart = aRes.nStartPos;
        nEnd = aRes.nEndPos;

        return true;
    }

    return false;
}

bool SvxOutlinerForwarder::GetAttributeRun( sal_Int32& nStartIndex, sal_Int32& nEndIndex, sal_Int32 nPara, sal_Int32 nIndex, bool bInCell ) const
{
    return SvxEditSourceHelper::GetAttributeRun( nStartIndex, nEndIndex, mpOutliner->GetEditEngine(), nPara, nIndex, bInCell );
}

sal_Int32 SvxOutlinerForwarder::GetLineCount( sal_Int32 nPara ) const
{
    return mpOutliner->GetLineCount(nPara);
}

sal_Int32 SvxOutlinerForwarder::GetLineLen( sal_Int32 nPara, sal_Int32 nLine ) const
{
    return mpOutliner->GetLineLen(nPara, nLine);
}

void SvxOutlinerForwarder::GetLineBoundaries( /*out*/sal_Int32 &rStart, /*out*/sal_Int32 &rEnd, sal_Int32 nPara, sal_Int32 nLine ) const
{
    return mpOutliner->GetEditEngine().GetLineBoundaries( rStart, rEnd, nPara, nLine );
}

sal_Int32 SvxOutlinerForwarder::GetLineNumberAtIndex( sal_Int32 nPara, sal_Int32 nIndex ) const
{
    return mpOutliner->GetEditEngine().GetLineNumberAtIndex( nPara, nIndex );
}

bool SvxOutlinerForwarder::QuickFormatDoc( bool )
{
    mpOutliner->QuickFormatDoc();

    return true;
}

bool SvxOutlinerForwarder::Delete( const ESelection& rSelection )
{
    flushCache();
    mpOutliner->QuickDelete( rSelection );
    mpOutliner->QuickFormatDoc();

    return true;
}

bool SvxOutlinerForwarder::InsertText( const OUString& rStr, const ESelection& rSelection )
{
    flushCache();
    mpOutliner->QuickInsertText( rStr, rSelection );
    mpOutliner->QuickFormatDoc();

    return true;
}

sal_Int16 SvxOutlinerForwarder::GetDepth( sal_Int32 nPara ) const
{
    DBG_ASSERT( 0 <= nPara && nPara < GetParagraphCount(), "SvxOutlinerForwarder::GetDepth: Invalid paragraph index");

    Paragraph* pPara = mpOutliner->GetParagraph( nPara );

    sal_Int16 nLevel = -1;

    if( pPara )
        nLevel = mpOutliner->GetDepth( nPara );

    return nLevel;
}

bool SvxOutlinerForwarder::SetDepth( sal_Int32 nPara, sal_Int16 nNewDepth )
{
    DBG_ASSERT( 0 <= nPara && nPara < GetParagraphCount(), "SvxOutlinerForwarder::SetDepth: Invalid paragraph index");

    if( (nNewDepth >= -1) && (nNewDepth <= 9) && (0 <= nPara && nPara < GetParagraphCount()) )
    {
        Paragraph* pPara = mpOutliner->GetParagraph( nPara );
        if( pPara )
        {
            mpOutliner->SetDepth( pPara, nNewDepth );

//          const bool bOutlinerText = pSdrObject && (pSdrObject->GetObjInventor() == SdrInventor) && (pSdrObject->GetObjIdentifier() == OBJ_OUTLINETEXT);
            if( mbOutlinerText )
                mpOutliner->SetLevelDependendStyleSheet( nPara );

            return true;
        }
    }

    return false;
}

sal_Int32 SvxOutlinerForwarder::GetNumberingStartValue( sal_Int32 nPara )
{
    if( 0 <= nPara && nPara < GetParagraphCount() )
    {
        return mpOutliner->GetNumberingStartValue( nPara );
    }
    else
    {
        OSL_FAIL( "SvxOutlinerForwarder::GetNumberingStartValue)(), Invalid paragraph index");
        return -1;
    }
}

void SvxOutlinerForwarder::SetNumberingStartValue(  sal_Int32 nPara, sal_Int32 nNumberingStartValue )
{
    if( 0 <= nPara && nPara < GetParagraphCount() )
    {
        mpOutliner->SetNumberingStartValue( nPara, nNumberingStartValue );
    }
    else
    {
        OSL_FAIL( "SvxOutlinerForwarder::SetNumberingStartValue)(), Invalid paragraph index");
    }
}

bool SvxOutlinerForwarder::IsParaIsNumberingRestart( sal_Int32 nPara )
{
    if( 0 <= nPara && nPara < GetParagraphCount() )
    {
        return mpOutliner->IsParaIsNumberingRestart( nPara );
    }
    else
    {
        OSL_FAIL( "SvxOutlinerForwarder::IsParaIsNumberingRestart)(), Invalid paragraph index");
        return false;
    }
}

void SvxOutlinerForwarder::SetParaIsNumberingRestart(  sal_Int32 nPara, bool bParaIsNumberingRestart )
{
    if( 0 <= nPara && nPara < GetParagraphCount() )
    {
        mpOutliner->SetParaIsNumberingRestart( nPara, bParaIsNumberingRestart );
    }
    else
    {
        OSL_FAIL( "SvxOutlinerForwarder::SetParaIsNumberingRestart)(), Invalid paragraph index");
    }
}

const SfxItemSet * SvxOutlinerForwarder::GetEmptyItemSetPtr()
{
    EditEngine& rEditEngine = const_cast< EditEngine& >( mpOutliner->GetEditEngine() );
    return &rEditEngine.GetEmptyItemSet();
}

void SvxOutlinerForwarder::AppendParagraph()
{
    EditEngine& rEditEngine = const_cast< EditEngine& >( mpOutliner->GetEditEngine() );
    rEditEngine.InsertParagraph( rEditEngine.GetParagraphCount(), OUString() );
}

sal_Int32 SvxOutlinerForwarder::AppendTextPortion( sal_Int32 nPara, const OUString &rText, const SfxItemSet & /*rSet*/ )
{
    sal_Int32 nLen = 0;

    EditEngine& rEditEngine = const_cast< EditEngine& >( mpOutliner->GetEditEngine() );
    sal_Int32 nParaCount = rEditEngine.GetParagraphCount();
    DBG_ASSERT( 0 <= nPara && nPara < nParaCount, "paragraph index out of bounds" );
    if (0 <= nPara && nPara < nParaCount)
    {
        nLen = rEditEngine.GetTextLen( nPara );
        rEditEngine.QuickInsertText( rText, ESelection( nPara, nLen, nPara, nLen ) );
    }

    return nLen;
}

void  SvxOutlinerForwarder::CopyText(const SvxTextForwarder& rSource)
{
    const SvxOutlinerForwarder* pSourceForwarder = dynamic_cast< const SvxOutlinerForwarder* >( &rSource );
    if( !pSourceForwarder )
        return;
    const std::shared_ptr< OutlinerParaObject > pNewOutlinerParaObject(pSourceForwarder->mpOutliner->CreateParaObject());
    mpOutliner->SetText( *pNewOutlinerParaObject );
}


sal_Int32 SvxTextForwarder::GetNumberingStartValue( sal_Int32 )
{
    return -1;
}

void SvxTextForwarder::SetNumberingStartValue( sal_Int32, sal_Int32 )
{
}

bool SvxTextForwarder::IsParaIsNumberingRestart( sal_Int32  )
{
    return false;
}

void SvxTextForwarder::SetParaIsNumberingRestart( sal_Int32, bool )
{
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
