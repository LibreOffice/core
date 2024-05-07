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
#pragma once

#include <svx/svxdlg.hxx>
#include <svx/zoom_def.hxx>
#include <com/sun/star/container/XNameReplace.hpp>

#include <tools/link.hxx>
#include <com/sun/star/frame/XFrame.hpp>

#include <cfgutil.hxx>
#include <svx/cuicharmap.hxx>
#include <dlgname.hxx>
#include <cuifmsearch.hxx>
#include <cuigaldlg.hxx>
#include <cuigrfflt.hxx>
#include <cuihyperdlg.hxx>
#include <cuiimapwnd.hxx>
#include <cuisrchdlg.hxx>
#include <cuitabarea.hxx>
#include <cuitbxform.hxx>
#include <DiagramDialog.hxx>
#include <hangulhanjadlg.hxx>
#include <hyphen.hxx>
#include <insdlg.hxx>
#include <labdlg.hxx>
#include <linkdlg.hxx>
#include <multipat.hxx>
#include <optdict.hxx>
#include <passwdomdlg.hxx>
#include <pastedlg.hxx>
#include <postdlg.hxx>
#include <QrCodeGenDialog.hxx>
#include <screenshotannotationdlg.hxx>
#include <securityoptions.hxx>
#include <showcols.hxx>
#include <SignatureLineDialog.hxx>
#include <SignSignatureLineDialog.hxx>
#include <SpellDialog.hxx>
#include <srchxtra.hxx>
#include <thesdlg.hxx>
#include <transfrm.hxx>
#include <zoom.hxx>
#include <AdditionsDialog.hxx>

#define DECL_ABSTDLG_CLASS_(Class,Base,Dialog,StdPtr) \
class Class##_Impl final : public Base \
{ \
    StdPtr<Dialog> m_xDlg; \
public: \
    explicit Class##_Impl(StdPtr<Dialog> p) : m_xDlg(std::move(p)) {} \
    virtual short Execute() override;

#define DECL_ABSTDLG_CLASS_UNIQUE(Class,Base,Dialog) \
DECL_ABSTDLG_CLASS_(Class,Base,Dialog,std::unique_ptr)

#define DECL_ABSTDLG_CLASS_SHARED(Class,Base,Dialog) \
DECL_ABSTDLG_CLASS_(Class,Base,Dialog,std::shared_ptr)

#define IMPL_ABSTDLG_CLASS(Class) \
short Class##_Impl::Execute() \
{ \
    return m_xDlg->run(); \
}

#define DECL_ABSTDLG_CLASS(Class,Dialog) \
DECL_ABSTDLG_CLASS_UNIQUE(Class,Class,Dialog)

// Async AKA std::shared_ptr

#define DECL_ABSTDLG_CLASS_SHARED_ASYNC(Class,Base,Dialog) \
DECL_ABSTDLG_CLASS_SHARED(Class,Base,Dialog) \
    virtual bool StartExecuteAsync(AsyncContext &rCtx) override;

#define DECL_ABSTDLG_CLASS_ASYNC(Class,Dialog) \
DECL_ABSTDLG_CLASS_SHARED_ASYNC(Class,Class,Dialog)

#define IMPL_ABSTDLG_CLASS_ASYNC(Class,Dialog) \
short Class##_Impl::Execute() \
{ \
    return m_xDlg->run(); \
} \
bool Class##_Impl::StartExecuteAsync(AsyncContext &rCtx) \
{ \
    return Dialog::runAsync(m_xDlg, rCtx.maEndDialogFn); \
}

// CuiAbstractController_Impl
DECL_ABSTDLG_CLASS_UNIQUE(CuiAbstractController, VclAbstractDialog, weld::DialogController)
};

// CuiAbstractControllerAsync_Impl
DECL_ABSTDLG_CLASS_SHARED_ASYNC(CuiAbstractControllerAsync, VclAbstractDialog, weld::DialogController)
};

// CuiAbstractWidgetTestControllerAsync_Impl
DECL_ABSTDLG_CLASS_SHARED_ASYNC(CuiAbstractWidgetTestControllerAsync, VclAbstractDialog, weld::GenericDialogController)
};

// CuiAbstractSingleTabController_Impl
DECL_ABSTDLG_CLASS_SHARED_ASYNC(CuiAbstractSingleTabController, SfxAbstractDialog, SfxSingleTabDialogController)
    virtual const SfxItemSet*   GetOutputItemSet() const override;
    //From class Window.
    virtual void          SetText( const OUString& rStr ) override;
};

