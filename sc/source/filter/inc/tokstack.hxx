/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_TOKSTACK_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_TOKSTACK_HXX

#include <string.h>
#include "compiler.hxx"
#include "tokenarray.hxx"
#include <osl/diagnose.h>

#include <memory>
#include <vector>

namespace svl {

class SharedStringPool;

}

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
    TokenId&    operator =( const TokenId& r ) { nId = r.nId; return *this; }
    TokenId&    operator =( sal_uInt16 n ) { nId = n; return *this; }
    operator const sal_uInt16&() const { return nId; }
};

struct ScComplexRefData;
class TokenStack;

enum E_TYPE
{
    T_Id,       // Id-Folge
    T_Str,      // String
    T_D,        // Double
    T_Err,      // Error code
    T_RefC,     // Cell Reference
    T_RefA,     // Area Reference
    T_RN,       // Range Name
    T_Ext,      // something unknown with function name
    T_Nlf,      // token for natural language formula
    T_Matrix,   // token for inline arrays
    T_ExtName,  // token for external names
    T_ExtRefC,
    T_ExtRefA
};

template<typename T, int InitialCapacity>
struct TokenPoolPool
{
    std::unique_ptr<T[]> ppP_Str;
    sal_uInt16                            m_capacity;
    sal_uInt16                            m_writemark;

    TokenPoolPool() :
        ppP_Str( new T[InitialCapacity] ),
        m_capacity(InitialCapacity),
        m_writemark(0)
    {
    }
    bool Grow(sal_uInt16 nByMin = 1)
    {
        sal_uInt16 nP_StrNew = lcl_canGrow(m_capacity, nByMin);
        if (!nP_StrNew)
            return false;

        T* ppP_StrNew = new T[ nP_StrNew ];

        for( sal_uInt16 i = 0 ; i < m_capacity ; i++ )
            ppP_StrNew[ i ] = std::move(ppP_Str[ i ]);

        m_capacity = nP_StrNew;

        ppP_Str.reset( ppP_StrNew );
        return true;
    }
    /** Returns the new number of elements, or 0 if overflow. */
    static sal_uInt16 lcl_canGrow( sal_uInt16 nOld, sal_uInt16 nByMin = 1 )
    {
        if (!nOld)
            return nByMin ? nByMin : 1;
        if (nOld == SAL_MAX_UINT16)
            return 0;
        sal_uInt32 nNew = ::std::max( static_cast<sal_uInt32>(nOld) * 2,
                static_cast<sal_uInt32>(nOld) + nByMin);
        if (nNew > SAL_MAX_UINT16)
            nNew = SAL_MAX_UINT16;
        if (nNew - nByMin < nOld)
            nNew = 0;
        return static_cast<sal_uInt16>(nNew);
    }
    T* getIfInRange(sal_uInt16 n) const
    {
         return ( n < m_capacity ) ? &ppP_Str[ n ] : nullptr;
    }
    T const & operator[](sal_uInt16 n) const
    {
         return ppP_Str[ n ];
    }
    T & operator[](sal_uInt16 n)
    {
         return ppP_Str[ n ];
    }
};

class TokenPool
{
    // !ATTENTION!: external Id-Basis is 1, internal 0!
    // return Id = 0 -> Error
private:
    svl::SharedStringPool& mrStringPool;

        TokenPoolPool<std::unique_ptr<OUString>, 4>
                                        ppP_Str;    // Pool for Strings

        TokenPoolPool<double, 8>        pP_Dbl;     // Pool for Doubles

        TokenPoolPool<sal_uInt16, 8>
                                        pP_Err;     // Pool for error codes

        TokenPoolPool<std::unique_ptr<ScSingleRefData>, 32>
                                        ppP_RefTr;  // Pool for References
        std::unique_ptr<sal_uInt16[]>   pP_Id;      // Pool for Id-sets
        sal_uInt16                      nP_Id;
        sal_uInt16                      nP_IdAkt;
        sal_uInt16                      nP_IdLast;  // last set-start

        struct  EXTCONT
        {
            DefTokenId              eId;
            OUString                aText;
                                    EXTCONT( const DefTokenId e, const OUString& r ) :
                                        eId( e ), aText( r ){}
        };
        TokenPoolPool<std::unique_ptr<EXTCONT>, 32>
                                        ppP_Ext;

        TokenPoolPool<std::unique_ptr<ScSingleRefData>, 16>
                                        ppP_Nlf;

        std::unique_ptr<ScMatrix*[]>    ppP_Matrix;     // Pool for Matrices
        sal_uInt16                      nP_Matrix;
        sal_uInt16                      nP_MatrixAkt;

        /** for storage of named ranges */
        struct RangeName
        {
            sal_uInt16 mnIndex;
            sal_Int16  mnSheet;
        };
        ::std::vector<RangeName> maRangeNames;

        /** for storage of external names */
        struct ExtName
        {
            sal_uInt16  mnFileId;
            OUString    maName;
        };
        ::std::vector<ExtName>      maExtNames;

        /** for storage of external cell references */
        struct ExtCellRef
        {
            sal_uInt16      mnFileId;
            OUString        maTabName;
            ScSingleRefData   maRef;
        };
        ::std::vector<ExtCellRef>   maExtCellRefs;

        /** for storage of external area references */
        struct ExtAreaRef
        {
            sal_uInt16      mnFileId;
            OUString        maTabName;
            ScComplexRefData    maRef;
        };
        ::std::vector<ExtAreaRef>   maExtAreaRefs;

        std::unique_ptr<sal_uInt16[]>   pElement;   // Array with Indices for elements
        std::unique_ptr<E_TYPE[]>       pType;      // ...with Type-Info
        std::unique_ptr<sal_uInt16[]>   pSize;      // ...with size (Anz. sal_uInt16)
        sal_uInt16                      nElement;
        sal_uInt16                      nElementAkt;

