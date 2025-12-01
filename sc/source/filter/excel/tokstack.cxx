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

#include <tokstack.hxx>
#include <scmatrix.hxx>

#include <comphelper/scopeguard.hxx>
#include <svl/sharedstringpool.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>

#include <algorithm>
#include <string.h>

const sal_uInt16    TokenPool::nScTokenOff = 8192;

TokenStack::TokenStack(  )
    : pStack( new TokenId[ nSize ] )
{
    Reset();
}

TokenStack::~TokenStack()
{
}

// !ATTENTION!": to the outside the numbering starts with 1!
// !ATTENTION!": SC-Token are stored with an offset nScTokenOff
//                  -> to distinguish from other tokens

TokenPool::TokenPool( svl::SharedStringPool& rSPool ) :
    mrStringPool(rSPool)
{
    // pool for Id-sequences
    nP_Id = 256;
    pP_Id.reset( new sal_uInt16[ nP_Id ] );

    maElements.reserve(32);
    nP_IdLast = 0;

    nP_Matrix = 16;
    ppP_Matrix.reset( new ScMatrix*[ nP_Matrix ] );
    memset( ppP_Matrix.get(), 0, sizeof( ScMatrix* ) * nP_Matrix );

    Reset();
}

TokenPool::~TokenPool()
{
    ClearMatrix();
}

/** Returns the new number of elements, or 0 if overflow. */
static sal_uInt16 lcl_canGrow( sal_uInt16 nOld )
{
    if (!nOld)
        return 1;
    if (nOld == SAL_MAX_UINT16)
        return 0;
    sal_uInt32 nNew = ::std::max( static_cast<sal_uInt32>(nOld) * 2,
            static_cast<sal_uInt32>(nOld) + 1);
    if (nNew > SAL_MAX_UINT16)
        nNew = SAL_MAX_UINT16;
    if (nNew - 1 < nOld)
        nNew = 0;
    return static_cast<sal_uInt16>(nNew);
}

bool TokenPool::GrowId()
{
    sal_uInt16 nP_IdNew = lcl_canGrow( nP_Id);
    if (!nP_IdNew)
        return false;

    sal_uInt16* pP_IdNew = new (::std::nothrow) sal_uInt16[ nP_IdNew ];
    if (!pP_IdNew)
        return false;

    for( sal_uInt16 nL = 0 ; nL < nP_Id ; nL++ )
        pP_IdNew[ nL ] = pP_Id[ nL ];

    nP_Id = nP_IdNew;

    pP_Id.reset( pP_IdNew );
    return true;
}

bool TokenPool::CanAddElement()
{
    // Last possible ID to be assigned somewhere is maElements.size()+1
    if (maElements.size() + 1 == nScTokenOff - 1)
    {
        SAL_WARN("sc.filter","TokenPool::CheckElementOrGrow - last possible ID " << maElements.size()+1);
        return false;
    }

    return true;
}

bool TokenPool::GrowMatrix()
{
    sal_uInt16 nNewSize = lcl_canGrow( nP_Matrix);
    if (!nNewSize)
        return false;

    ScMatrix**  ppNew = new (::std::nothrow) ScMatrix*[ nNewSize ];
    if (!ppNew)
        return false;

    memset( ppNew, 0, sizeof( ScMatrix* ) * nNewSize );
    for( sal_uInt16 nL = 0 ; nL < nP_Matrix ; nL++ )
        ppNew[ nL ] = ppP_Matrix[ nL ];

    ppP_Matrix.reset( ppNew );
    nP_Matrix = nNewSize;
    return true;
}

