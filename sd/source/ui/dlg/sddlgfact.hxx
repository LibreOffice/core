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
#ifndef INCLUDED_SD_SOURCE_UI_DLG_SDDLGFACT_HXX
#define INCLUDED_SD_SOURCE_UI_DLG_SDDLGFACT_HXX

#include "sdabstdlg.hxx"
#include <sfx2/basedlgs.hxx>

#define DECL_ABSTDLG_BASE(Class,DialogClass)            \
    ScopedVclPtr<DialogClass> pDlg;                     \
public:                                                 \
    explicit Class(DialogClass* p)                      \
        : pDlg(p)                                       \
    {                                                   \
    }                                                   \
    virtual std::vector<OString> getAllPageUIXMLDescriptions() const override; \
    virtual bool selectPageByUIXMLDescription(const OString& rUIXMLDescription) override; \
    virtual Bitmap createScreenshot() const override;   \
    virtual OString GetScreenshotId() const override;   \
    virtual         ~Class();                           \
    virtual short   Execute() override ;

#define IMPL_ABSTDLG_BASE(Class)                    \
std::vector<OString> Class::getAllPageUIXMLDescriptions() const { return pDlg->getAllPageUIXMLDescriptions(); } \
bool Class::selectPageByUIXMLDescription(const OString& rUIXMLDescription) { return pDlg->selectPageByUIXMLDescription(rUIXMLDescription); } \
Bitmap Class::createScreenshot() const { return pDlg->createScreenshot();} \
OString Class::GetScreenshotId() const { return pDlg->GetScreenshotId();} \
Class::~Class()                                     \
{                                                   \
}                                                   \
short Class::Execute()                              \
{                                                   \
    return pDlg->Execute();                         \
}

namespace sd {
    class MorphDlg;
    class CopyDlg;
    class BreakDlg;
    class OutlineBulletDlg;
        class HeaderFooterDialog;
}

class Dialog;
class SdVclAbstractDialog_Impl : public VclAbstractDialog
{
    DECL_ABSTDLG_BASE(SdVclAbstractDialog_Impl,Dialog)
};

class AbstractCopyDlg_Impl : public AbstractCopyDlg
{
    DECL_ABSTDLG_BASE(AbstractCopyDlg_Impl,::sd::CopyDlg)
    virtual void    GetAttr( SfxItemSet& rOutAttrs ) override;
};

class SdCustomShowDlg;
class AbstractSdCustomShowDlg_Impl : public AbstractSdCustomShowDlg
{
    DECL_ABSTDLG_BASE(AbstractSdCustomShowDlg_Impl,SdCustomShowDlg)
    virtual bool        IsModified() const override ;
    virtual bool        IsCustomShow() const override ;
};

class SfxTabDialog;
class SdAbstractTabDialog_Impl : public SfxAbstractTabDialog
{
    DECL_ABSTDLG_BASE( SdAbstractTabDialog_Impl,SfxTabDialog )
    virtual void                SetCurPageId( sal_uInt16 nId ) override;
    virtual void                SetCurPageId( const OString& rName ) override;
    virtual const SfxItemSet*   GetOutputItemSet() const override;
    virtual const sal_uInt16*       GetInputRanges( const SfxItemPool& pItem ) override;
    virtual void                SetInputSet( const SfxItemSet* pInSet ) override;
        //From class Window.
    virtual void        SetText( const OUString& rStr ) override;
    virtual OUString    GetText() const override;
};

class SfxTabDialog;
class AbstractBulletDialog_Impl : public SfxAbstractTabDialog
{
    DECL_ABSTDLG_BASE( AbstractBulletDialog_Impl,SfxTabDialog )
    virtual void                SetCurPageId( sal_uInt16 nId ) override;
    virtual void                SetCurPageId( const OString& rName ) override;
    virtual const SfxItemSet*   GetOutputItemSet() const override;
    virtual const sal_uInt16*       GetInputRanges( const SfxItemPool& pItem ) override;
    virtual void                SetInputSet( const SfxItemSet* pInSet ) override;
        //From class Window.
    virtual void        SetText( const OUString& rStr ) override;
    virtual OUString    GetText() const override;
};

class SdPresLayoutTemplateDlg;
class SdPresLayoutTemplateDlg_Impl : public SfxAbstractTabDialog
{
    DECL_ABSTDLG_BASE( SdPresLayoutTemplateDlg_Impl,SdPresLayoutTemplateDlg )
    virtual void                SetCurPageId( sal_uInt16 nId ) override;
    virtual void                SetCurPageId( const OString& rName ) override;
    virtual const SfxItemSet*   GetOutputItemSet() const override;
    virtual const sal_uInt16*       GetInputRanges( const SfxItemPool& pItem ) override;
    virtual void                SetInputSet( const SfxItemSet* pInSet ) override;
        //From class Window.
    virtual void        SetText( const OUString& rStr ) override;
    virtual OUString    GetText() const override;
};

