/*************************************************************************
 *
 *  $RCSfile: sdabstdlg.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-05-10 15:41:01 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _SD_ABSTDLG_HXX
#define _SD_ABSTDLG_HXX

// include ---------------------------------------------------------------

#include <tools/solar.h>
#include <tools/string.hxx>
#include <sfx2/sfxdlg.hxx>
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYACCESS_HPP_
#include <com/sun/star/beans/XPropertyAccess.hpp>
#endif
#include "prlayout.hxx"
#include "sdenumdef.hxx"
#include "pres.hxx"

namespace sd {
    class View;
    class DrawDocShell;
    class ViewShell;
    class DrawView;
}

class SfxTabPage;
class SfxViewFrame;
class SfxBindings;
//class SfxItemSet;
class ResId;
class String;
class SfxItemPool;
class SfxObjectShell;
class SfxObjectShellLock;
class SvxFieldData;
class GDIMetaFile;
class XColorTable;
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

class AbstractCopyDlg : public VclAbstractDialog  //add for CopyDlg
{
public:
    virtual void    GetAttr( SfxItemSet& rOutAttrs ) = 0;
};

class AbstractSdCustomShowDlg : public VclAbstractDialog  //add for SdCustomShowDlg
{
public:
    virtual BOOL        IsModified() const = 0;
    virtual BOOL        IsCustomShow() const = 0;
};

class AbstractAssistentDlg : public VclAbstractDialog  //add for AssistentDlg
{
public:
    virtual SfxObjectShellLock GetDocument() = 0;
    virtual OutputType GetOutputMedium() const = 0;
    virtual BOOL IsSummary() const = 0;
    virtual StartType GetStartType() const = 0;
    virtual String GetDocPath() const = 0;
    virtual BOOL GetStartWithFlag() const = 0;
    virtual BOOL IsDocEmpty() const = 0;
    virtual String GetPassword() = 0;
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
    virtual void SetInputFields(BOOL bEnableX, BOOL bEnableY) = 0;
    //from class ::Window
    virtual void    SetText( const XubString& rStr ) = 0;
};

class AbstractSdInsertLayerDlg : public VclAbstractDialog  //add for SdInsertLayerDlg
{
public:
    virtual void    GetAttr( SfxItemSet& rOutAttrs ) = 0;
    //from class ::Window
    virtual void    SetHelpId( ULONG nHelpId ) = 0;
};

class AbstractSdInsertPasteDlg : public VclAbstractDialog  //add for SdInsertPasteDlg
{
public:
    virtual BOOL            IsInsertBefore() const = 0;
};

class AbstractSdInsertPagesObjsDlg : public VclAbstractDialog  //add for SdInsertPagesObjsDlg
{
public:
    virtual ::Window*   GetWindow() = 0;  //this method is added for return a ::Window type pointer
    virtual List*       GetList( USHORT nType ) = 0;
    virtual BOOL        IsLink() = 0;
    virtual BOOL        IsRemoveUnnessesaryMasterPages() const = 0;
};

class AbstractMorphDlg : public VclAbstractDialog  //add for MorphDlg
{
public:
    virtual void            SaveSettings() const = 0;
    virtual USHORT          GetFadeSteps() const = 0;
    virtual BOOL            IsAttributeFade() const = 0;
    virtual BOOL            IsOrientationFade() const = 0;
};

class AbstractSdNewFoilDlg : public VclAbstractDialog  //add for SdNewFoilDlg
{
public:
    virtual void    GetAttr( SfxItemSet& rOutAttrs ) = 0;
};

class AbstractSdStartPresDlg : public VclAbstractDialog  //add for SdStartPresentationDlg
{
public:
    virtual void    GetAttr( SfxItemSet& rOutAttrs ) = 0;
};

class AbstractSdPrintDlg : public VclAbstractDialog  //add for SdPrintDlg
{
public:
    virtual USHORT  GetAttr() = 0;
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

//---------------------------------------------------------
class SdAbstractDialogFactory
{
public:
    static SdAbstractDialogFactory*     Create();

    virtual VclAbstractDialog*          CreateBreakDlg( const ResId& rResId,
                                            ::Window* pWindow,
                                            ::sd::DrawView* pDrView,
                                            ::sd::DrawDocShell* pShell,
                                            ULONG nSumActionCount,
                                            ULONG nObjCount ) = 0; //add for BreakDlg
    virtual AbstractCopyDlg*            CreateCopyDlg( const ResId& rResId,
                                            ::Window* pWindow, const SfxItemSet& rInAttrs,
                                            XColorTable* pColTab, ::sd::View* pView ) = 0; //add for CopyDlg
    virtual AbstractSdCustomShowDlg*    CreateSdCustomShowDlg( const ResId& rResId,
                                            ::Window* pWindow, SdDrawDocument& rDrawDoc ) = 0; //add for SdCustomShowDlg
    virtual SfxAbstractTabDialog*       CreateSdTabDialog( const ResId& rResId,
                                            ::Window* pParent, const SfxItemSet* pAttr,
                                            SfxObjectShell* pDocShell, BOOL bAreaPage = TRUE ) = 0; //add for SdCharDlg, SdPageDlg
    virtual AbstractAssistentDlg*       CreateAssistentDlg( const ResId& rResId,
                                            ::Window* pParent, BOOL bAutoPilot) = 0; //add for AssistentDlg
    virtual AbstractSdModifyFieldDlg*   CreateSdModifyFieldDlg( const ResId& rResId,
                                            ::Window* pWindow, const SvxFieldData* pInField, const SfxItemSet& rSet ) = 0; //add for SdModifyFieldDlg
    virtual AbstractSdSnapLineDlg*      CreateSdSnapLineDlg( const ResId& rResId,
                                            ::Window* pWindow, const SfxItemSet& rInAttrs, ::sd::View* pView) = 0; //add for SdSnapLineDlg
    virtual AbstractSdInsertLayerDlg*   CreateSdInsertLayerDlg( const ResId& rResId,
                                            ::Window* pWindow,
                                            const SfxItemSet& rInAttrs,
                                            BOOL bDeletable,
                                            String aStr ) = 0; //add for SdInsertLayerDlg
    virtual AbstractSdInsertPasteDlg*   CreateSdInsertPasteDlg( const ResId& rResId, ::Window* pWindow ) = 0; //add for SdInsertPasteDlg
    virtual AbstractSdInsertPagesObjsDlg*   CreateSdInsertPagesObjsDlg( const ResId& rResId,
                                                ::Window* pParent,
                                                const SdDrawDocument* pDoc,
                                                SfxMedium* pSfxMedium,
                                                const String& rFileName ) = 0; //add for SdInsertPagesObjsDlg
    virtual AbstractMorphDlg*           CreateMorphDlg( const ResId& rResId,
                                                ::Window* pParent,
                                                const SdrObject* pObj1,
                                                const SdrObject* pObj2) = 0; //add for MorphDlg
    virtual AbstractSdNewFoilDlg*       CreateSdNewFoilDlg( const ResId& rResId,
                                                ::Window* pWindow,
                                                const SfxItemSet& rInAttrs,
                                                PageKind ePgKind,
                                                ::sd::DrawDocShell* pDocShell,
                                                BOOL bChangeFoil ) = 0; //add for SdNewFoilDlg
    virtual SfxAbstractTabDialog*       CreateSdItemSetTabDlg ( const ResId& rResId,
                                                ::Window* pParent,
                                                const SfxItemSet* pAttr,
                                                ::sd::View* pView = NULL ) = 0; //add for OutlineBulletDlg, SdParagraphDlg
    virtual AbstractSdStartPresDlg*     CreateSdStartPresentationDlg( const ResId& rResId,
                                                ::Window* pWindow,
                                                const SfxItemSet& rInAttrs,
                                                List& rPageNames,
                                                List* pCSList ) = 0; //add for SdStartPresentationDlg
    virtual AbstractSdPrintDlg*         CreateSdPrintDlg( const ResId& rResId, ::Window* pWindow ) = 0; //add for SdPrintDlg
    virtual SfxAbstractTabDialog*       CreateSdPresLayoutTemplateDlg( const ResId& rResId,
                                                SfxObjectShell* pDocSh, ::Window* pParent,
                                                SdResId DlgId, SfxStyleSheetBase& rStyleBase,
                                                PresentationObjects ePO, SfxStyleSheetBasePool* pSSPool ) = 0; //add for SdPresLayoutTemplateDlg
    virtual AbstractSdPresLayoutDlg*    CreateSdPresLayoutDlg( const ResId& rResId,
                                                ::sd::DrawDocShell* pDocShell,
                                                ::sd::ViewShell* pViewShell,
                                                ::Window* pWindow,
                                                const SfxItemSet& rInAttrs) = 0; //add for SdPresLayoutDlg
    virtual SfxAbstractTabDialog*       CreateSdTabTemplateDlg( const ResId& rResId,
                                                ::Window* pParent,
                                                const SfxObjectShell* pDocShell,
                                                SfxStyleSheetBase& rStyleBase,
                                                SdrModel* pModel,
                                                SdrView* pView ) = 0; //add for SdTabTemplateDlg
    virtual AbstractSfxSingleTabDialog*  CreateSfxSingleTabDialog( const ResId& rResId,
                                                ::Window* pParent,
                                                const SfxItemSet* pAttr,
                                                ::sd::View* pView ) = 0; //add for SdActionDlg
    virtual AbstractSdVectorizeDlg*     CreateSdVectorizeDlg( const ResId& rResId,
                                                ::Window* pParent, const Bitmap& rBmp,
                                                ::sd::DrawDocShell* pDocShell ) = 0; //add for SdVectorizeDlg
    virtual AbstractSdPublishingDlg*    CreateSdPublishingDlg( const ResId& rResId,
                                                ::Window* pWindow, DocumentType eDocType) = 0; //add for SdPublishingDlg

    virtual CreateTabPage               GetTabPageCreatorFunc( USHORT nId ) = 0;
    virtual GetTabPageRanges            GetTabPageRangesFunc( USHORT nId ) = 0;
};
#endif

