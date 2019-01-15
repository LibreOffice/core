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

    // pool for Ids
    nElement = 32;
    pElement.reset( new sal_uInt16[ nElement ] );
    pType.reset( new E_TYPE[ nElement ] );
    pSize.reset( new sal_uInt16[ nElement ] );
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

bool TokenPool::CheckElementOrGrow()
{
    // Last possible ID to be assigned somewhere is nElementCurrent+1
    if (nElementCurrent + 1 == nScTokenOff - 1)
    {
        SAL_WARN("sc.filter","TokenPool::CheckElementOrGrow - last possible ID " << nElementCurrent+1);
        return false;
    }

    if (nElementCurrent >= nElement)
        return GrowElement();

    return true;
}

bool TokenPool::GrowElement()
{
    sal_uInt16 nElementNew = lcl_canGrow( nElement);
    if (!nElementNew)
        return false;

    std::unique_ptr<sal_uInt16[]> pElementNew(new (::std::nothrow) sal_uInt16[ nElementNew ]);
    std::unique_ptr<E_TYPE[]> pTypeNew(new (::std::nothrow) E_TYPE[ nElementNew ]);
    std::unique_ptr<sal_uInt16[]> pSizeNew(new (::std::nothrow) sal_uInt16[ nElementNew ]);
    if (!pElementNew || !pTypeNew || !pSizeNew)
    {
        return false;
    }

    for( sal_uInt16 nL = 0 ; nL < nElement ; nL++ )
    {
        pElementNew[ nL ] = pElement[ nL ];
        pTypeNew[ nL ] = pType[ nL ];
        pSizeNew[ nL ] = pSize[ nL ];
    }

    nElement = nElementNew;

    pElement = std::move( pElementNew );
    pType = std::move( pTypeNew );
    pSize = std::move( pSizeNew );
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
    if (nId >= nElementCurrent)
    {
        SAL_WARN("sc.filter","TokenPool::GetElement - Id too large, " << nId << " >= " << nElementCurrent);
        return false;
    }

    bool bRet = true;
    if( pType[ nId ] == T_Id )
        bRet = GetElementRek( nId, pScToken );
    else
    {
        switch( pType[ nId ] )
        {
            case T_Str:
                {
                    sal_uInt16 n = pElement[ nId ];
                    auto* p = ppP_Str.getIfInRange( n );
                    if (p)
                        pScToken->AddString(mrStringPool.intern(**p));
                    else
                        bRet = false;
                }
                break;
            case T_D:
                {
                    sal_uInt16 n = pElement[ nId ];
                    if (n < pP_Dbl.m_writemark)
                        pScToken->AddDouble( pP_Dbl[ n ] );
                    else
                        bRet = false;
                }
                break;
            case T_Err:
                break;
/* TODO: in case we had FormulaTokenArray::AddError() */
#if 0
                {
                    sal_uInt16 n = pElement[ nId ];
                    if (n < nP_Err)
                        pScToken->AddError( pP_Err[ n ] );
                    else
                        bRet = false;
                }
#endif
            case T_RefC:
                {
                    sal_uInt16 n = pElement[ nId ];
                    auto p = ppP_RefTr.getIfInRange(n);
                    if (p)
                        pScToken->AddSingleReference( **p );
                    else
                        bRet = false;
                }
                break;
            case T_RefA:
                {
                    sal_uInt16 n = pElement[ nId ];
                    if (n < ppP_RefTr.m_writemark && ppP_RefTr[ n ] && n+1 < ppP_RefTr.m_writemark && ppP_RefTr[ n + 1 ])
                    {
                        ScComplexRefData aScComplexRefData;
                        aScComplexRefData.Ref1 = *ppP_RefTr[ n ];
                        aScComplexRefData.Ref2 = *ppP_RefTr[ n + 1 ];
                        pScToken->AddDoubleReference( aScComplexRefData );
                    }
                    else
                        bRet = false;
                }
                break;
            case T_RN:
            {
                sal_uInt16 n = pElement[nId];
                if (n < maRangeNames.size())
                {
                    const RangeName& r = maRangeNames[n];
                    pScToken->AddRangeName(r.mnIndex, r.mnSheet);
                }
            }
            break;
            case T_Ext:
                {
                    sal_uInt16 n = pElement[ nId ];
                    auto       p = ppP_Ext.getIfInRange(n);

                    if( p )
                    {
                        if( (*p)->eId == ocEuroConvert )
                            pScToken->AddOpCode( (*p)->eId );
                        else
                            pScToken->AddExternal( (*p)->aText, (*p)->eId );
                    }
                    else
                        bRet = false;
                }
                break;
            case T_Nlf:
                {
                    sal_uInt16 n = pElement[ nId ];
                    auto       p = ppP_Nlf.getIfInRange(n);

                    if( p )
                        pScToken->AddColRowName( **p );
                    else
                        bRet = false;
                }
                break;
            case T_Matrix:
                {
                    sal_uInt16      n = pElement[ nId ];
                    ScMatrix*       p = ( n < nP_Matrix )? ppP_Matrix[ n ] : nullptr;

                    if( p )
                        pScToken->AddMatrix( p );
                    else
                        bRet = false;
                }
                break;
            case T_ExtName:
            {
                sal_uInt16 n = pElement[nId];
                if (n < maExtNames.size())
                {
                    const ExtName& r = maExtNames[n];
                    pScToken->AddExternalName(r.mnFileId, mrStringPool.intern( r.maName));
                }
                else
                    bRet = false;
            }
            break;
            case T_ExtRefC:
            {
                sal_uInt16 n = pElement[nId];
                if (n < maExtCellRefs.size())
                {
                    const ExtCellRef& r = maExtCellRefs[n];
                    pScToken->AddExternalSingleReference(r.mnFileId, mrStringPool.intern( r.maTabName), r.maRef);
                }
                else
                    bRet = false;
            }
            break;
            case T_ExtRefA:
            {
                sal_uInt16 n = pElement[nId];
                if (n < maExtAreaRefs.size())
                {
                    const ExtAreaRef& r = maExtAreaRefs[n];
                    pScToken->AddExternalDoubleReference(r.mnFileId, mrStringPool.intern( r.maTabName), r.maRef);
                }
                else
                    bRet = false;
            }
            break;
            default:
                OSL_FAIL("-TokenPool::GetElement(): undefined state!?");
                bRet = false;
        }
    }
    return bRet;
}

