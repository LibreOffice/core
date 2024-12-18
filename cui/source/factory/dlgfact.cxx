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

#include <config_extensions.h>
#include <config_wasm_strip.h>

#include <align.hxx>
#include "dlgfact.hxx"

#include <about.hxx>
#include <dlgname.hxx>
#include <securityoptions.hxx>
#include <AdditionsDialog.hxx>
#include <sfx2/app.hxx>
#include <sfx2/basedlgs.hxx>
#include <sfx2/pageids.hxx>
#include <svx/dialogs.hrc>
#include <svx/svxids.hrc>
#include <numfmt.hxx>
#include <splitcelldlg.hxx>
#include <cuiimapwnd.hxx>
#include <svx/cuicharmap.hxx>
#include <srchxtra.hxx>
#include <textanim.hxx>
#include <autocdlg.hxx>
#include <treeopt.hxx>
#include <labdlg.hxx>
#include <hangulhanjadlg.hxx>
#include <showcols.hxx>
#include <zoom.hxx>
#include <cuigaldlg.hxx>
#include <transfrm.hxx>
#include <bbdlg.hxx>
#include <cuisrchdlg.hxx>
#include <cuitbxform.hxx>
#include <optdict.hxx>
#include <multipat.hxx>
#include <cuihyperdlg.hxx>
#include <cuifmsearch.hxx>
#include <cuigrfflt.hxx>
#include <cuitabarea.hxx>
#include <cuitabline.hxx>
#include <measure.hxx>
#include <connect.hxx>
#include <dbregister.hxx>
#include <cuioptgenrl.hxx>
#include <optasian.hxx>
#include <insdlg.hxx>
#include <pastedlg.hxx>
#include <linkdlg.hxx>
#include <SignatureLineDialog.hxx>
#include <SignSignatureLineDialog.hxx>
#include <QrCodeGenDialog.hxx>
#include <SpellDialog.hxx>
#include <cfg.hxx>
#include <numpages.hxx>
#include <paragrph.hxx>
#include <tabstpge.hxx>
#include <textattr.hxx>
#include <backgrnd.hxx>
#include <border.hxx>
#include <chardlg.hxx>
#include <page.hxx>
#include <postdlg.hxx>
#include <grfpage.hxx>
#include <scriptdlg.hxx>
#include <cfgutil.hxx>
#include <macropg.hxx>
#include <sdrcelldlg.hxx>
#include <newtabledlg.hxx>
#include <macroass.hxx>
#include <insrc.hxx>
#include <passwdomdlg.hxx>
#include <screenshotannotationdlg.hxx>
#include <hyphen.hxx>
#include <thesdlg.hxx>
#include <tipofthedaydlg.hxx>
#include <widgettestdlg.hxx>
#include <toolbarmodedlg.hxx>
#include <DiagramDialog.hxx>
#include <fileextcheckdlg.hxx>
#include <TextColumnsPage.hxx>

#include <MacroManagerDialog.hxx>

using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::container;

using ::com::sun::star::uno::Reference;

using namespace svx;

namespace
{
// Templated base class storing the dialog pointer, and implementing Execute, StartExecuteAsync,
// and the shared constructor used by derived classes
template <class Base, class Dialog, template <class...> class Ptr, bool Async>
    requires std::is_base_of_v<VclAbstractDialog, Base>
             && std::is_base_of_v<weld::DialogController, Dialog>
class ControllerImpl_BASE : public Base
{
public:
    explicit ControllerImpl_BASE(Ptr<Dialog> p)
        : m_pDlg(std::move(p))
    {
    }

    short Execute() override { return m_pDlg->run(); }

    bool StartExecuteAsync(VclAbstractDialog::AsyncContext& rCtx) override
    {
        if constexpr (Async)
            return Dialog::runAsync(this->m_pDlg, rCtx.maEndDialogFn);
        else
            return Base::StartExecuteAsync(rCtx); // assert / fail
    }

protected:
    Ptr<Dialog> m_pDlg;
};

// Base for synchronously called dialogs, using unique_ptr
template <class Base, class Dialog>
using ControllerImpl_Sync = ControllerImpl_BASE<Base, Dialog, std::unique_ptr, false>;

// Base for synchronously called dialogs, using shared_ptr
template <class Base, class Dialog>
using ControllerImpl_Sync_Shared = ControllerImpl_BASE<Base, Dialog, std::shared_ptr, false>;

// Base for asynchronously called dialogs
template <class Base, class Dialog>
using ControllerImpl_Async = ControllerImpl_BASE<Base, Dialog, std::shared_ptr, true>;

// Dialogs with simplest interface

using CuiAbstractController_Impl = ControllerImpl_Sync<VclAbstractDialog, weld::DialogController>;

using CuiAbstractControllerAsync_Impl
    = ControllerImpl_Async<VclAbstractDialog, weld::DialogController>;
}

// Create dialogs with simplest interface
VclPtr<VclAbstractDialog> AbstractDialogFactory_Impl::CreateVclDialog(weld::Window* pParent, sal_uInt32 nResId)
{
    switch ( nResId )
    {
        case SID_OPTIONS_DATABASES :
        {
            Reference< frame::XFrame > xFrame;
            auto xDlg = std::make_shared<OfaTreeOptionsDialog>(pParent, xFrame, /*bActivateLastSelection*/false);
            xDlg->ActivatePage(SID_SB_DBREGISTEROPTIONS);
            return VclPtr<CuiAbstractControllerAsync_Impl>::Create(std::move(xDlg));
        }
        case SID_OPTIONS_TREEDIALOG :
        case SID_LANGUAGE_OPTIONS :
        {
            bool bActivateLastSelection = false;
            if (nResId == SID_OPTIONS_TREEDIALOG)
                bActivateLastSelection = true;
            Reference< frame::XFrame > xFrame;
            auto xDlg = std::make_unique<OfaTreeOptionsDialog>(pParent, xFrame, bActivateLastSelection);
            if (nResId == SID_LANGUAGE_OPTIONS)
            {
                //open the tab page "tools/options/languages"
                xDlg->ActivatePage(OFA_TP_LANGUAGES_FOR_SET_DOCUMENT_LANGUAGE);
            }
            return VclPtr<CuiAbstractController_Impl>::Create(std::move(xDlg));
        }
        break;

        default:
            assert(false);
            return nullptr;
    }
}

VclPtr<VclAbstractDialog> AbstractDialogFactory_Impl::CreateFrameDialog(weld::Window* pParent, const Reference< frame::XFrame >& rxFrame,
    sal_uInt32 nResId, sal_uInt16 nPageId, const OUString& rParameter)
{
    std::unique_ptr<OfaTreeOptionsDialog> xDlg;
    if (SID_OPTIONS_TREEDIALOG == nResId || SID_OPTIONS_DATABASES == nResId)
    {
        // only activate last page if we don't want to activate a special page
        bool bActivateLastSelection = ( nResId != SID_OPTIONS_DATABASES && rParameter.isEmpty() && !nPageId);
        xDlg = std::make_unique<OfaTreeOptionsDialog>(pParent, rxFrame, bActivateLastSelection);
        if ( nResId == SID_OPTIONS_DATABASES )
            xDlg->ActivatePage(SID_SB_DBREGISTEROPTIONS);
        else if (nPageId)
            xDlg->ActivatePage(nPageId);
        else if ( !rParameter.isEmpty() )
            xDlg->ActivatePage( rParameter );
    }

    if (xDlg)
        return VclPtr<CuiAbstractController_Impl>::Create(std::move(xDlg));
    return nullptr;
}

namespace
{
class AbstractSecurityOptionsDialog_Impl final
    : public ControllerImpl_Sync<AbstractSecurityOptionsDialog, SecurityOptionsDialog>
{
public:
    using ControllerImpl_BASE::ControllerImpl_BASE; // inherited ctor
    bool SetSecurityOptions() override { return m_pDlg->SetSecurityOptions(); }
};
}

VclPtr<AbstractSecurityOptionsDialog> AbstractDialogFactory_Impl::CreateSvxSecurityOptionsDialog(weld::Window* pParent)
{
    return VclPtr<AbstractSecurityOptionsDialog_Impl>::Create(std::make_unique<svx::SecurityOptionsDialog>(pParent));
}

namespace
{
class CuiAbstractTabController_Impl final
    : public ControllerImpl_Async<SfxAbstractTabDialog, SfxTabDialogController>
{
public:
    using ControllerImpl_BASE::ControllerImpl_BASE; // inherited ctor
    void SetCurPageId(const OUString& rName) override { m_pDlg->SetCurPageId(rName); }
    const SfxItemSet* GetOutputItemSet() const override { return m_pDlg->GetOutputItemSet(); }
    WhichRangesContainer GetInputRanges(const SfxItemPool& pItem) override
    {
        return m_pDlg->GetInputRanges(pItem);
    }
    void SetInputSet(const SfxItemSet* pInSet) override { m_pDlg->SetInputSet(pInSet); }

    // From class Window.
    void SetText(const OUString& rStr) override { m_pDlg->set_title(rStr); }

    // screenshotting
    std::vector<OUString> getAllPageUIXMLDescriptions() const override
    {
        return m_pDlg->getAllPageUIXMLDescriptions();
    }
    bool selectPageByUIXMLDescription(const OUString& rUIXMLDescription) override
    {
        return m_pDlg->selectPageByUIXMLDescription(rUIXMLDescription);
    }
    BitmapEx createScreenshot() const override { return m_pDlg->createScreenshot(); }
    OUString GetScreenshotId() const override { return m_pDlg->GetScreenshotId(); }
};
}

