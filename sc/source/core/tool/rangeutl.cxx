/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: rangeutl.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 13:14:16 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------

#include <tools/debug.hxx>

#include "rangeutl.hxx"
#include "document.hxx"
#include "global.hxx"
#include "dbcolect.hxx"
#include "rangenam.hxx"
#include "scresid.hxx"
#include "globstr.hrc"
#ifndef SC_CONVUNO_HXX
#include "convuno.hxx"
#endif

using ::rtl::OUString;
using namespace ::com::sun::star;

//------------------------------------------------------------------------

BOOL ScRangeUtil::MakeArea( const String&   rAreaStr,
                            ScArea&         rArea,
                            ScDocument*     pDoc,
                            SCTAB           nTab,
                            ScAddress::Details const & rDetails ) const
{
    // Eingabe in rAreaStr: "$Tabelle1.$A1:$D17"

    // BROKEN BROKEN BROKEN
    // but it is only used in the consolidate dialog.  Ignore for now.

    BOOL        nSuccess    = FALSE;
    USHORT      nPointPos   = rAreaStr.Search('.');
    USHORT      nColonPos   = rAreaStr.Search(':');
    String      aStrArea( rAreaStr );
    ScRefAddress    startPos;
    ScRefAddress    endPos;

    if ( nColonPos == STRING_NOTFOUND )
        if ( nPointPos != STRING_NOTFOUND )
        {
            aStrArea += ':';
            aStrArea += rAreaStr.Copy( nPointPos+1 ); // '.' nicht mitkopieren
        }

    nSuccess = ConvertDoubleRef( pDoc, aStrArea, nTab, startPos, endPos, rDetails );

    if ( nSuccess )
        rArea = ScArea( startPos.Tab(),
                        startPos.Col(), startPos.Row(),
                        endPos.Col(),   endPos.Row() );

    return nSuccess;
}

//------------------------------------------------------------------------

void ScRangeUtil::CutPosString( const String&   theAreaStr,
                                String&         thePosStr ) const
{
    String  aPosStr;
    // BROKEN BROKEN BROKEN
    // but it is only used in the consolidate dialog.  Ignore for now.

    USHORT  nColonPos = theAreaStr.Search(':');

    if ( nColonPos != STRING_NOTFOUND )
        aPosStr = theAreaStr.Copy( 0, nColonPos ); // ':' nicht mitkopieren
    else
        aPosStr = theAreaStr;

    thePosStr = aPosStr;
}

//------------------------------------------------------------------------

BOOL ScRangeUtil::IsAbsTabArea( const String&   rAreaStr,
                                ScDocument*     pDoc,
                                ScArea***       pppAreas,
                                USHORT*         pAreaCount,
                                BOOL            /* bAcceptCellRef */,
                                ScAddress::Details const & rDetails ) const
{
    DBG_ASSERT( pDoc, "Kein Dokument uebergeben!" );
    if ( !pDoc )
        return FALSE;

    // BROKEN BROKEN BROKEN
    // but it is only used in the consolidate dialog.  Ignore for now.

    /*
     * Erwartet wird ein String der Form
     *      "$Tabelle1.$A$1:$Tabelle3.$D$17"
     * Wenn bAcceptCellRef == TRUE ist, wird auch ein String der Form
     *      "$Tabelle1.$A$1"
     * akzeptiert.
     *
     * als Ergebnis wird ein ScArea-Array angelegt,
     * welches ueber ppAreas bekannt gegeben wird und auch
     * wieder geloescht werden muss!
     */

    BOOL    bStrOk = FALSE;
    String  aTempAreaStr(rAreaStr);
    String  aStartPosStr;
    String  aEndPosStr;

    if ( STRING_NOTFOUND == aTempAreaStr.Search(':') )
    {
        aTempAreaStr.Append(':');
        aTempAreaStr.Append(rAreaStr);
    }

    USHORT   nColonPos = aTempAreaStr.Search(':');

    if (   STRING_NOTFOUND != nColonPos
        && STRING_NOTFOUND != aTempAreaStr.Search('.') )
    {
        ScRefAddress    aStartPos;
        ScRefAddress    aEndPos;

        aStartPosStr = aTempAreaStr.Copy( 0,           nColonPos  );
        aEndPosStr   = aTempAreaStr.Copy( nColonPos+1, STRING_LEN );

        if ( ConvertSingleRef( pDoc, aStartPosStr, 0, aStartPos, rDetails ) )
        {
            if ( ConvertSingleRef( pDoc, aEndPosStr, aStartPos.Tab(), aEndPos, rDetails ) )
            {
                aStartPos.SetRelCol( FALSE );
                aStartPos.SetRelRow( FALSE );
                aStartPos.SetRelTab( FALSE );
                aEndPos.SetRelCol( FALSE );
                aEndPos.SetRelRow( FALSE );
                aEndPos.SetRelTab( FALSE );

                bStrOk = TRUE;

                if ( pppAreas && pAreaCount ) // Array zurueckgegeben?
                {
                    SCTAB       nStartTab   = aStartPos.Tab();
                    SCTAB       nEndTab     = aEndPos.Tab();
                    USHORT      nTabCount   = static_cast<USHORT>(nEndTab-nStartTab+1);
                    ScArea**    theAreas    = new ScArea*[nTabCount];
                    SCTAB       nTab        = 0;
                    USHORT      i           = 0;
                    ScArea      theArea( 0, aStartPos.Col(), aStartPos.Row(),
                                            aEndPos.Col(), aEndPos.Row() );

                    nTab = nStartTab;
                    for ( i=0; i<nTabCount; i++ )
                    {
                        theAreas[i] = new ScArea( theArea );
                        theAreas[i]->nTab = nTab;
                        nTab++;
                    }
                    *pppAreas   = theAreas;
                    *pAreaCount = nTabCount;
                }
            }
        }
    }

    return bStrOk;
}

