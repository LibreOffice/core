/*************************************************************************
 *
 *  $RCSfile: tokstack.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:12 $
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
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#ifndef PCH
#include <segmentc.hxx>
#include <string.h>
#endif

#include "compiler.hxx"
#include "tokstack.hxx"
#include "global.hxx"

const UINT16    TokenPool::nScTokenOff = 8192;


TokenStack::TokenStack( UINT16 nNewSize )
{
    pStack = new TokenId[ nNewSize ];

    Reset();
    nSize = nNewSize;
}


TokenStack::~TokenStack()
{
    delete[] pStack;
}




//------------------------------------------------------------------------

// !ACHTUNG!: nach Aussen hin beginnt die Nummerierung mit 1!
// !ACHTUNG!: SC-Token werden mit einem Offset nScTokenOff abgelegt
//              -> Unterscheidung von anderen Token


TokenPool::TokenPool( void )
{
    UINT16  nLauf = nScTokenOff;

    // Sammelstelle fuer Id-Folgen
    nP_Id = 256;
    pP_Id = new UINT16[ nP_Id ];

    // Sammelstelle fuer Ids
    nElement = 32;
    pElement = new UINT16[ nElement ];
    pType = new E_TYPE[ nElement ];
    pSize = new UINT16[ nElement ];
    nP_IdLast = 0;

    // Sammelstelle fuer Strings
    nP_Str = 4;
    ppP_Str = new String *[ nP_Str ];
    for( nLauf = 0 ; nLauf < nP_Str ; nLauf++ )
        ppP_Str[ nLauf ] = NULL;

    // Sammelstelle fuer double
    nP_Dbl = 8;
    pP_Dbl = new double[ nP_Dbl ];

    // Sammelstellen fuer Referenzen
    nP_RefTr = 32;
    ppP_RefTr = new SingleRefData *[ nP_RefTr ];
    for( nLauf = 0 ; nLauf < nP_RefTr ; nLauf++ )
        ppP_RefTr[ nLauf ] = NULL;

    nP_Ext = 32;
    ppP_Ext = new EXTCONT*[ nP_Ext ];
    memset( ppP_Ext, NULL, sizeof( EXTCONT* ) * nP_Ext );

    pScToken = new ScTokenArray;

    Reset();
}


TokenPool::~TokenPool()
{
    UINT16  n;

    delete[] pP_Id;
    delete[] pElement;
    delete[] pType;
    delete[] pSize;
    delete[] pP_Dbl;

    for( n = 0 ; n < nP_RefTr ; n++/*, pAktTr++*/ )
    {
        if( ppP_RefTr[ n ] )
            delete ppP_RefTr[ n ];
    }
    delete[] ppP_RefTr;

    for( n = 0 ; n < nP_Str ; n++/*, pAktStr++*/ )
    {
        if( ppP_Str[ n ] )
            delete ppP_Str[ n ];
    }
    delete[] ppP_Str;

    for( n = 0 ; n < nP_Ext ; n++ )
    {
        if( ppP_Ext[ n ] )
            delete ppP_Ext[ n ];
    }
    delete[] ppP_Ext;

    delete pScToken;
}


void TokenPool::GrowString( void )
{
    UINT16      nP_StrNew = nP_Str * 2;
    UINT16      nL;

    String**    ppP_StrNew = new String *[ nP_StrNew ];

    for( nL = 0 ; nL < nP_Str ; nL++ )
        ppP_StrNew[ nL ] = ppP_Str[ nL ];
    for( nL = nP_Str ; nL < nP_StrNew ; nL++ )
        ppP_StrNew[ nL ] = NULL;

    nP_Str = nP_StrNew;

    delete[]    ppP_Str;
    ppP_Str = ppP_StrNew;
}


void TokenPool::GrowDouble( void )
{
    UINT16      nP_DblNew = nP_Dbl * 2;

    double*     pP_DblNew = new double[ nP_DblNew ];

    for( UINT16 nL = 0 ; nL < nP_Dbl ; nL++ )
        pP_DblNew[ nL ] = pP_Dbl[ nL ];

    nP_Dbl = nP_DblNew;

    delete[] pP_Dbl;
    pP_Dbl = pP_DblNew;
}


