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

#include <config_wasm_strip.h>

#include <memory>

#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <comphelper/flagguard.hxx>
#include <o3tl/any.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <hintids.hxx>
#include <sfx2/styledlg.hxx>
#include <svl/whiter.hxx>
#include <sfx2/tplpitem.hxx>
#include <sfx2/request.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/newstyle.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/viewfrm.hxx>
#include <svl/stritem.hxx>
#include <svl/languageoptions.hxx>
#include <svl/ctloptions.hxx>
#include <sfx2/htmlmode.hxx>
#include <swmodule.hxx>
#include <fchrfmt.hxx>
#include <svtools/htmlcfg.hxx>
#include <svx/xdef.hxx>
#include <SwStyleNameMapper.hxx>
#include <SwRewriter.hxx>
#include <numrule.hxx>
#include <swundo.hxx>
#include <svx/drawitem.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <docsh.hxx>
#include <uitool.hxx>
#include <cmdid.h>
#include <viewopt.hxx>
#include <doc.hxx>
#include <drawdoc.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentState.hxx>
#include <frmfmt.hxx>
#include <charfmt.hxx>
#include <poolfmt.hxx>
#include <pagedesc.hxx>
#include <docstyle.hxx>
#include <uiitems.hxx>
#include <fmtcol.hxx>
#include <edtwin.hxx>
#include <unochart.hxx>
#include <swabstdlg.hxx>
#include <paratr.hxx>
#include <tblafmt.hxx>
#include <sfx2/watermarkitem.hxx>
#include <svl/grabbagitem.hxx>
#include <SwUndoFmt.hxx>
#include <strings.hrc>
#include <AccessibilityCheck.hxx>

using namespace ::com::sun::star;

void  SwDocShell::StateStyleSheet(SfxItemSet& rSet, SwWrtShell* pSh)
{
    SfxWhichIter aIter(rSet);
    sal_uInt16  nWhich  = aIter.FirstWhich();
    SfxStyleFamily nActualFamily = SfxStyleFamily(USHRT_MAX);

    SwWrtShell* pShell = pSh ? pSh : GetWrtShell();
    if(!pShell)
    {
        while (nWhich)
        {
            rSet.DisableItem(nWhich);
            nWhich = aIter.NextWhich();
        }
        return;
    }
    else
    {
        SfxViewFrame* pFrame = pShell->GetView().GetViewFrame();
        std::unique_ptr<SfxPoolItem> pItem;
        pFrame->GetBindings().QueryState(SID_STYLE_FAMILY, pItem);
        SfxUInt16Item* pFamilyItem = dynamic_cast<SfxUInt16Item*>(pItem.get());
        if (pFamilyItem)
        {
            nActualFamily = static_cast<SfxStyleFamily>(pFamilyItem->GetValue());
        }
    }

    while (nWhich)
    {
        // determine current template to every family
        OUString aName;
        SwTableAutoFormat aTableAutoFormat("dummy"); // needed to check if can take a table auto format at current cursor position
        switch (nWhich)
        {
            case SID_STYLE_APPLY:
            {// here the template and its family are passed to the StyleBox
             // so that this family is being showed
                if(pShell->IsFrameSelected())
                {
                    SwFrameFormat* pFormat = pShell->GetSelectedFrameFormat();
                    if( pFormat )
                        aName = pFormat->GetName();
                }
                else
                {
                    SwTextFormatColl* pColl = pShell->GetCurTextFormatColl();
                    if(pColl)
                        aName = pColl->GetName();
                }
                rSet.Put(SfxTemplateItem(nWhich, aName));
            }
            break;
            case SID_STYLE_FAMILY1:
                if( !pShell->IsFrameSelected() )
                {
                    SwCharFormat* pFormat = pShell->GetCurCharFormat();
                    if(pFormat)
                        aName = pFormat->GetName();
                    else
                        aName = SwResId(STR_POOLCHR_STANDARD);
                    rSet.Put(SfxTemplateItem(nWhich, aName));
                }
                break;

            case SID_STYLE_FAMILY2:
                if(!pShell->IsFrameSelected())
                {
                    OUString aProgName;
                    SwTextFormatColl* pColl = pShell->GetCurTextFormatColl();
                    if(pColl)
                    {
                        aName = pColl->GetName();
                        sal_uInt16 nId = pColl->GetPoolFormatId();
                        SwStyleNameMapper::FillProgName(nId, aProgName);
                    }

                    SfxTemplateItem aItem(nWhich, aName, aProgName);

                    SfxStyleSearchBits nMask = SfxStyleSearchBits::Auto;
                    if (m_xDoc->getIDocumentSettingAccess().get(DocumentSettingId::HTML_MODE))
                        nMask = SfxStyleSearchBits::SwHtml;
                    else
                    {
                        const FrameTypeFlags nSelection = pShell->GetFrameType(nullptr,true);
                        if(pShell->GetCurTOX())
                            nMask = SfxStyleSearchBits::SwIndex  ;
                        else if(nSelection & FrameTypeFlags::HEADER     ||
                                nSelection & FrameTypeFlags::FOOTER     ||
                                nSelection & FrameTypeFlags::TABLE      ||
                                nSelection & FrameTypeFlags::FLY_ANY    ||
                                nSelection & FrameTypeFlags::FOOTNOTE   ||
                                nSelection & FrameTypeFlags::FTNPAGE)
                            nMask = SfxStyleSearchBits::SwExtra;
                        else
                            nMask = SfxStyleSearchBits::SwText;
                    }

                    aItem.SetValue(nMask);
                    rSet.Put(aItem);
                }

                break;

            case SID_STYLE_FAMILY3:

                if (m_xDoc->getIDocumentSettingAccess().get(DocumentSettingId::HTML_MODE))
                    rSet.DisableItem( nWhich );
                else
                {
                    SwFrameFormat* pFormat = pShell->GetSelectedFrameFormat();
                    if(pFormat && pShell->IsFrameSelected())
                    {
                        aName = pFormat->GetName();
                        rSet.Put(SfxTemplateItem(nWhich, aName));
                    }
                }
                break;

            case SID_STYLE_FAMILY4:
            {
                if (m_xDoc->getIDocumentSettingAccess().get(DocumentSettingId::HTML_MODE) && !SvxHtmlOptions::IsPrintLayoutExtension())
                    rSet.DisableItem( nWhich );
                else
                {
                    size_t n = pShell->GetCurPageDesc( false );
                    if( n < pShell->GetPageDescCnt() )
                        aName = pShell->GetPageDesc( n ).GetName();

                    rSet.Put(SfxTemplateItem(nWhich, aName));
                }
            }
            break;
            case SID_STYLE_FAMILY5:
                {
                    const SwNumRule* pRule = pShell->GetNumRuleAtCurrCursorPos();
                    if( pRule )
                        aName = pRule->GetName();

                    rSet.Put(SfxTemplateItem(nWhich, aName));
                }
                break;
            case SID_STYLE_FAMILY6:
                {
                    const SwTableNode *pTableNd = pShell->IsCursorInTable();
                    if( pTableNd )
                        aName = pTableNd->GetTable().GetTableStyleName();

                    rSet.Put(SfxTemplateItem(nWhich, aName));
                }
                break;

            case SID_STYLE_WATERCAN:
            {
                SwEditWin& rEdtWin = pShell->GetView().GetEditWin();
                SwApplyTemplate* pApply = rEdtWin.GetApplyTemplate();
                rSet.Put(SfxBoolItem(nWhich, pApply && pApply->eType != SfxStyleFamily(0)));
            }
            break;
            case SID_STYLE_UPDATE_BY_EXAMPLE:
                if( pShell->IsFrameSelected()
                        ? SfxStyleFamily::Frame != nActualFamily
                        : ( SfxStyleFamily::Frame == nActualFamily ||
                            SfxStyleFamily::Page == nActualFamily ||
                            (SfxStyleFamily::Pseudo == nActualFamily && !pShell->GetNumRuleAtCurrCursorPos()) ||
                            (SfxStyleFamily::Table == nActualFamily && !pShell->GetTableAutoFormat(aTableAutoFormat))) )
                {
                    rSet.DisableItem( nWhich );
                }
                break;

            case SID_STYLE_NEW_BY_EXAMPLE:
                if( (pShell->IsFrameSelected()
                        ? SfxStyleFamily::Frame != nActualFamily
                        : SfxStyleFamily::Frame == nActualFamily) ||
                    (SfxStyleFamily::Pseudo == nActualFamily && !pShell->GetNumRuleAtCurrCursorPos()) ||
                    (SfxStyleFamily::Table == nActualFamily && !pShell->GetTableAutoFormat(aTableAutoFormat)) )
                {
                    rSet.DisableItem( nWhich );
                }
                break;

            case SID_CLASSIFICATION_APPLY:
                // Just trigger ClassificationCategoriesController::statusChanged().
                rSet.InvalidateItem(nWhich);
                break;
            case SID_CLASSIFICATION_DIALOG:
                rSet.InvalidateItem(nWhich);
                break;
            case SID_STYLE_EDIT:
                break;
            case SID_WATERMARK:
                if (pSh)
                {
                    SfxWatermarkItem aItem = pSh->GetWatermark();
                    rSet.Put(aItem);
                }
                break;
            default:
                OSL_FAIL("Invalid SlotId");
        }
        nWhich = aIter.NextWhich();
    }
}

