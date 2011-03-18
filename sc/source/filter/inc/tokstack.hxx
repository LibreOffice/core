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

#ifndef SC_TOKSTACK_HXX
#define SC_TOKSTACK_HXX

#include <string.h>
#include <tools/debug.hxx>
#include "compiler.hxx"
#include "tokenarray.hxx"

#include <vector>

typedef OpCode DefTokenId;
// in PRODUCT version: ambiguity between OpCode (being sal_uInt16) and UINT16
// Unfortunately a typedef is just a dumb alias and not a real type ...
//typedef sal_uInt16 TokenId;
struct TokenId
{
        sal_uInt16          nId;

                        TokenId() : nId( 0 ) {}
                        TokenId( sal_uInt16 n ) : nId( n ) {}
                        TokenId( const TokenId& r ) : nId( r.nId ) {}
    inline  TokenId&    operator =( const TokenId& r ) { nId = r.nId; return *this; }
    inline  TokenId&    operator =( sal_uInt16 n ) { nId = n; return *this; }
    inline              operator sal_uInt16&() { return nId; }
    inline              operator const sal_uInt16&() const { return nId; }
    inline  sal_Bool        operator <( sal_uInt16 n ) const { return nId < n; }
    inline  sal_Bool        operator >( sal_uInt16 n ) const { return nId > n; }
    inline  sal_Bool        operator <=( sal_uInt16 n ) const { return nId <= n; }
    inline  sal_Bool        operator >=( sal_uInt16 n ) const { return nId >= n; }
    inline  sal_Bool        operator ==( sal_uInt16 n ) const { return nId == n; }
    inline  sal_Bool        operator !=( sal_uInt16 n ) const { return nId != n; }
};


//------------------------------------------------------------------------
struct ScComplexRefData;
class TokenStack;
class ScToken;


enum E_TYPE
{
    T_Id,       // Id-Folge
    T_Str,      // String
    T_D,        // Double
    T_Err,      // Error code
    T_RefC,     // Cell Reference
    T_RefA,     // Area Reference
    T_RN,       // Range Name
    T_Ext,      // irgendwas Unbekanntes mit Funktionsnamen
    T_Nlf,      // token for natural language formula
    T_Matrix,   // token for inline arrays
    T_ExtName,  // token for external names
    T_ExtRefC,
    T_ExtRefA,
    T_Error     // fuer Abfrage im Fehlerfall
};




class TokenPool
{
    // !ACHTUNG!: externe Id-Basis ist 1, interne 0!
    // Ausgabe Id = 0 -> Fehlerfall
    private:
        String**                    ppP_Str;    // Pool fuer Strings
        sal_uInt16                      nP_Str;     // ...mit Groesse
        sal_uInt16                      nP_StrAkt;  // ...und Schreibmarke

        double*                     pP_Dbl;     // Pool fuer Doubles
        sal_uInt16                      nP_Dbl;
        sal_uInt16                      nP_DblAkt;

        sal_uInt16*                     pP_Err;     // Pool for error codes
        sal_uInt16                      nP_Err;
        sal_uInt16                      nP_ErrAkt;

        ScSingleRefData**               ppP_RefTr;  // Pool fuer Referenzen
        sal_uInt16                      nP_RefTr;
        sal_uInt16                      nP_RefTrAkt;

        sal_uInt16*                     pP_Id;      // Pool fuer Id-Folgen
        sal_uInt16                      nP_Id;
        sal_uInt16                      nP_IdAkt;
        sal_uInt16                      nP_IdLast;  // letzter Folgen-Beginn

        struct  EXTCONT
        {
            DefTokenId              eId;
            String                  aText;
                                    EXTCONT( const DefTokenId e, const String& r ) :
                                        eId( e ), aText( r ){}
        };
        EXTCONT**                   ppP_Ext;
        sal_uInt16                      nP_Ext;
        sal_uInt16                      nP_ExtAkt;

        struct  NLFCONT
        {
            ScSingleRefData         aRef;
                                    NLFCONT( const ScSingleRefData& r ) : aRef( r ) {}
        };
        NLFCONT**                   ppP_Nlf;
        sal_uInt16                      nP_Nlf;
        sal_uInt16                      nP_NlfAkt;