// CuiAbstractTabController_Impl
DECL_ABSTDLG_CLASS_SHARED_ASYNC(CuiAbstractTabController, SfxAbstractTabDialog, SfxTabDialogController)
    virtual void                SetCurPageId( const OUString &rName ) override;
    virtual const SfxItemSet*   GetOutputItemSet() const override;
    virtual WhichRangesContainer GetInputRanges( const SfxItemPool& pItem ) override;
    virtual void                SetInputSet( const SfxItemSet* pInSet ) override;
    virtual void        SetText( const OUString& rStr ) override;

    // screenshotting
    virtual std::vector<OUString> getAllPageUIXMLDescriptions() const override;
    virtual bool selectPageByUIXMLDescription(const OUString& rUIXMLDescription) override;
    virtual BitmapEx createScreenshot() const override;
    virtual OUString GetScreenshotId() const override;
};

// AbstractHangulHanjaConversionDialog_Impl
DECL_ABSTDLG_CLASS_UNIQUE(AbstractHangulHanjaConversionDialog,AbstractHangulHanjaConversionDialog,svx::HangulHanjaConversionDialog)
    virtual void      EndDialog(sal_Int32 nResult) override;
    virtual void      EnableRubySupport( bool _bVal ) override;
    virtual void      SetByCharacter( bool _bByCharacter ) override ;
    virtual void      SetConversionDirectionState( bool _bTryBothDirections, editeng::HangulHanjaConversion::ConversionDirection _ePrimaryConversionDirection ) override;
    virtual void      SetConversionFormat( editeng::HangulHanjaConversion::ConversionFormat _eType ) override;
    virtual void      SetOptionsChangedHdl( const Link<LinkParamNone*,void>& _rHdl ) override;
    virtual void      SetIgnoreHdl( const Link<weld::Button&,void>& _rHdl ) override;
    virtual void      SetIgnoreAllHdl( const Link<weld::Button&,void>& _rHdl ) override ;
    virtual void      SetChangeHdl( const Link<weld::Button&,void>& _rHdl ) override ;
    virtual void      SetChangeAllHdl( const Link<weld::Button&,void>& rHdl ) override ;
    virtual void      SetClickByCharacterHdl( const Link<weld::Toggleable&,void>& rHdl ) override ;
    virtual void      SetConversionFormatChangedHdl( const Link<weld::Toggleable&,void>& _rHdl ) override ;
    virtual void      SetFindHdl( const Link<weld::Button&,void>& _rHdl ) override;
    virtual bool      GetUseBothDirections( ) const override;
    virtual editeng::HangulHanjaConversion::ConversionDirection
                      GetDirection( editeng::HangulHanjaConversion::ConversionDirection _eDefaultDirection ) const override;
    virtual void      SetCurrentString(
                        const OUString& _rNewString,
                        const css::uno::Sequence< OUString >& _rSuggestions,
                        bool _bOriginatesFromDocument = true
                      ) override;
    virtual OUString  GetCurrentString( ) const override ;
    virtual editeng::HangulHanjaConversion::ConversionFormat
                      GetConversionFormat( ) const override ;
    virtual void      FocusSuggestion( ) override;
    virtual OUString  GetCurrentSuggestion( ) const override;
};

// AbstractThesaurusDialog_Impl
DECL_ABSTDLG_CLASS_ASYNC(AbstractThesaurusDialog,SvxThesaurusDialog)
    virtual OUString    GetWord() override;
};

// AbstractHyphenWordDialog_Impl
DECL_ABSTDLG_CLASS(AbstractHyphenWordDialog,SvxHyphenWordDialog)
};

// AbstractFmShowColsDialog_Impl
DECL_ABSTDLG_CLASS(AbstractFmShowColsDialog,FmShowColsDialog)
    virtual void SetColumns(const css::uno::Reference< css::container::XIndexContainer>& xCols) override;
};

// AbstractSvxZoomDialog_Impl
DECL_ABSTDLG_CLASS(AbstractSvxZoomDialog,SvxZoomDialog)
    virtual void  SetLimits( sal_uInt16 nMin, sal_uInt16 nMax ) override;
    virtual void  HideButton( ZoomButtonId nBtnId ) override;
    virtual const SfxItemSet*   GetOutputItemSet() const override ;
};

// AbstractSpellDialog_Impl
DECL_ABSTDLG_CLASS_SHARED_ASYNC(AbstractSpellDialog, AbstractSpellDialog, svx::SpellDialog)
    virtual void InvalidateDialog() override;
    virtual std::shared_ptr<SfxDialogController> GetController() override;
    virtual SfxBindings& GetBindings() override;
};

// AbstractTitleDialog_Impl
DECL_ABSTDLG_CLASS(AbstractTitleDialog,TitleDialog)
    virtual OUString  GetTitle() const override ;

};

