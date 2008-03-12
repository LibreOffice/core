/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sddlgfact.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 11:37:19 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
class VclAbstractDialog_Impl : public VclAbstractDialog
{
    DECL_ABSTDLG_BASE(VclAbstractDialog_Impl,Dialog)
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
    virtual BOOL        IsModified() const ;
    virtual BOOL        IsCustomShow() const ;
};

//add for SdCharDlg begin
class SfxTabDialog;
class AbstractTabDialog_Impl : public SfxAbstractTabDialog
{
    DECL_ABSTDLG_BASE( AbstractTabDialog_Impl,SfxTabDialog )
    virtual void                SetCurPageId( USHORT nId );
    virtual const SfxItemSet*   GetOutputItemSet() const;
    virtual const USHORT*       GetInputRanges( const SfxItemPool& pItem );
    virtual void                SetInputSet( const SfxItemSet* pInSet );
        //From class Window.
    virtual void        SetText( const XubString& rStr );
    virtual String      GetText() const;
};
//add for SdCharDlg end


//add for OutlineBulletDlg begin
class SfxTabDialog;
class AbstractBulletDialog_Impl : public SfxAbstractTabDialog
{
    DECL_ABSTDLG_BASE( AbstractBulletDialog_Impl,SfxTabDialog )
    virtual void                SetCurPageId( USHORT nId );
    virtual const SfxItemSet*   GetOutputItemSet() const;
    virtual const USHORT*       GetInputRanges( const SfxItemPool& pItem );
    virtual void                SetInputSet( const SfxItemSet* pInSet );
        //From class Window.
    virtual void        SetText( const XubString& rStr );
    virtual String      GetText() const;
};
//add for OutlineBulletDlg end

class SdPresLayoutTemplateDlg;
class SdPresLayoutTemplateDlg_Impl : public SfxAbstractTabDialog
{
    DECL_ABSTDLG_BASE( SdPresLayoutTemplateDlg_Impl,SdPresLayoutTemplateDlg )
    virtual void                SetCurPageId( USHORT nId );
    virtual const SfxItemSet*   GetOutputItemSet() const;
    virtual const USHORT*       GetInputRanges( const SfxItemPool& pItem );
    virtual void                SetInputSet( const SfxItemSet* pInSet );
        //From class Window.
    virtual void        SetText( const XubString& rStr );
    virtual String      GetText() const;
};

// add for AssistentDlg
class AssistentDlg;
class AbstractAssistentDlg_Impl : public AbstractAssistentDlg
{
    DECL_ABSTDLG_BASE(AbstractAssistentDlg_Impl,AssistentDlg)
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
    virtual void SetInputFields(BOOL bEnableX, BOOL bEnableY);
    //from class Window
    virtual void    SetText( const XubString& rStr );
};

// add for SdInsertLayerDlg
class SdInsertLayerDlg;
class AbstractSdInsertLayerDlg_Impl : public AbstractSdInsertLayerDlg
{
    DECL_ABSTDLG_BASE(AbstractSdInsertLayerDlg_Impl,SdInsertLayerDlg)
    virtual void    GetAttr( SfxItemSet& rOutAttrs ) ;
    //from class Window
    virtual void    SetHelpId( ULONG nHelpId ) ;
};

// add for SdInsertPasteDlg
class SdInsertPasteDlg;
class AbstractSdInsertPasteDlg_Impl : public AbstractSdInsertPasteDlg
{
    DECL_ABSTDLG_BASE(AbstractSdInsertPasteDlg_Impl,SdInsertPasteDlg)
    virtual BOOL            IsInsertBefore() const;
};

// add for SdInsertPagesObjsDlg
class SdInsertPagesObjsDlg;
class AbstractSdInsertPagesObjsDlg_Impl : public AbstractSdInsertPagesObjsDlg
{
    DECL_ABSTDLG_BASE(AbstractSdInsertPagesObjsDlg_Impl,SdInsertPagesObjsDlg)
    virtual ::Window *  GetWindow(); //this method is added for return a Window type pointer
    virtual List*       GetList( USHORT nType );
    virtual BOOL        IsLink();
    virtual BOOL        IsRemoveUnnessesaryMasterPages() const;
};

// add for MorphDlg
class AbstractMorphDlg_Impl : public AbstractMorphDlg
{
    DECL_ABSTDLG_BASE(AbstractMorphDlg_Impl,::sd::MorphDlg)
    virtual void            SaveSettings() const;
    virtual USHORT          GetFadeSteps() const;
    virtual BOOL            IsAttributeFade() const ;
    virtual BOOL            IsOrientationFade() const ;
};

// add for SdStartPresentationDlg
class SdStartPresentationDlg;
class AbstractSdStartPresDlg_Impl : public AbstractSdStartPresDlg
{
    DECL_ABSTDLG_BASE(AbstractSdStartPresDlg_Impl,SdStartPresentationDlg)
    virtual void    GetAttr( SfxItemSet& rOutAttrs );
};

