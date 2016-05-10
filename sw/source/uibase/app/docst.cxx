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

#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

#include <comphelper/flagguard.hxx>

#include <hintids.hxx>
#include <sfx2/app.hxx>
#include <sfx2/styledlg.hxx>
#include <svl/whiter.hxx>
#include <sfx2/templdlg.hxx>
#include <sfx2/tplpitem.hxx>
#include <sfx2/request.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/newstyle.hxx>
#include <sfx2/printer.hxx>
#include <svl/macitem.hxx>
#include <svl/stritem.hxx>
#include <svl/languageoptions.hxx>
#include <editeng/eeitem.hxx>
#include <sfx2/htmlmode.hxx>
#include <swmodule.hxx>
#include <wdocsh.hxx>
#include <fmtfsize.hxx>
#include <fchrfmt.hxx>
#include <svtools/htmlcfg.hxx>
#include <svx/xdef.hxx>
#include <SwStyleNameMapper.hxx>
#include <SwRewriter.hxx>
#include <numrule.hxx>
#include <swundo.hxx>
#include <svx/svdmodel.hxx>
#include <svx/drawitem.hxx>
#include "view.hxx"
#include "wrtsh.hxx"
#include "docsh.hxx"
#include "uitool.hxx"
#include "cmdid.h"
#include "globals.hrc"
#include "viewopt.hxx"
#include <doc.hxx>
#include <drawdoc.hxx>
#include "IDocumentDrawModelAccess.hxx"
#include <IDocumentUndoRedo.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentListsAccess.hxx>
#include <IDocumentState.hxx>
#include "swstyle.h"
#include "frmfmt.hxx"
#include "charfmt.hxx"
#include "poolfmt.hxx"
#include "pagedesc.hxx"
#include "docstyle.hxx"
#include "uiitems.hxx"
#include "fmtcol.hxx"
#include "frmmgr.hxx"
#include "swevent.hxx"
#include "edtwin.hxx"
#include "unochart.hxx"
#include "app.hrc"
#include "swabstdlg.hxx"
#include <list.hxx>
#include <paratr.hxx>

extern bool g_bNoInterrupt;       // in swmodule.cxx

using namespace ::com::sun::star;