// AbstractScriptSelectorDialog_Impl
DECL_ABSTDLG_CLASS_ASYNC(AbstractScriptSelectorDialog,SvxScriptSelectorDialog)
    virtual OUString GetScriptURL() const override;
    virtual void SetRunLabel() override;
};

// AbstractGalleryIdDialog_Impl
DECL_ABSTDLG_CLASS(AbstractGalleryIdDialog,GalleryIdDialog)
    virtual sal_uInt32 GetId() const override;
};

// AbstractURLDlg_Impl
DECL_ABSTDLG_CLASS(AbstractURLDlg, URLDlg)
    virtual OUString      GetURL() const override;
    virtual OUString      GetAltText() const override;
    virtual OUString      GetDesc() const override;
    virtual OUString      GetTarget() const override;
    virtual OUString      GetName() const override;
};

// AbstractSvxSearchSimilarityDialog_Impl
DECL_ABSTDLG_CLASS_ASYNC(AbstractSvxSearchSimilarityDialog,SvxSearchSimilarityDialog)
    virtual sal_uInt16              GetOther() override;
    virtual sal_uInt16              GetShorter() override;
    virtual sal_uInt16              GetLonger() override;
    virtual bool                    IsRelaxed() override;
};

// AbstractSvxJSearchOptionsDialog_Impl
DECL_ABSTDLG_CLASS(AbstractSvxJSearchOptionsDialog,SvxJSearchOptionsDialog)
    virtual TransliterationFlags    GetTransliterationFlags() const override;
};

// AbstractSvxTransformTabDialog_Impl
DECL_ABSTDLG_CLASS_ASYNC(AbstractSvxTransformTabDialog,SvxTransformTabDialog)
    virtual void SetValidateFramePosLink( const Link<SvxSwFrameValidation&,void>& rLink ) override;
    virtual void                SetCurPageId( const OUString& rName ) override;
    virtual const SfxItemSet*   GetOutputItemSet() const override;
    virtual WhichRangesContainer GetInputRanges( const SfxItemPool& pItem ) override;
    virtual void                SetInputSet( const SfxItemSet* pInSet ) override;
    virtual void        SetText( const OUString& rStr ) override;
};

// AbstractSvxCaptionDialog_Impl
DECL_ABSTDLG_CLASS_ASYNC(AbstractSvxCaptionDialog,SvxCaptionTabDialog)
    virtual void SetValidateFramePosLink( const Link<SvxSwFrameValidation&,void>& rLink ) override;
    virtual void                SetCurPageId( const OUString& rName ) override;
    virtual const SfxItemSet*   GetOutputItemSet() const override;
    virtual WhichRangesContainer GetInputRanges( const SfxItemPool& pItem ) override;
    virtual void                SetInputSet( const SfxItemSet* pInSet ) override;
    virtual void        SetText( const OUString& rStr ) override;
};

// AbstractFmInputRecordNoDialog_Impl
DECL_ABSTDLG_CLASS(AbstractFmInputRecordNoDialog,FmInputRecordNoDialog)
    virtual void SetValue(tools::Long nNew) override ;
    virtual tools::Long GetValue() const override ;
};

// AbstractSvxNewDictionaryDialog_Impl
DECL_ABSTDLG_CLASS(AbstractSvxNewDictionaryDialog,SvxNewDictionaryDialog)
    virtual css::uno::Reference< css::linguistic2::XDictionary >  GetNewDictionary() override;
};

// AbstractSvxNameDialog_Impl
DECL_ABSTDLG_CLASS(AbstractSvxNameDialog,SvxNameDialog)
    virtual OUString GetName() override;
    virtual void    SetCheckNameHdl( const Link<AbstractSvxNameDialog&,bool>& rLink ) override ;
    virtual void    SetCheckNameTooltipHdl( const Link<AbstractSvxNameDialog&, OUString>& rLink ) override ;
    virtual void    SetEditHelpId(const OUString&) override ;
    //from class Window
    virtual void    SetHelpId( const OUString& ) override ;
    virtual void    SetText( const OUString& rStr ) override ;

private:
    Link<AbstractSvxNameDialog&,bool> aCheckNameHdl;
    Link<AbstractSvxNameDialog&,OUString> aCheckNameTooltipHdl;
    DECL_LINK(CheckNameHdl, SvxNameDialog&, bool);
    DECL_LINK(CheckNameTooltipHdl, SvxNameDialog&, OUString);
};

class SvxObjectNameDialog;
class SvxObjectTitleDescDialog;

// AbstractSvxObjectNameDialog_Impl
DECL_ABSTDLG_CLASS_ASYNC(AbstractSvxObjectNameDialog,SvxObjectNameDialog)
    virtual OUString GetName() override;
    virtual void SetCheckNameHdl(const Link<AbstractSvxObjectNameDialog&,bool>& rLink) override;

