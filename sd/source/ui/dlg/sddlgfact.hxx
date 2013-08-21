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
#ifndef _SD_DLGFACT_HXX
#define _SD_DLGFACT_HXX

#include "sdabstdlg.hxx"
#include <sfx2/basedlgs.hxx>

#define DECL_ABSTDLG_BASE(Class,DialogClass)        \
    DialogClass*        pDlg;                       \
public:                                             \
                    Class( DialogClass* p)          \
                     : pDlg(p)                      \
                     {}                             \
    virtual         ~Class();                       \
    virtual short   Execute() ;

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
// add for BreakDlg
class Dialog;
class SdVclAbstractDialog_Impl : public VclAbstractDialog
{
    DECL_ABSTDLG_BASE(SdVclAbstractDialog_Impl,Dialog)
};

// add for CopyDlg

class AbstractCopyDlg_Impl : public AbstractCopyDlg
{
    DECL_ABSTDLG_BASE(AbstractCopyDlg_Impl,::sd::CopyDlg)
    virtual void    GetAttr( SfxItemSet& rOutAttrs );
};

// add for SdCustomShowDlg
class SdCustomShowDlg;
class AbstractSdCustomShowDlg_Impl : public AbstractSdCustomShowDlg
{
    DECL_ABSTDLG_BASE(AbstractSdCustomShowDlg_Impl,SdCustomShowDlg)
    virtual sal_Bool        IsModified() const ;
    virtual sal_Bool        IsCustomShow() const ;
};

//add for SdCharDlg begin
class SfxTabDialog;
class SdAbstractTabDialog_Impl : public SfxAbstractTabDialog
{
    DECL_ABSTDLG_BASE( SdAbstractTabDialog_Impl,SfxTabDialog )
    virtual void                SetCurPageId( sal_uInt16 nId );
    virtual void                SetCurPageId( const OString& rName );
    virtual const SfxItemSet*   GetOutputItemSet() const;
    virtual const sal_uInt16*       GetInputRanges( const SfxItemPool& pItem );
    virtual void                SetInputSet( const SfxItemSet* pInSet );
        //From class Window.
    virtual void        SetText( const OUString& rStr );
    virtual OUString    GetText() const;
};
//add for SdCharDlg end


//add for OutlineBulletDlg begin
class SfxTabDialog;
class AbstractBulletDialog_Impl : public SfxAbstractTabDialog
{
    DECL_ABSTDLG_BASE( AbstractBulletDialog_Impl,SfxTabDialog )
    virtual void                SetCurPageId( sal_uInt16 nId );
    virtual void                SetCurPageId( const OString& rName );
    virtual const SfxItemSet*   GetOutputItemSet() const;
    virtual const sal_uInt16*       GetInputRanges( const SfxItemPool& pItem );
    virtual void                SetInputSet( const SfxItemSet* pInSet );
        //From class Window.
    virtual void        SetText( const OUString& rStr );
    virtual OUString    GetText() const;
};
//add for OutlineBulletDlg end

class SdPresLayoutTemplateDlg;
class SdPresLayoutTemplateDlg_Impl : public SfxAbstractTabDialog
{
    DECL_ABSTDLG_BASE( SdPresLayoutTemplateDlg_Impl,SdPresLayoutTemplateDlg )
    virtual void                SetCurPageId( sal_uInt16 nId );
    virtual void                SetCurPageId( const OString& rName );
    virtual const SfxItemSet*   GetOutputItemSet() const;
    virtual const sal_uInt16*       GetInputRanges( const SfxItemPool& pItem );
    virtual void                SetInputSet( const SfxItemSet* pInSet );
        //From class Window.
    virtual void        SetText( const OUString& rStr );
    virtual OUString    GetText() const;
};

// add for AssistentDlg
class AssistentDlg;
class AbstractAssistentDlg_Impl : public AbstractAssistentDlg
{
    DECL_ABSTDLG_BASE(AbstractAssistentDlg_Impl,AssistentDlg)
    virtual SfxObjectShellLock GetDocument();
    virtual OutputType GetOutputMedium() const;
    virtual sal_Bool IsSummary() const;
    virtual StartType GetStartType() const;
    virtual OUString GetDocPath() const;
    virtual sal_Bool GetStartWithFlag() const;
    virtual sal_Bool IsDocEmpty() const;
    virtual com::sun::star::uno::Sequence< com::sun::star::beans::NamedValue > GetPassword();
};

// add for SdModifyFieldDlg
class SdModifyFieldDlg;
class AbstractSdModifyFieldDlg_Impl : public AbstractSdModifyFieldDlg
{
    DECL_ABSTDLG_BASE(AbstractSdModifyFieldDlg_Impl,SdModifyFieldDlg)
    virtual SvxFieldData*       GetField();
    virtual SfxItemSet          GetItemSet();
};