// evaluate StyleSheet-Requests
void SwDocShell::ExecStyleSheet( SfxRequest& rReq )
{
    sal_uInt16  nSlot   = rReq.GetSlot();

    const SfxItemSet* pArgs = rReq.GetArgs();
    const SfxPoolItem* pItem;
    switch (nSlot)
    {
    case SID_STYLE_NEW:
        if( pArgs && SfxItemState::SET == pArgs->GetItemState( SID_STYLE_FAMILY,
            false, &pItem ))
        {
            const SfxStyleFamily nFamily = static_cast<SfxStyleFamily>(static_cast<const SfxUInt16Item*>(pItem)->GetValue());

            OUString sName;
            SfxStyleSearchBits nMask = SfxStyleSearchBits::Auto;
            if( SfxItemState::SET == pArgs->GetItemState( SID_STYLE_NEW,
                false, &pItem ))
                sName = static_cast<const SfxStringItem*>(pItem)->GetValue();
            if( SfxItemState::SET == pArgs->GetItemState( SID_STYLE_MASK,
                false, &pItem ))
                nMask = static_cast<SfxStyleSearchBits>(static_cast<const SfxUInt16Item*>(pItem)->GetValue());
            OUString sParent;
            if( SfxItemState::SET == pArgs->GetItemState( SID_STYLE_REFERENCE,
                false, &pItem ))
                sParent = static_cast<const SfxStringItem*>(pItem)->GetValue();

            if (sName.isEmpty() && m_xBasePool)
                sName = SfxStyleDialogController::GenerateUnusedName(*m_xBasePool, nFamily);

            Edit(rReq.GetFrameWeld(), sName, sParent, nFamily, nMask, true, OString(), nullptr, &rReq, nSlot);
        }
        break;

        case SID_STYLE_APPLY:
            if( !pArgs )
            {
                GetView()->GetViewFrame()->GetDispatcher()->Execute(SID_STYLE_DESIGNER);
                break;
            }
            else
            {
                // convert internal StyleName to DisplayName (slot implementation uses the latter)
                const SfxStringItem* pNameItem = rReq.GetArg<SfxStringItem>(SID_APPLY_STYLE);
                const SfxStringItem* pFamilyItem = rReq.GetArg<SfxStringItem>(SID_STYLE_FAMILYNAME);
                if ( pFamilyItem && pNameItem )
                {
                    uno::Reference< style::XStyleFamiliesSupplier > xModel(GetModel(), uno::UNO_QUERY);
                    try
                    {
                        uno::Reference< container::XNameAccess > xStyles;
                        uno::Reference< container::XNameAccess > xCont = xModel->getStyleFamilies();
                        xCont->getByName(pFamilyItem->GetValue()) >>= xStyles;
                        uno::Reference< beans::XPropertySet > xInfo;
                        xStyles->getByName( pNameItem->GetValue() ) >>= xInfo;
                        OUString aUIName;
                        xInfo->getPropertyValue("DisplayName") >>= aUIName;
                        if ( !aUIName.isEmpty() )
                            rReq.AppendItem( SfxStringItem( SID_STYLE_APPLY, aUIName ) );
                    }
                    catch (const uno::Exception&)
                    {
                    }
                }
            }

            [[fallthrough]];

        case SID_STYLE_EDIT:
        case SID_STYLE_DELETE:
        case SID_STYLE_HIDE:
        case SID_STYLE_SHOW:
        case SID_STYLE_WATERCAN:
        case SID_STYLE_FAMILY:
        case SID_STYLE_UPDATE_BY_EXAMPLE:
        case SID_STYLE_NEW_BY_EXAMPLE:
        {
            OUString aParam;
            SfxStyleFamily nFamily = SfxStyleFamily::Para;
            SfxStyleSearchBits nMask = SfxStyleSearchBits::Auto;
            SwWrtShell* pActShell = nullptr;

            if( !pArgs )
            {
                switch (nSlot)
                {
                    case SID_STYLE_NEW_BY_EXAMPLE:
                    {
                        SfxStyleSheetBasePool& rPool = *GetStyleSheetPool();
                        SfxNewStyleDlg aDlg(GetView()->GetFrameWeld(), rPool, nFamily);
                        if (aDlg.run() == RET_OK)
                        {
                            aParam = aDlg.GetName();
                            rReq.AppendItem(SfxStringItem(nSlot, aParam));
                        }
                    }
                    break;

                    case SID_STYLE_UPDATE_BY_EXAMPLE:
                    case SID_STYLE_EDIT:
                    {
                        SwTextFormatColl* pColl = GetWrtShell()->GetCurTextFormatColl();
                        if(pColl)
                        {
                            aParam = pColl->GetName();
                            rReq.AppendItem(SfxStringItem(nSlot, aParam));
                        }
                    }
                    break;
                }
            }
            else
            {
                SAL_WARN_IF( !pArgs->Count(), "sw.ui", "SfxBug ItemSet is empty" );

                SwWrtShell* pShell = GetWrtShell();
                if( SfxItemState::SET == pArgs->GetItemState(nSlot, false, &pItem ))
                    aParam = static_cast<const SfxStringItem*>(pItem)->GetValue();

                if( SfxItemState::SET == pArgs->GetItemState(SID_STYLE_FAMILY,
                    false, &pItem ))
                    nFamily = static_cast<SfxStyleFamily>(static_cast<const SfxUInt16Item*>(pItem)->GetValue());

                if( SfxItemState::SET == pArgs->GetItemState(SID_STYLE_FAMILYNAME, false, &pItem ))
                {
                    OUString aFamily = static_cast<const SfxStringItem*>(pItem)->GetValue();
                    if(aFamily == "CharacterStyles")
                        nFamily = SfxStyleFamily::Char;
                    else
                    if(aFamily == "ParagraphStyles")
                        nFamily = SfxStyleFamily::Para;
                    else
                    if(aFamily == "PageStyles")
                        nFamily = SfxStyleFamily::Page;
                    else
                    if(aFamily == "FrameStyles")
                        nFamily = SfxStyleFamily::Frame;
                    else
                    if(aFamily == "NumberingStyles")
                        nFamily = SfxStyleFamily::Pseudo;
                    else
                    if(aFamily == "TableStyles")
                        nFamily = SfxStyleFamily::Table;
                }

                if( SfxItemState::SET == pArgs->GetItemState(SID_STYLE_MASK,
                    false, &pItem ))
                    nMask = static_cast<SfxStyleSearchBits>(static_cast<const SfxUInt16Item*>(pItem)->GetValue());
                if( SfxItemState::SET == pArgs->GetItemState(FN_PARAM_WRTSHELL,
                    false, &pItem ))
                    pActShell = pShell = static_cast<SwWrtShell*>(static_cast<const SwPtrItem*>(pItem)->GetValue());

                if( nSlot == SID_STYLE_UPDATE_BY_EXAMPLE && aParam.isEmpty() )
                {
                    switch( nFamily )
                    {
                        case SfxStyleFamily::Para:
                        {
                            SwTextFormatColl* pColl = pShell->GetCurTextFormatColl();
                            if(pColl)
                                aParam = pColl->GetName();
                        }
                        break;
                        case SfxStyleFamily::Frame:
                        {
                            SwFrameFormat* pFrame = m_pWrtShell->GetSelectedFrameFormat();
                            if( pFrame )
                                aParam = pFrame->GetName();
                        }
                        break;
                        case SfxStyleFamily::Char:
                        {
                            SwCharFormat* pChar = m_pWrtShell->GetCurCharFormat();
                            if( pChar )
                                aParam = pChar->GetName();
                        }
                        break;
                        case SfxStyleFamily::Pseudo:
                        if(SfxItemState::SET == pArgs->GetItemState(SID_STYLE_UPD_BY_EX_NAME, false, &pItem))
                        {
                            aParam = static_cast<const SfxStringItem*>(pItem)->GetValue();
                        }
                        break;
                        case SfxStyleFamily::Table:
                        if(SfxItemState::SET == pArgs->GetItemState(SID_STYLE_UPD_BY_EX_NAME, false, &pItem))
                        {
                            aParam = static_cast<const SfxStringItem*>(pItem)->GetValue();
                        }
                        break;
                        default: break;
                    }
                    rReq.AppendItem(SfxStringItem(nSlot, aParam));
                }
            }
            if (!aParam.isEmpty() || nSlot == SID_STYLE_WATERCAN )
            {
                sal_uInt16 nRet = 0xffff;
                bool bReturns = false;

                switch(nSlot)
                {
                    case SID_STYLE_EDIT:
                        Edit(rReq.GetFrameWeld(), aParam, OUString(), nFamily, nMask, false, OString(), pActShell);
                        break;
                    case SID_STYLE_DELETE:
                        Delete(aParam, nFamily);
                        break;
                    case SID_STYLE_HIDE:
                    case SID_STYLE_SHOW:
                        Hide(aParam, nFamily, nSlot == SID_STYLE_HIDE);
                        break;
                    case SID_STYLE_APPLY:
                        // Shell-switch in ApplyStyles
                        nRet = static_cast<sal_uInt16>(ApplyStyles(aParam, nFamily, pActShell, rReq.GetModifier() ));
                        bReturns = true;
                        break;
                    case SID_STYLE_WATERCAN:
                        nRet = static_cast<sal_uInt16>(DoWaterCan(aParam, nFamily));
                        bReturns = true;
                        break;
                    case SID_STYLE_UPDATE_BY_EXAMPLE:
                        UpdateStyle(aParam, nFamily, pActShell);
                        break;
                    case SID_STYLE_NEW_BY_EXAMPLE:
                        MakeByExample(aParam, nFamily, nMask, pActShell);
                        break;

                    default:
                        OSL_FAIL("Invalid SlotId");
                }

                if (bReturns)
                {
                    if(rReq.IsAPI()) // Basic only gets TRUE or FALSE
                        rReq.SetReturnValue(SfxUInt16Item(nSlot, sal_uInt16(nRet !=0)));
                    else
                        rReq.SetReturnValue(SfxUInt16Item(nSlot, nRet));
                }

                rReq.Done();
            }

            break;
        }
    }
}

