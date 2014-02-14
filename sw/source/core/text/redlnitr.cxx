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

#include "hintids.hxx"
#include <svl/whiter.hxx>
#include <tools/shl.hxx>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <swmodule.hxx>
#include <redline.hxx>
#include <txtatr.hxx>
#include <docary.hxx>
#include <itratr.hxx>
#include <ndtxt.hxx>
#include <doc.hxx>
#include <rootfrm.hxx>
#include <breakit.hxx>
#include <vcl/keycodes.hxx>
#include <vcl/cmdevt.hxx>
#include <vcl/settings.hxx>
#include <txtfrm.hxx>
#include <vcl/svapp.hxx>
#include <redlnitr.hxx>
#include <extinput.hxx>
#include <sfx2/printer.hxx>
#include <vcl/window.hxx>

using namespace ::com::sun::star;

/*************************************************************************
 *                      SwAttrIter::CtorInitAttrIter()
 *************************************************************************/
void SwAttrIter::CtorInitAttrIter( SwTxtNode& rTxtNode, SwScriptInfo& rScrInf, SwTxtFrm* pFrm )
{
    // Beim HTML-Import kann es vorkommen, dass kein Layout existiert.
    SwRootFrm* pRootFrm = rTxtNode.getIDocumentLayoutAccess()->GetCurrentLayout();
    pShell = pRootFrm ? pRootFrm->GetCurrShell() : 0;

    pScriptInfo = &rScrInf;

    // attributes set at the whole paragraph
    pAttrSet = rTxtNode.GetpSwAttrSet();
    // attribute array
    pHints = rTxtNode.GetpSwpHints();

    // Build a font matching the default paragraph style:
    SwFontAccess aFontAccess( &rTxtNode.GetAnyFmtColl(), pShell );
    delete pFnt;
    pFnt = new SwFont( *aFontAccess.Get()->GetFont() );

    // set font to vertical if frame layout is vertical
    bool bVertLayout = false;
    bool bRTL = false;
    if ( pFrm )
    {
        if ( pFrm->IsVertical() )
        {
            bVertLayout = true;
            pFnt->SetVertical( pFnt->GetOrientation(), true );
        }
        bRTL = pFrm->IsRightToLeft();
    }

    // Initialize the default attribute of the attribute handler
    // based on the attribute array cached together with the font.
    // If any further attributes for the paragraph are given in pAttrSet
    // consider them during construction of the default array, and apply
    // them to the font
    aAttrHandler.Init( aFontAccess.Get()->GetDefault(), pAttrSet,
                       *rTxtNode.getIDocumentSettingAccess(), pShell, *pFnt, bVertLayout );

    aMagicNo[SW_LATIN] = aMagicNo[SW_CJK] = aMagicNo[SW_CTL] = NULL;

    // determine script changes if not already done for current paragraph
    OSL_ENSURE( pScriptInfo, "No script info available");
    if ( pScriptInfo->GetInvalidityA() != COMPLETE_STRING )
         pScriptInfo->InitScriptInfo( rTxtNode, bRTL );

    if ( g_pBreakIt->GetBreakIter().is() )
    {
        pFnt->SetActual( SwScriptInfo::WhichFont( 0, 0, pScriptInfo ) );

        sal_Int32 nChg = 0;
        sal_uInt16 nCnt = 0;

        do
        {
            nChg = pScriptInfo->GetScriptChg( nCnt );
            sal_uInt16 nScript = pScriptInfo->GetScriptType( nCnt++ );
            sal_uInt8 nTmp = 4;
            switch ( nScript ) {
                case i18n::ScriptType::ASIAN :
                    if( !aMagicNo[SW_CJK] ) nTmp = SW_CJK; break;
                case i18n::ScriptType::COMPLEX :
                    if( !aMagicNo[SW_CTL] ) nTmp = SW_CTL; break;
                default:
                    if( !aMagicNo[SW_LATIN ] ) nTmp = SW_LATIN;
            }
            if( nTmp < 4 )
            {
                pFnt->ChkMagic( pShell, nTmp );
                pFnt->GetMagic( aMagicNo[ nTmp ], aFntIdx[ nTmp ], nTmp );
            }
        } while (nChg < rTxtNode.GetTxt().getLength());
    }
    else
    {
        pFnt->ChkMagic( pShell, SW_LATIN );
        pFnt->GetMagic( aMagicNo[ SW_LATIN ], aFntIdx[ SW_LATIN ], SW_LATIN );
    }

    nStartIndex = nEndIndex = nPos = nChgCnt = 0;
    nPropFont = 0;
    SwDoc* pDoc = rTxtNode.GetDoc();
    const IDocumentRedlineAccess* pIDRA = rTxtNode.getIDocumentRedlineAccess();

    const SwExtTextInput* pExtInp = pDoc->GetExtTextInput( rTxtNode );
    const bool bShow = IDocumentRedlineAccess::IsShowChanges( pIDRA->GetRedlineMode() );
    if( pExtInp || bShow )
    {
        sal_uInt16 nRedlPos = pIDRA->GetRedlinePos( rTxtNode, USHRT_MAX );
        if( pExtInp || USHRT_MAX != nRedlPos )
        {
            const std::vector<sal_uInt16> *pArr = 0;
            sal_Int32 nInputStt = 0;
            if( pExtInp )
            {
                pArr = &pExtInp->GetAttrs();
                nInputStt = pExtInp->Start()->nContent.GetIndex();
                Seek( 0 );
            }

            pRedln = new SwRedlineItr( rTxtNode, *pFnt, aAttrHandler, nRedlPos,
                                        bShow, pArr, nInputStt );

            if( pRedln->IsOn() )
                ++nChgCnt;
        }
    }
}