// TabDialog outside the drawing layer
VclPtr<SfxAbstractTabDialog> AbstractDialogFactory_Impl::CreateAutoCorrTabDialog(weld::Window* pParent, const SfxItemSet* pAttrSet)
{
    return VclPtr<CuiAbstractTabController_Impl>::Create(std::make_shared<OfaAutoCorrDlg>(pParent, pAttrSet));
}

VclPtr<SfxAbstractTabDialog> AbstractDialogFactory_Impl::CreateCustomizeTabDialog(weld::Window* pParent,
                                                const SfxItemSet* pAttrSet,
                                                const Reference< frame::XFrame >& xViewFrame )
{
    auto xDlg1 = std::make_shared<SvxConfigDialog>(pParent, pAttrSet);
    xDlg1->SetFrame(xViewFrame);
    return VclPtr<CuiAbstractTabController_Impl>::Create(std::move(xDlg1));
}

// TabDialog that use functionality of the drawing layer
VclPtr<SfxAbstractTabDialog> AbstractDialogFactory_Impl::CreateTextTabDialog(weld::Window* pParent,
                                            const SfxItemSet* pAttrSet,
                                            SdrView* pView)
{
    return VclPtr<CuiAbstractTabController_Impl>::Create(std::make_shared<SvxTextTabDialog>(pParent, pAttrSet, pView));
}

namespace
{
// AbstractSvxCaptionDialog implementations just forwards everything to the dialog
class AbstractSvxCaptionDialog_Impl final
    : public ControllerImpl_Async<AbstractSvxCaptionDialog, SvxCaptionTabDialog>
{
public:
    using ControllerImpl_BASE::ControllerImpl_BASE; // inherited ctor
    void SetValidateFramePosLink(const Link<SvxSwFrameValidation&, void>& rLink) override
    {
        m_pDlg->SetValidateFramePosLink(rLink);
    }
    void SetCurPageId(const OUString& rName) override { m_pDlg->SetCurPageId(rName); }
    const SfxItemSet* GetOutputItemSet() const override { return m_pDlg->GetOutputItemSet(); }
    WhichRangesContainer GetInputRanges(const SfxItemPool& pItem) override
    {
        return m_pDlg->GetInputRanges(pItem);
    }
    void SetInputSet(const SfxItemSet* pInSet) override { m_pDlg->SetInputSet(pInSet); }
    void SetText(const OUString& rStr) override { m_pDlg->set_title(rStr); }
};
}

// TabDialog that use functionality of the drawing layer and add AnchorTypes -- for SvxCaptionTabDialog
VclPtr<AbstractSvxCaptionDialog> AbstractDialogFactory_Impl::CreateCaptionDialog(weld::Window* pParent,
                                                                                 const SdrView* pView,
                                                                                 SvxAnchorIds nAnchorTypes)
{
    return VclPtr<AbstractSvxCaptionDialog_Impl>::Create(std::make_shared<SvxCaptionTabDialog>(pParent, pView, nAnchorTypes));
}

namespace
{
class AbstractHangulHanjaConversionDialog_Impl final
    : public ControllerImpl_Sync<AbstractHangulHanjaConversionDialog, HangulHanjaConversionDialog>
{
public:
    using ControllerImpl_BASE::ControllerImpl_BASE; // inherited ctor
    void EndDialog(sal_Int32 nResult) override { m_pDlg->response(nResult); }
    void EnableRubySupport(bool bVal) override { m_pDlg->EnableRubySupport(bVal); }
    void SetByCharacter(bool bByCharacter) override { m_pDlg->SetByCharacter(bByCharacter); }
    void SetConversionDirectionState(
        bool bTryBothDirections,
        editeng::HangulHanjaConversion::ConversionDirection ePrimaryConversionDirection) override
    {
        m_pDlg->SetConversionDirectionState(bTryBothDirections, ePrimaryConversionDirection);
    }
    void SetConversionFormat(editeng::HangulHanjaConversion::ConversionFormat eType) override
    {
        m_pDlg->SetConversionFormat(eType);
    }
    void SetOptionsChangedHdl(const Link<LinkParamNone*, void>& rHdl) override
    {
        m_pDlg->SetOptionsChangedHdl(rHdl);
    }
    void SetIgnoreHdl(const Link<weld::Button&, void>& rHdl) override
    {
        m_pDlg->SetIgnoreHdl(rHdl);
    }
    void SetIgnoreAllHdl(const Link<weld::Button&, void>& rHdl) override
    {
        m_pDlg->SetIgnoreAllHdl(rHdl);
    }
    void SetChangeHdl(const Link<weld::Button&, void>& rHdl) override
    {
        m_pDlg->SetChangeHdl(rHdl);
    }
    void SetChangeAllHdl(const Link<weld::Button&, void>& rHdl) override
    {
        m_pDlg->SetChangeAllHdl(rHdl);
    }
    void SetClickByCharacterHdl(const Link<weld::Toggleable&, void>& rHdl) override
    {
        m_pDlg->SetClickByCharacterHdl(rHdl);
    };
    void SetConversionFormatChangedHdl(const Link<weld::Toggleable&, void>& rHdl) override
    {
        m_pDlg->SetConversionFormatChangedHdl(rHdl);
    };
    void SetFindHdl(const Link<weld::Button&, void>& _rHdl) override { m_pDlg->SetFindHdl(_rHdl); }
    bool GetUseBothDirections() const override { return m_pDlg->GetUseBothDirections(); }
    editeng::HangulHanjaConversion::ConversionDirection GetDirection(
        editeng::HangulHanjaConversion::ConversionDirection eDefaultDirection) const override
    {
        return m_pDlg->GetDirection(eDefaultDirection);
    }
    void SetCurrentString(const OUString& rNewString,
                          const css::uno::Sequence<OUString>& rSuggestions,
                          bool bOriginatesFromDocument = true) override
    {
        m_pDlg->SetCurrentString(rNewString, rSuggestions, bOriginatesFromDocument);
    }
    OUString GetCurrentString() const override { return m_pDlg->GetCurrentString(); }
    editeng::HangulHanjaConversion::ConversionFormat GetConversionFormat() const override
    {
        return m_pDlg->GetConversionFormat();
    }
    void FocusSuggestion() override { m_pDlg->FocusSuggestion(); }
    OUString GetCurrentSuggestion() const override { return m_pDlg->GetCurrentSuggestion(); }
};
}

VclPtr<AbstractHangulHanjaConversionDialog> AbstractDialogFactory_Impl::CreateHangulHanjaConversionDialog(weld::Widget* pParent)
{
    return VclPtr<AbstractHangulHanjaConversionDialog_Impl>::Create(std::make_unique<HangulHanjaConversionDialog>(pParent));
}

namespace
{
class AbstractThesaurusDialog_Impl final
    : public ControllerImpl_Async<AbstractThesaurusDialog, SvxThesaurusDialog>
{
public:
    using ControllerImpl_BASE::ControllerImpl_BASE; // inherited ctor
    OUString GetWord() override { return m_pDlg->GetWord(); }
};
}

VclPtr<AbstractThesaurusDialog> AbstractDialogFactory_Impl::CreateThesaurusDialog(weld::Widget* pParent,
                                css::uno::Reference<css::linguistic2::XThesaurus> xThesaurus,
                                const OUString &rWord, LanguageType nLanguage)
{
    return VclPtr<AbstractThesaurusDialog_Impl>::Create(std::make_shared<SvxThesaurusDialog>(pParent, xThesaurus, rWord, nLanguage));
}

#if !ENABLE_WASM_STRIP_EXTRA
namespace
{
class AbstractHyphenWordDialog_Impl final
    : public ControllerImpl_Sync<AbstractHyphenWordDialog, SvxHyphenWordDialog>
{
public:
    using ControllerImpl_BASE::ControllerImpl_BASE; // inherited ctor
#if ENABLE_WASM_STRIP_HUNSPELL
    short Execute() override { return 0; }
#endif
};
}
#endif

VclPtr<AbstractHyphenWordDialog> AbstractDialogFactory_Impl::CreateHyphenWordDialog(weld::Widget* pParent,
                                                const OUString &rWord, LanguageType nLang,
                                                css::uno::Reference< css::linguistic2::XHyphenator >  &xHyphen,
                                                SvxSpellWrapper* pWrapper)
{
#if !ENABLE_WASM_STRIP_EXTRA
    return VclPtr<AbstractHyphenWordDialog_Impl>::Create(std::make_unique<SvxHyphenWordDialog>(rWord, nLang, pParent, xHyphen, pWrapper));
#else
    (void) pParent;
    (void) rWord;
    (void) nLang;
    (void) xHyphen;
    (void) pWrapper;
    return nullptr;
#endif
}

namespace
{
class AbstractFmShowColsDialog_Impl final
    : public ControllerImpl_Sync<AbstractFmShowColsDialog, FmShowColsDialog>
{
public:
    using ControllerImpl_BASE::ControllerImpl_BASE; // inherited ctor
    void SetColumns(const Reference<XIndexContainer>& xCols) override { m_pDlg->SetColumns(xCols); }
};
}

VclPtr<AbstractFmShowColsDialog> AbstractDialogFactory_Impl::CreateFmShowColsDialog(weld::Window* pParent)
{
    return VclPtr<AbstractFmShowColsDialog_Impl>::Create(std::make_unique<FmShowColsDialog>(pParent));
}

