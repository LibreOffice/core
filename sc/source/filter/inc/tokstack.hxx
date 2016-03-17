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
    inline  TokenId&    operator =( const TokenId& r ) { nId = r.nId; return *this; }
    inline  TokenId&    operator =( sal_uInt16 n ) { nId = n; return *this; }
    inline              operator const sal_uInt16&() const { return nId; }
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
    T_ExtRefA,
    T_Error     // for check in case of error
};

class TokenPool
{
    // !ATTENTION!: external Id-Basis is 1, internal 0!
    // return Id = 0 -> Error
private:
    svl::SharedStringPool& mrStringPool;

        OUString**                      ppP_Str;    // Pool for Strings
        sal_uInt16                      nP_Str;     // ...with size
        sal_uInt16                      nP_StrAkt;  // ...and Write-Mark

        double*                     pP_Dbl;     // Pool for Doubles
        sal_uInt16                      nP_Dbl;
        sal_uInt16                      nP_DblAkt;

        sal_uInt16*                     pP_Err;     // Pool for error codes
        sal_uInt16                      nP_Err;
        sal_uInt16                      nP_ErrAkt;

        ScSingleRefData**               ppP_RefTr;  // Pool for References
        sal_uInt16                      nP_RefTr;
        sal_uInt16                      nP_RefTrAkt;

        sal_uInt16*                     pP_Id;      // Pool for Id-sets
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

        ScMatrix**                  ppP_Matrix;     // Pool for Matrices
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

        sal_uInt16*                     pElement;   // Array with Indices for elements
        E_TYPE*                         pType;      // ...with Type-Info
        sal_uInt16*                     pSize;      // ...with size (Anz. sal_uInt16)
        sal_uInt16                      nElement;
        sal_uInt16                      nElementAkt;

        static const sal_uInt16         nScTokenOff;// Offset for SC-Token
#ifdef DBG_UTIL
        sal_uInt16                      m_nRek; // recursion counter
#endif
        ScTokenArray*               pScToken;   // Token array

        bool                        GrowString();
        bool                        GrowDouble();
/* TODO: in case we had FormulaTokenArray::AddError() */
#if 0
        bool                        GrowError();
#endif
        bool                        GrowTripel( sal_uInt16 nByMin = 1 );
        bool                        GrowId();
        bool                        GrowElement();
        bool                        GrowExt();
        bool                        GrowNlf();
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
        TokenId*                    pStack;     // Stack as Array
        sal_uInt16                      nPos;       // Write-mark
        sal_uInt16                      nSize;      // first Index outside of stack
    public:
                                    TokenStack( sal_uInt16 nNewSize = 1024 );
                                    ~TokenStack();
        inline TokenStack&          operator <<( const TokenId& rNewId );
        inline void                 operator >>( TokenId &rId );

        inline void                 Reset();

        inline bool                 HasMoreTokens() const { return nPos > 0; }
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
