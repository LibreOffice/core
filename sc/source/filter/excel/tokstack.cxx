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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"


#ifndef PCH
#include <string.h>
#endif

#include "compiler.hxx"
#include "tokstack.hxx"
#include "global.hxx"
#include "scmatrix.hxx"

#include <stdio.h> // printf

const sal_uInt16    TokenPool::nScTokenOff = 8192;


TokenStack::TokenStack( sal_uInt16 nNewSize )
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
    sal_uInt16  nLauf = nScTokenOff;

    // Sammelstelle fuer Id-Folgen
    nP_Id = 256;
    pP_Id = new sal_uInt16[ nP_Id ];

    // Sammelstelle fuer Ids
    nElement = 32;
    pElement = new sal_uInt16[ nElement ];
    pType = new E_TYPE[ nElement ];
    pSize = new sal_uInt16[ nElement ];
    nP_IdLast = 0;

    // Sammelstelle fuer Strings
    nP_Str = 4;
    ppP_Str = new String *[ nP_Str ];
    for( nLauf = 0 ; nLauf < nP_Str ; nLauf++ )
        ppP_Str[ nLauf ] = NULL;

    // Sammelstelle fuer double
    nP_Dbl = 8;
    pP_Dbl = new double[ nP_Dbl ];

    // Sammelstelle fuer error codes
    nP_Err = 8;
    pP_Err = new sal_uInt16[ nP_Err ];

    // Sammelstellen fuer Referenzen
    nP_RefTr = 32;
    ppP_RefTr = new ScSingleRefData *[ nP_RefTr ];
    for( nLauf = 0 ; nLauf < nP_RefTr ; nLauf++ )
        ppP_RefTr[ nLauf ] = NULL;

    nP_Ext = 32;
    ppP_Ext = new EXTCONT*[ nP_Ext ];
    memset( ppP_Ext, 0, sizeof( EXTCONT* ) * nP_Ext );

    nP_Nlf = 16;
    ppP_Nlf = new NLFCONT*[ nP_Nlf ];
    memset( ppP_Nlf, 0, sizeof( NLFCONT* ) * nP_Nlf );

    nP_Matrix = 16;
    ppP_Matrix = new ScMatrix*[ nP_Matrix ];
    memset( ppP_Matrix, 0, sizeof( ScMatrix* ) * nP_Matrix );

    pScToken = new ScTokenArray;

    Reset();
}


TokenPool::~TokenPool()
{
    sal_uInt16  n;

    delete[] pP_Id;
    delete[] pElement;
    delete[] pType;
    delete[] pSize;
    delete[] pP_Dbl;
    delete[] pP_Err;

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

    for( n = 0 ; n < nP_Nlf ; n++ )
    {
        if( ppP_Nlf[ n ] )
            delete ppP_Nlf[ n ];
    }
    delete[] ppP_Nlf;

    for( n = 0 ; n < nP_Matrix ; n++ )
    {
        if( ppP_Matrix[ n ] )
            ppP_Matrix[ n ]->DecRef( );
    }
    delete[] ppP_Matrix;

    delete pScToken;
}


void TokenPool::GrowString( void )
{
    sal_uInt16      nP_StrNew = nP_Str * 2;
    sal_uInt16      nL;

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
    sal_uInt16      nP_DblNew = nP_Dbl * 2;

    double*     pP_DblNew = new double[ nP_DblNew ];

    for( sal_uInt16 nL = 0 ; nL < nP_Dbl ; nL++ )
        pP_DblNew[ nL ] = pP_Dbl[ nL ];

    nP_Dbl = nP_DblNew;

    delete[] pP_Dbl;
    pP_Dbl = pP_DblNew;
}

void TokenPool::GrowTripel( void )
{
    sal_uInt16          nP_RefTrNew = nP_RefTr * 2;
    sal_uInt16          nL;

    ScSingleRefData**   ppP_RefTrNew = new ScSingleRefData *[ nP_RefTrNew ];

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
    sal_uInt16  nP_IdNew = nP_Id * 2;

    sal_uInt16* pP_IdNew = new sal_uInt16[ nP_IdNew ];

    for( sal_uInt16 nL = 0 ; nL < nP_Id ; nL++ )
        pP_IdNew[ nL ] = pP_Id[ nL ];

    nP_Id = nP_IdNew;

    delete[] pP_Id;
    pP_Id = pP_IdNew;
}