namespace
{
class AbstractSvxZoomDialog_Impl final
    : public ControllerImpl_Sync<AbstractSvxZoomDialog, SvxZoomDialog>
{
public:
    using ControllerImpl_BASE::ControllerImpl_BASE; // inherited ctor
    void SetLimits(sal_uInt16 nMin, sal_uInt16 nMax) override { m_pDlg->SetLimits(nMin, nMax); }
    void HideButton(ZoomButtonId nBtnId) override { m_pDlg->HideButton(nBtnId); }
    const SfxItemSet* GetOutputItemSet() const override { return m_pDlg->GetOutputItemSet(); }
};
}

VclPtr<AbstractSvxZoomDialog> AbstractDialogFactory_Impl::CreateSvxZoomDialog(weld::Window* pParent, const SfxItemSet& rCoreSet)
{
    return VclPtr<AbstractSvxZoomDialog_Impl>::Create(std::make_unique<SvxZoomDialog>(pParent, rCoreSet));
}

namespace
{
class AbstractSpellDialog_Impl final : public ControllerImpl_Async<AbstractSpellDialog, SpellDialog>
{
public:
    using ControllerImpl_BASE::ControllerImpl_BASE; // inherited ctor
    void InvalidateDialog() override { m_pDlg->InvalidateDialog(); }
    std::shared_ptr<SfxDialogController> GetController() override { return m_pDlg; }
    SfxBindings& GetBindings() override { return m_pDlg->GetBindings(); }
};
}

VclPtr<AbstractSpellDialog> AbstractDialogFactory_Impl::CreateSvxSpellDialog(
                        weld::Window* pParent,
                        SfxBindings* pBindings,
                        svx::SpellDialogChildWindow* pSpellChildWindow)
{
    return VclPtr<AbstractSpellDialog_Impl>::Create(std::make_shared<svx::SpellDialog>(pSpellChildWindow, pParent, pBindings));
}

VclPtr<VclAbstractDialog> AbstractDialogFactory_Impl::CreateActualizeProgressDialog(weld::Widget* pParent,
                                                                                               GalleryTheme* pThm)
{
   return VclPtr<CuiAbstractController_Impl>::Create(std::make_unique<ActualizeProgress>(pParent, pThm));
}

void AbstractDialogFactory_Impl::ShowAsyncScriptErrorDialog(weld::Window* pParent, const css::uno::Any& rException)
{
    return SvxScriptErrorDialog::ShowAsyncErrorDialog(pParent, rException);
}

namespace
{
class AbstractScriptSelectorDialog_Impl final
    : public ControllerImpl_Async<AbstractScriptSelectorDialog, SvxScriptSelectorDialog>
{
public:
    using ControllerImpl_BASE::ControllerImpl_BASE; // inherited ctor
    OUString GetScriptURL() const override { return m_pDlg->GetScriptURL(); }
    void SetRunLabel() override { m_pDlg->SetRunLabel(); }
};
}

VclPtr<AbstractScriptSelectorDialog> AbstractDialogFactory_Impl::CreateScriptSelectorDialog(weld::Window* pParent,
        const Reference<frame::XFrame>& rxFrame)
{
    return VclPtr<AbstractScriptSelectorDialog_Impl>::Create(std::make_shared<SvxScriptSelectorDialog>(pParent, rxFrame));
}

#if HAVE_FEATURE_SCRIPTING
namespace
{
class AbstractMacroManagerDialog_Impl final
    : public ControllerImpl_Async<AbstractMacroManagerDialog, MacroManagerDialog>
{
public:
    using ControllerImpl_BASE::ControllerImpl_BASE; // inherited ctor
    OUString GetScriptURL() const override { return m_pDlg->GetScriptURL(); }
    void LoadLastUsedMacro() const override { m_pDlg->LoadLastUsedMacro(); }
};
}
#endif

VclPtr<AbstractMacroManagerDialog>
AbstractDialogFactory_Impl::CreateMacroManagerDialog(weld::Window* pParent,
                                                     const Reference<frame::XFrame>& rxFrame)
{
#if HAVE_FEATURE_SCRIPTING
    return VclPtr<AbstractMacroManagerDialog_Impl>::Create(
        std::make_shared<MacroManagerDialog>(pParent, rxFrame));
#else
    (void)pParent;
    (void)rxFrame;
    return nullptr;
#endif
}

VclPtr<VclAbstractDialog> AbstractDialogFactory_Impl::CreateSvxScriptOrgDialog(weld::Window* pParent,
                                            const OUString& rLanguage)
{
    return VclPtr<CuiAbstractController_Impl>::Create(std::make_unique<SvxScriptOrgDialog>(pParent, rLanguage));
}

namespace
{
class AbstractTitleDialog_Impl final : public ControllerImpl_Sync<AbstractTitleDialog, TitleDialog>
{
public:
    using ControllerImpl_BASE::ControllerImpl_BASE; // inherited ctor
    OUString GetTitle() const override { return m_pDlg->GetTitle(); }
};
}

VclPtr<AbstractTitleDialog> AbstractDialogFactory_Impl::CreateTitleDialog(weld::Widget* pParent,
                                            const OUString& rOldText)
{
   return VclPtr<AbstractTitleDialog_Impl>::Create(std::make_unique<TitleDialog>(pParent, rOldText));
}

namespace
{
class AbstractGalleryIdDialog_Impl final
    : public ControllerImpl_Sync<AbstractGalleryIdDialog, GalleryIdDialog>
{
public:
    using ControllerImpl_BASE::ControllerImpl_BASE; // inherited ctor
    sal_uInt32 GetId() const override { return m_pDlg->GetId(); }
};
}

VclPtr<AbstractGalleryIdDialog> AbstractDialogFactory_Impl::CreateGalleryIdDialog(weld::Widget* pParent,
                                            GalleryTheme* pThm)
{
   return VclPtr<AbstractGalleryIdDialog_Impl>::Create(std::make_unique<GalleryIdDialog>(pParent, pThm));
}

VclPtr<VclAbstractDialog> AbstractDialogFactory_Impl::CreateGalleryThemePropertiesDialog(weld::Widget* pParent,
                                            ExchangeData* pData,
                                            SfxItemSet* pItemSet)
{
    return VclPtr<CuiAbstractTabController_Impl>::Create(std::make_shared<GalleryThemeProperties>(
                                                         pParent, pData, pItemSet));
}

namespace
{
class AbstractURLDlg_Impl final : public ControllerImpl_Sync<AbstractURLDlg, URLDlg>
{
public:
    using ControllerImpl_BASE::ControllerImpl_BASE; // inherited ctor
    OUString GetURL() const override { return m_pDlg->GetURL(); }
    OUString GetAltText() const override { return m_pDlg->GetAltText(); }
    OUString GetDesc() const override { return m_pDlg->GetDesc(); }
    OUString GetTarget() const override { return m_pDlg->GetTarget(); }
    OUString GetName() const override { return m_pDlg->GetName(); }
};
}

VclPtr<AbstractURLDlg> AbstractDialogFactory_Impl::CreateURLDialog(weld::Widget* pParent,
                                            const OUString& rURL, const OUString& rAltText, const OUString& rDescription,
                                            const OUString& rTarget, const OUString& rName,
                                            TargetList& rTargetList )
{
    return VclPtr<AbstractURLDlg_Impl>::Create(std::make_unique<URLDlg>(pParent, rURL, rAltText, rDescription,
                                                                        rTarget, rName, rTargetList));

}

VclPtr<SfxAbstractTabDialog> AbstractDialogFactory_Impl::CreateTabItemDialog(weld::Window* pParent,
    const SfxItemSet& rSet)
{
    return VclPtr<CuiAbstractTabController_Impl>::Create(std::make_shared<SvxSearchFormatDialog>(
                                                         pParent, rSet));
}

VclPtr<VclAbstractDialog> AbstractDialogFactory_Impl::CreateSvxSearchAttributeDialog(weld::Window* pParent,
                                            SearchAttrItemList& rLst,
                                            const WhichRangesContainer& pWhRanges )
{
    return VclPtr<CuiAbstractController_Impl>::Create(std::make_unique<SvxSearchAttributeDialog>(pParent, rLst, pWhRanges));
}

namespace
{
class AbstractSvxSearchSimilarityDialog_Impl final
    : public ControllerImpl_Async<AbstractSvxSearchSimilarityDialog, SvxSearchSimilarityDialog>
{
public:
    using ControllerImpl_BASE::ControllerImpl_BASE; // inherited ctor
    sal_uInt16 GetOther() override { return m_pDlg->GetOther(); }
    sal_uInt16 GetShorter() override { return m_pDlg->GetShorter(); }
    sal_uInt16 GetLonger() override { return m_pDlg->GetLonger(); }
    bool IsRelaxed() override { return m_pDlg->IsRelaxed(); }
};
}

VclPtr<AbstractSvxSearchSimilarityDialog> AbstractDialogFactory_Impl::CreateSvxSearchSimilarityDialog(weld::Window* pParent,
                                                            bool bRelax,
                                                            sal_uInt16 nOther,
                                                            sal_uInt16 nShorter,
                                                            sal_uInt16 nLonger)
{
    return VclPtr<AbstractSvxSearchSimilarityDialog_Impl>::Create(std::make_unique<SvxSearchSimilarityDialog>(pParent, bRelax, nOther, nShorter, nLonger));
}

