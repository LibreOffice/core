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

#pragma once

#include <sfx2/objsh.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include <sfx2/viewsh.hxx>
#include <o3tl/deleter.hxx>
#include <comphelper/lok.hxx>
#include <comphelper/servicehelper.hxx>

#include <scdllapi.h>
#include <document.hxx>
#include <shellids.hxx>
#include <optutil.hxx>
#include <docuno.hxx>

#include <memory>
#include <string_view>
#include <unordered_map>
#include <map>

class ScRefreshTimerProtector;
class ScEditEngineDefaulter;
class SfxStyleSheetBasePool;
class SfxStyleSheetHint;
class INetURLObject;

class ScViewData;
class ScDocFunc;
class ScDrawLayer;
class ScTabViewShell;
class ScAutoStyleList;
class ScMarkData;
class ScPaintLockData;
class ScChangeAction;
class ScImportOptions;
class ScDocShellModificator;
class ScSheetSaveData;
class ScFlatBoolRowSegments;
struct ScColWidthParam;
class ScFormulaOptions;
namespace com::sun::star::script::vba { class XVBAScriptListener; }
namespace ooo::vba::excel { class XWorkbook; }
namespace com::sun::star::datatransfer { class XTransferable2; }
namespace sfx2 { class FileDialogHelper; }
struct DocShell_Impl;

typedef std::unordered_map< sal_uLong, sal_uLong > ScChangeActionMergeMap;

//enum ScDBFormat { SC_FORMAT_SDF, SC_FORMAT_DBF };

enum class LOKCommentNotificationType { Add, Modify, Remove };

extern "C" SAL_DLLPUBLIC_EXPORT bool TestImportDBF(SvStream &rStream);

                                    // Extra flags for Repaint
#define SC_PF_LINES         1
#define SC_PF_TESTMERGE     2
#define SC_PF_WHOLEROWS     4

class SC_DLLPUBLIC ScDocShell final: public SfxObjectShell, public SfxListener
{
    std::shared_ptr<ScDocument> m_pDocument;

    OUString            m_aDdeTextFmt;

    double              m_nPrtToScreenFactor;
    std::unique_ptr<DocShell_Impl> m_pImpl;
    std::unique_ptr<ScDocFunc> m_pDocFunc;

    bool                m_bHeaderOn;
    bool                m_bFooterOn;
    bool                m_bIsInplace:1;         // Is set by the View
    bool                m_bIsEmpty:1;
    bool                m_bIsInUndo:1;
    bool                m_bDocumentModifiedPending:1;
    bool                m_bUpdateEnabled:1;
    bool                m_bAreasChangedNeedBroadcast:1;
    sal_uInt16          m_nDocumentLock;
    sal_Int16           m_nCanUpdate;  // stores the UpdateDocMode from loading a document till update links

    std::unique_ptr<ScDBData> m_pOldAutoDBRange;

    std::unique_ptr<ScAutoStyleList>    m_pAutoStyleList;
    std::unique_ptr<ScPaintLockData>    m_pPaintLockData;
    std::unique_ptr<ScSheetSaveData>    m_pSheetSaveData;
    std::unique_ptr<ScFormatSaveData>   m_pFormatSaveData;

    std::unique_ptr<ScDocShellModificator, o3tl::default_delete<ScDocShellModificator>> m_pModificator; // #109979#; is used to load XML (created in BeforeXMLLoading and destroyed in AfterXMLLoading)

    css::uno::Reference< ooo::vba::excel::XWorkbook> mxAutomationWorkbookObject;

    // Only used by Vba helper functions
    css::uno::Reference<css::script::vba::XVBAScriptListener>   m_xVBAListener;
    css::uno::Reference<css::datatransfer::XTransferable2>      m_xClipData;

    SAL_DLLPRIVATE void          InitItems();
    SAL_DLLPRIVATE void          DoEnterHandler();
    SAL_DLLPRIVATE void          InitOptions(bool bForLoading);
    SAL_DLLPRIVATE void          ResetDrawObjectShell();

