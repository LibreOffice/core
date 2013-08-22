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
#ifndef _SW_ABSTDLG_HXX
#define _SW_ABSTDLG_HXX


#include <tools/solar.h>
#include <tools/string.hxx>
#include <sfx2/sfxdlg.hxx>
#include <svx/svxdlg.hxx>
#include <vcl/syswin.hxx>
#include <globals.hrc>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include "itabenum.hxx"

class SfxViewFrame;
class SfxBindings;
class SfxItemSet;
class ResId;
class Window;
class String;
class SfxItemPool;
class SfxStyleSheetBase;
class SwGlossaryHdl;
class SwField;

class SwLabFmtPage;
class SwLabRec;
class SwAsciiOptions;
class SwDocShell;
class SvStream;
class SwWrtShell;
class SfxRequest;
class SwView;
class SwTableAutoFmt;
class SwTOXMgr;
class SwForm;
struct CurTOXType;
class SwTOXDescription;
class SwTOXBase;
class SwSectionData;
struct SwDBData;
class SwField;
class Printer;
class SwLabItem;
class SwNewDBMgr;
class SwTableFUNC;
class SwChildWinWrapper;
struct SfxChildWinInfo;
class SwTOXMark;
struct SwDocStat;
#include <cnttab.hxx> //add for struct CurTOXType

namespace com{namespace sun{namespace star{
    namespace frame{
        class XFrame;
    }
    namespace sdbcx{
    class XColumnsSupplier;
    }
    namespace sdbc{
    class XDataSource;
    class XConnection;
    class XResultSet;
    }
}}}

typedef   void (*SwLabDlgMethod) (::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel>& xModel,   const SwLabItem& rItem);

typedef String      (*GlossaryGetCurrGroup)();
typedef void        (*GlossarySetActGroup)(const String& rNewGroup);

class AbstractGlossaryDlg : public VclAbstractDialog  //add for SwGlossaryDlg
{
public:
    virtual String          GetCurrGrpName() const = 0;
    virtual String          GetCurrShortName() const = 0;
};

class AbstractFldInputDlg : public VclAbstractDialog  //add for SwFldInputDlg
{
public:
    //from class SalFrame
    virtual void         SetWindowState( const OString & rStr ) = 0;
    virtual OString GetWindowState( sal_uLong nMask = WINDOWSTATE_MASK_ALL ) const = 0;
};

class AbstractInsFootNoteDlg : public VclAbstractDialog  //add for SwInsFootNoteDlg
{
public:
    virtual String          GetFontName() = 0;
    virtual sal_Bool            IsEndNote() = 0;
    virtual String          GetStr() = 0;
    //from class Window
    virtual void    SetHelpId( const OString& sHelpId ) = 0;
    virtual void    SetText( const OUString& rStr ) = 0;
};

class AbstractInsTableDlg : public VclAbstractDialog  //add for SwInsTableDlg
{
public:
    virtual void            GetValues( String& rName, sal_uInt16& rRow, sal_uInt16& rCol,
                                SwInsertTableOptions& rInsTblFlags, String& rTableAutoFmtName,
                                SwTableAutoFmt *& prTAFmt ) = 0;
};

class AbstractJavaEditDialog : public VclAbstractDialog  //add for SwJavaEditDialog
{
public:
    virtual OUString                GetText() = 0;
    virtual String              GetType() = 0;
    virtual sal_Bool                IsUrl() = 0;
    virtual sal_Bool                IsNew() = 0;
    virtual bool                IsUpdate() = 0;
};

class AbstractMailMergeDlg : public VclAbstractDialog  //add for SwMailMergeDlg
{
public:
    virtual sal_uInt16  GetMergeType() = 0;
    virtual const OUString& GetSaveFilter() const = 0;
    virtual const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > GetSelection() const = 0;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet> GetResultSet() const = 0;
    virtual bool IsSaveIndividualDocs() const = 0;
    virtual bool IsGenerateFromDataBase() const = 0;
    virtual String GetColumnName() const = 0;
    virtual String GetPath() const = 0;

};
class AbstractMailMergeCreateFromDlg : public VclAbstractDialog  //add for SwMailMergeCreateFromDlg
{
public:
    virtual sal_Bool    IsThisDocument() const = 0;
};
class AbstractMailMergeFieldConnectionsDlg : public VclAbstractDialog  //add for SwMailMergeFieldConnectionsDlg
{
public:
    virtual sal_Bool    IsUseExistingConnections() const = 0;
};