namespace {

class ApplyStyle
{
public:
    ApplyStyle(SwDocShell &rDocSh, bool bNew,
        rtl::Reference< SwDocStyleSheet > const & xTmp,
        SfxStyleFamily nFamily, SfxAbstractApplyTabDialog *pDlg,
        rtl::Reference< SfxStyleSheetBasePool > const & xBasePool,
        bool bModified)
        : m_pDlg(pDlg)
        , m_rDocSh(rDocSh)
        , m_bNew(bNew)
        , m_xTmp(xTmp)
        , m_nFamily(nFamily)
        , m_xBasePool(xBasePool)
        , m_bModified(bModified)
    {
    }
    DECL_LINK( ApplyHdl, LinkParamNone*, void );
    void apply()
    {
        ApplyHdl(nullptr);
    }
    VclPtr<SfxAbstractApplyTabDialog> m_pDlg;
    // true if the document was initially modified before ApplyStyle was created
    // or if ApplyStyle:::apply was called
    bool DocIsModified() const
    {
        return m_bModified;
    }
private:
    SwDocShell &m_rDocSh;
    bool m_bNew;
    rtl::Reference< SwDocStyleSheet > m_xTmp;
    SfxStyleFamily m_nFamily;
    rtl::Reference< SfxStyleSheetBasePool > m_xBasePool;
    bool m_bModified;
};

}

IMPL_LINK_NOARG(ApplyStyle, ApplyHdl, LinkParamNone*, void)
{
    SwWrtShell* pWrtShell = m_rDocSh.GetWrtShell();
    SwDoc* pDoc = m_rDocSh.GetDoc();
    SwView* pView = m_rDocSh.GetView();

    pWrtShell->StartAllAction();

    if( SfxStyleFamily::Para == m_nFamily )
    {
        SfxItemSet aSet( *m_pDlg->GetOutputItemSet() );
        ::ConvertAttrGenToChar(aSet, m_xTmp->GetItemSet(), /*bIsPara=*/true);
        ::SfxToSwPageDescAttr( *pWrtShell, aSet  );
        // reset indent attributes at paragraph style, if a list style
        // will be applied and no indent attributes will be applied.
        m_xTmp->SetItemSet( aSet, true );
    }
    else
    {
        if(SfxStyleFamily::Page == m_nFamily)
        {
            static const sal_uInt16 aInval[] = {
                SID_IMAGE_ORIENTATION,
                SID_ATTR_CHAR_FONT,
                FN_INSERT_CTRL, FN_INSERT_OBJ_CTRL, 0};
            pView->GetViewFrame()->GetBindings().Invalidate(aInval);
        }
        SfxItemSet aTmpSet( *m_pDlg->GetOutputItemSet() );
        if( SfxStyleFamily::Char == m_nFamily )
        {
            ::ConvertAttrGenToChar(aTmpSet, m_xTmp->GetItemSet());
        }

        m_xTmp->SetItemSet( aTmpSet );

        if( SfxStyleFamily::Page == m_nFamily && SvtCTLOptions().IsCTLFontEnabled() )
        {
            const SfxPoolItem *pItem = nullptr;
            if( aTmpSet.GetItemState( m_rDocSh.GetPool().GetTrueWhich( SID_ATTR_FRAMEDIRECTION, false ) , true, &pItem ) == SfxItemState::SET )
                SwChartHelper::DoUpdateAllCharts( pDoc );
        }

        if (m_nFamily == SfxStyleFamily::Page)
        {
            const SfxPoolItem* pItem = nullptr;
            if (aTmpSet.HasItem(SID_ATTR_CHAR_GRABBAG, &pItem))
            {
                const auto& rGrabBagItem = static_cast<const SfxGrabBagItem&>(*pItem);
                bool bGutterAtTop{};
                auto it = rGrabBagItem.GetGrabBag().find("GutterAtTop");
                if (it != rGrabBagItem.GetGrabBag().end())
                {
                    it->second >>= bGutterAtTop;
                }
                bool bOldGutterAtTop
                    = pDoc->getIDocumentSettingAccess().get(DocumentSettingId::GUTTER_AT_TOP);
                if (bOldGutterAtTop != bGutterAtTop)
                {
                    pDoc->getIDocumentSettingAccess().set(DocumentSettingId::GUTTER_AT_TOP,
                                                          bGutterAtTop);
                    pWrtShell->InvalidateLayout(/*bSizeChanged=*/true);
                }
            }
        }

        if (m_nFamily == SfxStyleFamily::Frame)
        {
            const SfxPoolItem* pItem = nullptr;
            if (aTmpSet.HasItem(FN_KEEP_ASPECT_RATIO, &pItem))
            {
                const auto& rBoolItem = static_cast<const SfxBoolItem&>(*pItem);
                const SwViewOption* pVOpt = pWrtShell->GetViewOptions();
                SwViewOption aUsrPref(*pVOpt);
                aUsrPref.SetKeepRatio(rBoolItem.GetValue());
                if (rBoolItem.GetValue() != pVOpt->IsKeepRatio())
                {
                    SW_MOD()->ApplyUsrPref(aUsrPref, &pWrtShell->GetView());
                }
            }
        }
    }

    if(m_bNew)
    {
        if(SfxStyleFamily::Frame == m_nFamily || SfxStyleFamily::Para == m_nFamily)
        {
            // clear FillStyle so that it works as a derived attribute
            SfxItemSet aTmpSet(*m_pDlg->GetOutputItemSet());

            aTmpSet.ClearItem(XATTR_FILLSTYLE);
            m_xTmp->SetItemSet(aTmpSet);
        }
    }

    if(SfxStyleFamily::Page == m_nFamily)
        pView->InvalidateRulerPos();

    if( m_bNew )
        m_xBasePool->Broadcast(SfxStyleSheetHint(SfxHintId::StyleSheetCreated, *m_xTmp));
    else
        m_xBasePool->Broadcast(SfxStyleSheetHint(SfxHintId::StyleSheetModified, *m_xTmp));

    pDoc->getIDocumentState().SetModified();
    if( !m_bModified )
    {
        pDoc->GetIDocumentUndoRedo().SetUndoNoResetModified();
        m_bModified = true;
    }

    pWrtShell->EndAllAction();
}