void TokenPool::GrowElement( void )
{
    sal_uInt16  nElementNew = nElement * 2;

    sal_uInt16* pElementNew = new sal_uInt16[ nElementNew ];
    E_TYPE* pTypeNew = new E_TYPE[ nElementNew ];
    sal_uInt16* pSizeNew = new sal_uInt16[ nElementNew ];

    for( sal_uInt16 nL = 0 ; nL < nElement ; nL++ )
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
    sal_uInt16      nNewSize = nP_Ext * 2;

    EXTCONT**   ppNew = new EXTCONT*[ nNewSize ];

    memset( ppNew, 0, sizeof( EXTCONT* ) * nNewSize );
    memcpy( ppNew, ppP_Ext, sizeof( EXTCONT* ) * nP_Ext );

    delete[] ppP_Ext;
    ppP_Ext = ppNew;
    nP_Ext = nNewSize;
}


void TokenPool::GrowNlf( void )
{
    sal_uInt16      nNewSize = nP_Nlf * 2;

    NLFCONT**   ppNew = new NLFCONT*[ nNewSize ];

    memset( ppNew, 0, sizeof( NLFCONT* ) * nNewSize );
    memcpy( ppNew, ppP_Nlf, sizeof( NLFCONT* ) * nP_Nlf );

    delete[] ppP_Nlf;
    ppP_Nlf = ppNew;
    nP_Nlf = nNewSize;
}


void TokenPool::GrowMatrix( void )
{
    sal_uInt16      nNewSize = nP_Matrix * 2;

    ScMatrix**  ppNew = new ScMatrix*[ nNewSize ];

    memset( ppNew, 0, sizeof( ScMatrix* ) * nNewSize );
    memcpy( ppNew, ppP_Matrix, sizeof( ScMatrix* ) * nP_Matrix );

    delete[] ppP_Matrix;
    ppP_Matrix = ppNew;
    nP_Matrix = nNewSize;
}

void TokenPool::GetElement( const sal_uInt16 nId )
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
                OSL_FAIL( "-TokenPool::GetElement(): hier hast Du nichts zu suchen!" );
                break;
#endif
            case T_Str:
                pScToken->AddString( ppP_Str[ pElement[ nId ] ]->GetBuffer() );
                break;
            case T_D:
                pScToken->AddDouble( pP_Dbl[ pElement[ nId ] ] );
                break;
            case T_Err:
                break;
            case T_RefC:
                pScToken->AddSingleReference( *ppP_RefTr[ pElement[ (sal_uInt16) nId ] ] );
                break;
            case T_RefA:
                {
                ScComplexRefData    aScComplexRefData;
                aScComplexRefData.Ref1 = *ppP_RefTr[ pElement[ nId ] ];
                aScComplexRefData.Ref2 = *ppP_RefTr[ pElement[ nId ] + 1 ];
                pScToken->AddDoubleReference( aScComplexRefData );
                }
                break;
            case T_RN:
            {
                sal_uInt16 n = pElement[nId];
                if (n < maRangeNames.size())
                {
                    const RangeName& r = maRangeNames[n];
                    pScToken->AddRangeName(r.mnIndex, r.mbGlobal);
                }
            }
            break;
            case T_Ext:
                {
                sal_uInt16          n = pElement[ nId ];
                EXTCONT*        p = ( n < nP_Ext )? ppP_Ext[ n ] : NULL;

                if( p )
                {
                    if( p->eId == ocEuroConvert )
                        pScToken->AddOpCode( p->eId );
                    else
                        pScToken->AddExternal( p->aText, p->eId );
                }
                }
                break;
            case T_Nlf:
                {
                sal_uInt16          n = pElement[ nId ];
                NLFCONT*        p = ( n < nP_Nlf )? ppP_Nlf[ n ] : NULL;

                if( p )
                        pScToken->AddColRowName( p->aRef );
                }
                break;
            case T_Matrix:
                {
                sal_uInt16          n = pElement[ nId ];
                ScMatrix*       p = ( n < nP_Matrix )? ppP_Matrix[ n ] : NULL;

                if( p )
                        pScToken->AddMatrix( p );
                }
                break;
            case T_ExtName:
            {
                sal_uInt16 n = pElement[nId];
                if (n < maExtNames.size())
                {
                    const ExtName& r = maExtNames[n];
                    pScToken->AddExternalName(r.mnFileId, r.maName);
                }
            }
            break;
            case T_ExtRefC:
            {
                sal_uInt16 n = pElement[nId];
                if (n < maExtCellRefs.size())
                {
                    const ExtCellRef& r = maExtCellRefs[n];
                    pScToken->AddExternalSingleReference(r.mnFileId, r.maTabName, r.maRef);
                }
            }
            break;
            case T_ExtRefA:
            {
                sal_uInt16 n = pElement[nId];
                if (n < maExtAreaRefs.size())
                {
                    const ExtAreaRef& r = maExtAreaRefs[n];
                    pScToken->AddExternalDoubleReference(r.mnFileId, r.maTabName, r.maRef);
                }
            }
            break;
            default:
                OSL_FAIL("-TokenPool::GetElement(): Zustand undefiniert!?");
        }
    }
}


