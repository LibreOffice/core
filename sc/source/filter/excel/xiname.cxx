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

#include "xiname.hxx"
#include "rangenam.hxx"
#include "xistream.hxx"
#include "excform.hxx"
#include "excimp8.hxx"
#include "scextopt.hxx"
#include "document.hxx"
#include <o3tl/make_unique.hxx>

// *** Implementation ***

XclImpName::TokenStrmData::TokenStrmData( XclImpStream& rStrm ) :
    mrStrm(rStrm), mnStrmPos(0), mnStrmSize(0) {}

XclImpName::XclImpName( XclImpStream& rStrm, sal_uInt16 nXclNameIdx ) :
    XclImpRoot( rStrm.GetRoot() ),
    mpScData( nullptr ),
    mcBuiltIn( EXC_BUILTIN_UNKNOWN ),
    mnScTab( SCTAB_MAX ),
    meNameType( ScRangeData::Type::Name ),
    mnXclTab( EXC_NAME_GLOBAL ),
    mnNameIndex( nXclNameIdx ),
    mbVBName( false ),
    mbMacro( false ),
    mpTokensData( nullptr )
{
    ExcelToSc& rFmlaConv = GetOldFmlaConverter();

    // 1) *** read data from stream *** ---------------------------------------

    sal_uInt16 nFlags = 0, nFmlaSize = 0, nExtSheet = EXC_NAME_GLOBAL;
    sal_uInt8 nNameLen = 0;

    switch( GetBiff() )
    {
        case EXC_BIFF2:
        {
            sal_uInt8 nFlagsBiff2;
            nFlagsBiff2 = rStrm.ReaduInt8();
            rStrm.Ignore( 1 );
            rStrm.Ignore( 1 ); //nShortCut
            nNameLen = rStrm.ReaduInt8();
            nFmlaSize = rStrm.ReaduInt8();
            ::set_flag( nFlags, EXC_NAME_FUNC, ::get_flag( nFlagsBiff2, EXC_NAME2_FUNC ) );
        }
        break;

        case EXC_BIFF3:
        case EXC_BIFF4:
        {
            nFlags = rStrm.ReaduInt16();
            rStrm.Ignore( 1 ); //nShortCut
            nNameLen = rStrm.ReaduInt8();
            nFmlaSize = rStrm.ReaduInt16();
        }
        break;

        case EXC_BIFF5:
        case EXC_BIFF8:
        {
            nFlags = rStrm.ReaduInt16();
            rStrm.Ignore( 1 ); //nShortCut
            nNameLen = rStrm.ReaduInt8();
            nFmlaSize = rStrm.ReaduInt16();
            nExtSheet = rStrm.ReaduInt16();
            mnXclTab = rStrm.ReaduInt16();
            rStrm.Ignore( 4 );
        }
        break;

        default: DBG_ERROR_BIFF();
    }

    if( GetBiff() <= EXC_BIFF5 )
        maXclName = rStrm.ReadRawByteString( nNameLen );
    else
        maXclName = rStrm.ReadUniString( nNameLen );

    // 2) *** convert sheet index and name *** --------------------------------

    // functions and VBA
    bool bFunction = ::get_flag( nFlags, EXC_NAME_FUNC );
    mbVBName = ::get_flag( nFlags, EXC_NAME_VB );
    mbMacro = ::get_flag( nFlags, EXC_NAME_PROC );

    // get built-in name, or convert characters invalid in Calc
    bool bBuiltIn = ::get_flag( nFlags, EXC_NAME_BUILTIN );

    // special case for BIFF5 filter range - name appears as plain text without built-in flag
    if( (GetBiff() == EXC_BIFF5) && (maXclName == XclTools::GetXclBuiltInDefName(EXC_BUILTIN_FILTERDATABASE)) )
    {
        bBuiltIn = true;
        maXclName = OUStringLiteral1<EXC_BUILTIN_FILTERDATABASE>();
    }

    // convert Excel name to Calc name
    if( mbVBName )
    {
        // VB macro name
        maScName = maXclName;
    }
    else if( bBuiltIn )
    {
        // built-in name
        if( !maXclName.isEmpty() )
            mcBuiltIn = maXclName[0];
        if( mcBuiltIn == '?' )      // NUL character is imported as '?'
            mcBuiltIn = '\0';
        maScName = XclTools::GetBuiltInDefName( mcBuiltIn );
    }
    else
    {
        // any other name
        maScName = ScfTools::ConvertToScDefinedName( maXclName );
    }

    // add index for local names
    if( mnXclTab != EXC_NAME_GLOBAL )
    {
        sal_uInt16 nUsedTab = (GetBiff() == EXC_BIFF8) ? mnXclTab : nExtSheet;
        // TODO: may not work for BIFF5, handle skipped sheets (all BIFF)
        mnScTab = static_cast< SCTAB >( nUsedTab - 1 );
    }

    // 3) *** convert the name definition formula *** -------------------------

    rFmlaConv.Reset();
    const ScTokenArray* pTokArr = nullptr; // pointer to token array, owned by rFmlaConv

    if( ::get_flag( nFlags, EXC_NAME_BIG ) )
    {
        // special, unsupported name
        rFmlaConv.GetDummy( pTokArr );
    }
    else if( bBuiltIn )
    {
        SCsTAB const nLocalTab = (mnXclTab == EXC_NAME_GLOBAL) ? SCTAB_MAX : (mnXclTab - 1);

        // --- print ranges or title ranges ---
        rStrm.PushPosition();
        switch( mcBuiltIn )
        {
            case EXC_BUILTIN_PRINTAREA:
                if( rFmlaConv.Convert( GetPrintAreaBuffer(), rStrm, nFmlaSize, nLocalTab, FT_RangeName ) == ConvOK )
                    meNameType |= ScRangeData::Type::PrintArea;
            break;
            case EXC_BUILTIN_PRINTTITLES:
                if( rFmlaConv.Convert( GetTitleAreaBuffer(), rStrm, nFmlaSize, nLocalTab, FT_RangeName ) == ConvOK )
                    meNameType |= ScRangeData::Type::ColHeader | ScRangeData::Type::RowHeader;
            break;
        }
        rStrm.PopPosition();

        // --- name formula ---
        // JEG : double check this.  It is clearly false for normal names
        //  but some of the builtins (sheettitle?) might be able to handle arrays
        rFmlaConv.Convert( pTokArr, rStrm, nFmlaSize, false, FT_RangeName );

        // --- auto or advanced filter ---
        if( (GetBiff() == EXC_BIFF8) && pTokArr && bBuiltIn )
        {
            ScRange aRange;
            if (pTokArr->IsReference(aRange, ScAddress()))
            {
                switch( mcBuiltIn )
                {
                    case EXC_BUILTIN_FILTERDATABASE:
                        GetFilterManager().Insert( &GetOldRoot(), aRange);
                    break;
                    case EXC_BUILTIN_CRITERIA:
                        GetFilterManager().AddAdvancedRange( aRange );
                        meNameType |= ScRangeData::Type::Criteria;
                    break;
                    case EXC_BUILTIN_EXTRACT:
                        if (pTokArr->IsValidReference(aRange, ScAddress()))
                            GetFilterManager().AddExtractPos( aRange );
                    break;
                }
            }
        }
    }
    else if( nFmlaSize > 0 )
    {
        // Regular defined name.  We need to convert the tokens after all the
        // names have been registered (for cross-referenced names).
        mpTokensData.reset(new TokenStrmData(rStrm));
        mpTokensData->mnStrmPos = rStrm.GetSvStreamPos();
        rStrm.StorePosition(mpTokensData->maStrmPos);
        mpTokensData->mnStrmSize = nFmlaSize;
    }

    if (pTokArr && !bFunction && !mbVBName)
        InsertName(pTokArr);
}