void TokenPool::GrowTripel( void )
{
    UINT16          nP_RefTrNew = nP_RefTr * 2;
    UINT16          nL;

    SingleRefData** ppP_RefTrNew = new SingleRefData *[ nP_RefTrNew ];

    for( nL = 0 ; nL < nP_RefTr ; nL++ )
        ppP_RefTrNew[ nL ] = ppP_RefTr[ nL ];
    for( nL = nP_RefTr ; nL < nP_RefTrNew ; nL++ )
        ppP_RefTrNew[ nL ] = NULL;

    nP_RefTr = nP_RefTrNew;

    delete[] ppP_RefTr;
    ppP_RefTr = ppP_RefTrNew;
}


void TokenPool::GrowId( void )
{
    UINT16  nP_IdNew = nP_Id * 2;

    UINT16* pP_IdNew = new UINT16[ nP_IdNew ];

    for( UINT16 nL = 0 ; nL < nP_Id ; nL++ )
        pP_IdNew[ nL ] = pP_Id[ nL ];

    nP_Id = nP_IdNew;

    delete[] pP_Id;
    pP_Id = pP_IdNew;
}


void TokenPool::GrowElement( void )
{
    UINT16  nElementNew = nElement * 2;

    UINT16* pElementNew = new UINT16[ nElementNew ];
    E_TYPE* pTypeNew = new E_TYPE[ nElementNew ];
    UINT16* pSizeNew = new UINT16[ nElementNew ];

    for( UINT16 nL = 0 ; nL < nElement ; nL++ )
    {
        pElementNew[ nL ] = pElement[ nL ];
        pTypeNew[ nL ] = pType[ nL ];
        pSizeNew[ nL ] = pSize[ nL ];
    }

    nElement = nElementNew;

    delete[] pElement;
    delete[] pType;
    delete[] pSize;
    pElement = pElementNew;
    pType = pTypeNew;
    pSize = pSizeNew;
}


void TokenPool::GrowExt( void )
{
    UINT16      nNewSize = nP_Ext * 2;

    EXTCONT**   ppNew = new EXTCONT*[ nNewSize ];

    memcpy( ppNew, ppP_Ext, sizeof( EXTCONT* ) * nP_Ext );

    delete[] ppP_Ext;
    nP_Ext = nNewSize;
}


void TokenPool::GetElement( const UINT16 nId )
{
    DBG_ASSERT( nId < nElementAkt, "*TokenPool::GetElement(): Id zu gross!?" );

    if( pType[ nId ] == T_Id )
        GetElementRek( nId );
    else
    {
        switch( pType[ nId ] )
        {
#ifdef DBG_UTIL
            case T_Id:
                DBG_ERROR( "-TokenPool::GetElement(): hier hast Du nichts zu suchen!" );
                break;
#endif
            case T_Str:
                pScToken->AddString( ppP_Str[ pElement[ nId ] ]->GetBuffer() );
                break;
            case T_D:
                pScToken->AddDouble( pP_Dbl[ pElement[ nId ] ] );
                break;
            case T_RefC:
                pScToken->AddSingleReference( *ppP_RefTr[ pElement[ (UINT16) nId ] ] );
                break;
            case T_RefA:
            {
                ComplRefData    aComplRefData;
                aComplRefData.Ref1 = *ppP_RefTr[ pElement[ nId ] ];
                aComplRefData.Ref2 = *ppP_RefTr[ pElement[ nId ] + 1 ];
                pScToken->AddDoubleReference( aComplRefData );
            }
                break;
            case T_RN:
                pScToken->AddName( pElement[ nId ] );
                break;
            case T_Ext:
            {
                UINT16          n = pElement[ nId ];
                EXTCONT*        p = ( n < nP_Ext )? ppP_Ext[ n ] : NULL;

                if( p )
                    ScToken*    pTok = pScToken->AddExternal( p->aText.GetBuffer() );
            }
                break;
            default:
                DBG_ERROR("-TokenPool::GetElement(): Zustand undefiniert!?");
        }
    }
}