//------------------------------------------------------------------------

BOOL ScRangeUtil::IsAbsArea( const String&  rAreaStr,
                             ScDocument*    pDoc,
                             SCTAB          nTab,
                             String*        pCompleteStr,
                             ScRefAddress*  pStartPos,
                             ScRefAddress*  pEndPos,
                             ScAddress::Details const & rDetails ) const
{
    BOOL        bIsAbsArea = FALSE;
    ScRefAddress    startPos;
    ScRefAddress    endPos;

    bIsAbsArea = ConvertDoubleRef( pDoc, rAreaStr, nTab, startPos, endPos, rDetails );

    if ( bIsAbsArea )
    {
        startPos.SetRelCol( FALSE );
        startPos.SetRelRow( FALSE );
        startPos.SetRelTab( FALSE );
        endPos  .SetRelCol( FALSE );
        endPos  .SetRelRow( FALSE );
        endPos  .SetRelTab( FALSE );

        if ( pCompleteStr )
        {
            *pCompleteStr  = startPos.GetRefString( pDoc, MAXTAB+1, rDetails );
            *pCompleteStr += ':';
            *pCompleteStr += endPos  .GetRefString( pDoc, nTab, rDetails );
        }

        if ( pStartPos && pEndPos )
        {
            *pStartPos = startPos;
            *pEndPos   = endPos;
        }
    }

    return bIsAbsArea;
}

//------------------------------------------------------------------------

BOOL ScRangeUtil::IsAbsPos( const String&   rPosStr,
                            ScDocument*     pDoc,
                            SCTAB           nTab,
                            String*         pCompleteStr,
                            ScRefAddress*   pPosTripel,
                            ScAddress::Details const & rDetails ) const
{
    BOOL        bIsAbsPos = FALSE;
    ScRefAddress    thePos;

    bIsAbsPos = ConvertSingleRef( pDoc, rPosStr, nTab, thePos, rDetails );
    thePos.SetRelCol( FALSE );
    thePos.SetRelRow( FALSE );
    thePos.SetRelTab( FALSE );

    if ( bIsAbsPos )
    {
        if ( pPosTripel )
            *pPosTripel = thePos;
        if ( pCompleteStr )
            *pCompleteStr = thePos.GetRefString( pDoc, MAXTAB+1, rDetails );
    }

    return bIsAbsPos;
}

//------------------------------------------------------------------------

