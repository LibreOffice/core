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

#include <uinums.hxx>

#include <hintids.hxx>
#include <svl/urihelper.hxx>
#include <unotools/pathoptions.hxx>
#include <tools/stream.hxx>
#include <sfx2/docfile.hxx>
#include <svl/itemiter.hxx>

#include <tools/resid.hxx>
#include <swtypes.hxx>
#include <wrtsh.hxx>
#include <poolfmt.hxx>
#include <charfmt.hxx>

#include <unomid.h>

using namespace ::com::sun::star;

#define CHAPTER_FILENAME "chapter.cfg"

/*
 Description:   Saving a rule
 Parameter:     rCopy -- the rule to save
                    nIdx -- position, where the rule is to be saved.
                        An old rule at that position will be overwritten.
*/

SwChapterNumRules::SwChapterNumRules()
{
    Init();
}

void SwChapterNumRules::Save()
{
    INetURLObject aURL;
    SvtPathOptions aPathOpt;
    aURL.SetSmartURL( aPathOpt.GetUserConfigPath() );
    aURL.setFinalSlash();
    aURL.Append(CHAPTER_FILENAME);

    SfxMedium aMedium( aURL.GetMainURL(INetURLObject::NO_DECODE), StreamMode::WRITE );
    SvStream* pStream = aMedium.GetOutStream();
    bool bRet = (pStream && pStream->GetError() == 0);
    if (bRet)
    {
        sw::ExportStoredChapterNumberingRules(*this, *pStream,CHAPTER_FILENAME);

        pStream->Flush();

        aMedium.Commit();
    }
}

SwChapterNumRules::~SwChapterNumRules()
{
    for( sal_uInt16 i = 0; i < nMaxRules; ++i )
        delete pNumRules[i];
}

void  SwChapterNumRules::Init()
{
    for(sal_uInt16 i = 0; i < nMaxRules; ++i )
        pNumRules[i] = 0;

    OUString sNm(CHAPTER_FILENAME);
    SvtPathOptions aOpt;
    if( aOpt.SearchFile( sNm, SvtPathOptions::PATH_USERCONFIG ))
    {
        SfxMedium aStrm( sNm, STREAM_STD_READ );
        sw::ImportStoredChapterNumberingRules(*this, *aStrm.GetInStream(),
                CHAPTER_FILENAME);
    }
}

void SwChapterNumRules::CreateEmptyNumRule(sal_uInt16 const nIndex)
{
    assert(nIndex < nMaxRules);
    assert(!pNumRules[nIndex]);
    pNumRules[nIndex] = new SwNumRulesWithName;
}

void SwChapterNumRules::ApplyNumRules(const SwNumRulesWithName &rCopy, sal_uInt16 nIdx)
{
    assert(nIdx < nMaxRules);
    if( !pNumRules[nIdx] )
        pNumRules[nIdx] = new SwNumRulesWithName( rCopy );
    else
        *pNumRules[nIdx] = rCopy;
    Save(); // store it immediately
}

SwNumRulesWithName::SwNumRulesWithName( const SwNumRule &rCopy,
                                        const OUString &rName )
    : maName(rName)
{
    for( sal_uInt16 n = 0; n < MAXLEVEL; ++n )
    {
        const SwNumFmt* pFmt = rCopy.GetNumFmt( n );
        if( pFmt )
            aFmts[ n ] = new _SwNumFmtGlobal( *pFmt );
        else
            aFmts[ n ] = 0;
    }
}

SwNumRulesWithName::SwNumRulesWithName()
{
    memset(aFmts, 0, sizeof(aFmts));
}

SwNumRulesWithName::SwNumRulesWithName( const SwNumRulesWithName& rCopy )
{
    memset( aFmts, 0, sizeof( aFmts ));
    *this = rCopy;
}

SwNumRulesWithName::~SwNumRulesWithName()
{
    for( int n = 0; n < MAXLEVEL; ++n )
        delete aFmts[ n ];
}

const SwNumRulesWithName& SwNumRulesWithName::operator=(const SwNumRulesWithName &rCopy)
{
    if( this != &rCopy )
    {
        maName = rCopy.maName;
        for( int n = 0; n < MAXLEVEL; ++n )
        {
            delete aFmts[ n ];

            _SwNumFmtGlobal* pFmt = rCopy.aFmts[ n ];
            if( pFmt )
                aFmts[ n ] = new _SwNumFmtGlobal( *pFmt );
            else
                aFmts[ n ] = 0;
        }
    }
    return *this;
}

