/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: docsh.hxx,v $
 *
 *  $Revision: 1.48 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 14:08:55 $
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

#ifndef SC_DOCSHELL_HXX
#define SC_DOCSHELL_HXX


#ifndef _SFX_OBJSH_HXX //autogen
#include <sfx2/objsh.hxx>
#endif

//REMOVE    #ifndef _SFX_INTERNO_HXX //autogen
//REMOVE    #include <sfx2/interno.hxx>
//REMOVE    #endif

#ifndef _SFX_OBJFAC_HXX //autogen
#include <sfx2/docfac.hxx>
#endif

#ifndef _SFXVIEWSH_HXX //autogen
#include <sfx2/viewsh.hxx>
#endif

#include <com/sun/star/frame/XLoadable.hpp>

#ifndef INCLUDED_SCDLLAPI_H
#include "scdllapi.h"
#endif

#ifndef SC_SCDLL_HXX
#include "scdll.hxx"
#endif

#ifndef SC_DOCUMENT_HXX
#include "document.hxx"
#endif

#ifndef SC_SHELLIDS_HXX
#include "shellids.hxx"
#endif

#ifndef SC_REFRESHTIMER_HXX
#include "refreshtimer.hxx"
#endif

class ScEditEngineDefaulter;
class FontList;
class PrintDialog;
class SfxStyleSheetBasePool;
class SfxStyleSheetHint;
struct ChartSelectionInfo;
class INetURLObject;

class ScPaintItem;
class ScViewData;
class ScDocFunc;
class ScPivot;
class ScDrawLayer;
class ScTabViewShell;
class ScSbxDocHelper;
class ScAutoStyleList;
class ScRange;
class ScMarkData;
class ScPaintLockData;
class ScJobSetup;
class ScChangeAction;
class VirtualDevice;
class ScImportOptions;
class ScDocShellModificator;
class ScOptSolverSave;

namespace sfx2 { class FileDialogHelper; }
struct DocShell_Impl;

//==================================================================

//enum ScDBFormat { SC_FORMAT_SDF, SC_FORMAT_DBF };

                                    // Extra-Flags fuer Repaint
#define SC_PF_LINES         1
#define SC_PF_TESTMERGE     2
#define SC_PF_WHOLEROWS     4

class SC_DLLPUBLIC ScDocShell: public SfxObjectShell, public SfxListener
{
    static const sal_Char __FAR_DATA pStarCalcDoc[];
    static const sal_Char __FAR_DATA pStyleName[];

    ScDocument          aDocument;

    String              aDdeTextFmt;
    String              aConvFilterName; //@ #BugId 54198

    double              nPrtToScreenFactor;
//!   FontList*           pFontList;
    DocShell_Impl*      pImpl;
    SfxUndoManager*     pUndoManager;
    ScDocFunc*          pDocFunc;

    //SfxObjectCreateMode   eShellMode;

    BOOL                bIsInplace;         // wird von der View gesetzt
    BOOL                bHeaderOn;
    BOOL                bFooterOn;
    BOOL                bNoInformLost;
    BOOL                bIsEmpty;
    BOOL                bIsInUndo;
    BOOL                bDocumentModifiedPending;
    USHORT              nDocumentLock;
    sal_Int16           nCanUpdate;  // stores the UpdateDocMode from loading a document till update links
    BOOL                bUpdateEnabled;

    ScDBData*           pOldAutoDBRange;

    ScSbxDocHelper*     pDocHelper;

    ScAutoStyleList*    pAutoStyleList;
    ScPaintLockData*    pPaintLockData;
    ScJobSetup*         pOldJobSetup;
    ScOptSolverSave*    pSolverSaveData;

    ScDocShellModificator* pModificator; // #109979#; is used to load XML (created in BeforeXMLLoading and destroyed in AfterXMLLoading)

    SC_DLLPRIVATE void          InitItems();
    SC_DLLPRIVATE void          DoEnterHandler();
    SC_DLLPRIVATE void          InitOptions();
    SC_DLLPRIVATE void          ResetDrawObjectShell();

    SC_DLLPRIVATE BOOL          GetTabParam( const SfxItemSet* pArgs, USHORT nId, SCTAB& rTab );
    //BOOL            LoadCalc( SotStorage* pStor );
    //BOOL            SaveCalc( SotStorage* pStor );
    SC_DLLPRIVATE BOOL            LoadXML( SfxMedium* pMedium, const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& );
    SC_DLLPRIVATE BOOL            SaveXML( SfxMedium* pMedium, const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& );
    SC_DLLPRIVATE SCTAB         GetSaveTab();

