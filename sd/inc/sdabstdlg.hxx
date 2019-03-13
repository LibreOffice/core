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

#include <rtl/string.hxx>
#include <sfx2/tabdlg.hxx>
#include <svl/itemset.hxx>
#include <tools/solar.h>
#include <vcl/abstdlg.hxx>
#include "prlayout.hxx"
#include "pres.hxx"
#include "sddllapi.h"

namespace sd {
    class View;
    class ViewShell;
    class DrawDocShell;
    class DrawView;
}

class SfxObjectShell;
class SvxFieldData;
class GDIMetaFile;
class SdDrawDocument;
class SfxMedium;
class SdrObject;
class SfxStyleSheetBasePool;
class SfxStyleSheetBase;
class SdrModel;
class SdrView;
class Bitmap;
namespace vcl { class Window; }
class SdPage;
class SdCustomShowList;
class SfxAbstractDialog;
class SfxAbstractTabDialog;

class AbstractCopyDlg : public VclAbstractDialog
{
protected:
    virtual ~AbstractCopyDlg() override = default;
public:
    virtual void    GetAttr( SfxItemSet& rOutAttrs ) = 0;
};

class AbstractSdCustomShowDlg : public VclAbstractDialog
{
protected:
    virtual ~AbstractSdCustomShowDlg() override = default;
public:
    virtual bool        IsModified() const = 0;
    virtual bool        IsCustomShow() const = 0;
};

class AbstractSdModifyFieldDlg : public VclAbstractDialog
{
protected:
    virtual ~AbstractSdModifyFieldDlg() override = default;
public:
    virtual SvxFieldData*       GetField() = 0;
    virtual SfxItemSet          GetItemSet() = 0;
};

class AbstractSdSnapLineDlg : public VclAbstractDialog
{
protected:
    virtual ~AbstractSdSnapLineDlg() override = default;
public:
    virtual void GetAttr(SfxItemSet& rOutAttrs) = 0;
    virtual void HideRadioGroup() = 0;
    virtual void HideDeleteBtn() = 0;
    virtual void SetInputFields(bool bEnableX, bool bEnableY) = 0;
    //from class vcl::Window
    virtual void    SetText( const OUString& rStr ) = 0;
};

class AbstractSdInsertLayerDlg : public VclAbstractDialog
{
protected:
    virtual ~AbstractSdInsertLayerDlg() override = default;
public:
    virtual void    GetAttr( SfxItemSet& rOutAttrs ) = 0;
    //from class vcl::Window
    virtual void    SetHelpId( const OString& rHelpId ) = 0;
};

class AbstractSdInsertPagesObjsDlg : public VclAbstractDialog
{
protected:
    virtual ~AbstractSdInsertPagesObjsDlg() override = default;
public:
    virtual std::vector<OUString> GetList ( const sal_uInt16 nType ) = 0;
    virtual bool        IsLink() = 0;
    virtual bool        IsRemoveUnnessesaryMasterPages() const = 0;
};

class AbstractMorphDlg : public VclAbstractDialog
{
protected:
    virtual ~AbstractMorphDlg() override = default;
public:
    virtual void            SaveSettings() const = 0;
    virtual sal_uInt16          GetFadeSteps() const = 0;
    virtual bool            IsAttributeFade() const = 0;
    virtual bool            IsOrientationFade() const = 0;
};

class AbstractSdStartPresDlg : public VclAbstractDialog
{
protected:
    virtual ~AbstractSdStartPresDlg() override = default;
public:
    virtual void    GetAttr( SfxItemSet& rOutAttrs ) = 0;
};

class AbstractSdPresLayoutDlg : public VclAbstractDialog
{
protected:
    virtual ~AbstractSdPresLayoutDlg() override = default;
public:
    virtual void    GetAttr(SfxItemSet& rOutAttrs) = 0;
};

class AbstractSdVectorizeDlg : public VclAbstractDialog
{
protected:
    virtual ~AbstractSdVectorizeDlg() override = default;
public:
    virtual const GDIMetaFile&  GetGDIMetaFile() const = 0;
};

class AbstractSdPublishingDlg : public VclAbstractDialog
{
protected:
    virtual ~AbstractSdPublishingDlg() override = default;
public:
    virtual void GetParameterSequence( css::uno::Sequence< css::beans::PropertyValue >& rParams ) = 0;
};

class AbstractHeaderFooterDialog : public VclAbstractDialog
{
protected:
    virtual ~AbstractHeaderFooterDialog() override = default;
};

class SdAbstractDialogFactory
{
public:
    SD_DLLPUBLIC static SdAbstractDialogFactory*     Create();

