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

#include <comphelper/string.hxx>
#include <hintids.hxx>

#include <doc.hxx>
#if OSL_DEBUG_LEVEL > 1
#include <stdio.h>
#endif
#include <osl/thread.h>
#include <vcl/help.hxx>
#include <svl/stritem.hxx>
#include <unotools/securityoptions.hxx>
#include <tools/urlobj.hxx>
#include <txtrfmrk.hxx>
#include <fmtrfmrk.hxx>
#include <editeng/flditem.hxx>
#include <svl/urihelper.hxx>
#include <svx/svdotext.hxx>
#include <editeng/outliner.hxx>
#include <svl/itemiter.hxx>
#include <svx/svdview.hxx>
#include <svx/svdpagv.hxx>
#include <swmodule.hxx>
#include <modcfg.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <docsh.hxx>
#include <edtwin.hxx>
#include <dpage.hxx>
#include <shellres.hxx>
#include <docufld.hxx>
#include <dbfld.hxx>
#include <reffld.hxx>
#include <cellatr.hxx>
#include <shdwcrsr.hxx>
#include <fmtcol.hxx>
#include <charfmt.hxx>
#include <fmtftn.hxx>
#include <redline.hxx>
#include <tox.hxx>
#include <txttxmrk.hxx>
#include <uitool.hxx>
#include <viewopt.hxx>
#include <docvw.hrc>
#include <utlui.hrc>

#include <PostItMgr.hxx>
#include <fmtfld.hxx>

// #i104300#
#include <IDocumentMarkAccess.hxx>
#include <ndtxt.hxx>

static OUString lcl_GetRedlineHelp( const SwRangeRedline& rRedl, sal_Bool bBalloon )
{
    sal_uInt16 nResId = 0;
    switch( rRedl.GetType() )
    {
    case nsRedlineType_t::REDLINE_INSERT:   nResId = STR_REDLINE_INSERT; break;
    case nsRedlineType_t::REDLINE_DELETE:   nResId = STR_REDLINE_DELETE; break;
    case nsRedlineType_t::REDLINE_FORMAT:   nResId = STR_REDLINE_FORMAT; break;
    case nsRedlineType_t::REDLINE_TABLE:        nResId = STR_REDLINE_TABLE; break;
    case nsRedlineType_t::REDLINE_FMTCOLL:  nResId = STR_REDLINE_FMTCOLL; break;
    }

    OUStringBuffer sBuf;
    if( nResId )
    {
        sBuf.append(SW_RESSTR(nResId));
        sBuf.append(": ");
        sBuf.append(rRedl.GetAuthorString());
        sBuf.append(" - ");
        sBuf.append(GetAppLangDateTimeString(rRedl.GetTimeStamp()));
        if( bBalloon && !rRedl.GetComment().isEmpty() )
            sBuf.append('\n').append(rRedl.GetComment());
    }
    return sBuf.makeStringAndClear();
}


