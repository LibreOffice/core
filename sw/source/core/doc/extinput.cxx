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

            // damit Undo / Redlining usw. richtig funktioniert,
            // muss ueber die Doc-Schnittstellen gegangen werden !!!
            if(eInputLanguage != LANGUAGE_DONTKNOW)
            {
                // --> FME 2005-02-11 #i41974# Only set language attribute
                // for CJK/CTL scripts.
                bool bLang = true;
                // <--
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
            String sTxt( pTNd->GetTxt().Copy( nSttCnt, nEndCnt - nSttCnt ));
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
                // then we must insert from the saved original text
                // some characters
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

        if( aAttrs.Count() )
            aAttrs.Remove( 0, aAttrs.Count() );
        if( rData.GetTextAttr() )
            aAttrs.Insert( rData.GetTextAttr(), rData.GetText().Len(), 0 );
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
        sOverwriteText = pTNd->GetTxt().Copy( nEndCnt < nSttCnt ? nEndCnt
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

// die Doc Schnittstellen:

SwExtTextInput* SwDoc::CreateExtTextInput( const SwPaM& rPam )
{
    SwExtTextInput* pNew = new SwExtTextInput( rPam, pExtInputRing );
    if( !pExtInputRing )
        pExtInputRing = pNew;
    pNew->SetMark();
    return pNew;
}

void SwDoc::DeleteExtTextInput( SwExtTextInput* pDel )
{
    if( pDel == pExtInputRing )
    {
        if( pDel->GetNext() != pExtInputRing )
            pExtInputRing = (SwPaM*)pDel->GetNext();
        else
            pExtInputRing = 0;
    }
    delete pDel;
}

SwExtTextInput* SwDoc::GetExtTextInput( const SwNode& rNd,
                                        xub_StrLen nCntntPos ) const
{
    SwExtTextInput* pRet = 0;
    if( pExtInputRing )
    {
        sal_uLong nNdIdx = rNd.GetIndex();
        SwExtTextInput* pTmp = (SwExtTextInput*)pExtInputRing;
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
        } while( pExtInputRing != (pTmp = (SwExtTextInput*)pExtInputRing ) );
    }
    return pRet;
}

SwExtTextInput* SwDoc::GetExtTextInput() const
{
    OSL_ENSURE( !pExtInputRing || pExtInputRing == pExtInputRing->GetNext(),
            "more then one InputEngine available" );
    return (SwExtTextInput*)pExtInputRing;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