    virtual VclPtr<VclAbstractDialog>          CreateBreakDlg(weld::Window* pWindow, ::sd::DrawView* pDrView, ::sd::DrawDocShell* pShell, sal_uLong nSumActionCount, sal_uLong nObjCount ) = 0;
    virtual VclPtr<AbstractCopyDlg>            CreateCopyDlg(weld::Window* pWindow, const SfxItemSet& rInAttrs, ::sd::View* pView ) = 0;
    virtual VclPtr<AbstractSdCustomShowDlg>    CreateSdCustomShowDlg(weld::Window* pWindow, SdDrawDocument& rDrawDoc) = 0;
    virtual VclPtr<SfxAbstractTabDialog>       CreateSdTabCharDialog(weld::Window* pWindow, const SfxItemSet* pAttr, SfxObjectShell* pDocShell) = 0;
    virtual VclPtr<SfxAbstractTabDialog>       CreateSdTabPageDialog(weld::Window* pWindow, const SfxItemSet* pAttr, SfxObjectShell* pDocShell, bool bAreaPage) = 0;
    virtual VclPtr<AbstractSdModifyFieldDlg>   CreateSdModifyFieldDlg(weld::Window* pWindow, const SvxFieldData* pInField, const SfxItemSet& rSet) = 0;
    virtual VclPtr<AbstractSdSnapLineDlg>      CreateSdSnapLineDlg(weld::Window* pParent, const SfxItemSet& rInAttrs, ::sd::View* pView) = 0;
    virtual VclPtr<AbstractSdInsertLayerDlg>   CreateSdInsertLayerDlg(weld::Window* pParent, const SfxItemSet& rInAttrs, bool bDeletable, const OUString& rStr) = 0;
    virtual VclPtr<AbstractSdInsertPagesObjsDlg> CreateSdInsertPagesObjsDlg(weld::Window* pParent, const SdDrawDocument* pDoc, SfxMedium* pSfxMedium, const OUString& rFileName) = 0;
    virtual VclPtr<AbstractMorphDlg>           CreateMorphDlg(weld::Window* pParent, const SdrObject* pObj1, const SdrObject* pObj2) = 0;
    virtual VclPtr<SfxAbstractTabDialog>       CreateSdOutlineBulletTabDlg(weld::Window* pParent, const SfxItemSet* pAttr, ::sd::View* pView) = 0;
    virtual VclPtr<SfxAbstractTabDialog>       CreateSdParagraphTabDlg(weld::Window* pWindow, const SfxItemSet* pAttr) = 0;
    virtual VclPtr<AbstractSdStartPresDlg>     CreateSdStartPresentationDlg(weld::Window* pWindow, const SfxItemSet& rInAttrs,
                                                                     const std::vector<OUString> &rPageNames, SdCustomShowList* pCSList ) = 0;
    virtual VclPtr<VclAbstractDialog>          CreateRemoteDialog( vcl::Window* pWindow ) = 0;
    virtual VclPtr<SfxAbstractTabDialog>       CreateSdPresLayoutTemplateDlg(SfxObjectShell* pDocSh, weld::Window* pParent, bool bBackgroundDlg, SfxStyleSheetBase& rStyleBase, PresentationObjects ePO, SfxStyleSheetBasePool* pSSPool) = 0;
    virtual VclPtr<AbstractSdPresLayoutDlg>    CreateSdPresLayoutDlg(weld::Window* pParent, ::sd::DrawDocShell* pDocShell, const SfxItemSet& rInAttrs) = 0;
    virtual VclPtr<SfxAbstractTabDialog>       CreateSdTabTemplateDlg(weld::Window* pParent, const SfxObjectShell* pDocShell, SfxStyleSheetBase& rStyleBase, SdrModel* pModel, SdrView* pView) = 0;
    virtual VclPtr<SfxAbstractDialog>          CreatSdActionDialog(weld::Window* pParent, const SfxItemSet* pAttr, ::sd::View* pView) = 0;
    virtual VclPtr<AbstractSdVectorizeDlg>     CreateSdVectorizeDlg(weld::Window* pParent, const Bitmap& rBmp, ::sd::DrawDocShell* pDocShell) = 0;
    virtual VclPtr<AbstractSdPublishingDlg>    CreateSdPublishingDlg( vcl::Window* pWindow, DocumentType eDocType) = 0;

    virtual VclPtr<VclAbstractDialog>          CreateMasterLayoutDialog(weld::Window* pParent, SdDrawDocument* pDoc, SdPage*) = 0;

    virtual VclPtr<AbstractHeaderFooterDialog> CreateHeaderFooterDialog( sd::ViewShell* pViewShell,
                                                                  vcl::Window* pParent,
                                                                  SdDrawDocument* pDoc,
                                                                  SdPage* pCurrentPage ) = 0;

    virtual CreateTabPage               GetSdOptionsContentsTabPageCreatorFunc() = 0;
    virtual CreateTabPage               GetSdPrintOptionsTabPageCreatorFunc() = 0;
    virtual CreateTabPage               GetSdOptionsMiscTabPageCreatorFunc() = 0;
    virtual CreateTabPage               GetSdOptionsSnapTabPageCreatorFunc() = 0;

    virtual VclPtr<VclAbstractDialog> CreateSdPhotoAlbumDialog(weld::Window* pWindow, SdDrawDocument* pDoc) = 0;

protected:
    ~SdAbstractDialogFactory() {}
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
