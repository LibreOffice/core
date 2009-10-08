/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: address.cxx,v $
 * $Revision: 1.11.30.3 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"

#include "address.hxx"
#include "global.hxx"
#include "compiler.hxx"
#include "document.hxx"

#include "globstr.hrc"
#include <sal/alloca.h>

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <sfx2/objsh.hxx>
#include <tools/urlobj.hxx>
using namespace ::com::sun::star;

////////////////////////////////////////////////////////////////////////////
const ScAddress::Details ScAddress::detailsOOOa1( CONV_OOO, 0, 0 );

ScAddress::Details::Details ( const ScDocument* pDoc,
                              const ScAddress & rAddr ) :
    eConv( pDoc->GetAddressConvention() ),
    nRow( rAddr.Row() ),
    nCol( rAddr.Col() )
{
}

void ScAddress::Details::SetPos ( const ScDocument* pDoc,
                                  const ScAddress & rAddr )
{
    nRow  = rAddr.Row();
    nCol  = rAddr.Col();
    eConv = pDoc->GetAddressConvention();
}

////////////////////////////////////////////////////////////////////////////

#include <iostream>

static long int
sal_Unicode_strtol ( const sal_Unicode*  p,
                     const sal_Unicode** pEnd )
{
    long int accum = 0, prev = 0;
    bool is_neg = false;

    if( *p == '-' )
    {
        is_neg = true;
        p++;
    }
    else if( *p == '+' )
        p++;

    while (CharClass::isAsciiDigit( *p ))
    {
        accum = accum * 10 + *p - '0';
        if( accum < prev )
        {
            *pEnd = NULL;
            return 0;
        }
        prev = accum;
        p++;
    }

    *pEnd = p;
    return is_neg ? -accum : accum;
}

// Returns NULL if the string should be a sheet name, but is invalid
// Returns a pointer to the first character after the sheet name
static const sal_Unicode *
lcl_XL_ParseSheetRef( const sal_Unicode *start,
                      ScAddress *pAddr,
                      const ScDocument* pDoc,
                      const String& rDocName,
                      String& rExternTabName,
                      bool allow_3d )
{
    String aTabName;
    SCTAB  nTab = 0;
    const sal_Unicode *p = start;

    //pAddr->SetTab( 0 );
    if( *p == '\'' ) // XL only seems to use single quotes for sheet names
    {
        for( p++; *p ; )
        {
            if( *p == '\'' )
            {
                if( p[1] != '\'' )  // end quote
                    break;
                p++;    // 2 quotes in a row are a quote in a the name
            }
            aTabName += *p++;
        }
        if( *p++ != '\'' )
            return NULL;
    }
    else
    {
        bool only_digits = TRUE;

        /*
         * Valid: Normal!a1
         * Valid: x.y!a1
         * Invalid: .y!a1
         *
         * Some names starting with digits are actually valid, but
         * unparse quoted. Things are quite tricky: most sheet names
         * starting with a digit are ok, but not those starting with
         * "[0-9]*\." or "[0-9]+[eE]".
         *
         * Valid: 42!a1
         * Valid: 4x!a1
         * Invalid: 1.!a1
         * Invalid: 1e!a1
         */
        while( 1 )
        {
            const sal_Unicode uc = *p;
            if( CharClass::isAsciiAlpha( uc ) || uc == '_' )
            {
                if( only_digits && p != start &&
                   (uc == 'e' || uc == 'E' ) )
                {
                    p = start;
                    break;
                }
                only_digits = FALSE;
                p++;
            }
            else if( CharClass::isAsciiDigit( uc ))
            {
                p++;
            }
            else if( uc == '.' )
            {
                if( only_digits ) // Valid, except after only digits.
                {
                    p = start;
                    break;
                }
                p++;
            } else
                break;
        }

        if( *p != '!' &&( !allow_3d || *p != ':' ) )
            return start;

        aTabName.Append( start, sal::static_int_cast<xub_StrLen>( p - start ) );
    }

    if( pDoc )
    {
        if( rDocName.Len() > 0 )
        {
            // This is a simplification of the OOo code which does an
            // optimization to manually construct the DocTab before calling
            // GetDocTabName
            String aDocTab = ScGlobal::GetDocTabName( rDocName, aTabName );
            if( !pDoc->GetTable( aDocTab, nTab ) &&
                 pDoc->ValidTabName( aTabName ) &&
                !pDoc->GetTable( aDocTab, nTab ) )
            {
                rExternTabName = aDocTab;
            }
        }
        else if( !pDoc->GetTable( aTabName, nTab ) )
            return start;
    }

    pAddr->SetTab( nTab );
    return p;
}

static const sal_Unicode*
lcl_ScRange_Parse_XL_Header( ScRange& r,
                             const sal_Unicode* p,
                             const ScDocument* pDoc,
                             String& rExternDocName,
                             String& rStartTabName,
                             String& rEndTabName,
                             USHORT& nFlags )
{
    const sal_Unicode* startTabs, *start = p;

    // Is this an external reference ?
    rStartTabName.Erase();
    rEndTabName.Erase();
    if( *p == '[' )
    {
        p++;
        // TODO : Get more detail on how paths are quoted
        // 1) I suspect only single quote is correct
        // 2) check whether this is another doubled quote rather than a
        //    backslash
        if( *p == '\'' || *p == '\"' )
        {
            for( const sal_Unicode cQuote = *p++; *p && *p != cQuote ; )
            {
                if( *p == '\\' && p[1] )
                    p++;
                rExternDocName += *p++;
            }
        }
        else
        {
            p = ScGlobal::UnicodeStrChr( start+1, ']' );
            if( p == NULL )
                return start;
            rExternDocName.Append( start+1, sal::static_int_cast<xub_StrLen>( p-(start+1) ) );
        }

        rExternDocName = ScGlobal::GetAbsDocName( rExternDocName,
            pDoc->GetDocumentShell() );
        if( *p != ']' )
            return start;
        p++;
    }

    startTabs = p;
    p = lcl_XL_ParseSheetRef( p, &r.aStart, pDoc, rExternDocName, rStartTabName, TRUE );
    if( NULL == p )
        return start;       // invalid tab
    if( p != startTabs )
    {
        nFlags |= SCA_VALID_TAB | SCA_TAB_3D | SCA_TAB_ABSOLUTE;
        if( *p == ':' ) // 3d ref
        {
            p = lcl_XL_ParseSheetRef( p+1, &r.aEnd, pDoc, rExternDocName, rEndTabName, FALSE );
            if( p == NULL )
                return start; // invalid tab
            nFlags |= SCA_VALID_TAB2 | SCA_TAB2_3D | SCA_TAB2_ABSOLUTE;
        }
        else
        {
            // If only one sheet is given, the full reference is still valid,
            // only the second 3D flag is not set.
            nFlags |= SCA_VALID_TAB2 | SCA_TAB2_ABSOLUTE;
            r.aEnd.SetTab( r.aStart.Tab() );
        }

        if( *p++ != '!' )
            return start;   // syntax error
    }
    else
    {
        nFlags |= SCA_VALID_TAB | SCA_VALID_TAB2;
        // Use the current tab, it needs to be passed in. : r.aEnd.SetTab( .. );
    }

    return p;
}

