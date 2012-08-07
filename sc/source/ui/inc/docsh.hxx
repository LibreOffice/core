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

#ifndef SC_DOCSHELL_HXX
#define SC_DOCSHELL_HXX


#include <sfx2/objsh.hxx>

#include <sfx2/docfac.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include <sfx2/viewsh.hxx>

#include "scdllapi.h"
#include "scdll.hxx"
#include "document.hxx"
#include "appoptio.hxx"
#include "formulaopt.hxx"
#include "shellids.hxx"
#include "refreshtimer.hxx"
#include "optutil.hxx"

#include <boost/unordered_map.hpp>
#include <cppuhelper/implbase1.hxx>

#include <sot/sotref.hxx>

class ScEditEngineDefaulter;
class SfxStyleSheetBasePool;
class SfxStyleSheetHint;
class INetURLObject;

class ScViewData;
class ScDocFunc;
class ScDrawLayer;
class ScTabViewShell;
class ScSbxDocHelper;
class ScAutoStyleList;
class ScRange;
class ScMarkData;
class ScPaintLockData;
class ScChangeAction;
class VirtualDevice;
class ScImportOptions;
class ScDocShellModificator;
class ScOptSolverSave;
class ScSheetSaveData;
class ScFlatBoolRowSegments;
struct ScColWidthParam;
#ifdef ENABLE_TELEPATHY
class Collaboration;
#endif

namespace sfx2 { class FileDialogHelper; }
struct DocShell_Impl;

typedef ::boost::unordered_map< sal_uLong, sal_uLong > ScChangeActionMergeMap;

//==================================================================

//enum ScDBFormat { SC_FORMAT_SDF, SC_FORMAT_DBF };

                                    // Extra-Flags fuer Repaint
#define SC_PF_LINES         1
#define SC_PF_TESTMERGE     2
#define SC_PF_WHOLEROWS     4

class SC_DLLPUBLIC ScDocShell: public SfxObjectShell, public SfxListener
{
    static const sal_Char pStarCalcDoc[];
    static const sal_Char pStyleName[];

    ScDocument          aDocument;

    String              aDdeTextFmt;

    double              nPrtToScreenFactor;
    DocShell_Impl*      pImpl;
    ScDocFunc*          pDocFunc;

    bool                bHeaderOn;
    bool                bFooterOn;
    bool                bIsInplace:1;         // wird von der View gesetzt
    bool                bIsEmpty:1;
    bool                bIsInUndo:1;
    bool                bDocumentModifiedPending:1;
    bool                bUpdateEnabled:1;
    sal_uInt16          nDocumentLock;
    sal_Int16           nCanUpdate;  // stores the UpdateDocMode from loading a document till update links

    ScDBData*           pOldAutoDBRange;

    ScSbxDocHelper*     pDocHelper;

    ScAutoStyleList*    pAutoStyleList;
    ScPaintLockData*    pPaintLockData;
    ScOptSolverSave*    pSolverSaveData;
    ScSheetSaveData*    pSheetSaveData;

    ScDocShellModificator* pModificator; // #109979#; is used to load XML (created in BeforeXMLLoading and destroyed in AfterXMLLoading)
#ifdef ENABLE_TELEPATHY
    Collaboration*      mpCollaboration;
#endif

    SC_DLLPRIVATE void          InitItems();
    SC_DLLPRIVATE void          DoEnterHandler();
    SC_DLLPRIVATE void          InitOptions(bool bForLoading);
    SC_DLLPRIVATE void          ResetDrawObjectShell();

    // SUNWS needs a forward declared friend, otherwise types and members
    // of the outer class are not accessible.
    class PrepareSaveGuard;
    friend class ScDocShell::PrepareSaveGuard;
    /** Do things that need to be done before saving to our own format and
        necessary clean ups in dtor. */
    class PrepareSaveGuard
    {
        public:
            explicit    PrepareSaveGuard( ScDocShell & rDocShell );
                        ~PrepareSaveGuard();
        private:
                        ScDocShell & mrDocShell;
    };