// add for SdSnapLineDlg
class SdSnapLineDlg;
class AbstractSdSnapLineDlg_Impl : public AbstractSdSnapLineDlg
{
    DECL_ABSTDLG_BASE(AbstractSdSnapLineDlg_Impl,SdSnapLineDlg)
    virtual void GetAttr(SfxItemSet& rOutAttrs);
    virtual void HideRadioGroup();
    virtual void HideDeleteBtn();
    virtual void SetInputFields(sal_Bool bEnableX, sal_Bool bEnableY);
    //from class Window
    virtual void    SetText( const OUString& rStr );
};

// add for SdInsertLayerDlg
class SdInsertLayerDlg;
class AbstractSdInsertLayerDlg_Impl : public AbstractSdInsertLayerDlg
{
    DECL_ABSTDLG_BASE(AbstractSdInsertLayerDlg_Impl,SdInsertLayerDlg)
    virtual void    GetAttr( SfxItemSet& rOutAttrs ) ;
    //from class Window
    virtual void    SetHelpId( const OString& rHelpId ) ;
};

// add for SdInsertPasteDlg
class SdInsertPasteDlg;
class AbstractSdInsertPasteDlg_Impl : public AbstractSdInsertPasteDlg
{
    DECL_ABSTDLG_BASE(AbstractSdInsertPasteDlg_Impl,SdInsertPasteDlg)
    virtual sal_Bool            IsInsertBefore() const;
};

// add for SdInsertPagesObjsDlg
class SdInsertPagesObjsDlg;
class AbstractSdInsertPagesObjsDlg_Impl : public AbstractSdInsertPagesObjsDlg
{
    DECL_ABSTDLG_BASE(AbstractSdInsertPagesObjsDlg_Impl,SdInsertPagesObjsDlg)
    virtual ::Window *  GetWindow(); //this method is added for return a Window type pointer
    virtual std::vector<OUString> GetList ( const sal_uInt16 nType );
    virtual sal_Bool        IsLink();
    virtual sal_Bool        IsRemoveUnnessesaryMasterPages() const;
};

// add for MorphDlg
class AbstractMorphDlg_Impl : public AbstractMorphDlg
{
    DECL_ABSTDLG_BASE(AbstractMorphDlg_Impl,::sd::MorphDlg)
    virtual void            SaveSettings() const;
    virtual sal_uInt16          GetFadeSteps() const;
    virtual sal_Bool            IsAttributeFade() const ;
    virtual sal_Bool            IsOrientationFade() const ;
};

// add for SdStartPresentationDlg
class SdStartPresentationDlg;
class AbstractSdStartPresDlg_Impl : public AbstractSdStartPresDlg
{
    DECL_ABSTDLG_BASE(AbstractSdStartPresDlg_Impl,SdStartPresentationDlg)
    virtual void    GetAttr( SfxItemSet& rOutAttrs );
};

// add for SdPresLayoutDlg
class SdPresLayoutDlg;
class AbstractSdPresLayoutDlg_Impl : public AbstractSdPresLayoutDlg
{
    DECL_ABSTDLG_BASE(AbstractSdPresLayoutDlg_Impl,SdPresLayoutDlg)
    virtual void    GetAttr(SfxItemSet& rOutAttrs);
};

// add for SdActionDlg
class SdAbstractSfxDialog_Impl : public SfxAbstractDialog
{
    DECL_ABSTDLG_BASE(SdAbstractSfxDialog_Impl,SfxModalDialog)
    virtual const SfxItemSet*   GetOutputItemSet() const;
    virtual void        SetText( const OUString& rStr );
    virtual OUString    GetText() const;
};

// add for SdVectorizeDlg
class SdVectorizeDlg;
class AbstractSdVectorizeDlg_Impl :public AbstractSdVectorizeDlg
{
    DECL_ABSTDLG_BASE(AbstractSdVectorizeDlg_Impl,SdVectorizeDlg)
    virtual const GDIMetaFile&  GetGDIMetaFile() const ;
};

// add for SdPublishingDlg
class SdPublishingDlg;
class AbstractSdPublishingDlg_Impl :public AbstractSdPublishingDlg
{
    DECL_ABSTDLG_BASE(AbstractSdPublishingDlg_Impl,SdPublishingDlg)
    virtual void GetParameterSequence( ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rParams );
};

// add for HeaderFooterDialog
class AbstractHeaderFooterDialog_Impl :public AbstractHeaderFooterDialog
{
  DECL_ABSTDLG_BASE(AbstractHeaderFooterDialog_Impl,::sd::HeaderFooterDialog)
  virtual void ApplyToAll( TabPage* pPage );
  virtual void Apply( TabPage* pPage );
  virtual void Cancel( TabPage* pPage );
};
//------------------------------------------------------------------------

//AbstractDialogFactory_Impl implementations
class SdAbstractDialogFactory_Impl : public SdAbstractDialogFactory
{

public:
    virtual ~SdAbstractDialogFactory_Impl() {}

