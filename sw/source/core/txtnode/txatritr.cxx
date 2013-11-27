/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <hintids.hxx>

#ifndef _COM_SUN_STAR_I18N_SCRIPTTYPE_HDL_
#include <com/sun/star/i18n/ScriptType.hdl>
#endif
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
                const sal_uInt16 nEndPos = *pHt->End();
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
                const sal_uInt16 nEndPos = *pHt->End();
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
    sal_uInt16 nIns = 0, nEndPos = *rAttr.End();
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
            const sal_uInt16* pEnd = pHt->End();
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