void XclImpName::ConvertTokens()
{
    if (!mpTokensData)
        return;

    ExcelToSc& rFmlaConv = GetOldFmlaConverter();
    rFmlaConv.Reset();
    const ScTokenArray* pArray = nullptr;

    XclImpStreamPos aOldPos;
    XclImpStream& rStrm = mpTokensData->mrStrm;
    rStrm.StorePosition(aOldPos);
    rStrm.RestorePosition(mpTokensData->maStrmPos);
    rFmlaConv.Convert(pArray, rStrm, mpTokensData->mnStrmSize, true, FT_RangeName);
    rStrm.RestorePosition(aOldPos);

    if (pArray)
        InsertName(pArray);

    mpTokensData.reset();
}

void XclImpName::InsertName(const ScTokenArray* pArray)
{
    // create the Calc name data
    ScRangeData* pData = new ScRangeData(&GetDocRef(), maScName, *pArray, ScAddress(), meNameType);
    pData->GuessPosition();             // calculate base position for relative refs
    pData->SetIndex( mnNameIndex );     // used as unique identifier in formulas
    if (mnXclTab == EXC_NAME_GLOBAL)
    {
        if (!GetDoc().GetRangeName()->insert(pData))
            pData = nullptr;
    }
    else
    {
        ScRangeName* pLocalNames = GetDoc().GetRangeName(mnScTab);
        if (pLocalNames)
        {
            if (!pLocalNames->insert(pData))
                pData = nullptr;
        }

        if (GetBiff() == EXC_BIFF8 && pData)
        {
            ScRange aRange;
            // discard deleted ranges ( for the moment at least )
            if ( pData->IsValidReference( aRange ) )
            {
                GetExtDocOptions().GetOrCreateTabSettings( mnXclTab );
            }
        }
    }
    if (pData)
        mpScData = pData;               // cache for later use
}

XclImpNameManager::XclImpNameManager( const XclImpRoot& rRoot ) :
    XclImpRoot( rRoot )
{
}

void XclImpNameManager::ReadName( XclImpStream& rStrm )
{
    sal_uLong nCount = maNameList.size();
    if( nCount < 0xFFFF )
        maNameList.push_back( o3tl::make_unique<XclImpName>( rStrm, static_cast< sal_uInt16 >( nCount + 1 ) ) );
}

const XclImpName* XclImpNameManager::FindName( const OUString& rXclName, SCTAB nScTab ) const
{
    const XclImpName* pGlobalName = nullptr;   // a found global name
    const XclImpName* pLocalName = nullptr;    // a found local name
    for( XclImpNameList::const_iterator itName = maNameList.begin(); itName != maNameList.end() && !pLocalName; ++itName )
    {
        if( (*itName)->GetXclName() == rXclName )
        {
            if( (*itName)->GetScTab() == nScTab )
                pLocalName = itName->get();
            else if( (*itName)->IsGlobal() )
                pGlobalName = itName->get();
        }
    }
    return pLocalName ? pLocalName : pGlobalName;
}

const XclImpName* XclImpNameManager::GetName( sal_uInt16 nXclNameIdx ) const
{
    OSL_ENSURE( nXclNameIdx > 0, "XclImpNameManager::GetName - index must be >0" );
    return ( nXclNameIdx <= 0 ||  nXclNameIdx > maNameList.size() ) ? nullptr : maNameList.at( nXclNameIdx - 1 ).get();
}

void XclImpNameManager::ConvertAllTokens()
{
    XclImpNameList::iterator it = maNameList.begin(), itEnd = maNameList.end();
    for (; it != itEnd; ++it)
        (*it)->ConvertTokens();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
