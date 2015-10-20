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
    if( aOpt.SearchFile( sNm ))
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
        const SwNumFormat* pFormat = rCopy.GetNumFormat( n );
        if( pFormat )
            aFormats[ n ] = new _SwNumFormatGlobal( *pFormat );
        else
            aFormats[ n ] = 0;
    }
}

SwNumRulesWithName::SwNumRulesWithName()
{
    memset(aFormats, 0, sizeof(aFormats));
}

SwNumRulesWithName::SwNumRulesWithName( const SwNumRulesWithName& rCopy )
{
    memset( aFormats, 0, sizeof( aFormats ));
    *this = rCopy;
}

SwNumRulesWithName::~SwNumRulesWithName()
{
    for( int n = 0; n < MAXLEVEL; ++n )
        delete aFormats[ n ];
}

const SwNumRulesWithName& SwNumRulesWithName::operator=(const SwNumRulesWithName &rCopy)
{
    if( this != &rCopy )
    {
        maName = rCopy.maName;
        for( int n = 0; n < MAXLEVEL; ++n )
        {
            delete aFormats[ n ];

            _SwNumFormatGlobal* pFormat = rCopy.aFormats[ n ];
            if( pFormat )
                aFormats[ n ] = new _SwNumFormatGlobal( *pFormat );
            else
                aFormats[ n ] = 0;
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
        _SwNumFormatGlobal* pFormat = aFormats[ n ];
        if( 0 != pFormat)
        {
            SwNumFormat aNew;
            pFormat->ChgNumFormat( rSh, aNew );
            rChg.Set( n, aNew );
        }
    }
}

void SwNumRulesWithName::GetNumFormat(
    size_t const nIndex, SwNumFormat const*& rpNumFormat, OUString const*& rpName) const
{
    rpNumFormat = (aFormats[nIndex]) ? &aFormats[nIndex]->aFormat : 0;
    rpName = (aFormats[nIndex]) ? &aFormats[nIndex]->sCharFormatName : 0;
}

void SwNumRulesWithName::SetNumFormat(
        size_t const nIndex, SwNumFormat const& rNumFormat, OUString const& rName)
{
    delete aFormats[nIndex];
    aFormats[nIndex] = new _SwNumFormatGlobal(rNumFormat);
    aFormats[nIndex]->sCharFormatName = rName;
    aFormats[nIndex]->nCharPoolId = USHRT_MAX;
    aFormats[nIndex]->m_Items.clear();
}

SwNumRulesWithName::_SwNumFormatGlobal::_SwNumFormatGlobal( const SwNumFormat& rFormat )
    : aFormat( rFormat ), nCharPoolId( USHRT_MAX )
{
    // relative gaps?????

    SwCharFormat* pFormat = rFormat.GetCharFormat();
    if( pFormat )
    {
        sCharFormatName = pFormat->GetName();
        nCharPoolId = pFormat->GetPoolFormatId();
        if( pFormat->GetAttrSet().Count() )
        {
            SfxItemIter aIter( pFormat->GetAttrSet() );
            const SfxPoolItem *pCurr = aIter.GetCurItem();
            while( true )
            {
                m_Items.push_back(std::unique_ptr<SfxPoolItem>(pCurr->Clone()));
                if( aIter.IsAtEnd() )
                    break;
                pCurr = aIter.NextItem();
            }
        }

        aFormat.SetCharFormat( 0 );
    }
}

SwNumRulesWithName::_SwNumFormatGlobal::_SwNumFormatGlobal( const _SwNumFormatGlobal& rFormat )
    :
    aFormat( rFormat.aFormat ),
    sCharFormatName( rFormat.sCharFormatName ),
    nCharPoolId( rFormat.nCharPoolId )
{
    for (size_t n = rFormat.m_Items.size(); n; )
    {
        m_Items.push_back(std::unique_ptr<SfxPoolItem>(rFormat.m_Items[ --n ]->Clone()));
    }
}

SwNumRulesWithName::_SwNumFormatGlobal::~_SwNumFormatGlobal()
{
}

void SwNumRulesWithName::_SwNumFormatGlobal::ChgNumFormat( SwWrtShell& rSh,
                            SwNumFormat& rNew ) const
{
    SwCharFormat* pFormat = 0;
    if( !sCharFormatName.isEmpty() )
    {
        // at first, look for the name
        sal_uInt16 nArrLen = rSh.GetCharFormatCount();
        for( sal_uInt16 i = 1; i < nArrLen; ++i )
        {
            pFormat = &rSh.GetCharFormat( i );
            if (pFormat->GetName()==sCharFormatName)
                // exists, so leave attributes as they are!
                break;
            pFormat = 0;
        }

        if( !pFormat )
        {
            if( IsPoolUserFormat( nCharPoolId ) )
            {
                pFormat = rSh.MakeCharFormat( sCharFormatName );
                pFormat->SetAuto(false);
            }
            else
                pFormat = rSh.GetCharFormatFromPool( nCharPoolId );

            if( !pFormat->HasWriterListeners() )       // set attributes
            {
                for (size_t n = m_Items.size(); n; )
                {
                    pFormat->SetFormatAttr( *m_Items[ --n ] );
                }
            }
        }
    }
    const_cast<SwNumFormat&>(aFormat).SetCharFormat( pFormat );
    rNew = aFormat;
    if( pFormat )
        const_cast<SwNumFormat&>(aFormat).SetCharFormat( 0 );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
