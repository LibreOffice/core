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

#include <vcl/textdata.hxx>
#include <textdat2.hxx>

#include <tools/debug.hxx>

TextSelection::TextSelection()
{
}

TextSelection::TextSelection( const TextPaM& rPaM ) :
    maStartPaM( rPaM ), maEndPaM( rPaM )
{
}

TextSelection::TextSelection( const TextPaM& rStart, const TextPaM& rEnd ) :
    maStartPaM( rStart ), maEndPaM( rEnd )
{
}

void TextSelection::Justify()
{
    if ( maEndPaM < maStartPaM )
    {
        TextPaM aTemp( maStartPaM );
        maStartPaM = maEndPaM;
        maEndPaM = aTemp;
    }
}

TETextPortionList::TETextPortionList()
{
}

TETextPortionList::~TETextPortionList()
{
    Reset();
}

TETextPortion* TETextPortionList::operator[]( size_t nPos )
{
    return maPortions[ nPos ];
}

std::vector<TETextPortion*>::iterator TETextPortionList::begin()
{
    return maPortions.begin();
}

std::vector<TETextPortion*>::const_iterator TETextPortionList::begin() const
{
    return maPortions.begin();
}

std::vector<TETextPortion*>::iterator TETextPortionList::end()
{
    return maPortions.end();
}

std::vector<TETextPortion*>::const_iterator TETextPortionList::end() const
{
    return maPortions.end();
}

bool TETextPortionList::empty() const
{
    return maPortions.empty();
}

size_t TETextPortionList::size() const
{
    return maPortions.size();
}

std::vector<TETextPortion*>::iterator TETextPortionList::erase( std::vector<TETextPortion*>::iterator aIter )
{
    return maPortions.erase( aIter );
}

std::vector<TETextPortion*>::iterator TETextPortionList::insert( std::vector<TETextPortion*>::iterator aIter,
                                                                 TETextPortion* pTP )
{
    return maPortions.insert( aIter, pTP );
}

void TETextPortionList::push_back( TETextPortion* pTP )
{
    maPortions.push_back( pTP );
}

void TETextPortionList::Reset()
{
    for ( auto pTP : maPortions )
        delete pTP;
    maPortions.clear();
}

void TETextPortionList::DeleteFromPortion( sal_uInt16 nDelFrom )
{
    SAL_WARN_IF( ( nDelFrom >= maPortions.size() ) && ( (nDelFrom != 0) || (maPortions.size() != 0) ), "vcl", "DeleteFromPortion: Out of range" );
    for ( auto it = maPortions.begin() + nDelFrom; it != maPortions.end(); ++it )
        delete *it;
    maPortions.erase( maPortions.begin() + nDelFrom, maPortions.end() );
}

sal_uInt16 TETextPortionList::FindPortion( sal_Int32 nCharPos, sal_Int32& nPortionStart, bool bPreferStartingPortion )
{
    // find left portion at nCharPos at portion border
    sal_Int32 nTmpPos = 0;
    for ( size_t nPortion = 0; nPortion < maPortions.size(); nPortion++ )
    {
        TETextPortion* pPortion = maPortions[ nPortion ];
        nTmpPos += pPortion->GetLen();
        if ( nTmpPos >= nCharPos )
        {
            // take this one if we don't prefer the starting portion, or if it's the last one
            if ( ( nTmpPos != nCharPos ) || !bPreferStartingPortion || ( nPortion == maPortions.size() - 1 ) )
            {
                nPortionStart = nTmpPos - pPortion->GetLen();
                return nPortion;
            }
        }
    }
    OSL_FAIL( "FindPortion: Nicht gefunden!" );
    return ( maPortions.size() - 1 );
}

TEParaPortion::TEParaPortion( TextNode* pN )
    : mpNode {pN}
    , mnInvalidPosStart {0}
    , mnInvalidDiff {0}
    , mbInvalid {true}
    , mbSimple {false}
{
}

TEParaPortion::~TEParaPortion()
{
}

void TEParaPortion::MarkInvalid( sal_Int32 nStart, sal_Int32 nDiff )
{
    if ( !mbInvalid )
    {
        mnInvalidPosStart = ( nDiff >= 0 ) ? nStart : ( nStart + nDiff );
        mnInvalidDiff = nDiff;
    }
    else
    {
        // simple consecutive typing
        if ( ( nDiff > 0 ) && ( mnInvalidDiff > 0 ) &&
             ( ( mnInvalidPosStart+mnInvalidDiff ) == nStart ) )
        {
            mnInvalidDiff = mnInvalidDiff + nDiff;
        }
        // simple consecutive deleting
        else if ( ( nDiff < 0 ) && ( mnInvalidDiff < 0 ) && ( mnInvalidPosStart == nStart ) )
        {
            mnInvalidPosStart = mnInvalidPosStart + nDiff;
            mnInvalidDiff = mnInvalidDiff + nDiff;
        }
        else
        {
            SAL_WARN_IF( ( nDiff < 0 ) && ( (nStart+nDiff) < 0 ), "vcl", "MarkInvalid: Diff out of Range" );
            mnInvalidPosStart = std::min( mnInvalidPosStart, nDiff < 0 ? nStart+nDiff : nDiff );
            mnInvalidDiff = 0;
            mbSimple = false;
        }
    }

    maWritingDirectionInfos.clear();

    mbInvalid = true;
}

