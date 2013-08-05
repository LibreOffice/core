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

void TETextPortionList::Reset()
{
    for ( iterator it = begin(); it != end(); ++it )
        delete *it;
    clear();
}

void TETextPortionList::DeleteFromPortion( sal_uInt16 nDelFrom )
{
    DBG_ASSERT( ( nDelFrom < size() ) || ( (nDelFrom == 0) && (size() == 0) ), "DeleteFromPortion: Out of range" );
    for ( iterator it = begin() + nDelFrom; it != end(); ++it )
        delete *it;
    erase( begin() + nDelFrom, end() );
}

sal_uInt16 TETextPortionList::FindPortion( sal_uInt16 nCharPos, sal_uInt16& nPortionStart, sal_Bool bPreferStartingPortion )
{
    // find left portion at nCharPos at portion border
    sal_uInt16 nTmpPos = 0;
    for ( sal_uInt16 nPortion = 0; nPortion < size(); nPortion++ )
    {
        TETextPortion* pPortion = operator[]( nPortion );
        nTmpPos = nTmpPos + pPortion->GetLen();
        if ( nTmpPos >= nCharPos )
        {
            // take this one if we don't prefer the starting portion, or if it's the last one
            if ( ( nTmpPos != nCharPos ) || !bPreferStartingPortion || ( nPortion == size() - 1 ) )
            {
                nPortionStart = nTmpPos - pPortion->GetLen();
                return nPortion;
            }
        }
    }
    OSL_FAIL( "FindPortion: Nicht gefunden!" );
    return ( size() - 1 );
}


TEParaPortion::TEParaPortion( TextNode* pN )
{
    mpNode = pN;
    mnInvalidPosStart = mnInvalidDiff = 0;
    mbInvalid = true;
    mbSimple = false;
}

TEParaPortion::~TEParaPortion()
{
}

void TEParaPortion::MarkInvalid( sal_uInt16 nStart, short nDiff )
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
            DBG_ASSERT( ( nDiff >= 0 ) || ( (nStart+nDiff) >= 0 ), "MarkInvalid: Diff out of Range" );
            mnInvalidPosStart = std::min( mnInvalidPosStart, (sal_uInt16) ( (nDiff < 0) ? nStart+nDiff : nDiff ) );
            mnInvalidDiff = 0;
            mbSimple = false;
        }
    }

    maWritingDirectionInfos.clear();

    mbInvalid = true;
}

void TEParaPortion::MarkSelectionInvalid( sal_uInt16 nStart, sal_uInt16 /*nEnd*/ )
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

sal_uInt16 TEParaPortion::GetLineNumber( sal_uInt16 nChar, sal_Bool bInclEnd )
{
    for ( sal_uInt16 nLine = 0; nLine < maLines.size(); nLine++ )
    {
        TextLine* pLine = maLines[ nLine ];
        if ( ( bInclEnd && ( pLine->GetEnd() >= nChar ) ) ||
             ( pLine->GetEnd() > nChar ) )
        {
            return nLine;
        }
    }

    // Then it should be at the end of the last line
    OSL_ENSURE(nChar == maLines[maLines.size() - 1]->GetEnd(), "wrong Index");
    OSL_ENSURE(!bInclEnd, "Line not found: FindLine");
    return ( maLines.size() - 1 );
}


void TEParaPortion::CorrectValuesBehindLastFormattedLine( sal_uInt16 nLastFormattedLine )
{
    sal_uInt16 nLines = maLines.size();
    DBG_ASSERT( nLines, "CorrectPortionNumbersFromLine: Leere Portion?" );
    if ( nLastFormattedLine < ( nLines - 1 ) )
    {
        const TextLine* pLastFormatted = maLines[ nLastFormattedLine ];
        const TextLine* pUnformatted = maLines[ nLastFormattedLine+1 ];
        short nPortionDiff = pUnformatted->GetStartPortion() - pLastFormatted->GetEndPortion();
        short nTextDiff = pUnformatted->GetStart() - pLastFormatted->GetEnd();
        nTextDiff++;    // LastFormatted->GetEnd() was inclusive => subtracted one too much!

        // The first unformated one has to start exactly one portion past the last
        // formated one.
        // If a portion got split in the changed row, nLastEnd could be > nNextStart!
        short nPDiff = sal::static_int_cast< short >(-( nPortionDiff-1 ));
        short nTDiff = sal::static_int_cast< short >(-( nTextDiff-1 ));
        if ( nPDiff || nTDiff )
        {
            for ( sal_uInt16 nL = nLastFormattedLine+1; nL < nLines; nL++ )
            {
                TextLine* pLine = maLines[ nL ];

                pLine->GetStartPortion() = pLine->GetStartPortion() + nPDiff;
                pLine->GetEndPortion() = pLine->GetEndPortion() + nPDiff;

                pLine->GetStart() = pLine->GetStart() + nTDiff;
                pLine->GetEnd() = pLine->GetEnd() + nTDiff;

                pLine->SetValid();
            }
        }
    }
}

TEParaPortions::TEParaPortions()
{
}

TEParaPortions::~TEParaPortions()
{
    Reset();
}

void TEParaPortions::Reset()
{
    TEParaPortions::iterator aIter( begin() );
    while ( aIter != end() )
        delete *aIter++;
    clear();
}

IdleFormatter::IdleFormatter()
{
    mpView = 0;
    mnRestarts = 0;
}

IdleFormatter::~IdleFormatter()
{
    mpView = 0;
}

void IdleFormatter::DoIdleFormat( TextView* pV, sal_uInt16 nMaxRestarts )
{
    mpView = pV;

    if ( IsActive() )
        mnRestarts++;

    if ( mnRestarts > nMaxRestarts )
    {
        mnRestarts = 0;
        ((Link&)GetTimeoutHdl()).Call( this );
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
        ((Link&)GetTimeoutHdl()).Call( this );
    }
}

TYPEINIT1( TextHint, SfxSimpleHint );

TextHint::TextHint( sal_uLong Id ) : SfxSimpleHint( Id )
{
    mnValue = 0;
}

TextHint::TextHint( sal_uLong Id, sal_uLong nValue ) : SfxSimpleHint( Id )
{
    mnValue = nValue;
}

TEIMEInfos::TEIMEInfos( const TextPaM& rPos, const OUString& rOldTextAfterStartPos )
: aOldTextAfterStartPos( rOldTextAfterStartPos )
{
    aPos = rPos;
    nLen = 0;
    bCursor = sal_True;
    pAttribs = NULL;
    bWasCursorOverwrite = sal_False;
}

TEIMEInfos::~TEIMEInfos()
{
    delete[] pAttribs;
}

void TEIMEInfos::CopyAttribs( const sal_uInt16* pA, sal_uInt16 nL )
{
    nLen = nL;
    delete[] pAttribs;
    pAttribs = new sal_uInt16[ nL ];
    memcpy( pAttribs, pA, nL*sizeof(sal_uInt16) );
}

void TEIMEInfos::DestroyAttribs()
{
    delete[] pAttribs;
    pAttribs = NULL;
    nLen = 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