bool TokenPool::GetElementRek( const sal_uInt16 nId, ScTokenArray* pScToken )
{
#ifdef DBG_UTIL
    m_nRek++;
    OSL_ENSURE(m_nRek <= nP_Id, "*TokenPool::GetElement(): recursion loops!?");
#endif

    OSL_ENSURE( nId < nElementCurrent, "*TokenPool::GetElementRek(): nId >= nElementCurrent" );

    if (nId >= nElementCurrent)
    {
        SAL_WARN("sc.filter", "*TokenPool::GetElementRek(): nId >= nElementCurrent");
#ifdef DBG_UTIL
        m_nRek--;
#endif
        return false;
    }

    if (pType[ nId ] != T_Id)
    {
        SAL_WARN("sc.filter", "-TokenPool::GetElementRek(): pType[ nId ] != T_Id");
#ifdef DBG_UTIL
        m_nRek--;
#endif
        return false;
    }

    bool bRet = true;
    sal_uInt16      nCnt = pSize[ nId ];
    sal_uInt16 nFirstId = pElement[ nId ];
    if (nFirstId >= nP_Id)
    {
        SAL_WARN("sc.filter", "TokenPool::GetElementRek: nFirstId >= nP_Id");
        nCnt = 0;
        bRet = false;
    }
    sal_uInt16* pCurrent = nCnt ? &pP_Id[ nFirstId ] : nullptr;
    if (nCnt > nP_Id - nFirstId)
    {
        SAL_WARN("sc.filter", "TokenPool::GetElementRek: nCnt > nP_Id - nFirstId");
        nCnt = nP_Id - nFirstId;
        bRet = false;
    }
    for( ; nCnt > 0 ; nCnt--, pCurrent++ )
    {
        assert(pCurrent);
        if( *pCurrent < nScTokenOff )
        {// recursion or not?
            if (*pCurrent >= nElementCurrent)
            {
                SAL_WARN("sc.filter", "TokenPool::GetElementRek: *pCurrent >= nElementCurrent");
                bRet = false;
            }
            else
            {
                if (pType[ *pCurrent ] == T_Id)
                    bRet = GetElementRek( *pCurrent, pScToken );
                else
                    bRet = GetElement( *pCurrent, pScToken );
            }
        }
        else    // elementary SC_Token
            pScToken->AddOpCode( static_cast<DefTokenId>( *pCurrent - nScTokenOff ) );
    }

#ifdef DBG_UTIL
    m_nRek--;
#endif
    return bRet;
}

