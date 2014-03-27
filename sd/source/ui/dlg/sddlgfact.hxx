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

#define DECL_ABSTDLG_BASE(Class,DialogClass)        \
    DialogClass*        pDlg;                       \
public:                                             \
                    Class( DialogClass* p)          \
                     : pDlg(p)                      \
                     {}                             \
    virtual         ~Class();                       \
    virtual short   Execute() SAL_OVERRIDE ;

#define IMPL_ABSTDLG_BASE(Class)                    \
Class::~Class()                                     \
{                                                   \
    delete pDlg;                                    \
}                                                   \
short Class::Execute()                             \
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
    virtual void    GetAttr( SfxItemSet& rOutAttrs ) SAL_OVERRIDE;
};

class SdCustomShowDlg;
class AbstractSdCustomShowDlg_Impl : public AbstractSdCustomShowDlg
{
    DECL_ABSTDLG_BASE(AbstractSdCustomShowDlg_Impl,SdCustomShowDlg)
    virtual sal_Bool        IsModified() const SAL_OVERRIDE ;
    virtual sal_Bool        IsCustomShow() const SAL_OVERRIDE ;
};

class SfxTabDialog;
class SdAbstractTabDialog_Impl : public SfxAbstractTabDialog
{
    DECL_ABSTDLG_BASE( SdAbstractTabDialog_Impl,SfxTabDialog )
    virtual void                SetCurPageId( sal_uInt16 nId ) SAL_OVERRIDE;
    virtual void                SetCurPageId( const OString& rName ) SAL_OVERRIDE;
    virtual const SfxItemSet*   GetOutputItemSet() const SAL_OVERRIDE;
    virtual const sal_uInt16*       GetInputRanges( const SfxItemPool& pItem ) SAL_OVERRIDE;
    virtual void                SetInputSet( const SfxItemSet* pInSet ) SAL_OVERRIDE;
        //From class Window.
    virtual void        SetText( const OUString& rStr ) SAL_OVERRIDE;
    virtual OUString    GetText() const SAL_OVERRIDE;
};

class SfxTabDialog;
class AbstractBulletDialog_Impl : public SfxAbstractTabDialog
{
    DECL_ABSTDLG_BASE( AbstractBulletDialog_Impl,SfxTabDialog )
    virtual void                SetCurPageId( sal_uInt16 nId ) SAL_OVERRIDE;
    virtual void                SetCurPageId( const OString& rName ) SAL_OVERRIDE;
    virtual const SfxItemSet*   GetOutputItemSet() const SAL_OVERRIDE;
    virtual const sal_uInt16*       GetInputRanges( const SfxItemPool& pItem ) SAL_OVERRIDE;
    virtual void                SetInputSet( const SfxItemSet* pInSet ) SAL_OVERRIDE;
        //From class Window.
    virtual void        SetText( const OUString& rStr ) SAL_OVERRIDE;
    virtual OUString    GetText() const SAL_OVERRIDE;
};

class SdPresLayoutTemplateDlg;
class SdPresLayoutTemplateDlg_Impl : public SfxAbstractTabDialog
{
    DECL_ABSTDLG_BASE( SdPresLayoutTemplateDlg_Impl,SdPresLayoutTemplateDlg )
    virtual void                SetCurPageId( sal_uInt16 nId ) SAL_OVERRIDE;
    virtual void                SetCurPageId( const OString& rName ) SAL_OVERRIDE;
    virtual const SfxItemSet*   GetOutputItemSet() const SAL_OVERRIDE;
    virtual const sal_uInt16*       GetInputRanges( const SfxItemPool& pItem ) SAL_OVERRIDE;
    virtual void                SetInputSet( const SfxItemSet* pInSet ) SAL_OVERRIDE;
        //From class Window.
    virtual void        SetText( const OUString& rStr ) SAL_OVERRIDE;
    virtual OUString    GetText() const SAL_OVERRIDE;
};