namespace
{
/// Checks if there is an Endnote page style in use, and makes sure it has the same orientation
/// with the Default (Standard) page style.
void syncEndnoteOrientation(const uno::Reference< style::XStyleFamiliesSupplier >& xStyleFamSupp)
{
    if (!xStyleFamSupp.is())
    {
        SAL_WARN("sw.ui", "Ref to XStyleFamiliesSupplier is null.");
        return;
    }
    uno::Reference<container::XNameAccess> xStyleFamilies = xStyleFamSupp->getStyleFamilies();

    if (!xStyleFamilies.is())
        return;

    uno::Reference<container::XNameAccess> xPageStyles(xStyleFamilies->getByName("PageStyles"),
                                                       uno::UNO_QUERY);

    if (!xPageStyles.is())
        return;

    uno::Reference<css::style::XStyle> xEndnotePageStyle(xPageStyles->getByName("Endnote"),
                                                  uno::UNO_QUERY);

    if (!xEndnotePageStyle.is())
        return;

    // Language-independent name of the "Default Style" is "Standard"
    uno::Reference<css::style::XStyle> xDefaultPageStyle(xPageStyles->getByName("Standard"),
                                                  uno::UNO_QUERY);
    if (!xDefaultPageStyle.is())
        return;

    if (xEndnotePageStyle->isUserDefined() || !xEndnotePageStyle->isInUse())
        return;

    uno::Reference<beans::XPropertySet> xEndnotePagePropSet(xPageStyles->getByName("Endnote"), uno::UNO_QUERY);
    uno::Reference<beans::XPropertySet> xDefaultPagePropSet(xPageStyles->getByName("Standard"), uno::UNO_QUERY);

    if (!xEndnotePagePropSet.is() || !xDefaultPagePropSet.is())
    {
        SAL_WARN("sw.ui", "xEndnotePagePropSet or xDefaultPagePropSet is null.");
        return;
    }

    auto const bIsDefLandScape = *o3tl::doAccess<bool>(
        xDefaultPagePropSet->getPropertyValue("IsLandscape"));
    auto const bIsEndLandScape = *o3tl::doAccess<bool>(
        xEndnotePagePropSet->getPropertyValue("IsLandscape"));

    if (bIsDefLandScape == bIsEndLandScape)
        return;

    auto const nWidth = xEndnotePagePropSet->getPropertyValue("Width");
    auto const nHeight = xEndnotePagePropSet->getPropertyValue("Height");

    xEndnotePagePropSet->setPropertyValue("IsLandscape", css::uno::toAny(bIsDefLandScape));
    xEndnotePagePropSet->setPropertyValue("Width", nHeight);
    xEndnotePagePropSet->setPropertyValue("Height", nWidth);
}
}

