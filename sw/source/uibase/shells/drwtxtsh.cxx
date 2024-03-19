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

#include <hintids.hxx>
#include <i18nlangtag/lang.h>
#include <i18nutil/transliteration.hxx>
#include <rtl/ustrbuf.hxx>
#include <svl/slstitm.hxx>
#include <svl/stritem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/editund2.hxx>
#include <svx/svdview.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/objface.hxx>
#include <svx/svdotext.hxx>
#include <svx/sdooitm.hxx>
#include <editeng/editeng.hxx>
#include <editeng/editview.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/scripttypeitem.hxx>
#include <sfx2/bindings.hxx>
#include <svx/fontwork.hxx>
#include <sfx2/request.hxx>
#include <vcl/EnumContext.hxx>
#include <svl/whiter.hxx>
#include <editeng/outliner.hxx>
#include <editeng/editstat.hxx>
#include <svx/svdoutl.hxx>
#include <com/sun/star/i18n/TextConversionOption.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <comphelper/propertysequence.hxx>
#include <osl/diagnose.h>
#include <swtypes.hxx>
#include <view.hxx>
#include <wrtsh.hxx>
#include <viewopt.hxx>
#include <drwtxtsh.hxx>
#include <breakit.hxx>

#include <cmdid.h>

#define ShellClass_SwDrawTextShell
#include <sfx2/msg.hxx>
#include <swslots.hxx>
#include <uitool.hxx>
#include <wview.hxx>
#include <swmodule.hxx>
#include <svx/svdoashp.hxx>
#include <svx/svxdlg.hxx>
#include <comphelper/processfactory.hxx>
#include <IDocumentUndoRedo.hxx>
#include <memory>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::i18n;

SFX_IMPL_INTERFACE(SwDrawTextShell, SfxShell)

void SwDrawTextShell::InitInterface_Impl()
{
    GetStaticInterface()->RegisterPopupMenu("drawtext");

    GetStaticInterface()->RegisterObjectBar(SFX_OBJECTBAR_OBJECT, SfxVisibilityFlags::Invisible, ToolbarId::Draw_Text_Toolbox_Sw);

    GetStaticInterface()->RegisterChildWindow(SvxFontWorkChildWindow::GetChildWindowId());
}


void SwDrawTextShell::Init()
{
    SwWrtShell &rSh = GetShell();
    m_pSdrView = rSh.GetDrawView();
    SdrOutliner * pOutliner = m_pSdrView->GetTextEditOutliner();
    //#97471# mouse click _and_ key input at the same time
    if( !pOutliner )
        return ;
    OutlinerView* pOLV = m_pSdrView->GetTextEditOutlinerView();
    EEControlBits nCtrl = pOutliner->GetControlWord();
    nCtrl |= EEControlBits::AUTOCORRECT;

    SetUndoManager(&pOutliner->GetUndoManager());

    // Now let's try an AutoSpell.

    const SwViewOption* pVOpt = rSh.GetViewOptions();
    if(pVOpt->IsOnlineSpell())
    {
        nCtrl |= EEControlBits::ONLINESPELLING|EEControlBits::ALLOWBIGOBJS;
    }
    else
        nCtrl &= ~EEControlBits::ONLINESPELLING;

    pOutliner->SetControlWord(nCtrl);
    pOLV->ShowCursor();
}

SwDrawTextShell::SwDrawTextShell(SwView &rV) :
    SfxShell(&rV),
    m_rView(rV)
{
    SwWrtShell &rSh = GetShell();
    SetPool(rSh.GetAttrPool().GetSecondaryPool());

    // Initialize and show cursor to start editing.
    Init();

    SetName("ObjectText");
    SfxShell::SetContextName(vcl::EnumContext::GetContextName(vcl::EnumContext::Context::DrawText));
}

SwDrawTextShell::~SwDrawTextShell()
{
    if ( GetView().GetCurShell() == this )
        m_rView.ResetSubShell();
}

