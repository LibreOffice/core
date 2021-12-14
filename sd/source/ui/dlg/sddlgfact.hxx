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

#include <sdabstdlg.hxx>
#include <sfx2/basedlgs.hxx>
#include <sfx2/sfxdlg.hxx>
#include <svx/svxdlg.hxx>

#include <morphdlg.hxx>
#include <copydlg.hxx>
#include <BreakDlg.hxx>
#include <headerfooterdlg.hxx>
#include <masterlayoutdlg.hxx>
#include <custsdlg.hxx>
#include <layeroptionsdlg.hxx>
#include <inspagob.hxx>
#include <dlgfield.hxx>
#include <sdpreslt.hxx>
#include <prltempl.hxx>
#include <pubdlg.hxx>
#include <dlgsnap.hxx>
#include <present.hxx>
#include <vectdlg.hxx>
#include <BulletAndPositionDlg.hxx>

//namespace sd {
//    class MorphDlg;
//    class CopyDlg;
//    class BreakDlg;
//    class HeaderFooterDialog;
//    class MasterLayoutDialog;
//}

class SvxBulletAndPositionDlg;

/// Provides managing and getting information from the numbering and position dialog.
class AbstractSvxBulletAndPositionDlg_Impl :public AbstractSvxBulletAndPositionDlg
{
    std::unique_ptr<SvxBulletAndPositionDlg> m_xDlg;
public:
    explicit AbstractSvxBulletAndPositionDlg_Impl(std::unique_ptr<SvxBulletAndPositionDlg> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual const SfxItemSet*   GetOutputItemSet( SfxItemSet* ) const override ;
    virtual bool IsApplyToMaster() override;
    virtual bool IsSlideScope() override;
};

class SdAbstractGenericDialog_Impl : public VclAbstractDialog
{
    std::unique_ptr<weld::GenericDialogController> m_xDlg;
public:
    explicit SdAbstractGenericDialog_Impl(std::unique_ptr<weld::GenericDialogController> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;

    // screenshotting
    virtual BitmapEx createScreenshot() const override;
    virtual OString GetScreenshotId() const override;
};

class AbstractMasterLayoutDialog_Impl : public VclAbstractDialog
{
private:
    std::unique_ptr<sd::MasterLayoutDialog> m_xDlg;
public:
    AbstractMasterLayoutDialog_Impl(std::unique_ptr<::sd::MasterLayoutDialog> pDlg);
    virtual short Execute() override;

    // screenshotting
    virtual BitmapEx createScreenshot() const override;
    virtual OString GetScreenshotId() const override;
};

class AbstractBreakDlg_Impl : public VclAbstractDialog
{
private:
    std::unique_ptr<sd::BreakDlg> m_xDlg;
public:
    AbstractBreakDlg_Impl(std::unique_ptr<::sd::BreakDlg> pDlg);
    virtual short Execute() override;

    // screenshotting
    virtual BitmapEx createScreenshot() const override;
    virtual OString GetScreenshotId() const override;
};

class AbstractCopyDlg_Impl : public AbstractCopyDlg
{
private:
    std::unique_ptr<sd::CopyDlg> m_xDlg;
public:
    AbstractCopyDlg_Impl(std::unique_ptr<::sd::CopyDlg> pDlg)
        : m_xDlg(std::move(pDlg))
    {
    }
    virtual short Execute() override;
    virtual void    GetAttr( SfxItemSet& rOutAttrs ) override;

    // screenshotting
    virtual BitmapEx createScreenshot() const override;
    virtual OString GetScreenshotId() const override;
};

class AbstractSdCustomShowDlg_Impl : public AbstractSdCustomShowDlg
{
private:
    std::unique_ptr<SdCustomShowDlg> m_xDlg;
public:
    AbstractSdCustomShowDlg_Impl(std::unique_ptr<SdCustomShowDlg> pDlg)
        : m_xDlg(std::move(pDlg))
    {
    }
    virtual short       Execute() override;
    virtual bool        IsCustomShow() const override ;

