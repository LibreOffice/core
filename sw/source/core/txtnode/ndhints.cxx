/*************************************************************************
 *
 *  $RCSfile: ndhints.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:27 $
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

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#include "txatbase.hxx"
#include "ndhints.hxx"

#ifndef PRODUCT
#define CHECK   Check();
#else
#define CHECK
#endif

_SV_IMPL_SORTAR_ALG( SwpHtStart, SwTxtAttr* )
_SV_IMPL_SORTAR_ALG( SwpHtEnd, SwTxtAttr* )

#ifdef NIE

void DumpHints( const SwpHtStart &rHtStart,
                const SwpHtEnd &rHtEnd )
{
#ifndef PRODUCT
    aDbstream << "DumpHints:" << endl;
    (aDbstream << "\tStarts:" ).WriteNumber(rHtStart.Count()) << endl;
    for( USHORT i = 0; i < rHtStart.Count(); ++i )
    {
        const SwTxtAttr *pHt = rHtStart[i];
        ((((aDbstream << '\t').WriteNumber( i )<< " [").WriteNumber( pHt->Which() )
            << ']' << '\t').WriteNumber( long( pHt ) )
                  << '\t').WriteNumber( *pHt->GetStart() );
        if( pHt->GetEnd() )
            (aDbstream << " -> " ).WriteNumber( *pHt->GetEnd() );
        aDbstream << endl;
    }
    (aDbstream << "\tEnds:").WriteNumber( rHtEnd.Count() )<< endl;
    for( i = 0; i < rHtEnd.Count(); ++i )
    {
        const SwTxtAttr *pHt = rHtEnd[i];
        (((aDbstream << '\t').WriteNumber( i )<< " [").WriteNumber( pHt->Which() )
            << ']' << '\t' ).WriteNumber( long( pHt ) );
        if( pHt->GetEnd() )
            (aDbstream << '\t').WriteNumber( *pHt->GetEnd() )<< " <- ";
        aDbstream.WriteNumber( *pHt->GetStart() )<< endl;
    }
    aDbstream << endl;
#endif
}
#else
inline void DumpHints(const SwpHtStart &, const SwpHtEnd &) { }
#endif

/*************************************************************************
 *                        inline IsEqual()
 *************************************************************************/

inline BOOL IsEqual( const SwTxtAttr &rHt1, const SwTxtAttr &rHt2 )
{
    return (long)(&rHt1) == (long)(&rHt2);
}

/*************************************************************************
 *                      IsLessStart()
 *************************************************************************/

// SV_IMPL_OP_PTRARR_SORT( SwpHtStart, SwTxtAttr* )
// kein SV_IMPL_PTRARR_SORT( name,ArrElement )
// unser SEEK_PTR_TO_OBJECT_NOTL( name,ArrElement )

// Sortierreihenfolge: Start, Ende (umgekehrt!), Which-Wert (umgekehrt!),
//                     als letztes die Adresse selbst

BOOL lcl_IsLessStart( const SwTxtAttr &rHt1, const SwTxtAttr &rHt2 )
{
    if ( *rHt1.GetStart() == *rHt2.GetStart() )
    {
        xub_StrLen nHt1 = *rHt1.GetAnyEnd();
        xub_StrLen nHt2 = *rHt2.GetAnyEnd();
        if ( nHt1 == nHt2 )
        {
            nHt1 = rHt1.Which();
            nHt2 = rHt2.Which();
            return nHt1 > nHt2 ||
                (nHt1 == nHt2 && (long)&rHt1 < (long)&rHt2);
        }
        else
            return ( nHt1 > nHt2 );
    }
    return ( *rHt1.GetStart() < *rHt2.GetStart() );
}

/*************************************************************************
 *                      inline IsLessEnd()
 *************************************************************************/

// Zuerst nach Ende danach nach Ptr
#ifdef HP9000
BOOL lcl_IsLessEnd( const SwTxtAttr &rHt1, const SwTxtAttr &rHt2 )
#else
inline BOOL lcl_IsLessEnd( const SwTxtAttr &rHt1, const SwTxtAttr &rHt2 )
#endif
{
    xub_StrLen nHt1 = *rHt1.GetAnyEnd();
    xub_StrLen nHt2 = *rHt2.GetAnyEnd();
    if ( nHt1 == nHt2 )
    {
        if ( *rHt1.GetStart() == *rHt2.GetStart() )
        {
            nHt1 = rHt1.Which();
            nHt2 = rHt2.Which();
            return nHt1 < nHt2 ||
                (nHt1 == nHt2 && (long)&rHt1 > (long)&rHt2);
        }
        else
            return ( *rHt1.GetStart() > *rHt2.GetStart() );
    }
    return ( nHt1 < nHt2 );
}

/*************************************************************************
 *                      SwpHtStart::Seek_Entry()
 *************************************************************************/