void TokenPool::GetElementRek( const sal_uInt16 nId )
{
#ifdef DBG_UTIL
    nRek++;
    DBG_ASSERT( nRek <= nP_Id, "*TokenPool::GetElement(): Rekursion loopt!?" );
#endif

    DBG_ASSERT( nId < nElementAkt, "*TokenPool::GetElementRek(): Id zu gross!?" );

    DBG_ASSERT( pType[ nId ] == T_Id, "-TokenPool::GetElementRek(): nId nicht Id-Folge!" );


    sal_uInt16      nAnz = pSize[ nId ];
    sal_uInt16*     pAkt = &pP_Id[ pElement[ nId ] ];
    for( ; nAnz > 0 ; nAnz--, pAkt++ )
    {
        if( *pAkt < nScTokenOff )
        {// Rekursion oder nicht?
            switch( pType[ *pAkt ] )
            {
                case T_Id:
                    GetElementRek( *pAkt );
                    break;
                case T_Str:
                    pScToken->AddString( ppP_Str[ pElement[ *pAkt ] ]->GetBuffer() );
                    break;
                case T_D:
                    pScToken->AddDouble( pP_Dbl[ pElement[ *pAkt ] ] );
                    break;
                case T_Err:
                    break;
                case T_RefC:
                    pScToken->AddSingleReference( *ppP_RefTr[ pElement[ *pAkt ] ] );
                    break;
                case T_RefA:
                    {
                    ScComplexRefData    aScComplexRefData;
                    aScComplexRefData.Ref1 = *ppP_RefTr[ pElement[ *pAkt ] ];
                    aScComplexRefData.Ref2 = *ppP_RefTr[ pElement[ *pAkt ] + 1 ];
                    pScToken->AddDoubleReference( aScComplexRefData );
                    }
                    break;
                case T_RN:
                {
                    sal_uInt16 n = pElement[*pAkt];
                    if (n < maRangeNames.size())
                    {
                        const RangeName& r = maRangeNames[n];
                        pScToken->AddRangeName(r.mnIndex, r.mbGlobal);
                    }
                }
                break;
                case T_Ext:
                    {
                    sal_uInt16      n = pElement[ *pAkt ];
                    EXTCONT*    p = ( n < nP_Ext )? ppP_Ext[ n ] : NULL;

                    if( p )
                        pScToken->AddExternal( p->aText, p->eId );
                    }
                    break;
                case T_Nlf:
                    {
                    sal_uInt16      n = pElement[ *pAkt ];
                    NLFCONT*    p = ( n < nP_Nlf )? ppP_Nlf[ n ] : NULL;

                    if( p )
                        pScToken->AddColRowName( p->aRef );
                    }
                    break;
                case T_Matrix:
                    {
                    sal_uInt16          n = pElement[ *pAkt ];
                    ScMatrix*       p = ( n < nP_Matrix )? ppP_Matrix[ n ] : NULL;

                    if( p )
                            pScToken->AddMatrix( p );
                    }
                    break;
                case T_ExtName:
                {
                    sal_uInt16 n = pElement[*pAkt];
                    if (n < maExtNames.size())
                    {
                        const ExtName& r = maExtNames[n];
                        pScToken->AddExternalName(r.mnFileId, r.maName);
                    }
                }
                break;
                case T_ExtRefC:
                {
                    sal_uInt16 n = pElement[*pAkt];
                    if (n < maExtCellRefs.size())
                    {
                        const ExtCellRef& r = maExtCellRefs[n];
                        pScToken->AddExternalSingleReference(r.mnFileId, r.maTabName, r.maRef);
                    }
                }
                break;
                case T_ExtRefA:
                {
                    sal_uInt16 n = pElement[*pAkt];
                    if (n < maExtAreaRefs.size())
                    {
                        const ExtAreaRef& r = maExtAreaRefs[n];
                        pScToken->AddExternalDoubleReference(r.mnFileId, r.maTabName, r.maRef);
                    }
                }
                break;
                default:
                    OSL_FAIL("-TokenPool::GetElementRek(): Zustand undefiniert!?");
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


const TokenId TokenPool::Store( const double& rDouble )
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

    return ( const TokenId ) nElementAkt; // Ausgabe von altem Wert + 1!
}


const TokenId TokenPool::Store( const sal_uInt16 nIndex )
{
    return StoreName(nIndex, true);
}


const TokenId TokenPool::Store( const String& rString )
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

    pSize[ nElementAkt ] = ( sal_uInt16 ) ppP_Str[ nP_StrAkt ]->Len();

    nElementAkt++;
    nP_StrAkt++;

    return ( const TokenId ) nElementAkt; // Ausgabe von altem Wert + 1!
}


const TokenId TokenPool::Store( const ScSingleRefData& rTr )
{
    if( nElementAkt >= nElement )
        GrowElement();

    if( nP_RefTrAkt >= nP_RefTr )
        GrowTripel();

    pElement[ nElementAkt ] = nP_RefTrAkt;
    pType[ nElementAkt ] = T_RefC;          // Typinfo Cell-Reff eintragen

    if( !ppP_RefTr[ nP_RefTrAkt ] )
        ppP_RefTr[ nP_RefTrAkt ] = new ScSingleRefData( rTr );
    else
        *ppP_RefTr[ nP_RefTrAkt ] = rTr;

    nElementAkt++;
    nP_RefTrAkt++;

    return ( const TokenId ) nElementAkt; // Ausgabe von altem Wert + 1!
}


const TokenId TokenPool::Store( const ScComplexRefData& rTr )
{
    if( nElementAkt >= nElement )
        GrowElement();

    if( nP_RefTrAkt + 1 >= nP_RefTr )
        GrowTripel();

    pElement[ nElementAkt ] = nP_RefTrAkt;
    pType[ nElementAkt ] = T_RefA;          // Typinfo Area-Reff eintragen

    if( !ppP_RefTr[ nP_RefTrAkt ] )
        ppP_RefTr[ nP_RefTrAkt ] = new ScSingleRefData( rTr.Ref1 );
    else
        *ppP_RefTr[ nP_RefTrAkt ] = rTr.Ref1;
    nP_RefTrAkt++;

    if( !ppP_RefTr[ nP_RefTrAkt ] )
        ppP_RefTr[ nP_RefTrAkt ] = new ScSingleRefData( rTr.Ref2 );
    else
        *ppP_RefTr[ nP_RefTrAkt ] = rTr.Ref2;
    nP_RefTrAkt++;

    nElementAkt++;

    return ( const TokenId ) nElementAkt; // Ausgabe von altem Wert + 1!
}


const TokenId TokenPool::Store( const DefTokenId e, const String& r )
{
    if( nElementAkt >= nElement )
        GrowElement();

    if( nP_ExtAkt >= nP_Ext )
        GrowExt();

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

    return ( const TokenId ) nElementAkt; // Ausgabe von altem Wert + 1!
}


const TokenId TokenPool::StoreNlf( const ScSingleRefData& rTr )
{
    if( nElementAkt >= nElement )
        GrowElement();

    if( nP_NlfAkt >= nP_Nlf )
        GrowNlf();

    pElement[ nElementAkt ] = nP_NlfAkt;
    pType[ nElementAkt ] = T_Nlf;

    if( ppP_Nlf[ nP_NlfAkt ] )
    {
        ppP_Nlf[ nP_NlfAkt ]->aRef = rTr;
    }
    else
        ppP_Nlf[ nP_NlfAkt ] = new NLFCONT( rTr );

    nElementAkt++;
    nP_NlfAkt++;

    return ( const TokenId ) nElementAkt;
}

const TokenId TokenPool::StoreMatrix()
{
    ScMatrix* pM;

    if( nElementAkt >= nElement )
        GrowElement();

    if( nP_MatrixAkt >= nP_Matrix )
        GrowMatrix();

    pElement[ nElementAkt ] = nP_MatrixAkt;
    pType[ nElementAkt ] = T_Matrix;

    pM = new ScMatrix( 0, 0 );
    pM->IncRef( );
    ppP_Matrix[ nP_MatrixAkt ] = pM;

    nElementAkt++;
    nP_MatrixAkt++;

    return ( const TokenId ) nElementAkt;
}

const TokenId TokenPool::StoreName( sal_uInt16 nIndex, bool bGlobal )
{
    if ( nElementAkt >= nElement )
        GrowElement();

    pElement[nElementAkt] = static_cast<sal_uInt16>(maRangeNames.size());
    pType[nElementAkt] = T_RN;

    maRangeNames.push_back(RangeName());
    RangeName& r = maRangeNames.back();
    r.mnIndex = nIndex;
    r.mbGlobal = bGlobal;

    ++nElementAkt;

    return static_cast<const TokenId>(nElementAkt);
}

const TokenId TokenPool::StoreExtName( sal_uInt16 nFileId, const String& rName )
{
    if ( nElementAkt >= nElement )
        GrowElement();

    pElement[nElementAkt] = static_cast<sal_uInt16>(maExtNames.size());
    pType[nElementAkt] = T_ExtName;

    maExtNames.push_back(ExtName());
    ExtName& r = maExtNames.back();
    r.mnFileId = nFileId;
    r.maName = rName;

    ++nElementAkt;

    return static_cast<const TokenId>(nElementAkt);
}

const TokenId TokenPool::StoreExtRef( sal_uInt16 nFileId, const String& rTabName, const ScSingleRefData& rRef )
{
    if ( nElementAkt >= nElement )
        GrowElement();

    pElement[nElementAkt] = static_cast<sal_uInt16>(maExtCellRefs.size());
    pType[nElementAkt] = T_ExtRefC;

    maExtCellRefs.push_back(ExtCellRef());
    ExtCellRef& r = maExtCellRefs.back();
    r.mnFileId = nFileId;
    r.maTabName = rTabName;
    r.maRef = rRef;

    ++nElementAkt;

    return static_cast<const TokenId>(nElementAkt);
}

const TokenId TokenPool::StoreExtRef( sal_uInt16 nFileId, const String& rTabName, const ScComplexRefData& rRef )
{
    if ( nElementAkt >= nElement )
        GrowElement();

    pElement[nElementAkt] = static_cast<sal_uInt16>(maExtAreaRefs.size());
    pType[nElementAkt] = T_ExtRefA;

    maExtAreaRefs.push_back(ExtAreaRef());
    ExtAreaRef& r = maExtAreaRefs.back();
    r.mnFileId = nFileId;
    r.maTabName = rTabName;
    r.maRef = rRef;

    ++nElementAkt;

    return static_cast<const TokenId>(nElementAkt);
}

void TokenPool::Reset( void )
{
    nP_IdAkt = nP_IdLast = nElementAkt = nP_StrAkt = nP_DblAkt = nP_ErrAkt = nP_RefTrAkt = nP_ExtAkt = nP_NlfAkt = nP_MatrixAkt = 0;
    maRangeNames.clear();
    maExtNames.clear();
    maExtCellRefs.clear();
    maExtAreaRefs.clear();
}


sal_Bool TokenPool::IsSingleOp( const TokenId& rId, const DefTokenId eId ) const
{
    sal_uInt16 nId = (sal_uInt16) rId;
    if( nId && nId <= nElementAkt )
    {// existent?
        nId--;
        if( T_Id == pType[ nId ] )
        {// Tokenfolge?
            if( pSize[ nId ] == 1 )
            {// GENAU 1 Token
                sal_uInt16  nSecId = pP_Id[ pElement[ nId ] ];
                if( nSecId >= nScTokenOff )
                {// Default-Token?
                    return ( DefTokenId ) ( nSecId - nScTokenOff ) == eId;  // Gesuchter?
                }
            }
        }
    }

    return false;
}

const String* TokenPool::GetExternal( const TokenId& rId ) const
{
    const String*   p = NULL;
    sal_uInt16 n = (sal_uInt16) rId;
    if( n && n <= nElementAkt )
    {
        n--;
        if( (pType[ n ] == T_Ext) && ppP_Ext[ pElement[ n ] ] )
            p = &ppP_Ext[ pElement[ n ] ]->aText;
    }

    return p;
}

ScMatrix* TokenPool::GetMatrix( unsigned int n ) const
{
    if( n < nP_MatrixAkt )
        return ppP_Matrix[ n ];
    else
        printf ("GETMATRIX %d >= %d\n", n, nP_MatrixAkt);
    return NULL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
