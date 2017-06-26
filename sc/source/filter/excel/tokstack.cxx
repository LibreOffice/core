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

#include "tokstack.hxx"
#include "compiler.hxx"
#include "global.hxx"
#include "scmatrix.hxx"

#include <svl/sharedstringpool.hxx>

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

    pScToken = new ScTokenArray;

    Reset();
}

TokenPool::~TokenPool()
{
    ClearMatrix();

    delete pScToken;
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

bool TokenPool::GrowElement()
{
    sal_uInt16 nElementNew = lcl_canGrow( nElement);
    if (!nElementNew)
        return false;

    sal_uInt16* pElementNew = new (::std::nothrow) sal_uInt16[ nElementNew ];
    E_TYPE* pTypeNew = new (::std::nothrow) E_TYPE[ nElementNew ];
    sal_uInt16* pSizeNew = new (::std::nothrow) sal_uInt16[ nElementNew ];
    if (!pElementNew || !pTypeNew || !pSizeNew)
    {
        delete [] pElementNew;
        delete [] pTypeNew;
        delete [] pSizeNew;
        return false;
    }

    for( sal_uInt16 nL = 0 ; nL < nElement ; nL++ )
    {
        pElementNew[ nL ] = pElement[ nL ];
        pTypeNew[ nL ] = pType[ nL ];
        pSizeNew[ nL ] = pSize[ nL ];
    }

    nElement = nElementNew;

    pElement.reset( pElementNew );
    pType.reset( pTypeNew );
    pSize.reset( pSizeNew );
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

    for( sal_uInt16 nL = 0 ; nL < nP_Matrix ; nL++ )
        ppNew[ nL ] = ppP_Matrix[ nL ];

    ppP_Matrix.reset( ppNew );
    nP_Matrix = nNewSize;
    return true;
}

bool TokenPool::GetElement( const sal_uInt16 nId )
{
    OSL_ENSURE( nId < nElementAkt, "*TokenPool::GetElement(): Id too large!?" );
    if (nId >= nElementAkt)
        return false;

    bool bRet = true;
    if( pType[ nId ] == T_Id )
        bRet = GetElementRek( nId );
    else
    {
        switch( pType[ nId ] )
        {
            case T_Str:
                {
                    sal_uInt16 n = pElement[ nId ];
                    auto* p = ppP_Str.getIfInRange( n );
                    if (p)
                        pScToken->AddString(mrStringPool.intern(*p->get()));
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

bool TokenPool::GetElementRek( const sal_uInt16 nId )
{
#ifdef DBG_UTIL
    m_nRek++;
    OSL_ENSURE(m_nRek <= nP_Id, "*TokenPool::GetElement(): recursion loops!?");
#endif

    OSL_ENSURE( nId < nElementAkt, "*TokenPool::GetElementRek(): nId >= nElementAkt" );

    if (nId >= nElementAkt)
    {
        SAL_WARN("sc.filter", "*TokenPool::GetElementRek(): nId >= nElementAkt");
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
    sal_uInt16      nAnz = pSize[ nId ];
    sal_uInt16 nFirstId = pElement[ nId ];
    if (nFirstId >= nP_Id)
    {
        SAL_WARN("sc.filter", "TokenPool::GetElementRek: nFirstId >= nP_Id");
        nAnz = 0;
        bRet = false;
    }
    sal_uInt16* pAkt = nAnz ? &pP_Id[ nFirstId ] : nullptr;
    if (nAnz > nP_Id - nFirstId)
    {
        SAL_WARN("sc.filter", "TokenPool::GetElementRek: nAnz > nP_Id - nFirstId");
        nAnz = nP_Id - nFirstId;
        bRet = false;
    }
    for( ; nAnz > 0 ; nAnz--, pAkt++ )
    {
        assert(pAkt);
        if( *pAkt < nScTokenOff )
        {// recursion or not?
            if (*pAkt >= nElementAkt)
            {
                SAL_WARN("sc.filter", "TokenPool::GetElementRek: *pAkt >= nElementAkt");
                bRet = false;
            }
            else
            {
                if (pType[ *pAkt ] == T_Id)
                    bRet = GetElementRek( *pAkt );
                else
                    bRet = GetElement( *pAkt );
            }
        }
        else    // elementary SC_Token
            pScToken->AddOpCode( ( DefTokenId ) ( *pAkt - nScTokenOff ) );
    }

#ifdef DBG_UTIL
    m_nRek--;
#endif
    return bRet;
}

void TokenPool::operator >>( TokenId& rId )
{
    rId = static_cast<TokenId>( nElementAkt + 1 );

    if( nElementAkt >= nElement )
        if (!GrowElement())
            return;

    pElement[ nElementAkt ] = nP_IdLast;    // Start of Token-sequence
    pType[ nElementAkt ] = T_Id;            // set Typeinfo
    pSize[ nElementAkt ] = nP_IdAkt - nP_IdLast;
        // write from nP_IdLast to nP_IdAkt-1 -> length of the sequence

    nElementAkt++;          // start value for next sequence
    nP_IdLast = nP_IdAkt;
}

const TokenId TokenPool::Store( const double& rDouble )
{
    if( nElementAkt >= nElement )
        if (!GrowElement())
            return static_cast<const TokenId>(nElementAkt+1);

    if( pP_Dbl.m_writemark >= pP_Dbl.m_capacity )
        if (!pP_Dbl.Grow())
            return static_cast<const TokenId>(nElementAkt+1);

    pElement[ nElementAkt ] = pP_Dbl.m_writemark;    // Index in Double-Array
    pType[ nElementAkt ] = T_D;             // set Typeinfo Double

    pP_Dbl[ pP_Dbl.m_writemark ] = rDouble;

    pSize[ nElementAkt ] = 1;               // does not matter

    nElementAkt++;
    pP_Dbl.m_writemark++;

    return static_cast<const TokenId>(nElementAkt); // return old value + 1!
}

const TokenId TokenPool::Store( const sal_uInt16 nIndex )
{
    return StoreName(nIndex, -1);
}

const TokenId TokenPool::Store( const OUString& rString )
{
    // mostly copied to Store( const sal_Char* ), to avoid a temporary string
    if( nElementAkt >= nElement )
        if (!GrowElement())
            return static_cast<const TokenId>(nElementAkt+1);

    if( ppP_Str.m_writemark >= ppP_Str.m_capacity )
        if (!ppP_Str.Grow())
            return static_cast<const TokenId>(nElementAkt+1);

    pElement[ nElementAkt ] = ppP_Str.m_writemark;    // Index in String-Array
    pType[ nElementAkt ] = T_Str;           // set Typeinfo String

    // create String
    if( !ppP_Str[ ppP_Str.m_writemark ] )
        //...but only, if it does not exist already
        ppP_Str[ ppP_Str.m_writemark ].reset( new OUString( rString ) );
    else
        //...copy otherwise
        *ppP_Str[ ppP_Str.m_writemark ] = rString;

    /* attention truncate to 16 bits */
    pSize[ nElementAkt ] = ( sal_uInt16 ) ppP_Str[ ppP_Str.m_writemark ]->getLength();

    nElementAkt++;
    ppP_Str.m_writemark++;

    return static_cast<const TokenId>(nElementAkt); // return old value + 1!
}

const TokenId TokenPool::Store( const ScSingleRefData& rTr )
{
    if( nElementAkt >= nElement )
        if (!GrowElement())
            return static_cast<const TokenId>(nElementAkt+1);

    if( ppP_RefTr.m_writemark >= ppP_RefTr.m_capacity )
        if (!ppP_RefTr.Grow())
            return static_cast<const TokenId>(nElementAkt+1);

    pElement[ nElementAkt ] = ppP_RefTr.m_writemark;
    pType[ nElementAkt ] = T_RefC;          // set Typeinfo Cell-Ref

    if( !ppP_RefTr[ ppP_RefTr.m_writemark ] )
        ppP_RefTr[ ppP_RefTr.m_writemark ].reset( new ScSingleRefData( rTr ) );
    else
        *ppP_RefTr[ ppP_RefTr.m_writemark ] = rTr;

    nElementAkt++;
    ppP_RefTr.m_writemark++;

    return static_cast<const TokenId>(nElementAkt); // return old value + 1!
}

const TokenId TokenPool::Store( const ScComplexRefData& rTr )
{
    if( nElementAkt >= nElement )
        if (!GrowElement())
            return static_cast<const TokenId>(nElementAkt+1);

    if( ppP_RefTr.m_writemark + 1 >= ppP_RefTr.m_capacity )
        if (!ppP_RefTr.Grow(2))
            return static_cast<const TokenId>(nElementAkt+1);

    pElement[ nElementAkt ] = ppP_RefTr.m_writemark;
    pType[ nElementAkt ] = T_RefA;          // setTypeinfo Area-Ref

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

    nElementAkt++;

    return static_cast<const TokenId>(nElementAkt); // return old value + 1!
}

const TokenId TokenPool::Store( const DefTokenId e, const OUString& r )
{
    if( nElementAkt >= nElement )
        if (!GrowElement())
            return static_cast<const TokenId>(nElementAkt+1);

    if( ppP_Ext.m_writemark >= ppP_Ext.m_capacity )
        if (!ppP_Ext.Grow())
            return static_cast<const TokenId>(nElementAkt+1);

    pElement[ nElementAkt ] = ppP_Ext.m_writemark;
    pType[ nElementAkt ] = T_Ext;           // set Typeinfo String

    if( ppP_Ext[ ppP_Ext.m_writemark ] )
    {
        ppP_Ext[ ppP_Ext.m_writemark ]->eId = e;
        ppP_Ext[ ppP_Ext.m_writemark ]->aText = r;
    }
    else
        ppP_Ext[ ppP_Ext.m_writemark ].reset( new EXTCONT( e, r ) );

    nElementAkt++;
    ppP_Ext.m_writemark++;

    return static_cast<const TokenId>(nElementAkt); // return old value + 1!
}

const TokenId TokenPool::StoreNlf( const ScSingleRefData& rTr )
{
    if( nElementAkt >= nElement )
        if (!GrowElement())
            return static_cast<const TokenId>(nElementAkt+1);

    if( ppP_Nlf.m_writemark >= ppP_Nlf.m_capacity )
        if (!ppP_Nlf.Grow())
            return static_cast<const TokenId>(nElementAkt+1);

    pElement[ nElementAkt ] = ppP_Nlf.m_writemark;
    pType[ nElementAkt ] = T_Nlf;

    if( ppP_Nlf[ ppP_Nlf.m_writemark ] )
    {
        *ppP_Nlf[ ppP_Nlf.m_writemark ] = rTr;
    }
    else
        ppP_Nlf[ ppP_Nlf.m_writemark ].reset( new ScSingleRefData( rTr ) );

    nElementAkt++;
    ppP_Nlf.m_writemark++;

    return static_cast<const TokenId>(nElementAkt);
}

const TokenId TokenPool::StoreMatrix()
{
    if( nElementAkt >= nElement )
        if (!GrowElement())
            return static_cast<const TokenId>(nElementAkt+1);

    if( nP_MatrixAkt >= nP_Matrix )
        if (!GrowMatrix())
            return static_cast<const TokenId>(nElementAkt+1);

    pElement[ nElementAkt ] = nP_MatrixAkt;
    pType[ nElementAkt ] = T_Matrix;

    ScMatrix* pM = new ScFullMatrix( 0, 0 );
    pM->IncRef( );
    ppP_Matrix[ nP_MatrixAkt ] = pM;

    nElementAkt++;
    nP_MatrixAkt++;

    return static_cast<const TokenId>(nElementAkt);
}

const TokenId TokenPool::StoreName( sal_uInt16 nIndex, sal_Int16 nSheet )
{
    if ( nElementAkt >= nElement )
        if (!GrowElement())
            return static_cast<const TokenId>(nElementAkt+1);

    pElement[nElementAkt] = static_cast<sal_uInt16>(maRangeNames.size());
    pType[nElementAkt] = T_RN;

    maRangeNames.push_back(RangeName());
    RangeName& r = maRangeNames.back();
    r.mnIndex = nIndex;
    r.mnSheet = nSheet;

    ++nElementAkt;

    return static_cast<const TokenId>(nElementAkt);
}

const TokenId TokenPool::StoreExtName( sal_uInt16 nFileId, const OUString& rName )
{
    if ( nElementAkt >= nElement )
        if (!GrowElement())
            return static_cast<const TokenId>(nElementAkt+1);

    pElement[nElementAkt] = static_cast<sal_uInt16>(maExtNames.size());
    pType[nElementAkt] = T_ExtName;

    maExtNames.push_back(ExtName());
    ExtName& r = maExtNames.back();
    r.mnFileId = nFileId;
    r.maName = rName;

    ++nElementAkt;

    return static_cast<const TokenId>(nElementAkt);
}

const TokenId TokenPool::StoreExtRef( sal_uInt16 nFileId, const OUString& rTabName, const ScSingleRefData& rRef )
{
    if ( nElementAkt >= nElement )
        if (!GrowElement())
            return static_cast<const TokenId>(nElementAkt+1);

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

const TokenId TokenPool::StoreExtRef( sal_uInt16 nFileId, const OUString& rTabName, const ScComplexRefData& rRef )
{
    if ( nElementAkt >= nElement )
        if (!GrowElement())
            return static_cast<const TokenId>(nElementAkt+1);

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

void TokenPool::Reset()
{
    nP_IdAkt = nP_IdLast = nElementAkt
        = ppP_Str.m_writemark = pP_Dbl.m_writemark = pP_Err.m_writemark
        = ppP_RefTr.m_writemark = ppP_Ext.m_writemark = ppP_Nlf.m_writemark = nP_MatrixAkt = 0;
    maRangeNames.clear();
    maExtNames.clear();
    maExtCellRefs.clear();
    maExtAreaRefs.clear();
    ClearMatrix();
}

bool TokenPool::IsSingleOp( const TokenId& rId, const DefTokenId eId ) const
{
    sal_uInt16 nId = (sal_uInt16) rId;
    if( nId && nId <= nElementAkt )
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
                        return ( DefTokenId ) ( nSecId - nScTokenOff ) == eId;  // wanted?
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
    sal_uInt16 n = (sal_uInt16) rId;
    if( n && n <= nElementAkt )
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
    if( n < nP_MatrixAkt )
        return ppP_Matrix[ n ];
    else
        SAL_WARN("sc.filter", "GetMatrix: " << n << " >= " << nP_MatrixAkt);
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