    /** Do things that need to be done before saving to our own format and
        necessary clean ups in dtor. */
    class SAL_DLLPRIVATE PrepareSaveGuard
    {
        public:
            explicit    PrepareSaveGuard( ScDocShell & rDocShell );
                        ~PrepareSaveGuard() COVERITY_NOEXCEPT_FALSE;
        private:
                        ScDocShell & mrDocShell;
    };

    SAL_DLLPRIVATE bool          LoadXML( SfxMedium* pMedium, const css::uno::Reference< css::embed::XStorage >& );
    SAL_DLLPRIVATE bool          SaveXML( SfxMedium* pMedium, const css::uno::Reference< css::embed::XStorage >& );
    SAL_DLLPRIVATE SCTAB         GetSaveTab();

    friend bool TestImportDBF(SvStream &rStream);

    SAL_DLLPRIVATE ErrCode       DBaseImport( const OUString& rFullFileName, rtl_TextEncoding eCharSet,
                                             std::map<SCCOL, ScColWidthParam>& aColWidthParam, ScFlatBoolRowSegments& rRowHeightsRecalc );
    SAL_DLLPRIVATE ErrCodeMsg    DBaseExport(
                                    const OUString& rFullFileName, rtl_TextEncoding eCharSet, bool& bHasMemo );

    SAL_DLLPRIVATE static bool       MoveFile( const INetURLObject& rSource, const INetURLObject& rDest );
    SAL_DLLPRIVATE static bool       KillFile( const INetURLObject& rURL );
    SAL_DLLPRIVATE static bool       IsDocument( const INetURLObject& rURL );

    SAL_DLLPRIVATE void          LockPaint_Impl(bool bDoc);
    SAL_DLLPRIVATE void          UnlockPaint_Impl(bool bDoc);
    SAL_DLLPRIVATE void          LockDocument_Impl(sal_uInt16 nNew);
    SAL_DLLPRIVATE void          UnlockDocument_Impl(sal_uInt16 nNew);

    SAL_DLLPRIVATE void          EnableSharedSettings( bool bEnable );
    SAL_DLLPRIVATE css::uno::Reference< css::frame::XModel > LoadSharedDocument();

    SAL_DLLPRIVATE void          UseSheetSaveEntries();

    SAL_DLLPRIVATE std::unique_ptr<ScDocFunc> CreateDocFunc();

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

public:
                    SFX_DECL_INTERFACE(SCID_DOC_SHELL)
                    SFX_DECL_OBJECTFACTORY();

private:
    /// SfxInterface initializer.
    static void InitInterface_Impl();

public:
    explicit        ScDocShell( const ScDocShell& rDocShell ) = delete;
    explicit        ScDocShell( const SfxModelFlags i_nSfxCreationFlags = SfxModelFlags::EMBEDDED_OBJECT, const std::shared_ptr<ScDocument>& pDoc = {} );
                    virtual ~ScDocShell() override;

    virtual SfxUndoManager*
                    GetUndoManager() override;

    virtual void    FillClass( SvGlobalName * pClassName,
                               SotClipboardFormatId * pFormat,
                               OUString * pFullTypeName,
                               sal_Int32 nFileFormat,
                               bool bTemplate = false ) const override;

    std::shared_ptr<sfx::IDocumentModelAccessor> GetDocumentModelAccessor() const override;
    virtual std::set<Color> GetDocColors() override;
    virtual std::shared_ptr<model::ColorSet> GetThemeColors() override;

    virtual bool    InitNew( const css::uno::Reference< css::embed::XStorage >& ) override;
    virtual bool    Load( SfxMedium& rMedium ) override;
    virtual bool    LoadFrom( SfxMedium& rMedium ) override;
    virtual bool    ConvertFrom( SfxMedium &rMedium ) override;
    virtual bool    LoadExternal( SfxMedium& rMedium ) override;
    virtual bool    Save() override;
    virtual bool    SaveAs( SfxMedium& rMedium ) override;
    virtual bool    ConvertTo( SfxMedium &rMedium ) override;
    virtual bool    PrepareClose( bool bUI = true ) override;
    virtual void    LoadStyles( SfxObjectShell &rSource ) override;