bool TokenPool::GetElement( const sal_uInt16 nId, ScTokenArray* pScToken )
{
    if (nId >= maElements.size())
    {
        SAL_WARN("sc.filter","TokenPool::GetElement - Id too large, " << nId << " >= " << maElements.size());
        return false;
    }

    switch ([[maybe_unused]] const auto [n, eType, _] = maElements[nId]; eType)
    {
        case T_Id:
            return GetElementRek(nId, pScToken);
        case T_Str:
            if (auto* p = ppP_Str.getIfInRange(n))
            {
                pScToken->AddString(mrStringPool.intern(**p));
                return true;
            }
            break;
        case T_D:
            if (n < pP_Dbl.m_writemark)
            {
                pScToken->AddDouble(pP_Dbl[n]);
                return true;
            }
            break;
        case T_Err:
            return true;
/* TODO: in case we had FormulaTokenArray::AddError() */
#if 0
            if (n < nP_Err)
            {
                pScToken->AddError(pP_Err[n]);
                return true;
            }
            break;
#endif
        case T_RefC:
            if (auto p = ppP_RefTr.getIfInRange(n))
            {
                pScToken->AddSingleReference(**p);
                return true;
            }
            break;
        case T_RefA:
            if (n + sal_uInt32(1) < ppP_RefTr.m_writemark && ppP_RefTr[n] && ppP_RefTr[n + 1])
            {
                ScComplexRefData aScComplexRefData;
                aScComplexRefData.Ref1 = *ppP_RefTr[n];
                aScComplexRefData.Ref2 = *ppP_RefTr[n + 1];
                pScToken->AddDoubleReference(aScComplexRefData);
                return true;
            }
            break;
        case T_RN:
            if (n < maRangeNames.size())
            {
                const RangeName& r = maRangeNames[n];
                pScToken->AddRangeName(r.mnIndex, r.mnSheet);
                return true;
            }
            break;
        case T_Ext:
            if (auto p = ppP_Ext.getIfInRange(n))
            {
                if ((*p)->eId == ocEuroConvert)
                    pScToken->AddOpCode((*p)->eId);
                else
                    pScToken->AddExternal((*p)->aText, (*p)->eId);
                return true;
            }
            break;
        case T_Nlf:
            if (auto p = ppP_Nlf.getIfInRange(n))
            {
                pScToken->AddColRowName(**p);
                return true;
            }
            break;
        case T_Matrix:
            if (ScMatrix* p = (n < nP_Matrix) ? ppP_Matrix[n] : nullptr)
            {
                pScToken->AddMatrix(p);
                return true;
            }
            break;
        case T_ExtName:
            if (n < maExtNames.size())
            {
                const ExtName& r = maExtNames[n];
                pScToken->AddExternalName(r.mnFileId, mrStringPool.intern(r.maName));
                return true;
            }
            break;
        case T_ExtRefC:
            if (n < maExtCellRefs.size())
            {
                const ExtCellRef& r = maExtCellRefs[n];
                pScToken->AddExternalSingleReference(r.mnFileId, mrStringPool.intern(r.maTabName), r.maRef);
                return true;
            }
            break;
        case T_ExtRefA:
            if (n < maExtAreaRefs.size())
            {
                const ExtAreaRef& r = maExtAreaRefs[n];
                pScToken->AddExternalDoubleReference(r.mnFileId, mrStringPool.intern(r.maTabName), r.maRef);
                return true;
            }
            break;
        default:
            OSL_FAIL("-TokenPool::GetElement(): undefined state!?");
            break;
    }
    return false;
}

bool TokenPool::GetElementRek( const sal_uInt16 nId, ScTokenArray* pScToken )
{
#ifdef DBG_UTIL
    m_nRek++;
    OSL_ENSURE(m_nRek <= nP_Id, "*TokenPool::GetElement(): recursion loops!?");
    comphelper::ScopeGuard decreaseRek([&]() { m_nRek--; });
#endif

    OSL_ENSURE(nId < maElements.size(), "*TokenPool::GetElementRek(): nId >= maElements.size()");

    if (nId >= maElements.size())
    {
        SAL_WARN("sc.filter", "*TokenPool::GetElementRek(): nId >= maElements.size()");
        return false;
    }

    auto [nFirstId, eType, nCnt] = maElements[nId];
    if (eType != T_Id)
    {
        SAL_WARN("sc.filter", "-TokenPool::GetElementRek(): pType[ nId ] != T_Id");
        return false;
    }

    if (nFirstId >= nP_Id)
    {
        SAL_WARN("sc.filter", "TokenPool::GetElementRek: nFirstId >= nP_Id");
        return false;
    }
    bool bRet = true;
    if (nCnt > nP_Id - nFirstId)
    {
        SAL_WARN("sc.filter", "TokenPool::GetElementRek: nCnt > nP_Id - nFirstId");
        nCnt = nP_Id - nFirstId;
        bRet = false;
    }
    for (sal_uInt16* pCurrent = &pP_Id[nFirstId]; nCnt > 0; nCnt--, pCurrent++)
    {
        if( *pCurrent < nScTokenOff )
        {// recursion or not?
            if (*pCurrent >= maElements.size())
            {
                SAL_WARN("sc.filter", "TokenPool::GetElementRek: *pCurrent >= maElements.size()");
                bRet = false;
            }
            else
            {
                bRet = GetElement( *pCurrent, pScToken );
            }
        }
        else    // elementary SC_Token
            pScToken->AddOpCode( static_cast<DefTokenId>( *pCurrent - nScTokenOff ) );
    }

    return bRet;
}