class AssistentDlg;
class AbstractAssistentDlg_Impl : public AbstractAssistentDlg
{
    DECL_ABSTDLG_BASE(AbstractAssistentDlg_Impl,AssistentDlg)
    virtual SfxObjectShellLock GetDocument() SAL_OVERRIDE;
    virtual OutputType GetOutputMedium() const SAL_OVERRIDE;
    virtual sal_Bool IsSummary() const SAL_OVERRIDE;
    virtual StartType GetStartType() const SAL_OVERRIDE;
    virtual OUString GetDocPath() const SAL_OVERRIDE;
    virtual sal_Bool GetStartWithFlag() const SAL_OVERRIDE;
    virtual sal_Bool IsDocEmpty() const SAL_OVERRIDE;
    virtual com::sun::star::uno::Sequence< com::sun::star::beans::NamedValue > GetPassword() SAL_OVERRIDE;
};

class SdModifyFieldDlg;
class AbstractSdModifyFieldDlg_Impl : public AbstractSdModifyFieldDlg
{
    DECL_ABSTDLG_BASE(AbstractSdModifyFieldDlg_Impl,SdModifyFieldDlg)
    virtual SvxFieldData*       GetField() SAL_OVERRIDE;
    virtual SfxItemSet          GetItemSet() SAL_OVERRIDE;
};

class SdSnapLineDlg;
class AbstractSdSnapLineDlg_Impl : public AbstractSdSnapLineDlg
{
    DECL_ABSTDLG_BASE(AbstractSdSnapLineDlg_Impl,SdSnapLineDlg)
    virtual void GetAttr(SfxItemSet& rOutAttrs) SAL_OVERRIDE;
    virtual void HideRadioGroup() SAL_OVERRIDE;
    virtual void HideDeleteBtn() SAL_OVERRIDE;
    virtual void SetInputFields(sal_Bool bEnableX, sal_Bool bEnableY) SAL_OVERRIDE;
    //from class Window
    virtual void    SetText( const OUString& rStr ) SAL_OVERRIDE;
};

class SdInsertLayerDlg;
class AbstractSdInsertLayerDlg_Impl : public AbstractSdInsertLayerDlg
{
    DECL_ABSTDLG_BASE(AbstractSdInsertLayerDlg_Impl,SdInsertLayerDlg)
    virtual void    GetAttr( SfxItemSet& rOutAttrs ) SAL_OVERRIDE ;
    //from class Window
    virtual void    SetHelpId( const OString& rHelpId ) SAL_OVERRIDE ;
};

class SdInsertPasteDlg;
class AbstractSdInsertPasteDlg_Impl : public AbstractSdInsertPasteDlg
{
    DECL_ABSTDLG_BASE(AbstractSdInsertPasteDlg_Impl,SdInsertPasteDlg)
    virtual sal_Bool            IsInsertBefore() const SAL_OVERRIDE;
};

class SdInsertPagesObjsDlg;
class AbstractSdInsertPagesObjsDlg_Impl : public AbstractSdInsertPagesObjsDlg
{
    DECL_ABSTDLG_BASE(AbstractSdInsertPagesObjsDlg_Impl,SdInsertPagesObjsDlg)
    virtual ::Window *  GetWindow() SAL_OVERRIDE;
    virtual std::vector<OUString> GetList ( const sal_uInt16 nType ) SAL_OVERRIDE;
    virtual sal_Bool        IsLink() SAL_OVERRIDE;
    virtual sal_Bool        IsRemoveUnnessesaryMasterPages() const SAL_OVERRIDE;
};

class AbstractMorphDlg_Impl : public AbstractMorphDlg
{
    DECL_ABSTDLG_BASE(AbstractMorphDlg_Impl,::sd::MorphDlg)
    virtual void            SaveSettings() const SAL_OVERRIDE;
    virtual sal_uInt16          GetFadeSteps() const SAL_OVERRIDE;
    virtual sal_Bool            IsAttributeFade() const SAL_OVERRIDE ;
    virtual sal_Bool            IsOrientationFade() const SAL_OVERRIDE ;
};

class SdStartPresentationDlg;
class AbstractSdStartPresDlg_Impl : public AbstractSdStartPresDlg
{
    DECL_ABSTDLG_BASE(AbstractSdStartPresDlg_Impl,SdStartPresentationDlg)
    virtual void    GetAttr( SfxItemSet& rOutAttrs ) SAL_OVERRIDE;
};