    SC_DLLPRIVATE sal_Bool            LoadXML( SfxMedium* pMedium, const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& );
    SC_DLLPRIVATE sal_Bool            SaveXML( SfxMedium* pMedium, const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& );
    SC_DLLPRIVATE SCTAB         GetSaveTab();

    SC_DLLPRIVATE sal_uLong         DBaseImport( const String& rFullFileName, CharSet eCharSet,
                                             ScColWidthParam aColWidthParam[MAXCOLCOUNT], ScFlatBoolRowSegments& rRowHeightsRecalc );
    SC_DLLPRIVATE sal_uLong DBaseExport(
        const rtl::OUString& rFullFileName, CharSet eCharSet, bool& bHasMemo );

    SC_DLLPRIVATE static bool       MoveFile( const INetURLObject& rSource, const INetURLObject& rDest );
    SC_DLLPRIVATE static bool       KillFile( const INetURLObject& rURL );
    SC_DLLPRIVATE static bool       IsDocument( const INetURLObject& rURL );

    SC_DLLPRIVATE void          LockPaint_Impl(sal_Bool bDoc);
    SC_DLLPRIVATE void          UnlockPaint_Impl(sal_Bool bDoc);
    SC_DLLPRIVATE void          LockDocument_Impl(sal_uInt16 nNew);
    SC_DLLPRIVATE void          UnlockDocument_Impl(sal_uInt16 nNew);

    SC_DLLPRIVATE void          EnableSharedSettings( bool bEnable );
    SC_DLLPRIVATE ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > LoadSharedDocument();

    SC_DLLPRIVATE void          UseSheetSaveEntries();

    SC_DLLPRIVATE ScDocFunc    *CreateDocFunc();

protected:

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

public:
                    TYPEINFO();

                    SFX_DECL_INTERFACE(SCID_DOC_SHELL)
                    SFX_DECL_OBJECTFACTORY();

                    ScDocShell( const ScDocShell& rDocShell );
                    ScDocShell( const sal_uInt64 i_nSfxCreationFlags = SFXMODEL_EMBEDDED_OBJECT );
                    ~ScDocShell();

    using SotObject::GetInterface;
    using SfxShell::Activate;           // with sal_Bool bMDI
    using SfxShell::Deactivate;         // with sal_Bool bMDI
    using SfxObjectShell::Print;        // print styles

#ifdef ENABLE_TELEPATHY
    SC_DLLPRIVATE Collaboration* GetCollaboration();
#endif
    virtual void    Activate();
    virtual void    Deactivate();

    virtual ::svl::IUndoManager*
                    GetUndoManager();

    virtual void    FillClass( SvGlobalName * pClassName,
                               sal_uInt32 * pFormat,
                               String * pAppName,
                               String * pFullTypeName,
                               String * pShortTypeName,
                               sal_Int32 nFileFormat,
                               sal_Bool bTemplate = false ) const;

    virtual sal_Bool    InitNew( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& );
    virtual sal_Bool    Load( SfxMedium& rMedium );
    virtual sal_Bool    LoadFrom( SfxMedium& rMedium );
    virtual sal_Bool    ConvertFrom( SfxMedium &rMedium );
    virtual sal_Bool    Save();
    virtual sal_Bool    SaveAs( SfxMedium& rMedium );
    virtual sal_Bool    ConvertTo( SfxMedium &rMedium );
    virtual sal_uInt16  PrepareClose( sal_Bool bUI = sal_True, sal_Bool bForBrowsing = false );
    virtual void    PrepareReload();
    virtual sal_Bool    IsInformationLost();
    virtual void    LoadStyles( SfxObjectShell &rSource );
    virtual sal_Bool    Insert( SfxObjectShell &rSource,
                                sal_uInt16 nSourceIdx1, sal_uInt16 nSourceIdx2, sal_uInt16 nSourceIdx3,
                                sal_uInt16 &nIdx1, sal_uInt16 &nIdx2, sal_uInt16 &nIdx3, sal_uInt16 &rIdxDeleted );

    virtual sal_Bool    SaveCompleted( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& );      // SfxInPlaceObject
    virtual sal_Bool    DoSaveCompleted( SfxMedium * pNewStor);     // SfxObjectShell
    virtual sal_Bool QuerySlotExecutable( sal_uInt16 nSlotId );

