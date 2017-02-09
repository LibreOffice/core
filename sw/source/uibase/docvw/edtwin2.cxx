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

#include <IDocumentMarkAccess.hxx>
#include <ndtxt.hxx>

static OUString lcl_GetRedlineHelp( const SwRangeRedline& rRedl, bool bBalloon )
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

OUString SwEditWin::ClipLongToolTip(const OUString& rText)
{
    OUString sDisplayText(rText);
    long nTextWidth = GetTextWidth(sDisplayText);
    long nMaxWidth = GetDesktopRectPixel().GetWidth() * 2 / 3;
    nMaxWidth = PixelToLogic(Size(nMaxWidth, 0)).Width();
    if (nTextWidth > nMaxWidth)
        sDisplayText = GetEllipsisString(sDisplayText, nMaxWidth, DrawTextFlags::CenterEllipsis);
    return sDisplayText;
}

void SwEditWin::RequestHelp(const HelpEvent &rEvt)
{
    SwWrtShell &rSh = m_rView.GetWrtShell();
    bool bQuickBalloon = bool(rEvt.GetMode() & ( HelpEventMode::QUICK | HelpEventMode::BALLOON ));
    if(bQuickBalloon && !rSh.GetViewOptions()->IsShowContentTips())
        return;
    bool bContinue = true;
    SET_CURR_SHELL(&rSh);
    OUString sText;
    Point aPos( PixelToLogic( ScreenToOutputPixel( rEvt.GetMousePosPixel() ) ));
    bool bBalloon = bool(rEvt.GetMode() & HelpEventMode::BALLOON);

    SdrView *pSdrView = rSh.GetDrawView();

    if( bQuickBalloon )
    {
        if( pSdrView )
        {
            SdrPageView* pPV = pSdrView->GetSdrPageView();
            SwDPage* pPage = pPV ? static_cast<SwDPage*>(pPV->GetPage()) : nullptr;
            bContinue = pPage && pPage->RequestHelp(this, pSdrView, rEvt);
        }
    }

    if( bContinue && bQuickBalloon)
    {
        SwRect aFieldRect;
        SwContentAtPos aContentAtPos( SwContentAtPos::SW_FIELD |
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

        if( rSh.GetContentAtPos( aPos, aContentAtPos, false, &aFieldRect ) )
        {
            QuickHelpFlags nStyle = QuickHelpFlags::NONE; // style of quick help
            switch( aContentAtPos.eContentAtPos )
            {
            case SwContentAtPos::SW_TABLEBOXFML:
                sText = "= ";
                sText += static_cast<const SwTableBoxFormula*>(aContentAtPos.aFnd.pAttr)->GetFormula();
                break;
#ifdef DBG_UTIL
            case SwContentAtPos::SW_TABLEBOXVALUE:
            {
                sText = OStringToOUString(OString::number(
                            static_cast<const SwTableBoxValue*>(aContentAtPos.aFnd.pAttr)->GetValue()),
                            osl_getThreadTextEncoding());
            }
            break;
            case SwContentAtPos::SW_CURR_ATTRS:
                sText = aContentAtPos.sStr;
                break;
#endif

            case SwContentAtPos::SW_INETATTR:
            {
                sText = static_cast<const SfxStringItem*>(aContentAtPos.aFnd.pAttr)->GetValue();
                sText = URIHelper::removePassword( sText,
                                        INetURLObject::EncodeMechanism::WasEncoded,
                                           INetURLObject::DecodeMechanism::Unambiguous);
                //#i63832# remove the link target type
                sal_Int32 nFound = sText.indexOf(cMarkSeparator);
                if( nFound != -1 && (++nFound) < sText.getLength() )
                {
                    OUString sSuffix( sText.copy(nFound) );
                    if( sSuffix == "table" ||
                        sSuffix == "frame" ||
                        sSuffix == "region" ||
                        sSuffix == "outline" ||
                        sSuffix == "text" ||
                        sSuffix == "graphic" ||
                        sSuffix == "ole" )
                    sText = sText.copy( 0, nFound - 1);
                }
                // #i104300#
                // special handling if target is a cross-reference bookmark
                {
                    OUString sTmpSearchStr = sText.copy( 1 );
                    IDocumentMarkAccess* pMarkAccess = rSh.getIDocumentMarkAccess();
                    IDocumentMarkAccess::const_iterator_t ppBkmk =
                                    pMarkAccess->findBookmark( sTmpSearchStr );
                    if ( ppBkmk != pMarkAccess->getBookmarksEnd() &&
                         IDocumentMarkAccess::GetType( *(ppBkmk->get()) )
                            == IDocumentMarkAccess::MarkType::CROSSREF_HEADING_BOOKMARK )
                    {
                        SwTextNode* pTextNode = ppBkmk->get()->GetMarkStart().nNode.GetNode().GetTextNode();
                        if ( pTextNode )
                        {
                            sText = pTextNode->GetExpandText( 0, pTextNode->Len(), true, true );

                            if( !sText.isEmpty() )
                            {
                                OUStringBuffer sTmp(sText.replaceAll(OUStringLiteral1(0xad), ""));
                                for (sal_Int32 i = 0; i < sTmp.getLength(); ++i)
                                {
                                    if (sTmp[i] < 0x20)
                                        sTmp[i] = 0x20;
                                    else if (sTmp[i] == 0x2011)
                                        sTmp[i] = '-';
                                }
                                sText = sTmp.makeStringAndClear();
                            }
                        }
                    }
                }
                // #i80029#
                bool bExecHyperlinks = m_rView.GetDocShell()->IsReadOnly();
                if ( !bExecHyperlinks )
                {
                    SvtSecurityOptions aSecOpts;
                    bExecHyperlinks = !aSecOpts.IsOptionSet( SvtSecurityOptions::EOption::CtrlClickHyperlink );

                    sText = ": " + sText;
                    if ( !bExecHyperlinks )
                        sText = SwViewShell::GetShellRes()->aLinkCtrlClick + sText;
                    else
                        sText = SwViewShell::GetShellRes()->aLinkClick + sText;
                }
                break;
            }
            case SwContentAtPos::SW_SMARTTAG:
            {
                vcl::KeyCode aCode( KEY_SPACE );
                vcl::KeyCode aModifiedCode( KEY_SPACE, KEY_MOD1 );
                OUString aModStr( aModifiedCode.GetName() );
                aModStr = aModStr.replaceFirst(aCode.GetName(), "");
                aModStr = aModStr.replaceAll("+", "");
                sText = SW_RESSTR(STR_SMARTTAG_CLICK).replaceAll("%s", aModStr);
            }
            break;

            case SwContentAtPos::SW_FTN:
                if( aContentAtPos.pFndTextAttr && aContentAtPos.aFnd.pAttr )
                {
                    const SwFormatFootnote* pFootnote = static_cast<const SwFormatFootnote*>(aContentAtPos.aFnd.pAttr);
                    OUString sTmp;
                    pFootnote->GetFootnoteText( sTmp );
                    sText = SW_RESSTR( pFootnote->IsEndNote()
                                    ? STR_ENDNOTE : STR_FTNNOTE ) + sTmp;
                    bBalloon = true;
                    if( aContentAtPos.IsInRTLText() )
                        nStyle |= QuickHelpFlags::BiDiRtl;
                }
                break;

            case SwContentAtPos::SW_REDLINE:
                sText = lcl_GetRedlineHelp(*aContentAtPos.aFnd.pRedl, bBalloon);
                break;

            case SwContentAtPos::SW_TOXMARK:
                sText = aContentAtPos.sStr;
                if( !sText.isEmpty() && aContentAtPos.pFndTextAttr )
                {
                    const SwTOXType* pTType = aContentAtPos.pFndTextAttr->
                                        GetTOXMark().GetTOXType();
                    if( pTType && !pTType->GetTypeName().isEmpty() )
                    {
                        sText = ": " + sText;
                        sText = pTType->GetTypeName() + sText;
                    }
                }
                break;
            case SwContentAtPos::SW_REFMARK:
                if(aContentAtPos.aFnd.pAttr)
                {
                    sText = SW_RESSTR(STR_CONTENT_TYPE_SINGLE_REFERENCE);
                    sText += ": ";
                    sText += static_cast<const SwFormatRefMark*>(aContentAtPos.aFnd.pAttr)->GetRefName();
                }
            break;

            default:
                {
                    SwModuleOptions* pModOpt = SW_MOD()->GetModuleConfig();
                    if(!pModOpt->IsHideFieldTips())
                    {
                        const SwField* pField = aContentAtPos.aFnd.pField;
                        switch( pField->Which() )
                        {
                        case RES_SETEXPFLD:
                        case RES_TABLEFLD:
                        case RES_GETEXPFLD:
                        {
                            sal_uInt16 nOldSubType = pField->GetSubType();
                            const_cast<SwField*>(pField)->SetSubType(nsSwExtendedSubType::SUB_CMD);
                            sText = pField->ExpandField(true);
                            const_cast<SwField*>(pField)->SetSubType(nOldSubType);
                        }
                        break;

                        case RES_POSTITFLD:
                            {
                                break;
                            }
                        case RES_INPUTFLD:  // BubbleHelp, because the suggestion could be quite long
                            bBalloon = true;
                            SAL_FALLTHROUGH;
                        case RES_JUMPEDITFLD:
                            sText = pField->GetPar2();
                            break;

                        case RES_DBFLD:
                            sText = pField->GetFieldName();
                            break;

                        case RES_USERFLD:
                        case RES_HIDDENTXTFLD:
                            sText = pField->GetPar1();
                            break;

                        case RES_DOCSTATFLD:
                            break;

                        case RES_MACROFLD:
                            sText = static_cast<const SwMacroField*>(pField)->GetMacro();
                            break;

                        case RES_GETREFFLD:
                        {
                            // #i85090#
                            const SwGetRefField* pRefField( dynamic_cast<const SwGetRefField*>(pField) );
                            OSL_ENSURE( pRefField,
                                    "<SwEditWin::RequestHelp(..)> - unexpected type of <pField>" );
                            if ( pRefField )
                            {
                                if ( pRefField->IsRefToHeadingCrossRefBookmark() ||
                                     pRefField->IsRefToNumItemCrossRefBookmark() )
                                {
                                    sText = pRefField->GetExpandedTextOfReferencedTextNode();
                                    if ( sText.getLength() > 80  )
                                    {
                                        sText = sText.copy(0, 80) + "...";
                                    }
                                }
                                else
                                {
                                    sText = static_cast<const SwGetRefField*>(pField)->GetSetRefName();
                                }
                            }
                        }
                        break;
                        }
                    }

                    if( sText.isEmpty() )
                    {
                        aContentAtPos.eContentAtPos = SwContentAtPos::SW_REDLINE;
                        if( rSh.GetContentAtPos( aPos, aContentAtPos, false, &aFieldRect ) )
                            sText = lcl_GetRedlineHelp(*aContentAtPos.aFnd.pRedl, bBalloon);
                    }
                }
            }
            if (!sText.isEmpty())
            {
                Rectangle aRect( aFieldRect.SVRect() );
                Point aPt( OutputToScreenPixel( LogicToPixel( aRect.TopLeft() )));
                aRect.Left()   = aPt.X();
                aRect.Top()    = aPt.Y();
                aPt = OutputToScreenPixel( LogicToPixel( aRect.BottomRight() ));
                aRect.Right()  = aPt.X();
                aRect.Bottom() = aPt.Y();

                if( bBalloon )
                    Help::ShowBalloon( this, rEvt.GetMousePosPixel(), aRect, sText );
                else
                {
                    // the show the help
                    OUString sDisplayText(ClipLongToolTip(sText));
                    Help::ShowQuickHelp(this, aRect, sDisplayText, nStyle);
                }
            }

            bContinue = false;
        }
        if( bContinue )
        {
            SwTab nTabCols = rSh.WhichMouseTabCol(aPos);
            sal_uInt16 nTabRes = 0;
            switch(nTabCols)
            {
                case SwTab::COL_HORI:
                case SwTab::COL_VERT:
                    nTabRes = STR_TABLE_COL_ADJUST;
                    break;
                case SwTab::ROW_HORI:
                case SwTab::ROW_VERT:
                    nTabRes = STR_TABLE_ROW_ADJUST;
                    break;
                // #i32329# Enhanced table selection
                case SwTab::SEL_HORI:
                case SwTab::SEL_HORI_RTL:
                case SwTab::SEL_VERT:
                    nTabRes = STR_TABLE_SELECT_ALL;
                    break;
                case SwTab::ROWSEL_HORI:
                case SwTab::ROWSEL_HORI_RTL:
                case SwTab::ROWSEL_VERT:
                    nTabRes = STR_TABLE_SELECT_ROW;
                    break;
                case SwTab::COLSEL_HORI:
                case SwTab::COLSEL_VERT:
                    nTabRes = STR_TABLE_SELECT_COL;
                    break;
                case SwTab::COL_NONE: break; // prevent compiler warning
            }
            if(nTabRes)
            {
                sText = SW_RESSTR(nTabRes);
                Size aTextSize( GetTextWidth(sText), GetTextHeight());
                Rectangle aRect(rEvt.GetMousePosPixel(), aTextSize);
                OUString sDisplayText(ClipLongToolTip(sText));
                Help::ShowQuickHelp(this, aRect, sDisplayText);
            }
            bContinue = false;
        }
    }

    if( bContinue )
        Window::RequestHelp( rEvt );
}

void SwEditWin::PrePaint(vcl::RenderContext& /*rRenderContext*/)
{
    SwWrtShell* pWrtShell = GetView().GetWrtShellPtr();

    if(pWrtShell)
    {
        pWrtShell->PrePaint();
    }
}

void SwEditWin::Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect)
{
    SwWrtShell* pWrtShell = GetView().GetWrtShellPtr();
    if(!pWrtShell)
        return;
    bool bPaintShadowCursor = false;
    if( m_pShadCursor )
    {
        Rectangle aRect( m_pShadCursor->GetRect());
        // fully resides inside?
        if( rRect.IsInside( aRect ) )
        {
            // dann aufheben
            delete m_pShadCursor;
            m_pShadCursor = nullptr;
        }
        else if( rRect.IsOver( aRect ))
        {
            // resides somewhat above, then everything is clipped outside
            // and we have to make the "inner part" at the end of the
            // Paint visible again. Otherwise Paint errors occur!
            bPaintShadowCursor = true;
        }
    }

    if ( GetView().GetVisArea().GetWidth()  <= 0 ||
              GetView().GetVisArea().GetHeight() <= 0 )
        Invalidate( rRect );
    else
    {
        pWrtShell->setOutputToWindow(true);
        pWrtShell->Paint(rRenderContext, rRect);
        pWrtShell->setOutputToWindow(false);
    }

    if( bPaintShadowCursor )
        m_pShadCursor->Paint();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
