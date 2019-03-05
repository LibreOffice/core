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
    , nCurScript(i18n::ScriptType::WEAK)
    , bForward(bFrwrd)
{
    assert(g_pBreakIt && g_pBreakIt->GetBreakIter().is());
    if ( ! bFrwrd && nStt )
        --nStt;

    sal_Int32 nPos = nStt;
    nCurScript = g_pBreakIt->GetBreakIter()->getScriptType(m_rText, nPos);
    if( i18n::ScriptType::WEAK == nCurScript )
    {
        if( nPos )
        {
            nPos = g_pBreakIt->GetBreakIter()->beginOfScript(
                                            m_rText, nPos, nCurScript);
            if (nPos > 0 && nPos < m_rText.getLength())
            {
                nStt = --nPos;
                nCurScript =
                    g_pBreakIt->GetBreakIter()->getScriptType(m_rText,nPos);
            }
        }
    }

    m_nChgPos = bForward
        ?  g_pBreakIt->GetBreakIter()->endOfScript(
                m_rText, nStt, nCurScript)
        :  g_pBreakIt->GetBreakIter()->beginOfScript(
                m_rText, nStt, nCurScript);
}

void SwScriptIterator::Next()
{
    assert(g_pBreakIt && g_pBreakIt->GetBreakIter().is());
    if (bForward && m_nChgPos >= 0 && m_nChgPos < m_rText.getLength())
    {
        nCurScript =
            g_pBreakIt->GetBreakIter()->getScriptType(m_rText, m_nChgPos);
        m_nChgPos = g_pBreakIt->GetBreakIter()->endOfScript(
                                        m_rText, m_nChgPos, nCurScript);
    }
    else if (!bForward && m_nChgPos > 0)
    {
        --m_nChgPos;
        nCurScript =
            g_pBreakIt->GetBreakIter()->getScriptType(m_rText, m_nChgPos);
        m_nChgPos = g_pBreakIt->GetBreakIter()->beginOfScript(
                                            m_rText, m_nChgPos, nCurScript);
    }
}

SwLanguageIterator::SwLanguageIterator( const SwTextNode& rTNd,
                                        sal_Int32 nStt )
    : aSIter( rTNd.GetText(), nStt ),
      rTextNd( rTNd ),
      pParaItem( nullptr ),
      nAttrPos( 0 ),
      nChgPos( nStt )
{
    SearchNextChg();
}

bool SwLanguageIterator::Next()
{
    bool bRet = false;
    if (nChgPos < aSIter.GetText().getLength())
    {
        bRet = true;
        if( !aStack.empty() )
        {
            do {
                const SwTextAttr* pHt = aStack.front();
                const sal_Int32 nEndPos = *pHt->End();
                if( nChgPos >= nEndPos )
                    aStack.pop_front();
                else
                    break;
            } while( !aStack.empty() );
        }

        if( !aStack.empty() )
        {
            const size_t nSavePos = nAttrPos;
            SearchNextChg();
            if( !aStack.empty() )
            {
                const SwTextAttr* pHt = aStack.front();
                const sal_Int32 nEndPos = *pHt->End();
                if( nChgPos >= nEndPos )
                {
                    nChgPos = nEndPos;
                    nAttrPos = nSavePos;

                    if( RES_TXTATR_CHARFMT == pHt->Which() )
                    {
                        const sal_uInt16 nWId = GetWhichOfScript( RES_CHRATR_LANGUAGE, aSIter.GetCurrScript() );
                        pCurItem = &pHt->GetCharFormat().GetCharFormat()->GetFormatAttr(nWId);
                    }
                    else
                        pCurItem = &pHt->GetAttr();

                    aStack.pop_front();
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
    for( ; nIns < aStack.size(); ++nIns )
        if( *aStack[ nIns ]->End() > nEndPos )
            break;

    aStack.insert( aStack.begin() + nIns, &rAttr );
}

void SwLanguageIterator::SearchNextChg()
{
    sal_uInt16 nWh = 0;
    if( nChgPos == aSIter.GetScriptChgPos() )
    {
        aSIter.Next();
        pParaItem = nullptr;
        nAttrPos = 0;       // must be restart at the beginning, because
                            // some attributes can start before or inside
                            // the current scripttype!
        aStack.clear();
    }
    if( !pParaItem )
    {
        nWh = GetWhichOfScript( RES_CHRATR_LANGUAGE, aSIter.GetCurrScript() );
        pParaItem = &rTextNd.GetSwAttrSet().Get( nWh );
    }

    sal_Int32 nStt = nChgPos;
    nChgPos = aSIter.GetScriptChgPos();
    pCurItem = pParaItem;

    const SwpHints* pHts = rTextNd.GetpSwpHints();
    if( pHts )
    {
        if( !nWh )
        {
            nWh = GetWhichOfScript( RES_CHRATR_LANGUAGE, aSIter.GetCurrScript() );
        }

        const SfxPoolItem* pItem = nullptr;
        for( ; nAttrPos < pHts->Count(); ++nAttrPos )
        {
            const SwTextAttr* pHt = pHts->Get( nAttrPos );
            const sal_Int32* pEnd = pHt->End();
            const sal_Int32 nHtStt = pHt->GetStart();
            if( nHtStt < nStt && ( !pEnd || *pEnd <= nStt ))
                continue;

            if( nHtStt >= nChgPos )
                break;

            pItem = CharFormat::GetItem( *pHt, nWh );
            if ( pItem )
            {
                if( nHtStt > nStt )
                {
                    if( nChgPos > nHtStt )
                        nChgPos = nHtStt;
                    break;
                }
                AddToStack( *pHt );
                pCurItem = pItem;
                if( *pEnd < nChgPos )
                    nChgPos = *pEnd;
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