    SC_DLLPRIVATE void          RemoveUnknownObjects();

    SC_DLLPRIVATE ULONG         DBaseImport( const String& rFullFileName, CharSet eCharSet,
                                 BOOL bSimpleColWidth[MAXCOLCOUNT] );
    SC_DLLPRIVATE ULONG         DBaseExport( const String& rFullFileName, CharSet eCharSet,
                                 BOOL& bHasMemo );

    SC_DLLPRIVATE static BOOL       MoveFile( const INetURLObject& rSource, const INetURLObject& rDest );
    SC_DLLPRIVATE static BOOL       KillFile( const INetURLObject& rURL );
    SC_DLLPRIVATE static BOOL       IsDocument( const INetURLObject& rURL );

    SC_DLLPRIVATE void          LockPaint_Impl(BOOL bDoc);
    SC_DLLPRIVATE void          UnlockPaint_Impl(BOOL bDoc);
    SC_DLLPRIVATE void          LockDocument_Impl(USHORT nNew);
    SC_DLLPRIVATE void          UnlockDocument_Impl(USHORT nNew);

protected:

    virtual void SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                         const SfxHint& rHint, const TypeId& rHintType );

public:
                    TYPEINFO();

                    SFX_DECL_INTERFACE(SCID_DOC_SHELL)
                    SFX_DECL_OBJECTFACTORY();

                    ScDocShell( const ScDocShell& rDocShell );
                    ScDocShell( SfxObjectCreateMode eMode = SFX_CREATE_MODE_EMBEDDED, const bool _bScriptSupport = true );
                    ~ScDocShell();

    using SotObject::GetInterface;
    using SfxShell::Activate;           // with BOOL bMDI
    using SfxShell::Deactivate;         // with BOOL bMDI
    using SfxObjectShell::Print;        // print styles

    virtual void    Activate();
    virtual void    Deactivate();

    virtual SfxUndoManager*     GetUndoManager();

    virtual void    FillClass( SvGlobalName * pClassName,
                               sal_uInt32 * pFormat,
                               String * pAppName,
                               String * pFullTypeName,
                               String * pShortTypeName,
                               sal_Int32 nFileFormat ) const;

    virtual BOOL    InitNew( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& );
    virtual BOOL    Load( SfxMedium& rMedium );
    virtual BOOL    LoadFrom( SfxMedium& rMedium );
    virtual BOOL    ConvertFrom( SfxMedium &rMedium );
    virtual BOOL    Save();
    virtual BOOL    SaveAs( SfxMedium& rMedium );
    virtual BOOL    ConvertTo( SfxMedium &rMedium );
    virtual USHORT  PrepareClose( BOOL bUI = TRUE, BOOL bForBrowsing = FALSE );
    virtual void    PrepareReload();
    virtual BOOL    IsInformationLost();
    virtual void    LoadStyles( SfxObjectShell &rSource );
    virtual BOOL    Insert( SfxObjectShell &rSource,
                                USHORT nSourceIdx1, USHORT nSourceIdx2, USHORT nSourceIdx3,
                                USHORT &nIdx1, USHORT &nIdx2, USHORT &nIdx3, USHORT &rIdxDeleted );

    virtual BOOL    SaveCompleted( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& );      // SfxInPlaceObject
    virtual BOOL    DoSaveCompleted( SfxMedium * pNewStor);     // SfxObjectShell

    virtual void    Draw( OutputDevice *, const JobSetup & rSetup,
                                USHORT nAspect = ASPECT_CONTENT );

    virtual void    SetVisArea( const Rectangle & rVisArea );

    using SfxObjectShell::GetVisArea;
    virtual Rectangle GetVisArea( USHORT nAspect ) const;

    virtual Printer* GetDocumentPrinter();

    virtual void    SetModified( BOOL = TRUE );

    void            SetVisAreaOrSize( const Rectangle& rVisArea, BOOL bModifyStart );

    virtual SfxDocumentInfoDialog*  CreateDocumentInfoDialog( Window *pParent,
                                                              const SfxItemSet &rSet );

    void    GetDocStat( ScDocStat& rDocStat );

    ScDocument*     GetDocument()   { return &aDocument; }
    ScDocFunc&      GetDocFunc()    { return *pDocFunc; }

    SfxPrinter*     GetPrinter( BOOL bCreateIfNotExist = TRUE );
    USHORT          SetPrinter( SfxPrinter* pNewPrinter, USHORT nDiffFlags = SFX_PRINTER_ALL );

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
    void            MergeDocument( ScDocument& rOtherDoc, bool bShared = false, bool bCheckDuplicates = false, ULONG nOffset = 0 );

    ScChangeAction* GetChangeAction( const ScAddress& rPos );
    void            SetChangeComment( ScChangeAction* pAction, const String& rComment );
    void            ExecuteChangeCommentDialog( ScChangeAction* pAction, Window* pParent,BOOL bPrevNext=TRUE );
                    /// Protect/unprotect ChangeTrack and return <TRUE/> if
                    /// protection was successfully changed.
                    /// If bJustQueryIfProtected==TRUE protection is not
                    /// changed and <TRUE/> is returned if not protected or
                    /// password was entered correctly.
    BOOL            ExecuteChangeProtectionDialog( Window* _pParent, BOOL bJustQueryIfProtected = FALSE );

    void            SetPrintZoom( SCTAB nTab, USHORT nScale, USHORT nPages );
    BOOL            AdjustPrintZoom( const ScRange& rRange );

    void            LoadStylesArgs( ScDocShell& rSource, BOOL bReplace, BOOL bCellStyles, BOOL bPageStyles );

    void            PageStyleModified( const String& rStyleName, BOOL bApi );

    void            NotifyStyle( const SfxStyleSheetHint& rHint );
    void            DoAutoStyle( const ScRange& rRange, const String& rStyle );

    Window*         GetActiveDialogParent();
    void            ErrorMessage( USHORT nGlobStrId );
    BOOL            IsEditable() const;

    BOOL            AdjustRowHeight( SCROW nStartRow, SCROW nEndRow, SCTAB nTab );
    void            UpdateAllRowHeights();

    void            PivotUpdate( ScPivot* pOldPivot, ScPivot* pNewPivot,
                                    BOOL bRecord = TRUE, BOOL bApi = FALSE );
    void            RefreshPivotTables( const ScRange& rSource );
    void            DoConsolidate( const ScConsolidateParam& rParam, BOOL bRecord = TRUE );
    void            UseScenario( SCTAB nTab, const String& rName, BOOL bRecord = TRUE );
    SCTAB           MakeScenario( SCTAB nTab, const String& rName, const String& rComment,
                                    const Color& rColor, USHORT nFlags,
                                    ScMarkData& rMark, BOOL bRecord = TRUE );
    void            ModifyScenario( SCTAB nTab, const String& rName, const String& rComment,
                                    const Color& rColor, USHORT nFlags );
    BOOL            MoveTable( SCTAB nSrcTab, SCTAB nDestTab, BOOL bCopy, BOOL bRecord );

    void            DoRecalc( BOOL bApi );
    void            DoHardRecalc( BOOL bApi );

    bool            CheckPrint( PrintDialog* pPrintDialog, ScMarkData* pMarkData,
                                bool bForceSelected, bool bIsAPI );
    void            PreparePrint( PrintDialog* pPrintDialog, ScMarkData* pMarkData );
    void            Print( SfxProgress& rProgress, PrintDialog* pPrintDialog,
                            ScMarkData* pMarkData, Window* pDialogParent,
                            BOOL bForceSelected, BOOL bIsAPI );

    void            UpdateOle( const ScViewData* pViewData, BOOL bSnapSize = FALSE );
    BOOL            IsOle();

    void            DBAreaDeleted( SCTAB nTab, SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2 );
    ScDBData*       GetDBData( const ScRange& rMarked, ScGetDBMode eMode, BOOL bForceMark );
    ScDBData*       GetOldAutoDBRange();    // has to be deleted by caller!
    void            CancelAutoDBRange();    // called when dialog is cancelled

    void            UpdateLinks();          // Link-Eintraege aktuallisieren
    BOOL            ReloadTabLinks();       // Links ausfuehren (Inhalt aktualisieren)

    void            PostEditView( ScEditEngineDefaulter* pEditEngine, const ScAddress& rCursorPos );

    void            PostPaint( SCCOL nStartCol, SCROW nStartRow, SCTAB nStartTab,
                            SCCOL nEndCol, SCROW nEndRow, SCTAB nEndTab, USHORT nPart,
                            USHORT nExtFlags = 0 );
    void            PostPaint( const ScRange& rRange, USHORT nPart, USHORT nExtFlags = 0 );

    void            PostPaintCell( SCCOL nCol, SCROW nRow, SCTAB nTab );
    void            PostPaintGridAll();
    void            PostPaintExtras();

    void            PostDataChanged();

    void            UpdatePaintExt( USHORT& rExtFlags, SCCOL nStartCol, SCROW nStartRow, SCTAB nStartTab,
                                                       SCCOL nEndCol, SCROW nEndRow, SCTAB nEndTab );
    void            UpdatePaintExt( USHORT& rExtFlags, const ScRange& rRange );

    void            SetDocumentModified( BOOL bIsModified = TRUE );
    void            SetDrawModified( BOOL bIsModified = TRUE );

    void            LockPaint();
    void            UnlockPaint();
    USHORT          GetLockCount() const;
    void            SetLockCount(USHORT nNew);

    void            LockDocument();
    void            UnlockDocument();

    DECL_LINK( DialogClosedHdl, sfx2::FileDialogHelper* );

    virtual SfxStyleSheetBasePool*  GetStyleSheetPool();

    void            SetInplace( BOOL bInplace );
    BOOL            IsEmpty() const;
    void            SetEmpty(BOOL bSet);

    BOOL            IsInUndo() const                { return bIsInUndo; }
    void            SetInUndo(BOOL bSet);

    void            CalcOutputFactor();
    double          GetOutputFactor() const;
    void            GetPageOnFromPageStyleSet( const SfxItemSet* pStyleSet,
                                               SCTAB             nCurTab,
                                               BOOL&             rbHeader,
                                               BOOL&             rbFooter );

    virtual long DdeGetData( const String& rItem, const String& rMimeType,
                                ::com::sun::star::uno::Any & rValue );
    virtual long DdeSetData( const String& rItem, const String& rMimeType,
                                const ::com::sun::star::uno::Any & rValue );
    virtual ::sfx2::SvLinkSource* DdeCreateLinkSource( const String& rItem );

    const String& GetDdeTextFmt() const { return aDdeTextFmt; }

    SfxBindings*    GetViewBindings();

    ScTabViewShell* GetBestViewShell();
    ScSbxDocHelper* GetDocHelperObject() { return pDocHelper; }

    void            SetDocumentModifiedPending( BOOL bVal )
                        { bDocumentModifiedPending = bVal; }
    BOOL            IsDocumentModifiedPending() const
                        { return bDocumentModifiedPending; }

    BOOL            IsUpdateEnabled() const
                        { return bUpdateEnabled; }
    void            SetUpdateEnabled(BOOL bValue)
                        { bUpdateEnabled = bValue; }

    OutputDevice*   GetRefDevice(); // WYSIWYG: Printer, otherwise VirtualDevice...

    static ScViewData* GetViewData();
    static SCTAB       GetCurTab();

    static ScDocShell* GetShellByNum( USHORT nDocNo );
    static String   GetOwnFilterName();
    static String   GetWebQueryFilterName();
    static String   GetAsciiFilterName();
    static String   GetLotusFilterName();
    static String   GetDBaseFilterName();
    static String   GetDifFilterName();
    static BOOL     HasAutomaticTableName( const String& rFilter );

    DECL_LINK( RefreshDBDataHdl, ScRefreshTimer* );

    void            BeforeXMLLoading();
    void            AfterXMLLoading(sal_Bool bRet);

    virtual sal_uInt16 GetHiddenInformationState( sal_uInt16 nStates );

    const ScOptSolverSave* GetSolverSaveData() const    { return pSolverSaveData; }     // may be null
    void            SetSolverSaveData( const ScOptSolverSave& rData );

    bool            MergeSharedDocument( ScDocument& rSharedDoc );

private:

    void            SwitchDocumentToShared( bool bShared );
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XLoadable > LoadSharedDocument();
};

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
            BOOL            bAutoCalcShellDisabled;
            BOOL            bIdleDisabled;

                            // not implemented
                            ScDocShellModificator( const ScDocShellModificator& );
    ScDocShellModificator&  operator=( const ScDocShellModificator& );

public:
                            ScDocShellModificator( ScDocShell& );
                            ~ScDocShellModificator();
            void            SetDocumentModified();
};



#endif


