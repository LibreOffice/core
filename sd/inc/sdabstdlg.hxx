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
#ifndef INCLUDED_SD_INC_SDABSTDLG_HXX
#define INCLUDED_SD_INC_SDABSTDLG_HXX

#include <vector>

#include <rtl/ref.hxx>
#include <rtl/string.hxx>
#include <tools/solar.h>
#include <sfx2/sfxdlg.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include "prlayout.hxx"
#include "sdenumdef.hxx"
#include "pres.hxx"

namespace sd {
    class View;
    class DrawDocShell;
    class DrawView;
}

class SfxObjectShell;
class SfxObjectShellLock;
class SvxFieldData;
class GDIMetaFile;
class XColorList;
class SdDrawDocument;
class SfxMedium;
class SdrObject;
class SfxStyleSheetBasePool;
class SfxStyleSheetBase;
class SdrModel;
class SdrView;
class Bitmap;
class List;
class SdResId;
class Window;
class SdPage;
class TabPage;
class ViewShell;
class SdCustomShowList;

class AbstractCopyDlg : public VclAbstractDialog
{
public:
    virtual void    GetAttr( SfxItemSet& rOutAttrs ) = 0;
};

class AbstractSdCustomShowDlg : public VclAbstractDialog
{
public:
    virtual sal_Bool        IsModified() const = 0;
    virtual sal_Bool        IsCustomShow() const = 0;
};

class AbstractAssistentDlg : public VclAbstractDialog
{
public:
    virtual SfxObjectShellLock GetDocument() = 0;
    virtual OutputType GetOutputMedium() const = 0;
    virtual sal_Bool IsSummary() const = 0;
    virtual StartType GetStartType() const = 0;
    virtual OUString GetDocPath() const = 0;
    virtual sal_Bool GetStartWithFlag() const = 0;
    virtual sal_Bool IsDocEmpty() const = 0;
    virtual com::sun::star::uno::Sequence< com::sun::star::beans::NamedValue > GetPassword() = 0;
};

class AbstractSdModifyFieldDlg : public VclAbstractDialog
{
public:
    virtual SvxFieldData*       GetField() = 0;
    virtual SfxItemSet          GetItemSet() = 0;
};

class AbstractSdSnapLineDlg : public VclAbstractDialog
{
public:
    virtual void GetAttr(SfxItemSet& rOutAttrs) = 0;
    virtual void HideRadioGroup() = 0;
    virtual void HideDeleteBtn() = 0;
    virtual void SetInputFields(sal_Bool bEnableX, sal_Bool bEnableY) = 0;
    //from class ::Window
    virtual void    SetText( const OUString& rStr ) = 0;
};

class AbstractSdInsertLayerDlg : public VclAbstractDialog
{
public:
    virtual void    GetAttr( SfxItemSet& rOutAttrs ) = 0;
    //from class ::Window
    virtual void    SetHelpId( const OString& rHelpId ) = 0;
};

class AbstractSdInsertPasteDlg : public VclAbstractDialog
{
public:
    virtual sal_Bool            IsInsertBefore() const = 0;
};

class AbstractSdInsertPagesObjsDlg : public VclAbstractDialog
{
public:
    virtual ::Window*   GetWindow() = 0;  //this method is added for return a ::Window type pointer
    virtual std::vector<OUString> GetList ( const sal_uInt16 nType ) = 0;
    virtual sal_Bool        IsLink() = 0;
    virtual sal_Bool        IsRemoveUnnessesaryMasterPages() const = 0;
};

class AbstractMorphDlg : public VclAbstractDialog
{
public:
    virtual void            SaveSettings() const = 0;
    virtual sal_uInt16          GetFadeSteps() const = 0;
    virtual sal_Bool            IsAttributeFade() const = 0;
    virtual sal_Bool            IsOrientationFade() const = 0;
};

class AbstractSdStartPresDlg : public VclAbstractDialog
{
public:
    virtual void    GetAttr( SfxItemSet& rOutAttrs ) = 0;
};

class AbstractSdPresLayoutDlg : public VclAbstractDialog
{
public:
    virtual void    GetAttr(SfxItemSet& rOutAttrs) = 0;
};

class AbstractSdVectorizeDlg : public VclAbstractDialog
{
public:
    virtual const GDIMetaFile&  GetGDIMetaFile() const = 0;
};

class AbstractSdPublishingDlg : public VclAbstractDialog
{
public:
    virtual void GetParameterSequence( ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rParams ) = 0;
};

class AbstractHeaderFooterDialog : public VclAbstractDialog
{
public:
    virtual void ApplyToAll() = 0;
    virtual void Apply() = 0;
    virtual void Cancel() = 0;
};


class SdAbstractDialogFactory
{
public:
    static SdAbstractDialogFactory*     Create();