void SwNumRulesWithName::MakeNumRule( SwWrtShell& rSh, SwNumRule& rChg ) const
{
    // #i89178#
    rChg = SwNumRule( maName, numfunc::GetDefaultPositionAndSpaceMode() );
    rChg.SetAutoRule( false );
    for( sal_uInt16 n = 0; n < MAXLEVEL; ++n )
    {
        _SwNumFmtGlobal* pFmt = aFmts[ n ];
        if( 0 != pFmt)
        {
            SwNumFmt aNew;
            pFmt->ChgNumFmt( rSh, aNew );
            rChg.Set( n, aNew );
        }
    }
}

void SwNumRulesWithName::GetNumFmt(
    size_t const nIndex, SwNumFmt const*& rpNumFmt, OUString const*& rpName) const
{
    rpNumFmt = (aFmts[nIndex]) ? &aFmts[nIndex]->aFmt : 0;
    rpName = (aFmts[nIndex]) ? &aFmts[nIndex]->sCharFmtName : 0;
}

void SwNumRulesWithName::SetNumFmt(
        size_t const nIndex, SwNumFmt const& rNumFmt, OUString const& rName)
{
    delete aFmts[nIndex];
    aFmts[nIndex] = new _SwNumFmtGlobal(rNumFmt);
    aFmts[nIndex]->sCharFmtName = rName;
    aFmts[nIndex]->nCharPoolId = USHRT_MAX;
    aFmts[nIndex]->aItems.clear();
}

SwNumRulesWithName::_SwNumFmtGlobal::_SwNumFmtGlobal( const SwNumFmt& rFmt )
    : aFmt( rFmt ), nCharPoolId( USHRT_MAX )
{
    // relative gaps?????

    SwCharFmt* pFmt = rFmt.GetCharFmt();
    if( pFmt )
    {
        sCharFmtName = pFmt->GetName();
        nCharPoolId = pFmt->GetPoolFmtId();
        if( pFmt->GetAttrSet().Count() )
        {
            SfxItemIter aIter( pFmt->GetAttrSet() );
            const SfxPoolItem *pCurr = aIter.GetCurItem();
            while( true )
            {
                aItems.push_back( pCurr->Clone() );
                if( aIter.IsAtEnd() )
                    break;
                pCurr = aIter.NextItem();
            }
        }

        aFmt.SetCharFmt( 0 );
    }
}

SwNumRulesWithName::_SwNumFmtGlobal::_SwNumFmtGlobal( const _SwNumFmtGlobal& rFmt )
    :
    aFmt( rFmt.aFmt ),
    sCharFmtName( rFmt.sCharFmtName ),
    nCharPoolId( rFmt.nCharPoolId )
{
    for( sal_uInt16 n = rFmt.aItems.size(); n; )
        aItems.push_back( rFmt.aItems[ --n ].Clone() );
}

SwNumRulesWithName::_SwNumFmtGlobal::~_SwNumFmtGlobal()
{
}

void SwNumRulesWithName::_SwNumFmtGlobal::ChgNumFmt( SwWrtShell& rSh,
                            SwNumFmt& rNew ) const
{
    SwCharFmt* pFmt = 0;
    if( !sCharFmtName.isEmpty() )
    {
        // at first, look for the name
        sal_uInt16 nArrLen = rSh.GetCharFmtCount();
        for( sal_uInt16 i = 1; i < nArrLen; ++i )
        {
            pFmt = &rSh.GetCharFmt( i );
            if (pFmt->GetName()==sCharFmtName)
                // exists, so leave attributes as they are!
                break;
            pFmt = 0;
        }

        if( !pFmt )
        {
            if( IsPoolUserFmt( nCharPoolId ) )
            {
                pFmt = rSh.MakeCharFmt( sCharFmtName );
                pFmt->SetAuto( false );
            }
            else
                pFmt = rSh.GetCharFmtFromPool( nCharPoolId );

            if( !pFmt->GetDepends() )       // set attributes
                for( sal_uInt16 n = aItems.size(); n; )
                    pFmt->SetFmtAttr( aItems[ --n ] );
        }
    }
    const_cast<SwNumFmt&>(aFmt).SetCharFmt( pFmt );
    rNew = aFmt;
    if( pFmt )
        const_cast<SwNumFmt&>(aFmt).SetCharFmt( 0 );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