VclPtr<SfxAbstractTabDialog> AbstractDialogFactory_Impl::CreateSvxBorderBackgroundDlg(
    weld::Window* pParent,
    const SfxItemSet& rCoreSet,
    bool bEnableDrawingLayerFillStyles)
{
    return VclPtr<CuiAbstractTabController_Impl>::Create(std::make_shared<SvxBorderBackgroundDlg>(
        pParent,
        rCoreSet,
        /*bEnableSelector*/true,
        bEnableDrawingLayerFillStyles));
}

namespace
{
// AbstractSvxTransformTabDialog implementations just forwards everything to the dialog
class AbstractSvxTransformTabDialog_Impl final
    : public ControllerImpl_Async<AbstractSvxTransformTabDialog, SvxTransformTabDialog>
{
public:
    using ControllerImpl_BASE::ControllerImpl_BASE; // inherited ctor
    void SetValidateFramePosLink(const Link<SvxSwFrameValidation&, void>& rLink) override
    {
        m_pDlg->SetValidateFramePosLink(rLink);
    }
    void SetCurPageId(const OUString& rName) override { m_pDlg->SetCurPageId(rName); }
    const SfxItemSet* GetOutputItemSet() const override { return m_pDlg->GetOutputItemSet(); }
    WhichRangesContainer GetInputRanges(const SfxItemPool& pItem) override
    {
        return m_pDlg->GetInputRanges(pItem);
    }
    void SetInputSet(const SfxItemSet* pInSet) override { m_pDlg->SetInputSet(pInSet); }
    // From class Window.
    void SetText(const OUString& rStr) override { m_pDlg->set_title(rStr); }
};
}

VclPtr<AbstractSvxTransformTabDialog> AbstractDialogFactory_Impl::CreateSvxTransformTabDialog(weld::Window* pParent,
                                                                                              const SfxItemSet* pAttr,
                                                                                              const SdrView* pView,
                                                                                              SvxAnchorIds nAnchorTypes)
{
    return VclPtr<AbstractSvxTransformTabDialog_Impl>::Create(std::make_shared<SvxTransformTabDialog>(pParent, pAttr,pView, nAnchorTypes));
}

VclPtr<SfxAbstractTabDialog> AbstractDialogFactory_Impl::CreateSchTransformTabDialog(weld::Window* pParent,
                                                                                     const SfxItemSet* pAttr,
                                                                                     const SdrView* pSdrView,
                                                                                     bool bSizeTabPage)
{
    auto pDlg = std::make_shared<SvxTransformTabDialog>(pParent, pAttr, pSdrView,
            bSizeTabPage ? SvxAnchorIds::NoProtect :  SvxAnchorIds::NoProtect|SvxAnchorIds::NoResize);
    pDlg->RemoveTabPage( u"RID_SVXPAGE_ANGLE"_ustr );
    pDlg->RemoveTabPage( u"RID_SVXPAGE_SLANT"_ustr );
    return VclPtr<CuiAbstractTabController_Impl>::Create(std::move(pDlg));
}

namespace
{
class AbstractSvxJSearchOptionsDialog_Impl final
    : public ControllerImpl_Sync<AbstractSvxJSearchOptionsDialog, SvxJSearchOptionsDialog>
{
public:
    using ControllerImpl_BASE::ControllerImpl_BASE; // inherited ctor
    TransliterationFlags GetTransliterationFlags() const override
    {
        return m_pDlg->GetTransliterationFlags();
    }
};
}

VclPtr<AbstractSvxJSearchOptionsDialog> AbstractDialogFactory_Impl::CreateSvxJSearchOptionsDialog(weld::Window* pParent,
                                                            const SfxItemSet& rOptionsSet,
                                                            TransliterationFlags nInitialFlags)
{
    return VclPtr<AbstractSvxJSearchOptionsDialog_Impl>::Create(std::make_unique<SvxJSearchOptionsDialog>(pParent, rOptionsSet, nInitialFlags));
}

namespace
{
class AbstractFmInputRecordNoDialog_Impl final
    : public ControllerImpl_Sync<AbstractFmInputRecordNoDialog, FmInputRecordNoDialog>
{
public:
    using ControllerImpl_BASE::ControllerImpl_BASE; // inherited ctor
    void SetValue(tools::Long nNew) override { m_pDlg->SetValue(nNew); }
    tools::Long GetValue() const override { return m_pDlg->GetValue(); }
};
}

VclPtr<AbstractFmInputRecordNoDialog> AbstractDialogFactory_Impl::CreateFmInputRecordNoDialog(weld::Window* pParent)
{
    return VclPtr<AbstractFmInputRecordNoDialog_Impl>::Create(std::make_unique<FmInputRecordNoDialog>(pParent));
}

namespace
{
class AbstractSvxNewDictionaryDialog_Impl final
    : public ControllerImpl_Sync<AbstractSvxNewDictionaryDialog, SvxNewDictionaryDialog>
{
public:
    using ControllerImpl_BASE::ControllerImpl_BASE; // inherited ctor
    Reference<css::linguistic2::XDictionary> GetNewDictionary() override
    {
        return m_pDlg->GetNewDictionary();
    }
};
}

VclPtr<AbstractSvxNewDictionaryDialog> AbstractDialogFactory_Impl::CreateSvxNewDictionaryDialog(weld::Window* pParent)
{
    return VclPtr<AbstractSvxNewDictionaryDialog_Impl>::Create(std::make_unique<SvxNewDictionaryDialog>(pParent));
}

VclPtr<VclAbstractDialog> AbstractDialogFactory_Impl::CreateSvxEditDictionaryDialog(weld::Window* pParent, const OUString& rName)
{
    return VclPtr<CuiAbstractController_Impl>::Create(std::make_unique<SvxEditDictionaryDialog>(pParent, rName));
}

namespace
{
class AbstractSvxNameDialog_Impl final
    : public ControllerImpl_Sync<AbstractSvxNameDialog, SvxNameDialog>
{
public:
    using ControllerImpl_BASE::ControllerImpl_BASE; // inherited ctor
    OUString GetName() override { return m_pDlg->GetName(); }
    void SetCheckNameHdl(const Link<AbstractSvxNameDialog&, bool>& rLink) override;
    void SetCheckNameTooltipHdl(const Link<AbstractSvxNameDialog&, OUString>& rLink) override;
    void SetEditHelpId(const OUString& rHelpId) override { m_pDlg->SetEditHelpId(rHelpId); }
    //from class Window
    void SetHelpId(const OUString& rHelpId) override { m_pDlg->set_help_id(rHelpId); }
    void SetText(const OUString& rStr) override { m_pDlg->set_title(rStr); }

private:
    Link<AbstractSvxNameDialog&, bool> aCheckNameHdl;
    Link<AbstractSvxNameDialog&, OUString> aCheckNameTooltipHdl;
    DECL_LINK(CheckNameHdl, SvxNameDialog&, bool);
    DECL_LINK(CheckNameTooltipHdl, SvxNameDialog&, OUString);
};
}

void AbstractSvxNameDialog_Impl::SetCheckNameHdl(const Link<AbstractSvxNameDialog&, bool>& rLink)
{
    aCheckNameHdl = rLink;
    if (rLink.IsSet())
        m_pDlg->SetCheckNameHdl(LINK(this, AbstractSvxNameDialog_Impl, CheckNameHdl));
    else
        m_pDlg->SetCheckNameHdl(Link<SvxNameDialog&, bool>());
}

void AbstractSvxNameDialog_Impl::SetCheckNameTooltipHdl(
    const Link<AbstractSvxNameDialog&, OUString>& rLink)
{
    aCheckNameTooltipHdl = rLink;
    if (rLink.IsSet())
        m_pDlg->SetCheckNameTooltipHdl(LINK(this, AbstractSvxNameDialog_Impl, CheckNameTooltipHdl));
    else
        m_pDlg->SetCheckNameTooltipHdl(Link<SvxNameDialog&, OUString>());
}

IMPL_LINK_NOARG(AbstractSvxNameDialog_Impl, CheckNameHdl, SvxNameDialog&, bool)
{
    return aCheckNameHdl.Call(*this);
}

IMPL_LINK_NOARG(AbstractSvxNameDialog_Impl, CheckNameTooltipHdl, SvxNameDialog&, OUString)
{
    return aCheckNameTooltipHdl.Call(*this);
}

VclPtr<AbstractSvxNameDialog> AbstractDialogFactory_Impl::CreateSvxNameDialog(weld::Window* pParent,
                                    const OUString& rName, const OUString& rDesc, const OUString& rTitle)
{
    return VclPtr<AbstractSvxNameDialog_Impl>::Create(std::make_unique<SvxNameDialog>(pParent, rName, rDesc, rTitle));
}

namespace
{
class AbstractSvxObjectNameDialog_Impl final
    : public ControllerImpl_Async<AbstractSvxObjectNameDialog, SvxObjectNameDialog>
{
public:
    using ControllerImpl_BASE::ControllerImpl_BASE; // inherited ctor
    OUString GetName() override { return m_pDlg->GetName(); }
    void SetCheckNameHdl(const Link<AbstractSvxObjectNameDialog&, bool>& rLink) override;

private:
    Link<AbstractSvxObjectNameDialog&, bool> aCheckNameHdl;
    DECL_LINK(CheckNameHdl, SvxObjectNameDialog&, bool);
};
}