SwWrtShell& SwDrawTextShell::GetShell()
{
    return m_rView.GetWrtShell();
}

// Disable slots with this status method

void SwDrawTextShell::StateDisableItems( SfxItemSet &rSet )
{
    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();

    while (nWhich)
    {
        rSet.DisableItem( nWhich );
        nWhich = aIter.NextWhich();
    }
}

void SwDrawTextShell::SetAttrToMarked(const SfxItemSet& rAttr)
{
    OutlinerView* pOLV = m_pSdrView->GetTextEditOutlinerView();
    tools::Rectangle aOutRect = pOLV->GetOutputArea();

    if (tools::Rectangle() != aOutRect)
    {
        GetShell().GetDrawView()->SetAttributes(rAttr);
    }
}

bool SwDrawTextShell::IsTextEdit() const
{
    return m_pSdrView->IsTextEdit();
}

void SwDrawTextShell::ExecFontWork(SfxRequest const & rReq)
{
    SwWrtShell &rSh = GetShell();
    FieldUnit eMetric = ::GetDfltMetric( dynamic_cast<SwWebView*>( &rSh.GetView()) != nullptr );
    SW_MOD()->PutItem(SfxUInt16Item(SID_ATTR_METRIC, static_cast< sal_uInt16 >(eMetric)) );
    SfxViewFrame& rVFrame = GetView().GetViewFrame();
    if ( rReq.GetArgs() )
    {
        rVFrame.SetChildWindow(SvxFontWorkChildWindow::GetChildWindowId(),
                                static_cast<const SfxBoolItem&>( (rReq.GetArgs()->
                                Get(SID_FONTWORK))).GetValue());
    }
    else
        rVFrame.ToggleChildWindow(SvxFontWorkChildWindow::GetChildWindowId());

    rVFrame.GetBindings().Invalidate(SID_FONTWORK);
}

void SwDrawTextShell::StateFontWork(SfxItemSet& rSet)
{
    const sal_uInt16 nId = SvxFontWorkChildWindow::GetChildWindowId();
    rSet.Put(SfxBoolItem(SID_FONTWORK, GetView().GetViewFrame().HasChildWindow(nId)));
}

// Edit SfxRequests for FontWork

void SwDrawTextShell::ExecFormText(SfxRequest const & rReq)
{
    SwWrtShell &rSh = GetShell();
    SdrView* pDrView = rSh.GetDrawView();

    const SdrMarkList& rMarkList = pDrView->GetMarkedObjectList();

    if ( !(rMarkList.GetMarkCount() == 1 && rReq.GetArgs()) )
        return;

    const SfxItemSet& rSet = *rReq.GetArgs();

    if ( pDrView->IsTextEdit() )
    {
        //#111733# Sometimes SdrEndTextEdit() initiates the change in selection and
        // 'this' is not valid anymore
        SwView& rTempView = GetView();
        pDrView->SdrEndTextEdit(true);
        //this removes the current shell from the dispatcher stack!!
        rTempView.AttrChangedNotify(nullptr);
    }

    pDrView->SetAttributes(rSet);

}

// Return Status values back to FontWork

void SwDrawTextShell::GetFormTextState(SfxItemSet& rSet)
{
    SwWrtShell &rSh = GetShell();
    SdrView* pDrView = rSh.GetDrawView();
    const SdrMarkList& rMarkList = pDrView->GetMarkedObjectList();
    const SdrObject* pObj = nullptr;

    if ( rMarkList.GetMarkCount() == 1 )
        pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();

    const SdrTextObj* pTextObj = DynCastSdrTextObj(pObj);
    const bool bDeactivate(
        !pObj ||
        !pTextObj ||
        !pTextObj->HasText() ||
        dynamic_cast< const SdrObjCustomShape* >(pObj)); // #121538# no FontWork for CustomShapes

    if (bDeactivate)
    {
        rSet.DisableItem(XATTR_FORMTXTSTYLE);
        rSet.DisableItem(XATTR_FORMTXTADJUST);
        rSet.DisableItem(XATTR_FORMTXTDISTANCE);
        rSet.DisableItem(XATTR_FORMTXTSTART);
        rSet.DisableItem(XATTR_FORMTXTMIRROR);
        rSet.DisableItem(XATTR_FORMTXTHIDEFORM);
        rSet.DisableItem(XATTR_FORMTXTOUTLINE);
        rSet.DisableItem(XATTR_FORMTXTSHADOW);
        rSet.DisableItem(XATTR_FORMTXTSHDWCOLOR);
        rSet.DisableItem(XATTR_FORMTXTSHDWXVAL);
        rSet.DisableItem(XATTR_FORMTXTSHDWYVAL);
    }
    else
    {
        pDrView->GetAttributes( rSet );
    }
}