class SdModifyFieldDlg;
class AbstractSdModifyFieldDlg_Impl : public AbstractSdModifyFieldDlg
{
    DECL_ABSTDLG_BASE(AbstractSdModifyFieldDlg_Impl,SdModifyFieldDlg)
    virtual SvxFieldData*       GetField() override;
    virtual SfxItemSet          GetItemSet() override;
};

class SdSnapLineDlg;
class AbstractSdSnapLineDlg_Impl : public AbstractSdSnapLineDlg
{
    DECL_ABSTDLG_BASE(AbstractSdSnapLineDlg_Impl,SdSnapLineDlg)
    virtual void GetAttr(SfxItemSet& rOutAttrs) override;
    virtual void HideRadioGroup() override;
    virtual void HideDeleteBtn() override;
    virtual void SetInputFields(bool bEnableX, bool bEnableY) override;
    //from class Window
    virtual void    SetText( const OUString& rStr ) override;
};

class SdInsertLayerDlg;
class AbstractSdInsertLayerDlg_Impl : public AbstractSdInsertLayerDlg
{
    DECL_ABSTDLG_BASE(AbstractSdInsertLayerDlg_Impl,SdInsertLayerDlg)
    virtual void    GetAttr( SfxItemSet& rOutAttrs ) override ;
    //from class Window
    virtual void    SetHelpId( const OString& rHelpId ) override ;
};

class SdInsertPagesObjsDlg;
class AbstractSdInsertPagesObjsDlg_Impl : public AbstractSdInsertPagesObjsDlg
{
    DECL_ABSTDLG_BASE(AbstractSdInsertPagesObjsDlg_Impl,SdInsertPagesObjsDlg)
    virtual std::vector<OUString> GetList ( const sal_uInt16 nType ) override;
    virtual bool        IsLink() override;
    virtual bool        IsRemoveUnnessesaryMasterPages() const override;
};

class AbstractMorphDlg_Impl : public AbstractMorphDlg
{
    DECL_ABSTDLG_BASE(AbstractMorphDlg_Impl,::sd::MorphDlg)
    virtual void            SaveSettings() const override;
    virtual sal_uInt16      GetFadeSteps() const override;
    virtual bool            IsAttributeFade() const override ;
    virtual bool            IsOrientationFade() const override ;
};

class SdStartPresentationDlg;
class AbstractSdStartPresDlg_Impl : public AbstractSdStartPresDlg
{
    DECL_ABSTDLG_BASE(AbstractSdStartPresDlg_Impl,SdStartPresentationDlg)
    virtual void    GetAttr( SfxItemSet& rOutAttrs ) override;
};

class SdPresLayoutDlg;
class AbstractSdPresLayoutDlg_Impl : public AbstractSdPresLayoutDlg
{
    DECL_ABSTDLG_BASE(AbstractSdPresLayoutDlg_Impl,SdPresLayoutDlg)
    virtual void    GetAttr(SfxItemSet& rOutAttrs) override;
};

class SdAbstractSfxDialog_Impl : public SfxAbstractDialog
{
    DECL_ABSTDLG_BASE(SdAbstractSfxDialog_Impl,SfxModalDialog)
    virtual const SfxItemSet*   GetOutputItemSet() const override;
    virtual void        SetText( const OUString& rStr ) override;
    virtual OUString    GetText() const override;
};

class SdVectorizeDlg;
class AbstractSdVectorizeDlg_Impl :public AbstractSdVectorizeDlg
{
    DECL_ABSTDLG_BASE(AbstractSdVectorizeDlg_Impl,SdVectorizeDlg)
    virtual const GDIMetaFile&  GetGDIMetaFile() const override ;
};

class SdPublishingDlg;
class AbstractSdPublishingDlg_Impl :public AbstractSdPublishingDlg
{
    DECL_ABSTDLG_BASE(AbstractSdPublishingDlg_Impl,SdPublishingDlg)
    virtual void GetParameterSequence( css::uno::Sequence< css::beans::PropertyValue >& rParams ) override;
};

class AbstractHeaderFooterDialog_Impl :public AbstractHeaderFooterDialog
{
  DECL_ABSTDLG_BASE(AbstractHeaderFooterDialog_Impl,::sd::HeaderFooterDialog)
};

//AbstractDialogFactory_Impl implementations
class SdAbstractDialogFactory_Impl : public SdAbstractDialogFactory
{

public:
    virtual ~SdAbstractDialogFactory_Impl() {}

