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

#include <memory>
#include <uinums.hxx>

#include <unotools/pathoptions.hxx>
#include <tools/stream.hxx>
#include <sfx2/docfile.hxx>
#include <svl/itemiter.hxx>

#include <swtypes.hxx>
#include <utility>
#include <wrtsh.hxx>
#include <poolfmt.hxx>
#include <charfmt.hxx>

using namespace ::com::sun::star;

constexpr OUString CHAPTER_FILENAME = u"chapter.cfg"_ustr;

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

    SfxMedium aMedium( aURL.GetMainURL(INetURLObject::DecodeMechanism::NONE), StreamMode::WRITE );
    SvStream* pStream = aMedium.GetOutStream();
    bool bRet = (pStream && pStream->GetError() == ERRCODE_NONE);
    if (bRet)
    {
        sw::ExportStoredChapterNumberingRules(*this, *pStream,CHAPTER_FILENAME);

        pStream->FlushBuffer();

        aMedium.Commit();
    }
}

SwChapterNumRules::~SwChapterNumRules()
{
}

void  SwChapterNumRules::Init()
{
    for(auto & rpNumRule : m_pNumRules)
        rpNumRule.reset();

    OUString sNm(CHAPTER_FILENAME);
    SvtPathOptions aOpt;
    if( aOpt.SearchFile( sNm ))
    {
        SfxMedium aStrm( sNm, StreamMode::STD_READ );
        sw::ImportStoredChapterNumberingRules(*this, *aStrm.GetInStream(),
                CHAPTER_FILENAME);
    }
}

void SwChapterNumRules::CreateEmptyNumRule(sal_uInt16 const nIndex)
{
    assert(nIndex < nMaxRules);
    assert(!m_pNumRules[nIndex]);
    m_pNumRules[nIndex].reset(new SwNumRulesWithName);
}

void SwChapterNumRules::ApplyNumRules(const SwNumRulesWithName &rCopy, sal_uInt16 nIdx)
{
    assert(nIdx < nMaxRules);
    if( !m_pNumRules[nIdx] )
        m_pNumRules[nIdx].reset(new SwNumRulesWithName( rCopy ));
    else
        *m_pNumRules[nIdx] = rCopy;
    Save(); // store it immediately
}

SwNumRulesWithName::SwNumRulesWithName( const SwNumRule &rCopy,
                                        OUString aName )
    : maName(std::move(aName))
{
    for( sal_uInt16 n = 0; n < MAXLEVEL; ++n )
    {
        const SwNumFormat* pFormat = rCopy.GetNumFormat( n );
        if( pFormat )
            m_aFormats[ n ].reset(new SwNumFormatGlobal( *pFormat ));
        else
            m_aFormats[ n ].reset();
    }
}

SwNumRulesWithName::SwNumRulesWithName( const SwNumRulesWithName& rCopy )
{
    *this = rCopy;
}

SwNumRulesWithName::~SwNumRulesWithName()
{
}

SwNumRulesWithName& SwNumRulesWithName::operator=(const SwNumRulesWithName &rCopy)
{
    if( this != &rCopy )
    {
        maName = rCopy.maName;
        for( int n = 0; n < MAXLEVEL; ++n )
        {
            SwNumFormatGlobal* pFormat = rCopy.m_aFormats[ n ].get();
            if( pFormat )
                m_aFormats[ n ].reset(new SwNumFormatGlobal( *pFormat ));
            else
                m_aFormats[ n ].reset();
        }
    }
    return *this;
}

void SwNumRulesWithName::ResetNumRule(SwWrtShell& rSh, SwNumRule& rNumRule) const
{
    // #i89178#
    rNumRule.Reset(maName);
    rNumRule.SetAutoRule( false );
    for (sal_uInt16 n = 0; n < MAXLEVEL; ++n)
    {
        SwNumFormatGlobal* pFormat = m_aFormats[ n ].get();
        if (!pFormat)
            continue;
        rNumRule.Set(n, pFormat->MakeNumFormat(rSh));
    }
}