void TokenPool::GetElementRek( const UINT16 nId )
{
#ifdef DBG_UTIL
    nRek++;
    DBG_ASSERT( nRek <= nP_Id, "*TokenPool::GetElement(): Rekursion loopt!?" );
#endif

    DBG_ASSERT( nId < nElementAkt, "*TokenPool::GetElementRek(): Id zu gross!?" );

    DBG_ASSERT( pType[ nId ] == T_Id, "-TokenPool::GetElementRek(): nId nicht Id-Folge!" );


    UINT16      nAnz = pSize[ nId ];
    UINT16*     pAkt = &pP_Id[ pElement[ nId ] ];
    for( ; nAnz > 0 ; nAnz--, pAkt++ )
    {
        if( *pAkt < nScTokenOff )
        {// Rekursion oder nicht?
            switch( pType[ ( TokenId ) *pAkt ] )
            {
                case T_Id:
                    GetElementRek( ( TokenId ) *pAkt );
                    break;
                case T_Str:
                    pScToken->AddString( ppP_Str[ pElement[ ( TokenId ) *pAkt ] ]->GetBuffer() );
                    break;
                case T_D:
                    pScToken->AddDouble( pP_Dbl[ pElement[ ( TokenId ) *pAkt ] ] );
                    break;
                case T_RefC:
                    pScToken->AddSingleReference( *ppP_RefTr[ pElement[ (UINT16) *pAkt ] ] );
                    break;
                case T_RefA:
                {
                    ComplRefData    aComplRefData;
                    aComplRefData.Ref1 = *ppP_RefTr[ pElement[ ( TokenId ) *pAkt ] ];
                    aComplRefData.Ref2 = *ppP_RefTr[ pElement[ ( TokenId ) *pAkt ] + 1 ];
                    pScToken->AddDoubleReference( aComplRefData );
                }
                    break;
                case T_RN:
                    pScToken->AddName( pElement[ ( TokenId ) *pAkt ] );
                    break;
                case T_Ext:
                {
                    UINT16      n = pElement[ ( TokenId ) *pAkt ];
                    EXTCONT*    p = ( n < nP_Ext )? ppP_Ext[ n ] : NULL;

                    if( p )
                        ScToken*    pTok = pScToken->AddExternal( p->aText.GetBuffer() );
                }
                    break;
                default:
                    DBG_ERROR("-TokenPool::GetElementRek(): Zustand undefiniert!?");
            }
        }
        else    // elementarer SC_Token
            pScToken->AddOpCode( ( DefTokenId ) ( *pAkt - nScTokenOff ) );
    }


#ifdef DBG_UTIL
    nRek--;
#endif
}


void TokenPool::operator >>( TokenId& rId )
{
    rId = ( TokenId ) ( nElementAkt + 1 );

    if( nElementAkt >= nElement )
        GrowElement();

    pElement[ nElementAkt ] = nP_IdLast;    // Start der Token-Folge
    pType[ nElementAkt ] = T_Id;            // Typinfo eintragen
    pSize[ nElementAkt ] = nP_IdAkt - nP_IdLast;
        // von nP_IdLast bis nP_IdAkt-1 geschrieben -> Laenge der Folge

    nElementAkt++;          // Startwerte fuer naechste Folge
    nP_IdLast = nP_IdAkt;
}


TokenId TokenPool::Store( const double& rDouble )
{
    if( nElementAkt >= nElement )
        GrowElement();

    if( nP_DblAkt >= nP_Dbl )
        GrowDouble();

    pElement[ nElementAkt ] = nP_DblAkt;    // Index in Double-Array
    pType[ nElementAkt ] = T_D;             // Typinfo Double eintragen

    pP_Dbl[ nP_DblAkt ] = rDouble;

    pSize[ nElementAkt ] = 1;           // eigentlich Banane

    nElementAkt++;
    nP_DblAkt++;

    return ( TokenId ) nElementAkt; // Ausgabe von altem Wert + 1!
}


TokenId TokenPool::Store( const UINT16 nIndex )
{
    if( nElementAkt >= nElement )
        GrowElement();

    pElement[ nElementAkt ] = nIndex;           // Daten direkt im Index!
    pType[ nElementAkt ] = T_RN;                // Typinfo Range Name eintragen

    nElementAkt++;
    return ( TokenId ) nElementAkt;             // Ausgabe von altem Wert + 1!
}


TokenId TokenPool::Store( const String& rString )
{
    // weitgehend nach Store( const sal_Char* ) kopiert, zur Vermeidung
    //  eines temporaeren Strings in "
    if( nElementAkt >= nElement )
        GrowElement();

    if( nP_StrAkt >= nP_Str )
        GrowString();

    pElement[ nElementAkt ] = nP_StrAkt;    // Index in String-Array
    pType[ nElementAkt ] = T_Str;           // Typinfo String eintragen

    // String anlegen
    if( !ppP_Str[ nP_StrAkt ] )
        //...aber nur, wenn noch nicht vorhanden
        ppP_Str[ nP_StrAkt ] = new String( rString );
    else
        //...ansonsten nur kopieren
        *ppP_Str[ nP_StrAkt ] = rString;

    DBG_ASSERT( sizeof( xub_StrLen ) <= 2, "*TokenPool::Store(): StrLen doesn't match!" );

    pSize[ nElementAkt ] = ( UINT16 ) ppP_Str[ nP_StrAkt ]->Len();

    nElementAkt++;
    nP_StrAkt++;

    return ( TokenId ) nElementAkt; // Ausgabe von altem Wert + 1!
}