    virtual VclAbstractDialog*          CreateBreakDlg(vcl::Window* pWindow, ::sd::DrawView* pDrView, ::sd::DrawDocShell* pShell, sal_uLong nSumActionCount, sal_uLong nObjCount) override;
    virtual AbstractCopyDlg*            CreateCopyDlg(vcl::Window* pParent, const SfxItemSet& rInAttrs, const rtl::Reference<XColorList> &pColTab, ::sd::View* pView) override;
    virtual AbstractSdCustomShowDlg*    CreateSdCustomShowDlg(vcl::Window* pParent, SdDrawDocument& rDrawDoc) override;
    virtual SfxAbstractTabDialog*       CreateSdTabCharDialog(vcl::Window* pWindow, const SfxItemSet* pAttr, SfxObjectShell* pDocShell) override;
    virtual SfxAbstractTabDialog*       CreateSdTabPageDialog(vcl::Window* pWindow, const SfxItemSet* pAttr, SfxObjectShell* pDocShell, bool bAreaPage = true) override;
    virtual AbstractSdModifyFieldDlg*   CreateSdModifyFieldDlg( vcl::Window* pWindow, const SvxFieldData* pInField, const SfxItemSet& rSet ) override;
    virtual AbstractSdSnapLineDlg*      CreateSdSnapLineDlg(vcl::Window* pParent, const SfxItemSet& rInAttrs, ::sd::View* pView) override;
    virtual AbstractSdInsertLayerDlg*   CreateSdInsertLayerDlg(vcl::Window* pParent, const SfxItemSet& rInAttrs, bool bDeletable, const OUString& aStr) override;
    virtual AbstractSdInsertPagesObjsDlg* CreateSdInsertPagesObjsDlg(vcl::Window* pParent, const SdDrawDocument* pDoc, SfxMedium* pSfxMedium, const OUString& rFileName ) override;
    virtual AbstractMorphDlg*           CreateMorphDlg(vcl::Window* pParent, const SdrObject* pObj1, const SdrObject* pObj2) override;
    virtual SfxAbstractTabDialog*       CreateSdOutlineBulletTabDlg(vcl::Window* pParent, const SfxItemSet* pAttr, ::sd::View* pView = nullptr) override;
    virtual SfxAbstractTabDialog*       CreateSdParagraphTabDlg(vcl::Window* pParent, const SfxItemSet* pAttr) override;
    virtual AbstractSdStartPresDlg*     CreateSdStartPresentationDlg( vcl::Window* pWindow, const SfxItemSet& rInAttrs,
                                                                     const std::vector<OUString> &rPageNames, SdCustomShowList* pCSList ) override;
    virtual VclAbstractDialog*          CreateRemoteDialog( vcl::Window* pWindow ) override; // ad for RemoteDialog
    virtual SfxAbstractTabDialog*       CreateSdPresLayoutTemplateDlg( SfxObjectShell* pDocSh, vcl::Window* pParent, const SdResId& DlgId, SfxStyleSheetBase& rStyleBase, PresentationObjects ePO, SfxStyleSheetBasePool* pSSPool ) override;
    virtual AbstractSdPresLayoutDlg*    CreateSdPresLayoutDlg( ::sd::DrawDocShell* pDocShell, vcl::Window* pWindow, const SfxItemSet& rInAttrs) override;
    virtual SfxAbstractTabDialog*       CreateSdTabTemplateDlg(vcl::Window* pParent, const SfxObjectShell* pDocShell, SfxStyleSheetBase& rStyleBase, SdrModel* pModel, SdrView* pView ) override;
    virtual SfxAbstractDialog*          CreatSdActionDialog(vcl::Window* pParent, const SfxItemSet* pAttr, ::sd::View* pView) override;
    virtual AbstractSdVectorizeDlg*     CreateSdVectorizeDlg(vcl::Window* pParent, const Bitmap& rBmp, ::sd::DrawDocShell* pDocShell) override;
    virtual AbstractSdPublishingDlg*    CreateSdPublishingDlg(vcl::Window* pWindow, DocumentType eDocType) override;

    virtual VclAbstractDialog* CreateSdPhotoAlbumDialog(vcl::Window* pWindow, SdDrawDocument* pDoc) override;

      virtual VclAbstractDialog*          CreateMasterLayoutDialog( vcl::Window* pParent,
                                                                  SdDrawDocument* pDoc,
                                                                  SdPage* ) override;

    virtual AbstractHeaderFooterDialog* CreateHeaderFooterDialog( sd::ViewShell* pViewShell,
                                                                  vcl::Window* pParent,
                                                                  SdDrawDocument* pDoc,
                                                                  SdPage* pCurrentPage ) override;

    // For TabPage
    virtual CreateTabPage               GetSdOptionsContentsTabPageCreatorFunc() override;
    virtual CreateTabPage               GetSdPrintOptionsTabPageCreatorFunc() override;
    virtual CreateTabPage               GetSdOptionsMiscTabPageCreatorFunc() override;
    virtual CreateTabPage               GetSdOptionsSnapTabPageCreatorFunc() override;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