private:
    Link<AbstractSvxObjectNameDialog&,bool> aCheckNameHdl;
    DECL_LINK(CheckNameHdl, SvxObjectNameDialog&, bool);
};

// AbstractSvxObjectTitleDescDialog_Impl
DECL_ABSTDLG_CLASS_ASYNC(AbstractSvxObjectTitleDescDialog,SvxObjectTitleDescDialog)
    virtual OUString GetTitle() override;
    virtual OUString GetDescription() override;
    virtual bool IsDecorative() override;
};

// AbstractSvxMultiPathDialog_Impl
DECL_ABSTDLG_CLASS(AbstractSvxMultiPathDialog,SvxMultiPathDialog)
    virtual OUString        GetPath() const override;
    virtual void            SetPath( const OUString& rPath ) override;
    virtual void            SetTitle( const OUString& rNewTitle ) override;
};

// AbstractSvxPathSelectDialog_Impl
DECL_ABSTDLG_CLASS_UNIQUE(AbstractSvxPathSelectDialog,AbstractSvxMultiPathDialog,SvxPathSelectDialog)
    virtual OUString        GetPath() const override;
    virtual void            SetPath( const OUString& rPath ) override;
    virtual void            SetTitle( const OUString& rNewTitle ) override;
};

// AbstractSvxHpLinkDlg_Impl
DECL_ABSTDLG_CLASS_SHARED(AbstractSvxHpLinkDlg,AbstractSvxHpLinkDlg,SvxHpLinkDlg)
    virtual std::shared_ptr<SfxDialogController> GetController() override;
    virtual bool        QueryClose() override;
};

// AbstractFmSearchDialog_Impl
struct FmFoundRecordInformation;
DECL_ABSTDLG_CLASS(AbstractFmSearchDialog,FmSearchDialog)
    virtual void SetFoundHandler(const Link<FmFoundRecordInformation&,void>& lnk) override ;
    virtual void SetCanceledNotFoundHdl(const Link<FmFoundRecordInformation&,void>& lnk) override;
    virtual void SetActiveField(const OUString& strField) override;
};

// AbstractGraphicFilterDialog_Impl
DECL_ABSTDLG_CLASS_ASYNC(AbstractGraphicFilterDialog,GraphicFilterDialog)
    virtual Graphic GetFilteredGraphic( const Graphic& rGraphic, double fScaleX, double fScaleY ) override;
};

// AbstractSvxAreaTabDialog_Impl
DECL_ABSTDLG_CLASS_ASYNC(AbstractSvxAreaTabDialog,SvxAreaTabDialog)
    virtual void SetCurPageId(const OUString& rName) override;
    virtual const SfxItemSet* GetOutputItemSet() const override;
    virtual WhichRangesContainer GetInputRanges( const SfxItemPool& pItem ) override;
    virtual void SetInputSet(const SfxItemSet* pInSet) override;
    virtual void SetText(const OUString& rStr) override;
};

// AbstractInsertObjectDialog_Impl
DECL_ABSTDLG_CLASS_UNIQUE(AbstractInsertObjectDialog,SfxAbstractInsertObjectDialog,InsertObjectDialog_Impl)
    virtual css::uno::Reference < css::embed::XEmbeddedObject > GetObject() override;
    virtual css::uno::Reference< css::io::XInputStream > GetIconIfIconified( OUString* pGraphicMediaType ) override;
    virtual bool IsCreateNew() override;
};

// AbstractPasteDialog_Impl
DECL_ABSTDLG_CLASS_SHARED_ASYNC(AbstractPasteDialog,SfxAbstractPasteDialog,SvPasteObjectDialog)
    virtual void Insert( SotClipboardFormatId nFormat, const OUString & rFormatName ) override;
    virtual void InsertUno( const OUString & sCmd, const OUString& sLabel ) override;
    virtual void SetObjName( const SvGlobalName & rClass, const OUString & rObjName ) override;
    virtual void PreGetFormat( const TransferableDataHelper& aHelper ) override;
    virtual SotClipboardFormatId GetFormatOnly() override;
    virtual SotClipboardFormatId GetFormat( const TransferableDataHelper& aHelper ) override;
};

// AbstractLinksDialog_Impl
DECL_ABSTDLG_CLASS_SHARED_ASYNC(AbstractLinksDialog,SfxAbstractLinksDialog,SvBaseLinksDlg)
};