static USHORT
lcl_XL_LinkSheetRef( ScRange& r,
                     ScDocument* pDoc,
                     const String& rExternDocName,
                     const String& rStartTabName,
                     const String& rEndTabName,
                     USHORT nFlags )
{
    SCTAB nTab;

    if( rExternDocName.Len() > 0 )
    {
        String aDocName = ScGlobal::GetAbsDocName( rExternDocName,
                                                   pDoc->GetDocumentShell() );

        String aDocTab;

        aDocTab = ScGlobal::GetDocTabName( aDocName, rStartTabName );
        if( !pDoc->LinkExternalTab( nTab, aDocTab, rExternDocName, rStartTabName ) )
            return 0;
        r.aStart.SetTab( nTab );

        if( rEndTabName.Len() > 0 &&
            rStartTabName != rEndTabName )
        {
            aDocTab = ScGlobal::GetDocTabName( aDocName, rEndTabName );
            if( !pDoc->LinkExternalTab( nTab, aDocTab, rExternDocName, rEndTabName ) )
            {
                DBG_ASSERT( r.IsValid(), "lcl_XL_LinkSheetRef - unable to link endTab of 3d ref" );
                return 0;
            }
        }
        r.aEnd.SetTab( nTab );
    }

    return nFlags;
}


static const sal_Unicode*
lcl_r1c1_get_col( const sal_Unicode* p,
                  const ScAddress::Details& rDetails,
                  ScAddress* pAddr, USHORT* nFlags )
{
    const sal_Unicode *pEnd;
    long int n;
    bool isRelative;

    if( p[0] == '\0' )
        return NULL;

    p++;
    if( (isRelative = (*p == '[') ) != false )
        p++;
    n = sal_Unicode_strtol( p, &pEnd );
    if( NULL == pEnd )
        return NULL;

    if( p == pEnd ) // C is a relative ref with offset 0
    {
        if( isRelative )
            return NULL;
        n = rDetails.nCol;
    }
    else if( isRelative )
    {
        if( *pEnd != ']' )
            return NULL;
        n += rDetails.nCol;
        pEnd++;
    }
    else
    {
        *nFlags |= SCA_COL_ABSOLUTE;
        n--;
    }

    if( n < 0 || n >= MAXCOLCOUNT )
        return NULL;
    pAddr->SetCol( static_cast<SCCOL>( n ) );
    *nFlags |= SCA_VALID_COL;

    return pEnd;
}
static inline const sal_Unicode*
lcl_r1c1_get_row( const sal_Unicode* p,
                  const ScAddress::Details& rDetails,
                  ScAddress* pAddr, USHORT* nFlags )
{
    const sal_Unicode *pEnd;
    long int n;
    bool isRelative;

    if( p[0] == '\0' )
        return NULL;

    p++;
    if( (isRelative = (*p == '[') ) != false )
        p++;
    n = sal_Unicode_strtol( p, &pEnd );
    if( NULL == pEnd )
        return NULL;

    if( p == pEnd ) // R is a relative ref with offset 0
    {
        if( isRelative )
            return NULL;
        n = rDetails.nRow;
    }
    else if( isRelative )
    {
        if( *pEnd != ']' )
            return NULL;
        n += rDetails.nRow;
        pEnd++;
    }
    else
    {
        *nFlags |= SCA_ROW_ABSOLUTE;
        n--;
    }

    if( n < 0 || n >= MAXROWCOUNT )
        return NULL;
    pAddr->SetRow( static_cast<SCROW>( n ) );
    *nFlags |= SCA_VALID_ROW;

    return pEnd;
}