class SdPresLayoutDlg;
class AbstractSdPresLayoutDlg_Impl : public AbstractSdPresLayoutDlg
{
    DECL_ABSTDLG_BASE(AbstractSdPresLayoutDlg_Impl,SdPresLayoutDlg)
    virtual void    GetAttr(SfxItemSet& rOutAttrs) SAL_OVERRIDE;
};

class SdAbstractSfxDialog_Impl : public SfxAbstractDialog
{
    DECL_ABSTDLG_BASE(SdAbstractSfxDialog_Impl,SfxModalDialog)
    virtual const SfxItemSet*   GetOutputItemSet() const SAL_OVERRIDE;
    virtual void        SetText( const OUString& rStr ) SAL_OVERRIDE;
    virtual OUString    GetText() const SAL_OVERRIDE;
};

class SdVectorizeDlg;
class AbstractSdVectorizeDlg_Impl :public AbstractSdVectorizeDlg
{
    DECL_ABSTDLG_BASE(AbstractSdVectorizeDlg_Impl,SdVectorizeDlg)
    virtual const GDIMetaFile&  GetGDIMetaFile() const SAL_OVERRIDE ;
};

class SdPublishingDlg;
class AbstractSdPublishingDlg_Impl :public AbstractSdPublishingDlg
{
    DECL_ABSTDLG_BASE(AbstractSdPublishingDlg_Impl,SdPublishingDlg)
    virtual void GetParameterSequence( ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rParams ) SAL_OVERRIDE;
};

class AbstractHeaderFooterDialog_Impl :public AbstractHeaderFooterDialog
{
  DECL_ABSTDLG_BASE(AbstractHeaderFooterDialog_Impl,::sd::HeaderFooterDialog)
  virtual void ApplyToAll() SAL_OVERRIDE;
  virtual void Apply() SAL_OVERRIDE;
  virtual void Cancel() SAL_OVERRIDE;
};

//AbstractDialogFactory_Impl implementations
class SdAbstractDialogFactory_Impl : public SdAbstractDialogFactory
{

public:
    virtual ~SdAbstractDialogFactory_Impl() {}

