/*************************************************************************
 *
 *  $RCSfile: textdata.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: mt $ $Date: 2000-11-08 10:42:11 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <textdata.hxx>
#include <textdat2.hxx>

#include <tools/debug.hxx>

SV_IMPL_PTRARR( TextLines, TextLinePtr );


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
    for ( USHORT nPortion = 0; nPortion < Count(); nPortion++ )
        delete GetObject( nPortion );
    Remove( 0, Count() );
}

void TETextPortionList::DeleteFromPortion( USHORT nDelFrom )
{
    DBG_ASSERT( ( nDelFrom < Count() ) || ( (nDelFrom == 0) && (Count() == 0) ), "DeleteFromPortion: Out of range" );
    for ( USHORT nP = nDelFrom; nP < Count(); nP++ )
        delete GetObject( nP );
    Remove( nDelFrom, Count()-nDelFrom );
}

USHORT TETextPortionList::FindPortion( USHORT nCharPos, USHORT& nPortionStart )
{
    // Bei nCharPos an Portion-Grenze wird die linke Portion gefunden
    USHORT nTmpPos = 0;
    for ( USHORT nPortion = 0; nPortion < Count(); nPortion++ )
    {
        TextPortion* pPortion = GetObject( nPortion );
        nTmpPos += pPortion->GetLen();
        if ( nTmpPos >= nCharPos )
        {
            nPortionStart = nTmpPos - pPortion->GetLen();
            return nPortion;
        }
    }
    DBG_ERROR( "FindPortion: Nicht gefunden!" );
    return ( Count() - 1 );
}

// -------------------------------------------------------------------------
// (+) class TEParaPortion
// -------------------------------------------------------------------------
TEParaPortion::TEParaPortion( TextNode* pN )
{
    mpNode = pN;
    mnInvalidPosStart = mnInvalidDiff = 0;
    mbInvalid = TRUE;
    mbSimple = FALSE;
}

TEParaPortion::~TEParaPortion()
{
}

void TEParaPortion::MarkInvalid( USHORT nStart, short nDiff )
{
    if ( mbInvalid == FALSE )
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
            mnInvalidDiff += nDiff;
        }
        // Einfaches hintereinander loeschen
        else if ( ( nDiff < 0 ) && ( mnInvalidDiff < 0 ) && ( mnInvalidPosStart == nStart ) )
        {
            mnInvalidPosStart += nDiff;
            mnInvalidDiff += nDiff;
        }
        else
        {
            DBG_ASSERT( ( nDiff >= 0 ) || ( (nStart+nDiff) >= 0 ), "MarkInvalid: Diff out of Range" );
            mnInvalidPosStart = Min( mnInvalidPosStart, (USHORT) ( (nDiff < 0) ? nStart+nDiff : nDiff ) );
            mnInvalidDiff = 0;
            mbSimple = FALSE;
        }
    }
    mbInvalid = TRUE;
}

void TEParaPortion::MarkSelectionInvalid( USHORT nStart, USHORT nEnd )
{
    if ( mbInvalid == FALSE )
    {
        mnInvalidPosStart = nStart;
//      nInvalidPosEnd = nEnd;
    }
    else
    {
        mnInvalidPosStart = Min( mnInvalidPosStart, nStart );
//      nInvalidPosEnd = pNode->Len();
    }
    mnInvalidDiff = 0;
    mbInvalid = TRUE;
    mbSimple = FALSE;
}

USHORT TEParaPortion::GetLineNumber( USHORT nChar, BOOL bInclEnd )
{
    for ( USHORT nLine = 0; nLine < maLines.Count(); nLine++ )
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


void TEParaPortion::CorrectValuesBehindLastFormattedLine( USHORT nLastFormattedLine )
{
    USHORT nLines = maLines.Count();
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
        short nPDiff = -( nPortionDiff-1 );
        short nTDiff = -( nTextDiff-1 );
        if ( nPDiff || nTDiff )
        {
            for ( USHORT nL = nLastFormattedLine+1; nL < nLines; nL++ )
            {
                TextLine* pLine = maLines[ nL ];

                pLine->GetStartPortion() += nPDiff;
                pLine->GetEndPortion() += nPDiff;

                pLine->GetStart() += nTDiff;
                pLine->GetEnd() += nTDiff;

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
    for ( USHORT nPortion = 0; nPortion < Count(); nPortion++ )
        delete GetObject( nPortion );
    Clear();
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

void IdleFormatter::DoIdleFormat( TextView* pV )
{
    mpView = pV;

    if ( IsActive() )
        mnRestarts++;

    if ( mnRestarts > 4 )
        ForceTimeout();
    else
        Start();
}

void IdleFormatter::ForceTimeout()
{
    if ( IsActive() )
    {
        Stop();
        ((Link&)GetTimeoutHdl()).Call( this );
    }
}

TYPEINIT1( TextHint, SfxSimpleHint );

TextHint::TextHint( ULONG nId ) : SfxSimpleHint( nId )
{
    mnValue = 0;
}

TextHint::TextHint( ULONG nId, ULONG nValue ) : SfxSimpleHint( nId )
{
    mnValue = nValue;
}

TEIMEInfos::TEIMEInfos( const TextPaM& rPos )
{
    aPos = rPos;
    nLen = 0;
    bCursor = TRUE;
    pAttribs = NULL;
    bWasCursorOverwrite = FALSE;
}

TEIMEInfos::~TEIMEInfos()
{
    delete pAttribs;
}

void TEIMEInfos::CopyAttribs( const USHORT* pA, USHORT nL )
{
    nLen = nL;
    delete pAttribs;
    pAttribs = new USHORT[ nL ];
    memcpy( pAttribs, pA, nL*sizeof(USHORT) );
}

void TEIMEInfos::DestroyAttribs()
{
    delete pAttribs;
    pAttribs = NULL;
}