    // screenshotting
    virtual BitmapEx createScreenshot() const override;
    virtual OString GetScreenshotId() const override;
};

class SdAbstractTabController_Impl : public SfxAbstractTabDialog
{
    std::shared_ptr<SfxTabDialogController> m_xDlg;
public:
    explicit SdAbstractTabController_Impl(std::shared_ptr<SfxTabDialogController> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual bool  StartExecuteAsync(AsyncContext &rCtx) override;
    virtual void                SetCurPageId( const OString &rName ) override;
    virtual const SfxItemSet*   GetOutputItemSet() const override;
    virtual WhichRangesContainer GetInputRanges( const SfxItemPool& pItem ) override;
    virtual void                SetInputSet( const SfxItemSet* pInSet ) override;
    virtual void        SetText( const OUString& rStr ) override;

    // screenshotting
    virtual BitmapEx createScreenshot() const override;
    virtual OString GetScreenshotId() const override;
};

class AbstractBulletDialog_Impl : public SfxAbstractTabDialog
{
    std::shared_ptr<SfxTabDialogController> m_xDlg;
public:
    explicit AbstractBulletDialog_Impl(std::shared_ptr<SfxTabDialogController> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual bool  StartExecuteAsync(AsyncContext &rCtx) override;
    virtual void                SetCurPageId( const OString& rName ) override;
    virtual const SfxItemSet*   GetOutputItemSet() const override;
    virtual WhichRangesContainer GetInputRanges( const SfxItemPool& pItem ) override;
    virtual void                SetInputSet( const SfxItemSet* pInSet ) override;
    virtual void        SetText( const OUString& rStr ) override;

    // screenshotting
    virtual BitmapEx createScreenshot() const override;
    virtual OString GetScreenshotId() const override;
};

class SdPresLayoutTemplateDlg_Impl : public SfxAbstractTabDialog
{
    std::shared_ptr<SdPresLayoutTemplateDlg> m_xDlg;
public:
    explicit SdPresLayoutTemplateDlg_Impl(std::shared_ptr<SdPresLayoutTemplateDlg> p)
        : m_xDlg(std::move(p))
    {
    }
    virtual short Execute() override;
    virtual bool  StartExecuteAsync(AsyncContext &rCtx) override;
    virtual void                SetCurPageId( const OString& rName ) override;
    virtual const SfxItemSet*   GetOutputItemSet() const override;
    virtual WhichRangesContainer GetInputRanges( const SfxItemPool& pItem ) override;
    virtual void                SetInputSet( const SfxItemSet* pInSet ) override;
    virtual void        SetText( const OUString& rStr ) override;

    // screenshotting
    virtual BitmapEx createScreenshot() const override;
    virtual OString GetScreenshotId() const override;
};

class AbstractSdModifyFieldDlg_Impl : public AbstractSdModifyFieldDlg
{
private:
    std::unique_ptr<SdModifyFieldDlg> m_xDlg;
public:
    AbstractSdModifyFieldDlg_Impl(std::unique_ptr<SdModifyFieldDlg> pDlg)
        : m_xDlg(std::move(pDlg))
    {
    }
    virtual short Execute() override;
    virtual SvxFieldData*       GetField() override;
    virtual SfxItemSet          GetItemSet() override;

    // screenshotting
    virtual BitmapEx createScreenshot() const override;
    virtual OString GetScreenshotId() const override;
};

class AbstractSdSnapLineDlg_Impl : public AbstractSdSnapLineDlg
{
private:
    std::unique_ptr<SdSnapLineDlg> m_xDlg;
public:
    AbstractSdSnapLineDlg_Impl(std::unique_ptr<SdSnapLineDlg> pDlg)
        : m_xDlg(std::move(pDlg))
    {
    }
    virtual short Execute() override;
    virtual void GetAttr(SfxItemSet& rOutAttrs) override;
    virtual void HideRadioGroup() override;
    virtual void HideDeleteBtn() override;
    virtual void SetInputFields(bool bEnableX, bool bEnableY) override;
    virtual void SetText( const OUString& rStr ) override;