void  SwDocShell::StateStyleSheet(SfxItemSet& rSet, SwWrtShell* pSh)
{
    SfxWhichIter aIter(rSet);
    sal_uInt16  nWhich  = aIter.FirstWhich();
    sal_uInt16 nActualFamily = USHRT_MAX;

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
        SfxPoolItem* pItem = nullptr;
        pFrame->GetBindings().QueryState(SID_STYLE_FAMILY, pItem);
        SfxUInt16Item* pFamilyItem = dynamic_cast<SfxUInt16Item*>(pItem);
        if (pFamilyItem)
        {
            nActualFamily = static_cast<sal_uInt16>(SfxTemplate::NIdToSfxFamilyId(pFamilyItem->GetValue()));
        }
        delete pItem;
    }

    while (nWhich)
    {
        // determine current template to every family
        OUString aName;
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
                        aName = SwStyleNameMapper::GetTextUINameArray()[
                            RES_POOLCOLL_STANDARD - RES_POOLCOLL_TEXT_BEGIN ];
                    rSet.Put(SfxTemplateItem(nWhich, aName));
                }
                break;

            case SID_STYLE_FAMILY2:
                if(!pShell->IsFrameSelected())
                {
                    SwTextFormatColl* pColl = pShell->GetCurTextFormatColl();
                    if(pColl)
                        aName = pColl->GetName();

                    SfxTemplateItem aItem(nWhich, aName);

                    sal_uInt16 nMask = 0;
                    if (m_pDoc->getIDocumentSettingAccess().get(DocumentSettingId::HTML_MODE))
                        nMask = SWSTYLEBIT_HTML;
                    else
                    {
                        const FrameTypeFlags nSelection = pShell->GetFrameType(nullptr,true);
                        if(pShell->GetCurTOX())
                            nMask = SWSTYLEBIT_IDX  ;
                        else if(nSelection & FrameTypeFlags::HEADER     ||
                                nSelection & FrameTypeFlags::FOOTER     ||
                                nSelection & FrameTypeFlags::TABLE      ||
                                nSelection & FrameTypeFlags::FLY_ANY    ||
                                nSelection & FrameTypeFlags::FOOTNOTE   ||
                                nSelection & FrameTypeFlags::FTNPAGE)
                            nMask = SWSTYLEBIT_EXTRA;
                        else
                            nMask = SWSTYLEBIT_TEXT;
                    }

                    aItem.SetValue(nMask);
                    rSet.Put(aItem);
                }

                break;

            case SID_STYLE_FAMILY3:

                if (m_pDoc->getIDocumentSettingAccess().get(DocumentSettingId::HTML_MODE))
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
                SvxHtmlOptions& rHtmlOpt = SvxHtmlOptions::Get();
                if (m_pDoc->getIDocumentSettingAccess().get(DocumentSettingId::HTML_MODE) && !rHtmlOpt.IsPrintLayoutExtension())
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

            case SID_STYLE_WATERCAN:
            {
                SwEditWin& rEdtWin = pShell->GetView().GetEditWin();
                SwApplyTemplate* pApply = rEdtWin.GetApplyTemplate();
                rSet.Put(SfxBoolItem(nWhich, pApply && pApply->eType != 0));
            }
            break;
            case SID_STYLE_UPDATE_BY_EXAMPLE:
                if( pShell->IsFrameSelected()
                        ? SFX_STYLE_FAMILY_FRAME != nActualFamily
                        : ( SFX_STYLE_FAMILY_FRAME == nActualFamily ||
                            SFX_STYLE_FAMILY_PAGE == nActualFamily ||
                            (SFX_STYLE_FAMILY_PSEUDO == nActualFamily && !pShell->GetNumRuleAtCurrCursorPos())) )
                {
                    rSet.DisableItem( nWhich );
                }
                break;

            case SID_STYLE_NEW_BY_EXAMPLE:
                if( (pShell->IsFrameSelected()
                        ? SFX_STYLE_FAMILY_FRAME != nActualFamily
                        : SFX_STYLE_FAMILY_FRAME == nActualFamily) ||
                    (SFX_STYLE_FAMILY_PSEUDO == nActualFamily && !pShell->GetNumRuleAtCurrCursorPos()) )
                {
                    rSet.DisableItem( nWhich );
                }
                break;

            case SID_CLASSIFICATION_APPLY:
                // Just trigger ClassificationCategoriesController::statusChanged().
                rSet.InvalidateItem(nWhich);
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
    sal_uInt16  nRet    = SFXSTYLEBIT_ALL;

    const SfxItemSet* pArgs = rReq.GetArgs();
    const SfxPoolItem* pItem;
    switch (nSlot)
    {
    case SID_STYLE_NEW:
        if( pArgs && SfxItemState::SET == pArgs->GetItemState( SID_STYLE_FAMILY,
            false, &pItem ))
        {
            const sal_uInt16 nFamily = static_cast<const SfxUInt16Item*>(pItem)->GetValue();

            OUString sName;
            sal_uInt16 nMask = 0;
            if( SfxItemState::SET == pArgs->GetItemState( SID_STYLE_NEW,
                false, &pItem ))
                sName = static_cast<const SfxStringItem*>(pItem)->GetValue();
            if( SfxItemState::SET == pArgs->GetItemState( SID_STYLE_MASK,
                false, &pItem ))
                nMask = static_cast<const SfxUInt16Item*>(pItem)->GetValue();
            OUString sParent;
            if( SfxItemState::SET == pArgs->GetItemState( SID_STYLE_REFERENCE,
                false, &pItem ))
                sParent = static_cast<const SfxStringItem*>(pItem)->GetValue();

            if (sName.isEmpty() && m_xBasePool.get())
                sName = SfxStyleDialog::GenerateUnusedName(*m_xBasePool);

            nRet = Edit( sName, sParent, nFamily, nMask, true, OString(), nullptr, rReq.IsAPI() );
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

            SAL_FALLTHROUGH;

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
            sal_uInt16 nFamily = SFX_STYLE_FAMILY_PARA;
            sal_uInt16 nMask = 0;
            SwWrtShell* pActShell = nullptr;

            if( !pArgs )
            {
                nFamily = SFX_STYLE_FAMILY_PARA;

                switch (nSlot)
                {
                    case SID_STYLE_NEW_BY_EXAMPLE:
                    {
                        VclPtrInstance<SfxNewStyleDlg> pDlg( nullptr, *GetStyleSheetPool());
                        if(RET_OK == pDlg->Execute())
                        {
                            aParam = pDlg->GetName();
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
                    nFamily = static_cast<const SfxUInt16Item*>(pItem)->GetValue();

                if( SfxItemState::SET == pArgs->GetItemState(SID_STYLE_FAMILYNAME, false, &pItem ))
                {
                    OUString aFamily = static_cast<const SfxStringItem*>(pItem)->GetValue();
                    if(aFamily == "CharacterStyles")
                        nFamily = SFX_STYLE_FAMILY_CHAR;
                    else
                    if(aFamily == "ParagraphStyles")
                        nFamily = SFX_STYLE_FAMILY_PARA;
                    else
                    if(aFamily == "PageStyles")
                        nFamily = SFX_STYLE_FAMILY_PAGE;
                    else
                    if(aFamily == "FrameStyles")
                        nFamily = SFX_STYLE_FAMILY_FRAME;
                    else
                    if(aFamily == "NumberingStyles")
                        nFamily = SFX_STYLE_FAMILY_PSEUDO;
                }

                if( SfxItemState::SET == pArgs->GetItemState(SID_STYLE_MASK,
                    false, &pItem ))
                    nMask = static_cast<const SfxUInt16Item*>(pItem)->GetValue();
                if( SfxItemState::SET == pArgs->GetItemState(FN_PARAM_WRTSHELL,
                    false, &pItem ))
                    pActShell = pShell = static_cast<SwWrtShell*>(static_cast<const SwPtrItem*>(pItem)->GetValue());

                if( nSlot == SID_STYLE_UPDATE_BY_EXAMPLE )
                {
                    switch( nFamily )
                    {
                        case SFX_STYLE_FAMILY_PARA:
                        {
                            SwTextFormatColl* pColl = pShell->GetCurTextFormatColl();
                            if(pColl)
                                aParam = pColl->GetName();
                        }
                        break;
                        case SFX_STYLE_FAMILY_FRAME:
                        {
                            SwFrameFormat* pFrame = m_pWrtShell->GetSelectedFrameFormat();
                            if( pFrame )
                                aParam = pFrame->GetName();
                        }
                        break;
                        case SFX_STYLE_FAMILY_CHAR:
                        {
                            SwCharFormat* pChar = m_pWrtShell->GetCurCharFormat();
                            if( pChar )
                                aParam = pChar->GetName();
                        }
                        break;
                        case SFX_STYLE_FAMILY_PSEUDO:
                        if(SfxItemState::SET == pArgs->GetItemState(SID_STYLE_UPD_BY_EX_NAME, false, &pItem))
                        {
                            aParam = static_cast<const SfxStringItem*>(pItem)->GetValue();
                        }
                        break;
                    }
                    rReq.AppendItem(SfxStringItem(nSlot, aParam));
                }
            }
            if (!aParam.isEmpty() || nSlot == SID_STYLE_WATERCAN )
            {
                switch(nSlot)
                {
                    case SID_STYLE_EDIT:
                        nRet = Edit(aParam, aEmptyOUStr, nFamily, nMask, false, OString(), pActShell );
                        break;
                    case SID_STYLE_DELETE:
                        nRet = sal_uInt16(Delete(aParam, nFamily));
                        break;
                    case SID_STYLE_HIDE:
                    case SID_STYLE_SHOW:
                        nRet = sal_uInt16(Hide(aParam, nFamily, nSlot == SID_STYLE_HIDE));
                        break;
                    case SID_STYLE_APPLY:
                        // Shell-switch in ApplyStyles
                        nRet = ApplyStyles(aParam, nFamily, pActShell, rReq.GetModifier() );
                        break;
                    case SID_STYLE_WATERCAN:
                        nRet = DoWaterCan(aParam, nFamily);
                        break;
                    case SID_STYLE_UPDATE_BY_EXAMPLE:
                        nRet = UpdateStyle(aParam, nFamily, pActShell);
                        break;
                    case SID_STYLE_NEW_BY_EXAMPLE:
                        nRet = MakeByExample(aParam, nFamily, nMask, pActShell );
                        break;

                    default:
                        OSL_FAIL("Invalid SlotId");
                }

                rReq.Done();
            }

            break;
        }
    }

        if(rReq.IsAPI()) // Basic only gets TRUE or FALSE
            rReq.SetReturnValue(SfxUInt16Item(nSlot, sal_uInt16(nRet !=0)));
        else
            rReq.SetReturnValue(SfxUInt16Item(nSlot, nRet));
}

class ApplyStyle
{
public:
    ApplyStyle(SwDocShell &rDocSh, bool bNew, SfxStyleSheetBase* pStyle,
        sal_uInt16 nRet, rtl::Reference< SwDocStyleSheet > xTmp,
        sal_uInt16 nFamily, SfxAbstractApplyTabDialog *pDlg,
        rtl::Reference< SfxStyleSheetBasePool > xBasePool,
        bool bModified)
        : m_rDocSh(rDocSh)
        , m_bNew(bNew)
        , m_pStyle(pStyle)
        , m_nRet(nRet)
        , m_xTmp(xTmp)
        , m_nFamily(nFamily)
        , m_pDlg(pDlg)
        , m_xBasePool(xBasePool)
        , m_bModified(bModified)
    {
    }
    DECL_LINK_TYPED( ApplyHdl, LinkParamNone*, void );
    void apply()
    {
        ApplyHdl(nullptr);
    }
    sal_uInt16 getRet() const { return m_nRet; }
private:
    SwDocShell &m_rDocSh;
    bool m_bNew;
    SfxStyleSheetBase* m_pStyle;
    sal_uInt16 m_nRet;
    rtl::Reference< SwDocStyleSheet > m_xTmp;
    sal_uInt16 m_nFamily;
    SfxAbstractApplyTabDialog *m_pDlg;
    rtl::Reference< SfxStyleSheetBasePool > m_xBasePool;
    bool m_bModified;
};

IMPL_LINK_NOARG_TYPED(ApplyStyle, ApplyHdl, LinkParamNone*, void)
{
    SwWrtShell* pWrtShell = m_rDocSh.GetWrtShell();
    SwDoc* pDoc = m_rDocSh.GetDoc();
    SwView* pView = m_rDocSh.GetView();

    pWrtShell->StartAllAction();

    // newly set the mask only with paragraph-templates
    if( m_bNew )
    {
        m_nRet = SFX_STYLE_FAMILY_PARA == m_pStyle->GetFamily()
                ? m_xTmp->GetMask()
                : SFXSTYLEBIT_USERDEF;
    }
    else if( m_pStyle->GetMask() != m_xTmp->GetMask() )
        m_nRet = m_xTmp->GetMask();

    if( SFX_STYLE_FAMILY_PARA == m_nFamily )
    {
        SfxItemSet aSet( *m_pDlg->GetOutputItemSet() );
        ::SfxToSwPageDescAttr( *pWrtShell, aSet  );
        // reset indent attributes at paragraph style, if a list style
        // will be applied and no indent attributes will be applied.
        m_xTmp->SetItemSet( aSet, true );
    }
    else
    {
        if(SFX_STYLE_FAMILY_PAGE == m_nFamily)
        {
            static const sal_uInt16 aInval[] = {
                SID_IMAGE_ORIENTATION,
                SID_ATTR_CHAR_FONT,
                FN_INSERT_CTRL, FN_INSERT_OBJ_CTRL, 0};
            pView->GetViewFrame()->GetBindings().Invalidate(aInval);
        }
        SfxItemSet aTmpSet( *m_pDlg->GetOutputItemSet() );
        if( SFX_STYLE_FAMILY_CHAR == m_nFamily )
        {
            ::ConvertAttrGenToChar(aTmpSet, m_xTmp->GetItemSet(), CONV_ATTR_STD);
        }

        m_xTmp->SetItemSet( aTmpSet );

        if( SFX_STYLE_FAMILY_PAGE == m_nFamily && SvtLanguageOptions().IsCTLFontEnabled() )
        {
            const SfxPoolItem *pItem = nullptr;
            if( aTmpSet.GetItemState( m_rDocSh.GetPool().GetTrueWhich( SID_ATTR_FRAMEDIRECTION, false ) , true, &pItem ) == SfxItemState::SET )
                SwChartHelper::DoUpdateAllCharts( pDoc );
        }
    }

    //UUUU
    if(m_bNew)
    {
        if(SFX_STYLE_FAMILY_FRAME == m_nFamily || SFX_STYLE_FAMILY_PARA == m_nFamily)
        {
            // clear FillStyle so that it works as a derived attribute
            SfxItemSet aTmpSet(*m_pDlg->GetOutputItemSet());

            aTmpSet.ClearItem(XATTR_FILLSTYLE);
            m_xTmp->SetItemSet(aTmpSet);
        }
    }

    if(SFX_STYLE_FAMILY_PAGE == m_nFamily)
        pView->InvalidateRulerPos();

    if( m_bNew )
        m_xBasePool->Broadcast( SfxStyleSheetHint( SfxStyleSheetHintId::CREATED, *m_xTmp.get() ) );

    pDoc->getIDocumentState().SetModified();
    if( !m_bModified )
    {
        pDoc->GetIDocumentUndoRedo().SetUndoNoResetModified();
    }

    pWrtShell->EndAllAction();
}

sal_uInt16 SwDocShell::Edit(
    const OUString &rName,
    const OUString &rParent,
    const sal_uInt16 nFamily,
    sal_uInt16 nMask,
    const bool bNew,
    const OString& sPage,
    SwWrtShell* pActShell,
    const bool bBasic )
{
    assert( GetWrtShell() );
    SfxStyleSheetBase *pStyle = nullptr;

    sal_uInt16 nRet = nMask;
    bool bModified = m_pDoc->getIDocumentState().IsModified();

    SwUndoId nNewStyleUndoId(UNDO_EMPTY);

    if( bNew )
    {
        if (!bBasic)
        {
            // start undo action in order to get only one undo action for the
            // UI new style + change style operations
            m_pWrtShell->StartUndo();
        }

        if( SFXSTYLEBIT_ALL != nMask && SFXSTYLEBIT_ALL_VISIBLE != nMask && SFXSTYLEBIT_USED != nMask )
            nMask |= SFXSTYLEBIT_USERDEF;
        else
            nMask = SFXSTYLEBIT_USERDEF;

        pStyle = &m_xBasePool->Make( rName, (SfxStyleFamily)nFamily, nMask );

        // set the current one as Parent
        SwDocStyleSheet* pDStyle = static_cast<SwDocStyleSheet*>(pStyle);
        switch( nFamily )
        {
            case SFX_STYLE_FAMILY_PARA:
            {
                if(!rParent.isEmpty())
                {
                    SwTextFormatColl* pColl = m_pWrtShell->FindTextFormatCollByName( rParent );
                    if(!pColl)
                    {
                        sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName(rParent, nsSwGetPoolIdFromName::GET_POOLID_TXTCOLL);
                        if(USHRT_MAX != nId)
                            pColl = m_pWrtShell->GetTextCollFromPool( nId );
                    }
                    pDStyle->GetCollection()->SetDerivedFrom( pColl );
                    pDStyle->PresetParent( rParent );

                     /*When a new paragraph style is created based on a "to outline style
                        assigned" paragraph style, the outline level attribute and the list
                        style attribute of the new paragraph style have to be set to 0
                        respectively "".*/
                    if (pColl && pColl->IsAssignedToListLevelOfOutlineStyle())
                    {
                        SwNumRuleItem aItem(aEmptyOUStr);
                        pDStyle->GetCollection()->SetFormatAttr( aItem );
                        pDStyle->GetCollection()->SetAttrOutlineLevel( 0 );
                    }
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
            case SFX_STYLE_FAMILY_CHAR:
            {
                if(!rParent.isEmpty())
                {
                    SwCharFormat* pCFormat = m_pWrtShell->FindCharFormatByName(rParent);
                    if(!pCFormat)
                    {
                        sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName(rParent, nsSwGetPoolIdFromName::GET_POOLID_CHRFMT);
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
            case SFX_STYLE_FAMILY_FRAME :
            {
                if(!rParent.isEmpty())
                {
                    SwFrameFormat* pFFormat = m_pWrtShell->GetDoc()->FindFrameFormatByName( rParent );
                    if(!pFFormat)
                    {
                        sal_uInt16 nId = SwStyleNameMapper::GetPoolIdFromUIName(rParent, nsSwGetPoolIdFromName::GET_POOLID_FRMFMT);
                        if(USHRT_MAX != nId)
                            pFFormat = m_pWrtShell->GetFrameFormatFromPool( nId );
                    }
                    pDStyle->GetFrameFormat()->SetDerivedFrom( pFFormat );
                    pDStyle->PresetParent( rParent );
                }
            }
            break;
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
        pStyle = m_xBasePool->Find( rName, (SfxStyleFamily)nFamily );
        SAL_WARN_IF( !pStyle, "sw.ui", "Style not found" );
    }

    if(!pStyle)
        return 0;

    // put dialogues together
    rtl::Reference< SwDocStyleSheet > xTmp( new SwDocStyleSheet( *static_cast<SwDocStyleSheet*>(pStyle) ) );
    if( SFX_STYLE_FAMILY_PARA == nFamily )
    {
        SfxItemSet& rSet = xTmp->GetItemSet();
        ::SwToSfxPageDescAttr( rSet );
        // merge list level indent attributes into the item set if needed
        xTmp->MergeIndentAttrsOfListStyle( rSet );
    }
    else if( SFX_STYLE_FAMILY_CHAR == nFamily )
    {
        ::ConvertAttrCharToGen(xTmp->GetItemSet(), CONV_ATTR_STD);
    }

    if(SFX_STYLE_FAMILY_PAGE == nFamily || SFX_STYLE_FAMILY_PARA == nFamily)
    {
        //UUUU create needed items for XPropertyList entries from the DrawModel so that
        // the Area TabPage can access them
        SfxItemSet& rSet = xTmp->GetItemSet();
        const SwDrawModel* pDrawModel = GetDoc()->getIDocumentDrawModelAccess().GetDrawModel();

        rSet.Put(SvxColorListItem(pDrawModel->GetColorList(), SID_COLOR_TABLE));
        rSet.Put(SvxGradientListItem(pDrawModel->GetGradientList(), SID_GRADIENT_LIST));
        rSet.Put(SvxHatchListItem(pDrawModel->GetHatchList(), SID_HATCH_LIST));
        rSet.Put(SvxBitmapListItem(pDrawModel->GetBitmapList(), SID_BITMAP_LIST));
    }

    if (!bBasic)
    {
        // prior to the dialog the HtmlMode at the DocShell is being sunk
        sal_uInt16 nHtmlMode = ::GetHtmlMode(this);

        // In HTML mode, we do not always have a printer. In order to show
        // the correct page size in the Format - Page dialog, we have to
        // get one here.
        SwWrtShell* pCurrShell = (pActShell) ? pActShell : m_pWrtShell;
        if( ( HTMLMODE_ON & nHtmlMode ) &&
            !pCurrShell->getIDocumentDeviceAccess().getPrinter( false ) )
            pCurrShell->InitPrt( pCurrShell->getIDocumentDeviceAccess().getPrinter( true ) );

        PutItem(SfxUInt16Item(SID_HTML_MODE, nHtmlMode));
        FieldUnit eMetric = ::GetDfltMetric(0 != (HTMLMODE_ON&nHtmlMode));
        SW_MOD()->PutItem(SfxUInt16Item(SID_ATTR_METRIC, static_cast< sal_uInt16 >(eMetric)));
        SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
        assert( pFact );
        std::unique_ptr<SfxAbstractApplyTabDialog> pDlg(pFact->CreateTemplateDialog(
                                                    nullptr, *(xTmp.get()), nFamily, sPage,
                                                    pActShell ? pActShell : m_pWrtShell, bNew));
        assert( pDlg );
        ApplyStyle aApplyStyleHelper(*this, bNew, pStyle, nRet, xTmp, nFamily, pDlg.get(), m_xBasePool, bModified);
        pDlg->SetApplyHdl(LINK(&aApplyStyleHelper, ApplyStyle, ApplyHdl));

        short nDlgRet = pDlg->Execute();

        if (RET_OK == nDlgRet)
        {
            aApplyStyleHelper.apply();
        }

        if (bNew)
        {
            SwRewriter aRewriter;
            aRewriter.AddRule(UndoArg1, xTmp->GetName());
            //Group the create style and change style operations together under the
            //one "create style" comment
            m_pWrtShell->EndUndo(nNewStyleUndoId, &aRewriter);
        }

        if (RET_OK != nDlgRet)
        {
            if( bNew )
            {
                GetWrtShell()->Undo();
                m_pDoc->GetIDocumentUndoRedo().ClearRedo();
            }

            if( !bModified )
                m_pDoc->getIDocumentState().ResetModified();
        }

        nRet = aApplyStyleHelper.getRet();
    }
    else
    {
        // prior to the dialog the HtmlMode at the DocShell is being sunk
        PutItem(SfxUInt16Item(SID_HTML_MODE, ::GetHtmlMode(this)));

        GetWrtShell()->StartAllAction();

        // newly set the mask only with paragraph-templates
        if( bNew )
        {
            nRet = SFX_STYLE_FAMILY_PARA == pStyle->GetFamily()
                    ? xTmp->GetMask()
                    : SFXSTYLEBIT_USERDEF;
        }
        else if( pStyle->GetMask() != xTmp->GetMask() )
            nRet = xTmp->GetMask();

        if( SFX_STYLE_FAMILY_PARA == nFamily )
            ::SfxToSwPageDescAttr( *GetWrtShell(), xTmp->GetItemSet() );
        else
        {
            ::ConvertAttrGenToChar(xTmp->GetItemSet(), xTmp->GetItemSet(), CONV_ATTR_STD);
        }
        if(SFX_STYLE_FAMILY_PAGE == nFamily)
            m_pView->InvalidateRulerPos();

        if( bNew )
            m_xBasePool->Broadcast( SfxStyleSheetHint( SfxStyleSheetHintId::CREATED, *xTmp.get() ) );

        m_pDoc->getIDocumentState().SetModified();
        if( !bModified )        // Bug 57028
        {
            m_pDoc->GetIDocumentUndoRedo().SetUndoNoResetModified();
        }
        GetWrtShell()->EndAllAction();
    }

    return nRet;
}

bool SwDocShell::Delete(const OUString &rName, sal_uInt16 nFamily)
{
    SfxStyleSheetBase *pStyle = m_xBasePool->Find(rName, (SfxStyleFamily)nFamily);

    if(pStyle)
    {
        assert( GetWrtShell() );

        GetWrtShell()->StartAllAction();
        m_xBasePool->Remove(pStyle);
        GetWrtShell()->EndAllAction();

        return true;
    }
    return false;
}

bool SwDocShell::Hide(const OUString &rName, sal_uInt16 nFamily, bool bHidden)
{
    SfxStyleSheetBase *pStyle = m_xBasePool->Find(rName, (SfxStyleFamily)nFamily);

    if(pStyle)
    {
        assert( GetWrtShell() );

        GetWrtShell()->StartAllAction();
        rtl::Reference< SwDocStyleSheet > xTmp( new SwDocStyleSheet( *static_cast<SwDocStyleSheet*>(pStyle) ) );
        xTmp->SetHidden( bHidden );
        GetWrtShell()->EndAllAction();

        return true;
    }
    return false;
}

// apply template
sal_uInt16 SwDocShell::ApplyStyles(const OUString &rName, sal_uInt16 nFamily,
                               SwWrtShell* pShell, const sal_uInt16 nMode )
{
    SwDocStyleSheet* pStyle = static_cast<SwDocStyleSheet*>( m_xBasePool->Find( rName, (SfxStyleFamily) nFamily ) );

    SAL_WARN_IF( !pStyle, "sw.ui", "Style not found" );

    if(!pStyle)
        return 0;

    SwWrtShell *pSh = pShell ? pShell : GetWrtShell();

    assert( pSh );

    pSh->StartAllAction();

    switch (nFamily)
    {
        case SFX_STYLE_FAMILY_CHAR:
        {
            SwFormatCharFormat aFormat(pStyle->GetCharFormat());
            pSh->SetAttrItem( aFormat, (nMode & KEY_SHIFT) ?
                SetAttrMode::DONTREPLACE : SetAttrMode::DEFAULT );
            break;
        }
        case SFX_STYLE_FAMILY_PARA:
        {
            // #i62675#
            // clear also list attributes at affected text nodes, if paragraph
            // style has the list style attribute set.
            pSh->SetTextFormatColl( pStyle->GetCollection(), true );
            break;
        }
        case SFX_STYLE_FAMILY_FRAME:
        {
            if ( pSh->IsFrameSelected() )
                pSh->SetFrameFormat( pStyle->GetFrameFormat() );
            break;
        }
        case SFX_STYLE_FAMILY_PAGE:
        {
            pSh->SetPageStyle(pStyle->GetPageDesc()->GetName());
            break;
        }
        case SFX_STYLE_FAMILY_PSEUDO:
        {
            // reset indent attribute on applying list style
            // continue list of list style
            const SwNumRule* pNumRule = pStyle->GetNumRule();
            const OUString sListIdForStyle =pNumRule->GetDefaultListId();
            pSh->SetCurNumRule( *pNumRule, false, sListIdForStyle, true );
            break;
        }
        default:
            OSL_FAIL("Unknown family");
    }
    pSh->EndAllAction();

    return nFamily;
}

// start watering-can
sal_uInt16 SwDocShell::DoWaterCan(const OUString &rName, sal_uInt16 nFamily)
{
    assert( GetWrtShell() );

    SwEditWin& rEdtWin = m_pView->GetEditWin();
    SwApplyTemplate* pApply = rEdtWin.GetApplyTemplate();
    bool bWaterCan = !(pApply && pApply->eType != 0);

    if( rName.isEmpty() )
        bWaterCan = false;

    SwApplyTemplate aTemplate;
    aTemplate.eType = nFamily;

    if(bWaterCan)
    {
        SwDocStyleSheet* pStyle =
            static_cast<SwDocStyleSheet*>( m_xBasePool->Find(rName, (SfxStyleFamily)nFamily) );

        SAL_WARN_IF( !pStyle, "sw.ui", "Where's the StyleSheet" );

        if(!pStyle) return nFamily;

        switch(nFamily)
        {
            case SFX_STYLE_FAMILY_CHAR:
                aTemplate.aColl.pCharFormat = pStyle->GetCharFormat();
                break;
            case SFX_STYLE_FAMILY_PARA:
                aTemplate.aColl.pTextColl = pStyle->GetCollection();
                break;
            case SFX_STYLE_FAMILY_FRAME:
                aTemplate.aColl.pFrameFormat = pStyle->GetFrameFormat();
                break;
            case SFX_STYLE_FAMILY_PAGE:
                aTemplate.aColl.pPageDesc = const_cast<SwPageDesc*>(pStyle->GetPageDesc());
                break;
            case SFX_STYLE_FAMILY_PSEUDO:
                aTemplate.aColl.pNumRule = const_cast<SwNumRule*>(pStyle->GetNumRule());
                break;

            default:
                OSL_FAIL("Unknown family");
        }
    }
    else
        aTemplate.eType = 0;

    m_pView->GetEditWin().SetApplyTemplate(aTemplate);

    return nFamily;
}

// update template
sal_uInt16 SwDocShell::UpdateStyle(const OUString &rName, sal_uInt16 nFamily, SwWrtShell* pShell)
{
    SwWrtShell* pCurrWrtShell = pShell ? pShell : GetWrtShell();
    assert( pCurrWrtShell );

    SwDocStyleSheet* pStyle =
        static_cast<SwDocStyleSheet*>( m_xBasePool->Find(rName, (SfxStyleFamily)nFamily) );

    if(!pStyle)
        return nFamily;

    switch(nFamily)
    {
        case SFX_STYLE_FAMILY_PARA:
        {
            SwTextFormatColl* pColl = pStyle->GetCollection();
            if(pColl && !pColl->IsDefault())
            {
                GetWrtShell()->StartAllAction();

                SwRewriter aRewriter;
                aRewriter.AddRule(UndoArg1, pColl->GetName());

                GetWrtShell()->StartUndo(UNDO_INSFMTATTR, &aRewriter);
                GetWrtShell()->FillByEx(pColl);
                    // also apply template to remove hard set attributes
                GetWrtShell()->SetTextFormatColl( pColl );
                GetWrtShell()->EndUndo();
                GetWrtShell()->EndAllAction();
            }
            break;
        }
        case SFX_STYLE_FAMILY_FRAME:
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
        case SFX_STYLE_FAMILY_CHAR:
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
        case SFX_STYLE_FAMILY_PSEUDO:
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
    }
    return nFamily;
}

// NewByExample
sal_uInt16 SwDocShell::MakeByExample( const OUString &rName, sal_uInt16 nFamily,
                                    sal_uInt16 nMask, SwWrtShell* pShell )
{
    SwWrtShell* pCurrWrtShell = pShell ? pShell : GetWrtShell();
    SwDocStyleSheet* pStyle = static_cast<SwDocStyleSheet*>( m_xBasePool->Find(
                                            rName, (SfxStyleFamily)nFamily ) );
    if(!pStyle)
    {
        // preserve the current mask of PI, then the new one is
        // immediately merged with the viewable area
        if( SFXSTYLEBIT_ALL == nMask || SFXSTYLEBIT_USED == nMask )
            nMask = SFXSTYLEBIT_USERDEF;
        else
            nMask |= SFXSTYLEBIT_USERDEF;

        pStyle = static_cast<SwDocStyleSheet*>( &m_xBasePool->Make(rName,
                                (SfxStyleFamily)nFamily, nMask ) );
    }

    switch(nFamily)
    {
        case  SFX_STYLE_FAMILY_PARA:
        {
            SwTextFormatColl* pColl = pStyle->GetCollection();
            if(pColl && !pColl->IsDefault())
            {
                pCurrWrtShell->StartAllAction();
                pCurrWrtShell->FillByEx(pColl);
                    // also apply template to remove hard set attributes
                pColl->SetDerivedFrom(pCurrWrtShell->GetCurTextFormatColl());

                    // set the mask at the Collection:
                sal_uInt16 nId = pColl->GetPoolFormatId() & 0x87ff;
                switch( nMask & 0x0fff )
                {
                    case SWSTYLEBIT_TEXT:
                        nId |= COLL_TEXT_BITS;
                        break;
                    case SWSTYLEBIT_CHAPTER:
                        nId |= COLL_DOC_BITS;
                        break;
                    case SWSTYLEBIT_LIST:
                        nId |= COLL_LISTS_BITS;
                        break;
                    case SWSTYLEBIT_IDX:
                        nId |= COLL_REGISTER_BITS;
                        break;
                    case SWSTYLEBIT_EXTRA:
                        nId |= COLL_EXTRA_BITS;
                        break;
                    case SWSTYLEBIT_HTML:
                        nId |= COLL_HTML_BITS;
                        break;
                }
                pColl->SetPoolFormatId(nId);

                pCurrWrtShell->SetTextFormatColl(pColl);
                pCurrWrtShell->EndAllAction();
            }
        }
        break;
        case SFX_STYLE_FAMILY_FRAME:
        {
            SwFrameFormat* pFrame = pStyle->GetFrameFormat();
            if(pCurrWrtShell->IsFrameSelected() && pFrame && !pFrame->IsDefault())
            {
                pCurrWrtShell->StartAllAction();

                SfxItemSet aSet(GetPool(), aFrameFormatSetRange );
                pCurrWrtShell->GetFlyFrameAttr( aSet );

                SwFrameFormat* pFFormat = pCurrWrtShell->GetSelectedFrameFormat();
                pFrame->SetDerivedFrom( pFFormat );

                pFrame->SetFormatAttr( aSet );
                    // also apply template to remove hard set attributes
                pCurrWrtShell->SetFrameFormat( pFrame );
                pCurrWrtShell->EndAllAction();
            }
        }
        break;
        case SFX_STYLE_FAMILY_CHAR:
        {
            SwCharFormat* pChar = pStyle->GetCharFormat();
            if(pChar && !pChar->IsDefault())
            {
                pCurrWrtShell->StartAllAction();
                pCurrWrtShell->FillByEx( pChar );
                pChar->SetDerivedFrom( pCurrWrtShell->GetCurCharFormat() );
                SwFormatCharFormat aFormat( pChar );
                pCurrWrtShell->SetAttrItem( aFormat );
                pCurrWrtShell->EndAllAction();
            }
        }
        break;

        case SFX_STYLE_FAMILY_PAGE:
        {
            pCurrWrtShell->StartAllAction();
            size_t nPgDsc = pCurrWrtShell->GetCurPageDesc();
            SwPageDesc& rSrc = (SwPageDesc&)pCurrWrtShell->GetPageDesc( nPgDsc );
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

        case SFX_STYLE_FAMILY_PSEUDO:
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
    }
    return nFamily;
}

std::set<Color> SwDocShell::GetDocColors()
{
    return m_pDoc->GetDocColors();
}

void  SwDocShell::LoadStyles( SfxObjectShell& rSource )
{
    _LoadStyles(rSource, false);
}

// bPreserveCurrentDocument determines whether SetFixFields() is called
// This call modifies the source document. This mustn't happen when the source
// is a document the user is working on.
// Calls of ::LoadStyles() normally use files especially loaded for the purpose
// of importing styles.
void SwDocShell::_LoadStyles( SfxObjectShell& rSource, bool bPreserveCurrentDocument )
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
            static_cast<SwDocShell&>(rSource).m_pDoc->getIDocumentFieldsAccess().SetFixFields(false, nullptr);
        if (m_pWrtShell)
        {
            // rhbz#818557, fdo#58893: EndAllAction will call SelectShell(),
            // which pushes a bunch of SfxShells that are not cleared
            // (for unknown reasons) when closing the document, causing crash;
            // setting g_bNoInterrupt appears to avoid the problem.
            ::comphelper::FlagRestorationGuard g(g_bNoInterrupt, true);
            m_pWrtShell->StartAllAction();
            m_pDoc->ReplaceStyles( *static_cast<SwDocShell&>(rSource).m_pDoc );
            m_pWrtShell->EndAllAction();
        }
        else
        {
            bool bModified = m_pDoc->getIDocumentState().IsModified();
            m_pDoc->ReplaceStyles( *static_cast<SwDocShell&>(rSource).m_pDoc );
            if (!bModified && m_pDoc->getIDocumentState().IsModified() && !m_pView)
            {
                // the View is created later, but overwrites the Modify-Flag.
                // Undo doesn't work anymore anyways.
                m_pDoc->GetIDocumentUndoRedo().SetUndoNoResetModified();
            }
        }
    }
    else
        SfxObjectShell::LoadStyles( rSource );
}

void SwDocShell::FormatPage(
    const OUString& rPage,
    const OString& rPageId,
    SwWrtShell& rActShell )
{
    Edit( rPage, aEmptyOUStr, SFX_STYLE_FAMILY_PAGE, 0, false, rPageId, &rActShell);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