void SwDrawTextShell::ExecDrawLingu(SfxRequest const &rReq)
{
    SwWrtShell &rSh = GetShell();
    OutlinerView* pOutlinerView = m_pSdrView->GetTextEditOutlinerView();
    if( !rSh.GetDrawView()->GetMarkedObjectList().GetMarkCount() )
        return;

    switch(rReq.GetSlot())
    {
    case SID_THESAURUS:
        pOutlinerView->StartThesaurus(rReq.GetFrameWeld());
        break;

    case SID_HANGUL_HANJA_CONVERSION:
        pOutlinerView->StartTextConversion(rReq.GetFrameWeld(),
                LANGUAGE_KOREAN, LANGUAGE_KOREAN, nullptr,
                i18n::TextConversionOption::CHARACTER_BY_CHARACTER, true, false);
        break;

    case SID_CHINESE_CONVERSION:
        {
            //open ChineseTranslationDialog
            Reference<XComponentContext> xContext = comphelper::getProcessComponentContext();
            if (!xContext.is())
                return;

            Reference<lang::XMultiComponentFactory> xMCF(xContext->getServiceManager());
            if (!xMCF.is())
                return;

            Reference<ui::dialogs::XExecutableDialog> xDialog(
                    xMCF->createInstanceWithContext("com.sun.star.linguistic2.ChineseTranslationDialog", xContext), UNO_QUERY);

            Reference<lang::XInitialization> xInit(xDialog, UNO_QUERY);

            if (!xInit.is())
                return;

            //  initialize dialog
            uno::Sequence<uno::Any> aSequence(comphelper::InitAnyPropertySequence(
            {
                {"ParentWindow", uno::Any(Reference<awt::XWindow>())}
            }));
            xInit->initialize( aSequence );

            //execute dialog
            sal_Int16 nDialogRet = xDialog->execute();
            if(RET_OK == nDialogRet)
            {
                //get some parameters from the dialog
                bool bToSimplified = true;
                bool bUseVariants = true;
                bool bCommonTerms = true;
                Reference<beans::XPropertySet> xPropertySet(xDialog, UNO_QUERY);
                if (xPropertySet.is())
                {
                    try
                    {
                        xPropertySet->getPropertyValue("IsDirectionToSimplified") >>= bToSimplified;
                        xPropertySet->getPropertyValue("IsUseCharacterVariants") >>= bUseVariants;
                        xPropertySet->getPropertyValue("IsTranslateCommonTerms") >>= bCommonTerms;
                    }
                    catch (const Exception&)
                    {
                    }
                }

                //execute translation
                LanguageType nSourceLang = bToSimplified ? LANGUAGE_CHINESE_TRADITIONAL : LANGUAGE_CHINESE_SIMPLIFIED;
                LanguageType nTargetLang = bToSimplified ? LANGUAGE_CHINESE_SIMPLIFIED : LANGUAGE_CHINESE_TRADITIONAL;
                sal_Int32 nOptions       = bUseVariants ? i18n::TextConversionOption::USE_CHARACTER_VARIANTS : 0;
                if(!bCommonTerms)
                    nOptions = nOptions | i18n::TextConversionOption::CHARACTER_BY_CHARACTER;

                vcl::Font aTargetFont = OutputDevice::GetDefaultFont(DefaultFontType::CJK_TEXT, nTargetLang, GetDefaultFontFlags::OnlyOne);

                pOutlinerView->StartTextConversion(rReq.GetFrameWeld(), nSourceLang, nTargetLang, &aTargetFont, nOptions, false, false);
            }

            Reference<lang::XComponent> xComponent(xDialog, UNO_QUERY);
            if (xComponent.is())
                xComponent->dispose();
        }
        break;

    default:
        OSL_ENSURE(false, "unexpected slot-id");
    }
}