// AbstractSvxPostItDialog_Impl
DECL_ABSTDLG_CLASS(AbstractSvxPostItDialog,SvxPostItDialog)
    virtual void                SetText( const OUString& rStr ) override;  //From class Window
    virtual const SfxItemSet*   GetOutputItemSet() const override;
    virtual void                SetPrevHdl( const Link<AbstractSvxPostItDialog&,void>& rLink ) override;
    virtual void                SetNextHdl( const Link<AbstractSvxPostItDialog&,void>& rLink ) override;
    virtual void                EnableTravel(bool bNext, bool bPrev) override;
    virtual OUString            GetNote() override;
    virtual void                SetNote(const OUString& rTxt) override;
    virtual void                ShowLastAuthor(const OUString& rAuthor, const OUString& rDate) override;
    virtual void                DontChangeAuthor() override;
    virtual void                HideAuthor() override;
    virtual std::shared_ptr<weld::Dialog> GetDialog() override;
private:
    Link<AbstractSvxPostItDialog&,void> aNextHdl;
    Link<AbstractSvxPostItDialog&,void> aPrevHdl;
    DECL_LINK(NextHdl, SvxPostItDialog&, void);
    DECL_LINK(PrevHdl, SvxPostItDialog&, void);
};

// AbstractPasswordToOpenModifyDialog_Impl
DECL_ABSTDLG_CLASS_SHARED_ASYNC(AbstractPasswordToOpenModifyDialog,AbstractPasswordToOpenModifyDialog, PasswordToOpenModifyDialog)
    virtual OUString  GetPasswordToOpen() const override;
    virtual OUString  GetPasswordToModify() const override;
    virtual bool      IsRecommendToOpenReadonly() const override;
    virtual void      Response(sal_Int32) override;
    virtual void      AllowEmpty() override;
};

// AbstractSvxCharacterMapDialog_Impl
DECL_ABSTDLG_CLASS_SHARED_ASYNC(AbstractSvxCharacterMapDialog,SfxAbstractDialog,SvxCharacterMap)
    virtual const SfxItemSet* GetOutputItemSet() const override;
    virtual void  SetText(const OUString& rStr) override;
};

// AbstractScreenshotAnnotationDlg_Impl
DECL_ABSTDLG_CLASS(AbstractScreenshotAnnotationDlg,ScreenshotAnnotationDlg)
};

// AbstractSecurityOptionsDialog_Impl
DECL_ABSTDLG_CLASS(AbstractSecurityOptionsDialog, svx::SecurityOptionsDialog)
    virtual bool SetSecurityOptions() override;
};

// AbstractSignatureLineDialog_Impl
DECL_ABSTDLG_CLASS_ASYNC(AbstractSignatureLineDialog,SignatureLineDialog)
    virtual void Apply() override { m_xDlg->Apply(); }
};

// AbstractQrCodeGenDialog_Impl
DECL_ABSTDLG_CLASS_ASYNC(AbstractQrCodeGenDialog,QrCodeGenDialog)
};

// AbstractSignSignatureLineDialog_Impl
DECL_ABSTDLG_CLASS_ASYNC(AbstractSignSignatureLineDialog,SignSignatureLineDialog)
    virtual void Apply() override { m_xDlg->Apply(); }
};

// AbstractAdditionsDialog_Impl
DECL_ABSTDLG_CLASS_ASYNC(AbstractAdditionsDialog,weld::GenericDialogController)
};

// AbstractDiagramDialog_Impl
DECL_ABSTDLG_CLASS_ASYNC(AbstractDiagramDialog,DiagramDialog)
};

//AbstractDialogFactory_Impl implementations
class AbstractDialogFactory_Impl : public SvxAbstractDialogFactory
{
public:
    virtual VclPtr<VclAbstractDialog>    CreateVclDialog(weld::Window* pParent, sal_uInt32 nResId) override;