        ScMatrix**                  ppP_Matrix;     // Pool fuer Matricies
        sal_uInt16                      nP_Matrix;
        sal_uInt16                      nP_MatrixAkt;

        /** for storage of named ranges */
        struct RangeName
        {
            sal_uInt16 mnIndex;
            bool mbGlobal;
        };
        ::std::vector<RangeName> maRangeNames;

        /** for storage of external names */
        struct ExtName
        {
            sal_uInt16  mnFileId;
            String      maName;
        };
        ::std::vector<ExtName>      maExtNames;

        /** for storage of external cell references */
        struct ExtCellRef
        {
            sal_uInt16      mnFileId;
            String          maTabName;
            ScSingleRefData   maRef;
        };
        ::std::vector<ExtCellRef>   maExtCellRefs;

        /** for storage of external area references */
        struct ExtAreaRef
        {
            sal_uInt16      mnFileId;
            String          maTabName;
            ScComplexRefData    maRef;
        };
        ::std::vector<ExtAreaRef>   maExtAreaRefs;

        sal_uInt16*                     pElement;   // Array mit Indizes fuer Elemente
        E_TYPE*                     pType;      // ...mit Typ-Info
        sal_uInt16*                     pSize;      // ...mit Laengenangabe (Anz. sal_uInt16)
        sal_uInt16                      nElement;
        sal_uInt16                      nElementAkt;

        static const sal_uInt16         nScTokenOff;// Offset fuer SC-Token
#ifdef DBG_UTIL
        sal_uInt16                      nRek;       // Rekursionszaehler
#endif
        ScTokenArray*               pScToken;   // Tokenbastler

        void                        GrowString( void );
        void                        GrowDouble( void );
        void                        GrowTripel( void );
        void                        GrowId( void );
        void                        GrowElement( void );
        void                        GrowExt( void );
        void                        GrowNlf( void );
        void                        GrowMatrix( void );
        void                        GetElement( const sal_uInt16 nId );
        void                        GetElementRek( const sal_uInt16 nId );
    public:
                                    TokenPool( void );
                                    ~TokenPool();
        inline TokenPool&           operator <<( const TokenId nId );
        inline TokenPool&           operator <<( const DefTokenId eId );
        inline TokenPool&           operator <<( TokenStack& rStack );
        void                        operator >>( TokenId& rId );
        inline void                 operator >>( TokenStack& rStack );
        inline const TokenId        Store( void );
        const TokenId               Store( const double& rDouble );

                                    // nur fuer Range-Names
        const TokenId               Store( const sal_uInt16 nIndex );
        inline const TokenId        Store( const sal_Int16 nWert );
        const TokenId               Store( const String& rString );
        const TokenId               Store( const ScSingleRefData& rTr );
        const TokenId               Store( const ScComplexRefData& rTr );

        const TokenId               Store( const DefTokenId eId, const String& rName );
                                        // 4 externals (e.g. AddIns, Makros...)
        const TokenId               StoreNlf( const ScSingleRefData& rTr );
        const TokenId               StoreMatrix();
        const TokenId               StoreName( sal_uInt16 nIndex, bool bGlobal );
        const TokenId               StoreExtName( sal_uInt16 nFileId, const String& rName );
        const TokenId               StoreExtRef( sal_uInt16 nFileId, const String& rTabName, const ScSingleRefData& rRef );
        const TokenId               StoreExtRef( sal_uInt16 nFileId, const String& rTabName, const ScComplexRefData& rRef );

        inline const TokenId        LastId( void ) const;
        inline const ScTokenArray*  operator []( const TokenId nId );
        void                        Reset( void );
        inline E_TYPE               GetType( const TokenId& nId ) const;
        sal_Bool                        IsSingleOp( const TokenId& nId, const DefTokenId eId ) const;
        const String*               GetExternal( const TokenId& nId ) const;
        ScMatrix*                   GetMatrix( unsigned int n ) const;
};




