/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SD_ABSTDLG_HXX
#define _SD_ABSTDLG_HXX

// include ---------------------------------------------------------------

#include <vector>

#include <rtl/ref.hxx>
#include <rtl/string.hxx>
#include <tools/solar.h>
#include <tools/string.hxx>
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
    class ViewShell;
    class DrawView;
}

class String;
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

class AbstractCopyDlg : public VclAbstractDialog  //add for CopyDlg
{
public:
    virtual void    GetAttr( SfxItemSet& rOutAttrs ) = 0;
};

class AbstractSdCustomShowDlg : public VclAbstractDialog  //add for SdCustomShowDlg
{
public:
    virtual sal_Bool        IsModified() const = 0;
    virtual sal_Bool        IsCustomShow() const = 0;
};

class AbstractAssistentDlg : public VclAbstractDialog  //add for AssistentDlg
{
public:
    virtual SfxObjectShellLock GetDocument() = 0;
    virtual OutputType GetOutputMedium() const = 0;
    virtual sal_Bool IsSummary() const = 0;
    virtual StartType GetStartType() const = 0;
    virtual String GetDocPath() const = 0;
    virtual sal_Bool GetStartWithFlag() const = 0;
    virtual sal_Bool IsDocEmpty() const = 0;
    virtual com::sun::star::uno::Sequence< com::sun::star::beans::NamedValue > GetPassword() = 0;
};

class AbstractSdModifyFieldDlg : public VclAbstractDialog  //add for SdModifyFieldDlg
{
public:
    virtual SvxFieldData*       GetField() = 0;
    virtual SfxItemSet          GetItemSet() = 0;
};

class AbstractSdSnapLineDlg : public VclAbstractDialog  //add for SdSnapLineDlg
{
public:
    virtual void GetAttr(SfxItemSet& rOutAttrs) = 0;
    virtual void HideRadioGroup() = 0;
    virtual void HideDeleteBtn() = 0;
    virtual void SetInputFields(sal_Bool bEnableX, sal_Bool bEnableY) = 0;
    //from class ::Window
    virtual void    SetText( const XubString& rStr ) = 0;
};

class AbstractSdInsertLayerDlg : public VclAbstractDialog  //add for SdInsertLayerDlg
{
public:
    virtual void    GetAttr( SfxItemSet& rOutAttrs ) = 0;
    //from class ::Window
    virtual void    SetHelpId( const rtl::OString& rHelpId ) = 0;
};

class AbstractSdInsertPasteDlg : public VclAbstractDialog  //add for SdInsertPasteDlg
{
public:
    virtual sal_Bool            IsInsertBefore() const = 0;
};

class AbstractSdInsertPagesObjsDlg : public VclAbstractDialog  //add for SdInsertPagesObjsDlg
{
public:
    virtual ::Window*   GetWindow() = 0;  //this method is added for return a ::Window type pointer
    virtual std::vector<rtl::OUString> GetList ( const sal_uInt16 nType ) = 0;
    virtual sal_Bool        IsLink() = 0;
    virtual sal_Bool        IsRemoveUnnessesaryMasterPages() const = 0;
};

class AbstractMorphDlg : public VclAbstractDialog  //add for MorphDlg
{
public:
    virtual void            SaveSettings() const = 0;
    virtual sal_uInt16          GetFadeSteps() const = 0;
    virtual sal_Bool            IsAttributeFade() const = 0;
    virtual sal_Bool            IsOrientationFade() const = 0;
};

class AbstractSdStartPresDlg : public VclAbstractDialog  //add for SdStartPresentationDlg
{
public:
    virtual void    GetAttr( SfxItemSet& rOutAttrs ) = 0;
};

class AbstractSdPresLayoutDlg : public VclAbstractDialog  //add for SdPresLayoutDlg
{
public:
    virtual void    GetAttr(SfxItemSet& rOutAttrs) = 0;
};

class AbstractSdVectorizeDlg : public VclAbstractDialog  //add for SdVectorizeDlg
{
public:
    virtual const GDIMetaFile&  GetGDIMetaFile() const = 0;
};

class AbstractSdPublishingDlg : public VclAbstractDialog  //add for SdPublishingDlg
{
public:
    virtual void GetParameterSequence( ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rParams ) = 0;
};