    virtual VclPtr<SfxAbstractDialog>    CreateSfxDialog( weld::Window* pParent,
                                            const SfxItemSet& rAttr,
                                            const SdrView* pView,
                                            sal_uInt32 nResId ) override;
    virtual VclPtr<SfxAbstractDialog>    CreateCharMapDialog(weld::Window* pParent,
                                                             const SfxItemSet& rAttr,
                                                             const css::uno::Reference< css::frame::XFrame >& rFrame) override;
    virtual VclPtr<SfxAbstractDialog>    CreateEventConfigDialog(weld::Widget* pParent,
                                                                 const SfxItemSet& rAttr,
                                                                 const css::uno::Reference< css::frame::XFrame >& rFrame) override;
    virtual VclPtr<VclAbstractDialog>    CreateFrameDialog(weld::Window* pParent, const css::uno::Reference< css::frame::XFrame >& rxFrame,
                                                           sal_uInt32 nResId, sal_uInt16 nPageId, const OUString& rParameter) override;
    virtual VclPtr<SfxAbstractTabDialog> CreateAutoCorrTabDialog(weld::Window* pParent, const SfxItemSet* pAttrSet) override;
    virtual VclPtr<SfxAbstractTabDialog> CreateCustomizeTabDialog(weld::Window* pParent,
                                            const SfxItemSet* pAttrSet,
                                            const css::uno::Reference< css::frame::XFrame >& xViewFrame ) override;
    virtual VclPtr<SfxAbstractTabDialog> CreateTextTabDialog( weld::Window* pParent,
                                            const SfxItemSet* pAttrSet,
                                            SdrView* pView ) override;
    virtual VclPtr<SfxAbstractTabDialog> CreateTabItemDialog(weld::Window* pParent, const SfxItemSet& rSet) override;
    virtual VclPtr<AbstractSvxCaptionDialog>
                                          CreateCaptionDialog(weld::Window* pParent,
                                            const SdrView* pView,
                                            SvxAnchorIds nAnchorTypes = SvxAnchorIds::NONE) override;
    virtual VclPtr<SfxAbstractInsertObjectDialog>
                                           CreateInsertObjectDialog(weld::Window* pParent, const OUString& rCommand,
                                            const css::uno::Reference < css::embed::XStorage >& xStor,
                                            const SvObjectServerList* pList ) override;
    virtual VclPtr<VclAbstractDialog>      CreateEditObjectDialog(weld::Window* pParent, const OUString& rCommand,
                                            const css::uno::Reference < css::embed::XEmbeddedObject >& xObj ) override;
    virtual VclPtr<SfxAbstractPasteDialog> CreatePasteDialog(weld::Window* pParent) override;
    virtual VclPtr<SfxAbstractLinksDialog> CreateLinksDialog(weld::Window* pParent, sfx2::LinkManager* pMgr, bool bHTML = false, sfx2::SvBaseLink* p=nullptr) override;

    virtual VclPtr<AbstractHangulHanjaConversionDialog> CreateHangulHanjaConversionDialog(weld::Widget* pParent) override;
    virtual VclPtr<AbstractThesaurusDialog>  CreateThesaurusDialog(weld::Widget*,
                                                css::uno::Reference<css::linguistic2::XThesaurus> xThesaurus,
                                                const OUString &rWord, LanguageType nLanguage) override;

    virtual VclPtr<AbstractHyphenWordDialog> CreateHyphenWordDialog(weld::Widget*,
                                                const OUString &rWord, LanguageType nLang,
                                                css::uno::Reference< css::linguistic2::XHyphenator >  &xHyphen,
                                                SvxSpellWrapper* pWrapper) override;

    virtual VclPtr<AbstractFmShowColsDialog> CreateFmShowColsDialog(weld::Window* pParent) override;
    virtual VclPtr<AbstractSvxZoomDialog> CreateSvxZoomDialog(weld::Window* pParent, const SfxItemSet& rCoreSet) override;
   // add for SvxBorderBackgroundDlg
    virtual VclPtr<SfxAbstractTabDialog> CreateSvxBorderBackgroundDlg(
       weld::Window* pParent,
       const SfxItemSet& rCoreSet,
       bool bEnableDrawingLayerFillStyles) override;

    virtual VclPtr<AbstractSvxTransformTabDialog> CreateSvxTransformTabDialog(weld::Window* pParent,
                                                                              const SfxItemSet* pAttr,
                                                                              const SdrView* pView,
                                                                              SvxAnchorIds nAnchorTypes = SvxAnchorIds::NONE) override ;
    virtual VclPtr<SfxAbstractTabDialog> CreateSchTransformTabDialog(weld::Window* pParent,
                                                                     const SfxItemSet* pAttr,
                                                                     const SdrView* pSdrView,
                                                                     bool bSizeTabPage) override;
    virtual VclPtr<AbstractSpellDialog>  CreateSvxSpellDialog(
                            weld::Window* pParent,
                            SfxBindings* pBindings,
                            svx::SpellDialogChildWindow* pSpellChildWindow ) override;

    virtual VclPtr<VclAbstractDialog> CreateActualizeProgressDialog(weld::Widget* pParent, GalleryTheme* pThm) override;
    virtual VclPtr<AbstractTitleDialog> CreateTitleDialog(weld::Widget* pParent, const OUString& rOldText) override;
    virtual VclPtr<AbstractGalleryIdDialog> CreateGalleryIdDialog(weld::Widget* pParent,
                                            GalleryTheme* pThm) override;
    virtual VclPtr<VclAbstractDialog> CreateGalleryThemePropertiesDialog(weld::Widget* pParent,
                                            ExchangeData* pData,
                                            SfxItemSet* pItemSet) override;