static USHORT
lcl_ScRange_Parse_XL_R1C1( ScRange& r,
                           const sal_Unicode* p,
                           ScDocument* pDoc,
                           const ScAddress::Details& rDetails,
                           BOOL bOnlyAcceptSingle )
{
    const sal_Unicode* pTmp = NULL;
    String aExternDocName, aStartTabName, aEndTabName;
    USHORT nFlags = SCA_VALID | SCA_VALID_TAB, nFlags2 = SCA_VALID_TAB2;

#if 0
    {
        ByteString  aStr(p, RTL_TEXTENCODING_UTF8);
        aStr.Append(static_cast< char >(0));
        std::cerr << "parse::XL::R1C1 \'" << aStr.GetBuffer() << '\'' << std::endl;
    }
#endif
    p = lcl_ScRange_Parse_XL_Header( r, p, pDoc,
        aExternDocName, aStartTabName, aEndTabName, nFlags );
    if( NULL == p )
        return 0;

    if( *p == 'R' || *p == 'r' )
    {
        if( NULL == (p = lcl_r1c1_get_row( p, rDetails, &r.aStart, &nFlags )) )
            goto failed;

        if( *p != 'C' && *p != 'c' )    // full row R#
        {
            if( p[0] != ':' || (p[1] != 'R' && p[1] != 'r' ) ||
                NULL == (pTmp = lcl_r1c1_get_row( p+1, rDetails, &r.aEnd, &nFlags2 )))
            {
                // Only the initial row number is given, or the second row
                // number is invalid. Fallback to just the initial R
                nFlags |= (nFlags << 4);
                r.aEnd.SetRow( r.aStart.Row() );
            }
            else
            {
                // Full row range successfully parsed.
                nFlags |= (nFlags2 << 4);
                p = pTmp;
            }

            if (p && p[0] != 0)
            {
                // any trailing invalid character must invalidate the whole address.
                nFlags &= ~(SCA_VALID | SCA_VALID_COL | SCA_VALID_ROW | SCA_VALID_TAB);
                return nFlags;
            }

            nFlags |=
                SCA_VALID_COL | SCA_VALID_COL2 |
                SCA_COL_ABSOLUTE | SCA_COL2_ABSOLUTE;
            r.aStart.SetCol( 0 );
            r.aEnd.SetCol( MAXCOL );

            return bOnlyAcceptSingle ? 0 : lcl_XL_LinkSheetRef( r, pDoc,
                aExternDocName, aStartTabName, aEndTabName, nFlags );
        }
        else if( NULL == (p = lcl_r1c1_get_col( p, rDetails, &r.aStart, &nFlags )))
            goto failed;

        if( p[0] != ':' ||
            (p[1] != 'R' && p[1] != 'r') ||
            NULL == (pTmp = lcl_r1c1_get_row( p+1, rDetails, &r.aEnd, &nFlags2 )) ||
            (*pTmp != 'C' && *pTmp != 'c') ||
            NULL == (pTmp = lcl_r1c1_get_col( pTmp, rDetails, &r.aEnd, &nFlags2 )))
        {
            // single cell reference

            if (p && p[0] != 0)
            {
                // any trailing invalid character must invalidate the whole address.
                nFlags &= ~(SCA_VALID | SCA_VALID_COL | SCA_VALID_ROW | SCA_VALID_TAB);
                return nFlags;
            }

            return bOnlyAcceptSingle ? lcl_XL_LinkSheetRef( r, pDoc,
                aExternDocName, aStartTabName, aEndTabName, nFlags ) : 0;
        }
        p = pTmp;

        // double reference

        if (p && p[0] != 0)
        {
            // any trailing invalid character must invalidate the whole range.
            nFlags &= ~(SCA_VALID | SCA_VALID_COL | SCA_VALID_ROW | SCA_VALID_TAB |
                        SCA_VALID_COL2 | SCA_VALID_ROW2 | SCA_VALID_TAB2);
            return nFlags;
        }

        nFlags |= (nFlags2 << 4);
        return bOnlyAcceptSingle ? 0 : lcl_XL_LinkSheetRef( r, pDoc,
            aExternDocName, aStartTabName, aEndTabName, nFlags );
    }
    else if( *p == 'C' || *p == 'c' )   // full col C#
    {
        if( NULL == (p = lcl_r1c1_get_col( p, rDetails, &r.aStart, &nFlags )))
            goto failed;

        if( p[0] != ':' || (p[1] != 'C' && p[1] != 'c') ||
            NULL == (pTmp = lcl_r1c1_get_col( p+1, rDetails, &r.aEnd, &nFlags2 )))
        {    // Fallback to just the initial C
            nFlags |= (nFlags << 4);
            r.aEnd.SetCol( r.aStart.Col() );
        }
        else
        {
            nFlags |= (nFlags2 << 4);
            p = pTmp;
        }

        if (p && p[0] != 0)
        {
            // any trailing invalid character must invalidate the whole address.
            nFlags &= ~(SCA_VALID | SCA_VALID_COL | SCA_VALID_ROW | SCA_VALID_TAB);
            return nFlags;
        }

        nFlags |=
            SCA_VALID_ROW | SCA_VALID_ROW2 |
            SCA_ROW_ABSOLUTE | SCA_ROW2_ABSOLUTE;
        r.aStart.SetRow( 0 );
        r.aEnd.SetRow( MAXROW );

        return bOnlyAcceptSingle ? 0 : lcl_XL_LinkSheetRef( r, pDoc,
            aExternDocName, aStartTabName, aEndTabName, nFlags );
    }

failed :
    return 0;
}

static inline const sal_Unicode*
lcl_a1_get_col( const sal_Unicode* p, ScAddress* pAddr, USHORT* nFlags )
{
    SCCOL nCol;

    if( *p == '$' )
        *nFlags |= SCA_COL_ABSOLUTE, p++;

    if( !CharClass::isAsciiAlpha( *p ) )
        return NULL;

    nCol = sal::static_int_cast<SCCOL>( toupper( char(*p++) ) - 'A' );
    while (nCol <= MAXCOL && CharClass::isAsciiAlpha(*p))
        nCol = sal::static_int_cast<SCCOL>( ((nCol + 1) * 26) + toupper( char(*p++) ) - 'A' );
    if( nCol > MAXCOL || CharClass::isAsciiAlpha( *p ) )
        return NULL;

    *nFlags |= SCA_VALID_COL;
    pAddr->SetCol( nCol );

    return p;
}

static inline const sal_Unicode*
lcl_a1_get_row( const sal_Unicode* p, ScAddress* pAddr, USHORT* nFlags )
{
    const sal_Unicode *pEnd;
    long int n;

    if( *p == '$' )
        *nFlags |= SCA_ROW_ABSOLUTE, p++;

    n = sal_Unicode_strtol( p, &pEnd ) - 1;
    if( NULL == pEnd || p == pEnd || n < 0 || n > MAXROW )
        return NULL;

    *nFlags |= SCA_VALID_ROW;
    pAddr->SetRow( static_cast<SCROW>(n) );

    return pEnd;
}

