/*************************************************************************
 *
 *  $RCSfile: acmplwrd.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:15 $
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


#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _INTN_HXX //autogen
#include <tools/intn.hxx>
#endif
#ifndef _APP_HXX //autogen
#include <vcl/svapp.hxx>
#endif

#ifndef _ACMPLWRD_HXX //autogen
#include <acmplwrd.hxx>
#endif
#ifndef _DOC_HXX //autogen
#include <doc.hxx>
#endif
#ifndef _NDINDEX_HXX //autogen
#include <ndindex.hxx>
#endif
#ifndef _NODE_HXX //autogen
#include <node.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _NDTXT_HXX //autogen
#include <ndtxt.hxx>
#endif
#ifndef _PAM_HXX //autogen
#include <pam.hxx>
#endif


SwAutoCompleteWord::SwAutoCompleteWord( USHORT nWords, USHORT nMWrdLen )
    : aWordLst( 0, 255 ), aLRULst( 0, 255 ),
    nMaxCount( nWords ),
    nMinWrdLen( nMWrdLen ),
    bLockWordLst( FALSE )
{
}

SwAutoCompleteWord::~SwAutoCompleteWord()
{
}

BOOL SwAutoCompleteWord::InsertWord( const String& rWord )
{
    BOOL bRet = FALSE;
    xub_StrLen nWrdLen = rWord.Len();
    while( nWrdLen && '.' == rWord.GetChar( nWrdLen-1 ))
        --nWrdLen;

    if( !bLockWordLst && nWrdLen > nMinWrdLen )
    {
        StringPtr pNew = new String( rWord, 0, nWrdLen );
        USHORT nInsPos;
        if( aWordLst.Insert( pNew, nInsPos ) )
        {
            bRet = TRUE;
            if( aLRULst.Count() < nMaxCount )
                aLRULst.Insert( pNew, 0 );
            else
            {
                // der letzte muss entfernt werden
                // damit der neue vorne Platz hat
                String* pDel = (String*)aLRULst[ nMaxCount - 1 ];

                void** ppData = (void**)aLRULst.GetData();
                memmove( ppData+1, ppData, (nMaxCount - 1) * sizeof( void* ));
                *ppData = pNew;

                aWordLst.Remove( pDel );
                delete pDel;
            }
        }
        else
        {
            delete pNew;
            // dann aber auf jedenfall nach "oben" moven
            pNew = aWordLst[ nInsPos ];
            nInsPos = aLRULst.GetPos( (void*)pNew );
            ASSERT( USHRT_MAX != nInsPos, "String nicht gefunden" );
            if( nInsPos )
            {
                void** ppData = (void**)aLRULst.GetData();
                memmove( ppData+1, ppData, nInsPos * sizeof( void* ) );
                *ppData = pNew;
            }
        }
    }
    return bRet;
}

BOOL SwAutoCompleteWord::RemoveWord( const String& rWord )
{
    BOOL bRet = FALSE;
    USHORT nPos;
    const StringPtr pStr = (StringPtr)&rWord;
    if( !bLockWordLst && aWordLst.Seek_Entry( pStr, &nPos ))
    {
        void* pDel = aWordLst[ nPos ];
        aWordLst.DeleteAndDestroy( nPos );

        nPos = aLRULst.GetPos( pDel );
        ASSERT( USHRT_MAX != nPos, "String nicht gefunden" );
        aLRULst.Remove( nPos );
    }
    return bRet;
}

BOOL SwAutoCompleteWord::SearchWord( const String& rWord, USHORT* pFndPos ) const
{
    const StringPtr pStr = (StringPtr)&rWord;
    return aWordLst.Seek_Entry( pStr, pFndPos );
}


BOOL SwAutoCompleteWord::SetToTop( const String& rWord )
{
    BOOL bRet = FALSE;
    USHORT nPos;
    const StringPtr pStr = (StringPtr)&rWord;
    if( !bLockWordLst && aWordLst.Seek_Entry( pStr, &nPos ))
    {
        bRet = TRUE;
        void* pTop = aWordLst[ nPos ];

        nPos = aLRULst.GetPos( pTop );
        ASSERT( USHRT_MAX != nPos, "String nicht gefunden" );
        if( nPos )
        {
            void** ppData = (void**)aLRULst.GetData();
            memmove( ppData+1, ppData, nPos * sizeof( void* ) );
            *ppData = pTop;
        }
    }
    return bRet;
}

void SwAutoCompleteWord::SetMaxCount( USHORT nNewMax )
{
    if( nNewMax < nMaxCount && aLRULst.Count() > nNewMax )
    {
        // dann die unten ueberhaengenden entfernen
        while( nNewMax >= aWordLst.Count() )
        {
            USHORT nPos = aWordLst.GetPos( (String*)aLRULst[ nNewMax-1 ] );
            ASSERT( USHRT_MAX != nPos, "String nicht gefunden" );
            aWordLst.DeleteAndDestroy( nPos );
        }
        aLRULst.Remove( nNewMax-1, aLRULst.Count() - nNewMax );
    }
    nMaxCount = nNewMax;
}

void SwAutoCompleteWord::SetMinWordLen( USHORT n )
{
    // will man wirklich alle Worte, die kleiner als die neue Min Laenge
    // sind entfernen?
    // JP 02.02.99 - erstmal nicht.

    // JP 11.03.99 - mal testhalber eingebaut
    if( n < nMinWrdLen )
    {
        for( USHORT nPos = 0; nPos < aWordLst.Count(); ++nPos  )
            if( aWordLst[ nPos ]->Len() < n )
            {
                void* pDel = aWordLst[ nPos ];
                aWordLst.DeleteAndDestroy( nPos );

                USHORT nDelPos = aLRULst.GetPos( pDel );
                ASSERT( USHRT_MAX != nDelPos, "String nicht gefunden" );
                aLRULst.Remove( nDelPos );
                --nPos;
            }
    }

    nMinWrdLen = n;
}

BOOL SwAutoCompleteWord::GetRange( const String& rWord, USHORT& rStt,
                                    USHORT& rEnd ) const
{
    const StringPtr pStr = (StringPtr)&rWord;
    aWordLst.Seek_Entry( pStr, &rStt );
    rEnd = rStt;

    const International& rInter = Application::GetAppInternational();
    while( rEnd < aWordLst.Count() && COMPARE_EQUAL ==
//!!! UNICODE: fehlendes interface
//          rInter.Compare( rWord, *aWordLst[ rEnd ]/*, rWord.Len()*/,
//                          INTN_COMPARE_IGNORECASE ) )
            rWord.CompareIgnoreCaseToAscii( *aWordLst[ rEnd ], rWord.Len() ) )
        ++rEnd;

    return rStt < rEnd;
}