/*************************************************************************
 * SwRedlineItr - Der Redline-Iterator
 *
 * Folgende Informationen/Zustaende gibt es im RedlineIterator:
 *
 * nFirst ist der erste Index der RedlineTbl, der mit dem Absatz ueberlappt.
 *
 * nAct ist der zur Zeit aktive ( wenn bOn gesetzt ist ) oder der naechste
 * in Frage kommende Index.
 * nStart und nEnd geben die Grenzen des Objekts innerhalb des Absatzes an.
 *
 * Wenn bOn gesetzt ist, ist der Font entsprechend manipuliert worden.
 *
 * Wenn nAct auf COMPLETE_STRING gesetzt wurde ( durch Reset() ), so ist zur Zeit
 * kein Redline aktiv, nStart und nEnd sind invalid.
 *************************************************************************/

SwRedlineItr::SwRedlineItr( const SwTxtNode& rTxtNd, SwFont& rFnt,
                            SwAttrHandler& rAH, sal_Int32 nRed, bool bShw,
                            const std::vector<sal_uInt16> *pArr,
                            sal_Int32 nExtStart )
    : rDoc( *rTxtNd.GetDoc() ), rAttrHandler( rAH ), pSet( 0 ),
      nNdIdx( rTxtNd.GetIndex() ), nFirst( nRed ),
      nAct( COMPLETE_STRING ), bOn( false ), bShow( bShw )
{
    if( pArr )
        pExt = new SwExtend( *pArr, nExtStart );
    else
        pExt = NULL;
    Seek (rFnt, 0, COMPLETE_STRING);
}

SwRedlineItr::~SwRedlineItr()
{
    Clear( NULL );
    delete pSet;
    delete pExt;
}

// Der Return-Wert von SwRedlineItr::Seek gibt an, ob der aktuelle Font
// veraendert wurde durch Verlassen (-1) oder Betreten eines Bereichs (+1)

short SwRedlineItr::_Seek(SwFont& rFnt, sal_Int32 nNew, sal_Int32 nOld)
{
    short nRet = 0;
    if( ExtOn() )
        return 0; // Abkuerzung: wenn wir innerhalb eines ExtendTextInputs sind
            // kann es keine anderen Attributwechsel (auch nicht durch Redlining) geben
    if( bShow )
    {
        if( bOn )
        {
            if( nNew >= nEnd )
            {
                --nRet;
                _Clear( &rFnt );    // Wir gehen hinter den aktuellen Bereich
                ++nAct;             // und pruefen gleich den naechsten
            }
            else if( nNew < nStart )
            {
                --nRet;
                _Clear( &rFnt );    // Wir gehen vor den aktuellen Bereich
                if( nAct > nFirst )
                    nAct = nFirst;  // Die Pruefung muss von vorne beginnen
                else
                    return nRet + EnterExtend( rFnt, nNew ); // Es gibt keinen vor uns.
            }
            else
                return nRet + EnterExtend( rFnt, nNew ); // Wir sind im gleichen Bereich geblieben.
        }
        if( COMPLETE_STRING == nAct || nOld > nNew )
            nAct = nFirst;

        nStart = COMPLETE_STRING;
        nEnd = COMPLETE_STRING;

        for( ; nAct < (sal_Int32)rDoc.GetRedlineTbl().size() ; ++nAct )
        {
            rDoc.GetRedlineTbl()[ nAct ]->CalcStartEnd( nNdIdx, nStart, nEnd );

            if( nNew < nEnd )
            {
                if( nNew >= nStart ) // der einzig moegliche Kandidat
                {
                    bOn = true;
                    const SwRangeRedline *pRed = rDoc.GetRedlineTbl()[ nAct ];

                    if (pSet)
                        pSet->ClearItem();
                    else
                    {
                        SwAttrPool& rPool =
                            const_cast<SwDoc&>(rDoc).GetAttrPool();
                        pSet = new SfxItemSet(rPool, RES_CHRATR_BEGIN, RES_CHRATR_END-1);
                    }

                    if( 1 < pRed->GetStackCount() )
                        FillHints( pRed->GetAuthor( 1 ), pRed->GetType( 1 ) );
                    FillHints( pRed->GetAuthor(), pRed->GetType() );

                    SfxWhichIter aIter( *pSet );
                    MSHORT nWhich = aIter.FirstWhich();
                    while( nWhich )
                    {
                        const SfxPoolItem* pItem;
                        if( ( nWhich < RES_CHRATR_END ) &&
                            ( SFX_ITEM_SET == pSet->GetItemState( nWhich, true, &pItem ) ) )
                        {
                            SwTxtAttr* pAttr = MakeRedlineTxtAttr(
                                const_cast<SwDoc&>(rDoc),
                                *const_cast<SfxPoolItem*>(pItem) );
                            pAttr->SetPriorityAttr( true );
                            m_Hints.push_back(pAttr);
                            rAttrHandler.PushAndChg( *pAttr, rFnt );
                            if( RES_CHRATR_COLOR == nWhich )
                                rFnt.SetNoCol( true );
                        }
                        nWhich = aIter.NextWhich();
                    }

                    ++nRet;
                }
                break;
            }
            nStart = COMPLETE_STRING;
            nEnd = COMPLETE_STRING;
        }
    }
    return nRet + EnterExtend( rFnt, nNew );
}

