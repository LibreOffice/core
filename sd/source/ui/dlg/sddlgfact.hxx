/*************************************************************************
 *
 *  $RCSfile: sddlgfact.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-05-10 15:45:15 $
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
#ifndef _SD_DLGFACT_HXX
#define _SD_DLGFACT_HXX

// include ---------------------------------------------------------------
#include "sdabstdlg.hxx"

#define DECL_ABSTDLG_BASE(Class,DialogClass)        \
    DialogClass*        pDlg;                       \
public:                                             \
                    Class( DialogClass* p)          \
                     : pDlg(p)                      \
                     {}                             \
    virtual         ~Class();                       \
    virtual USHORT  Execute() ;

#define IMPL_ABSTDLG_BASE(Class)                    \
Class::~Class()                                     \
{                                                   \
    delete pDlg;                                    \
}                                                   \
USHORT Class::Execute()                             \
{                                                   \
    return pDlg->Execute();                         \
}

namespace sd {
    class MorphDlg;
    class CopyDlg;
    class BreakDlg;
    class OutlineBulletDlg;
}
// add for BreakDlg
class Dialog;
class VclAbstractDialog_Impl : public VclAbstractDialog
{
    DECL_ABSTDLG_BASE(VclAbstractDialog_Impl,Dialog);
};

// add for CopyDlg

class AbstractCopyDlg_Impl : public AbstractCopyDlg
{
    DECL_ABSTDLG_BASE(AbstractCopyDlg_Impl,::sd::CopyDlg);
    virtual void    GetAttr( SfxItemSet& rOutAttrs );
};

// add for SdCustomShowDlg
class SdCustomShowDlg;
class AbstractSdCustomShowDlg_Impl : public AbstractSdCustomShowDlg
{
    DECL_ABSTDLG_BASE(AbstractSdCustomShowDlg_Impl,SdCustomShowDlg);
    virtual BOOL        IsModified() const ;
    virtual BOOL        IsCustomShow() const ;
};

//add for SdCharDlg begin
class SfxTabDialog;
class AbstractTabDialog_Impl : public SfxAbstractTabDialog
{
    DECL_ABSTDLG_BASE( AbstractTabDialog_Impl,SfxTabDialog );
    virtual void                SetCurPageId( USHORT nId );
    virtual const SfxItemSet*   GetOutputItemSet() const;
    virtual const USHORT*       GetInputRanges( const SfxItemPool& pItem ); //add by CHINA001
    virtual void                SetInputSet( const SfxItemSet* pInSet );   //add by CHINA001
        //From class Window.
    virtual void        SetText( const XubString& rStr ); //add by CHINA001
    virtual String      GetText() const; //add by CHINA001
};
//add for SdCharDlg end

// add for AssistentDlg
class AssistentDlg;
class AbstractAssistentDlg_Impl : public AbstractAssistentDlg
{
    DECL_ABSTDLG_BASE(AbstractAssistentDlg_Impl,AssistentDlg);
    virtual SfxObjectShellLock GetDocument();
    virtual OutputType GetOutputMedium() const;
    virtual BOOL IsSummary() const;
    virtual StartType GetStartType() const;
    virtual String GetDocPath() const;
    virtual BOOL GetStartWithFlag() const;
    virtual BOOL IsDocEmpty() const;
    virtual String GetPassword();
};

// add for SdModifyFieldDlg
class SdModifyFieldDlg;
class AbstractSdModifyFieldDlg_Impl : public AbstractSdModifyFieldDlg
{
    DECL_ABSTDLG_BASE(AbstractSdModifyFieldDlg_Impl,SdModifyFieldDlg);
    virtual SvxFieldData*       GetField();
    virtual SfxItemSet          GetItemSet();
};

// add for SdSnapLineDlg
class SdSnapLineDlg;
class AbstractSdSnapLineDlg_Impl : public AbstractSdSnapLineDlg
{
    DECL_ABSTDLG_BASE(AbstractSdSnapLineDlg_Impl,SdSnapLineDlg);
    virtual void GetAttr(SfxItemSet& rOutAttrs);
    virtual void HideRadioGroup();
    virtual void HideDeleteBtn();
    virtual void SetInputFields(BOOL bEnableX, BOOL bEnableY);
    //from class Window
    virtual void    SetText( const XubString& rStr );
};

// add for SdInsertLayerDlg
class SdInsertLayerDlg;
class AbstractSdInsertLayerDlg_Impl : public AbstractSdInsertLayerDlg
{
    DECL_ABSTDLG_BASE(AbstractSdInsertLayerDlg_Impl,SdInsertLayerDlg);
    virtual void    GetAttr( SfxItemSet& rOutAttrs ) ;
    //from class Window
    virtual void    SetHelpId( ULONG nHelpId ) ;
};

// add for SdInsertPasteDlg
class SdInsertPasteDlg;
class AbstractSdInsertPasteDlg_Impl : public AbstractSdInsertPasteDlg
{
    DECL_ABSTDLG_BASE(AbstractSdInsertPasteDlg_Impl,SdInsertPasteDlg);
    virtual BOOL            IsInsertBefore() const;
};

// add for SdInsertPagesObjsDlg
class SdInsertPagesObjsDlg;
class AbstractSdInsertPagesObjsDlg_Impl : public AbstractSdInsertPagesObjsDlg
{
    DECL_ABSTDLG_BASE(AbstractSdInsertPagesObjsDlg_Impl,SdInsertPagesObjsDlg);
    virtual ::Window *  GetWindow(); //this method is added for return a Window type pointer
    virtual List*       GetList( USHORT nType );
    virtual BOOL        IsLink();
    virtual BOOL        IsRemoveUnnessesaryMasterPages() const;
};

// add for MorphDlg
class AbstractMorphDlg_Impl : public AbstractMorphDlg
{
    DECL_ABSTDLG_BASE(AbstractMorphDlg_Impl,::sd::MorphDlg);
    virtual void            SaveSettings() const;
    virtual USHORT          GetFadeSteps() const;
    virtual BOOL            IsAttributeFade() const ;
    virtual BOOL            IsOrientationFade() const ;
};

// add for SdNewFoilDlg
class SdNewFoilDlg;
class AbstractSdNewFoilDlg_Impl : public AbstractSdNewFoilDlg
{
    DECL_ABSTDLG_BASE(AbstractSdNewFoilDlg_Impl,SdNewFoilDlg);
    virtual void    GetAttr( SfxItemSet& rOutAttrs );
};

// add for SdStartPresentationDlg
class SdStartPresentationDlg;
class AbstractSdStartPresDlg_Impl : public AbstractSdStartPresDlg
{
    DECL_ABSTDLG_BASE(AbstractSdStartPresDlg_Impl,SdStartPresentationDlg);
    virtual void    GetAttr( SfxItemSet& rOutAttrs );
};

// add for SdPrintDlg
class SdPrintDlg;
class AbstractSdPrintDlg_Impl : public AbstractSdPrintDlg
{
    DECL_ABSTDLG_BASE(AbstractSdPrintDlg_Impl,SdPrintDlg);
    virtual USHORT  GetAttr();
};

// add for SdPresLayoutDlg
class SdPresLayoutDlg;
class AbstractSdPresLayoutDlg_Impl : public AbstractSdPresLayoutDlg
{
    DECL_ABSTDLG_BASE(AbstractSdPresLayoutDlg_Impl,SdPresLayoutDlg);
    virtual void    GetAttr(SfxItemSet& rOutAttrs);
};

// add for SdActionDlg
class SfxSingleTabDialog;
class AbstractSfxSingleTabDialog_Impl :public AbstractSfxSingleTabDialog
{
    DECL_ABSTDLG_BASE(AbstractSfxSingleTabDialog_Impl,SfxSingleTabDialog);
    virtual const SfxItemSet*   GetOutputItemSet() const;
};

// add for SdVectorizeDlg
class SdVectorizeDlg;
class AbstractSdVectorizeDlg_Impl :public AbstractSdVectorizeDlg
{
    DECL_ABSTDLG_BASE(AbstractSdVectorizeDlg_Impl,SdVectorizeDlg);
    virtual const GDIMetaFile&  GetGDIMetaFile() const ;
};

// add for SdPublishingDlg
class SdPublishingDlg;
class AbstractSdPublishingDlg_Impl :public AbstractSdPublishingDlg
{
    DECL_ABSTDLG_BASE(AbstractSdPublishingDlg_Impl,SdPublishingDlg);
    virtual void GetParameterSequence( ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rParams );
};

//------------------------------------------------------------------------
//AbstractDialogFactory_Impl implementations
class SdAbstractDialogFactory_Impl : public SdAbstractDialogFactory
{

public:
    virtual VclAbstractDialog*          CreateBreakDlg( const ResId& rResId,
                                            ::Window* pWindow,
                                            ::sd::DrawView* pDrView,
                                            ::sd::DrawDocShell* pShell,
                                            ULONG nSumActionCount,
                                            ULONG nObjCount ); //add for BreakDlg
    virtual AbstractCopyDlg*            CreateCopyDlg( const ResId& rResId,
                                            ::Window* pWindow, const SfxItemSet& rInAttrs,
                                            XColorTable* pColTab, ::sd::View* pView ); //add for CopyDlg
    virtual AbstractSdCustomShowDlg*    CreateSdCustomShowDlg( const ResId& rResId,
                                            ::Window* pWindow, SdDrawDocument& rDrawDoc ); //add for SdCustomShowDlg
    virtual SfxAbstractTabDialog*       CreateSdTabDialog( const ResId& rResId,
                                            ::Window* pParent, const SfxItemSet* pAttr,
                                            SfxObjectShell* pDocShell, BOOL bAreaPage = TRUE ); //add for SdCharDlg, SdPageDlg
    virtual AbstractAssistentDlg*       CreateAssistentDlg( const ResId& rResId,
                                            ::Window* pParent, BOOL bAutoPilot); //add for AssistentDlg
    virtual AbstractSdModifyFieldDlg*   CreateSdModifyFieldDlg( const ResId& rResId,
                                            ::Window* pWindow, const SvxFieldData* pInField, const SfxItemSet& rSet ); //add for SdModifyFieldDlg
    virtual AbstractSdSnapLineDlg*      CreateSdSnapLineDlg( const ResId& rResId,
                                            ::Window* pWindow, const SfxItemSet& rInAttrs, ::sd::View* pView); //add for SdSnapLineDlg
    virtual AbstractSdInsertLayerDlg*   CreateSdInsertLayerDlg( const ResId& rResId,
                                            ::Window* pWindow,
                                            const SfxItemSet& rInAttrs,
                                            BOOL bDeletable,
                                            String aStr ); //add for SdInsertLayerDlg
    virtual AbstractSdInsertPasteDlg*   CreateSdInsertPasteDlg( const ResId& rResId, ::Window* pWindow ); //add for SdInsertPasteDlg
    virtual AbstractSdInsertPagesObjsDlg*   CreateSdInsertPagesObjsDlg( const ResId& rResId,
                                                ::Window* pParent,
                                                const SdDrawDocument* pDoc,
                                                SfxMedium* pSfxMedium,
                                                const String& rFileName ); //add for SdInsertPagesObjsDlg
    virtual AbstractMorphDlg*           CreateMorphDlg( const ResId& rResId,
                                                ::Window* pParent,
                                                const SdrObject* pObj1,
                                                const SdrObject* pObj2); //add for MorphDlg
    virtual AbstractSdNewFoilDlg*       CreateSdNewFoilDlg( const ResId& rResId,
                                                ::Window* pWindow,
                                                const SfxItemSet& rInAttrs,
                                                PageKind ePgKind,
                                                ::sd::DrawDocShell* pDocShell,
                                                BOOL bChangeFoil ); //add for SdNewFoilDlg
    virtual SfxAbstractTabDialog*       CreateSdItemSetTabDlg ( const ResId& rResId,
                                                ::Window* pParent,
                                                const SfxItemSet* pAttr,
                                                ::sd::View* pView = NULL ); //add for OutlineBulletDlg,SdParagraphDlg
    virtual AbstractSdStartPresDlg*     CreateSdStartPresentationDlg( const ResId& rResId,
                                                ::Window* pWindow,
                                                const SfxItemSet& rInAttrs,
                                                List& rPageNames,
                                                List* pCSList ); //add for SdStartPresentationDlg
    virtual AbstractSdPrintDlg*         CreateSdPrintDlg( const ResId& rResId, ::Window* pWindow ); //add for SdPrintDlg
    virtual SfxAbstractTabDialog*       CreateSdPresLayoutTemplateDlg( const ResId& rResId,
                                                SfxObjectShell* pDocSh, ::Window* pParent,
                                                SdResId DlgId, SfxStyleSheetBase& rStyleBase,
                                                PresentationObjects ePO, SfxStyleSheetBasePool* pSSPool ); //add for SdPresLayoutTemplateDlg
    virtual AbstractSdPresLayoutDlg*    CreateSdPresLayoutDlg( const ResId& rResId,
                                                ::sd::DrawDocShell* pDocShell,
                                                ::sd::ViewShell* pViewShell,
                                                ::Window* pWindow,
                                                const SfxItemSet& rInAttrs); //add for SdPresLayoutDlg
    virtual SfxAbstractTabDialog*       CreateSdTabTemplateDlg( const ResId& rResId,
                                                ::Window* pParent,
                                                const SfxObjectShell* pDocShell,
                                                SfxStyleSheetBase& rStyleBase,
                                                SdrModel* pModel,
                                                SdrView* pView ); //add for SdTabTemplateDlg
    virtual AbstractSfxSingleTabDialog* CreateSfxSingleTabDialog( const ResId& rResId,
                                                ::Window* pParent,
                                                const SfxItemSet* pAttr,
                                                ::sd::View* pView ); //add for SdActionDlg
    virtual AbstractSdVectorizeDlg*     CreateSdVectorizeDlg( const ResId& rResId,
                                                ::Window* pParent, const Bitmap& rBmp,
                                                ::sd::DrawDocShell* pDocShell ); //add for SdVectorizeDlg
    virtual AbstractSdPublishingDlg*    CreateSdPublishingDlg( const ResId& rResId,
                                                ::Window* pWindow, DocumentType eDocType); //add for SdPublishingDlg

    // For TabPage
    virtual CreateTabPage               GetTabPageCreatorFunc( USHORT nId );

    virtual GetTabPageRanges            GetTabPageRangesFunc( USHORT nId );

};

#endif