        static const sal_uInt16         nScTokenOff;// Offset for SC-Token
#ifdef DBG_UTIL
        sal_uInt16                      m_nRek; // recursion counter
#endif
        ScTokenArray*               pScToken;   // Token array

        bool                        GrowTripel( sal_uInt16 nByMin = 1 );
        bool                        GrowId();
        bool                        GrowElement();
        bool                        GrowMatrix();
        bool                        GetElement( const sal_uInt16 nId );
        bool                        GetElementRek( const sal_uInt16 nId );
        void                        ClearMatrix();
public:
    TokenPool( svl::SharedStringPool& rSPool );
                                    ~TokenPool();
        inline TokenPool&           operator <<( const TokenId& rId );
        inline TokenPool&           operator <<( const DefTokenId eId );
        inline TokenPool&           operator <<( TokenStack& rStack );
        void                        operator >>( TokenId& rId );
        inline void                 operator >>( TokenStack& rStack );
        inline const TokenId        Store();
        const TokenId               Store( const double& rDouble );

                                    // only for Range-Names
        const TokenId               Store( const sal_uInt16 nIndex );
        ;
        const TokenId               Store( const OUString& rString );
        const TokenId               Store( const ScSingleRefData& rTr );
        const TokenId               Store( const ScComplexRefData& rTr );

        const TokenId               Store( const DefTokenId eId, const OUString& rName );
                                        // 4 externals (e.g. AddIns, Macros...)
        const TokenId               StoreNlf( const ScSingleRefData& rTr );
        const TokenId               StoreMatrix();
        const TokenId               StoreName( sal_uInt16 nIndex, sal_Int16 nSheet );
        const TokenId               StoreExtName( sal_uInt16 nFileId, const OUString& rName );
        const TokenId               StoreExtRef( sal_uInt16 nFileId, const OUString& rTabName, const ScSingleRefData& rRef );
        const TokenId               StoreExtRef( sal_uInt16 nFileId, const OUString& rTabName, const ScComplexRefData& rRef );

        inline const ScTokenArray*  operator []( const TokenId& rId );
        void                        Reset();
        bool                        IsSingleOp( const TokenId& rId, const DefTokenId eId ) const;
        const OUString*             GetExternal( const TokenId& rId ) const;
        ScMatrix*                   GetMatrix( unsigned int n ) const;
};

class TokenStack
    // Stack for Token-Ids: reserve Id=0 for error; e.g. Get() returns 0 on error

{
    private:
        std::unique_ptr<TokenId[]>  pStack;       // Stack as Array
        sal_uInt16                  nPos;         // Write-mark
        static const sal_uInt16     nSize = 1024; // first Index outside of stack
    public:
                                    TokenStack();
                                    ~TokenStack();
        inline TokenStack&          operator <<( const TokenId& rNewId );
        inline void                 operator >>( TokenId &rId );

        inline void                 Reset();

        bool                 HasMoreTokens() const { return nPos > 0; }
        inline const TokenId        Get();
};

inline const TokenId TokenStack::Get()
{
    OSL_ENSURE( nPos > 0,
        "*TokenStack::Get(): is empty, is empty, ..." );

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

inline TokenStack &TokenStack::operator <<( const TokenId& rNewId )
{// Element on Stack
    OSL_ENSURE( nPos < nSize, "*TokenStack::<<(): Stack overflow" );
    if( nPos < nSize )
    {
        pStack[ nPos ] = rNewId;
        nPos++;
    }

    return *this;
}

inline void TokenStack::operator >>( TokenId& rId )
{// Element of Stack
    OSL_ENSURE( nPos > 0,
        "*TokenStack::>>(): is empty, is empty, ..." );
    if( nPos > 0 )
    {
        nPos--;
        rId = pStack[ nPos ];
    }
}

inline void TokenStack::Reset()
{
    nPos = 0;
}

inline TokenPool& TokenPool::operator <<( const TokenId& rId )
{
    // POST: rId's are stored consecutively in Pool under a new Id;
    //       finalize with >> or Store()
    // rId -> ( sal_uInt16 ) rId - 1;
    OSL_ENSURE( ( sal_uInt16 ) rId < nScTokenOff,
        "-TokenPool::operator <<: TokenId in DefToken-Range!" );

    if( nP_IdAkt >= nP_Id )
        if (!GrowId())
            return *this;

    pP_Id[ nP_IdAkt ] = ( ( sal_uInt16 ) rId ) - 1;
    nP_IdAkt++;

    return *this;
}

inline TokenPool& TokenPool::operator <<( const DefTokenId eId )
{
    OSL_ENSURE( ( sal_uInt32 ) eId + nScTokenOff < 0xFFFF,
        "-TokenPool::operator<<: enum too large!" );

    if( nP_IdAkt >= nP_Id )
        if (!GrowId())
            return *this;

    pP_Id[ nP_IdAkt ] = ( ( sal_uInt16 ) eId ) + nScTokenOff;
    nP_IdAkt++;

    return *this;
}

inline TokenPool& TokenPool::operator <<( TokenStack& rStack )
{
    if( nP_IdAkt >= nP_Id )
        if (!GrowId())
            return *this;

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

inline const TokenId TokenPool::Store()
{
    TokenId nId;
    *this >> nId;
    return nId;
}

const inline ScTokenArray* TokenPool::operator []( const TokenId& rId )
{
    pScToken->ClearScTokenArray();

    if( rId )
    {//...only if rId > 0!
#ifdef DBG_UTIL
        m_nRek = 0;
#endif
        GetElement( ( sal_uInt16 ) rId - 1 );
    }

    return pScToken;
}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