    virtual void    Draw( OutputDevice *, const JobSetup & rSetup,
                                sal_uInt16 nAspect = ASPECT_CONTENT );

    virtual void    SetVisArea( const Rectangle & rVisArea );

    using SfxObjectShell::GetVisArea;
    virtual Rectangle GetVisArea( sal_uInt16 nAspect ) const;

    virtual Printer* GetDocumentPrinter();

    virtual void    SetModified( sal_Bool = sal_True );

    void            SetVisAreaOrSize( const Rectangle& rVisArea, sal_Bool bModifyStart );

    virtual SfxDocumentInfoDialog*  CreateDocumentInfoDialog( Window *pParent,
                                                              const SfxItemSet &rSet );

    void    GetDocStat( ScDocStat& rDocStat );

    ScDocument*     GetDocument()   { return &aDocument; }
    ScDocFunc&      GetDocFunc()    { return *pDocFunc; }
    void            SetDocFunc( ScDocFunc *pDF ) { pDocFunc = pDF; }

    SfxPrinter*     GetPrinter( sal_Bool bCreateIfNotExist = sal_True );
    sal_uInt16          SetPrinter( SfxPrinter* pNewPrinter, sal_uInt16 nDiffFlags = SFX_PRINTER_ALL );

    void            UpdateFontList();

    String          CreateObjectName( const String& rPrefix );

    ScDrawLayer*    MakeDrawLayer();

    void            AsciiSave( SvStream& rStream, const ScImportOptions& rOpt );

    void            GetSbxState( SfxItemSet &rSet );
    void            GetDrawObjState( SfxItemSet &rSet );

    void            Execute( SfxRequest& rReq );
    void            GetState( SfxItemSet &rSet );
    void            ExecutePageStyle ( SfxViewShell& rCaller, SfxRequest& rReq, SCTAB nCurTab );
    void            GetStatePageStyle( SfxViewShell& rCaller, SfxItemSet& rSet, SCTAB nCurTab );

    void            CompareDocument( ScDocument& rOtherDoc );
    void            MergeDocument( ScDocument& rOtherDoc, bool bShared = false, bool bCheckDuplicates = false, sal_uLong nOffset = 0, ScChangeActionMergeMap* pMergeMap = NULL, bool bInverseMap = false );
    bool            MergeSharedDocument( ScDocShell* pSharedDocShell );

    ScChangeAction* GetChangeAction( const ScAddress& rPos );
    void            SetChangeComment( ScChangeAction* pAction, const String& rComment );
    void            ExecuteChangeCommentDialog( ScChangeAction* pAction, Window* pParent,sal_Bool bPrevNext=sal_True );
                    /// Protect/unprotect ChangeTrack and return <TRUE/> if
                    /// protection was successfully changed.
                    /// If bJustQueryIfProtected==sal_True protection is not
                    /// changed and <TRUE/> is returned if not protected or
                    /// password was entered correctly.
    bool            ExecuteChangeProtectionDialog( Window* _pParent, sal_Bool bJustQueryIfProtected = false );

    void            SetPrintZoom( SCTAB nTab, sal_uInt16 nScale, sal_uInt16 nPages );
    bool            AdjustPrintZoom( const ScRange& rRange );

    void            LoadStylesArgs( ScDocShell& rSource, bool bReplace, bool bCellStyles, bool bPageStyles );

    void            PageStyleModified( const rtl::OUString& rStyleName, sal_Bool bApi );

    void            NotifyStyle( const SfxStyleSheetHint& rHint );
    void            DoAutoStyle( const ScRange& rRange, const String& rStyle );

    Window*         GetActiveDialogParent();
    void            ErrorMessage( sal_uInt16 nGlobStrId );
    sal_Bool            IsEditable() const;

    sal_Bool            AdjustRowHeight( SCROW nStartRow, SCROW nEndRow, SCTAB nTab );
    void            UpdateAllRowHeights( const ScMarkData* pTabMark = NULL );
    void            UpdatePendingRowHeights( SCTAB nUpdateTab, bool bBefore = false );