    virtual VclAbstractDialog*          CreateBreakDlg(::Window* pWindow, ::sd::DrawView* pDrView, ::sd::DrawDocShell* pShell, sal_uLong nSumActionCount, sal_uLong nObjCount ) SAL_OVERRIDE;
    virtual AbstractCopyDlg*            CreateCopyDlg( ::Window* pWindow, const SfxItemSet& rInAttrs, const rtl::Reference<XColorList> &pColTab, ::sd::View* pView ) SAL_OVERRIDE;
    virtual AbstractSdCustomShowDlg*    CreateSdCustomShowDlg( ::Window* pWindow, SdDrawDocument& rDrawDoc ) SAL_OVERRIDE;
    virtual SfxAbstractTabDialog*       CreateSdTabCharDialog( ::Window* pParent, const SfxItemSet* pAttr, SfxObjectShell* pDocShell ) SAL_OVERRIDE;
    virtual SfxAbstractTabDialog*       CreateSdTabPageDialog( ::Window* pParent, const SfxItemSet* pAttr, SfxObjectShell* pDocShell, sal_Bool bAreaPage = sal_True ) SAL_OVERRIDE;
    virtual AbstractAssistentDlg*       CreateAssistentDlg( ::Window* pParent, sal_Bool bAutoPilot) SAL_OVERRIDE;
    virtual AbstractSdModifyFieldDlg*   CreateSdModifyFieldDlg( ::Window* pWindow, const SvxFieldData* pInField, const SfxItemSet& rSet ) SAL_OVERRIDE;
    virtual AbstractSdSnapLineDlg*      CreateSdSnapLineDlg( ::Window* pWindow, const SfxItemSet& rInAttrs, ::sd::View* pView) SAL_OVERRIDE;
    virtual AbstractSdInsertLayerDlg*   CreateSdInsertLayerDlg( ::Window* pWindow, const SfxItemSet& rInAttrs, bool bDeletable, const OUString& aStr ) SAL_OVERRIDE;
    virtual AbstractSdInsertPasteDlg*   CreateSdInsertPasteDlg( ::Window* pWindow ) SAL_OVERRIDE;
    virtual AbstractSdInsertPagesObjsDlg* CreateSdInsertPagesObjsDlg( ::Window* pParent, const SdDrawDocument* pDoc, SfxMedium* pSfxMedium, const OUString& rFileName ) SAL_OVERRIDE;
    virtual AbstractMorphDlg*           CreateMorphDlg( ::Window* pParent, const SdrObject* pObj1, const SdrObject* pObj2) SAL_OVERRIDE;
    virtual SfxAbstractTabDialog*       CreateSdOutlineBulletTabDlg ( ::Window* pParent, const SfxItemSet* pAttr, ::sd::View* pView = NULL ) SAL_OVERRIDE;
    virtual SfxAbstractTabDialog*       CreateSdParagraphTabDlg ( ::Window* pParent, const SfxItemSet* pAttr ) SAL_OVERRIDE;
    virtual AbstractSdStartPresDlg*     CreateSdStartPresentationDlg( ::Window* pWindow, const SfxItemSet& rInAttrs,
                                                                     const std::vector<OUString> &rPageNames, SdCustomShowList* pCSList ) SAL_OVERRIDE;
    virtual VclAbstractDialog*          CreateRemoteDialog( ::Window* pWindow ) SAL_OVERRIDE; // ad for RemoteDialog
    virtual SfxAbstractTabDialog*       CreateSdPresLayoutTemplateDlg( SfxObjectShell* pDocSh, ::Window* pParent, SdResId DlgId, SfxStyleSheetBase& rStyleBase, PresentationObjects ePO, SfxStyleSheetBasePool* pSSPool ) SAL_OVERRIDE;
    virtual AbstractSdPresLayoutDlg*    CreateSdPresLayoutDlg( ::sd::DrawDocShell* pDocShell, ::Window* pWindow, const SfxItemSet& rInAttrs) SAL_OVERRIDE;
    virtual SfxAbstractTabDialog*       CreateSdTabTemplateDlg( ::Window* pParent, const SfxObjectShell* pDocShell, SfxStyleSheetBase& rStyleBase, SdrModel* pModel, SdrView* pView ) SAL_OVERRIDE;
    virtual SfxAbstractDialog*          CreatSdActionDialog( ::Window* pParent, const SfxItemSet* pAttr, ::sd::View* pView ) SAL_OVERRIDE;
    virtual AbstractSdVectorizeDlg*     CreateSdVectorizeDlg( ::Window* pParent, const Bitmap& rBmp, ::sd::DrawDocShell* pDocShell ) SAL_OVERRIDE;
    virtual AbstractSdPublishingDlg*    CreateSdPublishingDlg( ::Window* pWindow, DocumentType eDocType) SAL_OVERRIDE;

    virtual VclAbstractDialog* CreateSdPhotoAlbumDialog( ::Window* pWindow, SdDrawDocument* pDoc) SAL_OVERRIDE;

      virtual VclAbstractDialog*          CreateMasterLayoutDialog( ::Window* pParent,
                                                                  SdDrawDocument* pDoc,
                                                                  SdPage* ) SAL_OVERRIDE;

    virtual AbstractHeaderFooterDialog* CreateHeaderFooterDialog( ViewShell* pViewShell,
                                                                  ::Window* pParent,
                                                                  SdDrawDocument* pDoc,
                                                                  SdPage* pCurrentPage ) SAL_OVERRIDE;

    // For TabPage
    virtual CreateTabPage               GetSdOptionsContentsTabPageCreatorFunc() SAL_OVERRIDE;
    virtual CreateTabPage               GetSdPrintOptionsTabPageCreatorFunc() SAL_OVERRIDE;
    virtual CreateTabPage               GetSdOptionsMiscTabPageCreatorFunc() SAL_OVERRIDE;
    virtual CreateTabPage               GetSdOptionsSnapTabPageCreatorFunc() SAL_OVERRIDE;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
