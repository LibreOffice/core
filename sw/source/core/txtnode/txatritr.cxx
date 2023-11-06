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

#include <txatritr.hxx>

#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <fchrfmt.hxx>
#include <charfmt.hxx>
#include <breakit.hxx>
#include <ndtxt.hxx>
#include <txatbase.hxx>

using namespace ::com::sun::star;

SwScriptIterator::SwScriptIterator(
            const OUString& rStr, sal_Int32 nStt, bool const bFrwrd)
    : m_rText(rStr)
    , m_nChgPos(rStr.getLength())
    , m_nCurScript(i18n::ScriptType::WEAK)
    , m_bForward(bFrwrd)
{
    assert(g_pBreakIt && g_pBreakIt->GetBreakIter().is());
    if ( ! bFrwrd && nStt )
        --nStt;

    sal_Int32 nPos = nStt;
    m_nCurScript = g_pBreakIt->GetBreakIter()->getScriptType(m_rText, nPos);
    if( i18n::ScriptType::WEAK == m_nCurScript )
    {
        if( nPos )
        {
            nPos = g_pBreakIt->GetBreakIter()->beginOfScript(
                                            m_rText, nPos, m_nCurScript);
            if (nPos > 0 && nPos < m_rText.getLength())
            {
                nStt = --nPos;
                m_nCurScript =
                    g_pBreakIt->GetBreakIter()->getScriptType(m_rText,nPos);
            }
        }
    }

    m_nChgPos = m_bForward
        ?  g_pBreakIt->GetBreakIter()->endOfScript(
                m_rText, nStt, m_nCurScript)
        :  g_pBreakIt->GetBreakIter()->beginOfScript(
                m_rText, nStt, m_nCurScript);
}

void SwScriptIterator::Next()
{
    assert(g_pBreakIt && g_pBreakIt->GetBreakIter().is());
    if (m_bForward && m_nChgPos >= 0 && m_nChgPos < m_rText.getLength())
    {
        m_nCurScript =
            g_pBreakIt->GetBreakIter()->getScriptType(m_rText, m_nChgPos);
        m_nChgPos = g_pBreakIt->GetBreakIter()->endOfScript(
                                        m_rText, m_nChgPos, m_nCurScript);
    }
    else if (!m_bForward && m_nChgPos > 0)
    {
        --m_nChgPos;
        m_nCurScript =
            g_pBreakIt->GetBreakIter()->getScriptType(m_rText, m_nChgPos);
        m_nChgPos = g_pBreakIt->GetBreakIter()->beginOfScript(
                                            m_rText, m_nChgPos, m_nCurScript);
    }
}

SwLanguageIterator::SwLanguageIterator( const SwTextNode& rTNd,
                                        sal_Int32 nStt )
    : m_aScriptIter( rTNd.GetText(), nStt ),
      m_rTextNode( rTNd ),
      m_pParaItem( nullptr ),
      m_nAttrPos( 0 ),
      m_nChgPos( nStt )
{
    SearchNextChg();
}

bool SwLanguageIterator::Next()
{
    bool bRet = false;
    if (m_nChgPos < m_aScriptIter.GetText().getLength())
    {
        bRet = true;
        if( !m_aStack.empty() )
        {
            do {
                const SwTextAttr* pHt = m_aStack.front();
                const sal_Int32 nEndPos = *pHt->End();
                if( m_nChgPos >= nEndPos )
                    m_aStack.pop_front();
                else
                    break;
            } while( !m_aStack.empty() );
        }

        if( !m_aStack.empty() )
        {
            const size_t nSavePos = m_nAttrPos;
            SearchNextChg();
            if( !m_aStack.empty() )
            {
                const SwTextAttr* pHt = m_aStack.front();
                const sal_Int32 nEndPos = *pHt->End();
                if( m_nChgPos >= nEndPos )
                {
                    m_nChgPos = nEndPos;
                    m_nAttrPos = nSavePos;

                    const sal_uInt16 nWId = GetWhichOfScript( RES_CHRATR_LANGUAGE, m_aScriptIter.GetCurrScript() );
                    m_pCurrentItem = CharFormat::GetItem(*pHt, nWId);

                    m_aStack.pop_front();
                }
            }
        }
        else
            SearchNextChg();
    }
    return bRet;
}

void SwLanguageIterator::AddToStack( const SwTextAttr& rAttr )
{
    size_t nIns = 0;
    const sal_Int32 nEndPos = *rAttr.End();
    for( ; nIns < m_aStack.size(); ++nIns )
        if( *m_aStack[ nIns ]->End() > nEndPos )
            break;

    m_aStack.insert( m_aStack.begin() + nIns, &rAttr );
}

void SwLanguageIterator::SearchNextChg()
{
    sal_uInt16 nWh = 0;
    if( m_nChgPos == m_aScriptIter.GetScriptChgPos() )
    {
        m_aScriptIter.Next();
        m_pParaItem = nullptr;
        m_nAttrPos = 0;       // must be restart at the beginning, because
                            // some attributes can start before or inside
                            // the current scripttype!
        m_aStack.clear();
    }
    if( !m_pParaItem )
    {
        nWh = GetWhichOfScript( RES_CHRATR_LANGUAGE, m_aScriptIter.GetCurrScript() );
        m_pParaItem = &m_rTextNode.GetSwAttrSet().Get( nWh );
    }

    sal_Int32 nStt = m_nChgPos;
    m_nChgPos = m_aScriptIter.GetScriptChgPos();
    m_pCurrentItem = m_pParaItem;

    const SwpHints* pHts = m_rTextNode.GetpSwpHints();
    if( !pHts )
        return;

    if( !nWh )
    {
        nWh = GetWhichOfScript( RES_CHRATR_LANGUAGE, m_aScriptIter.GetCurrScript() );
    }

    const SfxPoolItem* pItem = nullptr;
    for( ; m_nAttrPos < pHts->Count(); ++m_nAttrPos )
    {
        const SwTextAttr* pHt = pHts->Get( m_nAttrPos );
        const sal_Int32* pEnd = pHt->End();
        const sal_Int32 nHtStt = pHt->GetStart();
        if( nHtStt < nStt && ( !pEnd || *pEnd <= nStt ))
            continue;

        if( nHtStt >= m_nChgPos )
            break;

        pItem = CharFormat::GetItem( *pHt, nWh );
        if ( pItem )
        {
            if( nHtStt > nStt )
            {
                if( m_nChgPos > nHtStt )
                    m_nChgPos = nHtStt;
                break;
            }
            AddToStack( *pHt );
            m_pCurrentItem = pItem;
            if( *pEnd < m_nChgPos )
                m_nChgPos = *pEnd;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
