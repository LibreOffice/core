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

#include <memory>
#include <config_features.h>

#include <hintids.hxx>
#include <com/sun/star/linguistic2/XThesaurus.hpp>
#include <svl/aeitem.hxx>
#include <svl/whiter.hxx>
#include <svl/cjkoptions.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/objitem.hxx>
#include <svl/imageitm.hxx>
#include <svl/languageoptions.hxx>
#include <editeng/protitem.hxx>
#include <sfx2/linkmgr.hxx>
#include <editeng/langitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/tstpitem.hxx>
#include <sfx2/htmlmode.hxx>
#include <editeng/unolingu.hxx>
#include <sfx2/msgpool.hxx>
#include <swmodule.hxx>
#include <tox.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/app.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <basesh.hxx>
#include <uitool.hxx>
#include <viewopt.hxx>
#include <tablemgr.hxx>
#include <pagedesc.hxx>
#include <wview.hxx>
#include <globdoc.hxx>
#include <svl/stritem.hxx>
#include <unotools/moduleoptions.hxx>
#include <comphelper/lok.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <svl/visitem.hxx>
#include <redline.hxx>
#include <docary.hxx>

#include <cmdid.h>
#include <IDocumentRedlineAccess.hxx>

#include <doc.hxx>

using namespace ::com::sun::star;

