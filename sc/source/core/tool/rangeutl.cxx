/*************************************************************************
 *
 *  $RCSfile: rangeutl.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2004-06-04 10:38:52 $
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

#ifdef PCH
#include "core_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#include <tools/debug.hxx>

#include "rangeutl.hxx"
#include "document.hxx"
#include "global.hxx"
#include "dbcolect.hxx"
#include "rangenam.hxx"
#include "scresid.hxx"
#include "globstr.hrc"


//------------------------------------------------------------------------

BOOL ScRangeUtil::MakeArea( const String&   rAreaStr,
                            ScArea&         rArea,
                            ScDocument*     pDoc,
                            SCTAB           nTab ) const
{
    // Eingabe in rAreaStr: "$Tabelle1.$A1:$D17"

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

    nSuccess = ConvertDoubleRef( pDoc, aStrArea, nTab, startPos, endPos );

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
                                BOOL            bAcceptCellRef ) const
{
    DBG_ASSERT( pDoc, "Kein Dokument uebergeben!" );
    if ( !pDoc )
        return FALSE;

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

        if ( ConvertSingleRef( pDoc, aStartPosStr, 0, aStartPos ) )
        {
            if ( ConvertSingleRef( pDoc, aEndPosStr, aStartPos.Tab(), aEndPos ) )
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
                             ScRefAddress*  pEndPos ) const
{
    BOOL        bIsAbsArea = FALSE;
    ScRefAddress    startPos;
    ScRefAddress    endPos;

    bIsAbsArea = ConvertDoubleRef( pDoc, rAreaStr, nTab, startPos, endPos );

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
            *pCompleteStr  = startPos.GetRefString( pDoc, MAXTAB+1 );
            *pCompleteStr += ':';
            *pCompleteStr += endPos  .GetRefString( pDoc, nTab );
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
                            ScRefAddress*   pPosTripel ) const
{
    BOOL        bIsAbsPos = FALSE;
    ScRefAddress    thePos;

    bIsAbsPos = ConvertSingleRef( pDoc, rPosStr, nTab, thePos );

    thePos.SetRelCol( FALSE );
    thePos.SetRelRow( FALSE );
    thePos.SetRelTab( FALSE );

    if ( bIsAbsPos )
    {
        if ( pPosTripel )
            *pPosTripel = thePos;
        if ( pCompleteStr )
            *pCompleteStr = thePos.GetRefString( pDoc, MAXTAB+1 );
    }

    return bIsAbsPos;
}

//------------------------------------------------------------------------

BOOL ScRangeUtil::MakeRangeFromName (
    const String&   rName,
    ScDocument*     pDoc,
    SCTAB           nCurTab,
    ScRange&        rRange,
    RutlNameScope   eScope
                                  ) const
{
    BOOL bResult=FALSE;
    ScRangeUtil     aRangeUtil;
    SCTAB nTab;
    SCCOL nColStart;
    SCCOL nColEnd;
    SCROW nRowStart;
    SCROW nRowEnd;

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
                                       NULL, &aStartPos, &aEndPos ) )
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
                                          NULL, &aStartPos ) )
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

SvStream& operator>> ( SvStream& rStream, ScArea& rArea )
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

SvStream& operator<< ( SvStream& rStream, const ScArea& rArea )
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

void ScArea::GetString( String& rStr, BOOL bAbsolute, ScDocument* pDoc ) const
{
    ScRange aRange( ScAddress( nColStart, nRowStart, nTab ),
                    ScAddress( nColEnd,   nRowEnd,   nTab ) );
    USHORT  nFlags = bAbsolute ? SCA_COL_ABSOLUTE | SCA_ROW_ABSOLUTE : 0;

    aRange.Format( rStr, nFlags, pDoc );
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
                if ( pData && pData->IsReference(rRange) )
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