    virtual bool    DoSaveCompleted( SfxMedium * pNewStor=nullptr, bool bRegisterRecent=true ) override;     // SfxObjectShell
    virtual bool    QuerySlotExecutable( sal_uInt16 nSlotId ) override;

    virtual void    Draw(OutputDevice *, const JobSetup & rSetup, sal_uInt16 nAspect, bool bOutputForScreen) override;

    virtual void    SetVisArea( const tools::Rectangle & rVisArea ) override;

    virtual void    TerminateEditing() override;

    using SfxObjectShell::GetVisArea;
    virtual tools::Rectangle GetVisArea( sal_uInt16 nAspect ) const override;

    virtual Printer* GetDocumentPrinter() override;

    virtual void    SetModified( bool = true ) override;

    void            SetVisAreaOrSize( const tools::Rectangle& rVisArea );

    virtual std::shared_ptr<SfxDocumentInfoDialog> CreateDocumentInfoDialog(weld::Window* pParent, const SfxItemSet &rSet) override;

    void    GetDocStat( ScDocStat& rDocStat );

    const ScDocument& GetDocument() const { return *m_pDocument; }
    ScDocument&     GetDocument()   { return *m_pDocument; }
    ScDocFunc&      GetDocFunc()    { return *m_pDocFunc; }

    css::uno::Reference<css::datatransfer::XTransferable2> const & GetClipData() const { return m_xClipData; }
    void SetClipData(const css::uno::Reference<css::datatransfer::XTransferable2>& xTransferable) { m_xClipData = xTransferable; }

    SfxPrinter*     GetPrinter( bool bCreateIfNotExist = true );
    sal_uInt16      SetPrinter( VclPtr<SfxPrinter> const & pNewPrinter, SfxPrinterChangeFlags nDiffFlags = SFX_PRINTER_ALL );

    void            UpdateFontList();

    ScDrawLayer*    MakeDrawLayer();

    void            AsciiSave( SvStream& rStream, const ScImportOptions& rOpt, SCTAB nTab );

    void            Execute( SfxRequest& rReq );
    void            GetState( SfxItemSet &rSet );
    void            ExecutePageStyle ( const SfxViewShell& rCaller, SfxRequest& rReq, SCTAB nCurTab );
    void            GetStatePageStyle( SfxItemSet& rSet, SCTAB nCurTab );

    void            CompareDocument( ScDocument& rOtherDoc );
    void            MergeDocument( ScDocument& rOtherDoc, bool bShared = false, bool bCheckDuplicates = false, sal_uLong nOffset = 0, ScChangeActionMergeMap* pMergeMap = nullptr, bool bInverseMap = false );
    bool            MergeSharedDocument( ScDocShell* pSharedDocShell );

    ScChangeAction* GetChangeAction( const ScAddress& rPos );
    void            SetChangeComment( ScChangeAction* pAction, const OUString& rComment );
    void            ExecuteChangeCommentDialog( ScChangeAction* pAction, weld::Window* pParent, bool bPrevNext = true );
                    /// Protect/unprotect ChangeTrack and return <TRUE/> if
                    /// protection was successfully changed.
                    /// If bJustQueryIfProtected==sal_True protection is not
                    /// changed and <TRUE/> is returned if not protected or
                    /// password was entered correctly.
    bool            ExecuteChangeProtectionDialog( bool bJustQueryIfProtected = false );

    void            SetPrintZoom( SCTAB nTab, sal_uInt16 nScale, sal_uInt16 nPages );
    bool            AdjustPrintZoom( const ScRange& rRange );

    void            LoadStylesArgs( ScDocShell& rSource, bool bReplace, bool bCellStyles, bool bPageStyles );

    void            PageStyleModified( std::u16string_view rStyleName, bool bApi );

    void            NotifyStyle( const SfxStyleSheetHint& rHint );
    void            DoAutoStyle( const ScRange& rRange, const OUString& rStyle );