static USHORT
lcl_ScRange_Parse_XL_A1( ScRange& r,
                         const sal_Unicode* p,
                         ScDocument* pDoc,
                         BOOL bOnlyAcceptSingle )
{
    const sal_Unicode* tmp1, *tmp2;
    String aExternDocName, aStartTabName, aEndTabName;
    USHORT nFlags = SCA_VALID | SCA_VALID_TAB, nFlags2 = SCA_VALID_TAB2;

#if 0
    {
        ByteString  aStr(p, RTL_TEXTENCODING_UTF8);
        aStr.Append(static_cast< char >(0));
        std::cerr << "parse::XL::A1 \'" << aStr.GetBuffer() << '\'' << std::endl;
    }
#endif
    p = lcl_ScRange_Parse_XL_Header( r, p, pDoc,
        aExternDocName, aStartTabName, aEndTabName, nFlags );
    if( NULL == p )
        return 0;

    tmp1 = lcl_a1_get_col( p, &r.aStart, &nFlags );
    if( tmp1 == NULL )          // Is it a row only reference 3:5
    {
        if( bOnlyAcceptSingle ) // by definition full row refs are ranges
            return 0;

        tmp1 = lcl_a1_get_row( p, &r.aStart, &nFlags );
        if( !tmp1 || *tmp1++ != ':' ) // Even a singleton requires ':' (eg 2:2)
            return 0;
        tmp2 = lcl_a1_get_row( tmp1, &r.aEnd, &nFlags2 );
        if( !tmp2 )
            return 0;

        r.aStart.SetCol( 0 ); r.aEnd.SetCol( MAXCOL );
        nFlags |=
            SCA_VALID_COL | SCA_VALID_COL2 |
            SCA_COL_ABSOLUTE | SCA_COL2_ABSOLUTE;
        nFlags |= (nFlags2 << 4);
        return lcl_XL_LinkSheetRef( r, pDoc,
            aExternDocName, aStartTabName, aEndTabName, nFlags );
    }

    tmp2 = lcl_a1_get_row( tmp1, &r.aStart, &nFlags );
    if( tmp2 == NULL )          // check for col only reference F:H
    {
        if( bOnlyAcceptSingle ) // by definition full col refs are ranges
            return 0;

        if( *tmp1++ != ':' )    // Even a singleton requires ':' (eg F:F)
            return 0;
        tmp2 = lcl_a1_get_col( tmp1, &r.aEnd, &nFlags2 );
        if( !tmp2 )
        return 0;

        r.aStart.SetRow( 0 ); r.aEnd.SetRow( MAXROW );
        nFlags |=
            SCA_VALID_ROW | SCA_VALID_ROW2 |
            SCA_ROW_ABSOLUTE | SCA_ROW2_ABSOLUTE;
        nFlags |= (nFlags2 << 4);
        return lcl_XL_LinkSheetRef( r, pDoc,
            aExternDocName, aStartTabName, aEndTabName, nFlags );
    }

    // prepare as if it's a singleton, in case we want to fall back */
    r.aEnd.SetCol( r.aStart.Col() );
    r.aEnd.SetRow( r.aStart.Row() );    // don't overwrite sheet number as parsed in lcl_ScRange_Parse_XL_Header

    if ( bOnlyAcceptSingle )
    {
        if ( *tmp2 == 0 )
            return lcl_XL_LinkSheetRef( r, pDoc,
                aExternDocName, aStartTabName, aEndTabName, nFlags );
        else
        {
            // any trailing invalid character must invalidate the address.
            nFlags &= ~(SCA_VALID | SCA_VALID_COL | SCA_VALID_ROW | SCA_VALID_TAB);
            return nFlags;
        }
    }

    if( *tmp2 != ':' )
    {
        nFlags &= ~(SCA_VALID | SCA_VALID_COL | SCA_VALID_ROW | SCA_VALID_TAB |
                    SCA_VALID_COL2 | SCA_VALID_ROW2 | SCA_VALID_TAB2);
        return nFlags;
    }

    p = tmp2;
    tmp1 = lcl_a1_get_col( p+1, &r.aEnd, &nFlags2 );
    if( !tmp1 ) // strange, but valid singleton
        return lcl_XL_LinkSheetRef( r, pDoc,
            aExternDocName, aStartTabName, aEndTabName, nFlags );
    tmp2 = lcl_a1_get_row( tmp1, &r.aEnd, &nFlags2 );
    if( !tmp2 ) // strange, but valid singleton
        return lcl_XL_LinkSheetRef( r, pDoc,
            aExternDocName, aStartTabName, aEndTabName, nFlags );

    if ( *tmp2 != 0 )
    {
        // any trailing invalid character must invalidate the range.
        nFlags &= ~(SCA_VALID | SCA_VALID_COL | SCA_VALID_ROW | SCA_VALID_TAB |
                    SCA_VALID_COL2 | SCA_VALID_ROW2 | SCA_VALID_TAB2);
        return nFlags;
    }

    nFlags |= (nFlags2 << 4);
    return lcl_XL_LinkSheetRef( r, pDoc,
        aExternDocName, aStartTabName, aEndTabName, nFlags );
}

