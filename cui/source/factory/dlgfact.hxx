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
                                                                 std::unique_ptr<const SfxItemSet> xAttr,
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

    virtual VclPtr<AbstractMacroManagerDialog> CreateMacroManagerDialog(weld::Window* pParent,
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

    virtual VclPtr<VclAbstractDialog> CreateTipOfTheDayDialog(weld::Window* pParent) override;

    virtual VclPtr<VclAbstractDialog> CreateWidgetTestDialog(weld::Window* pParent) override;

    virtual VclPtr<VclAbstractDialog> CreateToolbarmodeDialog(weld::Window* pParent) override;

    virtual VclPtr<AbstractDiagramDialog> CreateDiagramDialog(
        weld::Window* pParent,
        SdrObjGroup& rDiagram) override;

    virtual VclPtr<AbstractQueryDialog> CreateQueryDialog(
        weld::Window* pParent,
        const OUString& sTitle, const OUString& sText, const OUString& sQuestion,
        bool bShowAgain) override;

#ifdef _WIN32
    virtual VclPtr<VclAbstractDialog> CreateFileExtCheckDialog(weld::Window* pParent,
                                                               const OUString& sTitle,
                                                               const OUString& sMsg) override;
#endif
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