BOOL SwpHtStart::Seek_Entry( const SwTxtAttr *pElement, USHORT *pPos ) const
{
    register USHORT nOben = Count(), nMitte, nUnten = 0;
    if( nOben > 0 )
    {
        nOben--;
        while( nUnten <= nOben )
        {
            nMitte = nUnten + ( nOben - nUnten ) / 2;
            const SwTxtAttr *pMitte = (*this)[nMitte];
            if( IsEqual( *pMitte, *pElement ) )
            {
                *pPos = nMitte;
                return TRUE;
            }
            else
                if( lcl_IsLessStart( *pMitte, *pElement ) )
                    nUnten = nMitte + 1;
                else
                    if( nMitte == 0 )
                    {
                        *pPos = nUnten;
                        return FALSE;
                    }
                    else
                        nOben = nMitte - 1;
        }
    }
    *pPos = nUnten;
    return FALSE;
}

/*************************************************************************
 *                      SwpHtEnd::Seek_Entry()
 *************************************************************************/

BOOL SwpHtEnd::Seek_Entry( const SwTxtAttr *pElement, USHORT *pPos ) const
{
    register USHORT nOben = Count(), nMitte, nUnten = 0;
    if( nOben > 0 )
    {
        nOben--;
        while( nUnten <= nOben )
        {
            nMitte = nUnten + ( nOben - nUnten ) / 2;
            const SwTxtAttr *pMitte = (*this)[nMitte];
            if( IsEqual( *pMitte, *pElement ) )
            {
                *pPos = nMitte;
                return TRUE;
            }
            else
                if( lcl_IsLessEnd( *pMitte, *pElement ) )
                    nUnten = nMitte + 1;
                else
                    if( nMitte == 0 )
                    {
                        *pPos = nUnten;
                        return FALSE;
                    }
                    else
                        nOben = nMitte - 1;
        }
    }
    *pPos = nUnten;
    return FALSE;
}

/*************************************************************************
 *                      class SwpHintsArr
 *************************************************************************/

void SwpHintsArr::Insert( const SwTxtAttr *pHt )
{
    Resort();
#ifndef PRODUCT
    USHORT nPos;
    ASSERT(!SwpHtStart::Seek_Entry( pHt, &nPos ), "Insert: hint already in HtStart");
    ASSERT(!aHtEnd.Seek_Entry( pHt, &nPos ), "Insert: hint already in HtEnd");
#endif
    SwpHtStart::Insert( pHt );
    aHtEnd.Insert( pHt );
#ifndef PRODUCT
#ifdef NIE
    (aDbstream << "Insert: " ).WriteNumber( long( pHt ) )<< endl;
    DumpHints( *this, aHtEnd );
#endif
#endif
    CHECK;
}

void SwpHintsArr::DeleteAtPos( const USHORT nPos )
{
    // Optimierung: nPos bezieht sich auf die Position im StartArray, also:
    const SwTxtAttr *pHt = SwpHtStart::operator[]( nPos );
    SwpHtStart::Remove( nPos );

    Resort();

    USHORT nEndPos;
    aHtEnd.Seek_Entry( pHt, &nEndPos );
    aHtEnd.Remove( nEndPos );
#ifndef PRODUCT
#ifdef NIE
    (aDbstream << "DeleteAtPos: " ).WriteNumber( long( pHt ) )<< endl;
    DumpHints( *this, aHtEnd );
#endif
#endif
    CHECK;
}

#ifndef PRODUCT

/*************************************************************************
 *                      SwpHintsArr::Check()
 *************************************************************************/


#define CHECK_ERR(cond, text) \
        if(!(cond)) \
        { \
            ASSERT(!this, text); \
            DumpHints(*(SwpHtStart*)this,aHtEnd); \
            const BOOL bErr = 0 == (cond); /* fuer den CV */ \
            return !((SwpHintsArr*)this)->Resort(); \
        }