class AbstractHeaderFooterDialog : public VclAbstractDialog // add for HeaderFooterDialog
{
public:
    virtual void ApplyToAll( TabPage* pPage ) = 0;
    virtual void Apply( TabPage* pPage ) = 0;
    virtual void Cancel( TabPage* pPage ) = 0;
};

//---------------------------------------------------------
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
    virtual AbstractSdInsertLayerDlg*   CreateSdInsertLayerDlg( ::Window* pWindow, const SfxItemSet& rInAttrs, bool bDeletable, String aStr ) = 0;
    virtual AbstractSdInsertPasteDlg*   CreateSdInsertPasteDlg( ::Window* pWindow ) = 0;
    virtual AbstractSdInsertPagesObjsDlg* CreateSdInsertPagesObjsDlg( ::Window* pParent, const SdDrawDocument* pDoc, SfxMedium* pSfxMedium, const String& rFileName ) = 0;
    virtual AbstractMorphDlg*           CreateMorphDlg( ::Window* pParent, const SdrObject* pObj1, const SdrObject* pObj2) = 0;
    virtual SfxAbstractTabDialog*       CreateSdOutlineBulletTabDlg ( ::Window* pParent, const SfxItemSet* pAttr, ::sd::View* pView = NULL ) = 0;
    virtual SfxAbstractTabDialog*       CreateSdParagraphTabDlg ( ::Window* pParent, const SfxItemSet* pAttr ) = 0;
    virtual AbstractSdStartPresDlg*     CreateSdStartPresentationDlg( ::Window* pWindow, const SfxItemSet& rInAttrs,
                                                                     const std::vector<String> &rPageNames, List* pCSList ) = 0;
    virtual VclAbstractDialog*          CreateRemoteDialog( ::Window* pWindow ) = 0;
    virtual SfxAbstractTabDialog*       CreateSdPresLayoutTemplateDlg( SfxObjectShell* pDocSh, ::Window* pParent, SdResId DlgId, SfxStyleSheetBase& rStyleBase, PresentationObjects ePO, SfxStyleSheetBasePool* pSSPool ) = 0;
    virtual AbstractSdPresLayoutDlg*    CreateSdPresLayoutDlg( ::sd::DrawDocShell* pDocShell, ::sd::ViewShell* pViewShell, ::Window* pWindow, const SfxItemSet& rInAttrs) = 0;
    virtual SfxAbstractTabDialog*       CreateSdTabTemplateDlg( ::Window* pParent, const SfxObjectShell* pDocShell, SfxStyleSheetBase& rStyleBase, SdrModel* pModel, SdrView* pView ) = 0;
    virtual SfxAbstractDialog*          CreatSdActionDialog( ::Window* pParent, const SfxItemSet* pAttr, ::sd::View* pView ) = 0;
    virtual AbstractSdVectorizeDlg*     CreateSdVectorizeDlg( ::Window* pParent, const Bitmap& rBmp, ::sd::DrawDocShell* pDocShell ) = 0;
    virtual AbstractSdPublishingDlg*    CreateSdPublishingDlg( ::Window* pWindow, DocumentType eDocType) = 0;

    virtual VclAbstractDialog*          CreateMasterLayoutDialog( ::Window* pParent,
                                                                  SdDrawDocument* pDoc,
                                                                  SdPage* ) = 0; // add for MasterLayoutDialog

    virtual AbstractHeaderFooterDialog* CreateHeaderFooterDialog( ViewShell* pViewShell,
                                                                  ::Window* pParent,
                                                                  SdDrawDocument* pDoc,
                                                                  SdPage* pCurrentPage ) = 0; // add for HeaderFooterDialog

    virtual CreateTabPage               GetSdOptionsContentsTabPageCreatorFunc() = 0;
    virtual CreateTabPage               GetSdPrintOptionsTabPageCreatorFunc() = 0;
    virtual CreateTabPage               GetSdOptionsMiscTabPageCreatorFunc() = 0;
    virtual CreateTabPage               GetSdOptionsSnapTabPageCreatorFunc() = 0;

protected:
    ~SdAbstractDialogFactory() {}
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
