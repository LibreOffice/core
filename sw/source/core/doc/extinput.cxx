/*************************************************************************
 *
 *  $RCSfile: extinput.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-12 12:17:52 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#pragma hdrstop

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SV_KEYCODES_HXX
#include <vcl/keycodes.hxx>
#endif
#ifndef _VCL_CMDEVT_HXX
#include <vcl/cmdevt.hxx>
#endif
#ifndef _EXTINPUT_HXX
#include <extinput.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _INDEX_HXX
#include <index.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _TXTFRM_HXX
#include <txtfrm.hxx>
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX
#include <svx/langitem.hxx>
#endif
#ifndef _SVX_SCRIPTTYPEITEM_HXX
#include <svx/scripttypeitem.hxx>
#endif
#ifndef _COM_SUN_STAR_I18N_SCRIPTTYPE_HPP_
#include <com/sun/star/i18n/ScriptType.hpp>
#endif


SwExtTextInput::SwExtTextInput( const SwPaM& rPam, Ring* pRing )
    : SwPaM( *rPam.GetPoint(), (SwPaM*)pRing ),
    eInputLanguage(LANGUAGE_DONTKNOW)
{
    bIsOverwriteCursor = FALSE;
    bInsText = TRUE;
}

SwExtTextInput::~SwExtTextInput()
{
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
            SwDoc* pDoc = GetDoc();
            if(eInputLanguage != LANGUAGE_DONTKNOW)
            {
                USHORT nWhich = RES_CHRATR_LANGUAGE;
                switch(GetI18NScriptTypeOfLanguage(eInputLanguage))
                {
                    case  ::com::sun::star::i18n::ScriptType::ASIAN:     nWhich = RES_CHRATR_CJK_LANGUAGE; break;
                    case  ::com::sun::star::i18n::ScriptType::COMPLEX:   nWhich = RES_CHRATR_CTL_LANGUAGE; break;
                }
                SvxLanguageItem aLangItem( eInputLanguage, nWhich );
                pDoc->Insert(*this, aLangItem, 0 );
            }
            rIdx = nSttCnt;
            String sTxt( pTNd->GetTxt().Copy( nSttCnt, nEndCnt - nSttCnt ));
            if( bIsOverwriteCursor && sOverwriteText.Len() )
            {
                xub_StrLen nLen = sTxt.Len();
                if( nLen > sOverwriteText.Len() )
                {
                    rIdx += sOverwriteText.Len();
                    pTNd->Erase( rIdx, nLen - sOverwriteText.Len() );
                    rIdx = nSttCnt;
                    pTNd->Replace( rIdx, sOverwriteText.Len(),
                                            sOverwriteText );
                    if( bInsText )
                    {
                        rIdx = nSttCnt;
                        pDoc->StartUndo( UNDO_OVERWRITE );
                        pDoc->Overwrite( *this, sTxt.Copy( 0,
                                                    sOverwriteText.Len() ));
                        pDoc->Insert( *this, sTxt.Copy( sOverwriteText.Len() ));
                        pDoc->EndUndo( UNDO_OVERWRITE );
                    }
                }
                else
                {
                    pTNd->Replace( rIdx, nLen, sOverwriteText.Copy( 0, nLen ));
                    if( bInsText )
                    {
                        rIdx = nSttCnt;
                        pDoc->Overwrite( *this, sTxt );
                    }
                }
            }
            else
            {
                pTNd->Erase( rIdx, nEndCnt - nSttCnt );

                if( bInsText )
                    pDoc->Insert( *this, sTxt );
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
                nReplace -= rNewStr.Len();
                aIdx += rNewStr.Len();
                pTNd->Replace( aIdx, nReplace,
                            sOverwriteText.Copy( rNewStr.Len(), nReplace ));
                aIdx = nSttCnt;
                nReplace = rNewStr.Len();
            }
            else if( sOverwriteText.Len() < nReplace )
            {
                nReplace -= sOverwriteText.Len();
                aIdx += sOverwriteText.Len();
                pTNd->Erase( aIdx, nReplace );
                aIdx = nSttCnt;
                nReplace = sOverwriteText.Len();
            }
            else if( (nReplace = sOverwriteText.Len()) > rNewStr.Len() )
                nReplace = rNewStr.Len();

            pTNd->Replace( aIdx, nReplace, rNewStr );
            if( !HasMark() )
                SetMark();
            GetMark()->nContent = aIdx;
        }
        else
        {
            if( nSttCnt < nEndCnt )
                pTNd->Erase( aIdx, nEndCnt - nSttCnt );

            pTNd->Insert( rNewStr, aIdx, INS_EMPTYEXPAND );
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

void SwExtTextInput::SetOverwriteCursor( BOOL bFlag )
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
        ULONG nNdIdx = rNd.GetIndex();
        SwExtTextInput* pTmp = (SwExtTextInput*)pExtInputRing;
        do {
            ULONG nPt = pTmp->GetPoint()->nNode.GetIndex(),
                  nMk = pTmp->GetMark()->nNode.GetIndex();
            xub_StrLen nPtCnt = pTmp->GetPoint()->nContent.GetIndex(),
                         nMkCnt = pTmp->GetMark()->nContent.GetIndex();

            if( nPt < nMk || ( nPt == nMk && nPtCnt < nMkCnt ))
            {
                ULONG nTmp = nMk; nMk = nPt; nPt = nTmp;
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
    ASSERT( !pExtInputRing || pExtInputRing == pExtInputRing->GetNext(),
            "more then one InputEngine available" );
    return (SwExtTextInput*)pExtInputRing;
}