    virtual VclPtr<AbstractURLDlg> CreateURLDialog(weld::Widget* pParent,
                                            const OUString& rURL, const OUString& rAltText, const OUString& rDescription,
                                            const OUString& rTarget, const OUString& rName,
                                            TargetList& rTargetList ) override;

    virtual VclPtr<VclAbstractDialog>      CreateSvxSearchAttributeDialog(weld::Window* pParent,
                                            SearchAttrItemList& rLst,
                                            const WhichRangesContainer& pWhRanges) override;
    virtual VclPtr<AbstractSvxSearchSimilarityDialog> CreateSvxSearchSimilarityDialog( weld::Window* pParent,
                                                            bool bRelax,
                                                            sal_uInt16 nOther,
                                                            sal_uInt16 nShorter,
                                                            sal_uInt16 nLonger) override;
    virtual VclPtr<AbstractSvxJSearchOptionsDialog> CreateSvxJSearchOptionsDialog(weld::Window* pParent,
                                                                                  const SfxItemSet& rOptionsSet,
                                                                                  TransliterationFlags nInitialFlags) override;
    virtual VclPtr<AbstractFmInputRecordNoDialog> CreateFmInputRecordNoDialog(weld::Window* pParent) override;
    virtual VclPtr<AbstractSvxNewDictionaryDialog> CreateSvxNewDictionaryDialog(weld::Window* pParent) override;
    virtual VclPtr<VclAbstractDialog>     CreateSvxEditDictionaryDialog(weld::Window* pParent, const OUString& rName) override;
    virtual VclPtr<AbstractSvxNameDialog> CreateSvxNameDialog(weld::Window* pParent,
                                            const OUString& rName, const OUString& rDesc, const OUString& rTitle = u""_ustr) override;
    // #i68101#
    virtual VclPtr<AbstractSvxObjectNameDialog> CreateSvxObjectNameDialog(weld::Window* pParent, const OUString& rName) override;
    virtual VclPtr<AbstractSvxObjectTitleDescDialog> CreateSvxObjectTitleDescDialog(weld::Window* pParent, const OUString& rTitle, const OUString& rDescription, bool isDecorative) override;
    virtual VclPtr<AbstractSvxMultiPathDialog>    CreateSvxMultiPathDialog(weld::Window* pParent) override;
    virtual VclPtr<AbstractSvxMultiPathDialog>    CreateSvxPathSelectDialog(weld::Window* pParent) override;
    virtual VclPtr<AbstractSvxHpLinkDlg>          CreateSvxHpLinkDlg(SfxChildWindow* pChild, SfxBindings* pBindings, weld::Window* pParent) override;
    virtual VclPtr<AbstractFmSearchDialog>        CreateFmSearchDialog(weld::Window* pParent,
                                                        const OUString& strInitialText,
                                                        const std::vector< OUString >& _rContexts,
                                                        sal_Int16 nInitialContext,
                                                        const Link<FmSearchContext&,sal_uInt32>& lnkContextSupplier) override;
    virtual VclPtr<AbstractGraphicFilterDialog>   CreateGraphicFilterEmboss(weld::Window* pParent,
                                                const Graphic& rGraphic) override;
    virtual VclPtr<AbstractGraphicFilterDialog>   CreateGraphicFilterPoster(weld::Window* pParent,
                                                const Graphic& rGraphic) override;
    virtual VclPtr<AbstractGraphicFilterDialog>   CreateGraphicFilterSepia(weld::Window* pParent,
                                                const Graphic& rGraphic) override;
    virtual VclPtr<AbstractGraphicFilterDialog>   CreateGraphicFilterSmooth(weld::Window* pParent,
                                                const Graphic& rGraphic, double nRadius) override;
    virtual VclPtr<AbstractGraphicFilterDialog>  CreateGraphicFilterSolarize(weld::Window* pParent,
                                                const Graphic& rGraphic) override;
    virtual VclPtr<AbstractGraphicFilterDialog>   CreateGraphicFilterMosaic(weld::Window* pParent,
                                                const Graphic& rGraphic) override;
    virtual VclPtr<AbstractSvxAreaTabDialog>       CreateSvxAreaTabDialog(weld::Window* pParent,
                                                                          const SfxItemSet* pAttr,
                                                                          SdrModel* pModel,
                                                                          bool bShadow,
                                                                          bool bSlideBackground) override;
    virtual VclPtr<SfxAbstractTabDialog>           CreateSvxLineTabDialog(weld::Window* pParent, const SfxItemSet* pAttr,
                                                                 SdrModel* pModel,
                                                                 const SdrObject* pObj,
                                                                 bool bHasObj ) override;
    virtual VclPtr<AbstractSvxPostItDialog>        CreateSvxPostItDialog(weld::Widget* pParent,
                                                                         const SfxItemSet& rCoreSet,
                                                                         bool bPrevNext = false) override;

