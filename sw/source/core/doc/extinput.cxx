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

#include <com/sun/star/i18n/ScriptType.hpp>

#include <editeng/langitem.hxx>
#include <editeng/scripttypeitem.hxx>

#include <vcl/keycodes.hxx>
#include <vcl/cmdevt.hxx>

#include <hintids.hxx>
#include <extinput.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <index.hxx>
#include <ndtxt.hxx>
#include <txtfrm.hxx>
#include <swundo.hxx>

using namespace ::com::sun::star;

SwExtTextInput::SwExtTextInput( const SwPaM& rPam, Ring* pRing )
    : SwPaM( *rPam.GetPoint(), (SwPaM*)pRing ),
    eInputLanguage(LANGUAGE_DONTKNOW)
{
    bIsOverwriteCursor = sal_False;
    bInsText = sal_True;
}

SwExtTextInput::~SwExtTextInput()
{
    SwDoc *const pDoc = GetDoc();
    if (pDoc->IsInDtor()) { return; /* #i58606# */ }

    SwTxtNode* pTNd = GetPoint()->nNode.GetNode().GetTxtNode();
    if( pTNd )
    {
        SwIndex& rIdx = GetPoint()->nContent;
        xub_StrLen nSttCnt = rIdx.GetIndex(),
                   nEndCnt = GetMark()->nContent.GetIndex();
        if( nEndCnt != nSttCnt )
        {
            if( nEndCnt < nSttCnt )
            {
                xub_StrLen n = nEndCnt; nEndCnt = nSttCnt; nSttCnt = n;
            }

            // In order to get Undo/Redlining etc. working correctly,
            // we need to go through the Doc interface
            if(eInputLanguage != LANGUAGE_DONTKNOW)
            {
                // #i41974# Only set language attribute
                // for CJK/CTL scripts.
                bool bLang = true;
                sal_uInt16 nWhich = RES_CHRATR_LANGUAGE;
                switch(GetI18NScriptTypeOfLanguage(eInputLanguage))
                {
                    case  i18n::ScriptType::ASIAN:     nWhich = RES_CHRATR_CJK_LANGUAGE; break;
                    case  i18n::ScriptType::COMPLEX:   nWhich = RES_CHRATR_CTL_LANGUAGE; break;
                    default: bLang = false;
                }
                if ( bLang )
                {
                    SvxLanguageItem aLangItem( eInputLanguage, nWhich );
                    pDoc->InsertPoolItem(*this, aLangItem, 0 );
                }
            }
            rIdx = nSttCnt;
            String sTxt( pTNd->GetTxt().copy(nSttCnt, nEndCnt - nSttCnt));
            if( bIsOverwriteCursor && sOverwriteText.Len() )
            {
                xub_StrLen nLen = sTxt.Len();
                if( nLen > sOverwriteText.Len() )
                {
                    rIdx += sOverwriteText.Len();
                    pTNd->EraseText( rIdx, nLen - sOverwriteText.Len() );
                    rIdx = nSttCnt;
                    pTNd->ReplaceText( rIdx, sOverwriteText.Len(),
                                            sOverwriteText );
                    if( bInsText )
                    {
                        rIdx = nSttCnt;
                        pDoc->GetIDocumentUndoRedo().StartUndo(
                                UNDO_OVERWRITE, NULL );
                        pDoc->Overwrite( *this, sTxt.Copy( 0,
                                                    sOverwriteText.Len() ));
                        pDoc->InsertString( *this,
                            sTxt.Copy( sOverwriteText.Len() ) );
                        pDoc->GetIDocumentUndoRedo().EndUndo(
                                UNDO_OVERWRITE, NULL );
                    }
                }
                else
                {
                    pTNd->ReplaceText( rIdx, nLen,
                            sOverwriteText.Copy( 0, nLen ));
                    if( bInsText )
                    {
                        rIdx = nSttCnt;
                        pDoc->Overwrite( *this, sTxt );
                    }
                }
            }
            else
            {
                pTNd->EraseText( rIdx, nEndCnt - nSttCnt );

                if( bInsText )
                {
                    pDoc->InsertString( *this, sTxt );
                }
            }
        }
    }
}