void SwDrawTextShell::ExecDraw(SfxRequest &rReq)
{
    SwWrtShell &rSh = GetShell();
    m_pSdrView = rSh.GetDrawView();
    OutlinerView* pOLV = m_pSdrView->GetTextEditOutlinerView();

    switch (rReq.GetSlot())
    {
        case FN_INSERT_SOFT_HYPHEN:
        case FN_INSERT_HARDHYPHEN:
        case FN_INSERT_HARD_SPACE:
        case FN_INSERT_NNBSP:
        case SID_INSERT_RLM :
        case SID_INSERT_LRM :
        case SID_INSERT_WJ :
        case SID_INSERT_ZWSP:
        {
            sal_Unicode cIns = 0;
            switch(rReq.GetSlot())
            {
                case FN_INSERT_SOFT_HYPHEN: cIns = CHAR_SOFTHYPHEN; break;
                case FN_INSERT_HARDHYPHEN: cIns = CHAR_HARDHYPHEN; break;
                case FN_INSERT_HARD_SPACE: cIns = CHAR_HARDBLANK; break;
                case FN_INSERT_NNBSP: cIns = CHAR_NNBSP; break;
                case SID_INSERT_RLM : cIns = CHAR_RLM ; break;
                case SID_INSERT_LRM : cIns = CHAR_LRM ; break;
                case SID_INSERT_ZWSP : cIns = CHAR_ZWSP ; break;
                case SID_INSERT_WJ: cIns = CHAR_WJ; break;
            }
            pOLV->InsertText( OUString(cIns));
            rReq.Done();
        }
        break;
        case SID_CHARMAP:
        {  // Insert special character
                InsertSymbol(rReq);
                break;
        }
        case FN_INSERT_STRING:
        {
            const SfxItemSet *pNewAttrs = rReq.GetArgs();
            sal_uInt16 nSlot = rReq.GetSlot();
            const SfxPoolItem* pItem = nullptr;
            if(pNewAttrs)
            {
                pNewAttrs->GetItemState(nSlot, false, &pItem );
                pOLV->InsertText(static_cast<const SfxStringItem *>(pItem)->GetValue());
            }
            break;
        }

        case SID_SELECTALL:
        {
            SdrOutliner * pOutliner = m_pSdrView->GetTextEditOutliner();
            if(pOutliner)
            {
                sal_Int32 nParaCount = pOutliner->GetParagraphCount();
                if (nParaCount > 0)
                    pOLV->SelectRange(0, nParaCount );
            }
        }
        break;

        case FN_FORMAT_RESET:   // delete hard text attributes
        {
            pOLV->RemoveAttribsKeepLanguages( true );
            pOLV->GetEditView().getEditEngine().RemoveFields();
            rReq.Done();
        }
        break;

        case FN_ESCAPE:
            if (m_pSdrView->IsTextEdit())
            {
                // Shell switch!
                rSh.EndTextEdit();
                SwView& rTempView = rSh.GetView();
                rTempView.ExitDraw();
                rSh.Edit();
                return;
            }
            break;
        case FN_DRAWTEXT_ATTR_DLG:
            {
                SfxItemSet aNewAttr(m_pSdrView->GetModel().GetItemPool());
                m_pSdrView->GetAttributes( aNewAttr );
                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                VclPtr<SfxAbstractTabDialog> pDlg(pFact->CreateTextTabDialog(
                            GetView().GetFrameWeld(),
                            &aNewAttr, m_pSdrView ));
                auto xRequest = std::make_shared<SfxRequest>(rReq);
                rReq.Ignore(); // the 'old' request is not relevant any more
                pDlg->StartExecuteAsync(
                    [this, pDlg, xRequest=std::move(xRequest)] (sal_Int32 nResult)->void
                    {
                        if (nResult == RET_OK)
                        {
                            if (m_pSdrView->AreObjectsMarked())
                            {
                                m_pSdrView->SetAttributes(*pDlg->GetOutputItemSet());
                                xRequest->Done(*(pDlg->GetOutputItemSet()));
                            }
                        }
                        pDlg->disposeOnce();
                    }
                );
            }
            break;
        case SID_TABLE_VERT_NONE:
        case SID_TABLE_VERT_CENTER:
        case SID_TABLE_VERT_BOTTOM:
            {
                sal_uInt16 nSId = rReq.GetSlot();
                if (m_pSdrView->AreObjectsMarked())
                {
                    SdrTextVertAdjust eTVA = SDRTEXTVERTADJUST_TOP;
                    if (nSId == SID_TABLE_VERT_CENTER)
                        eTVA = SDRTEXTVERTADJUST_CENTER;
                    else if (nSId == SID_TABLE_VERT_BOTTOM)
                        eTVA = SDRTEXTVERTADJUST_BOTTOM;

                    SfxItemSet aNewAttr(m_pSdrView->GetModel().GetItemPool());
                    m_pSdrView->GetAttributes( aNewAttr );
                    aNewAttr.Put(SdrTextVertAdjustItem(eTVA));
                    m_pSdrView->SetAttributes(aNewAttr);
                    rReq.Done();
                }

            }
            break;

        default:
            OSL_ENSURE(false, "unexpected slot-id");
            return;
    }

    GetView().GetViewFrame().GetBindings().InvalidateAll(false);

    if (IsTextEdit() && pOLV->GetOutliner()->IsModified())
        rSh.SetModified();
}