BOOL ScRangeUtil::MakeRangeFromName (
    const String&   rName,
    ScDocument*     pDoc,
    SCTAB           nCurTab,
    ScRange&        rRange,
    RutlNameScope   eScope,
    ScAddress::Details const & rDetails ) const
{
    BOOL bResult=FALSE;
    ScRangeUtil     aRangeUtil;
    SCTAB nTab = 0;
    SCCOL nColStart = 0;
    SCCOL nColEnd = 0;
    SCROW nRowStart = 0;
    SCROW nRowEnd = 0;

    if( eScope==RUTL_NAMES )
    {
        ScRangeName& rRangeNames = *(pDoc->GetRangeName());
        USHORT       nAt         = 0;

        if ( rRangeNames.SearchName( rName, nAt ) )
        {
            ScRangeData* pData = rRangeNames[nAt];
            String       aStrArea;
            ScRefAddress     aStartPos;
            ScRefAddress     aEndPos;

            pData->GetSymbol( aStrArea );

            if ( IsAbsArea( aStrArea, pDoc, nCurTab,
                            NULL, &aStartPos, &aEndPos, rDetails ) )
            {
                nTab       = aStartPos.Tab();
                nColStart  = aStartPos.Col();
                nRowStart  = aStartPos.Row();
                nColEnd    = aEndPos.Col();
                nRowEnd    = aEndPos.Row();
                bResult    = TRUE;
            }
            else
            {
                CutPosString( aStrArea, aStrArea );

                if ( IsAbsPos( aStrArea, pDoc, nCurTab,
                                          NULL, &aStartPos, rDetails ) )
                {
                    nTab       = aStartPos.Tab();
                    nColStart  = nColEnd = aStartPos.Col();
                    nRowStart  = nRowEnd = aStartPos.Row();
                    bResult    = TRUE;
                }
            }
        }
    }
    else if( eScope==RUTL_DBASE )
    {
        ScDBCollection& rDbNames = *(pDoc->GetDBCollection());
        USHORT          nAt = 0;

        if ( rDbNames.SearchName( rName, nAt ) )
        {
            ScDBData* pData = rDbNames[nAt];

            pData->GetArea( nTab, nColStart, nRowStart,
                                  nColEnd,   nRowEnd );
            bResult = TRUE;
        }
    }
    else
    {
        DBG_ERROR( "ScRangeUtil::MakeRangeFromName" );
    }

    if( bResult )
    {
        rRange = ScRange( nColStart, nRowStart, nTab, nColEnd, nRowEnd, nTab );
    }

    return bResult;
}

//========================================================================

void ScRangeStringConverter::AssignString(
        OUString& rString,
        const OUString& rNewStr,
        sal_Bool bAppendStr,
        sal_Unicode cSeperator)
{
    if( bAppendStr )
    {
        if( rNewStr.getLength() )
        {
            if( rString.getLength() )
                rString += rtl::OUString(cSeperator);
            rString += rNewStr;
        }
    }
    else
        rString = rNewStr;
}

sal_Int32 ScRangeStringConverter::IndexOf(
        const OUString& rString,
        sal_Unicode cSearchChar,
        sal_Int32 nOffset,
        sal_Unicode cQuote )
{
    sal_Int32       nLength     = rString.getLength();
    sal_Int32       nIndex      = nOffset;
    sal_Bool        bQuoted     = sal_False;
    sal_Bool        bExitLoop   = sal_False;

    while( !bExitLoop && (nIndex < nLength) )
    {
        sal_Unicode cCode = rString[ nIndex ];
        bExitLoop = (cCode == cSearchChar) && !bQuoted;
        bQuoted = (bQuoted != (cCode == cQuote));
        if( !bExitLoop )
            nIndex++;
    }
    return (nIndex < nLength) ? nIndex : -1;
}

sal_Int32 ScRangeStringConverter::IndexOfDifferent(
        const OUString& rString,
        sal_Unicode cSearchChar,
        sal_Int32 nOffset )
{
    sal_Int32       nLength     = rString.getLength();
    sal_Int32       nIndex      = nOffset;
    sal_Bool        bExitLoop   = sal_False;

    while( !bExitLoop && (nIndex < nLength) )
    {
        bExitLoop = (rString[ nIndex ] != cSearchChar);
        if( !bExitLoop )
            nIndex++;
    }
    return (nIndex < nLength) ? nIndex : -1;
}