    static weld::Window*  GetActiveDialogParent();
    void            ErrorMessage(TranslateId pGlobStrId);
    bool            IsEditable() const;

    bool            AdjustRowHeight( SCROW nStartRow, SCROW nEndRow, SCTAB nTab );
    void            UpdateAllRowHeights( const ScMarkData* pTabMark = nullptr );
    void            UpdateAllRowHeights(const bool bOnlyUsedRows);
    void            UpdatePendingRowHeights( SCTAB nUpdateTab, bool bBefore = false );

    void            RefreshPivotTables( const ScRange& rSource );
    void            DoConsolidate( const ScConsolidateParam& rParam, bool bRecord = true );
    void            UseScenario( SCTAB nTab, const OUString& rName, bool bRecord = true );
    SCTAB           MakeScenario(SCTAB nTab, const OUString& rName, const OUString& rComment,
                                    const Color& rColor, ScScenarioFlags nFlags,
                                    ScMarkData& rMark, bool bRecord = true);
    void            ModifyScenario(SCTAB nTab, const OUString& rName, const OUString& rComment,
                                    const Color& rColor, ScScenarioFlags nFlags);
    sal_uLong TransferTab( ScDocShell& rSrcDocShell, SCTAB nSrcPos,
                                SCTAB nDestPos, bool bInsertNew,
                                bool bNotifyAndPaint );

    bool            MoveTable( SCTAB nSrcTab, SCTAB nDestTab, bool bCopy, bool bRecord );

    void            DoRecalc( bool bApi );
    void            DoHardRecalc();

    void            UpdateOle(const ScViewData& rViewData, bool bSnapSize = false);
    bool            IsOle() const;

    void            DBAreaDeleted( SCTAB nTab, SCCOL nX1, SCROW nY1, SCCOL nX2 );
    ScDBData*       GetDBData( const ScRange& rMarked, ScGetDBMode eMode, ScGetDBSelection eSel );
    ScDBData*       GetAnonymousDBData(const ScRange& rRange);
    std::unique_ptr<ScDBData> GetOldAutoDBRange();
    void            CancelAutoDBRange();    // called when dialog is cancelled

    virtual void    ReconnectDdeLink(SfxObjectShell& rServer) override;
    void            UpdateLinks() override;
    void            SetInitialLinkUpdate( const SfxMedium* pMedium );
    void            AllowLinkUpdate();
    void            ReloadAllLinks();
    void            ReloadTabLinks();
    ScLkUpdMode     GetLinkUpdateModeState() const;

    void            SetFormulaOptions( const ScFormulaOptions& rOpt, bool bForLoading = false );
    /**
     * Called when the Options dialog is dismissed with the OK button, to
     * handle potentially conflicting option settings.
     */
    void            CheckConfigOptions();

    void            PostEditView( ScEditEngineDefaulter* pEditEngine, const ScAddress& rCursorPos );

    tools::Long     GetTwipWidthHint(const ScAddress& rPos);

    void            PostPaint( SCCOL nStartCol, SCROW nStartRow, SCTAB nStartTab,
                            SCCOL nEndCol, SCROW nEndRow, SCTAB nEndTab, PaintPartFlags nPart,
                            sal_uInt16 nExtFlags = 0, tools::Long nMaxWidthAffectedHint = -1 );
    void            PostPaint( const ScRangeList& rRanges, PaintPartFlags nPart, sal_uInt16 nExtFlags = 0,
                               tools::Long nMaxWidthAffectedHint = -1 );

    void            PostPaintCell( SCCOL nCol, SCROW nRow, SCTAB nTab, tools::Long nMaxWidthAffectedHint = -1);
    void            PostPaintCell( const ScAddress& rPos, tools::Long nMaxWidthAffectedHint = -1);
    void            PostPaintGridAll();
    void            PostPaintExtras();

    bool            IsPaintLocked() const { return m_pPaintLockData != nullptr; }

    void            PostDataChanged();