class AbstractMultiTOXTabDialog : public VclAbstractDialog  //add for SwMultiTOXTabDialog
{
public:
    virtual SwForm*             GetForm(CurTOXType eType) = 0;
    virtual CurTOXType          GetCurrentTOXType() const = 0;
    virtual SwTOXDescription&   GetTOXDescription(CurTOXType eTOXTypes) = 0;
    //from SfxTabDialog
    virtual const SfxItemSet*   GetOutputItemSet() const = 0;
};

class AbstractEditRegionDlg : public VclAbstractDialog  //add for SwEditRegionDlg
{
public:
    virtual void    SelectSection(const String& rSectionName) = 0;
};
class AbstractInsertSectionTabDialog : public VclAbstractDialog  //add for SwInsertSectionTabDialog
{
public:
    virtual void     SetSectionData(SwSectionData const& rSect) = 0;
};

class AbstractSwWordCountFloatDlg : public VclAbstractDialog
{
public:
    virtual void        UpdateCounts() = 0;
    virtual void        SetCounts(const SwDocStat &rCurrCnt, const SwDocStat &rDocStat) = 0;
    virtual Window *    GetWindow() = 0; //this method is added for return a Window type pointer
};

class AbstractSwInsertAbstractDlg : public VclAbstractDialog    // add for SwInsertAbstractDlg
{
public:
    virtual sal_uInt8   GetLevel() const = 0;
    virtual sal_uInt8   GetPara() const = 0;
};

class AbstractSwAsciiFilterDlg : public VclAbstractDialog // add for SwAsciiFilterDlg
{
public:
    virtual void FillOptions( SwAsciiOptions& rOptions ) = 0;

};

class AbstractSwBreakDlg : public VclAbstractDialog // add for SwBreakDlg
{
public:
    virtual String  GetTemplateName() = 0;
    virtual sal_uInt16  GetKind() = 0;
    virtual sal_uInt16  GetPageNumber() = 0;

};

class AbstractSplitTableDialog : public VclAbstractDialog // add for
{
public:
    virtual sal_uInt16 GetSplitMode() = 0;
};

class AbstractSwConvertTableDlg :  public VclAbstractDialog // add for SwConvertTableDlg
{
public:
    virtual void GetValues( sal_Unicode& rDelim,
                    SwInsertTableOptions& rInsTblFlags,
                    SwTableAutoFmt const*& prTAFmt ) = 0;
};

class AbstractSwInsertDBColAutoPilot :  public VclAbstractDialog // add for SwInsertDBColAutoPilot
{
public:

    virtual void DataToDoc( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& rSelection,
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource> rxSource,
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> xConnection,
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet > xResultSet) = 0;
};

class AbstractDropDownFieldDialog : public VclAbstractDialog //add for DropDownFieldDialog
{
public:
     virtual OString GetWindowState( sal_uLong nMask = WINDOWSTATE_MASK_ALL ) const = 0; //this method inherit from SystemWindow
     virtual void         SetWindowState( const OString & rStr ) =0;//this method inherit from SystemWindow
};

class AbstractSwLabDlg  : public SfxAbstractTabDialog //add for SwLabDlg
{
public:
     virtual const OUString& GetBusinessCardStr() const = 0;
     virtual Printer *GetPrt() =0;
};

class AbstractSwSelGlossaryDlg : public VclAbstractDialog //add for SwSelGlossaryDlg
{
public:
    virtual void InsertGlos(const String &rRegion, const String &rGlosName) = 0;    // inline
    virtual sal_uInt16 GetSelectedIdx() const = 0;  // inline
    virtual void SelectEntryPos(sal_uInt16 nIdx) = 0;   // inline
};

class AbstractSwAutoFormatDlg : public VclAbstractDialog //add for SwAutoFormatDlg
{
public:
    virtual void FillAutoFmtOfIndex( SwTableAutoFmt*& rToFill ) const = 0;
};

class AbstractSwFldDlg : public SfxAbstractTabDialog //add for SwFldDlg
{
public:
    virtual void                Start( sal_Bool bShow = sal_True ) = 0;  //this method from sfxtabdialog
    virtual void                Initialize(SfxChildWinInfo *pInfo) = 0;
    virtual void                ReInitDlg() = 0;
    virtual void                ActivateDatabasePage() = 0;
    virtual void                ShowReferencePage() = 0;
    virtual Window *            GetWindow() = 0; //this method is added for return a Window type pointer
};

class AbstractSwRenameXNamedDlg : public VclAbstractDialog //add for SwRenameXNamedDlg
{
public:
    virtual void    SetForbiddenChars( const String& rSet ) = 0;
    virtual void SetAlternativeAccess(
             ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > & xSecond,
             ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > & xThird ) = 0;
};