void ScRangeStringConverter::GetTokenByOffset(
        OUString& rToken,
        const OUString& rString,
        sal_Int32& nOffset,
        sal_Unicode cSeperator,
        sal_Unicode cQuote)
{
    sal_Int32 nLength = rString.getLength();
    if( nOffset >= nLength )
    {
        rToken = OUString();
        nOffset = -1;
    }
    else
    {
        sal_Int32 nTokenEnd = IndexOf( rString, cSeperator, nOffset, cQuote );
        if( nTokenEnd < 0 )
            nTokenEnd = nLength;
        rToken = rString.copy( nOffset, nTokenEnd - nOffset );

        sal_Int32 nNextBegin = IndexOfDifferent( rString, cSeperator, nTokenEnd );
        nOffset = (nNextBegin < 0) ? nLength : nNextBegin;
    }
}

sal_Int32 ScRangeStringConverter::GetTokenCount( const OUString& rString, sal_Unicode cSeperator, sal_Unicode cQuote )
{
    OUString    sToken;
    sal_Int32   nCount = 0;
    sal_Int32   nOffset = 0;
    while( nOffset >= 0 )
    {
        GetTokenByOffset( sToken, rString, nOffset, cQuote, cSeperator );
        if( nOffset >= 0 )
            nCount++;
    }
    return nCount;
}

void ScRangeStringConverter::AppendString( OUString& rString, const OUString& rNewStr, sal_Unicode cSeperator )
{
    AssignString( rString, rNewStr, sal_True, cSeperator );
}

//___________________________________________________________________

sal_Bool ScRangeStringConverter::GetAddressFromString(
        ScAddress& rAddress,
        const OUString& rAddressStr,
        const ScDocument* pDocument,
        sal_Int32& nOffset,
        sal_Unicode cSeperator,
        sal_Unicode cQuote )
{
    OUString sToken;
    GetTokenByOffset( sToken, rAddressStr, nOffset, cSeperator, cQuote );
    if( nOffset >= 0 )
        return ((rAddress.Parse( sToken, (ScDocument*) pDocument ) & SCA_VALID) == SCA_VALID);
    return sal_False;
}

sal_Bool ScRangeStringConverter::GetRangeFromString(
        ScRange& rRange,
        const OUString& rRangeStr,
        const ScDocument* pDocument,
        sal_Int32& nOffset,
        sal_Unicode cSeperator,
        sal_Unicode cQuote )
{
    OUString sToken;
    sal_Bool bResult(sal_False);
    GetTokenByOffset( sToken, rRangeStr, nOffset, cSeperator, cQuote );
    if( nOffset >= 0 )
    {
        sal_Int32 nIndex = IndexOf( sToken, ':', 0, cQuote );
        String aUIString(sToken);

        if( nIndex < 0 )
        {
            if ( aUIString.GetChar(0) == (sal_Unicode) '.' )
                aUIString.Erase( 0, 1 );
            bResult = ((rRange.aStart.Parse( aUIString, const_cast<ScDocument*> (pDocument)) & SCA_VALID) == SCA_VALID);
            rRange.aEnd = rRange.aStart;
        }
        else
        {
            if ( aUIString.GetChar(0) == (sal_Unicode) '.' )
                aUIString.Erase( 0, 1 );

            if ( nIndex < aUIString.Len() - 1 &&
                    aUIString.GetChar((xub_StrLen)nIndex + 1) == (sal_Unicode) '.' )
                aUIString.Erase( (xub_StrLen)nIndex + 1, 1 );

            bResult = ((rRange.Parse(aUIString, const_cast<ScDocument*> (pDocument)) & SCA_VALID) == SCA_VALID);
        }
    }
    return bResult;
}

sal_Bool ScRangeStringConverter::GetRangeListFromString(
        ScRangeList& rRangeList,
        const OUString& rRangeListStr,
        const ScDocument* pDocument,
        sal_Unicode cSeperator,
        sal_Unicode cQuote )
{
    sal_Bool bRet = sal_True;
    DBG_ASSERT( rRangeListStr.getLength(), "ScXMLConverter::GetRangeListFromString - empty string!" );
    sal_Int32 nOffset = 0;
    while( nOffset >= 0 )
    {
        ScRange* pRange = new ScRange;
        if( GetRangeFromString( *pRange, rRangeListStr, pDocument, nOffset, cSeperator, cQuote ) && (nOffset >= 0) )
            rRangeList.Insert( pRange, LIST_APPEND );
        else if (nOffset > -1)
            bRet = sal_False;
    }
    return bRet;
}