void TokenPool::operator >>( TokenId& rId )
{
    rId = static_cast<TokenId>( nElementCurrent + 1 );

    if (!CheckElementOrGrow())
        return;

    pElement[ nElementCurrent ] = nP_IdLast;    // Start of Token-sequence
    pType[ nElementCurrent ] = T_Id;            // set Typeinfo
    pSize[ nElementCurrent ] = nP_IdCurrent - nP_IdLast;
        // write from nP_IdLast to nP_IdCurrent-1 -> length of the sequence

    nElementCurrent++;          // start value for next sequence
    nP_IdLast = nP_IdCurrent;
}

const TokenId TokenPool::Store( const double& rDouble )
{
    if (!CheckElementOrGrow())
        return static_cast<const TokenId>(nElementCurrent+1);

    if( pP_Dbl.m_writemark >= pP_Dbl.m_capacity )
        if (!pP_Dbl.Grow())
            return static_cast<const TokenId>(nElementCurrent+1);

    pElement[ nElementCurrent ] = pP_Dbl.m_writemark;    // Index in Double-Array
    pType[ nElementCurrent ] = T_D;             // set Typeinfo Double

    pP_Dbl[ pP_Dbl.m_writemark ] = rDouble;

    pSize[ nElementCurrent ] = 1;               // does not matter

    nElementCurrent++;
    pP_Dbl.m_writemark++;

    return static_cast<const TokenId>(nElementCurrent); // return old value + 1!
}

const TokenId TokenPool::Store( const sal_uInt16 nIndex )
{
    return StoreName(nIndex, -1);
}

const TokenId TokenPool::Store( const OUString& rString )
{
    // mostly copied to Store( const sal_Char* ), to avoid a temporary string
    if (!CheckElementOrGrow())
        return static_cast<const TokenId>(nElementCurrent+1);

    if( ppP_Str.m_writemark >= ppP_Str.m_capacity )
        if (!ppP_Str.Grow())
            return static_cast<const TokenId>(nElementCurrent+1);

    pElement[ nElementCurrent ] = ppP_Str.m_writemark;    // Index in String-Array
    pType[ nElementCurrent ] = T_Str;           // set Typeinfo String

    // create String
    if( !ppP_Str[ ppP_Str.m_writemark ] )
        //...but only, if it does not exist already
        ppP_Str[ ppP_Str.m_writemark ].reset( new OUString( rString ) );
    else
        //...copy otherwise
        *ppP_Str[ ppP_Str.m_writemark ] = rString;

    /* attention truncate to 16 bits */
    pSize[ nElementCurrent ] = static_cast<sal_uInt16>(ppP_Str[ ppP_Str.m_writemark ]->getLength());

    nElementCurrent++;
    ppP_Str.m_writemark++;

    return static_cast<const TokenId>(nElementCurrent); // return old value + 1!
}