    virtual VclAbstractDialog*          CreateBreakDlg(::Window* pWindow, ::sd::DrawView* pDrView, ::sd::DrawDocShell* pShell, sal_uLong nSumActionCount, sal_uLong nObjCount ) = 0;
    virtual AbstractCopyDlg*            CreateCopyDlg( ::Window* pWindow, const SfxItemSet& rInAttrs, const rtl::Reference<XColorList> &pColTab, ::sd::View* pView ) = 0;
    virtual AbstractSdCustomShowDlg*    CreateSdCustomShowDlg( ::Window* pWindow, SdDrawDocument& rDrawDoc ) = 0;
    virtual SfxAbstractTabDialog*       CreateSdTabCharDialog( ::Window* pParent, const SfxItemSet* pAttr, SfxObjectShell* pDocShell ) = 0;
    virtual SfxAbstractTabDialog*       CreateSdTabPageDialog( ::Window* pParent, const SfxItemSet* pAttr, SfxObjectShell* pDocShell, sal_Bool bAreaPage = sal_True ) = 0;
    virtual AbstractAssistentDlg*       CreateAssistentDlg( ::Window* pParent, sal_Bool bAutoPilot) = 0;
    virtual AbstractSdModifyFieldDlg*   CreateSdModifyFieldDlg( ::Window* pWindow, const SvxFieldData* pInField, const SfxItemSet& rSet ) = 0;
    virtual AbstractSdSnapLineDlg*      CreateSdSnapLineDlg( ::Window* pWindow, const SfxItemSet& rInAttrs, ::sd::View* pView) = 0;
    virtual AbstractSdInsertLayerDlg*   CreateSdInsertLayerDlg( ::Window* pWindow, const SfxItemSet& rInAttrs, bool bDeletable, const OUString& aStr ) = 0;
    virtual AbstractSdInsertPasteDlg*   CreateSdInsertPasteDlg( ::Window* pWindow ) = 0;
    virtual AbstractSdInsertPagesObjsDlg* CreateSdInsertPagesObjsDlg( ::Window* pParent, const SdDrawDocument* pDoc, SfxMedium* pSfxMedium, const OUString& rFileName ) = 0;
    virtual AbstractMorphDlg*           CreateMorphDlg( ::Window* pParent, const SdrObject* pObj1, const SdrObject* pObj2) = 0;
    virtual SfxAbstractTabDialog*       CreateSdOutlineBulletTabDlg ( ::Window* pParent, const SfxItemSet* pAttr, ::sd::View* pView = NULL ) = 0;
    virtual SfxAbstractTabDialog*       CreateSdParagraphTabDlg ( ::Window* pParent, const SfxItemSet* pAttr ) = 0;
    virtual AbstractSdStartPresDlg*     CreateSdStartPresentationDlg( ::Window* pWindow, const SfxItemSet& rInAttrs,
                                                                     const std::vector<OUString> &rPageNames, SdCustomShowList* pCSList ) = 0;
    virtual VclAbstractDialog*          CreateRemoteDialog( ::Window* pWindow ) = 0;
    virtual SfxAbstractTabDialog*       CreateSdPresLayoutTemplateDlg( SfxObjectShell* pDocSh, ::Window* pParent, SdResId DlgId, SfxStyleSheetBase& rStyleBase, PresentationObjects ePO, SfxStyleSheetBasePool* pSSPool ) = 0;
    virtual AbstractSdPresLayoutDlg*    CreateSdPresLayoutDlg( ::sd::DrawDocShell* pDocShell, ::Window* pWindow, const SfxItemSet& rInAttrs) = 0;
    virtual SfxAbstractTabDialog*       CreateSdTabTemplateDlg( ::Window* pParent, const SfxObjectShell* pDocShell, SfxStyleSheetBase& rStyleBase, SdrModel* pModel, SdrView* pView ) = 0;
    virtual SfxAbstractDialog*          CreatSdActionDialog( ::Window* pParent, const SfxItemSet* pAttr, ::sd::View* pView ) = 0;
    virtual AbstractSdVectorizeDlg*     CreateSdVectorizeDlg( ::Window* pParent, const Bitmap& rBmp, ::sd::DrawDocShell* pDocShell ) = 0;
    virtual AbstractSdPublishingDlg*    CreateSdPublishingDlg( ::Window* pWindow, DocumentType eDocType) = 0;

    virtual VclAbstractDialog*          CreateMasterLayoutDialog( ::Window* pParent,
                                                                  SdDrawDocument* pDoc,
                                                                  SdPage* ) = 0;

    virtual AbstractHeaderFooterDialog* CreateHeaderFooterDialog( ViewShell* pViewShell,
                                                                  ::Window* pParent,
                                                                  SdDrawDocument* pDoc,
                                                                  SdPage* pCurrentPage ) = 0;

    virtual CreateTabPage               GetSdOptionsContentsTabPageCreatorFunc() = 0;
    virtual CreateTabPage               GetSdPrintOptionsTabPageCreatorFunc() = 0;
    virtual CreateTabPage               GetSdOptionsMiscTabPageCreatorFunc() = 0;
    virtual CreateTabPage               GetSdOptionsSnapTabPageCreatorFunc() = 0;

    virtual VclAbstractDialog* CreateSdPhotoAlbumDialog( ::Window* pWindow, SdDrawDocument* pDoc) = 0;

protected:
    ~SdAbstractDialogFactory() {}
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