class AbstractSwModalRedlineAcceptDlg : public VclAbstractDialog //add for SwModalRedlineAcceptDlg
{
public:
    virtual void            AcceptAll( sal_Bool bAccept ) = 0;
};

class AbstractMarkFloatDlg : public VclAbstractDialog     //add for SwIndexMarkFloatDlg & SwAuthMarkFloatDlg
{
public:
    virtual void    ReInitDlg(SwWrtShell& rWrtShell) = 0;
    virtual Window *    GetWindow() = 0; //this method is added for return a Window type pointer
};

#define RET_LOAD_DOC            100
#define RET_EDIT_DOC            101
#define RET_EDIT_RESULT_DOC     102
#define RET_TARGET_CREATED      103
#define RET_REMOVE_TARGET       104
#define RET_APPLY_TEMPLATE      105

class SwView;
class SwMailMergeConfigItem;

class AbstractMailMergeWizard : public VclAbstractDialog2
{
public:
    virtual void                SetReloadDocument(const String& rURL) = 0;
    virtual const String&       GetReloadDocument() const = 0;
    virtual sal_Bool                ShowPage( sal_uInt16 nLevel ) = 0;
    virtual sal_uInt16          GetRestartPage() const = 0;
};


// Swabstract fractory
class SwAbstractDialogFactory
{
public:
    static SwAbstractDialogFactory*     Create();

    virtual SfxAbstractDialog*              CreateSfxDialog( Window* pParent, //add for SvxMeasureDialog & SvxConnectionDialog
                                                                        const SfxItemSet& rAttr,
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& _rxFrame,
                                                                        sal_uInt32 nResId
                                                                        ) = 0;
    virtual AbstractSwWordCountFloatDlg* CreateSwWordCountDialog(SfxBindings* pBindings,
        SfxChildWindow* pChild, Window *pParent, SfxChildWinInfo* pInfo) = 0;

    virtual AbstractSwInsertAbstractDlg * CreateSwInsertAbstractDlg(Window* pParent) = 0; // add for SwInsertAbstractDlg
    virtual AbstractSwAsciiFilterDlg*  CreateSwAsciiFilterDlg ( Window* pParent, SwDocShell& rDocSh,
                                                                SvStream* pStream ) = 0;// add for SwAsciiFilterDlg
    virtual VclAbstractDialog * CreateSwInsertBookmarkDlg( Window *pParent, SwWrtShell &rSh, SfxRequest& rReq, int nResId ) = 0;// add for SwInsertBookmarkDlg

    virtual AbstractSwBreakDlg * CreateSwBreakDlg(Window *pParent, SwWrtShell &rSh) = 0; // add for SwBreakDlg
    virtual VclAbstractDialog   * CreateSwChangeDBDlg(SwView& rVw) = 0; //add for SwChangeDBDlg
    virtual SfxAbstractTabDialog *  CreateSwCharDlg(Window* pParent, SwView& pVw, const SfxItemSet& rCoreSet,
        sal_uInt8 nDialogMode, const String* pFmtStr = 0) = 0;  // add for SwCharDlg
    virtual AbstractSwConvertTableDlg* CreateSwConvertTableDlg(SwView& rView, bool bToTable) = 0; //add for SwConvertTableDlg
    virtual VclAbstractDialog * CreateSwCaptionDialog ( Window *pParent, SwView &rV,int nResId) = 0; //add for SwCaptionDialog

    virtual AbstractSwInsertDBColAutoPilot* CreateSwInsertDBColAutoPilot( SwView& rView, // add for SwInsertDBColAutoPilot
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource> rxSource,
        com::sun::star::uno::Reference<com::sun::star::sdbcx::XColumnsSupplier> xColSupp,
        const SwDBData& rData,  int nResId) = 0;
    virtual SfxAbstractTabDialog * CreateSwFootNoteOptionDlg(Window *pParent, SwWrtShell &rSh) = 0;//add for  SwFootNoteOptionDlg

    //add for DropDownFieldDialog
    virtual AbstractDropDownFieldDialog * CreateDropDownFieldDialog(Window *pParent, SwWrtShell &rSh,
        SwField* pField, sal_Bool bNextButton = sal_False) = 0;
    virtual SfxAbstractTabDialog* CreateSwEnvDlg ( Window* pParent, const SfxItemSet& rSet, SwWrtShell* pWrtSh, Printer* pPrt, sal_Bool bInsert ) = 0; //add for SwEnvDlg