void SwDocShell::Edit(
    weld::Window* pDialogParent,
    const OUString &rName,
    const OUString &rParent,
    const SfxStyleFamily nFamily,
    SfxStyleSearchBits nMask,
    const bool bNew,
    const OString& sPage,
    SwWrtShell* pActShell,
    SfxRequest* pReq,
    sal_uInt16 nSlot)
{
    assert( GetWrtShell() );
    const bool bBasic = pReq && pReq->IsAPI();
    SfxStyleSheetBase *pStyle = nullptr;

    bool bModified = m_xDoc->getIDocumentState().IsModified();

    SwUndoId nNewStyleUndoId(SwUndoId::EMPTY);

    if( bNew )
    {
        if (!bBasic)
        {
            // start undo action in order to get only one undo action for the
            // UI new style + change style operations
            m_pWrtShell->StartUndo();
        }

        if( SfxStyleSearchBits::All != nMask && SfxStyleSearchBits::AllVisible != nMask && SfxStyleSearchBits::Used != nMask )
            nMask |= SfxStyleSearchBits::UserDefined;
        else
            nMask = SfxStyleSearchBits::UserDefined;

        if ( nFamily == SfxStyleFamily::Para || nFamily == SfxStyleFamily::Char || nFamily == SfxStyleFamily::Frame )
        {
            // Prevent undo append from being done during paragraph, character, and frame style Make
            // Do it after ok return from style dialog when derived from style is known
            ::sw::UndoGuard const undoGuard(GetDoc()->GetIDocumentUndoRedo());
            pStyle = &m_xBasePool->Make( rName, nFamily, nMask );
        }
        else
        {
            pStyle = &m_xBasePool->Make( rName, nFamily, nMask );
        }

        // set the current one as Parent
        SwDocStyleSheet* pDStyle = static_cast<SwDocStyleSheet*>(pStyle);
        switch( nFamily )
        {
            case SfxStyleFamily::Para:
            {
                if(!rParent.isEmpty())
                {
                    SwTextFormatColl* pColl = m_pWrtShell->FindTextFormatCollByName( rParent );
                    if(!pColl)
                    {
                        sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName(rParent, SwGetPoolIdFromName::TxtColl);
                        if(USHRT_MAX != nId)
                            pColl = m_pWrtShell->GetTextCollFromPool( nId );
                    }
                    pDStyle->GetCollection()->SetDerivedFrom( pColl );
                    pDStyle->PresetParent( rParent );
                }
                else
                {
                    SwTextFormatColl* pColl = m_pWrtShell->GetCurTextFormatColl();
                    pDStyle->GetCollection()->SetDerivedFrom( pColl );
                    if( pColl )
                        pDStyle->PresetParent( pColl->GetName() );
                }
            }
            break;
            case SfxStyleFamily::Char:
            {
                if(!rParent.isEmpty())
                {
                    SwCharFormat* pCFormat = m_pWrtShell->FindCharFormatByName(rParent);
                    if(!pCFormat)
                    {
                        sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName(rParent, SwGetPoolIdFromName::ChrFmt);
                        if(USHRT_MAX != nId)
                            pCFormat = m_pWrtShell->GetCharFormatFromPool( nId );
                    }

                    pDStyle->GetCharFormat()->SetDerivedFrom( pCFormat );
                    pDStyle->PresetParent( rParent );
                }
                else
                {
                    SwCharFormat* pCFormat = m_pWrtShell->GetCurCharFormat();
                    pDStyle->GetCharFormat()->SetDerivedFrom( pCFormat );
                    if( pCFormat )
                        pDStyle->PresetParent( pCFormat->GetName() );
                }
            }
            break;
            case SfxStyleFamily::Frame :
            {
                if(!rParent.isEmpty())
                {
                    SwFrameFormat* pFFormat = m_pWrtShell->GetDoc()->FindFrameFormatByName( rParent );
                    if(!pFFormat)
                    {
                        sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName(rParent, SwGetPoolIdFromName::FrmFmt);
                        if(USHRT_MAX != nId)
                            pFFormat = m_pWrtShell->GetFrameFormatFromPool( nId );
                    }
                    pDStyle->GetFrameFormat()->SetDerivedFrom( pFFormat );
                    pDStyle->PresetParent( rParent );
                }
            }
            break;
            default: break;
        }

        if (!bBasic)
        {
            //Get the undo id for the type of style that was created in order to re-use that comment for the grouped
            //create style + change style operations
            m_pWrtShell->GetLastUndoInfo(nullptr, &nNewStyleUndoId);
        }
    }
    else
    {
        pStyle = m_xBasePool->Find( rName, nFamily );
        SAL_WARN_IF( !pStyle, "sw.ui", "Style not found" );
    }

    if(!pStyle)
        return;

    // put dialogues together
    rtl::Reference< SwDocStyleSheet > xTmp( new SwDocStyleSheet( *static_cast<SwDocStyleSheet*>(pStyle) ) );
    if( SfxStyleFamily::Para == nFamily )
    {
        SfxItemSet& rSet = xTmp->GetItemSet();
        ::SwToSfxPageDescAttr( rSet );
        // merge list level indent attributes into the item set if needed
        xTmp->MergeIndentAttrsOfListStyle( rSet );

        ::ConvertAttrCharToGen(xTmp->GetItemSet(), /*bIsPara=*/true);
    }
    else if( SfxStyleFamily::Char == nFamily )
    {
        ::ConvertAttrCharToGen(xTmp->GetItemSet());
    }

    if(SfxStyleFamily::Page == nFamily || SfxStyleFamily::Para == nFamily)
    {
        // create needed items for XPropertyList entries from the DrawModel so that
        // the Area TabPage can access them
        SfxItemSet& rSet = xTmp->GetItemSet();
        const SwDrawModel* pDrawModel = GetDoc()->getIDocumentDrawModelAccess().GetDrawModel();

        rSet.Put(SvxColorListItem(pDrawModel->GetColorList(), SID_COLOR_TABLE));
        rSet.Put(SvxGradientListItem(pDrawModel->GetGradientList(), SID_GRADIENT_LIST));
        rSet.Put(SvxHatchListItem(pDrawModel->GetHatchList(), SID_HATCH_LIST));
        rSet.Put(SvxBitmapListItem(pDrawModel->GetBitmapList(), SID_BITMAP_LIST));
        rSet.Put(SvxPatternListItem(pDrawModel->GetPatternList(), SID_PATTERN_LIST));

        std::optional<SfxGrabBagItem> oGrabBag;
        SfxPoolItem const* pItem(nullptr);
        if (SfxItemState::SET == rSet.GetItemState(SID_ATTR_CHAR_GRABBAG, true, &pItem))
        {
            oGrabBag.emplace(*static_cast<SfxGrabBagItem const*>(pItem));
        }
        else
        {
            oGrabBag.emplace(SID_ATTR_CHAR_GRABBAG);
        }
        bool bGutterAtTop
            = GetDoc()->getIDocumentSettingAccess().get(DocumentSettingId::GUTTER_AT_TOP);
        oGrabBag->GetGrabBag()["GutterAtTop"] <<= bGutterAtTop;
        rSet.Put(*oGrabBag);
    }

    SwWrtShell* pCurrShell = pActShell ? pActShell : m_pWrtShell;
    if (nFamily == SfxStyleFamily::Frame)
    {
        SfxItemSet& rSet = xTmp->GetItemSet();
        const SwViewOption* pVOpt = pCurrShell->GetViewOptions();
        rSet.Put(SfxBoolItem(FN_KEEP_ASPECT_RATIO, pVOpt->IsKeepRatio()));
    }

    if (!bBasic)
    {
        // prior to the dialog the HtmlMode at the DocShell is being sunk
        sal_uInt16 nHtmlMode = ::GetHtmlMode(this);

        // In HTML mode, we do not always have a printer. In order to show
        // the correct page size in the Format - Page dialog, we have to
        // get one here.
        if( ( HTMLMODE_ON & nHtmlMode ) &&
            !pCurrShell->getIDocumentDeviceAccess().getPrinter( false ) )
            pCurrShell->InitPrt( pCurrShell->getIDocumentDeviceAccess().getPrinter( true ) );

        PutItem(SfxUInt16Item(SID_HTML_MODE, nHtmlMode));
        FieldUnit eMetric = ::GetDfltMetric(0 != (HTMLMODE_ON&nHtmlMode));
        SW_MOD()->PutItem(SfxUInt16Item(SID_ATTR_METRIC, static_cast< sal_uInt16 >(eMetric)));
        SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
        if (!pDialogParent)
        {
            SAL_WARN("sw.ui", "no parent for dialog supplied, assuming document frame is good enough");
            pDialogParent = GetView()->GetFrameWeld();
        }
        VclPtr<SfxAbstractApplyTabDialog> pDlg(pFact->CreateTemplateDialog(pDialogParent,
                                                    *xTmp, nFamily, sPage, pCurrShell, bNew));
        auto pApplyStyleHelper = std::make_shared<ApplyStyle>(*this, bNew, xTmp, nFamily, pDlg.get(), m_xBasePool, bModified);
        pDlg->SetApplyHdl(LINK(pApplyStyleHelper.get(), ApplyStyle, ApplyHdl));

        std::shared_ptr<SfxRequest> pRequest;
        if (pReq)
        {
            pRequest = std::make_shared<SfxRequest>(*pReq);
            pReq->Ignore(); // the 'old' request is not relevant any more
        }

        bool bIsDefaultPage = nFamily == SfxStyleFamily::Page
                                    && rName == SwResId(STR_POOLPAGE_STANDARD)
                                    && pStyle->IsUsed()
                                    && !pStyle->IsUserDefined();

        pDlg->StartExecuteAsync([bIsDefaultPage, bNew, nFamily, nSlot, nNewStyleUndoId, pApplyStyleHelper, pRequest, xTmp, this](sal_Int32 nResult){
            if (RET_OK == nResult)
                pApplyStyleHelper->apply();

            if (bNew)
            {
                switch( nFamily )
                {
                    case SfxStyleFamily::Para:
                    {
                        if(!xTmp->GetParent().isEmpty())
                        {
                            SwTextFormatColl* pColl = m_pWrtShell->FindTextFormatCollByName(xTmp->GetParent());
                            if (GetDoc()->GetIDocumentUndoRedo().DoesUndo())
                            {
                                GetDoc()->GetIDocumentUndoRedo().AppendUndo(
                                    std::make_unique<SwUndoTextFormatCollCreate>(xTmp->GetCollection(), pColl, *GetDoc()));
                            }
                        }
                    }
                    break;
                    case SfxStyleFamily::Char:
                    {
                        if(!xTmp->GetParent().isEmpty())
                        {
                            SwCharFormat* pCFormat = m_pWrtShell->FindCharFormatByName(xTmp->GetParent());
                            if (GetDoc()->GetIDocumentUndoRedo().DoesUndo())
                            {
                                GetDoc()->GetIDocumentUndoRedo().AppendUndo(
                                    std::make_unique<SwUndoCharFormatCreate>(xTmp->GetCharFormat(), pCFormat, *GetDoc()));
                            }
                        }
                    }
                    break;
                    case SfxStyleFamily::Frame:
                    {
                        if(!xTmp->GetParent().isEmpty())
                        {
                            SwFrameFormat* pFFormat = m_pWrtShell->GetDoc()->FindFrameFormatByName(xTmp->GetParent());
                            if (GetDoc()->GetIDocumentUndoRedo().DoesUndo())
                            {
                                GetDoc()->GetIDocumentUndoRedo().AppendUndo(
                                    std::make_unique<SwUndoFrameFormatCreate>(xTmp->GetFrameFormat(), pFFormat, *GetDoc()));
                            }
                        }
                    }
                    break;
                    default: break;
                }

                SwRewriter aRewriter;
                aRewriter.AddRule(UndoArg1, xTmp->GetName());
                //Group the create style and change style operations together under the
                //one "create style" comment
                m_pWrtShell->EndUndo(nNewStyleUndoId, &aRewriter);
            }

            bool bDocModified = pApplyStyleHelper->DocIsModified();

            if (RET_OK != nResult)
            {
                if (bNew)
                {
                    GetWrtShell()->Undo();
                    m_xDoc->GetIDocumentUndoRedo().ClearRedo();
                }

                if (!bDocModified)
                    m_xDoc->getIDocumentState().ResetModified();
            }

            // Update Watermark if new page style was created
            if (nSlot == SID_STYLE_NEW && nFamily == SfxStyleFamily::Page)
            {
                SwWrtShell* pShell = GetWrtShell();
                const SfxWatermarkItem aWatermark = pShell->GetWatermark();
                pShell->SetWatermark(aWatermark);
            }

            pApplyStyleHelper->m_pDlg.disposeAndClear();
            if (pRequest)
                pRequest->Done();

            if (bIsDefaultPage && bDocModified)
            {
                uno::Reference< style::XStyleFamiliesSupplier > xStyleFamSupp(GetModel(), uno::UNO_QUERY);

                if (!xStyleFamSupp.is())
                {
                    SAL_WARN("sw.ui", "Ref to XStyleFamiliesSupplier is null.");
                    return;
                }

                syncEndnoteOrientation(xStyleFamSupp);
            }
        });
    }
    else
    {
        // prior to the dialog the HtmlMode at the DocShell is being sunk
        PutItem(SfxUInt16Item(SID_HTML_MODE, ::GetHtmlMode(this)));

        GetWrtShell()->StartAllAction();

        if( SfxStyleFamily::Para == nFamily )
        {
            ::SfxToSwPageDescAttr( *GetWrtShell(), xTmp->GetItemSet() );
            ::ConvertAttrGenToChar(xTmp->GetItemSet(), xTmp->GetItemSet(), /*bIsPara=*/true);
        }
        else
        {
            ::ConvertAttrGenToChar(xTmp->GetItemSet(), xTmp->GetItemSet());
        }
        if(SfxStyleFamily::Page == nFamily)
            m_pView->InvalidateRulerPos();

        if( bNew )
            m_xBasePool->Broadcast(SfxStyleSheetHint(SfxHintId::StyleSheetCreated, *xTmp));

        m_xDoc->getIDocumentState().SetModified();
        if( !bModified )        // Bug 57028
        {
            m_xDoc->GetIDocumentUndoRedo().SetUndoNoResetModified();
        }
        GetWrtShell()->EndAllAction();
    }
}