// Execute undo

void SwDrawTextShell::ExecUndo(SfxRequest &rReq)
{
    if( !IsTextEdit() )
        return;

    bool bCallBase = true;
    const SfxItemSet* pArgs = rReq.GetArgs();
    if( pArgs )
    {
        sal_uInt16 nId = rReq.GetSlot(), nCnt = 1;
        const SfxPoolItem* pItem;
        switch( nId )
        {
        case SID_UNDO:
        case SID_REDO:
            if( SfxItemState::SET == pArgs->GetItemState( nId, false, &pItem ) &&
                1 < (nCnt = static_cast<const SfxUInt16Item*>(pItem)->GetValue()) )
            {
                // then we make by ourself.
                SfxUndoManager* pUndoManager = GetUndoManager();
                if( pUndoManager )
                {
                    if( SID_UNDO == nId )
                        while( nCnt-- )
                            pUndoManager->Undo();
                    else
                        while( nCnt-- )
                            pUndoManager->Redo();
                }
                bCallBase = false;
                GetView().GetViewFrame().GetBindings().InvalidateAll(false);
            }
            break;
        }
    }
    if( bCallBase )
    {
        SfxViewFrame& rSfxViewFrame = GetView().GetViewFrame();
        rSfxViewFrame.ExecuteSlot(rReq, rSfxViewFrame.GetInterface());
    }
}

// State of undo