void TokenPool::operator >>( TokenId& rId )
{
    rId = static_cast<TokenId>(maElements.size() + 1);

    if (!CanAddElement())
        return;
    // Start of Token-sequence, Typeinfo, from nP_IdLast to nP_IdCurrent-1 -> length of the sequence
    maElements.emplace_back(nP_IdLast, T_Id, nP_IdCurrent - nP_IdLast);

    nP_IdLast = nP_IdCurrent;
}

TokenId TokenPool::Store( const double& rDouble )
{
    if (!CanAddElement())
        return static_cast<const TokenId>(maElements.size() + 1);

    if( pP_Dbl.m_writemark >= pP_Dbl.m_capacity )
        if (!pP_Dbl.Grow())
            return static_cast<const TokenId>(maElements.size() + 1);

    // Index in Double-Array, Typeinfo Double, size does not matter
    maElements.emplace_back(pP_Dbl.m_writemark, T_D, 1);

    pP_Dbl[ pP_Dbl.m_writemark ] = rDouble;
    pP_Dbl.m_writemark++;

    return static_cast<const TokenId>(maElements.size()); // return old value + 1!
}

TokenId TokenPool::Store( const sal_uInt16 nIndex )
{
    return StoreName(nIndex, -1);
}

TokenId TokenPool::Store( const OUString& rString )
{
    // mostly copied to Store( const char* ), to avoid a temporary string
    if (!CanAddElement())
        return static_cast<const TokenId>(maElements.size() + 1);

    if( ppP_Str.m_writemark >= ppP_Str.m_capacity )
        if (!ppP_Str.Grow())
            return static_cast<const TokenId>(maElements.size() + 1);

    // create String
    if( !ppP_Str[ ppP_Str.m_writemark ] )
        //...but only, if it does not exist already
        ppP_Str[ ppP_Str.m_writemark ].reset( new OUString( rString ) );
    else
        //...copy otherwise
        *ppP_Str[ ppP_Str.m_writemark ] = rString;

    // Index in String-Array, Typeinfo String, size (attention truncate to 16 bits)
    maElements.emplace_back(ppP_Str.m_writemark, T_Str,
                           static_cast<sal_uInt16>(ppP_Str[ppP_Str.m_writemark]->getLength()));

    ppP_Str.m_writemark++;

    return static_cast<const TokenId>(maElements.size()); // return old value + 1!
}

TokenId TokenPool::Store( const ScSingleRefData& rTr )
{
    if (!CanAddElement())
        return static_cast<const TokenId>(maElements.size() + 1);

    if( ppP_RefTr.m_writemark >= ppP_RefTr.m_capacity )
        if (!ppP_RefTr.Grow())
            return static_cast<const TokenId>(maElements.size() + 1);

    // Typeinfo Cell-Ref
    maElements.emplace_back(ppP_RefTr.m_writemark, T_RefC, 0);

    if( !ppP_RefTr[ ppP_RefTr.m_writemark ] )
        ppP_RefTr[ ppP_RefTr.m_writemark ].reset( new ScSingleRefData( rTr ) );
    else
        *ppP_RefTr[ ppP_RefTr.m_writemark ] = rTr;

    ppP_RefTr.m_writemark++;

    return static_cast<const TokenId>(maElements.size()); // return old value + 1!
}