//___________________________________________________________________

sal_Bool ScRangeStringConverter::GetAreaFromString(
        ScArea& rArea,
        const OUString& rRangeStr,
        const ScDocument* pDocument,
        sal_Int32& nOffset,
        sal_Unicode cSeperator,
        sal_Unicode cQuote )
{
    ScRange aScRange;
    sal_Bool bResult(sal_False);
    if( GetRangeFromString( aScRange, rRangeStr, pDocument, nOffset, cSeperator, cQuote ) && (nOffset >= 0) )
    {
        rArea.nTab = aScRange.aStart.Tab();
        rArea.nColStart = aScRange.aStart.Col();
        rArea.nRowStart = aScRange.aStart.Row();
        rArea.nColEnd = aScRange.aEnd.Col();
        rArea.nRowEnd = aScRange.aEnd.Row();
        bResult = sal_True;
    }
    return bResult;
}


//___________________________________________________________________

sal_Bool ScRangeStringConverter::GetAddressFromString(
        table::CellAddress& rAddress,
        const OUString& rAddressStr,
        const ScDocument* pDocument,
        sal_Int32& nOffset,
        sal_Unicode cSeperator,
        sal_Unicode cQuote )
{
    ScAddress aScAddress;
    sal_Bool bResult(sal_False);
    if( GetAddressFromString( aScAddress, rAddressStr, pDocument, nOffset, cSeperator, cQuote ) && (nOffset >= 0) )
    {
        ScUnoConversion::FillApiAddress( rAddress, aScAddress );
        bResult = sal_True;
    }
    return bResult;
}

sal_Bool ScRangeStringConverter::GetRangeFromString(
        table::CellRangeAddress& rRange,
        const OUString& rRangeStr,
        const ScDocument* pDocument,
        sal_Int32& nOffset,
        sal_Unicode cSeperator,
        sal_Unicode cQuote )
{
    ScRange aScRange;
    sal_Bool bResult(sal_False);
    if( GetRangeFromString( aScRange, rRangeStr, pDocument, nOffset, cSeperator, cQuote ) && (nOffset >= 0) )
    {
        ScUnoConversion::FillApiRange( rRange, aScRange );
        bResult = sal_True;
    }
    return bResult;
}

sal_Bool ScRangeStringConverter::GetRangeListFromString(
        uno::Sequence< table::CellRangeAddress >& rRangeSeq,
        const OUString& rRangeListStr,
        const ScDocument* pDocument,
        sal_Unicode cSeperator,
        sal_Unicode cQuote )
{
    sal_Bool bRet = sal_True;
    DBG_ASSERT( rRangeListStr.getLength(), "ScXMLConverter::GetRangeListFromString - empty string!" );
    table::CellRangeAddress aRange;
    sal_Int32 nOffset = 0;
    while( nOffset >= 0 )
    {
        if( GetRangeFromString( aRange, rRangeListStr, pDocument, nOffset, cSeperator, cQuote ) && (nOffset >= 0) )
        {
            rRangeSeq.realloc( rRangeSeq.getLength() + 1 );
            rRangeSeq[ rRangeSeq.getLength() - 1 ] = aRange;
        }
        else
            bRet = sal_False;
    }
    return bRet;
}


//___________________________________________________________________

void ScRangeStringConverter::GetStringFromAddress(
        OUString& rString,
        const ScAddress& rAddress,
        const ScDocument* pDocument,
        sal_Unicode cSeperator,
        sal_Bool bAppendStr,
        sal_uInt16 nFormatFlags )
{
    if (pDocument && pDocument->HasTable(rAddress.Tab()))
    {
        String sAddress;
        rAddress.Format( sAddress, nFormatFlags, (ScDocument*) pDocument );
        AssignString( rString, sAddress, bAppendStr, cSeperator );
    }
}