void AbstractSvxObjectNameDialog_Impl::SetCheckNameHdl(
    const Link<AbstractSvxObjectNameDialog&, bool>& rLink)
{
    aCheckNameHdl = rLink;

    if (rLink.IsSet())
    {
        m_pDlg->SetCheckNameHdl(LINK(this, AbstractSvxObjectNameDialog_Impl, CheckNameHdl));
    }
    else
    {
        m_pDlg->SetCheckNameHdl(Link<SvxObjectNameDialog&, bool>());
    }
}

IMPL_LINK_NOARG(AbstractSvxObjectNameDialog_Impl, CheckNameHdl, SvxObjectNameDialog&, bool)
{
    return aCheckNameHdl.Call(*this);
}

VclPtr<AbstractSvxObjectNameDialog> AbstractDialogFactory_Impl::CreateSvxObjectNameDialog(weld::Window* pParent, const OUString& rName)
{
    return VclPtr<AbstractSvxObjectNameDialog_Impl>::Create(std::make_unique<SvxObjectNameDialog>(pParent, rName));
}

namespace
{
class AbstractSvxObjectTitleDescDialog_Impl final
    : public ControllerImpl_Async<AbstractSvxObjectTitleDescDialog, SvxObjectTitleDescDialog>
{
public:
    using ControllerImpl_BASE::ControllerImpl_BASE; // inherited ctor
    OUString GetTitle() override { return m_pDlg->GetTitle(); }
    OUString GetDescription() override { return m_pDlg->GetDescription(); }
    bool IsDecorative() override { return m_pDlg->IsDecorative(); }
};
}

VclPtr<AbstractSvxObjectTitleDescDialog> AbstractDialogFactory_Impl::CreateSvxObjectTitleDescDialog(weld::Window* pParent, const OUString& rTitle, const OUString& rDescription, bool const isDecorative)
{
    return VclPtr<AbstractSvxObjectTitleDescDialog_Impl>::Create(std::make_unique<SvxObjectTitleDescDialog>(pParent, rTitle, rDescription, isDecorative));
}

namespace
{
class AbstractSvxMultiPathDialog_Impl final
    : public ControllerImpl_Sync<AbstractSvxMultiPathDialog, SvxMultiPathDialog>
{
public:
    using ControllerImpl_BASE::ControllerImpl_BASE; // inherited ctor
    OUString GetPath() const override { return m_pDlg->GetPath(); }
    void SetPath(const OUString& rPath) override { m_pDlg->SetPath(rPath); }
    void SetTitle(const OUString& rNewTitle) override { m_pDlg->SetTitle(rNewTitle); }
};
}

VclPtr<AbstractSvxMultiPathDialog> AbstractDialogFactory_Impl::CreateSvxMultiPathDialog(weld::Window* pParent)
{
    return VclPtr<AbstractSvxMultiPathDialog_Impl>::Create(std::make_unique<SvxMultiPathDialog>(pParent));
}

namespace
{
class AbstractSvxPathSelectDialog_Impl final
    : public ControllerImpl_Sync<AbstractSvxMultiPathDialog, SvxPathSelectDialog>
{
public:
    using ControllerImpl_BASE::ControllerImpl_BASE; // inherited ctor
    OUString GetPath() const override { return m_pDlg->GetPath(); }
    void SetPath(const OUString& rPath) override { m_pDlg->SetPath(rPath); }
    void SetTitle(const OUString& rNewTitle) override { m_pDlg->SetTitle(rNewTitle); }
};
}

VclPtr<AbstractSvxMultiPathDialog> AbstractDialogFactory_Impl::CreateSvxPathSelectDialog(weld::Window* pParent)
{
    return VclPtr<AbstractSvxPathSelectDialog_Impl>::Create(std::make_unique<SvxPathSelectDialog>(pParent));
}

namespace
{
class AbstractSvxHpLinkDlg_Impl final
    : public ControllerImpl_Sync_Shared<AbstractSvxHpLinkDlg, SvxHpLinkDlg>
{
public:
    using ControllerImpl_BASE::ControllerImpl_BASE; // inherited ctor
    std::shared_ptr<SfxDialogController> GetController() override { return m_pDlg; }
    bool QueryClose() override { return m_pDlg->QueryClose(); }
};
}

VclPtr<AbstractSvxHpLinkDlg> AbstractDialogFactory_Impl::CreateSvxHpLinkDlg(SfxChildWindow* pChild, SfxBindings* pBindings, weld::Window* pParent)
{
    return VclPtr<AbstractSvxHpLinkDlg_Impl>::Create(std::make_shared<SvxHpLinkDlg>(pBindings, pChild, pParent));
}

namespace
{
class AbstractFmSearchDialog_Impl final
    : public ControllerImpl_Sync<AbstractFmSearchDialog, FmSearchDialog>
{
public:
    using ControllerImpl_BASE::ControllerImpl_BASE; // inherited ctor
    void SetFoundHandler(const Link<FmFoundRecordInformation&, void>& lnk) override
    {
        m_pDlg->SetFoundHandler(lnk);
    }
    void SetCanceledNotFoundHdl(const Link<FmFoundRecordInformation&, void>& lnk) override
    {
        m_pDlg->SetCanceledNotFoundHdl(lnk);
    }
    void SetActiveField(const OUString& strField) override { m_pDlg->SetActiveField(strField); }
};
}

VclPtr<AbstractFmSearchDialog> AbstractDialogFactory_Impl::CreateFmSearchDialog(weld::Window* pParent,
                                                        const OUString& strInitialText,
                                                        const std::vector< OUString >& _rContexts,
                                                        sal_Int16 nInitialContext,
                                                        const Link<FmSearchContext&,sal_uInt32>& lnkContextSupplier)
{
    return VclPtr<AbstractFmSearchDialog_Impl>::Create(std::make_unique<FmSearchDialog>(pParent,
                                                         strInitialText, _rContexts, nInitialContext, lnkContextSupplier));

}

namespace
{
class AbstractGraphicFilterDialog_Impl final
    : public ControllerImpl_Async<AbstractGraphicFilterDialog, GraphicFilterDialog>
{
public:
    using ControllerImpl_BASE::ControllerImpl_BASE; // inherited ctor
    Graphic GetFilteredGraphic(const Graphic& rGraphic, double fScaleX, double fScaleY) override
    {
        return m_pDlg->GetFilteredGraphic(rGraphic, fScaleX, fScaleY);
    }
};
}

VclPtr<AbstractGraphicFilterDialog> AbstractDialogFactory_Impl::CreateGraphicFilterEmboss(weld::Window* pParent,
                                            const Graphic& rGraphic)
{
    return VclPtr<AbstractGraphicFilterDialog_Impl>::Create(std::make_unique<GraphicFilterEmboss>(pParent, rGraphic, RectPoint::MM));
}

VclPtr<AbstractGraphicFilterDialog> AbstractDialogFactory_Impl::CreateGraphicFilterPoster(weld::Window* pParent,
                                            const Graphic& rGraphic)
{
    return VclPtr<AbstractGraphicFilterDialog_Impl>::Create(std::make_unique<GraphicFilterPoster>(pParent, rGraphic, 16));
}

VclPtr<AbstractGraphicFilterDialog> AbstractDialogFactory_Impl::CreateGraphicFilterSepia(weld::Window* pParent,
                                            const Graphic& rGraphic)
{
    return VclPtr<AbstractGraphicFilterDialog_Impl>::Create(std::make_unique<GraphicFilterSepia>(pParent, rGraphic, 10));
}

VclPtr<AbstractGraphicFilterDialog> AbstractDialogFactory_Impl::CreateGraphicFilterSmooth(weld::Window* pParent,
                                            const Graphic& rGraphic, double nRadius)
{
    return VclPtr<AbstractGraphicFilterDialog_Impl>::Create(std::make_unique<GraphicFilterSmooth>(pParent, rGraphic, nRadius));
}

VclPtr<AbstractGraphicFilterDialog> AbstractDialogFactory_Impl::CreateGraphicFilterSolarize(weld::Window* pParent,
                                            const Graphic& rGraphic)
{
    return VclPtr<AbstractGraphicFilterDialog_Impl>::Create(std::make_unique<GraphicFilterSolarize>(pParent, rGraphic, 128, false /*bInvert*/));
}

VclPtr<AbstractGraphicFilterDialog> AbstractDialogFactory_Impl::CreateGraphicFilterMosaic(weld::Window* pParent,
                                            const Graphic& rGraphic)
{
    return VclPtr<AbstractGraphicFilterDialog_Impl>::Create(std::make_unique<GraphicFilterMosaic>(pParent, rGraphic, 4, 4, false /*bEnhanceEdges*/));
}

namespace
{
// AbstractSvxAreaTabDialog implementations just forwards everything to the dialog
class AbstractSvxAreaTabDialog_Impl final
    : public ControllerImpl_Async<AbstractSvxAreaTabDialog, SvxAreaTabDialog>
{
public:
    using ControllerImpl_BASE::ControllerImpl_BASE; // inherited ctor
    void SetCurPageId(const OUString& rName) override { m_pDlg->SetCurPageId(rName); }
    const SfxItemSet* GetOutputItemSet() const override { return m_pDlg->GetOutputItemSet(); }
    WhichRangesContainer GetInputRanges(const SfxItemPool& pItem) override
    {
        return m_pDlg->GetInputRanges(pItem);
    }
    void SetInputSet(const SfxItemSet* pInSet) override { m_pDlg->SetInputSet(pInSet); }
    void SetText(const OUString& rStr) override { m_pDlg->set_title(rStr); }
};
}