    // screenshotting
    virtual BitmapEx createScreenshot() const override;
    virtual OString GetScreenshotId() const override;
};

class AbstractSdInsertLayerDlg_Impl : public AbstractSdInsertLayerDlg
{
private:
    std::unique_ptr<SdInsertLayerDlg> m_xDlg;
public:
    AbstractSdInsertLayerDlg_Impl(std::unique_ptr<SdInsertLayerDlg> pDlg)
        : m_xDlg(std::move(pDlg))
    {
    }
    virtual short   Execute() override;
    virtual void    GetAttr( SfxItemSet& rOutAttrs ) override ;
    virtual void    SetHelpId( const OString& rHelpId ) override ;

    // screenshotting
    virtual BitmapEx createScreenshot() const override;
    virtual OString GetScreenshotId() const override;
};

class AbstractSdInsertPagesObjsDlg_Impl : public AbstractSdInsertPagesObjsDlg
{
private:
    std::unique_ptr<SdInsertPagesObjsDlg> m_xDlg;
public:
    AbstractSdInsertPagesObjsDlg_Impl(std::unique_ptr<SdInsertPagesObjsDlg> pDlg)
        : m_xDlg(std::move(pDlg))
    {
    }
    virtual short   Execute() override;
    virtual std::vector<OUString> GetList ( const sal_uInt16 nType ) override;
    virtual bool        IsLink() override;
    virtual bool        IsRemoveUnnecessaryMasterPages() const override;

    // screenshotting
    virtual BitmapEx createScreenshot() const override;
    virtual OString GetScreenshotId() const override;
};

class AbstractMorphDlg_Impl : public AbstractMorphDlg
{
private:
    std::unique_ptr<sd::MorphDlg> m_xDlg;
public:
    AbstractMorphDlg_Impl(std::unique_ptr<::sd::MorphDlg> pDlg)
        : m_xDlg(std::move(pDlg))
    {
    }
    virtual short   Execute() override;
    virtual void            SaveSettings() const override;
    virtual sal_uInt16      GetFadeSteps() const override;
    virtual bool            IsAttributeFade() const override ;
    virtual bool            IsOrientationFade() const override ;

    // screenshotting
    virtual BitmapEx createScreenshot() const override;
    virtual OString GetScreenshotId() const override;
};

class AbstractSdStartPresDlg_Impl : public AbstractSdStartPresDlg
{
private:
    std::unique_ptr<SdStartPresentationDlg> m_xDlg;
public:
    AbstractSdStartPresDlg_Impl(std::unique_ptr<SdStartPresentationDlg> pDlg)
        : m_xDlg(std::move(pDlg))
    {
    }
    virtual short   Execute() override;
    virtual void    GetAttr( SfxItemSet& rOutAttrs ) override;

    // screenshotting
    virtual BitmapEx createScreenshot() const override;
    virtual OString GetScreenshotId() const override;
};

class AbstractSdPresLayoutDlg_Impl : public AbstractSdPresLayoutDlg
{
private:
    std::unique_ptr<SdPresLayoutDlg> m_xDlg;
public:
    AbstractSdPresLayoutDlg_Impl(std::unique_ptr<SdPresLayoutDlg> pDlg)
        : m_xDlg(std::move(pDlg))
    {
    }
    virtual short   Execute() override;
    virtual void    GetAttr(SfxItemSet& rOutAttrs) override;