    void            RefreshPivotTables( const ScRange& rSource );
    void            DoConsolidate( const ScConsolidateParam& rParam, sal_Bool bRecord = sal_True );
    void            UseScenario( SCTAB nTab, const String& rName, sal_Bool bRecord = sal_True );
    SCTAB           MakeScenario( SCTAB nTab, const String& rName, const String& rComment,
                                    const Color& rColor, sal_uInt16 nFlags,
                                    ScMarkData& rMark, sal_Bool bRecord = sal_True );
    void            ModifyScenario( SCTAB nTab, const String& rName, const String& rComment,
                                    const Color& rColor, sal_uInt16 nFlags );
    sal_uLong TransferTab( ScDocShell& rSrcDocShell, SCTAB nSrcPos,
                                SCTAB nDestPos, sal_Bool bInsertNew,
                                sal_Bool bNotifyAndPaint );

    sal_Bool            MoveTable( SCTAB nSrcTab, SCTAB nDestTab, sal_Bool bCopy, sal_Bool bRecord );

    void            DoRecalc( bool bApi );
    void            DoHardRecalc( bool bApi );

    void            UpdateOle( const ScViewData* pViewData, sal_Bool bSnapSize = false);
    sal_Bool        IsOle();

    void            DBAreaDeleted( SCTAB nTab, SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2 );
    ScDBData*       GetDBData( const ScRange& rMarked, ScGetDBMode eMode, ScGetDBSelection eSel );
    ScDBData*       GetAnonymousDBData(const ScRange& rRange);
    ScDBData*       GetOldAutoDBRange();    // has to be deleted by caller!
    void            CancelAutoDBRange();    // called when dialog is cancelled

    virtual void    ReconnectDdeLink(SfxObjectShell& rServer);
    void            UpdateLinks();          // Link-Eintraege aktuallisieren
    sal_Bool            ReloadTabLinks();       // Links ausfuehren (Inhalt aktualisieren)

    void            SetFormulaOptions(const ScFormulaOptions& rOpt );
    virtual void    CheckConfigOptions();

    void            PostEditView( ScEditEngineDefaulter* pEditEngine, const ScAddress& rCursorPos );

    void            PostPaint( SCCOL nStartCol, SCROW nStartRow, SCTAB nStartTab,
                            SCCOL nEndCol, SCROW nEndRow, SCTAB nEndTab, sal_uInt16 nPart,
                            sal_uInt16 nExtFlags = 0 );
    void            PostPaint( const ScRangeList& rRanges, sal_uInt16 nPart, sal_uInt16 nExtFlags = 0 );

    void            PostPaintCell( SCCOL nCol, SCROW nRow, SCTAB nTab );
    void            PostPaintCell( const ScAddress& rPos );
    void            PostPaintGridAll();
    void            PostPaintExtras();

    bool            IsPaintLocked() const { return pPaintLockData != NULL; }

    void            PostDataChanged();

    void            UpdatePaintExt( sal_uInt16& rExtFlags, SCCOL nStartCol, SCROW nStartRow, SCTAB nStartTab,
                                                       SCCOL nEndCol, SCROW nEndRow, SCTAB nEndTab );
    void            UpdatePaintExt( sal_uInt16& rExtFlags, const ScRange& rRange );

    void            SetDocumentModified( sal_Bool bIsModified = sal_True );
    void            SetDrawModified( sal_Bool bIsModified = sal_True );

    void            LockPaint();
    void            UnlockPaint();
    sal_uInt16          GetLockCount() const;
    void            SetLockCount(sal_uInt16 nNew);

    void            LockDocument();
    void            UnlockDocument();

    DECL_LINK( DialogClosedHdl, sfx2::FileDialogHelper* );

    virtual SfxStyleSheetBasePool*  GetStyleSheetPool();

    void            SetInplace( bool bInplace );
    bool            IsEmpty() const;
    void            SetEmpty(bool bSet);

    bool            IsInUndo() const                { return bIsInUndo; }
    void            SetInUndo(bool bSet);

    void            CalcOutputFactor();
    double          GetOutputFactor() const;
    void            GetPageOnFromPageStyleSet( const SfxItemSet* pStyleSet,
                                               SCTAB             nCurTab,
                                               bool&             rbHeader,
                                               bool&             rbFooter );