VclPtr<AbstractSvxAreaTabDialog> AbstractDialogFactory_Impl::CreateSvxAreaTabDialog(weld::Window* pParent,
                                                            const SfxItemSet* pAttr,
                                                            SdrModel* pModel,
                                                            bool bShadow,
                                                            bool bSlideBackground)
{
    return VclPtr<AbstractSvxAreaTabDialog_Impl>::Create(
        std::make_shared<SvxAreaTabDialog>(pParent, pAttr, pModel, bShadow, bSlideBackground));
}

VclPtr<SfxAbstractTabDialog> AbstractDialogFactory_Impl::CreateSvxLineTabDialog(weld::Window* pParent, const SfxItemSet* pAttr, //add forSvxLineTabDialog
                                                                 SdrModel* pModel,
                                                                 const SdrObject* pObj ,
                                                                 bool bHasObj)
{
    return VclPtr<CuiAbstractTabController_Impl>::Create(std::make_shared<SvxLineTabDialog>(pParent, pAttr, pModel, pObj,bHasObj));
}

namespace
{
class AbstractSvxCharacterMapDialog_Impl
    : public ControllerImpl_Async<SfxAbstractDialog, SvxCharacterMap>
{
public:
    using ControllerImpl_BASE::ControllerImpl_BASE; // inherited ctor
    bool StartExecuteAsync(AsyncContext& rCtx) override
    {
        m_pDlg->prepForRun();
        return SvxCharacterMap::runAsync(m_pDlg, rCtx.maEndDialogFn);
    }
    const SfxItemSet* GetOutputItemSet() const override { return m_pDlg->GetOutputItemSet(); }
    void SetText(const OUString& rStr) override { m_pDlg->set_title(rStr); }
};
}

VclPtr<SfxAbstractDialog> AbstractDialogFactory_Impl::CreateCharMapDialog(weld::Window* pParent, const SfxItemSet& rAttr,
                                                                          const Reference< XFrame >& rDocumentFrame)
{
    return VclPtr<AbstractSvxCharacterMapDialog_Impl>::Create(std::make_unique<SvxCharacterMap>(pParent, &rAttr, rDocumentFrame));
}

namespace
{
class CuiAbstractSingleTabController_Impl final
    : public ControllerImpl_Async<SfxAbstractDialog, SfxSingleTabDialogController>
{
public:
    using ControllerImpl_BASE::ControllerImpl_BASE; // inherited ctor
    const SfxItemSet* GetOutputItemSet() const override { return m_pDlg->GetOutputItemSet(); }

    // From class Window.
    void SetText(const OUString& rStr) override { m_pDlg->set_title(rStr); }
};
}

VclPtr<SfxAbstractDialog> AbstractDialogFactory_Impl::CreateEventConfigDialog(weld::Widget* pParent,
                                                                              std::unique_ptr<const SfxItemSet> xAttr,
                                                                              const Reference< XFrame >& rDocumentFrame)
{
    return VclPtr<CuiAbstractSingleTabController_Impl>::Create(std::make_unique<SfxMacroAssignDlg>(pParent, rDocumentFrame, std::move(xAttr)));
}

VclPtr<SfxAbstractDialog> AbstractDialogFactory_Impl::CreateSfxDialog(weld::Window* pParent,
                                                                      const SfxItemSet& rAttr,
                                                                      const SdrView* pView,
                                                                      sal_uInt32 nResId)
{
    switch ( nResId )
    {
        case RID_SVXPAGE_MEASURE:
            return VclPtr<CuiAbstractSingleTabController_Impl>::Create(std::make_unique<SvxMeasureDialog>(pParent, rAttr, pView));
        case RID_SVXPAGE_CONNECTION:
            return VclPtr<CuiAbstractSingleTabController_Impl>::Create(std::make_unique<SvxConnectionDialog>(pParent, rAttr, pView));
        case RID_SFXPAGE_DBREGISTER:
            return VclPtr<CuiAbstractSingleTabController_Impl>::Create(std::make_unique<DatabaseRegistrationDialog>(pParent, rAttr));
    }

    return nullptr;
}

namespace
{
class AbstractSvxPostItDialog_Impl final
    : public ControllerImpl_Sync<AbstractSvxPostItDialog, SvxPostItDialog>
{
public:
    using ControllerImpl_BASE::ControllerImpl_BASE; // inherited ctor
    // From class Window
    void SetText(const OUString& rStr) override { m_pDlg->set_title(rStr); }
    const SfxItemSet* GetOutputItemSet() const override { return m_pDlg->GetOutputItemSet(); }
    void SetPrevHdl(const Link<AbstractSvxPostItDialog&, void>& rLink) override;
    void SetNextHdl(const Link<AbstractSvxPostItDialog&, void>& rLink) override;
    void EnableTravel(bool bNext, bool bPrev) override { m_pDlg->EnableTravel(bNext, bPrev); }
    OUString GetNote() override { return m_pDlg->GetNote(); }
    void SetNote(const OUString& rTxt) override { m_pDlg->SetNote(rTxt); }
    void ShowLastAuthor(const OUString& rAuthor, const OUString& rDate) override
    {
        m_pDlg->ShowLastAuthor(rAuthor, rDate);
    }
    void DontChangeAuthor() override { m_pDlg->DontChangeAuthor(); }
    void HideAuthor() override { m_pDlg->HideAuthor(); }
    std::shared_ptr<weld::Dialog> GetDialog() override { return m_pDlg->GetDialog(); }

private:
    Link<AbstractSvxPostItDialog&, void> aNextHdl;
    Link<AbstractSvxPostItDialog&, void> aPrevHdl;
    DECL_LINK(NextHdl, SvxPostItDialog&, void);
    DECL_LINK(PrevHdl, SvxPostItDialog&, void);
};

void AbstractSvxPostItDialog_Impl::SetNextHdl(const Link<AbstractSvxPostItDialog&, void>& rLink)
{
    aNextHdl = rLink;
    if (rLink.IsSet())
        m_pDlg->SetNextHdl(LINK(this, AbstractSvxPostItDialog_Impl, NextHdl));
    else
        m_pDlg->SetNextHdl(Link<SvxPostItDialog&, void>());
}

void AbstractSvxPostItDialog_Impl::SetPrevHdl(const Link<AbstractSvxPostItDialog&, void>& rLink)
{
    aPrevHdl = rLink;
    if (rLink.IsSet())
        m_pDlg->SetPrevHdl(LINK(this, AbstractSvxPostItDialog_Impl, PrevHdl));
    else
        m_pDlg->SetPrevHdl(Link<SvxPostItDialog&, void>());
}

IMPL_LINK_NOARG(AbstractSvxPostItDialog_Impl, NextHdl, SvxPostItDialog&, void)
{
    aNextHdl.Call(*this);
}

IMPL_LINK_NOARG(AbstractSvxPostItDialog_Impl, PrevHdl, SvxPostItDialog&, void)
{
    aPrevHdl.Call(*this);
}
}

VclPtr<AbstractSvxPostItDialog> AbstractDialogFactory_Impl::CreateSvxPostItDialog(weld::Widget* pParent,
                                                                                  const SfxItemSet& rCoreSet,
                                                                                  bool bPrevNext)
{
    return VclPtr<AbstractSvxPostItDialog_Impl>::Create(std::make_unique<SvxPostItDialog>(pParent, rCoreSet, bPrevNext));
}

namespace {

class SvxMacroAssignDialog_Impl : public ControllerImpl_Async<VclAbstractDialog, SvxMacroAssignDlg>
{
public:
    SvxMacroAssignDialog_Impl( weld::Window* _pParent, const Reference< XFrame >& _rxDocumentFrame, const bool _bUnoDialogMode,
            const Reference< XNameReplace >& _rxEvents, const sal_uInt16 _nInitiallySelectedEvent )
        : ControllerImpl_BASE(nullptr)
        , m_aItems( SfxGetpApp()->GetPool(), svl::Items<SID_ATTR_MACROITEM, SID_ATTR_MACROITEM> )
    {
        m_aItems.Put( SfxBoolItem( SID_ATTR_MACROITEM, _bUnoDialogMode ) );
        m_pDlg = std::make_shared<SvxMacroAssignDlg>(_pParent, _rxDocumentFrame, m_aItems, _rxEvents, _nInitiallySelectedEvent);
    }

private:
    SfxItemSet                              m_aItems;
};
}


VclPtr<VclAbstractDialog> AbstractDialogFactory_Impl::CreateSvxMacroAssignDlg(
    weld::Window* _pParent, const Reference< XFrame >& _rxDocumentFrame, const bool _bUnoDialogMode,
    const Reference< XNameReplace >& _rxEvents, const sal_uInt16 _nInitiallySelectedEvent )
{
    return VclPtr<SvxMacroAssignDialog_Impl>::Create(_pParent, _rxDocumentFrame, _bUnoDialogMode, _rxEvents, _nInitiallySelectedEvent);
}

