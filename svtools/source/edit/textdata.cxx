/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"

#include <svtools/textdata.hxx>
#include <textdat2.hxx>

#include <tools/debug.hxx>

SV_IMPL_PTRARR( TextLines, TextLinePtr );
SV_IMPL_VARARR( TEWritingDirectionInfos, TEWritingDirectionInfo );


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
    for ( sal_uInt16 nPortion = 0; nPortion < Count(); nPortion++ )
        delete GetObject( nPortion );
    Remove( 0, Count() );
}

void TETextPortionList::DeleteFromPortion( sal_uInt16 nDelFrom )
{
    DBG_ASSERT( ( nDelFrom < Count() ) || ( (nDelFrom == 0) && (Count() == 0) ), "DeleteFromPortion: Out of range" );
    for ( sal_uInt16 nP = nDelFrom; nP < Count(); nP++ )
        delete GetObject( nP );
    Remove( nDelFrom, Count()-nDelFrom );
}

sal_uInt16 TETextPortionList::FindPortion( sal_uInt16 nCharPos, sal_uInt16& nPortionStart, sal_Bool bPreferStartingPortion )
{
    // Bei nCharPos an Portion-Grenze wird die linke Portion gefunden
    sal_uInt16 nTmpPos = 0;
    for ( sal_uInt16 nPortion = 0; nPortion < Count(); nPortion++ )
    {
        TETextPortion* pPortion = GetObject( nPortion );
        nTmpPos = nTmpPos + pPortion->GetLen();
        if ( nTmpPos >= nCharPos )
        {
            // take this one if we don't prefer the starting portion, or if it's the last one
            if ( ( nTmpPos != nCharPos ) || !bPreferStartingPortion || ( nPortion == Count() - 1 ) )
            {
                nPortionStart = nTmpPos - pPortion->GetLen();
                return nPortion;
            }
        }
    }
    DBG_ERROR( "FindPortion: Nicht gefunden!" );
    return ( Count() - 1 );
}

/*
sal_uInt16 TETextPortionList::GetPortionStartIndex( sal_uInt16 nPortion )
{
    sal_uInt16 nPos = 0;
    for ( sal_uInt16 nP = 0; nP < nPortion; nP++ )
    {
        TETextPortion* pPortion = GetObject( nP );
        nPos += pPortion->GetLen();
    }
    return nPos;
}
*/


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

    maWritingDirectionInfos.Remove( 0, maWritingDirectionInfos.Count() );

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

    maWritingDirectionInfos.Remove( 0, maWritingDirectionInfos.Count() );

    mnInvalidDiff = 0;
    mbInvalid = sal_True;
    mbSimple = sal_False;
}

sal_uInt16 TEParaPortion::GetLineNumber( sal_uInt16 nChar, sal_Bool bInclEnd )
{
    for ( sal_uInt16 nLine = 0; nLine < maLines.Count(); nLine++ )
    {
        TextLine* pLine = maLines.GetObject( nLine );
        if ( ( bInclEnd && ( pLine->GetEnd() >= nChar ) ) ||
             ( pLine->GetEnd() > nChar ) )
        {
            return nLine;
        }
    }

    // Dann sollte es am Ende der letzten Zeile sein!
    DBG_ASSERT( nChar == maLines[ maLines.Count() - 1 ]->GetEnd(), "Index voll daneben!" );
    DBG_ASSERT( !bInclEnd, "Zeile nicht gefunden: FindLine" );
    return ( maLines.Count() - 1 );
}


void TEParaPortion::CorrectValuesBehindLastFormattedLine( sal_uInt16 nLastFormattedLine )
{
    sal_uInt16 nLines = maLines.Count();
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