    // screenshotting
    virtual BitmapEx createScreenshot() const override;
    virtual OString GetScreenshotId() const override;
};

class SdAbstractSfxDialog_Impl : public SfxAbstractDialog
{
private:
    std::unique_ptr<SfxSingleTabDialogController> m_xDlg;
public:
    SdAbstractSfxDialog_Impl(std::unique_ptr<SfxSingleTabDialogController> pDlg)
        : m_xDlg(std::move(pDlg))
    {
    }
    virtual short   Execute() override;
    virtual const SfxItemSet*   GetOutputItemSet() const override;
    virtual void    SetText( const OUString& rStr ) override;
};

class AbstractSdVectorizeDlg_Impl :public AbstractSdVectorizeDlg
{
private:
    std::unique_ptr<SdVectorizeDlg> m_xDlg;
public:
    AbstractSdVectorizeDlg_Impl(std::unique_ptr<SdVectorizeDlg> pDlg)
        : m_xDlg(std::move(pDlg))
    {
    }
    virtual short Execute() override;
    virtual const GDIMetaFile&  GetGDIMetaFile() const override ;

    // screenshotting
    virtual BitmapEx createScreenshot() const override;
    virtual OString GetScreenshotId() const override;
};

class AbstractSdPublishingDlg_Impl :public AbstractSdPublishingDlg
{
private:
    std::unique_ptr<SdPublishingDlg> m_xDlg;
public:
    AbstractSdPublishingDlg_Impl(std::unique_ptr<SdPublishingDlg> pDlg)
        : m_xDlg(std::move(pDlg))
    {
    }
    virtual short Execute() override;
    virtual void GetParameterSequence( css::uno::Sequence< css::beans::PropertyValue >& rParams ) override;

    // screenshotting
    virtual BitmapEx createScreenshot() const override;
    virtual OString GetScreenshotId() const override;
};

class AbstractHeaderFooterDialog_Impl :public AbstractHeaderFooterDialog
{
private:
    std::shared_ptr<::sd::HeaderFooterDialog> m_xDlg;
public:
    AbstractHeaderFooterDialog_Impl(std::shared_ptr<::sd::HeaderFooterDialog> pDlg)
        : m_xDlg(std::move(pDlg))
    {
    }
    virtual short Execute() override;
    virtual bool  StartExecuteAsync(AsyncContext &rCtx) override;

    // screenshotting
    virtual BitmapEx createScreenshot() const override;
    virtual OString GetScreenshotId() const override;
};

//AbstractDialogFactory_Impl implementations
class SdAbstractDialogFactory_Impl : public SdAbstractDialogFactory
{

public:
    virtual ~SdAbstractDialogFactory_Impl() {}