TokenId TokenPool::Store( const SingleRefData& rTr )
{
    if( nElementAkt >= nElement )
        GrowElement();

    if( nP_RefTrAkt >= nP_RefTr )
        GrowTripel();

    pElement[ nElementAkt ] = nP_RefTrAkt;
    pType[ nElementAkt ] = T_RefC;          // Typinfo Cell-Reff eintragen

    if( !ppP_RefTr[ nP_RefTrAkt ] )
        ppP_RefTr[ nP_RefTrAkt ] = new SingleRefData( rTr );
    else
        *ppP_RefTr[ nP_RefTrAkt ] = rTr;

    nElementAkt++;
    nP_RefTrAkt++;

    return ( TokenId ) nElementAkt; // Ausgabe von altem Wert + 1!
}


TokenId TokenPool::Store( const ComplRefData& rTr )
{
    if( nElementAkt >= nElement )
        GrowElement();

    if( nP_RefTrAkt + 1 >= nP_RefTr )
        GrowTripel();

    pElement[ nElementAkt ] = nP_RefTrAkt;
    pType[ nElementAkt ] = T_RefA;          // Typinfo Area-Reff eintragen

    if( !ppP_RefTr[ nP_RefTrAkt ] )
        ppP_RefTr[ nP_RefTrAkt ] = new SingleRefData( rTr.Ref1 );
    else
        *ppP_RefTr[ nP_RefTrAkt ] = rTr.Ref1;
    nP_RefTrAkt++;

    if( !ppP_RefTr[ nP_RefTrAkt ] )
        ppP_RefTr[ nP_RefTrAkt ] = new SingleRefData( rTr.Ref2 );
    else
        *ppP_RefTr[ nP_RefTrAkt ] = rTr.Ref2;
    nP_RefTrAkt++;

    nElementAkt++;

    return ( TokenId ) nElementAkt; // Ausgabe von altem Wert + 1!
}


TokenId TokenPool::Store( const DefTokenId e, const String& r )
{
    if( nElementAkt >= nElement )
        GrowElement();

    if( nP_ExtAkt >= nP_Ext )
        GrowString();

    pElement[ nElementAkt ] = nP_ExtAkt;
    pType[ nElementAkt ] = T_Ext;           // Typinfo String eintragen

    if( ppP_Ext[ nP_ExtAkt ] )
    {
        ppP_Ext[ nP_ExtAkt ]->eId = e;
        ppP_Ext[ nP_ExtAkt ]->aText = r;
    }
    else
        ppP_Ext[ nP_ExtAkt ] = new EXTCONT( e, r );

    nElementAkt++;
    nP_ExtAkt++;

    return ( TokenId ) nElementAkt; // Ausgabe von altem Wert + 1!
}


void TokenPool::Reset( void )
{
    nP_IdAkt = nP_IdLast = nElementAkt = nP_StrAkt = nP_DblAkt = nP_RefTrAkt = nP_ExtAkt = 0;
}


BOOL TokenPool::IsSingleOp( TokenId nId, const DefTokenId eId ) const
{
    if( nId && nId <= nElementAkt )
    {// existent?
        nId--;
        if( T_Id == pType[ nId ] )
        {// Tokenfolge?
            if( pSize[ nId ] == 1 )
            {// GENAU 1 Token
                UINT16  nSecId = pP_Id[ pElement[ nId ] ];
                if( nSecId >= nScTokenOff )
                {// Default-Token?
                    return ( DefTokenId ) ( nSecId - nScTokenOff ) == eId;  // Gesuchter?
                }
            }
        }
    }

    return FALSE;
}


BOOL TokenPool::IsExternal( TokenId n ) const
{
    if( n && n <= nElementAkt )
    {
        n--;
        return ( pType[ n ] == T_Ext );
    }

    return FALSE;
}


const String* TokenPool::GetString( TokenId n ) const
{
    const String*   p = NULL;
    if( n && n <= nElementAkt )
    {
        n--;
        if( pType[ n ] == T_Str )
            p = ppP_Str[ pElement[ n ] ];
    }

    return p;
}