void SwDocShell::Delete(const OUString &rName, SfxStyleFamily nFamily)
{
    SfxStyleSheetBase *pStyle = m_xBasePool->Find(rName, nFamily);

    if(pStyle)
    {
        assert( GetWrtShell() );

        GetWrtShell()->StartAllAction();
        m_xBasePool->Remove(pStyle);
        GetWrtShell()->EndAllAction();
    }
}

void SwDocShell::Hide(const OUString &rName, SfxStyleFamily nFamily, bool bHidden)
{
    SfxStyleSheetBase *pStyle = m_xBasePool->Find(rName, nFamily);

    if(pStyle)
    {
        assert( GetWrtShell() );

        GetWrtShell()->StartAllAction();
        rtl::Reference< SwDocStyleSheet > xTmp( new SwDocStyleSheet( *static_cast<SwDocStyleSheet*>(pStyle) ) );
        xTmp->SetHidden( bHidden );
        GetWrtShell()->EndAllAction();
    }
}

// apply template
SfxStyleFamily SwDocShell::ApplyStyles(const OUString &rName, SfxStyleFamily nFamily,
                               SwWrtShell* pShell, const sal_uInt16 nMode )
{
    MakeAllOutlineContentTemporarilyVisible a(GetDoc());

    SwDocStyleSheet* pStyle = static_cast<SwDocStyleSheet*>( m_xBasePool->Find( rName, nFamily ) );

    SAL_WARN_IF( !pStyle, "sw.ui", "Style not found" );

    if(!pStyle)
        return SfxStyleFamily::None;

    SwWrtShell *pSh = pShell ? pShell : GetWrtShell();

    assert( pSh );

    pSh->StartAllAction();

    switch (nFamily)
    {
        case SfxStyleFamily::Char:
        {
            SwFormatCharFormat aFormat(pStyle->GetCharFormat());
            pSh->SetAttrItem( aFormat, (nMode & KEY_SHIFT) ?
                SetAttrMode::DONTREPLACE : SetAttrMode::DEFAULT );
            break;
        }
        case SfxStyleFamily::Para:
        {
            // #i62675#
            // clear also list attributes at affected text nodes, if paragraph
            // style has the list style attribute set.
            pSh->SetTextFormatColl( pStyle->GetCollection(), true );
            break;
        }
        case SfxStyleFamily::Frame:
        {
            if ( pSh->IsFrameSelected() )
                pSh->SetFrameFormat( pStyle->GetFrameFormat() );
            break;
        }
        case SfxStyleFamily::Page:
        {
            pSh->SetPageStyle(pStyle->GetPageDesc()->GetName());
            break;
        }
        case SfxStyleFamily::Pseudo:
        {
            // reset indent attribute on applying list style
            // continue list of list style
            const SwNumRule* pNumRule = pStyle->GetNumRule();
            if (pNumRule->GetName() == "No List")
            {
                SfxViewFrame::Current()->GetDispatcher()->Execute(FN_NUM_BULLET_OFF);
                break;
            }
            const OUString sListIdForStyle =pNumRule->GetDefaultListId();
            pSh->SetCurNumRule( *pNumRule, false, sListIdForStyle, true );
            break;
        }
        case SfxStyleFamily::Table:
        {
            pSh->SetTableStyle(pStyle->GetName());
            break;
        }
        default:
            OSL_FAIL("Unknown family");
    }
    pSh->EndAllAction();

    return nFamily;
}

// start watering-can
SfxStyleFamily SwDocShell::DoWaterCan(const OUString &rName, SfxStyleFamily nFamily)
{
    assert( GetWrtShell() );

    SwEditWin& rEdtWin = m_pView->GetEditWin();
    SwApplyTemplate* pApply = rEdtWin.GetApplyTemplate();
    bool bWaterCan = !(pApply && pApply->eType != SfxStyleFamily(0));

    if( rName.isEmpty() )
        bWaterCan = false;

    SwApplyTemplate aTemplate;
    aTemplate.eType = nFamily;

    if(bWaterCan)
    {
        SwDocStyleSheet* pStyle =
            static_cast<SwDocStyleSheet*>( m_xBasePool->Find(rName, nFamily) );

        SAL_WARN_IF( !pStyle, "sw.ui", "Where's the StyleSheet" );

        if(!pStyle) return nFamily;

        switch(nFamily)
        {
            case SfxStyleFamily::Char:
                aTemplate.aColl.pCharFormat = pStyle->GetCharFormat();
                break;
            case SfxStyleFamily::Para:
                aTemplate.aColl.pTextColl = pStyle->GetCollection();
                break;
            case SfxStyleFamily::Frame:
                aTemplate.aColl.pFrameFormat = pStyle->GetFrameFormat();
                break;
            case SfxStyleFamily::Page:
                aTemplate.aColl.pPageDesc = const_cast<SwPageDesc*>(pStyle->GetPageDesc());
                break;
            case SfxStyleFamily::Pseudo:
                aTemplate.aColl.pNumRule = const_cast<SwNumRule*>(pStyle->GetNumRule());
                break;

            default:
                OSL_FAIL("Unknown family");
        }
    }
    else
        aTemplate.eType = SfxStyleFamily(0);

    m_pView->GetEditWin().SetApplyTemplate(aTemplate);

    return nFamily;
}