void SwDrawTextShell::StateUndo(SfxItemSet &rSet)
{
    if ( !IsTextEdit() )
        return;

    SfxViewFrame& rSfxViewFrame = GetView().GetViewFrame();
    SfxWhichIter aIter(rSet);
    sal_uInt16 nWhich = aIter.FirstWhich();
    while( nWhich )
    {
        switch ( nWhich )
        {
        case SID_GETUNDOSTRINGS:
        case SID_GETREDOSTRINGS:
            {
                SfxUndoManager* pUndoManager = GetUndoManager();
                if( pUndoManager )
                {
                    OUString (SfxUndoManager::*fnGetComment)( size_t, bool const ) const;

                    sal_uInt16 nCount;
                    if( SID_GETUNDOSTRINGS == nWhich )
                    {
                        nCount = pUndoManager->GetUndoActionCount();
                        fnGetComment = &SfxUndoManager::GetUndoActionComment;
                    }
                    else
                    {
                        nCount = pUndoManager->GetRedoActionCount();
                        fnGetComment = &SfxUndoManager::GetRedoActionComment;
                    }
                    if( nCount )
                    {
                        OUStringBuffer sList;
                        for( sal_uInt16 n = 0; n < nCount; ++n )
                            sList.append( (pUndoManager->*fnGetComment)( n, SfxUndoManager::TopLevel ) + "\n");

                        SfxStringListItem aItem( nWhich );
                        aItem.SetString( sList.makeStringAndClear() );
                        rSet.Put( aItem );
                    }
                }
                else
                    rSet.DisableItem( nWhich );
            }
            break;

        default:
            {
                auto* pUndoManager = dynamic_cast<IDocumentUndoRedo*>(GetUndoManager());
                if (pUndoManager)
                    pUndoManager->SetView(&GetView());
                rSfxViewFrame.GetSlotState(nWhich, rSfxViewFrame.GetInterface(), &rSet);
                if (pUndoManager)
                    pUndoManager->SetView(nullptr);
            }
        }

        nWhich = aIter.NextWhich();
    }
}

void SwDrawTextShell::ExecTransliteration( SfxRequest const & rReq )
{
    if (!m_pSdrView)
        return;

    using namespace i18n;

    TransliterationFlags nMode = TransliterationFlags::NONE;

    switch( rReq.GetSlot() )
    {
    case SID_TRANSLITERATE_SENTENCE_CASE:
        nMode = TransliterationFlags::SENTENCE_CASE;
        break;
    case SID_TRANSLITERATE_TITLE_CASE:
        nMode = TransliterationFlags::TITLE_CASE;
        break;
    case SID_TRANSLITERATE_TOGGLE_CASE:
        nMode = TransliterationFlags::TOGGLE_CASE;
        break;
    case SID_TRANSLITERATE_UPPER:
        nMode = TransliterationFlags::LOWERCASE_UPPERCASE;
        break;
    case SID_TRANSLITERATE_LOWER:
        nMode = TransliterationFlags::UPPERCASE_LOWERCASE;
        break;

    case SID_TRANSLITERATE_HALFWIDTH:
        nMode = TransliterationFlags::FULLWIDTH_HALFWIDTH;
        break;
    case SID_TRANSLITERATE_FULLWIDTH:
        nMode = TransliterationFlags::HALFWIDTH_FULLWIDTH;
        break;

    case SID_TRANSLITERATE_HIRAGANA:
        nMode = TransliterationFlags::KATAKANA_HIRAGANA;
        break;
    case SID_TRANSLITERATE_KATAKANA:
        nMode = TransliterationFlags::HIRAGANA_KATAKANA;
        break;

    default:
        OSL_ENSURE(false, "wrong dispatcher");
    }

    if( nMode != TransliterationFlags::NONE )
    {
        OutlinerView* pOLV = m_pSdrView->GetTextEditOutlinerView();

        if (!pOLV)
            return;

        pOLV->TransliterateText( nMode );
    }
}

void SwDrawTextShell::ExecRotateTransliteration( SfxRequest const & rReq )
{
    if( rReq.GetSlot() == SID_TRANSLITERATE_ROTATE_CASE )
    {
        if (!m_pSdrView)
            return;

        OutlinerView* pOLV = m_pSdrView->GetTextEditOutlinerView();

        if (!pOLV)
            return;

        pOLV->TransliterateText( m_aRotateCase.getNextMode() );
    }
}

// Insert special character (see SDraw: FUBULLET.CXX)