    void            UpdatePaintExt( sal_uInt16& rExtFlags, SCCOL nStartCol, SCROW nStartRow, SCTAB nStartTab,
                                                       SCCOL nEndCol, SCROW nEndRow, SCTAB nEndTab );
    void            UpdatePaintExt( sal_uInt16& rExtFlags, const ScRange& rRange );

    void            SetDocumentModified();
    void            SetDrawModified();

    void            LockPaint();
    void            UnlockPaint();
    sal_uInt16          GetLockCount() const { return m_nDocumentLock;}
    void            SetLockCount(sal_uInt16 nNew);

    void            LockDocument();
    void            UnlockDocument();

    DECL_DLLPRIVATE_LINK( DialogClosedHdl, sfx2::FileDialogHelper*, void );
    DECL_DLLPRIVATE_LINK( ReloadAllLinksHdl, weld::Button&, void );

    virtual SfxStyleSheetBasePool*  GetStyleSheetPool() override;

    void            SetInplace( bool bInplace );
    bool            IsEmpty() const { return m_bIsEmpty; }
    void            SetEmpty(bool bSet);

    bool            IsInUndo() const                { return m_bIsInUndo; }
    void            SetInUndo(bool bSet);

    void            CalcOutputFactor();
    double          GetOutputFactor() const { return m_nPrtToScreenFactor;}
    void            GetPageOnFromPageStyleSet( const SfxItemSet* pStyleSet,
                                               SCTAB             nCurTab,
                                               bool&             rbHeader,
                                               bool&             rbFooter );

#if defined(_WIN32)
    virtual bool DdeGetData( const OUString& rItem, const OUString& rMimeType,
                                css::uno::Any & rValue ) override;
    virtual bool DdeSetData( const OUString& rItem, const OUString& rMimeType,
                                const css::uno::Any & rValue ) override;
#endif

    virtual ::sfx2::SvLinkSource* DdeCreateLinkSource( const OUString& rItem ) override;

    const OUString& GetDdeTextFmt() const { return m_aDdeTextFmt; }

    SfxBindings*    GetViewBindings();

    ScTabViewShell* GetBestViewShell( bool bOnlyVisible = true );

    void            SetDocumentModifiedPending( bool bVal )
                        { m_bDocumentModifiedPending = bVal; }
    bool            IsDocumentModifiedPending() const
                        { return m_bDocumentModifiedPending; }

    bool            IsUpdateEnabled() const
                        { return m_bUpdateEnabled; }
    void            SetUpdateEnabled(bool bValue)
                        { m_bUpdateEnabled = bValue; }

    void            SetAreasChangedNeedBroadcast()
                        { m_bAreasChangedNeedBroadcast = true; }

    OutputDevice*   GetRefDevice(); // WYSIWYG: Printer, otherwise VirtualDevice...

    static ScViewData* GetViewData();
    static SCTAB       GetCurTab();

    static ScDocShell* GetShellByNum( sal_uInt16 nDocNo );
    static OUString   GetOwnFilterName();
    static OUString   GetHtmlFilterName();
    static OUString   GetWebQueryFilterName();
    static OUString   GetAsciiFilterName();
    static OUString   GetLotusFilterName();
    static OUString   GetDBaseFilterName();
    static OUString   GetDifFilterName();
    static bool       HasAutomaticTableName( std::u16string_view rFilter );
    static void       LOKCommentNotify(LOKCommentNotificationType nType, const ScDocument& rDocument, const ScAddress& rPos, const ScPostIt* pNote);

    DECL_DLLPRIVATE_LINK( RefreshDBDataHdl, Timer*, void );

    void            BeforeXMLLoading();
    void            AfterXMLLoading(bool bRet);

    virtual HiddenInformation GetHiddenInformationState( HiddenInformation nStates ) override;

    ScSheetSaveData* GetSheetSaveData();
    ScFormatSaveData* GetFormatSaveData();

    static void ResetKeyBindings( ScOptionsUtil::KeyBindingType eType );