void SwNumRulesWithName::GetNumFormat(
    size_t const nIndex, SwNumFormat const*& rpNumFormat, UIName const*& rpName) const
{
    rpNumFormat = (m_aFormats[nIndex]) ? &m_aFormats[nIndex]->m_aFormat : nullptr;
    rpName = (m_aFormats[nIndex]) ? &m_aFormats[nIndex]->m_sCharFormatName : nullptr;
}

void SwNumRulesWithName::SetNumFormat(
        size_t const nIndex, SwNumFormat const& rNumFormat, UIName const& rName)
{
    m_aFormats[nIndex].reset( new SwNumFormatGlobal(rNumFormat) );
    m_aFormats[nIndex]->m_sCharFormatName = rName;
    m_aFormats[nIndex]->m_nCharPoolId = USHRT_MAX;
    m_aFormats[nIndex]->m_Items.clear();
}

SwNumRulesWithName::SwNumFormatGlobal::SwNumFormatGlobal( const SwNumFormat& rFormat )
    : m_aFormat( rFormat ), m_nCharPoolId( USHRT_MAX )
{
    // relative gaps?????

    SwCharFormat* pFormat = rFormat.GetCharFormat();
    if( !pFormat )
        return;

    m_sCharFormatName = pFormat->GetName();
    m_nCharPoolId = pFormat->GetPoolFormatId();
    if( pFormat->GetAttrSet().Count() )
    {
        SfxItemIter aIter( pFormat->GetAttrSet() );
        const SfxPoolItem *pCurr = aIter.GetCurItem();
        do
        {
            m_Items.push_back(std::unique_ptr<SfxPoolItem>(pCurr->Clone()));
            pCurr = aIter.NextItem();
        } while (pCurr);
    }

    m_aFormat.SetCharFormat( nullptr );
}

SwNumRulesWithName::SwNumFormatGlobal::SwNumFormatGlobal( const SwNumFormatGlobal& rFormat )
    :
    m_aFormat( rFormat.m_aFormat ),
    m_sCharFormatName( rFormat.m_sCharFormatName ),
    m_nCharPoolId( rFormat.m_nCharPoolId )
{
    for (size_t n = rFormat.m_Items.size(); n; )
    {
        m_Items.push_back(std::unique_ptr<SfxPoolItem>(rFormat.m_Items[ --n ]->Clone()));
    }
}

SwNumRulesWithName::SwNumFormatGlobal::~SwNumFormatGlobal()
{
}

SwNumFormat SwNumRulesWithName::SwNumFormatGlobal::MakeNumFormat(SwWrtShell& rSh) const
{
    SwCharFormat* pFormat = nullptr;
    if( !m_sCharFormatName.isEmpty() )
    {
        // at first, look for the name
        sal_uInt16 nArrLen = rSh.GetCharFormatCount();
        for( sal_uInt16 i = 1; i < nArrLen; ++i )
        {
            pFormat = &rSh.GetCharFormat( i );
            if (pFormat->GetName()==m_sCharFormatName)
                // exists, so leave attributes as they are!
                break;
            pFormat = nullptr;
        }

        if( !pFormat )
        {
            if( IsPoolUserFormat( m_nCharPoolId ) )
            {
                pFormat = rSh.MakeCharFormat( m_sCharFormatName );
                pFormat->SetAuto(false);
            }
            else
                pFormat = rSh.GetCharFormatFromPool( m_nCharPoolId );

            if( !pFormat->HasWriterListeners() )       // set attributes
            {
                for (size_t n = m_Items.size(); n; )
                {
                    pFormat->SetFormatAttr( *m_Items[ --n ] );
                }
            }
        }
    }
    const_cast<SwNumFormat&>(m_aFormat).SetCharFormat(pFormat);
    SwNumFormat aNew = m_aFormat;
    if (pFormat)
        const_cast<SwNumFormat&>(m_aFormat).SetCharFormat(nullptr);
    return aNew;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