    virtual VclAbstractDialog*          CreateBreakDlg(::Window* pWindow, ::sd::DrawView* pDrView, ::sd::DrawDocShell* pShell, sal_uLong nSumActionCount, sal_uLong nObjCount );
    virtual AbstractCopyDlg*            CreateCopyDlg( ::Window* pWindow, const SfxItemSet& rInAttrs, const rtl::Reference<XColorList> &pColTab, ::sd::View* pView );
    virtual AbstractSdCustomShowDlg*    CreateSdCustomShowDlg( ::Window* pWindow, SdDrawDocument& rDrawDoc );
    virtual SfxAbstractTabDialog*       CreateSdTabCharDialog( ::Window* pParent, const SfxItemSet* pAttr, SfxObjectShell* pDocShell );
    virtual SfxAbstractTabDialog*       CreateSdTabPageDialog( ::Window* pParent, const SfxItemSet* pAttr, SfxObjectShell* pDocShell, sal_Bool bAreaPage = sal_True );
    virtual AbstractAssistentDlg*       CreateAssistentDlg( ::Window* pParent, sal_Bool bAutoPilot);
    virtual AbstractSdModifyFieldDlg*   CreateSdModifyFieldDlg( ::Window* pWindow, const SvxFieldData* pInField, const SfxItemSet& rSet );
    virtual AbstractSdSnapLineDlg*      CreateSdSnapLineDlg( ::Window* pWindow, const SfxItemSet& rInAttrs, ::sd::View* pView);
    virtual AbstractSdInsertLayerDlg*   CreateSdInsertLayerDlg( ::Window* pWindow, const SfxItemSet& rInAttrs, bool bDeletable, const OUString& aStr );
    virtual AbstractSdInsertPasteDlg*   CreateSdInsertPasteDlg( ::Window* pWindow );
    virtual AbstractSdInsertPagesObjsDlg* CreateSdInsertPagesObjsDlg( ::Window* pParent, const SdDrawDocument* pDoc, SfxMedium* pSfxMedium, const OUString& rFileName );
    virtual AbstractMorphDlg*           CreateMorphDlg( ::Window* pParent, const SdrObject* pObj1, const SdrObject* pObj2);
    virtual SfxAbstractTabDialog*       CreateSdOutlineBulletTabDlg ( ::Window* pParent, const SfxItemSet* pAttr, ::sd::View* pView = NULL );
    virtual SfxAbstractTabDialog*       CreateSdParagraphTabDlg ( ::Window* pParent, const SfxItemSet* pAttr );
    virtual AbstractSdStartPresDlg*     CreateSdStartPresentationDlg( ::Window* pWindow, const SfxItemSet& rInAttrs,
                                                                     const std::vector<OUString> &rPageNames, SdCustomShowList* pCSList );
    virtual VclAbstractDialog*          CreateRemoteDialog( ::Window* pWindow ); // ad for RemoteDialog
    virtual SfxAbstractTabDialog*       CreateSdPresLayoutTemplateDlg( SfxObjectShell* pDocSh, ::Window* pParent, SdResId DlgId, SfxStyleSheetBase& rStyleBase, PresentationObjects ePO, SfxStyleSheetBasePool* pSSPool );
    virtual AbstractSdPresLayoutDlg*    CreateSdPresLayoutDlg( ::sd::DrawDocShell* pDocShell, ::Window* pWindow, const SfxItemSet& rInAttrs);
    virtual SfxAbstractTabDialog*       CreateSdTabTemplateDlg( ::Window* pParent, const SfxObjectShell* pDocShell, SfxStyleSheetBase& rStyleBase, SdrModel* pModel, SdrView* pView );
    virtual SfxAbstractDialog*          CreatSdActionDialog( ::Window* pParent, const SfxItemSet* pAttr, ::sd::View* pView );
    virtual AbstractSdVectorizeDlg*     CreateSdVectorizeDlg( ::Window* pParent, const Bitmap& rBmp, ::sd::DrawDocShell* pDocShell );
    virtual AbstractSdPublishingDlg*    CreateSdPublishingDlg( ::Window* pWindow, DocumentType eDocType);

    virtual VclAbstractDialog* CreateSdPhotoAlbumDialog( ::Window* pWindow, SdDrawDocument* pDoc);

      virtual VclAbstractDialog*          CreateMasterLayoutDialog( ::Window* pParent,
                                                                  SdDrawDocument* pDoc,
                                                                  SdPage* ); // add for MasterLayoutDialog

    virtual AbstractHeaderFooterDialog* CreateHeaderFooterDialog( ViewShell* pViewShell,
                                                                  ::Window* pParent,
                                                                  SdDrawDocument* pDoc,
                                                                  SdPage* pCurrentPage ); // add for HeaderFooterDialog

    // For TabPage
    virtual CreateTabPage               GetSdOptionsContentsTabPageCreatorFunc();
    virtual CreateTabPage               GetSdPrintOptionsTabPageCreatorFunc();
    virtual CreateTabPage               GetSdOptionsMiscTabPageCreatorFunc();
    virtual CreateTabPage               GetSdOptionsSnapTabPageCreatorFunc();

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