void SwRedlineItr::FillHints( MSHORT nAuthor, RedlineType_t eType )
{
    switch ( eType )
    {
        case nsRedlineType_t::REDLINE_INSERT:
            SW_MOD()->GetInsertAuthorAttr(nAuthor, *pSet);
            break;
        case nsRedlineType_t::REDLINE_DELETE:
            SW_MOD()->GetDeletedAuthorAttr(nAuthor, *pSet);
            break;
        case nsRedlineType_t::REDLINE_FORMAT:
        case nsRedlineType_t::REDLINE_FMTCOLL:
            SW_MOD()->GetFormatAuthorAttr(nAuthor, *pSet);
            break;
        default:
            break;
    }
}

void SwRedlineItr::ChangeTxtAttr( SwFont* pFnt, SwTxtAttr &rHt, bool bChg )
{
    OSL_ENSURE( IsOn(), "SwRedlineItr::ChangeTxtAttr: Off?" );

    if( !bShow && !pExt )
        return;

    if( bChg )
    {
        if ( pExt && pExt->IsOn() )
            rAttrHandler.PushAndChg( rHt, *pExt->GetFont() );
        else
            rAttrHandler.PushAndChg( rHt, *pFnt );
    }
    else
    {
        OSL_ENSURE( ! pExt || ! pExt->IsOn(), "Pop of attribute during opened extension" );
        rAttrHandler.PopAndChg( rHt, *pFnt );
    }
}

void SwRedlineItr::_Clear( SwFont* pFnt )
{
    OSL_ENSURE( bOn, "SwRedlineItr::Clear: Off?" );
    bOn = false;
    while (!m_Hints.empty())
    {
        SwTxtAttr *pPos = m_Hints.front();
        m_Hints.pop_front();
        if( pFnt )
            rAttrHandler.PopAndChg( *pPos, *pFnt );
        else
            rAttrHandler.Pop( *pPos );
        SwTxtAttr::Destroy(pPos, const_cast<SwDoc&>(rDoc).GetAttrPool() );
    }
    if( pFnt )
        pFnt->SetNoCol( false );
}

sal_Int32 SwRedlineItr::_GetNextRedln( sal_Int32 nNext )
{
    nNext = NextExtend( nNext );
    if( !bShow || COMPLETE_STRING == nFirst )
        return nNext;
    if( COMPLETE_STRING == nAct )
    {
        nAct = nFirst;
        rDoc.GetRedlineTbl()[ nAct ]->CalcStartEnd( nNdIdx, nStart, nEnd );
    }
    if( bOn || !nStart )
    {
        if( nEnd < nNext )
            nNext = nEnd;
    }
    else if( nStart < nNext )
        nNext = nStart;
    return nNext;
}