TokenId TokenPool::Store( const ScComplexRefData& rTr )
{
    if (!CanAddElement())
        return static_cast<const TokenId>(maElements.size() + 1);

    if( ppP_RefTr.m_writemark + 1 >= ppP_RefTr.m_capacity )
        if (!ppP_RefTr.Grow(2))
            return static_cast<const TokenId>(maElements.size() + 1);

    // setTypeinfo Area-Ref
    maElements.emplace_back(ppP_RefTr.m_writemark, T_RefA, 0);

    if( !ppP_RefTr[ ppP_RefTr.m_writemark ] )
        ppP_RefTr[ ppP_RefTr.m_writemark ].reset( new ScSingleRefData( rTr.Ref1 ) );
    else
        *ppP_RefTr[ ppP_RefTr.m_writemark ] = rTr.Ref1;
    ppP_RefTr.m_writemark++;

    if( !ppP_RefTr[ ppP_RefTr.m_writemark ] )
        ppP_RefTr[ ppP_RefTr.m_writemark ].reset( new ScSingleRefData( rTr.Ref2 ) );
    else
        *ppP_RefTr[ ppP_RefTr.m_writemark ] = rTr.Ref2;
    ppP_RefTr.m_writemark++;

    return static_cast<const TokenId>(maElements.size()); // return old value + 2!
}

TokenId TokenPool::Store( const DefTokenId e, const OUString& r )
{
    if (!CanAddElement())
        return static_cast<const TokenId>(maElements.size() + 1);

    if( ppP_Ext.m_writemark >= ppP_Ext.m_capacity )
        if (!ppP_Ext.Grow())
            return static_cast<const TokenId>(maElements.size() + 1);

    // Typeinfo "something unknown with function name"
    maElements.emplace_back(ppP_Ext.m_writemark, T_Ext, 0);

    if( ppP_Ext[ ppP_Ext.m_writemark ] )
    {
        ppP_Ext[ ppP_Ext.m_writemark ]->eId = e;
        ppP_Ext[ ppP_Ext.m_writemark ]->aText = r;
    }
    else
        ppP_Ext[ ppP_Ext.m_writemark ].reset( new EXTCONT( e, r ) );

    ppP_Ext.m_writemark++;

    return static_cast<const TokenId>(maElements.size()); // return old value + 1!
}

TokenId TokenPool::StoreNlf( const ScSingleRefData& rTr )
{
    if (!CanAddElement())
        return static_cast<const TokenId>(maElements.size() + 1);

    if( ppP_Nlf.m_writemark >= ppP_Nlf.m_capacity )
        if (!ppP_Nlf.Grow())
            return static_cast<const TokenId>(maElements.size() + 1);

    maElements.emplace_back(ppP_Nlf.m_writemark, T_Nlf, 0);

    if( ppP_Nlf[ ppP_Nlf.m_writemark ] )
    {
        *ppP_Nlf[ ppP_Nlf.m_writemark ] = rTr;
    }
    else
        ppP_Nlf[ ppP_Nlf.m_writemark ].reset( new ScSingleRefData( rTr ) );

    ppP_Nlf.m_writemark++;

    return static_cast<const TokenId>(maElements.size());
}

TokenId TokenPool::StoreMatrix()
{
    if (!CanAddElement())
        return static_cast<const TokenId>(maElements.size() + 1);

    if( nP_MatrixCurrent >= nP_Matrix )
        if (!GrowMatrix())
            return static_cast<const TokenId>(maElements.size() + 1);

    maElements.emplace_back(nP_MatrixCurrent, T_Matrix, 0);

    ScMatrix* pM = new ScMatrix( 0, 0 );
    pM->IncRef( );
    ppP_Matrix[ nP_MatrixCurrent ] = pM;

    nP_MatrixCurrent++;

    return static_cast<const TokenId>(maElements.size());
}

TokenId TokenPool::StoreName( sal_uInt16 nIndex, sal_Int16 nSheet )
{
    if (!CanAddElement())
        return static_cast<const TokenId>(maElements.size() + 1);

    maElements.emplace_back(static_cast<sal_uInt16>(maRangeNames.size()), T_RN, 0);

    maRangeNames.emplace_back();
    RangeName& r = maRangeNames.back();
    r.mnIndex = nIndex;
    r.mnSheet = nSheet;

    return static_cast<const TokenId>(maElements.size());
}