void SwAutoCompleteWord::CheckChangedList( const SvStringsISortDtor& rNewLst )
{
    USHORT nMyLen = aWordLst.Count(), nNewLen = rNewLst.Count();
    USHORT nMyPos = 0, nNewPos = 0;

    for( ; nMyPos < nMyLen && nNewPos < nNewLen; ++nMyPos, ++nNewPos )
    {
        const StringPtr pStr = rNewLst[ nNewPos ];
        while( aWordLst[ nMyPos ] != pStr )
        {
            void* pDel = aWordLst[ nMyPos ];
            aWordLst.DeleteAndDestroy( nMyPos );

            USHORT nPos = aLRULst.GetPos( pDel );
            ASSERT( USHRT_MAX != nPos, "String nicht gefunden" );
            aLRULst.Remove( nPos );
            if( nMyPos >= --nMyLen )
                break;
        }
    }
    if( nMyPos < nMyLen )
    {
        for( ; nNewPos < nMyLen; ++nNewPos )
        {
            void* pDel = aWordLst[ nNewPos ];
            USHORT nPos = aLRULst.GetPos( pDel );
            ASSERT( USHRT_MAX != nPos, "String nicht gefunden" );
            aLRULst.Remove( nPos );
        }
        aWordLst.DeleteAndDestroy( nMyPos, nMyLen - nMyPos );
    }
}