const TokenId TokenPool::Store( const ScSingleRefData& rTr )
{
    if (!CheckElementOrGrow())
        return static_cast<const TokenId>(nElementCurrent+1);

    if( ppP_RefTr.m_writemark >= ppP_RefTr.m_capacity )
        if (!ppP_RefTr.Grow())
            return static_cast<const TokenId>(nElementCurrent+1);

    pElement[ nElementCurrent ] = ppP_RefTr.m_writemark;
    pType[ nElementCurrent ] = T_RefC;          // set Typeinfo Cell-Ref

    if( !ppP_RefTr[ ppP_RefTr.m_writemark ] )
        ppP_RefTr[ ppP_RefTr.m_writemark ].reset( new ScSingleRefData( rTr ) );
    else
        *ppP_RefTr[ ppP_RefTr.m_writemark ] = rTr;

    nElementCurrent++;
    ppP_RefTr.m_writemark++;

    return static_cast<const TokenId>(nElementCurrent); // return old value + 1!
}

const TokenId TokenPool::Store( const ScComplexRefData& rTr )
{
    if (!CheckElementOrGrow())
        return static_cast<const TokenId>(nElementCurrent+1);

    if( ppP_RefTr.m_writemark + 1 >= ppP_RefTr.m_capacity )
        if (!ppP_RefTr.Grow(2))
            return static_cast<const TokenId>(nElementCurrent+1);

    pElement[ nElementCurrent ] = ppP_RefTr.m_writemark;
    pType[ nElementCurrent ] = T_RefA;          // setTypeinfo Area-Ref

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

    nElementCurrent++;

    return static_cast<const TokenId>(nElementCurrent); // return old value + 1!
}

const TokenId TokenPool::Store( const DefTokenId e, const OUString& r )
{
    if (!CheckElementOrGrow())
        return static_cast<const TokenId>(nElementCurrent+1);

    if( ppP_Ext.m_writemark >= ppP_Ext.m_capacity )
        if (!ppP_Ext.Grow())
            return static_cast<const TokenId>(nElementCurrent+1);

    pElement[ nElementCurrent ] = ppP_Ext.m_writemark;
    pType[ nElementCurrent ] = T_Ext;           // set Typeinfo String

    if( ppP_Ext[ ppP_Ext.m_writemark ] )
    {
        ppP_Ext[ ppP_Ext.m_writemark ]->eId = e;
        ppP_Ext[ ppP_Ext.m_writemark ]->aText = r;
    }
    else
        ppP_Ext[ ppP_Ext.m_writemark ].reset( new EXTCONT( e, r ) );

    nElementCurrent++;
    ppP_Ext.m_writemark++;

    return static_cast<const TokenId>(nElementCurrent); // return old value + 1!
}

const TokenId TokenPool::StoreNlf( const ScSingleRefData& rTr )
{
    if (!CheckElementOrGrow())
        return static_cast<const TokenId>(nElementCurrent+1);

    if( ppP_Nlf.m_writemark >= ppP_Nlf.m_capacity )
        if (!ppP_Nlf.Grow())
            return static_cast<const TokenId>(nElementCurrent+1);

    pElement[ nElementCurrent ] = ppP_Nlf.m_writemark;
    pType[ nElementCurrent ] = T_Nlf;

    if( ppP_Nlf[ ppP_Nlf.m_writemark ] )
    {
        *ppP_Nlf[ ppP_Nlf.m_writemark ] = rTr;
    }
    else
        ppP_Nlf[ ppP_Nlf.m_writemark ].reset( new ScSingleRefData( rTr ) );

    nElementCurrent++;
    ppP_Nlf.m_writemark++;

    return static_cast<const TokenId>(nElementCurrent);
}

const TokenId TokenPool::StoreMatrix()
{
    if (!CheckElementOrGrow())
        return static_cast<const TokenId>(nElementCurrent+1);

    if( nP_MatrixCurrent >= nP_Matrix )
        if (!GrowMatrix())
            return static_cast<const TokenId>(nElementCurrent+1);

    pElement[ nElementCurrent ] = nP_MatrixCurrent;
    pType[ nElementCurrent ] = T_Matrix;

    ScMatrix* pM = new ScMatrix( 0, 0 );
    pM->IncRef( );
    ppP_Matrix[ nP_MatrixCurrent ] = pM;

    nElementCurrent++;
    nP_MatrixCurrent++;

    return static_cast<const TokenId>(nElementCurrent);
}