// Factories for TabPages
CreateTabPage AbstractDialogFactory_Impl::GetTabPageCreatorFunc( sal_uInt16 nId )
{
    switch ( nId )
    {
        case RID_SW_TP_BACKGROUND :
        case RID_SVXPAGE_BKG :
            return SvxBkgTabPage::Create;
        case RID_SVXPAGE_TEXTANIMATION :
            return SvxTextAnimationPage::Create;
        case RID_SVXPAGE_TRANSPARENCE :
            return SvxTransparenceTabPage::Create;
        case RID_SVXPAGE_AREA :
            return SvxAreaTabPage::Create;
        case RID_SVXPAGE_SHADOW :
            return SvxShadowTabPage::Create;
        case RID_SVXPAGE_LINE :
            return SvxLineTabPage::Create;
        case RID_SVXPAGE_CONNECTION :
            return SvxConnectionPage::Create;
        case RID_SVXPAGE_MEASURE :
            return SvxMeasurePage::Create;
        case RID_SFXPAGE_GENERAL :
            return SvxGeneralTabPage::Create;
        case RID_SVXPAGE_PICK_SINGLE_NUM :
            return SvxSingleNumPickTabPage::Create;
        case RID_SVXPAGE_PICK_BMP :
            return SvxBitmapPickTabPage::Create;
        case RID_SVXPAGE_PICK_BULLET :
            return SvxBulletPickTabPage::Create;
        case RID_SVXPAGE_NUM_OPTIONS :
            return SvxNumOptionsTabPage::Create;
        case RID_SVXPAGE_PICK_NUM :
            return SvxNumPickTabPage::Create;
        case RID_SVXPAGE_NUM_POSITION :
            return SvxNumPositionTabPage::Create;
        case RID_SVXPAGE_PARA_ASIAN :
            return SvxAsianTabPage::Create;
        case RID_SVXPAGE_EXT_PARAGRAPH :
            return SvxExtParagraphTabPage::Create;
        case RID_SVXPAGE_ALIGN_PARAGRAPH :
            return SvxParaAlignTabPage::Create;
        case RID_SVXPAGE_STD_PARAGRAPH :
            return SvxStdParagraphTabPage::Create;
        case RID_SVXPAGE_TABULATOR :
            return SvxTabulatorTabPage::Create;
        case RID_SVXPAGE_TEXTATTR :
            return SvxTextAttrPage::Create;
        case RID_SVXPAGE_ALIGNMENT :
            return svx::AlignmentTabPage::Create;
        case RID_SVXPAGE_BORDER :
            return SvxBorderTabPage::Create;
        case RID_SVXPAGE_CHAR_NAME :
            return SvxCharNamePage::Create;
        case RID_SVXPAGE_CHAR_EFFECTS :
            return SvxCharEffectsPage::Create;
        case RID_SVXPAGE_CHAR_POSITION :
            return SvxCharPositionPage::Create;
        case RID_SVXPAGE_CHAR_TWOLINES :
            return SvxCharTwoLinesPage::Create;
        case RID_SVXPAGE_NUMBERFORMAT :
            return SvxNumberFormatTabPage::Create;
        case RID_SVXPAGE_PAGE :
            return SvxPageDescPage::Create;
        case RID_SVXPAGE_GRFCROP :
            return SvxGrfCropPage::Create;
        case RID_SVXPAGE_MACROASSIGN :
            return SfxMacroTabPage::Create;
        case RID_SVXPAGE_TEXTCOLUMNS:
            return SvxTextColumnsPage::Create;
        default:
            break;
    }

    return nullptr;
}

DialogGetRanges AbstractDialogFactory_Impl::GetDialogGetRangesFunc()
{
    return SvxPostItDialog::GetRanges;
}

GetTabPageRanges AbstractDialogFactory_Impl::GetTabPageRangesFunc( sal_uInt16 nId )
{
    switch ( nId )
    {
        case RID_SVXPAGE_TEXTANIMATION :
            return SvxTextAnimationPage::GetRanges;
        case RID_SVXPAGE_TRANSPARENCE :
            return SvxTransparenceTabPage::GetRanges;
        case RID_SVXPAGE_AREA :
            return SvxAreaTabPage::GetRanges;
        case RID_SVXPAGE_SHADOW :
            return SvxShadowTabPage::GetRanges;
        case RID_SVXPAGE_LINE :
            return SvxLineTabPage::GetRanges;
        case RID_SVXPAGE_CONNECTION :
            return SvxConnectionPage::GetRanges;
        case RID_SVXPAGE_MEASURE :
            return SvxMeasurePage::GetRanges;
        case RID_SVXPAGE_PARA_ASIAN :
            return SvxAsianTabPage::GetRanges;
        case RID_SVXPAGE_EXT_PARAGRAPH :
            return SvxExtParagraphTabPage::GetRanges;
        case RID_SVXPAGE_ALIGN_PARAGRAPH :
            return SvxParaAlignTabPage::GetRanges;
        case RID_SVXPAGE_STD_PARAGRAPH :
            return SvxStdParagraphTabPage::GetRanges;
        case RID_SVXPAGE_TABULATOR :
            return SvxTabulatorTabPage::GetRanges;
        case RID_SVXPAGE_TEXTATTR :
            return SvxTextAttrPage::GetRanges;
        case RID_SVXPAGE_ALIGNMENT :
            return svx::AlignmentTabPage::GetRanges;
        case RID_SW_TP_BACKGROUND :
        case RID_SVXPAGE_BKG:
            return SvxBkgTabPage::GetRanges;
        case RID_SVXPAGE_BORDER :
            return SvxBorderTabPage::GetRanges;
        case RID_SVXPAGE_CHAR_NAME :
            return SvxCharNamePage::GetRanges;
        case RID_SVXPAGE_CHAR_EFFECTS :
            return SvxCharEffectsPage::GetRanges;
        case RID_SVXPAGE_CHAR_POSITION :
            return SvxCharPositionPage::GetRanges;
        case RID_SVXPAGE_CHAR_TWOLINES :
            return SvxCharTwoLinesPage::GetRanges;
        case RID_SVXPAGE_NUMBERFORMAT :
            return SvxNumberFormatTabPage::GetRanges;
        case RID_SVXPAGE_PAGE :
            return SvxPageDescPage::GetRanges;
        case RID_SVXPAGE_ASIAN_LAYOUT:
            return SvxAsianLayoutPage::GetRanges;
        case RID_SVXPAGE_TEXTCOLUMNS:
            return SvxTextColumnsPage::GetRanges;
        default:
            break;
    }

    return nullptr;
}

namespace
{
class AbstractInsertObjectDialog_Impl final
    : public ControllerImpl_Sync<SfxAbstractInsertObjectDialog, InsertObjectDialog_Impl>
{
public:
    using ControllerImpl_BASE::ControllerImpl_BASE; // inherited ctor
    Reference<css::embed::XEmbeddedObject> GetObject() override { return m_pDlg->GetObject(); }
    Reference<css::io::XInputStream> GetIconIfIconified(OUString* pGraphicMediaType) override
    {
        return m_pDlg->GetIconIfIconified(pGraphicMediaType);
    }
    bool IsCreateNew() override { return m_pDlg->IsCreateNew(); }
};
}

VclPtr<SfxAbstractInsertObjectDialog> AbstractDialogFactory_Impl::CreateInsertObjectDialog(weld::Window* pParent, const OUString& rCommand,
            const Reference <css::embed::XStorage>& xStor, const SvObjectServerList* pList)
{
    std::unique_ptr<InsertObjectDialog_Impl> pDlg;
    if ( rCommand == ".uno:InsertObject" )
        pDlg.reset(new SvInsertOleDlg(pParent, xStor, pList));
    else if ( rCommand == ".uno:InsertObjectFloatingFrame" )
        pDlg.reset(new SfxInsertFloatingFrameDialog(pParent, xStor));

    if ( pDlg )
    {
        pDlg->SetHelpId(rCommand);
        return VclPtr<AbstractInsertObjectDialog_Impl>::Create( std::move(pDlg) );
    }
    return nullptr;
}

VclPtr<VclAbstractDialog> AbstractDialogFactory_Impl::CreateEditObjectDialog(weld::Window* pParent, const OUString& rCommand,
            const Reference<css::embed::XEmbeddedObject>& xObj)
{
    if ( rCommand == ".uno:InsertObjectFloatingFrame" )
    {
        auto pDlg = std::make_unique<SfxInsertFloatingFrameDialog>(pParent, xObj);
        pDlg->SetHelpId(rCommand);
        return VclPtr<AbstractInsertObjectDialog_Impl>::Create( std::move(pDlg) );
    }
    return nullptr;
}

namespace
{
class AbstractPasteDialog_Impl final
    : public ControllerImpl_Async<SfxAbstractPasteDialog, SvPasteObjectDialog>
{
public:
    using ControllerImpl_BASE::ControllerImpl_BASE; // inherited ctor
    void Insert(SotClipboardFormatId nFormat, const OUString& rFormatName) override
    {
        m_pDlg->Insert(nFormat, rFormatName);
    }
    void InsertUno(const OUString& sCmd, const OUString& sLabel) override
    {
        m_pDlg->InsertUno(sCmd, sLabel);
    }
    void SetObjName(const SvGlobalName& rClass, const OUString& rObjName) override
    {
        m_pDlg->SetObjName(rClass, rObjName);
    }
    void PreGetFormat(const TransferableDataHelper& aHelper) override
    {
        m_pDlg->PreGetFormat(aHelper);
    }
    SotClipboardFormatId GetFormatOnly() override { return m_pDlg->GetFormatOnly(); }
    SotClipboardFormatId GetFormat(const TransferableDataHelper& aHelper) override
    {
        return m_pDlg->GetFormat(aHelper);
    }
};
}