void SwExtTextInput::SetInputData( const CommandExtTextInputData& rData )
{
    SwTxtNode* pTNd = GetPoint()->nNode.GetNode().GetTxtNode();
    if( pTNd )
    {
        xub_StrLen nSttCnt = GetPoint()->nContent.GetIndex(),
                    nEndCnt = GetMark()->nContent.GetIndex();
        if( nEndCnt < nSttCnt )
        {
            xub_StrLen n = nEndCnt; nEndCnt = nSttCnt; nSttCnt = n;
        }

        SwIndex aIdx( pTNd, nSttCnt );
        const String& rNewStr = rData.GetText();

        if( bIsOverwriteCursor && sOverwriteText.Len() )
        {
            xub_StrLen nReplace = nEndCnt - nSttCnt;
            if( rNewStr.Len() < nReplace )
            {
                // We have to insert some characters from the saved original text
                nReplace = nReplace - rNewStr.Len();
                aIdx += rNewStr.Len();
                pTNd->ReplaceText( aIdx, nReplace,
                            sOverwriteText.Copy( rNewStr.Len(), nReplace ));
                aIdx = nSttCnt;
                nReplace = rNewStr.Len();
            }
            else if( sOverwriteText.Len() < nReplace )
            {
                nReplace = nReplace - sOverwriteText.Len();
                aIdx += sOverwriteText.Len();
                pTNd->EraseText( aIdx, nReplace );
                aIdx = nSttCnt;
                nReplace = sOverwriteText.Len();
            }
            else if( (nReplace = sOverwriteText.Len()) > rNewStr.Len() )
                nReplace = rNewStr.Len();

            pTNd->ReplaceText( aIdx, nReplace, rNewStr );
            if( !HasMark() )
                SetMark();
            GetMark()->nContent = aIdx;
        }
        else
        {
            if( nSttCnt < nEndCnt )
            {
                pTNd->EraseText( aIdx, nEndCnt - nSttCnt );
            }

            pTNd->InsertText( rNewStr, aIdx,
                    IDocumentContentOperations::INS_EMPTYEXPAND );
            if( !HasMark() )
                SetMark();
        }

        GetPoint()->nContent = nSttCnt;

        aAttrs.clear();
        if( rData.GetTextAttr() )
        {
            const sal_uInt16 *pAttrs = rData.GetTextAttr();
            aAttrs.insert( aAttrs.begin(), pAttrs, pAttrs + rData.GetText().getLength() );
        }
    }
}

void SwExtTextInput::SetOverwriteCursor( sal_Bool bFlag )
{
    bIsOverwriteCursor = bFlag;

    SwTxtNode* pTNd;
    if( bIsOverwriteCursor &&
        0 != (pTNd = GetPoint()->nNode.GetNode().GetTxtNode()) )
    {
        xub_StrLen nSttCnt = GetPoint()->nContent.GetIndex(),
                    nEndCnt = GetMark()->nContent.GetIndex();
        sOverwriteText = pTNd->GetTxt().copy( nEndCnt < nSttCnt ? nEndCnt
                                                                : nSttCnt );
        if( sOverwriteText.Len() )
        {
            xub_StrLen nInWrdAttrPos = sOverwriteText.Search( CH_TXTATR_INWORD ),
                    nWrdAttrPos = sOverwriteText.Search( CH_TXTATR_BREAKWORD );
            if( nWrdAttrPos < nInWrdAttrPos )
                nInWrdAttrPos = nWrdAttrPos;
            if( STRING_NOTFOUND != nInWrdAttrPos )
                sOverwriteText.Erase( nInWrdAttrPos );
        }
    }
}

// The Doc interfaces

SwExtTextInput* SwDoc::CreateExtTextInput( const SwPaM& rPam )
{
    SwExtTextInput* pNew = new SwExtTextInput( rPam, mpExtInputRing );
    if( !mpExtInputRing )
        mpExtInputRing = pNew;
    pNew->SetMark();
    return pNew;
}

void SwDoc::DeleteExtTextInput( SwExtTextInput* pDel )
{
    if( pDel == mpExtInputRing )
    {
        if( pDel->GetNext() != mpExtInputRing )
            mpExtInputRing = (SwPaM*)pDel->GetNext();
        else
            mpExtInputRing = 0;
    }
    delete pDel;
}

SwExtTextInput* SwDoc::GetExtTextInput( const SwNode& rNd,
                                        xub_StrLen nCntntPos ) const
{
    SwExtTextInput* pRet = 0;
    if( mpExtInputRing )
    {
        sal_uLong nNdIdx = rNd.GetIndex();
        SwExtTextInput* pTmp = (SwExtTextInput*)mpExtInputRing;
        do {
            sal_uLong nPt = pTmp->GetPoint()->nNode.GetIndex(),
                  nMk = pTmp->GetMark()->nNode.GetIndex();
            xub_StrLen nPtCnt = pTmp->GetPoint()->nContent.GetIndex(),
                         nMkCnt = pTmp->GetMark()->nContent.GetIndex();

            if( nPt < nMk || ( nPt == nMk && nPtCnt < nMkCnt ))
            {
                sal_uLong nTmp = nMk; nMk = nPt; nPt = nTmp;
                nTmp = nMkCnt; nMkCnt = nPtCnt; nPtCnt = (xub_StrLen)nTmp;
            }

            if( nMk <= nNdIdx && nNdIdx <= nPt &&
                ( STRING_NOTFOUND == nCntntPos ||
                    ( nMkCnt <= nCntntPos && nCntntPos <= nPtCnt )))
            {
                pRet = pTmp;
                break;
            }
        } while( mpExtInputRing != (pTmp = (SwExtTextInput*)mpExtInputRing ) );
    }
    return pRet;
}

SwExtTextInput* SwDoc::GetExtTextInput() const
{
    OSL_ENSURE( !mpExtInputRing || mpExtInputRing == mpExtInputRing->GetNext(),
            "more then one InputEngine available" );
    return (SwExtTextInput*)mpExtInputRing;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