class TokenStack
    // Stack fuer Token-Ids: Id 0 sollte reserviert bleiben als
    //  fehlerhafte Id, da z.B. Get() im Fehlerfall 0 liefert
{
    private:
        TokenId*                    pStack;     // Stack als Array
        sal_uInt16                      nPos;       // Schreibmarke
        sal_uInt16                      nSize;      // Erster Index ausserhalb des Stacks
    public:
                                    TokenStack( sal_uInt16 nNewSize = 1024 );
                                    ~TokenStack();
        inline TokenStack&          operator <<( const TokenId nNewId );
        inline void                 operator >>( TokenId &rId );

        inline void                 Reset( void );

        inline bool                 HasMoreTokens() const { return nPos > 0; }
        inline const TokenId        Get( void );
};




inline const TokenId TokenStack::Get( void )
{
    DBG_ASSERT( nPos > 0,
        "*TokenStack::Get(): Leer ist leer, ist leer, ist leer, ist..." );

    TokenId nRet;

    if( nPos == 0 )
        nRet = 0;
    else
    {
        nPos--;
        nRet = pStack[ nPos ];
    }

    return nRet;
}


inline TokenStack &TokenStack::operator <<( const TokenId nNewId )
{// Element auf Stack
    DBG_ASSERT( nPos < nSize, "*TokenStack::<<(): Stackueberlauf" );
    if( nPos < nSize )
    {
        pStack[ nPos ] = nNewId;
        nPos++;
    }

    return *this;
}


inline void TokenStack::operator >>( TokenId& rId )
{// Element von Stack
    DBG_ASSERT( nPos > 0,
        "*TokenStack::>>(): Leer ist leer, ist leer, ist leer, ..." );
    if( nPos > 0 )
    {
        nPos--;
        rId = pStack[ nPos ];
    }
}


inline void TokenStack::Reset( void )
{
    nPos = 0;
}




inline TokenPool& TokenPool::operator <<( const TokenId nId )
{
    // POST: nId's werden hintereinander im Pool unter einer neuen Id
    //       abgelegt. Vorgang wird mit >> oder Store() abgeschlossen
    // nId -> ( sal_uInt16 ) nId - 1;
    DBG_ASSERT( ( sal_uInt16 ) nId < nScTokenOff,
        "-TokenPool::operator <<: TokenId im DefToken-Bereich!" );

    if( nP_IdAkt >= nP_Id )
        GrowId();

    pP_Id[ nP_IdAkt ] = ( ( sal_uInt16 ) nId ) - 1;
    nP_IdAkt++;

    return *this;
}


inline TokenPool& TokenPool::operator <<( const DefTokenId eId )
{
    DBG_ASSERT( ( sal_uInt32 ) eId + nScTokenOff < 0xFFFF,
        "-TokenPool::operator<<: enmum zu gross!" );

    if( nP_IdAkt >= nP_Id )
        GrowId();

    pP_Id[ nP_IdAkt ] = ( ( sal_uInt16 ) eId ) + nScTokenOff;
    nP_IdAkt++;

    return *this;
}


inline TokenPool& TokenPool::operator <<( TokenStack& rStack )
{
    if( nP_IdAkt >= nP_Id )
        GrowId();

    pP_Id[ nP_IdAkt ] = ( ( sal_uInt16 ) rStack.Get() ) - 1;
    nP_IdAkt++;

    return *this;
}


inline void TokenPool::operator >>( TokenStack& rStack )
{
    TokenId nId;
    *this >> nId;
    rStack << nId;
}


inline const TokenId TokenPool::Store( void )
{
    TokenId nId;
    *this >> nId;
    return nId;
}


inline const TokenId TokenPool::Store( const sal_Int16 nWert )
{
    return Store( ( double ) nWert );
}


inline const TokenId TokenPool::LastId( void ) const
{
    return ( TokenId ) nElementAkt; // stimmt, da Ausgabe mit Offset 1!
}


const inline ScTokenArray* TokenPool::operator []( const TokenId nId )
{
    pScToken->Clear();

    if( nId )
    {//...nur wenn nId > 0!
#ifdef DBG_UTIL
        nRek = 0;
#endif
        GetElement( ( sal_uInt16 ) nId - 1 );
    }

    return pScToken;
}


inline E_TYPE TokenPool::GetType( const TokenId& rId ) const
{
    E_TYPE nRet;

    sal_uInt16 nId = (sal_uInt16) rId - 1;

    if( nId < nElementAkt )
        nRet = pType[ nId ] ;
    else
        nRet = T_Error;

    return nRet;
}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
