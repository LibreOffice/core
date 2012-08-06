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


#include <vcl/textdata.hxx>
#include <textdat2.hxx>

#include <tools/debug.hxx>


// -------------------------------------------------------------------------
// (+) class TextSelection
// -------------------------------------------------------------------------

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


// -------------------------------------------------------------------------
// (+) class TETextPortionList
// -------------------------------------------------------------------------
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
    // Bei nCharPos an Portion-Grenze wird die linke Portion gefunden
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


// -------------------------------------------------------------------------
// (+) class TEParaPortion
// -------------------------------------------------------------------------
TEParaPortion::TEParaPortion( TextNode* pN )
{
    mpNode = pN;
    mnInvalidPosStart = mnInvalidDiff = 0;
    mbInvalid = sal_True;
    mbSimple = sal_False;
}

TEParaPortion::~TEParaPortion()
{
}

void TEParaPortion::MarkInvalid( sal_uInt16 nStart, short nDiff )
{
    if ( mbInvalid == sal_False )
    {
        mnInvalidPosStart = ( nDiff >= 0 ) ? nStart : ( nStart + nDiff );
        mnInvalidDiff = nDiff;
    }
    else
    {
        // Einfaches hintereinander tippen
        if ( ( nDiff > 0 ) && ( mnInvalidDiff > 0 ) &&
             ( ( mnInvalidPosStart+mnInvalidDiff ) == nStart ) )
        {
            mnInvalidDiff = mnInvalidDiff + nDiff;
        }
        // Einfaches hintereinander loeschen
        else if ( ( nDiff < 0 ) && ( mnInvalidDiff < 0 ) && ( mnInvalidPosStart == nStart ) )
        {
            mnInvalidPosStart = mnInvalidPosStart + nDiff;
            mnInvalidDiff = mnInvalidDiff + nDiff;
        }
        else
        {
            DBG_ASSERT( ( nDiff >= 0 ) || ( (nStart+nDiff) >= 0 ), "MarkInvalid: Diff out of Range" );
            mnInvalidPosStart = Min( mnInvalidPosStart, (sal_uInt16) ( (nDiff < 0) ? nStart+nDiff : nDiff ) );
            mnInvalidDiff = 0;
            mbSimple = sal_False;
        }
    }

    maWritingDirectionInfos.clear();

    mbInvalid = sal_True;
}

void TEParaPortion::MarkSelectionInvalid( sal_uInt16 nStart, sal_uInt16 /*nEnd*/ )
{
    if ( mbInvalid == sal_False )
    {
        mnInvalidPosStart = nStart;
//      nInvalidPosEnd = nEnd;
    }
    else
    {
        mnInvalidPosStart = Min( mnInvalidPosStart, nStart );
//      nInvalidPosEnd = pNode->Len();
    }

    maWritingDirectionInfos.clear();

    mnInvalidDiff = 0;
    mbInvalid = sal_True;
    mbSimple = sal_False;
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
        nTextDiff++;    // LastFormatted->GetEnd() war incl. => 1 zuviel abgezogen!

        // Die erste unformatierte muss genau eine Portion hinter der letzten der
        // formatierten beginnen:
        // Wenn in der geaenderten Zeile eine Portion gesplittet wurde,
        // kann nLastEnd > nNextStart sein!
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

// -------------------------------------------------------------------------
// (+) class TEParaPortions
// -------------------------------------------------------------------------
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

// -------------------------------------------------------------------------
// (+) class IdleFormatter
// -------------------------------------------------------------------------
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

TEIMEInfos::TEIMEInfos( const TextPaM& rPos, const String& rOldTextAfterStartPos )
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
    delete pAttribs;
    pAttribs = new sal_uInt16[ nL ];
    memcpy( pAttribs, pA, nL*sizeof(sal_uInt16) );
}

void TEIMEInfos::DestroyAttribs()
{
    delete pAttribs;
    pAttribs = NULL;
    nLen = 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