// update template
void SwDocShell::UpdateStyle(const OUString &rName, SfxStyleFamily nFamily, SwWrtShell* pShell)
{
    SwWrtShell* pCurrWrtShell = pShell ? pShell : GetWrtShell();
    assert( pCurrWrtShell );

    SwDocStyleSheet* pStyle =
        static_cast<SwDocStyleSheet*>( m_xBasePool->Find(rName, nFamily) );

    if (!pStyle)
        return;

    switch(nFamily)
    {
        case SfxStyleFamily::Para:
        {
            SwTextFormatColl* pColl = pStyle->GetCollection();
            if(pColl && !pColl->IsDefault())
            {
                GetWrtShell()->StartAllAction();

                SwRewriter aRewriter;
                aRewriter.AddRule(UndoArg1, pColl->GetName());

                GetWrtShell()->StartUndo(SwUndoId::INSFMTATTR, &aRewriter);
                GetWrtShell()->FillByEx(pColl);
                    // also apply template to remove hard set attributes
                GetWrtShell()->SetTextFormatColl( pColl );
                GetWrtShell()->EndUndo();
                GetWrtShell()->EndAllAction();
            }
            break;
        }
        case SfxStyleFamily::Frame:
        {
            SwFrameFormat* pFrame = pStyle->GetFrameFormat();
            if( pCurrWrtShell->IsFrameSelected() && pFrame && !pFrame->IsDefault() )
            {
                SfxItemSet aSet( GetPool(), aFrameFormatSetRange );
                pCurrWrtShell->StartAllAction();
                pCurrWrtShell->GetFlyFrameAttr( aSet );

                // #i105535#
                // no update of anchor attribute
                aSet.ClearItem( RES_ANCHOR );

                pFrame->SetFormatAttr( aSet );

                    // also apply template to remove hard set attributes
                pCurrWrtShell->SetFrameFormat( pFrame, true );
                pCurrWrtShell->EndAllAction();
            }
        }
        break;
        case SfxStyleFamily::Char:
        {
            SwCharFormat* pChar = pStyle->GetCharFormat();
            if( pChar && !pChar->IsDefault() )
            {
                pCurrWrtShell->StartAllAction();
                pCurrWrtShell->FillByEx(pChar);
                // also apply template to remove hard set attributes
                pCurrWrtShell->EndAllAction();
            }

        }
        break;
        case SfxStyleFamily::Pseudo:
        {
            const SwNumRule* pCurRule;
            if( pStyle->GetNumRule() &&
                nullptr != ( pCurRule = pCurrWrtShell->GetNumRuleAtCurrCursorPos() ))
            {
                SwNumRule aRule( *pCurRule );
                // #i91400#
                aRule.SetName( pStyle->GetNumRule()->GetName(),
                               pCurrWrtShell->GetDoc()->getIDocumentListsAccess() );
                pCurrWrtShell->ChgNumRuleFormats( aRule );
            }
        }
        break;
        case SfxStyleFamily::Table:
        {
            if(GetFEShell()->IsTableMode())
            {
                GetFEShell()->TableCursorToCursor();
            }
            SwTableAutoFormat aFormat(rName);
            if (pCurrWrtShell->GetTableAutoFormat(aFormat))
            {
                pCurrWrtShell->StartAllAction();
                pCurrWrtShell->GetDoc()->ChgTableStyle(rName, aFormat);
                pCurrWrtShell->EndAllAction();
            }

        }
        break;
        default: break;
    }

    m_xDoc->BroadcastStyleOperation(rName, nFamily, SfxHintId::StyleSheetModified);
}