void SwDrawTextShell::InsertSymbol(SfxRequest& rReq)
{
    OutlinerView* pOLV = m_pSdrView->GetTextEditOutlinerView();
    if(!pOLV)
        return;
    const SfxItemSet *pArgs = rReq.GetArgs();
    const SfxStringItem* pItem = nullptr;
    if( pArgs )
        pItem = pArgs->GetItemIfSet(SID_CHARMAP, false);

    OUString sSym;
    OUString sFontName;
    if ( pItem )
    {
        sSym = pItem->GetValue();
        const SfxStringItem* pFontItem = pArgs->GetItemIfSet( SID_ATTR_SPECIALCHAR, false);
        if ( pFontItem )
            sFontName = pFontItem->GetValue();
    }

    SfxItemSet aSet(pOLV->GetAttribs());
    SvtScriptType nScript = pOLV->GetSelectedScriptType();
    std::shared_ptr<SvxFontItem> aSetDlgFont(std::make_shared<SvxFontItem>(RES_CHRATR_FONT));
    {
        SvxScriptSetItem aSetItem( SID_ATTR_CHAR_FONT, *aSet.GetPool() );
        aSetItem.GetItemSet().Put( aSet, false );
        const SfxPoolItem* pI = aSetItem.GetItemOfScript( nScript );
        if( pI )
            aSetDlgFont.reset(static_cast<SvxFontItem*>(pI->Clone()));
        else
        {
            TypedWhichId<SvxFontItem> nFontWhich =
                GetWhichOfScript(
                        SID_ATTR_CHAR_FONT,
                        SvtLanguageOptions::GetI18NScriptTypeOfLanguage( GetAppLanguage() ) );
            aSetDlgFont.reset(aSet.Get( nFontWhich ).Clone());
        }
        if (sFontName.isEmpty())
            sFontName = aSetDlgFont->GetFamilyName();
    }

    vcl::Font aFont(sFontName, Size(1,1));
    if(sSym.isEmpty())
    {
        SfxAllItemSet aAllSet( GetPool() );
        aAllSet.Put( SfxBoolItem( FN_PARAM_1, false ) );

        SwViewOption aOpt(*m_rView.GetWrtShell().GetViewOptions());
        const OUString& sSymbolFont = aOpt.GetSymbolFont();
        if( !sSymbolFont.isEmpty() )
            aAllSet.Put( SfxStringItem( SID_FONT_NAME, sSymbolFont ) );
        else
            aAllSet.Put( SfxStringItem( SID_FONT_NAME, aSetDlgFont->GetFamilyName() ) );

        // If character is selected, it can be shown
        SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
        auto xFrame = m_rView.GetViewFrame().GetFrame().GetFrameInterface();
        VclPtr<SfxAbstractDialog> pDlg(pFact->CreateCharMapDialog(m_rView.GetFrameWeld(), aAllSet, xFrame));
        pDlg->StartExecuteAsync(
            [pDlg] (sal_Int32 /*nResult*/)->void
            {
                pDlg->disposeOnce();
            }
        );
        return;
    }

    // do not flicker
    pOLV->HideCursor();
    SdrOutliner * pOutliner = m_pSdrView->GetTextEditOutliner();
    pOutliner->SetUpdateLayout(false);

    SfxItemSet aOldSet( pOLV->GetAttribs() );
    SfxItemSetFixed<
            EE_CHAR_FONTINFO, EE_CHAR_FONTINFO,
            EE_CHAR_FONTINFO_CJK, EE_CHAR_FONTINFO_CTL>  aFontSet( *aOldSet.GetPool() );
    aFontSet.Set( aOldSet );

    // Insert string
    pOLV->InsertText( sSym );

    // assign attributes (Set font)
    SfxItemSet aFontAttribSet( *aFontSet.GetPool(), aFontSet.GetRanges() );
    SvxFontItem aFontItem (aFont.GetFamilyType(), aFont.GetFamilyName(),
                            aFont.GetStyleName(), aFont.GetPitch(),
                            aFont.GetCharSet(),
                            EE_CHAR_FONTINFO );
    nScript = g_pBreakIt->GetAllScriptsOfText( sSym );
    if( SvtScriptType::LATIN & nScript )
        aFontAttribSet.Put( aFontItem );
    if( SvtScriptType::ASIAN & nScript )
    {
        aFontItem.SetWhich(EE_CHAR_FONTINFO_CJK);
        aFontAttribSet.Put( aFontItem );
    }
    if( SvtScriptType::COMPLEX & nScript )
    {
        aFontItem.SetWhich(EE_CHAR_FONTINFO_CTL);
        aFontAttribSet.Put( aFontItem );
    }
    pOLV->SetAttribs(aFontAttribSet);

    // Remove selection
    ESelection aSel(pOLV->GetSelection());
    aSel.nStartPara = aSel.nEndPara;
    aSel.nStartPos = aSel.nEndPos;
    pOLV->SetSelection(aSel);

    // Restore old font
    pOLV->SetAttribs( aFontSet );

    // From now on show again
    pOutliner->SetUpdateLayout(true);
    pOLV->ShowCursor();

    rReq.AppendItem( SfxStringItem( SID_CHARMAP, sSym ) );
    if(!aFont.GetFamilyName().isEmpty())
        rReq.AppendItem( SfxStringItem( SID_ATTR_SPECIALCHAR, aFont.GetFamilyName() ) );
    rReq.Done();

}