bool SwRedlineItr::_ChkSpecialUnderline() const
{
    // Wenn die Unterstreichung oder das Escapement vom Redling kommt,
    // wenden wir immer das SpecialUnderlining, d.h. die Unterstreichung
    // unter der Grundlinie an.
    for (size_t i = 0; i < m_Hints.size(); ++i)
    {
        MSHORT nWhich = m_Hints[i]->Which();
        if( RES_CHRATR_UNDERLINE == nWhich ||
            RES_CHRATR_ESCAPEMENT == nWhich )
            return true;
    }
    return false;
}

bool SwRedlineItr::CheckLine( sal_Int32 nChkStart, sal_Int32 nChkEnd )
{
    if( nFirst == COMPLETE_STRING )
        return false;
    if( nChkEnd == nChkStart ) // Leerzeilen gucken ein Zeichen weiter.
        ++nChkEnd;
    sal_Int32 nOldStart = nStart;
    sal_Int32 nOldEnd = nEnd;
    sal_Int32 nOldAct = nAct;
    bool bRet = false;

    for( nAct = nFirst; nAct < (sal_Int32)rDoc.GetRedlineTbl().size() ; ++nAct )
    {
        rDoc.GetRedlineTbl()[ nAct ]->CalcStartEnd( nNdIdx, nStart, nEnd );
        if( nChkEnd < nStart )
            break;
        if( nChkStart <= nEnd && ( nChkEnd > nStart || COMPLETE_STRING == nEnd ) )
        {
            bRet = true;
            break;
        }
    }

    nStart = nOldStart;
    nEnd = nOldEnd;
    nAct = nOldAct;
    return bRet;
}

void SwExtend::ActualizeFont( SwFont &rFnt, sal_uInt16 nAttr )
{
    if ( nAttr & EXTTEXTINPUT_ATTR_UNDERLINE )
        rFnt.SetUnderline( UNDERLINE_SINGLE );
    else if ( nAttr & EXTTEXTINPUT_ATTR_BOLDUNDERLINE )
        rFnt.SetUnderline( UNDERLINE_BOLD );
    else if ( nAttr & EXTTEXTINPUT_ATTR_DOTTEDUNDERLINE )
        rFnt.SetUnderline( UNDERLINE_DOTTED );
    else if ( nAttr & EXTTEXTINPUT_ATTR_DASHDOTUNDERLINE )
        rFnt.SetUnderline( UNDERLINE_DOTTED );

    if ( nAttr & EXTTEXTINPUT_ATTR_REDTEXT )
        rFnt.SetColor( Color( COL_RED ) );

    if ( nAttr & EXTTEXTINPUT_ATTR_HIGHLIGHT )
    {
        const StyleSettings& rStyleSettings = GetpApp()->GetSettings().GetStyleSettings();
        rFnt.SetColor( rStyleSettings.GetHighlightTextColor() );
        rFnt.SetBackColor( new Color( rStyleSettings.GetHighlightColor() ) );
    }
    if ( nAttr & EXTTEXTINPUT_ATTR_GRAYWAVELINE )
        rFnt.SetGreyWave( true );
}

short SwExtend::Enter(SwFont& rFnt, sal_Int32 nNew)
{
    OSL_ENSURE( !Inside(), "SwExtend: Enter without Leave" );
    OSL_ENSURE( !pFnt, "SwExtend: Enter with Font" );
    nPos = nNew;
    if( Inside() )
    {
        pFnt = new SwFont( rFnt );
        ActualizeFont( rFnt, rArr[ nPos - nStart ] );
        return 1;
    }
    return 0;
}

bool SwExtend::_Leave(SwFont& rFnt, sal_Int32 nNew)
{
    OSL_ENSURE( Inside(), "SwExtend: Leave without Enter" );
    MSHORT nOldAttr = rArr[ nPos - nStart ];
    nPos = nNew;
    if( Inside() )
    {   // Wir sind innerhalb des ExtendText-Bereichs geblieben
        MSHORT nAttr = rArr[ nPos - nStart ];
        if( nOldAttr != nAttr ) // Gibt es einen (inneren) Attributwechsel?
        {
            rFnt = *pFnt;
            ActualizeFont( rFnt, nAttr );
        }
    }
    else
    {
        rFnt = *pFnt;
        delete pFnt;
        pFnt = NULL;
        return true;
    }
    return false;
}

sal_Int32 SwExtend::Next( sal_Int32 nNext )
{
    if( nPos < nStart )
    {
        if( nNext > nStart )
            nNext = nStart;
    }
    else if( nPos < nEnd )
    {
        sal_Int32 nIdx = nPos - nStart;
        MSHORT nAttr = rArr[ nIdx ];
        while( ++nIdx < (sal_Int32)rArr.size() && nAttr == rArr[ nIdx ] )
            ; //nothing
        nIdx = nIdx + nStart;
        if( nNext > nIdx )
            nNext = nIdx;
    }
    return nNext;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