    virtual VclPtr<AbstractSvxBulletAndPositionDlg> CreateSvxBulletAndPositionDlg(weld::Window* pParent, const SfxItemSet* pAttr, ::sd::View* pView) override;
    virtual VclPtr<VclAbstractDialog>          CreateBreakDlg(weld::Window* pWindow, ::sd::DrawView* pDrView, ::sd::DrawDocShell* pShell, sal_uLong nSumActionCount, sal_uLong nObjCount) override;
    virtual VclPtr<AbstractCopyDlg>            CreateCopyDlg(weld::Window* pParent, const SfxItemSet& rInAttrs, ::sd::View* pView) override;
    virtual VclPtr<AbstractSdCustomShowDlg>    CreateSdCustomShowDlg(weld::Window* pParent, SdDrawDocument& rDrawDoc) override;
    virtual VclPtr<SfxAbstractTabDialog>       CreateSdTabCharDialog(weld::Window* pWindow, const SfxItemSet* pAttr, SfxObjectShell* pDocShell) override;
    virtual VclPtr<SfxAbstractTabDialog>       CreateSdTabPageDialog(weld::Window* pWindow, const SfxItemSet* pAttr, SfxObjectShell* pDocShell, bool bAreaPage, bool bIsImpressDoc, bool bIsImpressMaster) override;
    virtual VclPtr<AbstractSdModifyFieldDlg>   CreateSdModifyFieldDlg(weld::Window* pWindow, const SvxFieldData* pInField, const SfxItemSet& rSet) override;
    virtual VclPtr<AbstractSdSnapLineDlg>      CreateSdSnapLineDlg(weld::Window* pParent, const SfxItemSet& rInAttrs, ::sd::View* pView) override;
    virtual VclPtr<AbstractSdInsertLayerDlg>   CreateSdInsertLayerDlg(weld::Window* pParent, const SfxItemSet& rInAttrs, bool bDeletable, const OUString& aStr) override;
    virtual VclPtr<AbstractSdInsertPagesObjsDlg> CreateSdInsertPagesObjsDlg(weld::Window* pParent, const SdDrawDocument* pDoc, SfxMedium* pSfxMedium, const OUString& rFileName ) override;
    virtual VclPtr<AbstractMorphDlg>           CreateMorphDlg(weld::Window* pParent, const SdrObject* pObj1, const SdrObject* pObj2) override;
    virtual VclPtr<SfxAbstractTabDialog>       CreateSdOutlineBulletTabDlg(weld::Window* pParent, const SfxItemSet* pAttr, ::sd::View* pView) override;
    virtual VclPtr<SfxAbstractTabDialog>       CreateSdParagraphTabDlg(weld::Window* pParent, const SfxItemSet* pAttr) override;
    virtual VclPtr<AbstractSdStartPresDlg>     CreateSdStartPresentationDlg(weld::Window* pWindow, const SfxItemSet& rInAttrs,
                                                                     const std::vector<OUString> &rPageNames, SdCustomShowList* pCSList ) override;
    virtual VclPtr<VclAbstractDialog>          CreateRemoteDialog(weld::Window* pWindow) override; // add for RemoteDialog
    virtual VclPtr<SfxAbstractTabDialog>       CreateSdPresLayoutTemplateDlg(SfxObjectShell* pDocSh, weld::Window* pParent, bool bBackgroundDlg, SfxStyleSheetBase& rStyleBase, PresentationObjects ePO, SfxStyleSheetBasePool* pSSPool) override;
    virtual VclPtr<AbstractSdPresLayoutDlg>    CreateSdPresLayoutDlg(weld::Window* pParent, ::sd::DrawDocShell* pDocShell, const SfxItemSet& rInAttrs) override;
    virtual VclPtr<SfxAbstractTabDialog>       CreateSdTabTemplateDlg(weld::Window* pParent, const SfxObjectShell* pDocShell, SfxStyleSheetBase& rStyleBase, SdrModel* pModel, SdrView* pView ) override;
    virtual VclPtr<SfxAbstractDialog>          CreatSdActionDialog(weld::Window* pParent, const SfxItemSet* pAttr, ::sd::View* pView) override;
    virtual VclPtr<AbstractSdVectorizeDlg>     CreateSdVectorizeDlg(weld::Window* pParent, const Bitmap& rBmp, ::sd::DrawDocShell* pDocShell) override;
    virtual VclPtr<AbstractSdPublishingDlg>    CreateSdPublishingDlg(weld::Window* pWindow, DocumentType eDocType) override;

    virtual VclPtr<VclAbstractDialog>          CreateSdPhotoAlbumDialog(weld::Window* pWindow, SdDrawDocument* pDoc) override;

    virtual VclPtr<VclAbstractDialog>          CreateMasterLayoutDialog(weld::Window* pParent, SdDrawDocument* pDoc, SdPage*) override;

    virtual VclPtr<AbstractHeaderFooterDialog> CreateHeaderFooterDialog(sd::ViewShell* pViewShell,
                                                                        weld::Window* pParent,
                                                                        SdDrawDocument* pDoc,
                                                                        SdPage* pCurrentPage) override;

    // For TabPage
    virtual CreateTabPage               GetSdOptionsContentsTabPageCreatorFunc() override;
    virtual CreateTabPage               GetSdPrintOptionsTabPageCreatorFunc() override;
    virtual CreateTabPage               GetSdOptionsMiscTabPageCreatorFunc() override;
    virtual CreateTabPage               GetSdOptionsSnapTabPageCreatorFunc() override;

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