    virtual long DdeGetData( const String& rItem, const String& rMimeType,
                                ::com::sun::star::uno::Any & rValue );
    virtual long DdeSetData( const String& rItem, const String& rMimeType,
                                const ::com::sun::star::uno::Any & rValue );
    virtual ::sfx2::SvLinkSource* DdeCreateLinkSource( const String& rItem );

    const String& GetDdeTextFmt() const { return aDdeTextFmt; }

    SfxBindings*    GetViewBindings();

    ScTabViewShell* GetBestViewShell( sal_Bool bOnlyVisible = sal_True );
    ScSbxDocHelper* GetDocHelperObject() { return pDocHelper; }

    void            SetDocumentModifiedPending( bool bVal )
                        { bDocumentModifiedPending = bVal; }
    bool            IsDocumentModifiedPending() const
                        { return bDocumentModifiedPending; }

    bool            IsUpdateEnabled() const
                        { return bUpdateEnabled; }
    void            SetUpdateEnabled(bool bValue)
                        { bUpdateEnabled = bValue; }

    OutputDevice*   GetRefDevice(); // WYSIWYG: Printer, otherwise VirtualDevice...

    static ScViewData* GetViewData();
    static SCTAB       GetCurTab();

    static ScDocShell* GetShellByNum( sal_uInt16 nDocNo );
    static String   GetOwnFilterName();
        static String   GetHtmlFilterName();
    static String   GetWebQueryFilterName();
    static String   GetAsciiFilterName();
    static String   GetLotusFilterName();
    static String   GetDBaseFilterName();
    static String   GetDifFilterName();
    static sal_Bool     HasAutomaticTableName( const String& rFilter );

    DECL_LINK( RefreshDBDataHdl, ScRefreshTimer* );

    void            BeforeXMLLoading();
    void            AfterXMLLoading(sal_Bool bRet);

    virtual sal_uInt16 GetHiddenInformationState( sal_uInt16 nStates );

    const ScOptSolverSave* GetSolverSaveData() const    { return pSolverSaveData; }     // may be null
    void            SetSolverSaveData( const ScOptSolverSave& rData );
    ScSheetSaveData* GetSheetSaveData();

    void ResetKeyBindings( ScOptionsUtil::KeyBindingType eType );

    // passwword protection for Calc (derived from SfxObjectShell)
    // see also:    FID_CHG_RECORD, SID_CHG_PROTECT
    virtual bool    IsChangeRecording() const;
    virtual bool    HasChangeRecordProtection() const;
    virtual void    SetChangeRecording( bool bActivate );
    virtual bool    SetProtectionPassword( const String &rPassword );
    virtual bool    GetProtectionHash( /*out*/ ::com::sun::star::uno::Sequence< sal_Int8 > &rPasswordHash );
};


void UpdateAcceptChangesDialog();


SO2_DECL_REF(ScDocShell)
SO2_IMPL_REF(ScDocShell)


// Vor Modifizierungen des Dokuments anlegen und danach zerstoeren.
// Merkt sich im Ctor AutoCalcShellDisabled und IdleDisabled, schaltet sie ab
// und stellt sie im Dtor wieder her, AutoCalcShellDisabled ggbf. auch vor
// einem ScDocShell SetDocumentModified.
// SetDocumentModified hierdran aufrufen statt an der ScDocShell.
// Im Dtor wird wenn ScDocShell bDocumentModifiedPending gesetzt ist und
// bAutoCalcShellDisabled nicht gesetzt ist ein SetDocumentModified gerufen.
class SC_DLLPUBLIC ScDocShellModificator
{
            ScDocShell&     rDocShell;
    ScRefreshTimerProtector aProtector;
            sal_Bool            bAutoCalcShellDisabled;
            sal_Bool            bIdleDisabled;

                            // not implemented
                            ScDocShellModificator( const ScDocShellModificator& );
    ScDocShellModificator&  operator=( const ScDocShellModificator& );

public:
                            ScDocShellModificator( ScDocShell& );
                            ~ScDocShellModificator();
            void            SetDocumentModified();
};



#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