BOOL SwpHintsArr::Check() const
{
    // 1) gleiche Anzahl in beiden Arrays
    CHECK_ERR( Count() == aHtEnd.Count(), "HintsCheck: wrong sizes" );
    xub_StrLen nLastStart = 0;
    xub_StrLen nLastEnd   = 0;

    const SwTxtAttr *pLastStart = 0;
    const SwTxtAttr *pLastEnd = 0;

    for( USHORT i = 0; i < Count(); ++i )
    {
        // --- Start-Kontrolle ---

        // 2a) gueltiger Pointer? vgl. DELETEFF
        const SwTxtAttr *pHt = (*this)[i];
        CHECK_ERR( 0xFF != *(char*)pHt, "HintsCheck: start ptr was deleted" );

        // 3a) Stimmt die Start-Sortierung?
        xub_StrLen nIdx = *pHt->GetStart();
        CHECK_ERR( nIdx >= nLastStart, "HintsCheck: starts are unsorted" );

        // 4a) IsLessStart-Konsistenz
        if( pLastStart )
            CHECK_ERR( lcl_IsLessStart( *pLastStart, *pHt ), "HintsCheck: IsLastStart" );

        nLastStart = nIdx;
        pLastStart = pHt;

        // --- End-Kontrolle ---

        // 2b) gueltiger Pointer? vgl. DELETEFF
        const SwTxtAttr *pHtEnd = aHtEnd[i];
        CHECK_ERR( 0xFF != *(char*)pHtEnd, "HintsCheck: end ptr was deleted" );

        // 3b) Stimmt die End-Sortierung?
        nIdx = *pHtEnd->GetAnyEnd();
        CHECK_ERR( nIdx >= nLastEnd, "HintsCheck: ends are unsorted" );
        nLastEnd = nIdx;

        // 4b) IsLessEnd-Konsistenz
        if( pLastEnd )
            CHECK_ERR( lcl_IsLessEnd( *pLastEnd, *pHtEnd ), "HintsCheck: IsLastEnd" );

        nLastEnd = nIdx;
        pLastEnd = pHtEnd;

        // --- Ueberkreuzungen ---

        // 5) gleiche Pointer in beiden Arrays
        nIdx = GetStartOf( pHtEnd );
        CHECK_ERR( STRING_LEN != nIdx, "HintsCheck: no GetStartOf" );

        // 6) gleiche Pointer in beiden Arrays
        nIdx = GetEndOf( pHt );
        CHECK_ERR( STRING_LEN != nIdx, "HintsCheck: no GetEndOf" );
    }
    return TRUE;
}

#endif      /* PRODUCT */

/*************************************************************************
 *                          SwpHintsArr::Resort()
 *************************************************************************/

// Resort() wird vor jedem Insert und Delete gerufen.
// Wenn Textmasse geloescht wird, so werden die Indizes in
// ndtxt.cxx angepasst. Leider erfolgt noch keine Neusortierung
// auf gleichen Positionen.

BOOL SwpHintsArr::Resort()
{
    BOOL bResort = FALSE;
    const SwTxtAttr *pLast = 0;
    for( USHORT i = 0; i < SwpHtStart::Count(); ++i )
    {
        const SwTxtAttr *pHt = (*this)[i];
        if( pLast && !lcl_IsLessStart( *pLast, *pHt ) )
        {
#ifdef NIE
#ifndef PRODUCT
//            ASSERT( bResort, "!Resort/Start: correcting hints-array" );
            aDbstream << "Resort: Starts" << endl;
            DumpHints( *this, aHtEnd );
#endif
#endif
            // Aufpassen: nicht die unsere SwpHintsArr-Methoden rufen,
            // weil dort ein Resort steht!
            // AMA: Bisher ( -> USED ) wurde ein Arrayinhalt [3,4,4,3] nur in
            //      [3,4,3,4] sortiert, nicht in [3,3,4,4]
#ifdef USED
            SwpHtStart::Delete( i - 1 );
            SwpHtStart::Insert( pLast );
            USHORT nPos;
            if( SwpHtStart::Seek_Entry( pLast, &nPos ) && nPos > i )
                --i;
#else
            SwpHtStart::Remove( i );
            SwpHtStart::Insert( pHt );
            pHt = (*this)[i];
            if ( pHt != pLast )
                --i;
#endif //!USED
            bResort = TRUE;
        }
        pLast = pHt;
    }

    pLast = 0;
    for( i = 0; i < aHtEnd.Count(); ++i )
    {
        const SwTxtAttr *pHt = aHtEnd[i];
        if( pLast && !lcl_IsLessEnd( *pLast, *pHt ) )
        {
#ifdef NIE
#ifndef PRODUCT
//            ASSERT( bResort, "!Resort/Ends: correcting hints-array" );
            aDbstream << "Resort: Ends" << endl;
            DumpHints( *this, aHtEnd );
#endif
#endif
// AMA: siehe oben
#ifdef USED
            aHtEnd.Delete( i - 1 );
            aHtEnd.Insert( pLast );
            USHORT nPos;
            if( aHtEnd.Seek_Entry( pLast, &nPos ) && nPos > i )
                --i;
#else
            aHtEnd.Remove( i );
            aHtEnd.Insert( pHt );
            pHt = aHtEnd[i]; // normalerweise == pLast
            // Wenn die Unordnung etwas groesser ist (24200),
            // muessen wir Position i erneut vergleichen.
            if ( pLast != pHt )
                --i;
#endif //!USED
            bResort = TRUE;
        }
        pLast = pHt;
    }
#ifndef PRODUCT
#ifdef NIE
    aDbstream << "Resorted:" << endl;
    DumpHints( *this, aHtEnd );
#endif
#endif
    return bResort;
}