    virtual AbstractSwLabDlg* CreateSwLabDlg(Window* pParent, const SfxItemSet& rSet, //add for SwLabDlg
                                                     SwNewDBMgr* pNewDBMgr, bool bLabel) = 0;

    virtual SwLabDlgMethod GetSwLabDlgStaticMethod () =0;//add for SwLabDlg

    virtual SfxAbstractTabDialog* CreateSwParaDlg ( Window *pParent, //add for SwParaDlg
                                                    SwView& rVw,
                                                    const SfxItemSet& rCoreSet,
                                                    sal_uInt8 nDialogMode,
                                                    const String *pCollName = 0,
                                                    sal_Bool bDraw = sal_False,
                                                    OString sDefPage = OString() ) = 0;

    virtual AbstractSwSelGlossaryDlg * CreateSwSelGlossaryDlg(Window * pParent, const String &rShortName) = 0; //add for SwSelGlossaryDlg

    virtual VclAbstractDialog * CreateVclAbstractDialog ( Window * pParent, SwWrtShell &rSh, int nResId ) = 0; //add for  SwTableHeightDlg SwSortDlg
    virtual AbstractSplitTableDialog * CreateSplitTblDialog ( Window * pParent, SwWrtShell &rSh ) = 0; //add for  SwSplitTblDlg

    virtual AbstractSwAutoFormatDlg * CreateSwAutoFormatDlg( Window* pParent, SwWrtShell* pShell, //add for SwAutoFormatDlg
                                                            sal_Bool bSetAutoFmt = sal_True,
                                                            const SwTableAutoFmt* pSelFmt = 0 ) = 0;
    virtual SfxAbstractDialog * CreateSwWrapDlg ( Window* pParent, SfxItemSet& rSet, SwWrtShell* pSh, sal_Bool bDrawMode, int nResId ) = 0; //add for SwWrapDlg

    virtual VclAbstractDialog * CreateSwTableWidthDlg(Window *pParent, SwTableFUNC &rFnc) = 0; //add for SwTableWidthDlg
    virtual SfxAbstractTabDialog* CreateSwTableTabDlg(Window* pParent, SfxItemPool& Pool,
        const SfxItemSet* pItemSet, SwWrtShell* pSh) = 0; //add for SwTableTabDlg

    virtual AbstractSwFldDlg * CreateSwFldDlg(SfxBindings* pB, SwChildWinWrapper* pCW, Window *pParent) = 0; //add for SwFldDlg
    virtual SfxAbstractDialog*   CreateSwFldEditDlg ( SwView& rVw, int nResId ) = 0; //add for SwFldEditDlg
    virtual AbstractSwRenameXNamedDlg * CreateSwRenameXNamedDlg(Window* pParent, //add for SwRenameXNamedDlg
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNamed > & xNamed,
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > & xNameAccess) = 0;
    virtual AbstractSwModalRedlineAcceptDlg * CreateSwModalRedlineAcceptDlg(Window *pParent) = 0;