// add for SdPrintDlg
class SdPrintDlg;
class AbstractSdPrintDlg_Impl : public AbstractSdPrintDlg
{
    DECL_ABSTDLG_BASE(AbstractSdPrintDlg_Impl,SdPrintDlg)
    virtual USHORT  GetAttr();
};

// add for SdPresLayoutDlg
class SdPresLayoutDlg;
class AbstractSdPresLayoutDlg_Impl : public AbstractSdPresLayoutDlg
{
    DECL_ABSTDLG_BASE(AbstractSdPresLayoutDlg_Impl,SdPresLayoutDlg)
    virtual void    GetAttr(SfxItemSet& rOutAttrs);
};

// add for SdActionDlg
class SfxSingleTabDialog;
class AbstractSfxSingleTabDialog_Impl :public AbstractSfxSingleTabDialog
{
    DECL_ABSTDLG_BASE(AbstractSfxSingleTabDialog_Impl,SfxSingleTabDialog)
    virtual const SfxItemSet*   GetOutputItemSet() const;
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
    virtual VclAbstractDialog*          CreateBreakDlg(::Window* pWindow, ::sd::DrawView* pDrView, ::sd::DrawDocShell* pShell, ULONG nSumActionCount, ULONG nObjCount );
    virtual AbstractCopyDlg*            CreateCopyDlg( ::Window* pWindow, const SfxItemSet& rInAttrs, XColorTable* pColTab, ::sd::View* pView );
    virtual AbstractSdCustomShowDlg*    CreateSdCustomShowDlg( ::Window* pWindow, SdDrawDocument& rDrawDoc );
    virtual SfxAbstractTabDialog*       CreateSdTabCharDialog( ::Window* pParent, const SfxItemSet* pAttr, SfxObjectShell* pDocShell );
    virtual SfxAbstractTabDialog*       CreateSdTabPageDialog( ::Window* pParent, const SfxItemSet* pAttr, SfxObjectShell* pDocShell, BOOL bAreaPage = TRUE );
    virtual AbstractAssistentDlg*       CreateAssistentDlg( ::Window* pParent, BOOL bAutoPilot);
    virtual AbstractSdModifyFieldDlg*   CreateSdModifyFieldDlg( ::Window* pWindow, const SvxFieldData* pInField, const SfxItemSet& rSet );
    virtual AbstractSdSnapLineDlg*      CreateSdSnapLineDlg( ::Window* pWindow, const SfxItemSet& rInAttrs, ::sd::View* pView);
    virtual AbstractSdInsertLayerDlg*   CreateSdInsertLayerDlg( ::Window* pWindow, const SfxItemSet& rInAttrs, bool bDeletable, String aStr );
    virtual AbstractSdInsertPasteDlg*   CreateSdInsertPasteDlg( ::Window* pWindow );
    virtual AbstractSdInsertPagesObjsDlg* CreateSdInsertPagesObjsDlg( ::Window* pParent, const SdDrawDocument* pDoc, SfxMedium* pSfxMedium, const String& rFileName );
    virtual AbstractMorphDlg*           CreateMorphDlg( ::Window* pParent, const SdrObject* pObj1, const SdrObject* pObj2);
    virtual SfxAbstractTabDialog*       CreateSdOutlineBulletTabDlg ( ::Window* pParent, const SfxItemSet* pAttr, ::sd::View* pView = NULL );
    virtual SfxAbstractTabDialog*       CreateSdParagraphTabDlg ( ::Window* pParent, const SfxItemSet* pAttr );
    virtual AbstractSdStartPresDlg*     CreateSdStartPresentationDlg( ::Window* pWindow, const SfxItemSet& rInAttrs, List& rPageNames, List* pCSList );
    virtual AbstractSdPrintDlg*         CreateSdPrintDlg( ::Window* pWindow ); //add for SdPrintDlg
    virtual SfxAbstractTabDialog*       CreateSdPresLayoutTemplateDlg( SfxObjectShell* pDocSh, ::Window* pParent, SdResId DlgId, SfxStyleSheetBase& rStyleBase, PresentationObjects ePO, SfxStyleSheetBasePool* pSSPool );
    virtual AbstractSdPresLayoutDlg*    CreateSdPresLayoutDlg( ::sd::DrawDocShell* pDocShell, ::sd::ViewShell* pViewShell, ::Window* pWindow, const SfxItemSet& rInAttrs);
    virtual SfxAbstractTabDialog*       CreateSdTabTemplateDlg( ::Window* pParent, const SfxObjectShell* pDocShell, SfxStyleSheetBase& rStyleBase, SdrModel* pModel, SdrView* pView );
    virtual AbstractSfxSingleTabDialog* CreatSdActionDialog( ::Window* pParent, const SfxItemSet* pAttr, ::sd::View* pView );
    virtual AbstractSdVectorizeDlg*     CreateSdVectorizeDlg( ::Window* pParent, const Bitmap& rBmp, ::sd::DrawDocShell* pDocShell );
    virtual AbstractSdPublishingDlg*    CreateSdPublishingDlg( ::Window* pWindow, DocumentType eDocType);

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