void ScRangeStringConverter::GetStringFromRange(
        OUString& rString,
        const ScRange& rRange,
        const ScDocument* pDocument,
        sal_Unicode cSeperator,
        sal_Bool bAppendStr,
        sal_uInt16 nFormatFlags )
{
    if (pDocument && pDocument->HasTable(rRange.aStart.Tab()))
    {
        ScAddress aStartAddress( rRange.aStart );
        ScAddress aEndAddress( rRange.aEnd );
        String sStartAddress;
        String sEndAddress;
        aStartAddress.Format( sStartAddress, nFormatFlags, (ScDocument*) pDocument,
                              ScAddress::CONV_OOO );
        aEndAddress.Format( sEndAddress, nFormatFlags, (ScDocument*) pDocument,
                            ScAddress::CONV_OOO );
        OUString sOUStartAddress( sStartAddress );
        sOUStartAddress += OUString(':');
        sOUStartAddress += OUString( sEndAddress );
        AssignString( rString, sOUStartAddress, bAppendStr, cSeperator );
    }
}

void ScRangeStringConverter::GetStringFromRangeList(
        OUString& rString,
        const ScRangeList* pRangeList,
        const ScDocument* pDocument,
        sal_Unicode cSeperator,
        sal_uInt16 nFormatFlags )
{
    OUString sRangeListStr;
    if( pRangeList )
    {
        sal_Int32 nCount = pRangeList->Count();
        for( sal_Int32 nIndex = 0; nIndex < nCount; nIndex++ )
        {
            const ScRange* pRange = pRangeList->GetObject( nIndex );
            if( pRange )
                GetStringFromRange( sRangeListStr, *pRange, pDocument, cSeperator, sal_True, nFormatFlags );
        }
    }
    rString = sRangeListStr;
}


//___________________________________________________________________

void ScRangeStringConverter::GetStringFromArea(
        OUString& rString,
        const ScArea& rArea,
        const ScDocument* pDocument,
        sal_Unicode cSeperator,
        sal_Bool bAppendStr,
        sal_uInt16 nFormatFlags )
{
    ScRange aRange( rArea.nColStart, rArea.nRowStart, rArea.nTab, rArea.nColEnd, rArea.nRowEnd, rArea.nTab );
    GetStringFromRange( rString, aRange, pDocument, cSeperator, bAppendStr, nFormatFlags );
}


//___________________________________________________________________

void ScRangeStringConverter::GetStringFromAddress(
        OUString& rString,
        const table::CellAddress& rAddress,
        const ScDocument* pDocument,
        sal_Unicode cSeperator,
        sal_Bool bAppendStr,
        sal_uInt16 nFormatFlags )
{
    ScAddress aScAddress( static_cast<SCCOL>(rAddress.Column), static_cast<SCROW>(rAddress.Row), rAddress.Sheet );
    GetStringFromAddress( rString, aScAddress, pDocument, cSeperator, bAppendStr, nFormatFlags );
}

void ScRangeStringConverter::GetStringFromRange(
        OUString& rString,
        const table::CellRangeAddress& rRange,
        const ScDocument* pDocument,
        sal_Unicode cSeperator,
        sal_Bool bAppendStr,
        sal_uInt16 nFormatFlags )
{
    ScRange aScRange( static_cast<SCCOL>(rRange.StartColumn), static_cast<SCROW>(rRange.StartRow), rRange.Sheet,
        static_cast<SCCOL>(rRange.EndColumn), static_cast<SCROW>(rRange.EndRow), rRange.Sheet );
    GetStringFromRange( rString, aScRange, pDocument, cSeperator, bAppendStr, nFormatFlags );
}

void ScRangeStringConverter::GetStringFromRangeList(
        OUString& rString,
        const uno::Sequence< table::CellRangeAddress >& rRangeSeq,
        const ScDocument* pDocument,
        sal_Unicode cSeperator,
        sal_uInt16 nFormatFlags )
{
    OUString sRangeListStr;
    sal_Int32 nCount = rRangeSeq.getLength();
    for( sal_Int32 nIndex = 0; nIndex < nCount; nIndex++ )
    {
        const table::CellRangeAddress& rRange = rRangeSeq[ nIndex ];
        GetStringFromRange( sRangeListStr, rRange, pDocument, cSeperator, sal_True, nFormatFlags );
    }
    rString = sRangeListStr;
}

//========================================================================

ScArea::ScArea( SCTAB tab,
                SCCOL colStart, SCROW rowStart,
                SCCOL colEnd,   SCROW rowEnd ) :
        nTab     ( tab ),
        nColStart( colStart ),  nRowStart( rowStart ),
        nColEnd  ( colEnd ),    nRowEnd  ( rowEnd )
{
}