// NewByExample
void SwDocShell::MakeByExample( const OUString &rName, SfxStyleFamily nFamily,
                                    SfxStyleSearchBits nMask, SwWrtShell* pShell )
{
    SwWrtShell* pCurrWrtShell = pShell ? pShell : GetWrtShell();
    SwDocStyleSheet* pStyle = static_cast<SwDocStyleSheet*>( m_xBasePool->Find(
                                            rName, nFamily ) );
    if(!pStyle)
    {
        // preserve the current mask of PI, then the new one is
        // immediately merged with the viewable area
        if( SfxStyleSearchBits::All == nMask || SfxStyleSearchBits::Used == nMask )
            nMask = SfxStyleSearchBits::UserDefined;
        else
            nMask |= SfxStyleSearchBits::UserDefined;

        if (nFamily == SfxStyleFamily::Para || nFamily == SfxStyleFamily::Char || nFamily == SfxStyleFamily::Frame)
        {
            // Prevent undo append from being done during paragraph, character, and frame style Make. Do it later
            ::sw::UndoGuard const undoGuard(GetDoc()->GetIDocumentUndoRedo());
            pStyle = static_cast<SwDocStyleSheet*>(&m_xBasePool->Make(rName, nFamily, nMask));
        }
        else
        {
            pStyle = static_cast<SwDocStyleSheet*>(&m_xBasePool->Make(rName, nFamily, nMask));
        }
    }

    switch(nFamily)
    {
        case  SfxStyleFamily::Para:
        {
            SwTextFormatColl* pColl = pStyle->GetCollection();
            if(pColl && !pColl->IsDefault())
            {
                pCurrWrtShell->StartAllAction();
                pCurrWrtShell->FillByEx(pColl);
                    // also apply template to remove hard set attributes
                SwTextFormatColl * pDerivedFrom = pCurrWrtShell->GetCurTextFormatColl();
                pColl->SetDerivedFrom(pDerivedFrom);

                    // set the mask at the Collection:
                sal_uInt16 nId = pColl->GetPoolFormatId() & 0x87ff;
                switch( nMask & static_cast<SfxStyleSearchBits>(0x0fff) )
                {
                    case SfxStyleSearchBits::SwText:
                        nId |= COLL_TEXT_BITS;
                        break;
                    case SfxStyleSearchBits::SwChapter:
                        nId |= COLL_DOC_BITS;
                        break;
                    case SfxStyleSearchBits::SwList:
                        nId |= COLL_LISTS_BITS;
                        break;
                    case SfxStyleSearchBits::SwIndex:
                        nId |= COLL_REGISTER_BITS;
                        break;
                    case SfxStyleSearchBits::SwExtra:
                        nId |= COLL_EXTRA_BITS;
                        break;
                    case SfxStyleSearchBits::SwHtml:
                        nId |= COLL_HTML_BITS;
                        break;
                    default: break;
                }
                pColl->SetPoolFormatId(nId);

                if (GetDoc()->GetIDocumentUndoRedo().DoesUndo())
                {
                    GetDoc()->GetIDocumentUndoRedo().AppendUndo(
                        std::make_unique<SwUndoTextFormatCollCreate>(pColl, pDerivedFrom, *GetDoc()));
                }
                pCurrWrtShell->SetTextFormatColl(pColl);
                pCurrWrtShell->EndAllAction();
            }
        }
        break;
        case SfxStyleFamily::Frame:
        {
            SwFrameFormat* pFrame = pStyle->GetFrameFormat();
            if(pCurrWrtShell->IsFrameSelected() && pFrame && !pFrame->IsDefault())
            {
                pCurrWrtShell->StartAllAction();

                SfxItemSet aSet(GetPool(), aFrameFormatSetRange );
                pCurrWrtShell->GetFlyFrameAttr( aSet );
                aSet.ClearItem(RES_ANCHOR); // tdf#112574 no anchor in styles

                SwFrameFormat* pFFormat = pCurrWrtShell->GetSelectedFrameFormat();
                pFrame->SetDerivedFrom( pFFormat );
                pFrame->SetFormatAttr( aSet );
                if (GetDoc()->GetIDocumentUndoRedo().DoesUndo())
                {
                    GetDoc()->GetIDocumentUndoRedo().AppendUndo(
                        std::make_unique<SwUndoFrameFormatCreate>(pFrame, pFFormat, *GetDoc()));
                }
                // also apply template to remove hard set attributes
                pCurrWrtShell->SetFrameFormat(pFrame);
                pCurrWrtShell->EndAllAction();
            }
        }
        break;
        case SfxStyleFamily::Char:
        {
            SwCharFormat* pChar = pStyle->GetCharFormat();
            if(pChar && !pChar->IsDefault())
            {
                pCurrWrtShell->StartAllAction();
                pCurrWrtShell->FillByEx( pChar );
                SwCharFormat * pDerivedFrom = pCurrWrtShell->GetCurCharFormat();
                pChar->SetDerivedFrom( pDerivedFrom );
                SwFormatCharFormat aFormat( pChar );

                if (GetDoc()->GetIDocumentUndoRedo().DoesUndo())
                {
                    // Looks like sometimes pDerivedFrom can be null and this is not supported by redo code
                    // So use default format as a derived from in such situations
                    GetDoc()->GetIDocumentUndoRedo().AppendUndo(
                        std::make_unique<SwUndoCharFormatCreate>(
                            pChar, pDerivedFrom ? pDerivedFrom : GetDoc()->GetDfltCharFormat(),
                            *GetDoc()));
                }
                pCurrWrtShell->SetAttrItem(aFormat);
                pCurrWrtShell->EndAllAction();
            }
        }
        break;

        case SfxStyleFamily::Page:
        {
            pCurrWrtShell->StartAllAction();
            size_t nPgDsc = pCurrWrtShell->GetCurPageDesc();
            SwPageDesc& rSrc = const_cast<SwPageDesc&>(pCurrWrtShell->GetPageDesc( nPgDsc ));
            SwPageDesc& rDest = *const_cast<SwPageDesc*>(pStyle->GetPageDesc());

            sal_uInt16 nPoolId = rDest.GetPoolFormatId();
            sal_uInt16 nHId = rDest.GetPoolHelpId();
            sal_uInt8 nHFId = rDest.GetPoolHlpFileId();

            pCurrWrtShell->GetDoc()->CopyPageDesc( rSrc, rDest );

            // PoolId must NEVER be copied!
            rDest.SetPoolFormatId( nPoolId );
            rDest.SetPoolHelpId( nHId );
            rDest.SetPoolHlpFileId( nHFId );

            // when Headers/Footers are created, there is no Undo anymore!
            pCurrWrtShell->GetDoc()->GetIDocumentUndoRedo().DelAllUndoObj();

            pCurrWrtShell->EndAllAction();
        }
        break;

        case SfxStyleFamily::Pseudo:
        {
            const SwNumRule* pCurRule = pCurrWrtShell->GetNumRuleAtCurrCursorPos();

            if (pCurRule)
            {
                pCurrWrtShell->StartAllAction();

                SwNumRule aRule( *pCurRule );
                OUString sOrigRule( aRule.GetName() );
                // #i91400#
                aRule.SetName( pStyle->GetNumRule()->GetName(),
                               pCurrWrtShell->GetDoc()->getIDocumentListsAccess() );
                pCurrWrtShell->ChgNumRuleFormats( aRule );

                pCurrWrtShell->ReplaceNumRule( sOrigRule, aRule.GetName() );

                pCurrWrtShell->EndAllAction();
            }
        }
        break;

        case SfxStyleFamily::Table:
        {
            SwTableAutoFormat* pFormat = pStyle->GetTableFormat();
            if (pCurrWrtShell->GetTableAutoFormat(*pFormat))
            {
                pCurrWrtShell->StartAllAction();

                pCurrWrtShell->SetTableStyle(rName);

                pCurrWrtShell->EndAllAction();
            }
        }
        break;

        default: break;
    }

    m_xDoc->BroadcastStyleOperation(rName, nFamily, SfxHintId::StyleSheetCreated);
}

sfx::AccessibilityIssueCollection SwDocShell::runAccessibilityCheck()
{
#ifndef ENABLE_WASM_STRIP_ACCESSIBILITY
    sw::AccessibilityCheck aCheck(m_xDoc.get());
    aCheck.check();
    return aCheck.getIssueCollection();
#else
    return sfx::AccessibilityIssueCollection();
#endif
}

std::set<Color> SwDocShell::GetDocColors()
{
    return m_xDoc->GetDocColors();
}

void  SwDocShell::LoadStyles( SfxObjectShell& rSource )
{
    LoadStyles_(rSource, false);
}

// bPreserveCurrentDocument determines whether SetFixFields() is called
// This call modifies the source document. This mustn't happen when the source
// is a document the user is working on.
// Calls of ::LoadStyles() normally use files especially loaded for the purpose
// of importing styles.
void SwDocShell::LoadStyles_( SfxObjectShell& rSource, bool bPreserveCurrentDocument )
{
/*  [Description]

    This method is called by SFx if Styles have to be reloaded from a
    document-template. Existing Styles should be overwritten by that.
    That's why the document has to be reformatted. Therefore applications
    will usually override this method and call the baseclass' implementation
    in their implementation.
*/
    // When the source is our document, we do the checking ourselves
    // (much quicker and doesn't use the crutch StxStylePool).
    if( dynamic_cast<const SwDocShell*>( &rSource) !=  nullptr)
    {
        // in order for the Headers/Footers not to get the fixed content
        // of the template, update all the Source's
        // FixFields once.
        if(!bPreserveCurrentDocument)
            static_cast<SwDocShell&>(rSource).m_xDoc->getIDocumentFieldsAccess().SetFixFields(nullptr);
        if (m_pWrtShell)
        {
            // rhbz#818557, fdo#58893: EndAllAction will call SelectShell(),
            // which pushes a bunch of SfxShells that are not cleared
            // (for unknown reasons) when closing the document, causing crash;
            // setting g_bNoInterrupt appears to avoid the problem.
            ::comphelper::FlagRestorationGuard g(g_bNoInterrupt, true);
            m_pWrtShell->StartAllAction();
            m_xDoc->ReplaceStyles( *static_cast<SwDocShell&>(rSource).m_xDoc );
            m_pWrtShell->EndAllAction();
        }
        else
        {
            bool bModified = m_xDoc->getIDocumentState().IsModified();
            m_xDoc->ReplaceStyles( *static_cast<SwDocShell&>(rSource).m_xDoc );
            if (!bModified && m_xDoc->getIDocumentState().IsModified() && !m_pView)
            {
                // the View is created later, but overwrites the Modify-Flag.
                // Undo doesn't work anymore anyways.
                m_xDoc->GetIDocumentUndoRedo().SetUndoNoResetModified();
            }
        }
    }
    else
        SfxObjectShell::LoadStyles( rSource );
}

void SwDocShell::FormatPage(
    weld::Window* pDialogParent,
    const OUString& rPage,
    const OString& rPageId,
    SwWrtShell& rActShell,
    SfxRequest* pRequest)
{
    Edit(pDialogParent, rPage, OUString(), SfxStyleFamily::Page, SfxStyleSearchBits::Auto, false, rPageId, &rActShell, pRequest);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