TokenId TokenPool::StoreExtName( sal_uInt16 nFileId, const OUString& rName )
{
    if (!CanAddElement())
        return static_cast<const TokenId>(maElements.size() + 1);

    maElements.emplace_back(static_cast<sal_uInt16>(maExtNames.size()), T_ExtName, 0);

    maExtNames.emplace_back();
    ExtName& r = maExtNames.back();
    r.mnFileId = nFileId;
    r.maName = rName;

    return static_cast<const TokenId>(maElements.size());
}

TokenId TokenPool::StoreExtRef( sal_uInt16 nFileId, const OUString& rTabName, const ScSingleRefData& rRef )
{
    if (!CanAddElement())
        return static_cast<const TokenId>(maElements.size() + 1);

    maElements.emplace_back(static_cast<sal_uInt16>(maExtCellRefs.size()), T_ExtRefC, 0);

    maExtCellRefs.emplace_back();
    ExtCellRef& r = maExtCellRefs.back();
    r.mnFileId = nFileId;
    r.maTabName = rTabName;
    r.maRef = rRef;

    return static_cast<const TokenId>(maElements.size());
}

TokenId TokenPool::StoreExtRef( sal_uInt16 nFileId, const OUString& rTabName, const ScComplexRefData& rRef )
{
    if (!CanAddElement())
        return static_cast<const TokenId>(maElements.size() + 1);

    maElements.emplace_back(static_cast<sal_uInt16>(maExtAreaRefs.size()), T_ExtRefA, 0);

    maExtAreaRefs.emplace_back();
    ExtAreaRef& r = maExtAreaRefs.back();
    r.mnFileId = nFileId;
    r.maTabName = rTabName;
    r.maRef = rRef;

    return static_cast<const TokenId>(maElements.size());
}

void TokenPool::Reset()
{
    nP_IdCurrent = nP_IdLast
        = ppP_Str.m_writemark = pP_Dbl.m_writemark = pP_Err.m_writemark
        = ppP_RefTr.m_writemark = ppP_Ext.m_writemark = ppP_Nlf.m_writemark = nP_MatrixCurrent = 0;
    maElements.clear();
    maRangeNames.clear();
    maExtNames.clear();
    maExtCellRefs.clear();
    maExtAreaRefs.clear();
    ClearMatrix();
}

bool TokenPool::IsSingleOp( const TokenId& rId, const DefTokenId eId ) const
{
    sal_uInt16 nId = static_cast<sal_uInt16>(rId);
    if (nId && nId <= maElements.size())
    {// existent?
        nId--;
        if (const auto [nPid, eType, nSize] = maElements[nId]; T_Id == eType)
        {// Token-Sequence?
            if (nSize == 1)
            {// EXACTLY 1 Token
                if (nPid < nP_Id)
                {
                    sal_uInt16 nSecId = pP_Id[ nPid ];
                    if( nSecId >= nScTokenOff )
                    {// Default-Token?
                        return static_cast<DefTokenId>( nSecId - nScTokenOff ) == eId;  // wanted?
                    }
                }
            }
        }
    }

    return false;
}

const OUString* TokenPool::GetExternal( const TokenId& rId ) const
{
    const OUString*   p = nullptr;
    sal_uInt16 n = static_cast<sal_uInt16>(rId);
    if (n && n <= maElements.size())
    {
        n--;
        if ([[maybe_unused]] const auto [nExt, eType, _] = maElements[n]; eType == T_Ext)
        {
            if ( nExt < ppP_Ext.m_writemark && ppP_Ext[ nExt ] )
                p = &ppP_Ext[ nExt ]->aText;
        }
    }

    return p;
}

ScMatrix* TokenPool::GetMatrix( unsigned int n ) const
{
    if( n < nP_MatrixCurrent )
        return ppP_Matrix[ n ];
    else
        SAL_WARN("sc.filter", "GetMatrix: " << n << " >= " << nP_MatrixCurrent);
    return nullptr;
}

void TokenPool::ClearMatrix()
{
    for(sal_uInt16 n = 0 ; n < nP_Matrix ; n++ )
    {
        if( ppP_Matrix[ n ] )
        {
            ppP_Matrix[ n ]->DecRef( );
            ppP_Matrix[n] = nullptr;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