void SwView::GetState(SfxItemSet &rSet)
{
    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();
    FrameTypeFlags eFrameType = FrameTypeFlags::NONE;
    bool bGetFrameType = false;
    bool bWeb = dynamic_cast<SwWebView*>( this ) !=  nullptr;

    while(nWhich)
    {
        switch(nWhich)
        {
            case FN_NAV_ELEMENT:
                // used to update all instances of this control
                rSet.InvalidateItem( nWhich );
            break;
            case FN_EDIT_LINK_DLG:
                if( m_pWrtShell->GetLinkManager().GetLinks().empty() )
                    rSet.DisableItem(nWhich);
                else if( m_pWrtShell->IsSelFrameMode() &&
                    m_pWrtShell->IsSelObjProtected(FlyProtectFlags::Content) != FlyProtectFlags::NONE)
                {
                    rSet.DisableItem(nWhich);
                }
                break;

            case SID_DRAWTBX_LINES:
                if ( bWeb )
                    rSet.DisableItem(nWhich);
                break;

            case SID_INSERT_GRAPHIC:
                if( m_pWrtShell->CursorInsideInputField() )
                {
                    rSet.DisableItem(nWhich);
                }
                break;
            case SID_INSERT_SIGNATURELINE:
                if( m_nSelectionType != SelectionType::Text )
                {
                    rSet.DisableItem(nWhich);
                }
                break;
            case SID_EDIT_SIGNATURELINE:
            case SID_SIGN_SIGNATURELINE:
                if (!isSignatureLineSelected())
                    rSet.DisableItem(nWhich);
                break;
            case FN_INSERT_CAPTION:
                {
                    // There are captions for graphics, OLE objects, frames and tables
                    if( !bGetFrameType )
                    {
                        eFrameType = m_pWrtShell->GetFrameType(nullptr, true);
                        bGetFrameType = true;
                    }
                    if (! ( ((eFrameType & FrameTypeFlags::FLY_ANY) && m_nSelectionType != SelectionType::DrawObjectEditMode)||
                        m_nSelectionType & SelectionType::Table ||
                        m_nSelectionType & SelectionType::DrawObject) )
                    {
                        rSet.DisableItem(nWhich);
                    }
                    else if((m_pWrtShell->IsObjSelected() || m_pWrtShell->IsFrameSelected()) &&
                        (m_pWrtShell->IsSelObjProtected( FlyProtectFlags::Parent) != FlyProtectFlags::NONE ||
                        m_pWrtShell->IsSelObjProtected( FlyProtectFlags::Content ) != FlyProtectFlags::NONE))
                    {
                        rSet.DisableItem(nWhich);
                    }
                    else if( m_pWrtShell->IsTableMode()
                        || isSignatureLineSelected()
                        || m_pWrtShell->CursorInsideInputField() )
                    {
                        rSet.DisableItem(nWhich);
                    }
                }
                break;

            case FN_EDIT_FOOTNOTE:
            {
                if( !m_pWrtShell->GetCurFootnote() )
                    rSet.DisableItem(nWhich);
            }
            break;

            case FN_CHANGE_PAGENUM:
            {
                FrameTypeFlags nType = m_pWrtShell->GetFrameType(nullptr,true);
                if( ( FrameTypeFlags::FLY_ANY | FrameTypeFlags::HEADER | FrameTypeFlags::FOOTER |
                      FrameTypeFlags::FOOTNOTE | FrameTypeFlags::DRAWOBJ ) & nType )
                    rSet.DisableItem(nWhich);
                else
                    rSet.Put(SfxUInt16Item(nWhich, m_pWrtShell->GetPageOffset()));
            }
            break;
            case SID_PRINTDOC:
            case SID_PRINTDOCDIRECT:
                GetSlotState( nWhich, SfxViewShell::GetInterface(), &rSet );
            break;
            case SID_ATTR_PAGE_ORIENTATION:
            case SID_ATTR_PAGE:
            case SID_ATTR_PAGE_SIZE:
            case SID_ATTR_PAGE_PAPERBIN:
            case RES_PAPER_BIN:
            case FN_PARAM_FTN_INFO:
            {
                const size_t nCurIdx = m_pWrtShell->GetCurPageDesc();
                const SwPageDesc& rDesc = m_pWrtShell->GetPageDesc( nCurIdx );

                // set correct parent to get the XFILL_NONE FillStyle as needed
                if(!rSet.GetParent())
                {
                    const SwFrameFormat& rMaster = rDesc.GetMaster();

                    rSet.SetParent(&rMaster.GetDoc()->GetDfltFrameFormat()->GetAttrSet());
                }

                ::PageDescToItemSet( rDesc, rSet);
            }
            break;
            case RES_BACKGROUND:
            case SID_ATTR_BRUSH:
            {
                const size_t nCurIdx = m_pWrtShell->GetCurPageDesc();
                const SwPageDesc& rDesc = m_pWrtShell->GetPageDesc( nCurIdx );
                const SwFrameFormat& rMaster = rDesc.GetMaster();
                const SvxBrushItem& rBrush = rMaster.GetFormatAttr(RES_BACKGROUND);
                rSet.Put(rBrush);
            }
            break;
            case SID_CLEARHISTORY:
            {
                rSet.Put(SfxBoolItem(nWhich, m_pWrtShell->GetLastUndoInfo(nullptr, nullptr)));
            }
            break;
            case SID_UNDO:
            {
                // which must not be present, so let them create:
                if( !m_pShell )
                    SelectShell();

                const SfxPoolItem* pState = m_pShell->GetSlotState(SID_UNDO);
                if(pState)
                    rSet.Put(*pState);
                else
                    rSet.DisableItem(nWhich);
            }
            break;
            case FN_INSERT_OBJ_CTRL:
                if( bWeb
                    || m_pWrtShell->CursorInsideInputField() )
                {
                    rSet.DisableItem(nWhich);
                }
                break;

            case FN_UPDATE_TOX:
                if(!m_pWrtShell->GetTOXCount())
                    rSet.DisableItem(nWhich);
            break;
            case FN_EDIT_CURRENT_TOX:
            case FN_UPDATE_CUR_TOX:
            {
                const SwTOXBase* pBase = nullptr;
                if(nullptr == (pBase = m_pWrtShell->GetCurTOX()) ||
                    (FN_EDIT_CURRENT_TOX == nWhich && pBase->IsTOXBaseInReadonly()))
                    rSet.DisableItem(nWhich);
            }
            break;
            case SID_TWAIN_SELECT:
            case SID_TWAIN_TRANSFER:
#if defined(_WIN32) || defined UNX
            {
                if(!SW_MOD()->GetScannerManager().is())
                    rSet.DisableItem(nWhich);
            }
#endif
            break;
            case RES_PARATR_TABSTOP:
            case SID_ATTR_DEFTABSTOP:
            {
                const SvxTabStopItem& rDefTabs = m_pWrtShell->GetDefault(RES_PARATR_TABSTOP);
                rSet.Put( SfxUInt16Item( nWhich,
                                                static_cast<sal_uInt16>(::GetTabDist(rDefTabs))));
            }
            break;
            case SID_ATTR_LANGUAGE:
            {
                std::unique_ptr<SfxPoolItem> pNewItem(
                    m_pWrtShell->GetDefault(RES_CHRATR_LANGUAGE).CloneSetWhich(SID_ATTR_LANGUAGE));
                rSet.Put(*pNewItem);
            }
            break;
            case RES_CHRATR_CJK_LANGUAGE:
            {
                std::unique_ptr<SfxPoolItem> pNewItem(
                    m_pWrtShell->GetDefault(RES_CHRATR_CJK_LANGUAGE).CloneSetWhich(RES_CHRATR_CJK_LANGUAGE));
                rSet.Put(*pNewItem);
            }
            break;
            case RES_CHRATR_CTL_LANGUAGE:
            {
                std::unique_ptr<SfxPoolItem> pNewItem(
                    m_pWrtShell->GetDefault(RES_CHRATR_CTL_LANGUAGE).CloneSetWhich(RES_CHRATR_CTL_LANGUAGE));
                rSet.Put(*pNewItem);
            }
            break;
            case FN_REDLINE_ON:
                rSet.Put( SfxBoolItem( nWhich, GetDocShell()->IsChangeRecording() ) );
            break;
            case FN_REDLINE_PROTECT :
                rSet.Put( SfxBoolItem( nWhich, GetDocShell()->HasChangeRecordProtection() ) );
            break;
            case FN_REDLINE_SHOW:
            {
                rSet.Put( SfxBoolItem( nWhich, IDocumentRedlineAccess::IsShowChanges(m_pWrtShell->GetRedlineFlags()) ));
            }
            break;
            case SID_AVMEDIA_PLAYER :
            case FN_REDLINE_ACCEPT :
            {
                SfxViewFrame* pVFrame = GetViewFrame();
                if (pVFrame->KnowsChildWindow(nWhich))
                    rSet.Put(SfxBoolItem( nWhich, pVFrame->HasChildWindow(nWhich)));
                else
                    rSet.DisableItem(nWhich);
            }
            break;
            case FN_REDLINE_ACCEPT_DIRECT:
            case FN_REDLINE_REJECT_DIRECT:
            {
                SwDoc *pDoc = m_pWrtShell->GetDoc();
                SwPaM *pCursor = m_pWrtShell->GetCursor();
                bool bDisable = false;
                if (GetDocShell()->HasChangeRecordProtection())
                    bDisable = true;
                else if (pCursor->HasMark())
                {
                    // If the selection does not contain redlines, disable accepting/rejecting changes.
                    SwRedlineTable::size_type index = 0;
                    const SwRedlineTable& table = pDoc->getIDocumentRedlineAccess().GetRedlineTable();
                    const SwRangeRedline* redline = table.FindAtPosition( *pCursor->Start(), index );
                    if( redline != nullptr && *redline->Start() == *pCursor->End())
                        redline = nullptr;
                    if( redline == nullptr )
                    {
                        for(; index < table.size(); ++index )
                        {
                            const SwRangeRedline* tmp = table[ index ];
                            if( *tmp->Start() >= *pCursor->End())
                                break;
                            if( tmp->HasMark() && tmp->IsVisible())
                            {
                                redline = tmp;
                                break;
                            }
                        }
                    }
                    if( redline == nullptr )
                        bDisable = true;
                }
                else
                {
                    // If the cursor position isn't on a redline, disable
                    // accepting/rejecting changes.
                    if (nullptr == pDoc->getIDocumentRedlineAccess().GetRedline(*pCursor->Start(), nullptr))
                        bDisable = true;
                }

                // LibreOfficeKit wants to handle changes by index, so always allow here.
                if (bDisable)
                    rSet.DisableItem(nWhich);
                if (comphelper::LibreOfficeKit::isActive())
                {
                    OString aPayload(".uno:TrackedChangeIndex=");
                    SwRedlineTable::size_type nRedline = 0;
                    if (pDoc->getIDocumentRedlineAccess().GetRedline(*pCursor->Start(), &nRedline))
                        aPayload += OString::number(nRedline);
                    libreOfficeKitViewCallback(LOK_CALLBACK_STATE_CHANGED, aPayload.getStr());
                }
            }
            break;

            case FN_REDLINE_NEXT_CHANGE:
            case FN_REDLINE_PREV_CHANGE:
            {
                // Enable change navigation if we have any redlines. Ideally we should disable
                // "Next Change" if we're at or past the last change, and similarly for
                // "Previous Change"
                if (0 == m_pWrtShell->GetRedlineCount())
                    rSet.DisableItem(nWhich);
            }
            break;

            case SID_THESAURUS:
            {
                SwWrtShell  &rSh = GetWrtShell();
                if (2 <= rSh.GetCursorCnt())  // multi selection?
                    rSet.DisableItem(nWhich);
                else
                {
                    LanguageType nLang = rSh.GetCurLang();

                    // disable "Thesaurus" (menu entry and key shortcut) if the
                    // language is not supported (by default it is enabled)
                    uno::Reference< linguistic2::XThesaurus >  xThes( ::GetThesaurus() );
                    if (!xThes.is() || nLang == LANGUAGE_NONE ||
                        !xThes->hasLocale( LanguageTag::convertToLocale( nLang ) ))
                        rSet.DisableItem(nWhich);
                }
            }
            break;
            case SID_HANGUL_HANJA_CONVERSION:
            case SID_CHINESE_CONVERSION:
            {
                if (!SvtCJKOptions().IsAnyEnabled())
                {
                    GetViewFrame()->GetBindings().SetVisibleState( nWhich, false );
                    rSet.DisableItem(nWhich);
                }
                else
                    GetViewFrame()->GetBindings().SetVisibleState( nWhich, true );
            }
            break;
            case SID_MAIL_SCROLLBODY_PAGEDOWN:
                {
                    const long nBottom = m_pWrtShell->GetDocSize().Height() + DOCUMENTBORDER;
                    const long nAct = GetVisArea().Bottom();
                    rSet.Put(SfxBoolItem(SID_MAIL_SCROLLBODY_PAGEDOWN, nAct < nBottom ));
                }
                break;

            case SID_DOCUMENT_COMPARE:
            case SID_DOCUMENT_MERGE:
                if( dynamic_cast<const SwGlobalDocShell* >(GetDocShell()) != nullptr||
                    (SID_DOCUMENT_MERGE == nWhich && m_pWrtShell->getIDocumentRedlineAccess().GetRedlinePassword().getLength()))
                    rSet.DisableItem(nWhich);
            break;
            case  SID_VIEW_DATA_SOURCE_BROWSER:
                if ( !SvtModuleOptions().IsModuleInstalled( SvtModuleOptions::EModule::DATABASE ) )
                    rSet.Put( SfxVisibilityItem( nWhich, false ) );
                else
                    rSet.Put( SfxBoolItem( nWhich, GetViewFrame()->HasChildWindow( SID_BROWSER ) ) );
            break;
            case SID_READONLY_MODE:
                rSet.Put(SfxBoolItem(nWhich,
                    m_pWrtShell->HasReadonlySel()||GetDocShell()->IsReadOnly()));
            break;
            case SID_IMAGE_ORIENTATION:
            {
                SfxImageItem aImageItem(nWhich);
                if(m_pWrtShell->IsInVerticalText())
                    aImageItem.SetRotation( 2700 );
                if(m_pWrtShell->IsInRightToLeftText())
                    aImageItem.SetMirrored( true );
                rSet.Put(aImageItem);
            }
            break;
            case FN_INSERT_FIELD_DATA_ONLY :
                if(!m_bInMailMerge && !GetViewFrame()->HasChildWindow(nWhich))
                    rSet.DisableItem(nWhich);
            break;
            case FN_MAILMERGE_SENDMAIL_CHILDWINDOW:
            break;
            case SID_ALIGN_ANY_LEFT :
            case SID_ALIGN_ANY_HCENTER  :
            case SID_ALIGN_ANY_RIGHT    :
            case SID_ALIGN_ANY_JUSTIFIED:
            case SID_ALIGN_ANY_TOP      :
            case SID_ALIGN_ANY_VCENTER  :
            case SID_ALIGN_ANY_BOTTOM   :
            case SID_ALIGN_ANY_HDEFAULT :
            case SID_ALIGN_ANY_VDEFAULT :
            {
                if( !m_pShell )
                    SelectShell();
                sal_uInt16 nAlias = 0;
                if( m_nSelectionType & (SelectionType::DrawObjectEditMode|SelectionType::Text) )
                {
                    switch( nWhich )
                    {
                        case SID_ALIGN_ANY_LEFT :       nAlias = SID_ATTR_PARA_ADJUST_LEFT; break;
                        case SID_ALIGN_ANY_HCENTER  :   nAlias = SID_ATTR_PARA_ADJUST_CENTER; break;
                        case SID_ALIGN_ANY_RIGHT    :   nAlias = SID_ATTR_PARA_ADJUST_RIGHT; break;
                        case SID_ALIGN_ANY_JUSTIFIED:   nAlias = SID_ATTR_PARA_ADJUST_BLOCK; break;
                        case SID_ALIGN_ANY_TOP      :   nAlias = SID_TABLE_VERT_NONE; break;
                        case SID_ALIGN_ANY_VCENTER  :   nAlias = SID_TABLE_VERT_CENTER; break;
                        case SID_ALIGN_ANY_BOTTOM   :   nAlias = SID_TABLE_VERT_BOTTOM; break;
                    }
                }
                else
                {
                    switch( nWhich )
                    {
                        case SID_ALIGN_ANY_LEFT :       nAlias = SID_OBJECT_ALIGN_LEFT    ; break;
                        case SID_ALIGN_ANY_HCENTER  :   nAlias = SID_OBJECT_ALIGN_CENTER ; break;
                        case SID_ALIGN_ANY_RIGHT    :   nAlias = SID_OBJECT_ALIGN_RIGHT  ; break;
                        case SID_ALIGN_ANY_TOP      :   nAlias = SID_OBJECT_ALIGN_UP     ;  break;
                        case SID_ALIGN_ANY_VCENTER  :   nAlias = SID_OBJECT_ALIGN_MIDDLE ;  break;
                        case SID_ALIGN_ANY_BOTTOM   :   nAlias = SID_OBJECT_ALIGN_DOWN    ; break;
                    }
                }
                //these slots are either re-mapped to text or object alignment
                const SfxPoolItem* pState = nullptr;
                if(nAlias)
                    GetViewFrame()->GetDispatcher()->QueryState( nAlias, pState );
                if(pState)
                {
                    if (!(m_nSelectionType & SelectionType::DrawObject))
                    {
                        std::unique_ptr<SfxPoolItem> pNewItem(pState->CloneSetWhich(nWhich));
                        rSet.Put(*pNewItem);
                    }
                }
                else
                    rSet.DisableItem(nWhich);
            }
            break;
        }
        nWhich = aIter.NextWhich();
    }
}