const TokenId TokenPool::StoreName( sal_uInt16 nIndex, sal_Int16 nSheet )
{
    if (!CheckElementOrGrow())
        return static_cast<const TokenId>(nElementCurrent+1);

    pElement[nElementCurrent] = static_cast<sal_uInt16>(maRangeNames.size());
    pType[nElementCurrent] = T_RN;

    maRangeNames.emplace_back();
    RangeName& r = maRangeNames.back();
    r.mnIndex = nIndex;
    r.mnSheet = nSheet;

    ++nElementCurrent;

    return static_cast<const TokenId>(nElementCurrent);
}

const TokenId TokenPool::StoreExtName( sal_uInt16 nFileId, const OUString& rName )
{
    if (!CheckElementOrGrow())
        return static_cast<const TokenId>(nElementCurrent+1);

    pElement[nElementCurrent] = static_cast<sal_uInt16>(maExtNames.size());
    pType[nElementCurrent] = T_ExtName;

    maExtNames.emplace_back();
    ExtName& r = maExtNames.back();
    r.mnFileId = nFileId;
    r.maName = rName;

    ++nElementCurrent;

    return static_cast<const TokenId>(nElementCurrent);
}

const TokenId TokenPool::StoreExtRef( sal_uInt16 nFileId, const OUString& rTabName, const ScSingleRefData& rRef )
{
    if (!CheckElementOrGrow())
        return static_cast<const TokenId>(nElementCurrent+1);

    pElement[nElementCurrent] = static_cast<sal_uInt16>(maExtCellRefs.size());
    pType[nElementCurrent] = T_ExtRefC;

    maExtCellRefs.emplace_back();
    ExtCellRef& r = maExtCellRefs.back();
    r.mnFileId = nFileId;
    r.maTabName = rTabName;
    r.maRef = rRef;

    ++nElementCurrent;

    return static_cast<const TokenId>(nElementCurrent);
}

const TokenId TokenPool::StoreExtRef( sal_uInt16 nFileId, const OUString& rTabName, const ScComplexRefData& rRef )
{
    if (!CheckElementOrGrow())
        return static_cast<const TokenId>(nElementCurrent+1);

    pElement[nElementCurrent] = static_cast<sal_uInt16>(maExtAreaRefs.size());
    pType[nElementCurrent] = T_ExtRefA;

    maExtAreaRefs.emplace_back();
    ExtAreaRef& r = maExtAreaRefs.back();
    r.mnFileId = nFileId;
    r.maTabName = rTabName;
    r.maRef = rRef;

    ++nElementCurrent;

    return static_cast<const TokenId>(nElementCurrent);
}

void TokenPool::Reset()
{
    nP_IdCurrent = nP_IdLast = nElementCurrent
        = ppP_Str.m_writemark = pP_Dbl.m_writemark = pP_Err.m_writemark
        = ppP_RefTr.m_writemark = ppP_Ext.m_writemark = ppP_Nlf.m_writemark = nP_MatrixCurrent = 0;
    maRangeNames.clear();
    maExtNames.clear();
    maExtCellRefs.clear();
    maExtAreaRefs.clear();
    ClearMatrix();
}

bool TokenPool::IsSingleOp( const TokenId& rId, const DefTokenId eId ) const
{
    sal_uInt16 nId = static_cast<sal_uInt16>(rId);
    if( nId && nId <= nElementCurrent )
    {// existent?
        nId--;
        if( T_Id == pType[ nId ] )
        {// Token-Sequence?
            if( pSize[ nId ] == 1 )
            {// EXACTLY 1 Token
                sal_uInt16 nPid = pElement[ nId ];
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
    if( n && n <= nElementCurrent )
    {
        n--;
        if( pType[ n ] == T_Ext )
        {
            sal_uInt16 nExt = pElement[ n ];
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