//------------------------------------------------------------------------

ScArea::ScArea( const ScArea& r ) :
        nTab     ( r.nTab ),
        nColStart( r.nColStart ),   nRowStart( r.nRowStart ),
        nColEnd  ( r.nColEnd ),     nRowEnd  ( r.nRowEnd )
{
}

//------------------------------------------------------------------------

void ScArea::Clear()
{
    nTab = 0;
    nColStart = nColEnd = 0;
    nRowStart = nRowEnd = 0;
}

//------------------------------------------------------------------------

ScArea& ScArea::operator=( const ScArea& r )
{
    nTab        = r.nTab;
    nColStart   = r.nColStart;
    nRowStart   = r.nRowStart;
    nColEnd     = r.nColEnd;
    nRowEnd     = r.nRowEnd;
    return *this;
}

//------------------------------------------------------------------------

BOOL ScArea::operator==( const ScArea& r ) const
{
    return (   (nTab        == r.nTab)
            && (nColStart   == r.nColStart)
            && (nRowStart   == r.nRowStart)
            && (nColEnd     == r.nColEnd)
            && (nRowEnd     == r.nRowEnd) );
}

//------------------------------------------------------------------------

SvStream& operator>> ( SvStream& rStream, ScArea& /* rArea */ )
{
#if SC_ROWLIMIT_STREAM_ACCESS
#error address types changed!
    rStream >> rArea.nTab;
    rStream >> rArea.nColStart;
    rStream >> rArea.nRowStart;
    rStream >> rArea.nColEnd;
    rStream >> rArea.nRowEnd;
#endif // SC_ROWLIMIT_STREAM_ACCESS
    return rStream;
}

//------------------------------------------------------------------------

SvStream& operator<< ( SvStream& rStream, const ScArea& /* rArea */ )
{
#if SC_ROWLIMIT_STREAM_ACCESS
#error address types changed!
    rStream << rArea.nTab;
    rStream << rArea.nColStart;
    rStream << rArea.nRowStart;
    rStream << rArea.nColEnd;
    rStream << rArea.nRowEnd;
#endif // SC_ROWLIMIT_STREAM_ACCESS
    return rStream;
}

//------------------------------------------------------------------------

void ScArea::GetString( String& rStr, BOOL bAbsolute, ScDocument* pDoc,
                        ScAddress::Details const & rDetails ) const
{
    ScRange aRange( ScAddress( nColStart, nRowStart, nTab ),
                    ScAddress( nColEnd,   nRowEnd,   nTab ) );
    USHORT  nFlags = bAbsolute ? SCA_COL_ABSOLUTE | SCA_ROW_ABSOLUTE : 0;

    aRange.Format( rStr, nFlags, pDoc, rDetails );
}

//------------------------------------------------------------------------

ScAreaNameIterator::ScAreaNameIterator( ScDocument* pDoc ) :
    aStrNoName( ScGlobal::GetRscString(STR_DB_NONAME) )
{
    pRangeName = pDoc->GetRangeName();
    pDBCollection = pDoc->GetDBCollection();
    nPos = 0;
    bFirstPass = TRUE;
}

BOOL ScAreaNameIterator::Next( String& rName, ScRange& rRange )
{
    for (;;)
    {
        if ( bFirstPass )                                   // erst Bereichsnamen
        {
            if ( pRangeName && nPos < pRangeName->GetCount() )
            {
                ScRangeData* pData = (*pRangeName)[nPos++];
                if ( pData && pData->IsValidReference(rRange) )
                {
                    rName = pData->GetName();
                    return TRUE;                            // gefunden
                }
            }
            else
            {
                bFirstPass = FALSE;
                nPos = 0;
            }
        }
        if ( !bFirstPass )                                  // dann DB-Bereiche
        {
            if ( pDBCollection && nPos < pDBCollection->GetCount() )
            {
                ScDBData* pData = (*pDBCollection)[nPos++];
                if (pData && pData->GetName() != aStrNoName)
                {
                    pData->GetArea( rRange );
                    rName = pData->GetName();
                    return TRUE;                            // gefunden
                }
            }
            else
                return FALSE;                               // gibt nichts mehr
        }
    }
}




