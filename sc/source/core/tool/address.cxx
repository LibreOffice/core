/*************************************************************************
 *
 *  $RCSfile: address.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $  $Date: 2004-06-04 10:32:48 $
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
 *  Copyright 2004 by Sun Microsystems, Inc.
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
 *  Copyright: 2004 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#pragma hdrstop

#include "address.hxx"
#include "global.hxx"
#include "compiler.hxx"
#include "document.hxx"

#include "globstr.hrc"


USHORT lcl_ConvertSingleRef( BOOL& bExternal, const sal_Unicode* p,
            ScDocument* pDoc, ScAddress& rAddr )
{
    if ( !*p )
        return 0;
    USHORT  nRes = 0;
    String  aDocName;       // der pure Dokumentenname
    String  aDocTab;        // zusammengesetzt fuer Table
    String  aTab;
    BOOL    bExtDoc = FALSE;
    BOOL    bNeedExtTab = FALSE;

    if ( *p == '\'' && ScGlobal::UnicodeStrChr( p, SC_COMPILER_FILE_TAB_SEP ) )
    {
        BOOL bQuote = TRUE;         // Dokumentenname ist immer quoted
        aDocTab += *p++;
        while ( bQuote && *p )
        {
            if ( *p == '\'' && *(p-1) != '\\' )
                bQuote = FALSE;
            else if( !(*p == '\\' && *(p+1) == '\'') )
                aDocName += *p;     // falls escaped Quote: nur Quote in den Namen
            aDocTab += *p++;
        }
        aDocTab += *p;              // den SC_COMPILER_FILE_TAB_SEP mitnehmen
        if( *p++ == SC_COMPILER_FILE_TAB_SEP )
            bExtDoc = TRUE;
        else
            return nRes;
    }

    SCCOL   nCol = 0;
    SCROW   nRow = 0;
    SCTAB   nTab = 0;
    USHORT  nBits = SCA_VALID_TAB;
    const sal_Unicode* q;
    if ( ScGlobal::UnicodeStrChr( p, '.') )
    {
        nRes |= SCA_TAB_3D;
        if ( bExtDoc )
            nRes |= SCA_TAB_ABSOLUTE;
        if (*p == '$')
            nRes |= SCA_TAB_ABSOLUTE, p++;
        BOOL bQuote = FALSE;
        if( *p == '\'' )
            p++, bQuote = TRUE;
        while (*p && (*p != '.'))
        {
            if( bQuote && *p == '\'' )
            {
                p++; break;
            }
            aTab += *p++;
        }
        if( *p++ != '.' )
            nBits = 0;
        if ( pDoc )
        {
            if ( bExtDoc )
            {
                bExternal = TRUE;
                aDocTab += aTab;    // "'Doc'#Tab"
                if ( !pDoc->GetTable( aDocTab, nTab ) )
                {
                    if ( pDoc->ValidTabName( aTab ) )
                    {
                        aDocName = ScGlobal::GetAbsDocName( aDocName,
                            pDoc->GetDocumentShell() );
                        aDocTab = ScGlobal::GetDocTabName( aDocName, aTab );
                        if ( !pDoc->GetTable( aDocTab, nTab ) )
                        {
                            // erst einfuegen, wenn Rest der Ref ok
                            bNeedExtTab = TRUE;
                            nBits = 0;
                        }
                    }
                    else
                        nBits = 0;
                }
            }
            else
            {
                if ( !pDoc->GetTable( aTab, nTab ) )
                    nBits = 0;
            }
        }
        else
            nBits = 0;
    }
    else
    {
        if ( bExtDoc )
            return nRes;        // nach Dokument muss Tabelle folgen
        nTab = rAddr.Tab();
    }
    nRes |= nBits;

    q = p;
    if (*p)
    {
        nBits = SCA_VALID_COL;
        if (*p == '$')
            nBits |= SCA_COL_ABSOLUTE, p++;

        if (CharClass::isAsciiAlpha( *p ))
        {
            nCol = toupper( char(*p++) ) - 'A';
            while (nCol < MAXCOL && CharClass::isAsciiAlpha(*p))
                nCol = ((nCol + 1) * 26) + toupper( char(*p++) ) - 'A';
        }
        else
            nBits = 0;

        if( nCol > MAXCOL || CharClass::isAsciiAlpha( *p ) )
            nBits = 0;
        nRes |= nBits;
        if( !nBits )
            p = q;
    }

    q = p;
    if (*p)
    {
        nBits = SCA_VALID_ROW;
        if (*p == '$')
            nBits |= SCA_ROW_ABSOLUTE, p++;
        if( !CharClass::isAsciiDigit( *p ) )
        {
            nBits = 0;
            nRow = SCROW(-1);
        }
        else
        {
            String aTmp( p );
            long n = aTmp.ToInt32() - 1;
            while (CharClass::isAsciiDigit( *p ))
                p++;
            if( n < 0 || n > MAXROW )
                nBits = 0;
            nRow = static_cast<SCROW>(n);
        }
        nRes |= nBits;
        if( !nBits )
            p = q;
    }
    if ( bNeedExtTab )
    {
        if ( (nRes & SCA_VALID_ROW) && (nRes & SCA_VALID_COL)
          && pDoc->LinkExternalTab( nTab, aDocTab, aDocName, aTab ) )
        {
            nRes |= SCA_VALID_TAB;
        }
        else
            nRes = 0;   // #NAME? statt #REF!, Dateiname bleibt erhalten
    }
    if ( !(nRes & SCA_VALID_ROW) && (nRes & SCA_VALID_COL)
            && !( (nRes & SCA_TAB_3D) && (nRes & SCA_VALID_TAB)) )
    {   // keine Row, keine Tab, aber Col => DM (...), B (...) o.ae.
        nRes = 0;
    }
    if( !*p )
    {
        USHORT nMask = nRes & ( SCA_VALID_ROW | SCA_VALID_COL | SCA_VALID_TAB );
        if( nMask == ( SCA_VALID_ROW | SCA_VALID_COL | SCA_VALID_TAB ) )
            nRes |= SCA_VALID;
    }
    else
        nRes = 0;
    rAddr.Set( nCol, nRow, nTab );
    return nRes;
}


bool ConvertSingleRef( ScDocument* pDoc, const String& rRefString,
            SCTAB nDefTab, ScRefAddress& rRefAddress )
{
    BOOL bExternal = FALSE;
    ScAddress aAddr( 0, 0, nDefTab );
    USHORT nRes = lcl_ConvertSingleRef( bExternal, rRefString.GetBuffer(), pDoc, aAddr );
    if( nRes & SCA_VALID )
    {
        rRefAddress.Set( aAddr,
                ((nRes & SCA_COL_ABSOLUTE) == 0),
                ((nRes & SCA_ROW_ABSOLUTE) == 0),
                ((nRes & SCA_TAB_ABSOLUTE) == 0));
        return TRUE;
    }
    else
        return FALSE;
}


bool ConvertDoubleRef( ScDocument* pDoc, const String& rRefString, SCTAB nDefTab,
            ScRefAddress& rStartRefAddress, ScRefAddress& rEndRefAddress )
{
    BOOL bRet = FALSE;
    xub_StrLen nPos = rRefString.Search(':');
    if (nPos != STRING_NOTFOUND)
    {
        String aTmp( rRefString );
        sal_Unicode* p = aTmp.GetBufferAccess();
        p[ nPos ] = 0;
        if ( ConvertSingleRef( pDoc, p, nDefTab, rStartRefAddress ) )
        {
            nDefTab = rStartRefAddress.Tab();
            bRet = ConvertSingleRef( pDoc, p + nPos + 1, nDefTab, rEndRefAddress );
        }
    }
    return bRet;
}


USHORT ScAddress::Parse( const String& r, ScDocument* pDoc )
{
    BOOL bExternal = FALSE;
    return lcl_ConvertSingleRef( bExternal, r.GetBuffer(), pDoc, *this );
}


bool ScRange::Intersects( const ScRange& r ) const
{
    return !(
        Min( aEnd.Col(), r.aEnd.Col() ) < Max( aStart.Col(), r.aStart.Col() )
     || Min( aEnd.Row(), r.aEnd.Row() ) < Max( aStart.Row(), r.aStart.Row() )
     || Min( aEnd.Tab(), r.aEnd.Tab() ) < Max( aStart.Tab(), r.aStart.Tab() )
        );
}


void ScRange::Justify()
{
    SCCOL nTempCol;
    if ( aEnd.Col() < (nTempCol = aStart.Col()) )
    {
        aStart.SetCol(aEnd.Col()); aEnd.SetCol(nTempCol);
    }
    SCROW nTempRow;
    if ( aEnd.Row() < (nTempRow = aStart.Row()) )
    {
        aStart.SetRow(aEnd.Row()); aEnd.SetRow(nTempRow);
    }
    SCTAB nTempTab;
    if ( aEnd.Tab() < (nTempTab = aStart.Tab()) )
    {
        aStart.SetTab(aEnd.Tab()); aEnd.SetTab(nTempTab);
    }
}


void ScRange::ExtendOne()
{
    //  Range fuer Rahmen etc. in X und Y Richtung um 1 erweitern

    SCCOLROW nVal;

    if ((nVal = aStart.Col()) > 0)
        aStart.SetCol(static_cast<SCCOL>(nVal-1));
    if ((nVal = aStart.Row()) > 0)
        aStart.SetRow(nVal-1);

    if ((nVal = aEnd.Col()) < MAXCOL)
        aEnd.SetCol(static_cast<SCCOL>(nVal+1));
    if ((nVal = aEnd.Row()) < MAXROW)
        aEnd.SetRow(nVal+1);
}


USHORT ScRange::Parse( const String& r, ScDocument* pDoc )
{
    USHORT nRes1 = 0, nRes2 = 0;
    xub_StrLen nTmp = 0;
    xub_StrLen nPos = STRING_NOTFOUND;
    while ( (nTmp = r.Search( ':', nTmp )) != STRING_NOTFOUND )
        nPos = nTmp++;      // der letzte zaehlt, koennte 'd:\...'!a1:a2 sein
    if (nPos != STRING_NOTFOUND)
    {
        String aTmp( r );
        sal_Unicode* p = aTmp.GetBufferAccess();
        p[ nPos ] = 0;
        BOOL bExternal = FALSE;
        if( nRes1 = lcl_ConvertSingleRef( bExternal, p, pDoc, aStart ) )
        {
            aEnd = aStart;  // die Tab _muss_ gleich sein, so ist`s weniger Code
            if ( nRes2 = lcl_ConvertSingleRef( bExternal, p + nPos+ 1, pDoc, aEnd ) )
            {
                if ( bExternal && aStart.Tab() != aEnd.Tab() )
                    nRes2 &= ~SCA_VALID_TAB;    // #REF!
                else
                {
                    // PutInOrder / Justify
                    USHORT nMask, nBits1, nBits2;
                    SCCOL nTempCol;
                    if ( aEnd.Col() < (nTempCol = aStart.Col()) )
                    {
                        aStart.SetCol(aEnd.Col()); aEnd.SetCol(nTempCol);
                        nMask = (SCA_VALID_COL | SCA_COL_ABSOLUTE);
                        nBits1 = nRes1 & nMask;
                        nBits2 = nRes2 & nMask;
                        nRes1 = (nRes1 & ~nMask) | nBits2;
                        nRes2 = (nRes2 & ~nMask) | nBits1;
                    }
                    SCROW nTempRow;
                    if ( aEnd.Row() < (nTempRow = aStart.Row()) )
                    {
                        aStart.SetRow(aEnd.Row()); aEnd.SetRow(nTempRow);
                        nMask = (SCA_VALID_ROW | SCA_ROW_ABSOLUTE);
                        nBits1 = nRes1 & nMask;
                        nBits2 = nRes2 & nMask;
                        nRes1 = (nRes1 & ~nMask) | nBits2;
                        nRes2 = (nRes2 & ~nMask) | nBits1;
                    }
                    SCTAB nTempTab;
                    if ( aEnd.Tab() < (nTempTab = aStart.Tab()) )
                    {
                        aStart.SetTab(aEnd.Tab()); aEnd.SetTab(nTempTab);
                        nMask = (SCA_VALID_TAB | SCA_TAB_ABSOLUTE | SCA_TAB_3D);
                        nBits1 = nRes1 & nMask;
                        nBits2 = nRes2 & nMask;
                        nRes1 = (nRes1 & ~nMask) | nBits2;
                        nRes2 = (nRes2 & ~nMask) | nBits1;
                    }
                    if ( ((nRes1 & ( SCA_TAB_ABSOLUTE | SCA_TAB_3D ))
                            == ( SCA_TAB_ABSOLUTE | SCA_TAB_3D ))
                            && !(nRes2 & SCA_TAB_3D) )
                        nRes2 |= SCA_TAB_ABSOLUTE;
                }
            }
            else
                nRes1 = 0;      // #38840# keine Tokens aus halben Sachen
        }
    }
    nRes1 = ( ( nRes1 | nRes2 ) & SCA_VALID )
          | nRes1
          | ( ( nRes2 & 0x070F ) << 4 );
    return nRes1;
}


USHORT ScRange::ParseAny( const String& r, ScDocument* pDoc )
{
    USHORT nRet = Parse( r, pDoc );
    const USHORT nValid = SCA_VALID | SCA_VALID_COL2 | SCA_VALID_ROW2 |
        SCA_VALID_TAB2;
    if ( (nRet & nValid) != nValid )
    {
        ScAddress aAdr;
        nRet = aAdr.Parse( r, pDoc );
        if ( nRet & SCA_VALID )
            aStart = aEnd = aAdr;
    }
    return nRet;
}


void ScAddress::Format( String& r, USHORT nFlags, ScDocument* pDoc ) const
{
    r.Erase();
    if( nFlags & SCA_VALID )
        nFlags |= ( SCA_VALID_ROW | SCA_VALID_COL | SCA_VALID_TAB );
    if( pDoc && (nFlags & SCA_VALID_TAB ) )
    {
        if ( nTab >= pDoc->GetTableCount() )
        {
            r = ScGlobal::GetRscString( STR_NOREF_STR );
            return;
        }
//      if( nFlags & ( SCA_TAB_ABSOLUTE | SCA_TAB_3D ) )
        if( nFlags & SCA_TAB_3D )
        {
            String aTabName;
            pDoc->GetName( nTab, aTabName );

            //  externe Referenzen (wie in ScCompiler::MakeTabStr)
            String aDoc;
            if ( aTabName.GetChar(0) == '\'' )
            {   // "'Doc'#Tab"
                xub_StrLen nPos, nLen = 1;
                while( (nPos = aTabName.Search( '\'', nLen ))
                        != STRING_NOTFOUND )
                    nLen = nPos + 1;
                if ( aTabName.GetChar(nLen) == SC_COMPILER_FILE_TAB_SEP )
                {
                    aDoc = aTabName.Copy( 0, nLen + 1 );
                    aTabName.Erase( 0, nLen + 1 );
                }
            }
            r += aDoc;

            if( nFlags & SCA_TAB_ABSOLUTE )
                r += '$';
            ScCompiler::CheckTabQuotes( aTabName );
            r += aTabName;
            r += '.';
        }
    }
    if( nFlags & SCA_VALID_COL )
    {
        if( nFlags & SCA_COL_ABSOLUTE )
            r += '$';
        ColToAlpha( r, nCol);
    }
    if( nFlags & SCA_VALID_ROW )
    {
        if ( nFlags & SCA_ROW_ABSOLUTE )
            r += '$';
        r += String::CreateFromInt32( Row()+1 );
    }
}


void ScRange::Format( String& r, USHORT nFlags, ScDocument* pDoc ) const
{
    if( !( nFlags & SCA_VALID ) )
        r = ScGlobal::GetRscString( STR_NOREF_STR );
    else
    {
        BOOL bOneTab = (aStart.Tab() == aEnd.Tab());
        if ( !bOneTab )
            nFlags |= SCA_TAB_3D;
        aStart.Format( r, nFlags, pDoc );
        if( aStart != aEnd )
        {
            String aName;
            nFlags = ( nFlags & SCA_VALID ) | ( ( nFlags >> 4 ) & 0x070F );
            if ( bOneTab )
                pDoc = NULL;
            else
                nFlags |= SCA_TAB_3D;
            aEnd.Format( aName, nFlags, pDoc );
            r += ':';
            r += aName;
        }
    }
}


bool ScAddress::Move( SCsCOL dx, SCsROW dy, SCsTAB dz, ScDocument* pDoc )
{
    SCsTAB nMaxTab = pDoc ? pDoc->GetTableCount() : MAXTAB+1;
    dx = Col() + dx;
    dy = Row() + dy;
    dz = Tab() + dz;
    BOOL bValid = TRUE;
    if( dx < 0 )
        dx = 0, bValid = FALSE;
    else if( dx > MAXCOL )
        dx = MAXCOL, bValid =FALSE;
    if( dy < 0 )
        dy = 0, bValid = FALSE;
    else if( dy > MAXROW )
        dy = MAXROW, bValid =FALSE;
    if( dz < 0 )
        dz = 0, bValid = FALSE;
    else if( dz >= nMaxTab )
        dz = nMaxTab-1, bValid =FALSE;
    Set( dx, dy, dz );
    return bValid;
}


bool ScRange::Move( SCsCOL dx, SCsROW dy, SCsTAB dz, ScDocument* pDoc )
{
    // Einfahces &, damit beides ausgefuehrt wird!!
    return aStart.Move( dx, dy, dz, pDoc ) & aEnd.Move( dx, dy, dz, pDoc );
}

// --- moved from ScTripel -----------------------------------------------

String ScAddress::GetText() const
{
    String aString('(');
    aString += String::CreateFromInt32( nCol );
    aString += ',';
    aString += String::CreateFromInt32( nRow );
    aString += ',';
    aString += String::CreateFromInt32( nTab );
    aString += ')';
    return aString;
}


String ScAddress::GetColRowString( bool bAbsolute ) const
{
    String aString;
    if (bAbsolute)
        aString.Append( '$' );

    ColToAlpha( aString, nCol);

    if ( bAbsolute )
        aString.Append( '$' );

    aString += String::CreateFromInt32(nRow+1);

    return aString;
}


String ScRefAddress::GetRefString( ScDocument* pDoc, SCTAB nActTab) const
{
    if ( !pDoc )
        return EMPTY_STRING;
    if ( Tab()+1 > pDoc->GetTableCount() )
        return ScGlobal::GetRscString( STR_NOREF_STR );

    String aString;
    USHORT nFlags = SCA_VALID;
    if ( nActTab != Tab() )
    {
        nFlags |= SCA_TAB_3D;
        if ( !bRelTab )
            nFlags |= SCA_TAB_ABSOLUTE;
    }
    if ( !bRelCol )
        nFlags |= SCA_COL_ABSOLUTE;
    if ( !bRelRow )
        nFlags |= SCA_ROW_ABSOLUTE;

    aAdr.Format( aString, nFlags, pDoc );

    return aString;
}

//------------------------------------------------------------------------

void ColToAlpha( rtl::OUStringBuffer& rBuf, SCCOL nCol )
{
    if (nCol < 26*26)
    {
        if (nCol < 26)
            rBuf.append( static_cast<sal_Unicode>( 'A' +
                        static_cast<sal_uInt16>(nCol)));
        else
        {
            rBuf.append( static_cast<sal_Unicode>( 'A' +
                        (static_cast<sal_uInt16>(nCol) / 26) - 1));
            rBuf.append( static_cast<sal_Unicode>( 'A' +
                        (static_cast<sal_uInt16>(nCol) % 26)));
        }
    }
    else
    {
        String aStr;
        while (nCol >= 26)
        {
            SCCOL nC = nCol % 26;
            aStr += static_cast<sal_Unicode>( 'A' +
                    static_cast<sal_uInt16>(nC));
            nCol -= nC;
            nCol = nCol / 26 - 1;
        }
        aStr += static_cast<sal_Unicode>( 'A' +
                static_cast<sal_uInt16>(nCol));
        aStr.Reverse();
        rBuf.append( aStr);
    }
}


bool AlphaToCol( SCCOL& rCol, const String& rStr)
{
    SCCOL nResult = 0;
    xub_StrLen nStop = rStr.Len();
    xub_StrLen nPos = 0;
    sal_Unicode c;
    while (nResult <= MAXCOL && nPos < nStop && (c = rStr.GetChar( nPos)) &&
            CharClass::isAsciiAlpha(c))
    {
        if (nPos > 0)
            nResult = (nResult + 1) * 26;
        nResult += ScGlobal::ToUpperAlpha(c) - 'A';
        ++nPos;
    }
    bool bOk = (ValidCol(nResult) && nPos > 0);
    if (bOk)
        rCol = nResult;
    return bOk;
}