void SwEditWin::RequestHelp(const HelpEvent &rEvt)
{
    SwWrtShell &rSh = m_rView.GetWrtShell();
    bool bQuickBalloon = 0 != (rEvt.GetMode() & ( HELPMODE_QUICK | HELPMODE_BALLOON ));
    if(bQuickBalloon && !rSh.GetViewOptions()->IsShowContentTips())
        return;
    bool bContinue = true;
    SET_CURR_SHELL(&rSh);
    OUString sTxt;
    Point aPos( PixelToLogic( ScreenToOutputPixel( rEvt.GetMousePosPixel() ) ));
    sal_Bool bBalloon = static_cast< sal_Bool >(rEvt.GetMode() & HELPMODE_BALLOON);

    SdrView *pSdrView = rSh.GetDrawView();

    if( bQuickBalloon )
    {
        if( pSdrView )
        {
            SdrPageView* pPV = pSdrView->GetSdrPageView();
            SwDPage* pPage = pPV ? ((SwDPage*)pPV->GetPage()) : 0;
            bContinue = pPage && pPage->RequestHelp(this, pSdrView, rEvt);
        }
    }

    if( bContinue && bQuickBalloon)
    {
        SwRect aFldRect;
        SwContentAtPos aCntntAtPos( SwContentAtPos::SW_FIELD |
                                    SwContentAtPos::SW_INETATTR |
                                    SwContentAtPos::SW_FTN |
                                    SwContentAtPos::SW_REDLINE |
                                    SwContentAtPos::SW_TOXMARK |
                                    SwContentAtPos::SW_REFMARK |
                                    SwContentAtPos::SW_SMARTTAG |
#ifdef DBG_UTIL
                                    SwContentAtPos::SW_TABLEBOXVALUE |
                        ( bBalloon ? SwContentAtPos::SW_CURR_ATTRS : 0) |
#endif
                                    SwContentAtPos::SW_TABLEBOXFML );

        if( rSh.GetContentAtPos( aPos, aCntntAtPos, sal_False, &aFldRect ) )
        {
            sal_uInt16 nStyle = 0; // style of quick help
            switch( aCntntAtPos.eCntntAtPos )
            {
            case SwContentAtPos::SW_TABLEBOXFML:
                sTxt = "= ";
                sTxt += ((SwTblBoxFormula*)aCntntAtPos.aFnd.pAttr)->GetFormula();
                break;
#ifdef DBG_UTIL
            case SwContentAtPos::SW_TABLEBOXVALUE:
            {
                sTxt = OStringToOUString(OString::number(
                            ((SwTblBoxValue*)aCntntAtPos.aFnd.pAttr)->GetValue()),
                            osl_getThreadTextEncoding());
            }
            break;
            case SwContentAtPos::SW_CURR_ATTRS:
                sTxt = aCntntAtPos.sStr;
                break;
#endif

            case SwContentAtPos::SW_INETATTR:
            {
                sTxt = ((SfxStringItem*)aCntntAtPos.aFnd.pAttr)->GetValue();
                sTxt = URIHelper::removePassword( sTxt,
                                        INetURLObject::WAS_ENCODED,
                                           INetURLObject::DECODE_UNAMBIGUOUS);
                //#i63832# remove the link target type
                sal_Int32 nFound = sTxt.indexOf(cMarkSeparator);
                if( nFound != -1 && (++nFound) < sTxt.getLength() )
                {
                    OUString sSuffix( sTxt.copy(nFound) );
                    if( sSuffix == "table" ||
                        sSuffix == "frame" ||
                        sSuffix == "region" ||
                        sSuffix == "outline" ||
                        sSuffix == "text" ||
                        sSuffix == "graphic" ||
                        sSuffix == "ole" )
                    sTxt = sTxt.copy( 0, nFound - 1);
                }
                // #i104300#
                // special handling if target is a cross-reference bookmark
                {
                    OUString sTmpSearchStr = sTxt.copy( 1 );
                    IDocumentMarkAccess* const pMarkAccess =
                                                rSh.getIDocumentMarkAccess();
                    IDocumentMarkAccess::const_iterator_t ppBkmk =
                                    pMarkAccess->findBookmark( sTmpSearchStr );
                    if ( ppBkmk != pMarkAccess->getBookmarksEnd() &&
                         IDocumentMarkAccess::GetType( *(ppBkmk->get()) )
                            == IDocumentMarkAccess::CROSSREF_HEADING_BOOKMARK )
                    {
                        SwTxtNode* pTxtNode = ppBkmk->get()->GetMarkStart().nNode.GetNode().GetTxtNode();
                        if ( pTxtNode )
                        {
                            sTxt = pTxtNode->GetExpandTxt( 0, pTxtNode->Len(), true, true );

                            if( !sTxt.isEmpty() )
                            {
                                OUStringBuffer sTmp(comphelper::string::remove(sTxt, 0xad));
                                for (sal_Int32 i = 0; i < sTmp.getLength(); ++i)
                                {
                                    if (sTmp[i] < 0x20)
                                        sTmp[i] = 0x20;
                                    else if (sTmp[i] == 0x2011)
                                        sTmp[i] = '-';
                                }
                                sTxt = sTmp.makeStringAndClear();
                            }
                        }
                    }
                }
                // #i80029#
                sal_Bool bExecHyperlinks = m_rView.GetDocShell()->IsReadOnly();
                if ( !bExecHyperlinks )
                {
                    SvtSecurityOptions aSecOpts;
                    bExecHyperlinks = !aSecOpts.IsOptionSet( SvtSecurityOptions::E_CTRLCLICK_HYPERLINK );

                    sTxt = ": " + sTxt;
                    if ( !bExecHyperlinks )
                        sTxt = SwViewShell::GetShellRes()->aLinkCtrlClick + sTxt;
                    else
                        sTxt = SwViewShell::GetShellRes()->aLinkClick + sTxt;
                }
                break;
            }
            case SwContentAtPos::SW_SMARTTAG:
            {
                KeyCode aCode( KEY_SPACE );
                KeyCode aModifiedCode( KEY_SPACE, KEY_MOD1 );
                OUString aModStr( aModifiedCode.GetName() );
                aModStr = aModStr.replaceFirst(aCode.GetName(), OUString());
                aModStr = aModStr.replaceAll("+", OUString());
                sTxt = SW_RESSTR(STR_SMARTTAG_CLICK).replaceAll("%s", aModStr);
            }
            break;

            case SwContentAtPos::SW_FTN:
                if( aCntntAtPos.pFndTxtAttr && aCntntAtPos.aFnd.pAttr )
                {
                    const SwFmtFtn* pFtn = (SwFmtFtn*)aCntntAtPos.aFnd.pAttr;
                    OUString sTmp;
                    pFtn->GetFtnText( sTmp );
                    sTxt = SW_RESSTR( pFtn->IsEndNote()
                                    ? STR_ENDNOTE : STR_FTNNOTE ) + sTmp;
                    bBalloon = sal_True;
                    if( aCntntAtPos.IsInRTLText() )
                        nStyle |= QUICKHELP_BIDI_RTL;
                }
                break;

            case SwContentAtPos::SW_REDLINE:
                sTxt = lcl_GetRedlineHelp(*aCntntAtPos.aFnd.pRedl, bBalloon);
                break;

            case SwContentAtPos::SW_TOXMARK:
                sTxt = aCntntAtPos.sStr;
                if( !sTxt.isEmpty() && aCntntAtPos.pFndTxtAttr )
                {
                    const SwTOXType* pTType = aCntntAtPos.pFndTxtAttr->
                                        GetTOXMark().GetTOXType();
                    if( pTType && !pTType->GetTypeName().isEmpty() )
                    {
                        sTxt = ": " + sTxt;
                        sTxt = pTType->GetTypeName() + sTxt;
                    }
                }
                break;
            case SwContentAtPos::SW_REFMARK:
                if(aCntntAtPos.aFnd.pAttr)
                {
                    sTxt = SW_RESSTR(STR_CONTENT_TYPE_SINGLE_REFERENCE);
                    sTxt += ": ";
                    sTxt += ((const SwFmtRefMark*)aCntntAtPos.aFnd.pAttr)->GetRefName();
                }
            break;

            default:
                {
                    SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();
                    if(!pModOpt->IsHideFieldTips())
                    {
                        const SwField* pFld = aCntntAtPos.aFnd.pFld;
                        switch( pFld->Which() )
                        {
                        case RES_SETEXPFLD:
                        case RES_TABLEFLD:
                        case RES_GETEXPFLD:
                        {
                            sal_uInt16 nOldSubType = pFld->GetSubType();
                            ((SwField*)pFld)->SetSubType(nsSwExtendedSubType::SUB_CMD);
                            sTxt = pFld->ExpandField(true);
                            ((SwField*)pFld)->SetSubType(nOldSubType);
                        }
                        break;

                        case RES_POSTITFLD:
                            {
                                break;
                            }
                        case RES_INPUTFLD:  // BubbleHelp, because the suggestion could be quite long
                            bBalloon = sal_True;
                            /* no break */
                        case RES_JUMPEDITFLD:
                            sTxt = pFld->GetPar2();
                            break;

                        case RES_DBFLD:
                            sTxt = pFld->GetFieldName();
                            break;

                        case RES_USERFLD:
                        case RES_HIDDENTXTFLD:
                            sTxt = pFld->GetPar1();
                            break;

                        case RES_DOCSTATFLD:
                            break;

                        case RES_MACROFLD:
                            sTxt = ((const SwMacroField*)pFld)->GetMacro();
                            break;

                        case RES_GETREFFLD:
                        {
                            // #i85090#
                            const SwGetRefField* pRefFld( dynamic_cast<const SwGetRefField*>(pFld) );
                            OSL_ENSURE( pRefFld,
                                    "<SwEditWin::RequestHelp(..)> - unexpected type of <pFld>" );
                            if ( pRefFld )
                            {
                                if ( pRefFld->IsRefToHeadingCrossRefBookmark() ||
                                     pRefFld->IsRefToNumItemCrossRefBookmark() )
                                {
                                    sTxt = pRefFld->GetExpandedTxtOfReferencedTxtNode();
                                    if ( sTxt.getLength() > 80  )
                                    {
                                        sTxt = sTxt.copy(0, 80) + "...";
                                    }
                                }
                                else
                                {
                                    sTxt = ((SwGetRefField*)pFld)->GetSetRefName();
                                }
                            }
                        }
                        break;
                        }
                    }

                    if( sTxt.isEmpty() )
                    {
                        aCntntAtPos.eCntntAtPos = SwContentAtPos::SW_REDLINE;
                        if( rSh.GetContentAtPos( aPos, aCntntAtPos, sal_False, &aFldRect ) )
                            sTxt = lcl_GetRedlineHelp(*aCntntAtPos.aFnd.pRedl, bBalloon);
                    }
                }
            }
            if (!sTxt.isEmpty())
            {
                if( bBalloon )
                    Help::ShowBalloon( this, rEvt.GetMousePosPixel(), sTxt );
                else
                {
                    // the show the help
                    Rectangle aRect( aFldRect.SVRect() );
                    Point aPt( OutputToScreenPixel( LogicToPixel( aRect.TopLeft() )));
                    aRect.Left()   = aPt.X();
                    aRect.Top()    = aPt.Y();
                    aPt = OutputToScreenPixel( LogicToPixel( aRect.BottomRight() ));
                    aRect.Right()  = aPt.X();
                    aRect.Bottom() = aPt.Y();
                    Help::ShowQuickHelp( this, aRect, sTxt, nStyle );
                }
            }

            bContinue = false;
        }
        if( bContinue )
        {
            sal_uInt8 nTabCols = rSh.WhichMouseTabCol(aPos);
            sal_uInt16 nTabRes = 0;
            switch(nTabCols)
            {
                case SW_TABCOL_HORI:
                case SW_TABCOL_VERT:
                    nTabRes = STR_TABLE_COL_ADJUST;
                    break;
                case SW_TABROW_HORI:
                case SW_TABROW_VERT:
                    nTabRes = STR_TABLE_ROW_ADJUST;
                    break;
                // #i32329# Enhanced table selection
                case SW_TABSEL_HORI:
                case SW_TABSEL_HORI_RTL:
                case SW_TABSEL_VERT:
                    nTabRes = STR_TABLE_SELECT_ALL;
                    break;
                case SW_TABROWSEL_HORI:
                case SW_TABROWSEL_HORI_RTL:
                case SW_TABROWSEL_VERT:
                    nTabRes = STR_TABLE_SELECT_ROW;
                    break;
                case SW_TABCOLSEL_HORI:
                case SW_TABCOLSEL_VERT:
                    nTabRes = STR_TABLE_SELECT_COL;
                    break;
            }
            if(nTabRes)
            {
                sTxt = SW_RESSTR(nTabRes);
                Size aTxtSize( GetTextWidth(sTxt), GetTextHeight());
                Rectangle aRect(rEvt.GetMousePosPixel(), aTxtSize);
                Help::ShowQuickHelp(this, aRect, sTxt);
            }
            bContinue = false;
        }
    }

    if( bContinue && pSdrView && bQuickBalloon)
    {
        SdrViewEvent aVEvt;
        SdrHitKind eHit = pSdrView->PickAnything(aPos, aVEvt);
        const SvxURLField *pField;
        SdrObject* pObj = NULL;

        if ((pField = aVEvt.pURLField) != 0)
        {
            // hit an URL field
            if (pField)
            {
                pObj = aVEvt.pObj;
                sTxt = pField->GetURL();

                bContinue = false;
            }
        }
        if (bContinue && eHit == SDRHIT_TEXTEDIT)
        {
            // look for URL field in DrawText object that is opened for editing
            OutlinerView* pOLV = pSdrView->GetTextEditOutlinerView();
            const SvxFieldItem* pFieldItem;

            if (pSdrView->AreObjectsMarked())
            {
                const SdrMarkList& rMarkList = pSdrView->GetMarkedObjectList();

                if (rMarkList.GetMarkCount() == 1)
                    pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
            }

            if (pObj && pObj->ISA(SdrTextObj) && pOLV &&
                    (pFieldItem = pOLV->GetFieldUnderMousePointer()) != 0)
            {
                pField = dynamic_cast<const SvxURLField*>(pFieldItem->GetField());

                if (pField )
                {
                    sTxt = ((const SvxURLField*) pField)->GetURL();
                    bContinue = false;
                }
            }
        }
        if (!sTxt.isEmpty() && pObj)
        {
            sTxt = URIHelper::removePassword( sTxt, INetURLObject::WAS_ENCODED,
                                           INetURLObject::DECODE_UNAMBIGUOUS);

            Rectangle aLogicPix = LogicToPixel(pObj->GetLogicRect());
            Rectangle aScreenRect(OutputToScreenPixel(aLogicPix.TopLeft()),
                                OutputToScreenPixel(aLogicPix.BottomRight()));

            if (bBalloon)
                Help::ShowBalloon(this, rEvt.GetMousePosPixel(), aScreenRect, sTxt);
            else
                Help::ShowQuickHelp(this, aScreenRect, sTxt);
        }
    }

    if( bContinue )
        Window::RequestHelp( rEvt );
}

void SwEditWin::PrePaint()
{
    SwWrtShell* pWrtShell = GetView().GetWrtShellPtr();

    if(pWrtShell)
    {
        pWrtShell->PrePaint();
    }
}

void  SwEditWin::Paint(const Rectangle& rRect)
{
    SwWrtShell* pWrtShell = GetView().GetWrtShellPtr();
    if(!pWrtShell)
        return;
    bool bPaintShadowCrsr = false;
    if( m_pShadCrsr )
    {
        Rectangle aRect( m_pShadCrsr->GetRect());
        // fully resides inside?
        if( rRect.IsInside( aRect ) )
            // dann aufheben
            delete m_pShadCrsr, m_pShadCrsr = 0;
        else if( rRect.IsOver( aRect ))
        {
            // resides somewhat above, then everything is clipped outside
            // and we have to make the "inner part" at the end of the
            // Paint visible again. Otherwise Paint errors occur!
            bPaintShadowCrsr = true;
        }
    }

    if ( GetView().GetVisArea().GetWidth()  <= 0 ||
              GetView().GetVisArea().GetHeight() <= 0 )
        Invalidate( rRect );
    else
        pWrtShell->Paint( rRect );

    if( bPaintShadowCrsr )
        m_pShadCrsr->Paint();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