    // For TabPage
    virtual CreateTabPage               GetTabPageCreatorFunc( sal_uInt16 nId ) override;

    virtual GetTabPageRanges            GetTabPageRangesFunc( sal_uInt16 nId ) override;
    virtual DialogGetRanges             GetDialogGetRangesFunc() override;
    virtual VclPtr<VclAbstractDialog>   CreateSvxScriptOrgDialog(weld::Window* pParent, const OUString& rLanguage) override;

    virtual VclPtr<AbstractScriptSelectorDialog> CreateScriptSelectorDialog(weld::Window* pParent,
            const css::uno::Reference< css::frame::XFrame >& rxFrame) override;

    virtual void ShowAsyncScriptErrorDialog(weld::Window* pParent, const css::uno::Any& rException) override;

    virtual VclPtr<VclAbstractDialog>  CreateSvxMacroAssignDlg(
                weld::Window* _pParent,
                const css::uno::Reference< css::frame::XFrame >& _rxDocumentFrame,
                const bool _bUnoDialogMode,
                const css::uno::Reference< css::container::XNameReplace >& _rxEvents,
                const sal_uInt16 _nInitiallySelectedEvent
            ) override;

    virtual VclPtr<SfxAbstractTabDialog> CreateSvxFormatCellsDialog(weld::Window* pParent, const SfxItemSet& rAttr, const SdrModel& rModel, bool bStyle) override;

    virtual VclPtr<SvxAbstractSplitTableDialog> CreateSvxSplitTableDialog(weld::Window* pParent, bool bIsTableVertical, tools::Long nMaxVertical) override;

    virtual std::shared_ptr<SvxAbstractNewTableDialog> CreateSvxNewTableDialog(weld::Window* pParent) override ;

    virtual VclPtr<VclAbstractDialog>          CreateOptionsDialog(
        weld::Window* pParent, const OUString& rExtensionId ) override;

    virtual VclPtr<SvxAbstractInsRowColDlg> CreateSvxInsRowColDlg(weld::Window* pParent, bool bCol, const OUString& rHelpId) override;

    virtual VclPtr<AbstractPasswordToOpenModifyDialog> CreatePasswordToOpenModifyDialog(weld::Window* pParent, sal_uInt16 nMaxPasswdLen, bool bIsPasswordToModify) override;

    virtual VclPtr<AbstractScreenshotAnnotationDlg> CreateScreenshotAnnotationDlg(weld::Dialog& rParentDialog) override;

    virtual VclPtr<AbstractSecurityOptionsDialog> CreateSvxSecurityOptionsDialog(weld::Window* pParent) override;

    virtual VclPtr<AbstractSignatureLineDialog>
    CreateSignatureLineDialog(weld::Window* pParent,
                              const css::uno::Reference<css::frame::XModel> xModel, bool bEditExisting) override;

    virtual VclPtr<AbstractSignSignatureLineDialog>
    CreateSignSignatureLineDialog(weld::Window* pParent,
                                  const css::uno::Reference<css::frame::XModel> xModel) override;

    virtual VclPtr<AbstractQrCodeGenDialog>
    CreateQrCodeGenDialog(weld::Window* pParent,
                              const css::uno::Reference<css::frame::XModel> xModel, bool bEditExisting) override;

    virtual VclPtr<AbstractAdditionsDialog>
    CreateAdditionsDialog(weld::Window* pParent, const OUString& sAdditionsTag) override;

    virtual VclPtr<VclAbstractDialog> CreateAboutDialog(weld::Window* pParent) override;

    virtual VclPtr<VclAbstractDialog> CreateWhatsNewDialog(weld::Window* pParent, const bool bWelcome) override;

    virtual VclPtr<VclAbstractDialog> CreateTipOfTheDayDialog(weld::Window* pParent) override;

    virtual VclPtr<VclAbstractDialog> CreateWidgetTestDialog(weld::Window* pParent) override;

    virtual VclPtr<VclAbstractDialog> CreateToolbarmodeDialog(weld::Window* pParent) override;

    virtual VclPtr<AbstractDiagramDialog> CreateDiagramDialog(
        weld::Window* pParent,
        SdrObjGroup& rDiagram) override;

#ifdef _WIN32
    virtual VclPtr<VclAbstractDialog> CreateFileExtCheckDialog(weld::Window* pParent,
                                                               const OUString& sTitle,
                                                               const OUString& sMsg) override;
#endif
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