SfxUndoManager* SwDrawTextShell::GetUndoManager()
{
    SwWrtShell &rSh = GetShell();
    m_pSdrView = rSh.GetDrawView();
    SdrOutliner * pOutliner = m_pSdrView->GetTextEditOutliner();
    return &pOutliner->GetUndoManager();
}

void SwDrawTextShell::GetStatePropPanelAttr(SfxItemSet &rSet)
{
    SfxWhichIter    aIter( rSet );
    sal_uInt16 nWhich = aIter.FirstWhich();

    SwWrtShell &rSh = GetShell();
    m_pSdrView = rSh.GetDrawView();

    SfxItemSet aAttrs(m_pSdrView->GetModel().GetItemPool());
    m_pSdrView->GetAttributes( aAttrs );

    while ( nWhich )
    {
        sal_uInt16 nSlotId = SfxItemPool::IsWhich(nWhich)
            ? GetPool().GetSlotId(nWhich)
            : nWhich;
        switch ( nSlotId )
        {
            case SID_TABLE_VERT_NONE:
            case SID_TABLE_VERT_CENTER:
            case SID_TABLE_VERT_BOTTOM:
                bool bContour = false;
                SfxItemState eConState = aAttrs.GetItemState( SDRATTR_TEXT_CONTOURFRAME );
                if( eConState != SfxItemState::INVALID )
                {
                    bContour = aAttrs.Get( SDRATTR_TEXT_CONTOURFRAME ).GetValue();
                }
                if (bContour) break;

                SfxItemState eVState = aAttrs.GetItemState( SDRATTR_TEXT_VERTADJUST );
                //SfxItemState eHState = aAttrs.GetItemState( SDRATTR_TEXT_HORZADJUST );

                //if(SfxItemState::INVALID != eVState && SfxItemState::INVALID != eHState)
                if(SfxItemState::INVALID != eVState)
                {
                    SdrTextVertAdjust eTVA = aAttrs.Get(SDRATTR_TEXT_VERTADJUST).GetValue();
                    bool bSet = (nSlotId == SID_TABLE_VERT_NONE && eTVA == SDRTEXTVERTADJUST_TOP) ||
                            (nSlotId == SID_TABLE_VERT_CENTER && eTVA == SDRTEXTVERTADJUST_CENTER) ||
                            (nSlotId == SID_TABLE_VERT_BOTTOM && eTVA == SDRTEXTVERTADJUST_BOTTOM);
                    rSet.Put(SfxBoolItem(nSlotId, bSet));
                }
                else
                {
                    rSet.Put(SfxBoolItem(nSlotId, false));
                }
                break;
        }
        nWhich = aIter.NextWhich();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