VclPtr<SfxAbstractPasteDialog> AbstractDialogFactory_Impl::CreatePasteDialog(weld::Window* pParent)
{
    return VclPtr<AbstractPasteDialog_Impl>::Create(std::make_shared<SvPasteObjectDialog>(pParent));
}

VclPtr<SfxAbstractLinksDialog> AbstractDialogFactory_Impl::CreateLinksDialog(weld::Window* pParent, sfx2::LinkManager* pMgr, bool bHTML, sfx2::SvBaseLink* p)
{
    using AbstractLinksDialog_Impl = ControllerImpl_Async<SfxAbstractLinksDialog, SvBaseLinksDlg>;
    auto xLinkDlg(std::make_unique<SvBaseLinksDlg>(pParent, pMgr, bHTML));
    if (p)
        xLinkDlg->SetActLink(p);
    return VclPtr<AbstractLinksDialog_Impl>::Create(std::move(xLinkDlg));
}

VclPtr<SfxAbstractTabDialog> AbstractDialogFactory_Impl::CreateSvxFormatCellsDialog(weld::Window* pParent, const SfxItemSet& rAttr, const SdrModel& rModel, bool bStyle)
{
    return VclPtr<CuiAbstractTabController_Impl>::Create(std::make_shared<SvxFormatCellsDialog>(pParent, rAttr, rModel, bStyle));
}

VclPtr<SvxAbstractSplitTableDialog> AbstractDialogFactory_Impl::CreateSvxSplitTableDialog(weld::Window* pParent, bool bIsTableVertical, tools::Long nMaxVertical)
{
    return VclPtr<SvxAbstractSplitTableDialog_Impl>::Create( std::make_shared<SvxSplitTableDlg>(pParent, bIsTableVertical, nMaxVertical, 99 ));
}

std::shared_ptr<SvxAbstractNewTableDialog> AbstractDialogFactory_Impl::CreateSvxNewTableDialog(weld::Window* pParent)
{
    return std::make_shared<SvxNewTableDialogWrapper>(pParent);
}

VclPtr<VclAbstractDialog> AbstractDialogFactory_Impl::CreateOptionsDialog(weld::Window* pParent, const OUString& rExtensionId)
{
    return VclPtr<CuiAbstractController_Impl>::Create(std::make_unique<OfaTreeOptionsDialog>(pParent, rExtensionId));
}

VclPtr<SvxAbstractInsRowColDlg> AbstractDialogFactory_Impl::CreateSvxInsRowColDlg(weld::Window* pParent, bool bCol, const OUString& rHelpId)
{
    return VclPtr<SvxAbstractInsRowColDlg_Impl>::Create(std::make_shared<SvxInsRowColDlg>(pParent, bCol, rHelpId));
}

namespace
{
class AbstractPasswordToOpenModifyDialog_Impl final
    : public ControllerImpl_Async<AbstractPasswordToOpenModifyDialog, PasswordToOpenModifyDialog>
{
public:
    using ControllerImpl_BASE::ControllerImpl_BASE; // inherited ctor
    OUString GetPasswordToOpen() const override { return m_pDlg->GetPasswordToOpen(); }
    OUString GetPasswordToModify() const override { return m_pDlg->GetPasswordToModify(); }
    bool IsRecommendToOpenReadonly() const override { return m_pDlg->IsRecommendToOpenReadonly(); }
    void Response(sal_Int32 nResult) override { m_pDlg->response(nResult); }
    void AllowEmpty() override { m_pDlg->AllowEmpty(); }
};
}

VclPtr<AbstractPasswordToOpenModifyDialog> AbstractDialogFactory_Impl::CreatePasswordToOpenModifyDialog(
    weld::Window * pParent, sal_uInt16 nMaxPasswdLen, bool bIsPasswordToModify)
{
    return VclPtr<AbstractPasswordToOpenModifyDialog_Impl>::Create(std::make_unique<PasswordToOpenModifyDialog>(pParent, nMaxPasswdLen, bIsPasswordToModify));
}

VclPtr<AbstractScreenshotAnnotationDlg> AbstractDialogFactory_Impl::CreateScreenshotAnnotationDlg(weld::Dialog& rParentDialog)
{
    using AbstractScreenshotAnnotationDlg_Impl
        = ControllerImpl_Sync<AbstractScreenshotAnnotationDlg, ScreenshotAnnotationDlg>;

    return VclPtr<AbstractScreenshotAnnotationDlg_Impl>::Create(std::make_unique<ScreenshotAnnotationDlg>(rParentDialog));
}

namespace
{
class AbstractSignatureLineDialog_Impl final
    : public ControllerImpl_Async<AbstractSignatureLineDialog, SignatureLineDialog>
{
public:
    using ControllerImpl_BASE::ControllerImpl_BASE; // inherited ctor
    void Apply() override { m_pDlg->Apply(); }
};
}

VclPtr<AbstractSignatureLineDialog> AbstractDialogFactory_Impl::CreateSignatureLineDialog(
    weld::Window* pParent, const Reference<XModel> xModel, bool bEditExisting)
{
    return VclPtr<AbstractSignatureLineDialog_Impl>::Create(
        std::make_unique<SignatureLineDialog>(pParent, xModel, bEditExisting));
}

namespace
{
class AbstractSignSignatureLineDialog_Impl final
    : public ControllerImpl_Async<AbstractSignSignatureLineDialog, SignSignatureLineDialog>
{
public:
    using ControllerImpl_BASE::ControllerImpl_BASE; // inherited ctor
    void Apply() override { m_pDlg->Apply(); }
};
}

VclPtr<AbstractSignSignatureLineDialog>
AbstractDialogFactory_Impl::CreateSignSignatureLineDialog(weld::Window* pParent,
                                                          const Reference<XModel> xModel)
{
    return VclPtr<AbstractSignSignatureLineDialog_Impl>::Create(
        std::make_unique<SignSignatureLineDialog>(pParent, xModel));
}

VclPtr<AbstractQrCodeGenDialog> AbstractDialogFactory_Impl::CreateQrCodeGenDialog(
    weld::Window* pParent, const Reference<XModel> xModel, bool bEditExisting)
{
    using AbstractQrCodeGenDialog_Impl
        = ControllerImpl_Async<AbstractQrCodeGenDialog, QrCodeGenDialog>;
    return VclPtr<AbstractQrCodeGenDialog_Impl>::Create(
        std::make_shared<QrCodeGenDialog>(pParent, xModel, bEditExisting));
}

VclPtr<AbstractAdditionsDialog> AbstractDialogFactory_Impl::CreateAdditionsDialog(
    weld::Window* pParent, const OUString& sAdditionsTag)
{
#if HAVE_FEATURE_EXTENSIONS
    using AbstractAdditionsDialog_Impl
        = ControllerImpl_Async<AbstractAdditionsDialog, weld::GenericDialogController>;
    return VclPtr<AbstractAdditionsDialog_Impl>::Create(
        std::make_unique<AdditionsDialog>(pParent, sAdditionsTag));
#else
    (void) pParent;
    (void) sAdditionsTag;
    return nullptr;
#endif
}

VclPtr<VclAbstractDialog>
AbstractDialogFactory_Impl::CreateAboutDialog(weld::Window* pParent)
{
    return VclPtr<CuiAbstractControllerAsync_Impl>::Create(
        std::make_shared<AboutDialog>(pParent));
}

VclPtr<VclAbstractDialog>
AbstractDialogFactory_Impl::CreateTipOfTheDayDialog(weld::Window* pParent)
{
#if !ENABLE_WASM_STRIP_PINGUSER
    return VclPtr<CuiAbstractControllerAsync_Impl>::Create(
        std::make_shared<TipOfTheDayDialog>(pParent));
#else
    (void) pParent;
    return nullptr;
#endif
}

VclPtr<VclAbstractDialog>
AbstractDialogFactory_Impl::CreateWidgetTestDialog(weld::Window* pParent)
{
    using CuiAbstractWidgetTestControllerAsync_Impl
        = ControllerImpl_Async<VclAbstractDialog, weld::GenericDialogController>;
    return VclPtr<CuiAbstractWidgetTestControllerAsync_Impl>::Create(
        std::make_shared<WidgetTestDialog>(pParent));
}

VclPtr<VclAbstractDialog>
AbstractDialogFactory_Impl::CreateToolbarmodeDialog(weld::Window* pParent)
{
    return VclPtr<CuiAbstractController_Impl>::Create(
        std::make_unique<ToolbarmodeDialog>(pParent));
}

VclPtr<AbstractDiagramDialog>
AbstractDialogFactory_Impl::CreateDiagramDialog(
    weld::Window* pParent,
    SdrObjGroup& rDiagram)
{
    using AbstractDiagramDialog_Impl = ControllerImpl_Async<AbstractDiagramDialog, DiagramDialog>;
    return VclPtr<AbstractDiagramDialog_Impl>::Create(
        std::make_unique<DiagramDialog>(pParent, rDiagram));
}

#ifdef _WIN32
VclPtr<VclAbstractDialog>
AbstractDialogFactory_Impl::CreateFileExtCheckDialog(weld::Window* pParent, const OUString& sTitle,
                                                     const OUString& sMsg)
{
    return VclPtr<CuiAbstractController_Impl>::Create(
        std::make_unique<FileExtCheckDialog>(pParent, sTitle, sMsg));
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