void SwView::GetDrawState(SfxItemSet &rSet)
{
    SfxWhichIter aIter(rSet);
    bool bWeb = dynamic_cast<SwWebView*>( this ) !=  nullptr;

    for( sal_uInt16 nWhich = aIter.FirstWhich(); nWhich;
                                            nWhich = aIter.NextWhich() )
        switch(nWhich)
        {
        case SID_DRAW_LINE:
        case SID_DRAW_XLINE:
        case SID_LINE_ARROW_END:
        case SID_LINE_ARROW_CIRCLE:
        case SID_LINE_ARROW_SQUARE:
        case SID_LINE_ARROW_START:
        case SID_LINE_CIRCLE_ARROW:
        case SID_LINE_SQUARE_ARROW:
        case SID_LINE_ARROWS:
        case SID_DRAW_MEASURELINE:
        case SID_DRAW_RECT:
        case SID_DRAW_ELLIPSE:
        case SID_DRAW_XPOLYGON_NOFILL:
        case SID_DRAW_XPOLYGON:
        case SID_DRAW_POLYGON_NOFILL:
        case SID_DRAW_POLYGON:
        case SID_DRAW_BEZIER_NOFILL:
        case SID_DRAW_BEZIER_FILL:
        case SID_DRAW_FREELINE_NOFILL:
        case SID_DRAW_FREELINE:
        case SID_DRAW_ARC:
        case SID_DRAW_PIE:
        case SID_DRAW_CIRCLECUT:
        case SID_DRAW_TEXT:
        case SID_DRAW_CAPTION:
            if ( bWeb )
                rSet.DisableItem( nWhich );
            else
                rSet.Put( SfxBoolItem( nWhich, m_nDrawSfxId == nWhich ) );
            break;

        case SID_DRAW_TEXT_VERTICAL:
        case SID_DRAW_CAPTION_VERTICAL:
            if ( bWeb || !SvtLanguageOptions().IsVerticalTextEnabled() )
                rSet.DisableItem( nWhich );
            else
                rSet.Put( SfxBoolItem( nWhich, m_nDrawSfxId == nWhich ) );
            break;

        case SID_DRAW_TEXT_MARQUEE:
            if (::GetHtmlMode(GetDocShell()) & HTMLMODE_SOME_STYLES)
                rSet.Put( SfxBoolItem(nWhich, m_nDrawSfxId == nWhich));
            else
                rSet.DisableItem(nWhich);
            break;
        case SID_OBJECT_SELECT:
            rSet.Put( SfxBoolItem(nWhich, m_nDrawSfxId == nWhich ||
                                          m_nFormSfxId == nWhich));
            break;

        case SID_INSERT_DRAW:
        case SID_FONTWORK_GALLERY_FLOATER :
        case SID_DRAWTBX_ARROWS:
        {
            if ( bWeb )
                rSet.DisableItem( nWhich );
        }
        break;

        case SID_DRAWTBX_CS_BASIC :
        case SID_DRAWTBX_CS_SYMBOL :
        case SID_DRAWTBX_CS_ARROW :
        case SID_DRAWTBX_CS_FLOWCHART :
        case SID_DRAWTBX_CS_CALLOUT :
        case SID_DRAWTBX_CS_STAR :
        {
            if ( bWeb )
                rSet.DisableItem( nWhich );
            else
                rSet.Put( SfxStringItem( nWhich, m_nDrawSfxId == nWhich ? m_sDrawCustom : OUString() ) );
        }
        break;

        }
}

bool SwView::HasUIFeature(SfxShellFeature nFeature) const
{
    assert((nFeature & ~SfxShellFeature::SwMask) == SfxShellFeature::NONE);
    switch(nFeature)
    {
    case SfxShellFeature::SwChildWindowLabel:
        return m_pWrtShell->IsLabelDoc();
    default:
        return false;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