    // password protection for Calc (derived from SfxObjectShell)
    // see also:    FID_CHG_RECORD, SID_CHG_PROTECT
    virtual bool    IsChangeRecording() const override;
    virtual bool    HasChangeRecordProtection() const override;
    virtual void    SetChangeRecording( bool bActivate, bool bLockAllViews = false ) override;
    virtual void    SetProtectionPassword( const OUString &rPassword ) override;
    virtual bool    GetProtectionHash( /*out*/ css::uno::Sequence< sal_Int8 > &rPasswordHash ) override;

    void SnapVisArea( tools::Rectangle& rRect ) const;

    void RegisterAutomationWorkbookObject(css::uno::Reference< ooo::vba::excel::XWorkbook > const& xWorkbook);

    ScModelObj* GetModel() const { return static_cast<ScModelObj*>(SfxObjectShell::GetModel().get()); }
};

void UpdateAcceptChangesDialog();

typedef tools::SvRef<ScDocShell> ScDocShellRef;

/** Create before modifications of the document and destroy thereafter.
    Call SetDocumentModified() at an instance of this class instead of at
    ScDocShell.

    Remembers in the ctor ScDocument's AutoCalcShellDisabled and IdleDisabled,
    switches them off and restores them in the dtor, AutoCalcShellDisabled
    also before a ScDocShell::SetDocumentModified() call if necessary.
    In the dtor, if ScDocShell's bDocumentModifiedPending is set and
    bAutoCalcShellDisabled is not set, then ScDocShell::SetDocumentModified()
    is called.

    Several instances can be used in nested calls to ScDocFunc or ScDocShell
    methods to avoid multiple modified status changes, only the last instance
    destroyed calls ScDocShell::SetDocumentModified().
 */
class ScDocShellModificator
{
    ScDocShell&     rDocShell;
    std::unique_ptr<ScRefreshTimerProtector> mpProtector;
    bool            bAutoCalcShellDisabled;
    bool            bIdleEnabled;

    ScDocShellModificator( const ScDocShellModificator& ) = delete;
    ScDocShellModificator&  operator=( const ScDocShellModificator& ) = delete;

public:
    explicit ScDocShellModificator( ScDocShell& );
    ~ScDocShellModificator() COVERITY_NOEXCEPT_FALSE;
    void            SetDocumentModified();
};

//#i97876# Spreadsheet data changes are not notified
namespace HelperNotifyChanges
{
    inline bool isDataAreaInvalidateType(std::u16string_view rType)
    {
        if (rType == u"delete-content")
            return true;
        if (rType == u"delete-rows")
            return true;
        if (rType == u"delete-columns")
            return true;
        if (rType == u"undo")
            return true;
        if (rType == u"redo")
            return true;
        if (rType == u"paste")
            return true;
        if (rType == u"note")
            return true;

        return false;
    }

    inline bool getMustPropagateChangesModel(ScModelObj* pModelObj)
    {
        return pModelObj && pModelObj->HasChangesListeners();
    }

    inline void Notify(ScModelObj &rModelObj, const ScRangeList &rChangeRanges,
        const OUString &rType = OUString("cell-change"),
        const css::uno::Sequence< css::beans::PropertyValue >& rProperties =
            css::uno::Sequence< css::beans::PropertyValue >())
    {
        rModelObj.NotifyChanges(rType, rChangeRanges, rProperties);
    }

    inline void NotifyIfChangesListeners(const ScDocShell &rDocShell, const ScRange &rRange,
        const OUString &rType = OUString("cell-change"))
    {
        ScModelObj* pModelObj = rDocShell.GetModel();
        ScRangeList aChangeRanges(rRange);

        if (getMustPropagateChangesModel(pModelObj))
            Notify(*pModelObj, aChangeRanges, rType);
        else if (pModelObj) // possibly need to invalidate getCellArea results
        {
            Notify(*pModelObj, aChangeRanges, isDataAreaInvalidateType(rType)
                ? OUString("data-area-invalidate") : OUString("data-area-extend"));
        }
    }
};

void VBA_InsertModule( ScDocument& rDoc, SCTAB nTab, const OUString& sModuleSource );

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