void TEParaPortion::MarkSelectionInvalid( sal_Int32 nStart, sal_Int32 /*nEnd*/ )
{
    if ( !mbInvalid )
    {
        mnInvalidPosStart = nStart;
//      nInvalidPosEnd = nEnd;
    }
    else
    {
        mnInvalidPosStart = std::min( mnInvalidPosStart, nStart );
//      nInvalidPosEnd = pNode->Len();
    }

    maWritingDirectionInfos.clear();

    mnInvalidDiff = 0;
    mbInvalid = true;
    mbSimple = false;
}

sal_uInt16 TEParaPortion::GetLineNumber( sal_Int32 nChar, bool bInclEnd )
{
    for ( size_t nLine = 0; nLine < maLines.size(); nLine++ )
    {
        TextLine& rLine = maLines[ nLine ];
        if ( ( bInclEnd && ( rLine.GetEnd() >= nChar ) ) ||
             ( rLine.GetEnd() > nChar ) )
        {
            return nLine;
        }
    }

    // Then it should be at the end of the last line
    OSL_ENSURE(nChar == maLines.back().GetEnd(), "wrong Index");
    OSL_ENSURE(!bInclEnd, "Line not found: FindLine");
    return ( maLines.size() - 1 );
}

void TEParaPortion::CorrectValuesBehindLastFormattedLine( sal_uInt16 nLastFormattedLine )
{
    sal_uInt16 nLines = maLines.size();
    SAL_WARN_IF( !nLines, "vcl", "CorrectPortionNumbersFromLine: Leere Portion?" );
    if ( nLastFormattedLine < ( nLines - 1 ) )
    {
        const TextLine& rLastFormatted = maLines[ nLastFormattedLine ];
        const TextLine& rUnformatted = maLines[ nLastFormattedLine+1 ];
        short nPortionDiff = rUnformatted.GetStartPortion() - rLastFormatted.GetEndPortion();
        sal_Int32 nTextDiff = rUnformatted.GetStart() - rLastFormatted.GetEnd();
        nTextDiff++;    // LastFormatted.GetEnd() was inclusive => subtracted one too much!

        // The first unformatted one has to start exactly one portion past the last
        // formatted one.
        // If a portion got split in the changed row, nLastEnd could be > nNextStart!
        short nPDiff = sal::static_int_cast< short >(-( nPortionDiff-1 ));
        const sal_Int32 nTDiff = -( nTextDiff-1 );
        if ( nPDiff || nTDiff )
        {
            for ( sal_uInt16 nL = nLastFormattedLine+1; nL < nLines; nL++ )
            {
                TextLine& rLine = maLines[ nL ];

                rLine.GetStartPortion() = rLine.GetStartPortion() + nPDiff;
                rLine.GetEndPortion() = rLine.GetEndPortion() + nPDiff;

                rLine.GetStart() = rLine.GetStart() + nTDiff;
                rLine.GetEnd() = rLine.GetEnd() + nTDiff;

                rLine.SetValid();
            }
        }
    }
}

TEParaPortions::~TEParaPortions()
{
   std::vector<TEParaPortion*>::iterator aIter( mvData.begin() );
   while ( aIter != mvData.end() )
        delete *aIter++;
}

IdleFormatter::IdleFormatter()
{
    mpView = nullptr;
    mnRestarts = 0;
    SetPriority(SchedulerPriority::HIGH_IDLE);
}

IdleFormatter::~IdleFormatter()
{
    mpView = nullptr;
}

void IdleFormatter::DoIdleFormat( TextView* pV, sal_uInt16 nMaxRestarts )
{
    mpView = pV;

    if ( IsActive() )
        mnRestarts++;

    if ( mnRestarts > nMaxRestarts )
    {
        mnRestarts = 0;
        Invoke();
    }
    else
    {
        Start();
    }
}

void IdleFormatter::ForceTimeout()
{
    if ( IsActive() )
    {
        Stop();
        mnRestarts = 0;
        Invoke();
    }
}

TextHint::TextHint( sal_uInt32 Id ) : SfxHint( Id ), mnValue(0)
{
}

TextHint::TextHint( sal_uInt32 Id, sal_uLong nValue ) : SfxHint( Id ), mnValue(nValue)
{
}

TEIMEInfos::TEIMEInfos( const TextPaM& rPos, const OUString& rOldTextAfterStartPos )
: aOldTextAfterStartPos( rOldTextAfterStartPos )
{
    aPos = rPos;
    nLen = 0;
    bCursor = true;
    pAttribs = nullptr;
    bWasCursorOverwrite = false;
}

TEIMEInfos::~TEIMEInfos()
{
    delete[] pAttribs;
}

void TEIMEInfos::CopyAttribs(const ExtTextInputAttr* pA, sal_Int32 nL)
{
    nLen = nL;
    delete[] pAttribs;
    pAttribs = new ExtTextInputAttr[ nL ];
    memcpy( pAttribs, pA, nL*sizeof(ExtTextInputAttr) );
}

void TEIMEInfos::DestroyAttribs()
{
    delete[] pAttribs;
    pAttribs = nullptr;
    nLen = 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
