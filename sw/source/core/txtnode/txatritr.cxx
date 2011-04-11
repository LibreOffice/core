/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <hintids.hxx>

#include <com/sun/star/i18n/ScriptType.hdl>
#include <tools/string.hxx>
#include <editeng/langitem.hxx>
#include <txatritr.hxx>
#include <fchrfmt.hxx>
#include <charfmt.hxx>
#include <breakit.hxx>
#include <ndtxt.hxx>
#include <txatbase.hxx>

using namespace ::com::sun::star::i18n;


SwScriptIterator::SwScriptIterator( const String& rStr, xub_StrLen nStt, sal_Bool bFrwrd )
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

sal_Bool SwScriptIterator::Next()
{
    sal_Bool bRet = sal_False;
    if( pBreakIt->GetBreakIter().is() )
    {
        if ( bForward && nChgPos < rText.Len() )
        {
            nCurScript = pBreakIt->GetBreakIter()->getScriptType( rText, nChgPos );
            nChgPos = (xub_StrLen)pBreakIt->GetBreakIter()->endOfScript(
                                                rText, nChgPos, nCurScript );
            bRet = sal_True;
        }
        else if ( ! bForward && nChgPos )
        {
            --nChgPos;
            nCurScript = pBreakIt->GetBreakIter()->getScriptType( rText, nChgPos );
            nChgPos = (xub_StrLen)pBreakIt->GetBreakIter()->beginOfScript(
                                                rText, nChgPos, nCurScript );
            bRet = sal_True;
        }
    }
    else
        nChgPos = rText.Len();
    return bRet;
}

// --------------------------------------------------------------------

SwTxtAttrIterator::SwTxtAttrIterator( const SwTxtNode& rTNd, sal_uInt16 nWhchId,
                                        xub_StrLen nStt,
                                        sal_Bool bUseGetWhichOfScript )
    : aSIter( rTNd.GetTxt(), nStt ), rTxtNd( rTNd ),
    pParaItem( 0 ), nChgPos( nStt ), nAttrPos( 0 ), nWhichId( nWhchId ),
    bIsUseGetWhichOfScript( bUseGetWhichOfScript )
{
    SearchNextChg();
}

sal_Bool SwTxtAttrIterator::Next()
{
    sal_Bool bRet = sal_False;
    if( nChgPos < aSIter.GetText().Len() )
    {
        bRet = sal_True;
        if( aStack.Count() )
        {
            do {
                const SwTxtAttr* pHt = (SwTxtAttr*)aStack[ 0 ];
                sal_uInt16 nEndPos = *pHt->GetEnd();
                if( nChgPos >= nEndPos )
                    aStack.Remove( 0 );
                else
                    break;
            } while( aStack.Count() );
        }

        if( aStack.Count() )
        {
            sal_uInt16 nSavePos = nAttrPos;
            SearchNextChg();
            if( aStack.Count() )
            {
                const SwTxtAttr* pHt = (SwTxtAttr*)aStack[ 0 ];
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

                    aStack.Remove( 0 );
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
    void* pAdd = (void*)&rAttr;
    sal_uInt16 nIns = 0, nEndPos = *rAttr.GetEnd();
    for( ; nIns < aStack.Count(); ++nIns )
        if( *((SwTxtAttr*)aStack[ nIns ] )->GetEnd() > nEndPos )
            break;

    aStack.Insert( pAdd, nIns );
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
        aStack.Remove( 0, aStack.Count() );
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