    virtual VclAbstractDialog*          CreateTblMergeDialog( Window* pParent, sal_Bool& rWithPrev ) = 0; //add for SwMergeTblDlg
    virtual SfxAbstractTabDialog*       CreateFrmTabDialog(const OString &rDialogType,
                                                SfxViewFrame *pFrame, Window *pParent,
                                                const SfxItemSet& rCoreSet,
                                                bool bNewFrm = true,
                                                bool bFmt = false,
                                                OString sDefPage = OString(),
                                                const String* pFmtStr = 0) = 0;  //add for SwFrmDlg
    // @param nSlot
    // Identifies optional Slot by which the creation of the Template (Style) dialog is triggered.
    // Currently used, if nRegion == SFX_STYLE_FAMILY_PAGE in order to activate certain dialog pane
    virtual SfxAbstractApplyTabDialog*  CreateTemplateDialog(
                                                Window*             pParent,
                                                SfxStyleSheetBase&  rBase,
                                                sal_uInt16          nRegion,
                                                OString             sPage = OString(),
                                                SwWrtShell*         pActShell = 0,
                                                bool                bNew = false) = 0; //add for SwTemplateDlg
    virtual AbstractGlossaryDlg*        CreateGlossaryDlg(SfxViewFrame* pViewFrame,
                                                SwGlossaryHdl* pGlosHdl,
                                                SwWrtShell *pWrtShell) = 0; //add for SwGlossaryDlg
    virtual AbstractFldInputDlg*        CreateFldInputDlg(Window *pParent,
        SwWrtShell &rSh, SwField* pField, bool bNextButton = false) = 0; //add for SwFldInputDlg
    virtual AbstractInsFootNoteDlg*     CreateInsFootNoteDlg(Window * pParent,
        SwWrtShell &rSh, sal_Bool bEd = sal_False) = 0; //add for SwInsFootNoteDlg
    virtual VclAbstractDialog*          CreateTitlePageDlg ( Window * pParent ) = 0;
    virtual VclAbstractDialog *         CreateVclSwViewDialog(SwView& rView) = 0; //add for SwInsRowColDlg, SwLineNumberingDlg
    virtual AbstractInsTableDlg*        CreateInsTableDlg(SwView& rView) = 0; //add for SwInsTableDlg
    virtual AbstractJavaEditDialog*     CreateJavaEditDialog(Window* pParent,
        SwWrtShell* pWrtSh) = 0; //add for SwJavaEditDialog
    virtual AbstractMailMergeDlg*       CreateMailMergeDlg( int nResId,
                                                Window* pParent, SwWrtShell& rSh,
                                                 const String& rSourceName,
                                                const String& rTblName,
                                                sal_Int32 nCommandType,
                                                const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& xConnection,
                                                ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >* pSelection = 0 ) = 0; //add for SwMailMergeDlg
    virtual AbstractMailMergeCreateFromDlg*     CreateMailMergeCreateFromDlg(Window* pParent) = 0;
    virtual AbstractMailMergeFieldConnectionsDlg* CreateMailMergeFieldConnectionsDlg(Window* pParent) = 0; //add for SwMailMergeFieldConnectionsDlg
    virtual VclAbstractDialog*          CreateMultiTOXMarkDlg( int nResId,
                                                Window* pParent, SwTOXMgr &rTOXMgr ) = 0; //add for SwMultiTOXMarkDlg
    virtual SfxAbstractTabDialog*       CreateSwTabDialog( int nResId,
                                                Window* pParent,
                                                const SfxItemSet* pSwItemSet,
                                                SwWrtShell &) = 0; //add for SwSvxNumBulletTabDialog, SwOutlineTabDialog
    virtual AbstractMultiTOXTabDialog*      CreateMultiTOXTabDialog( int nResId,
                                                Window* pParent, const SfxItemSet& rSet,
                                                SwWrtShell &rShell,
                                                SwTOXBase* pCurTOX, sal_uInt16 nToxType = USHRT_MAX,
                                                sal_Bool bGlobal = sal_False) = 0; //add for SwMultiTOXTabDialog
    virtual AbstractEditRegionDlg*      CreateEditRegionDlg(Window* pParent, SwWrtShell& rWrtSh) = 0; //add for SwEditRegionDlg
    virtual AbstractInsertSectionTabDialog*     CreateInsertSectionTabDialog(
        Window* pParent, const SfxItemSet& rSet, SwWrtShell& rSh) = 0; //add for SwInsertSectionTabDialog
    virtual AbstractMarkFloatDlg*       CreateIndexMarkFloatDlg(
                                                    SfxBindings* pBindings,
                                                       SfxChildWindow* pChild,
                                                       Window *pParent,
                                                    SfxChildWinInfo* pInfo,
                                                       sal_Bool bNew=sal_True) = 0; //add for SwIndexMarkFloatDlg
    virtual AbstractMarkFloatDlg*       CreateAuthMarkFloatDlg(
                                                    SfxBindings* pBindings,
                                                       SfxChildWindow* pChild,
                                                       Window *pParent,
                                                    SfxChildWinInfo* pInfo,
                                                       sal_Bool bNew=sal_True) = 0; //add for SwAuthMarkFloatDlg
    virtual VclAbstractDialog *         CreateIndexMarkModalDlg(
                                                Window *pParent, SwWrtShell& rSh, SwTOXMark* pCurTOXMark) = 0; //add for SwIndexMarkModalDlg

    virtual AbstractMailMergeWizard*    CreateMailMergeWizard(SwView& rView, SwMailMergeConfigItem& rConfigItem) = 0;

    //add for static func in SwGlossaryDlg
    virtual GlossaryGetCurrGroup        GetGlossaryCurrGroupFunc() = 0;
    virtual GlossarySetActGroup         SetGlossaryActGroupFunc() = 0;

    // for tabpage
    virtual CreateTabPage               GetTabPageCreatorFunc( sal_uInt16 nId ) = 0;
    virtual GetTabPageRanges            GetTabPageRangesFunc( sal_uInt16 nId ) = 0;

protected:
    ~SwAbstractDialogFactory() {}
};

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