static USHORT
lcl_ScAddress_Parse_OOo( BOOL& bExternal, const sal_Unicode* p,
                         ScDocument* pDoc, ScAddress& rAddr )
{
    USHORT  nRes = 0;
    String  aDocName;       // der pure Dokumentenname
    String  aDocTab;        // zusammengesetzt fuer Table
    String  aTab;
    BOOL    bExtDoc = FALSE;
    BOOL    bNeedExtTab = FALSE;

    // Lets see if this is a reference to something in an external file.
    // A Documentname is always quoted and has a trailing #
    if ( *p == '\'' && ScGlobal::UnicodeStrChr( p, SC_COMPILER_FILE_TAB_SEP ) )
    {
        const sal_Unicode *pStart = p;
        BOOL bQuote = TRUE;         // A Documentname is always quoted
        aDocTab += *p++;
        while ( bQuote && *p )
        {
            if ( *p == '\'' && *(p-1) != '\\' )
                bQuote = FALSE;
            else if( !(*p == '\\' && *(p+1) == '\'') )
                aDocName += *p;     // An escaped Quote in the Documentname
            aDocTab += *p++;
        }
        aDocTab += *p;              // den SC_COMPILER_FILE_TAB_SEP mitnehmen
        if( *p++ == SC_COMPILER_FILE_TAB_SEP )
            bExtDoc = TRUE;
        else
        {
            // It wasn't a document after all, reset and continue as normal
            p = pStart;
            aDocTab = String();
        }
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

        // Tokens that start at ' can have anything in them until a final '
        // but '' marks an escaped '
        // We've earlier guaranteed that a string containing '' will be
        // surrounded by '
        if( *p == '\'' )
        {
            ++p;
            while (*p)
            {
                if (*p == '\'')
                {
                    if ( (*(p+1) != '\'') )
                        break;
                    else
                        *p++;
                }
                aTab += *p++;
            }
        }

        while (*p)
        {
            if( *p == '.')
                break;

            if( *p == '\'' )
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
            nCol = sal::static_int_cast<SCCOL>( toupper( char(*p++) ) - 'A' );
            while (nCol < MAXCOL && CharClass::isAsciiAlpha(*p))
                nCol = sal::static_int_cast<SCCOL>( ((nCol + 1) * 26) + toupper( char(*p++) ) - 'A' );
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

static USHORT
lcl_ScAddress_Parse ( BOOL& bExternal, const sal_Unicode* p,
                      ScDocument* pDoc, ScAddress& rAddr,
                      const ScAddress::Details& rDetails )
{
    if( !*p )
        return 0;

    switch (rDetails.eConv)
    {
    default :
    case ScAddress::CONV_OOO:
        {
            return lcl_ScAddress_Parse_OOo( bExternal, p, pDoc, rAddr );
        }

    case ScAddress::CONV_XL_A1:
        {
            ScRange r = rAddr;
            USHORT nFlags = lcl_ScRange_Parse_XL_A1( r, p, pDoc, TRUE );
            rAddr = r.aStart;
            return nFlags;
        }
    case ScAddress::CONV_XL_R1C1:
        {
            ScRange r = rAddr;
            USHORT nFlags = lcl_ScRange_Parse_XL_R1C1( r, p, pDoc, rDetails, TRUE );
            rAddr = r.aStart;
            return nFlags;
        }
    }
}


bool ConvertSingleRef( ScDocument* pDoc, const String& rRefString,
                       SCTAB nDefTab, ScRefAddress& rRefAddress,
                       const ScAddress::Details& rDetails )
{
    BOOL bExternal = FALSE;
    ScAddress aAddr( 0, 0, nDefTab );
    USHORT nRes = lcl_ScAddress_Parse( bExternal, rRefString.GetBuffer(), pDoc, aAddr, rDetails );
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
                       ScRefAddress& rStartRefAddress, ScRefAddress& rEndRefAddress,
                       const ScAddress::Details& rDetails )
{
    BOOL bRet = FALSE;
    // FIXME : This will break for Lotus
    xub_StrLen nPos = rRefString.Search(':');
    if (nPos != STRING_NOTFOUND)
    {
        String aTmp( rRefString );
        sal_Unicode* p = aTmp.GetBufferAccess();
        p[ nPos ] = 0;
        if( ConvertSingleRef( pDoc, p, nDefTab, rStartRefAddress, rDetails ) )
        {
            nDefTab = rStartRefAddress.Tab();
            bRet = ConvertSingleRef( pDoc, p + nPos + 1, nDefTab, rEndRefAddress, rDetails );
        }
    }
    return bRet;
}


USHORT ScAddress::Parse( const String& r, ScDocument* pDoc,
                         const Details& rDetails)
{
    BOOL bExternal = FALSE;
    return lcl_ScAddress_Parse( bExternal, r.GetBuffer(), pDoc, *this, rDetails );
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

void ScRange::ExtendTo( const ScRange& rRange )
{
    DBG_ASSERT( rRange.IsValid(), "ScRange::ExtendTo - cannot extend to invalid range" );
    if( IsValid() )
    {
        aStart.SetCol( ::std::min( aStart.Col(), rRange.aStart.Col() ) );
        aStart.SetRow( ::std::min( aStart.Row(), rRange.aStart.Row() ) );
        aStart.SetTab( ::std::min( aStart.Tab(), rRange.aStart.Tab() ) );
        aEnd.SetCol(   ::std::max( aEnd.Col(),   rRange.aEnd.Col() ) );
        aEnd.SetRow(   ::std::max( aEnd.Row(),   rRange.aEnd.Row() ) );
        aEnd.SetTab(   ::std::max( aEnd.Tab(),   rRange.aEnd.Tab() ) );
    }
    else
        *this = rRange;
}

static USHORT
lcl_ScRange_Parse_OOo( ScRange &aRange, const String& r, ScDocument* pDoc )
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
        if( (nRes1 = lcl_ScAddress_Parse_OOo( bExternal, p, pDoc, aRange.aStart ) ) != 0 )
        {
            aRange.aEnd = aRange.aStart;  // die Tab _muss_ gleich sein, so ist`s weniger Code
            if ( (nRes2 = lcl_ScAddress_Parse_OOo( bExternal, p + nPos+ 1, pDoc, aRange.aEnd ) ) != 0 )
            {
                if ( bExternal && aRange.aStart.Tab() != aRange.aEnd.Tab() )
                    nRes2 &= ~SCA_VALID_TAB;    // #REF!
                else
                {
                    // PutInOrder / Justify
                    USHORT nMask, nBits1, nBits2;
                    SCCOL nTempCol;
                    if ( aRange.aEnd.Col() < (nTempCol = aRange.aStart.Col()) )
                    {
                        aRange.aStart.SetCol(aRange.aEnd.Col()); aRange.aEnd.SetCol(nTempCol);
                        nMask = (SCA_VALID_COL | SCA_COL_ABSOLUTE);
                        nBits1 = nRes1 & nMask;
                        nBits2 = nRes2 & nMask;
                        nRes1 = (nRes1 & ~nMask) | nBits2;
                        nRes2 = (nRes2 & ~nMask) | nBits1;
                    }
                    SCROW nTempRow;
                    if ( aRange.aEnd.Row() < (nTempRow = aRange.aStart.Row()) )
                    {
                        aRange.aStart.SetRow(aRange.aEnd.Row()); aRange.aEnd.SetRow(nTempRow);
                        nMask = (SCA_VALID_ROW | SCA_ROW_ABSOLUTE);
                        nBits1 = nRes1 & nMask;
                        nBits2 = nRes2 & nMask;
                        nRes1 = (nRes1 & ~nMask) | nBits2;
                        nRes2 = (nRes2 & ~nMask) | nBits1;
                    }
                    SCTAB nTempTab;
                    if ( aRange.aEnd.Tab() < (nTempTab = aRange.aStart.Tab()) )
                    {
                        aRange.aStart.SetTab(aRange.aEnd.Tab()); aRange.aEnd.SetTab(nTempTab);
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

USHORT ScRange::Parse( const String& r, ScDocument* pDoc,
                       const ScAddress::Details& rDetails )
{
    if ( r.Len() <= 0 )
        return 0;

    switch (rDetails.eConv)
    {
    default :
    case ScAddress::CONV_OOO:
        return lcl_ScRange_Parse_OOo( *this, r, pDoc );

    case ScAddress::CONV_XL_A1:
        return lcl_ScRange_Parse_XL_A1( *this, r.GetBuffer(), pDoc, FALSE );

    case ScAddress::CONV_XL_R1C1:
        return lcl_ScRange_Parse_XL_R1C1( *this, r.GetBuffer(), pDoc, rDetails, FALSE );
    }
}


// Accept a full range, or an address
USHORT ScRange::ParseAny( const String& r, ScDocument* pDoc,
                          const ScAddress::Details& rDetails )
{
    USHORT nRet = Parse( r, pDoc, rDetails );
    const USHORT nValid = SCA_VALID | SCA_VALID_COL2 | SCA_VALID_ROW2 |
        SCA_VALID_TAB2;

    if ( (nRet & nValid) != nValid )
    {
        ScAddress aAdr;
        nRet = aAdr.Parse( r, pDoc, rDetails );
        if ( nRet & SCA_VALID )
            aStart = aEnd = aAdr;
    }
    return nRet;
}

// Parse only full row references
USHORT ScRange::ParseCols( const String& rStr, ScDocument* pDoc,
                           const ScAddress::Details& rDetails )
{
    const sal_Unicode* p = rStr.GetBuffer();
    USHORT nRes = 0, ignored = 0;

    if( NULL == p )
        return 0;

    pDoc = NULL; // make compiler shutup we may need this later

    switch (rDetails.eConv)
    {
    default :
    case ScAddress::CONV_OOO: // No full col refs in OOO yet, assume XL notation
    case ScAddress::CONV_XL_A1:
        if (NULL != (p = lcl_a1_get_col( p, &aStart, &ignored ) ) )
        {
            if( p[0] == ':')
            {
                if( NULL != (p = lcl_a1_get_col( p+1, &aEnd, &ignored )))
                {
                    nRes = SCA_VALID_COL;
                }
            }
            else
            {
                aEnd = aStart;
                nRes = SCA_VALID_COL;
            }
        }
        break;

    case ScAddress::CONV_XL_R1C1:
        if ((p[0] == 'C' || p[0] != 'c') &&
            NULL != (p = lcl_r1c1_get_col( p, rDetails, &aStart, &ignored )))
        {
            if( p[0] == ':')
            {
                if( (p[1] == 'C' || p[1] == 'c') &&
                    NULL != (p = lcl_r1c1_get_col( p+1, rDetails, &aEnd, &ignored )))
                {
                    nRes = SCA_VALID_COL;
                }
            }
            else
            {
                aEnd = aStart;
                nRes = SCA_VALID_COL;
            }
        }
        break;
    }

    return (p != NULL && *p == '\0') ? nRes : 0;
}

// Parse only full row references
USHORT ScRange::ParseRows( const String& rStr, ScDocument* pDoc,
                           const ScAddress::Details& rDetails )
{
    const sal_Unicode* p = rStr.GetBuffer();
    USHORT nRes = 0, ignored = 0;

    if( NULL == p )
        return 0;

    pDoc = NULL; // make compiler shutup we may need this later

    switch (rDetails.eConv)
    {
    default :
    case ScAddress::CONV_OOO: // No full row refs in OOO yet, assume XL notation
    case ScAddress::CONV_XL_A1:
        if (NULL != (p = lcl_a1_get_row( p, &aStart, &ignored ) ) )
        {
            if( p[0] == ':')
            {
                if( NULL != (p = lcl_a1_get_row( p+1, &aEnd, &ignored )))
                {
                    nRes = SCA_VALID_COL;
                }
            }
            else
            {
                aEnd = aStart;
                nRes = SCA_VALID_COL;
            }
        }
        break;

    case ScAddress::CONV_XL_R1C1:
        if ((p[0] == 'R' || p[0] != 'r') &&
            NULL != (p = lcl_r1c1_get_row( p, rDetails, &aStart, &ignored )))
        {
            if( p[0] == ':')
            {
                if( (p[1] == 'R' || p[1] == 'r') &&
                    NULL != (p = lcl_r1c1_get_row( p+1, rDetails, &aEnd, &ignored )))
                {
                    nRes = SCA_VALID_COL;
                }
            }
            else
            {
                aEnd = aStart;
                nRes = SCA_VALID_COL;
            }
        }
        break;
    }

    return (p != NULL && *p == '\0') ? nRes : 0;
}

static inline void
lcl_a1_append_c ( String &r, int nCol, bool bIsAbs )
{
    if( bIsAbs )
        r += '$';
    ColToAlpha( r, sal::static_int_cast<SCCOL>(nCol) );
}

static inline void
lcl_a1_append_r ( String &r, int nRow, bool bIsAbs )
{
    if ( bIsAbs )
        r += '$';
    r += String::CreateFromInt32( nRow+1 );
}

static inline void
lcl_r1c1_append_c ( String &r, int nCol, bool bIsAbs,
                    const ScAddress::Details& rDetails )
{
    r += 'C';
    if (bIsAbs)
    {
        r += String::CreateFromInt32( nCol + 1 );
    }
    else
    {
        nCol -= rDetails.nCol;
        if (nCol != 0) {
            r += '[';
            r += String::CreateFromInt32( nCol );
            r += ']';
        }
    }
}
static inline void
lcl_r1c1_append_r ( String &r, int nRow, bool bIsAbs,
                    const ScAddress::Details& rDetails )
{
    r += 'R';
    if (bIsAbs)
    {
        r += String::CreateFromInt32( nRow + 1 );
    }
    else
    {
        nRow -= rDetails.nRow;
        if (nRow != 0) {
            r += '[';
            r += String::CreateFromInt32( nRow );
            r += ']';
        }
    }
}

static String
getFileNameFromDoc( const ScDocument* pDoc )
{
    // TODO : er points at ScGlobal::GetAbsDocName()
    // as a better template.  Look into it
    String sFileName;
    SfxObjectShell* pShell;

    if( NULL != pDoc &&
        NULL != (pShell = pDoc->GetDocumentShell() ) )
    {
        uno::Reference< frame::XModel > xModel( pShell->GetModel(), uno::UNO_QUERY );
        if( xModel.is() )
        {
            if( xModel->getURL().getLength() )
            {
                INetURLObject aURL( xModel->getURL() );
                sFileName = aURL.GetLastName();
            }
            else
                sFileName = pShell->GetTitle();
        }
    }
#if 0
        {
            ByteString  aStr( sFileName, RTL_TEXTENCODING_UTF8 );
            aStr.Append(static_cast< char >(0));
            std::cerr << "docname \'" << aStr.GetBuffer() << '\'' << std::endl;
        }
#endif
    return sFileName;
}

void ScAddress::Format( String& r, USHORT nFlags, ScDocument* pDoc,
                        const Details& rDetails) const
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
            String aTabName, aDocName;
            pDoc->GetName( nTab, aTabName );
            //  externe Referenzen (wie in ScCompiler::MakeTabStr)
            if( aTabName.GetChar(0) == '\'' )
            {   // "'Doc'#Tab"
                xub_StrLen nPos, nLen = 1;
                while( (nPos = aTabName.Search( '\'', nLen ))
                        != STRING_NOTFOUND )
                    nLen = nPos + 1;
                if( aTabName.GetChar(nLen) == SC_COMPILER_FILE_TAB_SEP )
                {
                    aDocName = aTabName.Copy( 0, nLen + 1 );
                    aTabName.Erase( 0, nLen + 1 );
                }
            }
            else if( nFlags & SCA_FORCE_DOC )
            {
                // VBA has an 'external' flag that forces the addition of the
                // tab name _and_ the doc name.  The VBA code would be
                // needlessly complicated if it constructed an actual external
                // reference so we add this somewhat cheesy kludge to force the
                // addition of the document name even for non-external references
                aDocName = getFileNameFromDoc( pDoc );
            }
            ScCompiler::CheckTabQuotes( aTabName, rDetails.eConv);

            switch( rDetails.eConv )
            {
            default :
            case CONV_OOO:
                r += aDocName;
                if( nFlags & SCA_TAB_ABSOLUTE )
                    r += '$';
                r += aTabName;
                r += '.';
                break;

            case CONV_XL_A1:
            case CONV_XL_R1C1:
                if (aDocName.Len() > 0)
                {
                    r += '[';
                    r += aDocName;
                    r += ']';
                }
                r += aTabName;
                r += '!';
                break;
            }
        }
    }
    switch( rDetails.eConv )
    {
    default :
    case CONV_OOO:
    case CONV_XL_A1:
        if( nFlags & SCA_VALID_COL )
            lcl_a1_append_c ( r, nCol, nFlags & SCA_COL_ABSOLUTE );
        if( nFlags & SCA_VALID_ROW )
            lcl_a1_append_r ( r, nRow, nFlags & SCA_ROW_ABSOLUTE );
        break;

    case CONV_XL_R1C1:
        if( nFlags & SCA_VALID_ROW )
            lcl_r1c1_append_r ( r, nRow, nFlags & SCA_ROW_ABSOLUTE, rDetails );
        if( nFlags & SCA_VALID_COL )
            lcl_r1c1_append_c ( r, nCol, nFlags & SCA_COL_ABSOLUTE, rDetails );
        break;
    }
}

static void
lcl_Split_DocTab( const ScDocument* pDoc,  SCTAB nTab,
                  const ScAddress::Details& rDetails,
                  USHORT nFlags,
                  String& rTabName, String& rDocName )
{
    pDoc->GetName( nTab, rTabName );
    rDocName.Erase();
#if 0
    {
        ByteString  aStr(rTabName, RTL_TEXTENCODING_UTF8);
        aStr.Append(static_cast< char >(0));
        std::cerr << "tabname \'" << aStr.GetBuffer() << '\'' << std::endl;
    }
#endif
    //  externe Referenzen (wie in ScCompiler::MakeTabStr)
    if ( rTabName.GetChar(0) == '\'' )
    {   // "'Doc'#Tab"
        xub_StrLen nPos, nLen = 1;
        while( (nPos = rTabName.Search( '\'', nLen )) != STRING_NOTFOUND )
            nLen = nPos + 1;
        if ( rTabName.GetChar(nLen) == SC_COMPILER_FILE_TAB_SEP )
        {
            rDocName = rTabName.Copy( 0, nLen + 1 );
            rTabName.Erase( 0, nLen + 1 );
        }
    }
    else if( nFlags & SCA_FORCE_DOC )
    {
        // VBA has an 'external' flag that forces the addition of the
        // tab name _and_ the doc name.  The VBA code would be
        // needlessly complicated if it constructed an actual external
        // reference so we add this somewhat cheesy kludge to force the
        // addition of the document name even for non-external references
        rDocName = getFileNameFromDoc( pDoc );
    }
    ScCompiler::CheckTabQuotes( rTabName, rDetails.eConv);
}

static void
lcl_ScRange_Format_XL_Header( String& r, const ScRange& rRange,
                              USHORT nFlags, ScDocument* pDoc,
                              const ScAddress::Details& rDetails )
{
    if( nFlags & SCA_TAB_3D )
    {
        String aTabName, aDocName;
        lcl_Split_DocTab( pDoc, rRange.aStart.Tab(), rDetails, nFlags,
                          aTabName, aDocName );
        if( aDocName.Len() > 0 )
        {
            r += '[';
            r += aDocName;
            r += ']';
        }
        r += aTabName;

        if( nFlags & SCA_TAB2_3D )
        {
            lcl_Split_DocTab( pDoc, rRange.aEnd.Tab(), rDetails, nFlags,
                              aTabName, aDocName );
            r += ':';
            r += aTabName;
        }
        r += '!';
    }
}

void ScRange::Format( String& r, USHORT nFlags, ScDocument* pDoc,
                      const ScAddress::Details& rDetails ) const
{
    r.Erase();
    if( !( nFlags & SCA_VALID ) )
    {
        r = ScGlobal::GetRscString( STR_NOREF_STR );
        return;
    }

#define absrel_differ(nFlags, mask) (((nFlags) & (mask)) ^ (((nFlags) >> 4) & (mask)))
    switch( rDetails.eConv ) {
    default :
    case ScAddress::CONV_OOO: {
        BOOL bOneTab = (aStart.Tab() == aEnd.Tab());
        if ( !bOneTab )
            nFlags |= SCA_TAB_3D;
        aStart.Format( r, nFlags, pDoc, rDetails );
        if( aStart != aEnd ||
            absrel_differ( nFlags, SCA_COL_ABSOLUTE ) ||
            absrel_differ( nFlags, SCA_ROW_ABSOLUTE ))
        {
            String aName;
            nFlags = ( nFlags & SCA_VALID ) | ( ( nFlags >> 4 ) & 0x070F );
            if ( bOneTab )
                pDoc = NULL;
            else
                nFlags |= SCA_TAB_3D;
            aEnd.Format( aName, nFlags, pDoc, rDetails );
            r += ':';
            r += aName;
        }
    }
    break;

    case ScAddress::CONV_XL_A1:
        lcl_ScRange_Format_XL_Header( r, *this, nFlags, pDoc, rDetails );
        if( aStart.Col() == 0 && aEnd.Col() >= MAXCOL )
        {
            // Full col refs always require 2 rows (2:2)
            lcl_a1_append_r( r, aStart.Row(), nFlags & SCA_ROW_ABSOLUTE );
            r += ':';
            lcl_a1_append_r( r, aEnd.Row(), nFlags & SCA_ROW2_ABSOLUTE );
        }
        else if( aStart.Row() == 0 && aEnd.Row() >= MAXROW )
        {
            // Full row refs always require 2 cols (A:A)
            lcl_a1_append_c( r, aStart.Col(), nFlags & SCA_COL_ABSOLUTE );
            r += ':';
            lcl_a1_append_c( r, aEnd.Col(), nFlags & SCA_COL2_ABSOLUTE );
        }
        else
        {
            lcl_a1_append_c ( r, aStart.Col(), nFlags & SCA_COL_ABSOLUTE );
            lcl_a1_append_r ( r, aStart.Row(), nFlags & SCA_ROW_ABSOLUTE );
            if( aStart.Col() != aEnd.Col() ||
                absrel_differ( nFlags, SCA_COL_ABSOLUTE ) ||
                aStart.Row() != aEnd.Row() ||
                absrel_differ( nFlags, SCA_ROW_ABSOLUTE )) {
                r += ':';
                lcl_a1_append_c ( r, aEnd.Col(), nFlags & SCA_COL2_ABSOLUTE );
                lcl_a1_append_r ( r, aEnd.Row(), nFlags & SCA_ROW2_ABSOLUTE );
            }
        }
    break;

    case ScAddress::CONV_XL_R1C1:
        lcl_ScRange_Format_XL_Header( r, *this, nFlags, pDoc, rDetails );
        if( aStart.Col() == 0 && aEnd.Col() >= MAXCOL )
        {
            lcl_r1c1_append_r( r, aStart.Row(), nFlags & SCA_ROW_ABSOLUTE, rDetails );
            if( aStart.Row() != aEnd.Row() ||
                absrel_differ( nFlags, SCA_ROW_ABSOLUTE )) {
                r += ':';
                lcl_r1c1_append_r( r, aEnd.Row(), nFlags & SCA_ROW2_ABSOLUTE, rDetails );
            }
        }
        else if( aStart.Row() == 0 && aEnd.Row() >= MAXROW )
        {
            lcl_r1c1_append_c( r, aStart.Col(), nFlags & SCA_COL_ABSOLUTE, rDetails );
            if( aStart.Col() != aEnd.Col() ||
                absrel_differ( nFlags, SCA_COL_ABSOLUTE )) {
                r += ':';
                lcl_r1c1_append_c( r, aEnd.Col(), nFlags & SCA_COL2_ABSOLUTE, rDetails );
            }
        }
        else
        {
            lcl_r1c1_append_r( r, aStart.Row(), nFlags & SCA_ROW_ABSOLUTE, rDetails );
            lcl_r1c1_append_c( r, aStart.Col(), nFlags & SCA_COL_ABSOLUTE, rDetails );
            if( aStart.Col() != aEnd.Col() ||
                absrel_differ( nFlags, SCA_COL_ABSOLUTE ) ||
                aStart.Row() != aEnd.Row() ||
                absrel_differ( nFlags, SCA_ROW_ABSOLUTE )) {
                r += ':';
                lcl_r1c1_append_r( r, aEnd.Row(), nFlags & SCA_ROW2_ABSOLUTE, rDetails );
                lcl_r1c1_append_c( r, aEnd.Col(), nFlags & SCA_COL2_ABSOLUTE, rDetails );
            }
        }
    }
#undef  absrel_differ
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


String ScAddress::GetColRowString( bool bAbsolute,
                                   const Details& rDetails ) const
{
    String aString;

    switch( rDetails.eConv )
    {
    default :
    case ScAddress::CONV_OOO:
    case ScAddress::CONV_XL_A1:
    if (bAbsolute)
        aString.Append( '$' );

    ColToAlpha( aString, nCol);

    if ( bAbsolute )
        aString.Append( '$' );

    aString += String::CreateFromInt32(nRow+1);
        break;

    case ScAddress::CONV_XL_R1C1:
        lcl_r1c1_append_r ( aString, nRow, bAbsolute, rDetails );
        lcl_r1c1_append_c ( aString, nCol, bAbsolute, rDetails );
        break;
    }

    return aString;
}


String ScRefAddress::GetRefString( ScDocument* pDoc, SCTAB nActTab,
                                   const ScAddress::Details& rDetails ) const
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

    aAdr.Format( aString, nFlags, pDoc, rDetails );

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
            nCol = sal::static_int_cast<SCCOL>( nCol - nC );
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
    while (nResult <= MAXCOL && nPos < nStop && (c = rStr.GetChar( nPos)) != 0 &&
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
