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

#include <hintids.hxx>

#include <com/sun/star/i18n/ScriptType.hpp>
#include <tools/string.hxx>
#include <editeng/langitem.hxx>
#include <txatritr.hxx>
#include <fchrfmt.hxx>
#include <charfmt.hxx>
#include <breakit.hxx>
#include <ndtxt.hxx>
#include <txatbase.hxx>

using namespace ::com::sun::star::i18n;


SwScriptIterator::SwScriptIterator( const String& rStr, xub_StrLen nStt, bool bFrwrd )
    : rText( rStr ),
      nChgPos( rStr.Len() ),
      nCurScript( ScriptType::WEAK ),
      bForward( bFrwrd )
{
    if( pBreakIt->GetBreakIter().is() )
    {
        if ( ! bFrwrd && nStt )
            --nStt;

        xub_StrLen nPos = nStt;
        nCurScript = pBreakIt->GetBreakIter()->getScriptType( rText, nPos );
        if( ScriptType::WEAK == nCurScript )
        {
            if( nPos )
            {
                nPos = (xub_StrLen)pBreakIt->GetBreakIter()->beginOfScript(
                                                rText, nPos, nCurScript );
                if( nPos && nPos < rText.Len() )
                {
                    nStt = --nPos;
                    nCurScript = pBreakIt->GetBreakIter()->getScriptType( rText,nPos);
                }
            }
        }

        nChgPos = bForward ?
                  (xub_StrLen)pBreakIt->GetBreakIter()->endOfScript( rText, nStt, nCurScript ) :
                  (xub_StrLen)pBreakIt->GetBreakIter()->beginOfScript( rText, nStt, nCurScript );
    }
}

bool SwScriptIterator::Next()
{
    bool bRet = false;
    if( pBreakIt->GetBreakIter().is() )
    {
        if ( bForward && nChgPos < rText.Len() )
        {
            nCurScript = pBreakIt->GetBreakIter()->getScriptType( rText, nChgPos );
            nChgPos = (xub_StrLen)pBreakIt->GetBreakIter()->endOfScript(
                                                rText, nChgPos, nCurScript );
            bRet = true;
        }
        else if ( ! bForward && nChgPos )
        {
            --nChgPos;
            nCurScript = pBreakIt->GetBreakIter()->getScriptType( rText, nChgPos );
            nChgPos = (xub_StrLen)pBreakIt->GetBreakIter()->beginOfScript(
                                                rText, nChgPos, nCurScript );
            bRet = true;
        }
    }
    else
        nChgPos = rText.Len();
    return bRet;
}

// --------------------------------------------------------------------

SwTxtAttrIterator::SwTxtAttrIterator( const SwTxtNode& rTNd, sal_uInt16 nWhchId,
                                        xub_StrLen nStt,
                                        bool bUseGetWhichOfScript )
    : aSIter( rTNd.GetTxt(), nStt ), rTxtNd( rTNd ),
    pParaItem( 0 ), nChgPos( nStt ), nAttrPos( 0 ), nWhichId( nWhchId ),
    bIsUseGetWhichOfScript( bUseGetWhichOfScript )
{
    SearchNextChg();
}

bool SwTxtAttrIterator::Next()
{
    bool bRet = false;
    if( nChgPos < aSIter.GetText().Len() )
    {
        bRet = true;
        if( !aStack.empty() )
        {
            do {
                const SwTxtAttr* pHt = aStack.front();
                sal_uInt16 nEndPos = *pHt->GetEnd();
                if( nChgPos >= nEndPos )
                    aStack.pop_front();
                else
                    break;
            } while( !aStack.empty() );
        }

        if( !aStack.empty() )
        {
            sal_uInt16 nSavePos = nAttrPos;
            SearchNextChg();
            if( !aStack.empty() )
            {
                const SwTxtAttr* pHt = aStack.front();
                sal_uInt16 nEndPos = *pHt->GetEnd();
                if( nChgPos >= nEndPos )
                {
                    nChgPos = nEndPos;
                    nAttrPos = nSavePos;

                    if( RES_TXTATR_CHARFMT == pHt->Which() )
                    {
                        sal_uInt16 nWId = bIsUseGetWhichOfScript ?
                                GetWhichOfScript( nWhichId,
                                                  aSIter.GetCurrScript() ) : nWhichId;
                        pCurItem = &pHt->GetCharFmt().GetCharFmt()->GetFmtAttr(nWId);
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

void SwTxtAttrIterator::AddToStack( const SwTxtAttr& rAttr )
{
    sal_uInt16 nIns = 0, nEndPos = *rAttr.GetEnd();
    for( ; nIns < aStack.size(); ++nIns )
        if( *aStack[ nIns ]->GetEnd() > nEndPos )
            break;

    aStack.insert( aStack.begin() + nIns, &rAttr );
}

void SwTxtAttrIterator::SearchNextChg()
{
    sal_uInt16 nWh = 0;
    if( nChgPos == aSIter.GetScriptChgPos() )
    {
        aSIter.Next();
        pParaItem = 0;
        nAttrPos = 0;       // must be restart at the beginning, because
                            // some attributes can start before or inside
                            // the current scripttype!
        aStack.clear();
    }
    if( !pParaItem )
    {
        nWh = bIsUseGetWhichOfScript ?
                GetWhichOfScript( nWhichId,
                                  aSIter.GetCurrScript() ) : nWhichId;
        pParaItem = &rTxtNd.GetSwAttrSet().Get( nWh );
    }

    xub_StrLen nStt = nChgPos;
    nChgPos = aSIter.GetScriptChgPos();
    pCurItem = pParaItem;

    const SwpHints* pHts = rTxtNd.GetpSwpHints();
    if( pHts )
    {
        if( !nWh )
        {
            nWh =  bIsUseGetWhichOfScript ?
                        GetWhichOfScript( nWhichId,
                                          aSIter.GetCurrScript() ) : nWhichId;
        }

        const SfxPoolItem* pItem = 0;
        for( ; nAttrPos < pHts->Count(); ++nAttrPos )
        {
            const SwTxtAttr* pHt = (*pHts)[ nAttrPos ];
            const sal_uInt16* pEnd = pHt->GetEnd();
            const sal_uInt16 nHtStt = *pHt->GetStart();
            if( nHtStt < nStt && ( !pEnd || *pEnd <= nStt ))
                continue;

            if( nHtStt >= nChgPos )
                break;

            pItem = CharFmt::GetItem( *pHt, nWh );
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
