/*************************************************************************
 *
 *  $RCSfile: document.hxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: sab $ $Date: 2000-11-28 16:00:20 $
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

#ifndef SC_DOCUMENT_HXX
#define SC_DOCUMENT_HXX


#ifndef _SV_PRNTYPES_HXX //autogen
#include <vcl/prntypes.hxx>
#endif

#ifndef _SV_TIMER_HXX //autogen
#include <vcl/timer.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif


#ifndef SC_TABLE_HXX
#include "table.hxx"        // FastGetRowHeight (inline)
#endif

#ifndef SC_RANGELST_HXX
#include "rangelst.hxx"
#endif


class KeyEvent;
class OutputDevice;
class SdrObject;
class SfxBroadcaster;
class SfxHint;
class SfxItemSet;
class SfxObjectShell;
class SfxBindings;
class SfxPoolItem;
class SfxItemPool;
class SfxPrinter;
class SfxStatusBarManager;
class SfxStyleSheetBase;
class SvMemoryStream;
class SvNumberFormatter;
class SvxBorderLine;
class SvxBoxInfoItem;
class SvxBoxItem;
class SvxBrushItem;
class SvxLinkManager;
class SvxSearchItem;
class SvxShadowItem;
class Window;
class XColorTable;

class ScAutoFormatData;
class ScBaseCell;
class ScBroadcastAreaSlotMachine;
class ScChangeViewSettings;
class ScChartCollection;
class ScChartListenerCollection;
class ScConditionalFormat;
class ScConditionalFormatList;
class ScDBCollection;
class ScDBData;
class ScDetOpData;
class ScDetOpList;
class ScDocOptions;
class ScDocumentPool;
class ScDrawLayer;
class ScExtDocOptions;
class ScFormulaCell;
class SchMemChart;
class ScMarkData;
class ScOutlineTable;
class ScPatternAttr;
class ScPivot;
class ScPivotCollection;
class ScPrintRangeSaver;
class ScRangeData;
class ScRangeName;
class ScStyleSheet;
class ScStyleSheetPool;
class ScTable;
class ScTokenArray;
class ScValidationData;
class ScValidationDataList;
class ScViewOptions;
class TypedStrCollection;
class ScChangeTrack;
class ScFieldEditEngine;
struct ScConsolidateParam;
class ScDPObject;
class ScDPCollection;
class ScMatrix;
class ScScriptTypeData;

namespace com { namespace sun { namespace star {
    namespace lang {
        class XMultiServiceFactory;
    }
}}}


#ifdef _ZFORLIST_DECLARE_TABLE
class SvULONGTable;
#else
class Table;
typedef Table SvULONGTable;
#endif


#define SC_TAB_APPEND       0xFFFF
#define SC_DOC_NEW          0xFFFF
#define REPEAT_NONE         0xFFFF

#define SC_MACROCALL_ALLOWED        0
#define SC_MACROCALL_NOTALLOWED     1
#define SC_MACROCALL_ASK            2


enum ScDocumentMode
    {
        SCDOCMODE_DOCUMENT,
        SCDOCMODE_CLIP,
        SCDOCMODE_UNDO
    };



// -----------------------------------------------------------------------
//
//          structs fuer FillInfo
//

enum ScShadowPart
{
    SC_SHADOW_HSTART,
    SC_SHADOW_VSTART,
    SC_SHADOW_HORIZ,
    SC_SHADOW_VERT,
    SC_SHADOW_CORNER
};

#define SC_ROTDIR_NONE          0
#define SC_ROTDIR_STANDARD      1
#define SC_ROTDIR_LEFT          2
#define SC_ROTDIR_RIGHT         3
#define SC_ROTDIR_CENTER        4

struct CellInfo
    {
        ScBaseCell*                 pCell;

        const ScPatternAttr*        pPatternAttr;
        const SfxItemSet*           pConditionSet;

        const SvxBrushItem*         pBackground;

        const SvxBoxItem*           pLinesAttr;             // Original-Item (intern)

        const SvxBorderLine*        pThisBottom;            // einzelne inkl. zusammengefasst
        const SvxBorderLine*        pNextTop;               // (intern)
        const SvxBorderLine*        pThisRight;
        const SvxBorderLine*        pNextLeft;

        const SvxBorderLine*        pRightLine;             // dickere zum Zeichnen
        const SvxBorderLine*        pBottomLine;

        const SvxShadowItem*        pShadowAttr;            // Original-Item (intern)

        ScShadowPart                eHShadowPart;           // Schatten effektiv zum Zeichnen
        ScShadowPart                eVShadowPart;
        const SvxShadowItem*        pHShadowOrigin;
        const SvxShadowItem*        pVShadowOrigin;

        USHORT                      nWidth;

        BOOL                        bMarked;
        BOOL                        bStandard;
        BOOL                        bEmptyCellText;

        BOOL                        bMerged;
        BOOL                        bHOverlapped;
        BOOL                        bVOverlapped;
        BOOL                        bAutoFilter;
        BOOL                        bPushButton;
        BYTE                        nRotateDir;

        BOOL                        bPrinted;               // bei Bedarf (Pagebreak-Modus)

        BOOL                        bHideGrid;              // output-intern
        BOOL                        bEditEngine;            // output-intern
    };

#define SC_ROTMAX_NONE  USHRT_MAX

struct RowInfo
    {
        CellInfo*       pCellInfo;

        USHORT          nHeight;
        USHORT          nRowNo;
        USHORT          nRotMaxCol;         // SC_ROTMAX_NONE, wenn nichts

        BOOL            bEmptyBack;
        BOOL            bEmptyText;
        BOOL            bAutoFilter;
        BOOL            bPushButton;
        BOOL            bChanged;           // TRUE, wenn nicht getestet
    };

struct ScDocStat
{
    String  aDocName;
    USHORT  nTableCount;
    ULONG   nCellCount;
    USHORT  nPageCount;
};

// nicht 11 Parameter bei CopyBlockFromClip, konstante Werte der Schleife hier
struct ScCopyBlockFromClipParams
{
    ScDocument* pRefUndoDoc;
    ScDocument* pClipDoc;
    USHORT      nInsFlag;
    USHORT      nTabStart;
    USHORT      nTabEnd;
    BOOL        bAsLink;
};

#define ROWINFO_MAX 1024


// Spezialwert fuer Recalc-Alwyas-Zellen

#define BCA_BRDCST_ALWAYS ScAddress( 0, 32767, 0 )
#define BCA_LISTEN_ALWAYS ScRange( BCA_BRDCST_ALWAYS, BCA_BRDCST_ALWAYS )

// -----------------------------------------------------------------------

// DDE Link Modes

#define SC_DDE_DEFAULT      0
#define SC_DDE_ENGLISH      1
#define SC_DDE_TEXT         2

// -----------------------------------------------------------------------

class ScDocument
{
friend class ScDocumentIterator;
friend class ScValueIterator;
friend class ScQueryValueIterator;
friend class ScCellIterator;
friend class ScQueryCellIterator;
friend class ScHorizontalCellIterator;
friend class ScHorizontalAttrIterator;
friend class ScDocAttrIterator;
friend class ScAttrRectIterator;
friend class ScPivot;

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceManager;
    ScDocumentPool*     pDocPool;
    ScStyleSheetPool*   pStylePool;
    SfxItemPool*        pEditPool;                      // EditTextObjectPool
    SfxItemPool*        pEnginePool;                    // EditEnginePool
    ScFieldEditEngine*  pEditEngine;                    // mit pEditPool
    SfxObjectShell*     pShell;
    SfxPrinter*         pPrinter;
    ScDrawLayer*        pDrawLayer;                     // SdrModel
    XColorTable*        pColorTable;
    ScConditionalFormatList* pCondFormList;             // bedingte Formate
    ScValidationDataList* pValidationList;              // Gueltigkeit
    SvNumberFormatter*  pFormTable;
    SvULONGTable*       pFormatExchangeList;            // zum Umsetzen von Zahlenformaten
    ScTable*            pTab[MAXTAB+1];
    ScRangeName*        pRangeName;
    ScDBCollection*     pDBCollection;
    ScPivotCollection*  pPivotCollection;
    ScDPCollection*     pDPCollection;
    ScChartCollection*  pChartCollection;
    ScPatternAttr*      pSelectionAttr;                 // Attribute eines Blocks
    SvxLinkManager*     pLinkManager;
    ScFormulaCell*      pFormulaTree;                   // Berechnungsbaum Start
    ScFormulaCell*      pEOFormulaTree;                 // Berechnungsbaum Ende, letzte Zelle
    ScFormulaCell*      pFormulaTrack;                  // BroadcastTrack Start
    ScFormulaCell*      pEOFormulaTrack;                // BrodcastTrack Ende, letzte Zelle
    ScBroadcastAreaSlotMachine* pBASM;                  // BroadcastAreas
    ScChartListenerCollection* pChartListenerCollection;
    SvMemoryStream*     pClipData;
    ScDetOpList*        pDetOpList;
    ScChangeTrack*      pChangeTrack;
    SfxBroadcaster*     pUnoBroadcaster;
    ScChangeViewSettings* pChangeViewSettings;
    ScScriptTypeData*   pScriptTypeData;

    Link                aColorLink;                     // fuer Farben in Z.Formaten
    String              aProtectPass;
    String              aDocName;                       // opt: Dokumentname
    ScRangePairListRef  xColNameRanges;
    ScRangePairListRef  xRowNameRanges;

    ScViewOptions*      pViewOptions;                   // View-Optionen
    ScDocOptions*       pDocOptions;                    // Dokument-Optionen
    ScExtDocOptions*    pExtDocOptions;                 // fuer Import etc.
    ScConsolidateParam* pConsolidateDlgData;

    ScRange             aClipRange;
    ScRange             aEmbedRange;
    ScAddress           aCurTextWidthCalcPos;
    ScAddress           aOnlineSpellPos;                // within whole document
    ScRange             aVisSpellRange;
    ScAddress           aVisSpellPos;                   // within aVisSpellRange (see nVisSpellState)

    Timer               aTrackTimer;

    LanguageType        eLanguage;                      // default language
    LanguageType        eCjkLanguage;                   // default language for asian text
    LanguageType        eCtlLanguage;                   // default language for complex text
    CharSet             eSrcSet;                        // Einlesen: Quell-Zeichensatz

    ULONG               nFormulaCodeInTree;             // FormelRPN im Formelbaum
    USHORT              nInterpretLevel;                // >0 wenn im Interpreter
    USHORT              nMacroInterpretLevel;           // >0 wenn Macro im Interpreter
    USHORT              nMaxTableNumber;
    USHORT              nSrcVer;                        // Dateiversion (Laden/Speichern)
    USHORT              nSrcMaxRow;                     // Zeilenzahl zum Laden/Speichern
    USHORT              nFormulaTrackCount;
    USHORT              nHardRecalcState;               // 0: soft, 1: hard-warn, 2: hard
    USHORT              nVisibleTab;                    // fuer OLE etc.

    BOOL                bProtected;
    BOOL                bOwner;
    BOOL                bAutoCalc;                      // Automatisch Berechnen
    BOOL                bAutoCalcShellDisabled;         // in/von/fuer ScDocShell disabled
    // ob noch ForcedFormulas berechnet werden muessen,
    // im Zusammenspiel mit ScDocShell SetDocumentModified,
    // AutoCalcShellDisabled und TrackFormulas
    BOOL                bForcedFormulaPending;
    BOOL                bIsClip;
    BOOL                bCutMode;
    BOOL                bIsUndo;

    BOOL                bIsEmbedded;                    // Embedded-Bereich anzeigen/anpassen ?

    // kein SetDirty bei ScFormulaCell::CompileTokenArray sondern am Ende
    // von ScDocument::CompileAll[WithFormats], CopyScenario, CopyBlockFromClip
    BOOL                bNoSetDirty;
    // kein Broadcast, keine Listener aufbauen waehrend aus einem anderen
    // Doc (per Filter o.ae.) inserted wird, erst bei CompileAll / CalcAfterLoad
    BOOL                bInsertingFromOtherDoc;
    BOOL                bImportingXML;      // special handling of formula text
    BOOL                bCalcingAfterLoad;              // in CalcAfterLoad TRUE
    // wenn temporaer keine Listener auf/abgebaut werden sollen
    BOOL                bNoListening;
    BOOL                bLoadingDone;
    BOOL                bIdleDisabled;
    BOOL                bInLinkUpdate;                  // TableLink or AreaLink
    BOOL                bChartListenerCollectionNeedsUpdate;
    // ob RC_FORCED Formelzellen im Dokument sind/waren (einmal an immer an)
    BOOL                bHasForcedFormulas;
    // ist beim Laden/Speichern etwas weggelassen worden?
    BOOL                bLostData;
    // ob das Doc gerade zerstoert wird (kein Notify-Tracking etc. mehr)
    BOOL                bInDtorClear;
    // ob bei Spalte/Zeile einfuegen am Rand einer Referenz die Referenz
    // erweitert wird, wird in jedem UpdateReference aus InputOptions geholt,
    // gesetzt und am Ende von UpdateReference zurueckgesetzt
    BOOL                bExpandRefs;
    // fuer Detektiv-Update, wird bei jeder Aenderung an Formeln gesetzt
    BOOL                bDetectiveDirty;

    BYTE                nMacroCallMode;     // Makros per Warnung-Dialog disabled?
    BOOL                bHasMacroFunc;      // valid only after loading

    BYTE                nVisSpellState;

    inline BOOL         RowHidden( USHORT nRow, USHORT nTab );      // FillInfo

    ScLkUpdMode         eLinkMode;

public:
    long            GetCellCount() const;       // alle Zellen
    long            GetWeightedCount() const;   // Formeln und Edit staerker gewichtet
    ULONG           GetCodeCount() const;       // RPN-Code in Formeln
    DECL_LINK( GetUserDefinedColor, USHORT * );
                                                                // Numberformatter

public:
                    ScDocument( ScDocumentMode eMode = SCDOCMODE_DOCUMENT,
                                SfxObjectShell* pDocShell = NULL );
                    ~ScDocument();

    inline ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                    GetServiceManager() const { return xServiceManager; }

    const String&   GetName() const { return aDocName; }
    void            SetName( const String& r ) { aDocName = r; }

    void            GetDocStat( ScDocStat& rDocStat );

    void            InitDrawLayer( SfxObjectShell* pDocShell = NULL );
    XColorTable*    GetColorTable();

    SvxLinkManager*     GetLinkManager() { return pLinkManager; }
    void                SetLinkManager( SvxLinkManager* pNew );

    const ScDocOptions&     GetDocOptions() const;
    void                    SetDocOptions( const ScDocOptions& rOpt );
    const ScViewOptions&    GetViewOptions() const;
    void                    SetViewOptions( const ScViewOptions& rOpt );
    void                    SetPrintOptions();

    ScExtDocOptions*        GetExtDocOptions()  { return pExtDocOptions; }
    void                    SetExtDocOptions( ScExtDocOptions* pNewOptions );

    void                    GetLanguage( LanguageType& rLatin, LanguageType& rCjk, LanguageType& rCtl ) const;
    void                    SetLanguage( LanguageType eLatin, LanguageType eCjk, LanguageType eCtl );

    void                        SetConsolidateDlgData( const ScConsolidateParam* pData );
    const ScConsolidateParam*   GetConsolidateDlgData() const { return pConsolidateDlgData; }

    void            Clear();

    ScRangeName*    GetRangeName();
    void            SetRangeName( ScRangeName* pNewRangeName );
    USHORT          GetMaxTableNumber() { return nMaxTableNumber; }
    void            SetMaxTableNumber(USHORT nNumber) { nMaxTableNumber = nNumber; }

    ScRangePairList*    GetColNameRanges() { return &xColNameRanges; }
    ScRangePairList*    GetRowNameRanges() { return &xRowNameRanges; }
    ScRangePairListRef& GetColNameRangesRef() { return xColNameRanges; }
    ScRangePairListRef& GetRowNameRangesRef() { return xRowNameRanges; }

    ScDBCollection* GetDBCollection() const;
    void            SetDBCollection( ScDBCollection* pNewDBCollection );
    ScDBData*       GetDBAtCursor(USHORT nCol, USHORT nRow, USHORT nTab,
                                        BOOL bStartOnly = FALSE) const;
    ScDBData*       GetDBAtArea(USHORT nTab, USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2) const;

    ScRangeData*    GetRangeAtCursor(USHORT nCol, USHORT nRow, USHORT nTab,
                                        BOOL bStartOnly = FALSE) const;
    ScRangeData*    GetRangeAtBlock( const ScRange& rBlock, String* pName=NULL ) const;

    ScDPCollection*     GetDPCollection();
    ScDPObject*         GetDPAtCursor(USHORT nCol, USHORT nRow, USHORT nTab) const;

    ScPivotCollection*  GetPivotCollection() const;
    void                SetPivotCollection(ScPivotCollection* pNewPivotCollection);
    ScPivot*            GetPivotAtCursor(USHORT nCol, USHORT nRow, USHORT nTab) const;

    ScChartCollection*  GetChartCollection() const;
    void                SetChartCollection(ScChartCollection* pNewChartCollection);

    void            EnsureGraphicNames();

    SdrObject*      GetObjectAtPoint( USHORT nTab, const Point& rPos );
    BOOL            HasChartAtPoint( USHORT nTab, const Point& rPos, String* pName = NULL );
    void            UpdateChartArea( const String& rChartName, const ScRange& rNewArea,
                                        BOOL bColHeaders, BOOL bRowHeaders, BOOL bAdd,
                                        Window* pWindow );
    void            UpdateChartArea( const String& rChartName,
                                    const ScRangeListRef& rNewList,
                                    BOOL bColHeaders, BOOL bRowHeaders, BOOL bAdd,
                                    Window* pWindow );
    SchMemChart*    FindChartData(const String& rName, BOOL bForModify = FALSE);

    void            MakeTable( USHORT nTab );

    USHORT          GetVisibleTab() const       { return nVisibleTab; }
    void            SetVisibleTab(USHORT nTab)  { nVisibleTab = nTab; }

    BOOL            HasTable( USHORT nTab ) const;
    BOOL            GetName( USHORT nTab, String& rName ) const;
    BOOL            GetTable( const String& rName, USHORT& rTab ) const;
    inline USHORT   GetTableCount() const { return nMaxTableNumber; }
    SvULONGTable*   GetFormatExchangeList() const { return pFormatExchangeList; }

    void            SetDocProtection( BOOL bProtect, const String& rPasswd );
    void            SetTabProtection( USHORT nTab, BOOL bProtect, const String& rPasswd );
    BOOL            IsDocProtected() const;
    BOOL            IsDocEditable() const;
    BOOL            IsTabProtected( USHORT nTab ) const;
    const String&   GetDocPassword() const;
    const String&   GetTabPassword( USHORT nTab ) const;

    void            LockTable(USHORT nTab);
    void            UnlockTable(USHORT nTab);

    BOOL            IsBlockEditable( USHORT nTab, USHORT nStartCol, USHORT nStartRow,
                                        USHORT nEndCol, USHORT nEndRow,
                                        BOOL* pOnlyNotBecauseOfMatrix = NULL ) const;
    BOOL            IsSelectedBlockEditable( USHORT nStartCol, USHORT nStartRow,
                                            USHORT nEndCol, USHORT nEndRow,
                                            const ScMarkData& rMark ) const;
    BOOL            IsSelectionEditable( const ScMarkData& rMark,
                                        BOOL* pOnlyNotBecauseOfMatrix = NULL ) const;
    BOOL            IsSelectionOrBlockEditable( USHORT nTab, USHORT nStartCol, USHORT nStartRow,
                                        USHORT nEndCol, USHORT nEndRow,
                                        const ScMarkData& rMark ) const;
    BOOL            IsSelectedOrBlockEditable( USHORT nStartCol, USHORT nStartRow,
                                            USHORT nEndCol, USHORT nEndRow,
                                            const ScMarkData& rMark ) const;

    BOOL            HasSelectedBlockMatrixFragment( USHORT nStartCol, USHORT nStartRow,
                                            USHORT nEndCol, USHORT nEndRow,
                                            const ScMarkData& rMark ) const;

    BOOL            IsEmbedded() const;
    void            GetEmbedded( ScTripel& rStart, ScTripel& rEnd ) const;
    void            SetEmbedded( const ScTripel& rStart, const ScTripel& rEnd );
    void            ResetEmbedded();
    Rectangle       GetEmbeddedRect() const;                        // 1/100 mm
    void            SetEmbedded( const Rectangle& rRect );          // aus VisArea (1/100 mm)
    void            SnapVisArea( Rectangle& rRect ) const;          // 1/100 mm

    BOOL            ValidTabName( const String& rName ) const;
    BOOL            ValidNewTabName( const String& rName ) const;
    void            CreateValidTabName(String& rName) const;
    BOOL            InsertTab( USHORT nPos, const String& rName,
                                BOOL bExternalDocument = FALSE );
    BOOL            DeleteTab( USHORT nTab, ScDocument* pRefUndoDoc = NULL );
    BOOL            RenameTab( USHORT nTab, const String& rName,
                                BOOL bUpdateRef = TRUE,
                                BOOL bExternalDocument = FALSE );
    BOOL            MoveTab( USHORT nOldPos, USHORT nNewPos );
    BOOL            CopyTab( USHORT nOldPos, USHORT nNewPos,
                                const ScMarkData* pOnlyMarked = NULL );
    ULONG           TransferTab(ScDocument* pSrcDoc, USHORT nSrcPos, USHORT nDestPos,
                                    BOOL bInsertNew = TRUE,
                                    BOOL bResultsOnly = FALSE );
    void            TransferDrawPage(ScDocument* pSrcDoc, USHORT nSrcPos, USHORT nDestPos);
    void            ClearDrawPage(USHORT nTab);
    void            SetVisible( USHORT nTab, BOOL bVisible );
    BOOL            IsVisible( USHORT nTab ) const;
    void            SetScenario( USHORT nTab, BOOL bFlag );
    BOOL            IsScenario( USHORT nTab ) const;
    void            GetScenarioData( USHORT nTab, String& rComment,
                                        Color& rColor, USHORT& rFlags ) const;
    void            SetScenarioData( USHORT nTab, const String& rComment,
                                        const Color& rColor, USHORT nFlags );
    BOOL            IsActiveScenario( USHORT nTab ) const;
    void            SetActiveScenario( USHORT nTab, BOOL bActive );     // nur fuer Undo etc.
    BYTE            GetLinkMode( USHORT nTab ) const;
    BOOL            IsLinked( USHORT nTab ) const;
    const String&   GetLinkDoc( USHORT nTab ) const;
    const String&   GetLinkFlt( USHORT nTab ) const;
    const String&   GetLinkOpt( USHORT nTab ) const;
    const String&   GetLinkTab( USHORT nTab ) const;
    void            SetLink( USHORT nTab, BYTE nMode, const String& rDoc,
                            const String& rFilter, const String& rOptions,
                            const String& rTabName );
    BOOL            HasLink( const String& rDoc,
                            const String& rFilter, const String& rOptions ) const;
    BOOL            LinkEmptyTab( USHORT& nTab, const String& aDocTab,
                                    const String& aFileName,
                                    const String& aTabName );   // insert empty tab & link
    BOOL            LinkExternalTab( USHORT& nTab, const String& aDocTab,
                                    const String& aFileName,
                                    const String& aTabName );

    BOOL            HasDdeLinks() const;
    BOOL            HasAreaLinks() const;
    void            UpdateDdeLinks();
    void            UpdateAreaLinks();

    void            CopyDdeLinks( ScDocument* pDestDoc ) const;
    void            DisconnectDdeLinks();

                    // Fuer Excel-Import:
    void            CreateDdeLink( const String& rAppl, const String& rTopic, const String& rItem );

                    // Fuer StarOne Api:
    USHORT          GetDdeLinkCount() const;
    BOOL            GetDdeLinkData( USHORT nPos, String& rAppl, String& rTopic, String& rItem ) const;
    BOOL            UpdateDdeLink( const String& rAppl, const String& rTopic, const String& rItem );

                    // For XML Export/Import:
    BOOL            GetDdeLinkMode(USHORT nPos, USHORT& nMode);
    BOOL            GetDdeLinkResultDimension( USHORT nPos , USHORT& nCol, USHORT& nRow, ScMatrix*& pMatrix);
    BOOL            GetDdeLinkResult(const ScMatrix* pMatrix, USHORT nCol, USHORT nRow, String& rStrValue, double& rDoubValue, BOOL& bIsString);

    void            CreateDdeLink(const String& rAppl, const String& rTopic, const String& rItem, const BYTE nMode );
    BOOL            FindDdeLink(const String& rAppl, const String& rTopic, const String& rItem, const BYTE nMode, USHORT& nPos );
    BOOL            CreateDdeLinkResultDimension(USHORT nPos, USHORT nCols, USHORT nRows, ScMatrix*& pMatrix);
    void            SetDdeLinkResult(ScMatrix* pMatrix, const USHORT nCol, const USHORT nRow, const String& rStrValue, const double& rDoubValue, BOOL bString, BOOL bEmpty);


    SfxBindings*    GetViewBindings();
    SfxObjectShell* GetDocumentShell() const    { return pShell; }
    ScDrawLayer*    GetDrawLayer()              { return pDrawLayer; }
    SfxBroadcaster* GetDrawBroadcaster();       // zwecks Header-Vermeidung
    void            BeginDrawUndo();

    BOOL            IsChart( SdrObject* pObject );
    void            UpdateAllCharts( BOOL bDoUpdate = TRUE );
    void            UpdateChartRef( UpdateRefMode eUpdateRefMode,
                                    USHORT nCol1, USHORT nRow1, USHORT nTab1,
                                    USHORT nCol2, USHORT nRow2, USHORT nTab2,
                                    short nDx, short nDy, short nDz );
                    //! setzt nur die neue RangeList, keine ChartListener o.ae.
    void            SetChartRangeList( const String& rChartName,
                        const ScRangeListRef& rNewRangeListRef );

    BOOL            HasControl( USHORT nTab, const Rectangle& rMMRect );
    void            InvalidateControls( Window* pWin, USHORT nTab, const Rectangle& rMMRect );

    void            StopAnimations( USHORT nTab, Window* pWin );
    void            StartAnimations( USHORT nTab, Window* pWin );

    BOOL            HasBackgroundDraw( USHORT nTab, const Rectangle& rMMRect );
    BOOL            HasAnyDraw( USHORT nTab, const Rectangle& rMMRect );

    ScOutlineTable* GetOutlineTable( USHORT nTab, BOOL bCreate = FALSE );
    BOOL            SetOutlineTable( USHORT nTab, const ScOutlineTable* pNewOutline );

    void            DoAutoOutline( USHORT nStartCol, USHORT nStartRow,
                                    USHORT nEndCol, USHORT nEndRow, USHORT nTab );

    BOOL            DoSubTotals( USHORT nTab, ScSubTotalParam& rParam );
    void            RemoveSubTotals( USHORT nTab, ScSubTotalParam& rParam );
    BOOL            TestRemoveSubTotals( USHORT nTab, const ScSubTotalParam& rParam );

    void            PutCell( const ScAddress&, ScBaseCell* pCell, BOOL bForceTab = FALSE );
    void            PutCell( const ScAddress&, ScBaseCell* pCell,
                            ULONG nFormatIndex, BOOL bForceTab = FALSE);
    void            PutCell( USHORT nCol, USHORT nRow, USHORT nTab, ScBaseCell* pCell,
                            BOOL bForceTab = FALSE );
    void            PutCell(USHORT nCol, USHORT nRow, USHORT nTab, ScBaseCell* pCell,
                            ULONG nFormatIndex, BOOL bForceTab = FALSE);
                    //  return TRUE = Zahlformat gesetzt
    BOOL            SetString( USHORT nCol, USHORT nRow, USHORT nTab, const String& rString );
    void            SetValue( USHORT nCol, USHORT nRow, USHORT nTab, const double& rVal );
    void            SetNote( USHORT nCol, USHORT nRow, USHORT nTab, const ScPostIt& rNote );
    void            SetError( USHORT nCol, USHORT nRow, USHORT nTab, const USHORT nError);

    void            InsertMatrixFormula(USHORT nCol1, USHORT nRow1,
                                        USHORT nCol2, USHORT nRow2,
                                        const ScMarkData& rMark,
                                        const String& rFormula,
                                        const ScTokenArray* p = NULL );
    void            InsertTableOp(const ScTabOpParam& rParam,   // Mehrfachoperation
                                  USHORT nCol1, USHORT nRow1,
                                  USHORT nCol2, USHORT nRow2, const ScMarkData& rMark);

    void            GetString( USHORT nCol, USHORT nRow, USHORT nTab, String& rString );
    void            GetInputString( USHORT nCol, USHORT nRow, USHORT nTab, String& rString );
    double          GetValue( const ScAddress& );
    void            GetValue( USHORT nCol, USHORT nRow, USHORT nTab, double& rValue );
    double          RoundValueAsShown( double fVal, ULONG nFormat );
    void            GetNumberFormat( USHORT nCol, USHORT nRow, USHORT nTab,
                                     ULONG& rFormat );
    ULONG           GetNumberFormat( const ScAddress& ) const;
                    /// if no number format attribute is set the calculated
                    /// number format of the formula cell is returned
    void            GetNumberFormatInfo( short& nType, ULONG& nIndex,
                        const ScAddress& rPos, const ScFormulaCell& rFCell ) const;
    void            GetFormula( USHORT nCol, USHORT nRow, USHORT nTab, String& rFormula,
                                BOOL bAsciiExport = FALSE ) const;
    BOOL            GetNote( USHORT nCol, USHORT nRow, USHORT nTab, ScPostIt& rNote);
    void            GetCellType( USHORT nCol, USHORT nRow, USHORT nTab, CellType& rCellType ) const;
    CellType        GetCellType( const ScAddress& rPos ) const;
    void            GetCell( USHORT nCol, USHORT nRow, USHORT nTab, ScBaseCell*& rpCell ) const;
    ScBaseCell*     GetCell( const ScAddress& rPos ) const;

    void            RefreshNoteFlags();
    BOOL            HasNoteObject( USHORT nCol, USHORT nRow, USHORT nTab ) const;

    BOOL            HasData( USHORT nCol, USHORT nRow, USHORT nTab );
    BOOL            HasStringData( USHORT nCol, USHORT nRow, USHORT nTab ) const;
    BOOL            HasValueData( USHORT nCol, USHORT nRow, USHORT nTab ) const;
    USHORT          GetErrorData(USHORT nCol, USHORT nRow, USHORT nTab) const;
    BOOL            HasStringCells( const ScRange& rRange ) const;

    BOOL            ExtendMerge( USHORT nStartCol, USHORT nStartRow,
                                USHORT& rEndCol, USHORT& rEndRow, USHORT nTab,
                                BOOL bRefresh = FALSE, BOOL bAttrs = FALSE );
    BOOL            ExtendMerge( ScRange& rRange, BOOL bRefresh = FALSE, BOOL bAttrs = FALSE );
    BOOL            ExtendTotalMerge( ScRange& rRange );
    BOOL            ExtendOverlapped( USHORT& rStartCol, USHORT& rStartRow,
                                USHORT nEndCol, USHORT nEndRow, USHORT nTab );
    BOOL            ExtendOverlapped( ScRange& rRange );

    BOOL            RefreshAutoFilter( USHORT nStartCol, USHORT nStartRow,
                                USHORT nEndCol, USHORT nEndRow, USHORT nTab );

    void            DoMergeContents( USHORT nTab, USHORT nStartCol, USHORT nStartRow,
                                    USHORT nEndCol, USHORT nEndRow );
                    //  ohne Ueberpruefung:
    void            DoMerge( USHORT nTab, USHORT nStartCol, USHORT nStartRow,
                                    USHORT nEndCol, USHORT nEndRow );
    void            RemoveMerge( USHORT nCol, USHORT nRow, USHORT nTab );

    BOOL            IsBlockEmpty( USHORT nTab, USHORT nStartCol, USHORT nStartRow,
                                                USHORT nEndCol, USHORT nEndRow ) const;
    BOOL            IsPrintEmpty( USHORT nTab, USHORT nStartCol, USHORT nStartRow,
                                                USHORT nEndCol, USHORT nEndRow,
                                                BOOL bLeftIsEmpty = FALSE,
                                                ScRange* pLastRange = NULL,
                                                Rectangle* pLastMM = NULL ) const;

    BOOL            IsOverlapped( USHORT nCol, USHORT nRow, USHORT nTab ) const;
    BOOL            IsHorOverlapped( USHORT nCol, USHORT nRow, USHORT nTab ) const;
    BOOL            IsVerOverlapped( USHORT nCol, USHORT nRow, USHORT nTab ) const;

    BOOL            HasAttrib( USHORT nCol1, USHORT nRow1, USHORT nTab1,
                            USHORT nCol2, USHORT nRow2, USHORT nTab2, USHORT nMask );
    BOOL            HasAttrib( const ScRange& rRange, USHORT nMask );

    BOOL            HasLines( const ScRange& rRange, Rectangle& rSizes ) const;

    void            GetBorderLines( USHORT nCol, USHORT nRow, USHORT nTab,
                                    const SvxBorderLine** ppLeft,
                                    const SvxBorderLine** ppTop,
                                    const SvxBorderLine** ppRight,
                                    const SvxBorderLine** ppBottom ) const;

    void            ResetChanged( const ScRange& rRange );

    void            SetDirty();
    void            SetDirty( const ScRange& );
    void            SetDirtyVar();
    void            CalcAll();
    void            CalcAfterLoad();
    void            CompileAll();
    void            CompileXML();

                    // Automatisch Berechnen
    void            SetAutoCalc( BOOL bNewAutoCalc );
    BOOL            GetAutoCalc() const { return bAutoCalc; }
                    // Automatisch Berechnen in/von/fuer ScDocShell disabled
    void            SetAutoCalcShellDisabled( BOOL bNew ) { bAutoCalcShellDisabled = bNew; }
    BOOL            IsAutoCalcShellDisabled() const { return bAutoCalcShellDisabled; }
                    // ForcedFormulas zu berechnen
    void            SetForcedFormulaPending( BOOL bNew ) { bForcedFormulaPending = bNew; }
    BOOL            IsForcedFormulaPending() const { return bForcedFormulaPending; }

    void            GetErrCode( USHORT nCol, USHORT nRow, USHORT nTab, USHORT& rErrCode );
    USHORT          GetErrCode( const ScAddress& ) const;

    void            GetDataArea( USHORT nTab, USHORT& rStartCol, USHORT& rStartRow,
                                    USHORT& rEndCol, USHORT& rEndRow, BOOL bIncludeOld );
    BOOL            GetCellArea( USHORT nTab, USHORT& rEndCol, USHORT& rEndRow ) const;
    BOOL            GetTableArea( USHORT nTab, USHORT& rEndCol, USHORT& rEndRow ) const;
    BOOL            GetPrintArea( USHORT nTab, USHORT& rEndCol, USHORT& rEndRow,
                                    BOOL bNotes = TRUE ) const;
    BOOL            GetPrintAreaHor( USHORT nTab, USHORT nStartRow, USHORT nEndRow,
                                        USHORT& rEndCol, BOOL bNotes = TRUE ) const;
    BOOL            GetPrintAreaVer( USHORT nTab, USHORT nStartCol, USHORT nEndCol,
                                        USHORT& rEndRow, BOOL bNotes = TRUE ) const;
    void            InvalidateTableArea();

    BOOL            GetDataStart( USHORT nTab, USHORT& rStartCol, USHORT& rStartRow ) const;

    void            ExtendPrintArea( OutputDevice* pDev, USHORT nTab,
                                    USHORT nStartCol, USHORT nStartRow,
                                    USHORT& rEndCol, USHORT nEndRow );

    USHORT          GetEmptyLinesInBlock( USHORT nStartCol, USHORT nStartRow, USHORT nStartTab,
                                            USHORT nEndCol, USHORT nEndRow, USHORT nEndTab,
                                            ScDirection eDir );

    void            FindAreaPos( USHORT& rCol, USHORT& rRow, USHORT nTab, short nMovX, short nMovY );
    void            GetNextPos( USHORT& rCol, USHORT& rRow, USHORT nTab, short nMovX, short nMovY,
                                BOOL bMarked, BOOL bUnprotected, const ScMarkData& rMark );

    BOOL            GetNextMarkedCell( USHORT& rCol, USHORT& rRow, USHORT nTab,
                                        const ScMarkData& rMark );

    void            LimitChartArea( USHORT nTab, USHORT& rStartCol, USHORT& rStartRow,
                                                    USHORT& rEndCol, USHORT& rEndRow );
    void            LimitChartIfAll( ScRangeListRef& rRangeList );

    BOOL            InsertRow( USHORT nStartCol, USHORT nStartTab,
                               USHORT nEndCol,   USHORT nEndTab,
                               USHORT nStartRow, USHORT nSize );
    BOOL            InsertRow( const ScRange& rRange );
    void            DeleteRow( USHORT nStartCol, USHORT nStartTab,
                               USHORT nEndCol,   USHORT nEndTab,
                               USHORT nStartRow, USHORT nSize,
                               ScDocument* pRefUndoDoc = NULL, BOOL* pUndoOutline = NULL );
    void            DeleteRow( const ScRange& rRange,
                               ScDocument* pRefUndoDoc = NULL, BOOL* pUndoOutline = NULL );
    BOOL            InsertCol( USHORT nStartRow, USHORT nStartTab,
                               USHORT nEndRow,   USHORT nEndTab,
                               USHORT nStartCol, USHORT nSize );
    BOOL            InsertCol( const ScRange& rRange );
    void            DeleteCol( USHORT nStartRow, USHORT nStartTab,
                               USHORT nEndRow, USHORT nEndTab,
                               USHORT nStartCol, USHORT nSize,
                               ScDocument* pRefUndoDoc = NULL, BOOL* pUndoOutline = NULL );
    void            DeleteCol( const ScRange& rRange,
                               ScDocument* pRefUndoDoc = NULL, BOOL* pUndoOutline = NULL );

    BOOL            CanInsertRow( const ScRange& rRange ) const;
    BOOL            CanInsertCol( const ScRange& rRange ) const;

    void            FitBlock( const ScRange& rOld, const ScRange& rNew, BOOL bClear = TRUE );
    BOOL            CanFitBlock( const ScRange& rOld, const ScRange& rNew );

    BOOL            IsClipOrUndo() const                        { return bIsClip || bIsUndo; }
    BOOL            IsUndo() const                              { return bIsUndo; }
    BOOL            IsClipboard() const                         { return bIsClip; }
    void            ResetClip( ScDocument* pSourceDoc, const ScMarkData* pMarks );
    void            ResetClip( ScDocument* pSourceDoc, USHORT nTab );
    void            SetCutMode( BOOL bCut );
    BOOL            IsCutMode();
    void            SetClipArea( const ScRange& rArea, BOOL bCut = FALSE );

    void            DeleteObjectsInArea( USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2,
                                        const ScMarkData& rMark );
    void            DeleteObjectsInSelection( const ScMarkData& rMark );
    void            DeleteObjects( USHORT nTab );

    void            DeleteArea(USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2,
                            const ScMarkData& rMark, USHORT nDelFlag);
    void            DeleteAreaTab(USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2,
                                USHORT nTab, USHORT nDelFlag);
    void            DeleteAreaTab(const ScRange& rRange, USHORT nDelFlag);
    void            CopyToClip(USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2,
                                BOOL bCut, ScDocument* pClipDoc, BOOL bAllTabs,
                                const ScMarkData* pMarks = NULL, BOOL bKeepScenarioFlags=FALSE);
    void            CopyTabToClip(USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2,
                                USHORT nTab, ScDocument* pClipDoc = NULL);
    void            CopyBlockFromClip( USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2,
                                    const ScMarkData& rMark, short nDx, short nDy,
                                    const ScCopyBlockFromClipParams* pCBFCP );
    void            StartListeningFromClip( USHORT nCol1, USHORT nRow1,
                                        USHORT nCol2, USHORT nRow2,
                                        const ScMarkData& rMark, USHORT nInsFlag );
    void            BroadcastFromClip( USHORT nCol1, USHORT nRow1,
                                    USHORT nCol2, USHORT nRow2,
                                    const ScMarkData& rMark, USHORT nInsFlag );
    void            CopyFromClip( const ScRange& rDestRange, const ScMarkData& rMark,
                                    USHORT nInsFlag,
                                    ScDocument* pRefUndoDoc = NULL,
                                    ScDocument* pClipDoc = NULL,
                                    BOOL bResetCut = TRUE,
                                    BOOL bAsLink = FALSE );

    void            GetClipArea(USHORT& nClipX, USHORT& nClipY);
    void            GetClipStart(USHORT& nClipX, USHORT& nClipY);

    BOOL            IsClipboardSource() const;

    void            TransposeClip( ScDocument* pTransClip, USHORT nFlags, BOOL bAsLink );

    void            MixDocument( const ScRange& rRange, USHORT nFunction, BOOL bSkipEmpty,
                                    ScDocument* pSrcDoc );

    void            FillTab( const ScRange& rSrcArea, const ScMarkData& rMark,
                                USHORT nFlags, USHORT nFunction,
                                BOOL bSkipEmpty, BOOL bAsLink );
    void            FillTabMarked( USHORT nSrcTab, const ScMarkData& rMark,
                                USHORT nFlags, USHORT nFunction,
                                BOOL bSkipEmpty, BOOL bAsLink );

    void            InitUndo( ScDocument* pSrcDoc, USHORT nTab1, USHORT nTab2,
                                BOOL bColInfo = FALSE, BOOL bRowInfo = FALSE );
    void            AddUndoTab( USHORT nTab1, USHORT nTab2,
                                BOOL bColInfo = FALSE, BOOL bRowInfo = FALSE );

                    //  nicht mehr benutzen:
    void            CopyToDocument(USHORT nCol1, USHORT nRow1, USHORT nTab1,
                                USHORT nCol2, USHORT nRow2, USHORT nTab2,
                                USHORT nFlags, BOOL bMarked, ScDocument* pDestDoc,
                                const ScMarkData* pMarks = NULL, BOOL bColRowFlags = TRUE);
    void            UndoToDocument(USHORT nCol1, USHORT nRow1, USHORT nTab1,
                                USHORT nCol2, USHORT nRow2, USHORT nTab2,
                                USHORT nFlags, BOOL bMarked, ScDocument* pDestDoc,
                                const ScMarkData* pMarks = NULL);

    void            CopyToDocument(const ScRange& rRange,
                                USHORT nFlags, BOOL bMarked, ScDocument* pDestDoc,
                                const ScMarkData* pMarks = NULL, BOOL bColRowFlags = TRUE);
    void            UndoToDocument(const ScRange& rRange,
                                USHORT nFlags, BOOL bMarked, ScDocument* pDestDoc,
                                const ScMarkData* pMarks = NULL);

    void            CopyScenario( USHORT nSrcTab, USHORT nDestTab, BOOL bNewScenario = FALSE );
    BOOL            TestCopyScenario( USHORT nSrcTab, USHORT nDestTab ) const;
    void            MarkScenario( USHORT nSrcTab, USHORT nDestTab,
                                    ScMarkData& rDestMark, BOOL bResetMark = TRUE,
                                    USHORT nNeededBits = 0 ) const;
    BOOL            HasScenarioRange( USHORT nTab, const ScRange& rRange ) const;
    const ScRangeList* GetScenarioRanges( USHORT nTab ) const;

    void            CopyUpdated( ScDocument* pPosDoc, ScDocument* pDestDoc );

    void            UpdateReference( UpdateRefMode eUpdateRefMode, USHORT nCol1, USHORT nRow1, USHORT nTab1,
                                     USHORT nCol2, USHORT nRow2, USHORT nTab2,
                                     short nDx, short nDy, short nDz, ScDocument* pUndoDoc = NULL );

    void            UpdateTranspose( const ScAddress& rDestPos, ScDocument* pClipDoc,
                                        const ScMarkData& rMark, ScDocument* pUndoDoc = NULL );

    void            UpdateGrow( const ScRange& rArea, USHORT nGrowX, USHORT nGrowY );

    void            Fill(   USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2,
                            const ScMarkData& rMark,
                            USHORT nFillCount, FillDir eFillDir = FILL_TO_BOTTOM,
                            FillCmd eFillCmd = FILL_LINEAR, FillDateCmd eFillDateCmd = FILL_DAY,
                            double nStepValue = 1.0, double nMaxValue = 1E307);
    String          GetAutoFillPreview( const ScRange& rSource, USHORT nEndX, USHORT nEndY );

    BOOL            GetSelectionFunction( ScSubTotalFunc eFunc,
                                            const ScAddress& rCursor, const ScMarkData& rMark,
                                            double& rResult );

    const SfxPoolItem*      GetAttr( USHORT nCol, USHORT nRow, USHORT nTab, USHORT nWhich ) const;
    const ScPatternAttr*    GetPattern( USHORT nCol, USHORT nRow, USHORT nTab ) const;
    const ScPatternAttr*    GetSelectionPattern( const ScMarkData& rMark, BOOL bDeep = TRUE );
    ScPatternAttr*          CreateSelectionPattern( const ScMarkData& rMark, BOOL bDeep = TRUE );

    const ScConditionalFormat* GetCondFormat( USHORT nCol, USHORT nRow, USHORT nTab ) const;
    const SfxItemSet*   GetCondResult( USHORT nCol, USHORT nRow, USHORT nTab ) const;
    const SfxPoolItem*  GetEffItem( USHORT nCol, USHORT nRow, USHORT nTab, USHORT nWhich ) const;

    BYTE            GetCellScriptType( ScBaseCell* pCell, ULONG nNumberFormat );
    BYTE            GetScriptType( USHORT nCol, USHORT nRow, USHORT nTab, ScBaseCell* pCell = NULL );

    BOOL            HasDetectiveOperations() const;
    void            AddDetectiveOperation( const ScDetOpData& rData );
    void            ClearDetectiveOperations();
    ScDetOpList*    GetDetOpList() const                { return pDetOpList; }
    void            SetDetOpList(ScDetOpList* pNew);

    void            GetSelectionFrame( const ScMarkData& rMark,
                                       SvxBoxItem&      rLineOuter,
                                       SvxBoxInfoItem&  rLineInner );
    void            ApplySelectionFrame( const ScMarkData& rMark,
                                         const SvxBoxItem* pLineOuter,
                                         const SvxBoxInfoItem* pLineInner );

    void            ClearSelectionItems( const USHORT* pWhich, const ScMarkData& rMark );
    void            ChangeSelectionIndent( BOOL bIncrement, const ScMarkData& rMark );

    ULONG           AddCondFormat( const ScConditionalFormat& rNew );
    void            FindConditionalFormat( ULONG nKey, ScRangeList& rRanges );
    void            ConditionalChanged( ULONG nKey );
    void            SetConditionalUsed( ULONG nKey );       // beim Speichern

    ULONG           AddValidationEntry( const ScValidationData& rNew );
    void            SetValidationUsed( ULONG nKey );        // beim Speichern

    const ScValidationData* GetValidationEntry( ULONG nIndex ) const;

    ScConditionalFormatList* GetCondFormList() const        // Ref-Undo
                    { return pCondFormList; }
    void            SetCondFormList(ScConditionalFormatList* pNew);

    ScValidationDataList* GetValidationList() const
                    { return pValidationList; }

    void            ApplyAttr( USHORT nCol, USHORT nRow, USHORT nTab,
                                const SfxPoolItem& rAttr );
    void            ApplyPattern( USHORT nCol, USHORT nRow, USHORT nTab,
                                    const ScPatternAttr& rAttr );
    void            ApplyPatternArea( USHORT nStartCol, USHORT nStartRow,
                                        USHORT nEndCol, USHORT nEndRow,
                                        const ScMarkData& rMark, const ScPatternAttr& rAttr );
    void            ApplyPatternAreaTab( USHORT nStartCol, USHORT nStartRow,
                                            USHORT nEndCol, USHORT nEndRow, USHORT nTab,
                                            const ScPatternAttr& rAttr );
    void            ApplyPatternIfNumberformatIncompatible(
                            const ScRange& rRange, const ScMarkData& rMark,
                            const ScPatternAttr& rPattern, short nNewType );

    void            ApplyStyle( USHORT nCol, USHORT nRow, USHORT nTab,
                                const ScStyleSheet& rStyle);
    void            ApplyStyleArea( USHORT nStartCol, USHORT nStartRow,
                                    USHORT nEndCol, USHORT nEndRow,
                                    const ScMarkData& rMark, const ScStyleSheet& rStyle);
    void            ApplyStyleAreaTab( USHORT nStartCol, USHORT nStartRow,
                                        USHORT nEndCol, USHORT nEndRow, USHORT nTab,
                                        const ScStyleSheet& rStyle);

    void            ApplySelectionStyle( const ScStyleSheet& rStyle, const ScMarkData& rMark );
    void            ApplySelectionLineStyle( const ScMarkData& rMark,
                                            const SvxBorderLine* pLine, BOOL bColorOnly );

    const ScStyleSheet* GetStyle( USHORT nCol, USHORT nRow, USHORT nTab ) const;
    const ScStyleSheet* GetSelectionStyle( const ScMarkData& rMark ) const;

    void            StyleSheetChanged( const SfxStyleSheetBase* pStyleSheet, BOOL bRemoved,
                                        OutputDevice* pDev,
                                        double nPPTX, double nPPTY,
                                        const Fraction& rZoomX, const Fraction& rZoomY );

    BOOL            IsStyleSheetUsed( const SfxStyleSheetBase& rStyle ) const;

                    // Rueckgabe TRUE bei ApplyFlags: Wert geaendert
    BOOL            ApplyFlags( USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow,
                                    const ScMarkData& rMark, INT16 nFlags );
    BOOL            ApplyFlagsTab( USHORT nStartCol, USHORT nStartRow,
                                            USHORT nEndCol, USHORT nEndRow,
                                            USHORT nTab, INT16 nFlags );
    BOOL            RemoveFlags( USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow,
                                    const ScMarkData& rMark, INT16 nFlags );
    BOOL            RemoveFlagsTab( USHORT nStartCol, USHORT nStartRow,
                                            USHORT nEndCol, USHORT nEndRow,
                                            USHORT nTab, INT16 nFlags );

    void            SetPattern( const ScAddress&, const ScPatternAttr& rAttr,
                                    BOOL bPutToPool = FALSE );
    void            SetPattern( USHORT nCol, USHORT nRow, USHORT nTab, const ScPatternAttr& rAttr,
                                    BOOL bPutToPool = FALSE );
    void            DeleteNumberFormat( const ULONG* pDelKeys, ULONG nCount );

    void            AutoFormat( USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow,
                                    USHORT nFormatNo, const ScMarkData& rMark );
    void            GetAutoFormatData( USHORT nTab, USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow,
                                        ScAutoFormatData& rData );
    BOOL            SearchAndReplace( const SvxSearchItem& rSearchItem,
                                        USHORT& rCol, USHORT& rRow, USHORT& rTab,
                                        ScMarkData& rMark,
                                        String& rUndoStr, ScDocument* pUndoDoc = NULL );

                    // Col/Row von Folgeaufrufen bestimmen
                    // (z.B. nicht gefunden von Anfang, oder folgende Tabellen)
    static void     GetSearchAndReplaceStart( const SvxSearchItem& rSearchItem,
                        USHORT& rCol, USHORT& rRow );

    BOOL            Solver(USHORT nFCol, USHORT nFRow, USHORT nFTab,
                            USHORT nVCol, USHORT nVRow, USHORT nVTab,
                            const String& sValStr, double& nX);

    void            ApplySelectionPattern( const ScPatternAttr& rAttr, const ScMarkData& rMark );
    void            DeleteSelection( USHORT nDelFlag, const ScMarkData& rMark );
    void            DeleteSelectionTab( USHORT nTab, USHORT nDelFlag, const ScMarkData& rMark );

                    //

    void            SetColWidth( USHORT nCol, USHORT nTab, USHORT nNewWidth );
    void            SetRowHeight( USHORT nRow, USHORT nTab, USHORT nNewHeight );
    void            SetRowHeightRange( USHORT nStartRow, USHORT nEndRow, USHORT nTab,
                                            USHORT nNewHeight );
    void            SetManualHeight( USHORT nStartRow, USHORT nEndRow, USHORT nTab, BOOL bManual );

    USHORT          GetColWidth( USHORT nCol, USHORT nTab ) const;
    USHORT          GetRowHeight( USHORT nRow, USHORT nTab ) const;
    ULONG           GetColOffset( USHORT nCol, USHORT nTab ) const;
    ULONG           GetRowOffset( USHORT nRow, USHORT nTab ) const;

    USHORT          GetOriginalWidth( USHORT nCol, USHORT nTab ) const;

    inline USHORT   FastGetRowHeight( USHORT nRow, USHORT nTab ) const;     // ohne Ueberpruefungen!

    USHORT          GetHiddenRowCount( USHORT nRow, USHORT nTab ) const;

    USHORT          GetOptimalColWidth( USHORT nCol, USHORT nTab, OutputDevice* pDev,
                                        double nPPTX, double nPPTY,
                                        const Fraction& rZoomX, const Fraction& rZoomY,
                                        BOOL bFormula,
                                        const ScMarkData* pMarkData = NULL,
                                        BOOL bSimpleTextImport = FALSE );
    BOOL            SetOptimalHeight( USHORT nStartRow, USHORT nEndRow, USHORT nTab, USHORT nExtra,
                                        OutputDevice* pDev,
                                        double nPPTX, double nPPTY,
                                        const Fraction& rZoomX, const Fraction& rZoomY,
                                        BOOL bShrink );
    long            GetNeededSize( USHORT nCol, USHORT nRow, USHORT nTab,
                                    OutputDevice* pDev,
                                    double nPPTX, double nPPTY,
                                    const Fraction& rZoomX, const Fraction& rZoomY,
                                    BOOL bWidth, BOOL bTotalSize = FALSE );

    void            ShowCol(USHORT nCol, USHORT nTab, BOOL bShow);
    void            ShowRow(USHORT nRow, USHORT nTab, BOOL bShow);
    void            ShowRows(USHORT nRow1, USHORT nRow2, USHORT nTab, BOOL bShow);
    void            SetColFlags( USHORT nCol, USHORT nTab, BYTE nNewFlags );
    void            SetRowFlags( USHORT nRow, USHORT nTab, BYTE nNewFlags );

    BYTE            GetColFlags( USHORT nCol, USHORT nTab ) const;
    BYTE            GetRowFlags( USHORT nRow, USHORT nTab ) const;

    USHORT          GetLastFlaggedCol( USHORT nTab ) const;
    USHORT          GetLastFlaggedRow( USHORT nTab ) const;
    USHORT          GetNextDifferentFlaggedCol( USHORT nTab, USHORT nStart) const;
    USHORT          GetNextDifferentFlaggedRow( USHORT nTab, USHORT nStart) const;

    BOOL            IsFiltered( USHORT nRow, USHORT nTab ) const;

    BOOL            UpdateOutlineCol( USHORT nStartCol, USHORT nEndCol, USHORT nTab, BOOL bShow );
    BOOL            UpdateOutlineRow( USHORT nStartRow, USHORT nEndRow, USHORT nTab, BOOL bShow );

    void            StripHidden( USHORT& rX1, USHORT& rY1, USHORT& rX2, USHORT& rY2, USHORT nTab );
    void            ExtendHidden( USHORT& rX1, USHORT& rY1, USHORT& rX2, USHORT& rY2, USHORT nTab );

    ScPatternAttr*      GetDefPattern() const;
    ScDocumentPool*     GetPool();
    ScStyleSheetPool*   GetStyleSheetPool() const;

    // PageStyle:
    const String&   GetPageStyle( USHORT nTab ) const;
    void            SetPageStyle( USHORT nTab, const String& rName );
    Size            GetPageSize( USHORT nTab ) const;
    void            SetPageSize( USHORT nTab, const Size& rSize );
    void            SetRepeatArea( USHORT nTab, USHORT nStartCol, USHORT nEndCol, USHORT nStartRow, USHORT nEndRow );
    void            UpdatePageBreaks();
    void            UpdatePageBreaks( USHORT nTab, const ScRange* pUserArea = NULL );
    void            RemoveManualBreaks( USHORT nTab );
    BOOL            HasManualBreaks( USHORT nTab ) const;

    BOOL            IsPageStyleInUse( const String& rStrPageStyle, USHORT* pInTab = NULL );
    BOOL            RemovePageStyleInUse( const String& rStrPageStyle );
    BOOL            RenamePageStyleInUse( const String& rOld, const String& rNew );
    void            ModifyStyleSheet( SfxStyleSheetBase& rPageStyle,
                                      const SfxItemSet&  rChanges );

    void            PageStyleModified( USHORT nTab, const String& rNewName );

    BOOL            NeedPageResetAfterTab( USHORT nTab ) const;

    // war vorher im PageStyle untergracht. Jetzt an jeder Tabelle:
    BOOL            HasPrintRange();
    USHORT          GetPrintRangeCount( USHORT nTab );
    const ScRange*  GetPrintRange( USHORT nTab, USHORT nPos );
    const ScRange*  GetRepeatColRange( USHORT nTab );
    const ScRange*  GetRepeatRowRange( USHORT nTab );
    void            SetPrintRangeCount( USHORT nTab, USHORT nNew );
    void            SetPrintRange( USHORT nTab, USHORT nPos, const ScRange& rNew );
    void            SetRepeatColRange( USHORT nTab, const ScRange* pNew );
    void            SetRepeatRowRange( USHORT nTab, const ScRange* pNew );
    ScPrintRangeSaver* CreatePrintRangeSaver() const;
    void            RestorePrintRanges( const ScPrintRangeSaver& rSaver );

    Rectangle       GetMMRect( USHORT nStartCol, USHORT nStartRow,
                                USHORT nEndCol, USHORT nEndRow, USHORT nTab );
    ScRange         GetRange( USHORT nTab, const Rectangle& rMMRect );

    BOOL            LoadPool( SvStream& rStream, BOOL bLoadRefCounts );
    BOOL            SavePool( SvStream& rStream ) const;

    BOOL            Load( SvStream& rStream, ScProgress* pProgress );
    BOOL            Save( SvStream& rStream, ScProgress* pProgress ) const;

    void            UpdStlShtPtrsFrmNms();
    void            StylesToNames();

    void            CopyStdStylesFrom( ScDocument* pSrcDoc );

    CharSet         GetSrcCharSet() const   { return eSrcSet; }
    ULONG           GetSrcVersion() const   { return nSrcVer; }
    USHORT          GetSrcMaxRow() const    { return nSrcMaxRow; }

    void            SetLostData();
    BOOL            HasLostData() const     { return bLostData; }

    void            SetSrcCharSet( CharSet eNew )   { eSrcSet = eNew; }
    void            UpdateFontCharSet();

    friend SvStream& operator>>( SvStream& rStream, ScDocument& rDocument );
    friend SvStream& operator<<( SvStream& rStream, const ScDocument& rDocument );

    USHORT          FillInfo( RowInfo* pRowInfo, USHORT nX1, USHORT nY1, USHORT nX2, USHORT nY2,
                        USHORT nTab, double nScaleX, double nScaleY,
                        BOOL bPageMode, BOOL bFormulaMode,
                        const ScMarkData* pMarkData = NULL );

    SvNumberFormatter*  GetFormatTable() const { return pFormTable; }

    void            Sort( USHORT nTab, const ScSortParam& rSortParam, BOOL bKeepQuery );
    USHORT          Query( USHORT nTab, const ScQueryParam& rQueryParam, BOOL bKeepSub );
    BOOL            ValidQuery( USHORT nRow, USHORT nTab, const ScQueryParam& rQueryParam );
    BOOL            CreateQueryParam( USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2,
                                        USHORT nTab, ScQueryParam& rQueryParam );
    void            GetUpperCellString(USHORT nCol, USHORT nRow, USHORT nTab, String& rStr);

    BOOL            GetFilterEntries( USHORT nCol, USHORT nRow, USHORT nTab,
                                TypedStrCollection& rStrings );
    BOOL            GetFilterEntriesArea( USHORT nCol, USHORT nStartRow, USHORT nEndRow,
                                USHORT nTab, TypedStrCollection& rStrings );
    BOOL            GetDataEntries( USHORT nCol, USHORT nRow, USHORT nTab,
                                TypedStrCollection& rStrings, BOOL bLimit = FALSE );
    BOOL            GetFormulaEntries( TypedStrCollection& rStrings );

    BOOL            HasAutoFilter( USHORT nCol, USHORT nRow, USHORT nTab );

    BOOL            HasColHeader( USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow,
                                    USHORT nTab );
    BOOL            HasRowHeader( USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow,
                                    USHORT nTab );

    SfxPrinter*     GetPrinter();
    void            SetPrinter( SfxPrinter* pNewPrinter );
    void            EraseNonUsedSharedNames(USHORT nLevel);
    BOOL            GetNextSpellingCell(USHORT& nCol, USHORT& nRow, USHORT nTab,
                                        BOOL bInSel, const ScMarkData& rMark) const;

    BOOL            ReplaceStyle(const SvxSearchItem& rSearchItem,
                                 USHORT nCol, USHORT nRow, USHORT nTab,
                                 ScMarkData& rMark, BOOL bIsUndo);

    void            DoColResize( USHORT nTab, USHORT nCol1, USHORT nCol2, USHORT nAdd );

    // Idleberechnung der OutputDevice-Zelltextbreite
    BOOL            IsLoadingDone() const { return bLoadingDone; }
    void            InvalidateTextWidth( const String& rStyleName );
    void            InvalidateTextWidth( USHORT nTab );
    void            InvalidateTextWidth( const ScAddress* pAdrFrom = NULL,
                                         const ScAddress* pAdrTo   = NULL,
                                         BOOL bBroadcast = FALSE );

    BOOL            IdleCalcTextWidth();
    BOOL            IdleCheckLinks();

    BOOL            ContinueOnlineSpelling();   // TRUE = etwas gefunden

    BOOL            IsIdleDisabled() const      { return bIdleDisabled; }
    void            DisableIdle(BOOL bDo)       { bIdleDisabled = bDo; }

    BOOL            IsDetectiveDirty() const     { return bDetectiveDirty; }
    void            SetDetectiveDirty(BOOL bSet) { bDetectiveDirty = bSet; }

    void            RemoveAutoSpellObj();
    void            SetOnlineSpellPos( const ScAddress& rPos );
    BOOL            SetVisibleSpellRange( const ScRange& rRange );  // TRUE = changed

    BYTE            GetMacroCallMode() const     { return nMacroCallMode; }
    void            SetMacroCallMode(BYTE nNew)  { nMacroCallMode = nNew; }

    BOOL            GetHasMacroFunc() const      { return bHasMacroFunc; }
    void            SetHasMacroFunc(BOOL bSet)   { bHasMacroFunc = bSet; }

    BOOL            HasMacroCallsAfterLoad();
    BOOL            CheckMacroWarn();

    // fuer Broadcasting/Listening
    void            SetNoSetDirty( BOOL bVal ) { bNoSetDirty = bVal; }
    BOOL            GetNoSetDirty() const { return bNoSetDirty; }
    void            SetInsertingFromOtherDoc( BOOL bVal ) { bInsertingFromOtherDoc = bVal; }
    BOOL            IsInsertingFromOtherDoc() const { return bInsertingFromOtherDoc; }
    void            SetImportingXML( BOOL bVal ) { bImportingXML = bVal; }
    BOOL            IsImportingXML() const { return bImportingXML; }
    void            SetCalcingAfterLoad( BOOL bVal ) { bCalcingAfterLoad = bVal; }
    BOOL            IsCalcingAfterLoad() const { return bCalcingAfterLoad; }
    void            SetNoListening( BOOL bVal ) { bNoListening = bVal; }
    BOOL            GetNoListening() const { return bNoListening; }

    ScChartListenerCollection* GetChartListenerCollection() const
                        { return pChartListenerCollection; }
    void            SetChartListenerCollection( ScChartListenerCollection*,
                        BOOL bSetChartRangeLists = FALSE );
    void            UpdateChart( const String& rName, Window* pWin );
    void            UpdateChartListenerCollection();
    BOOL            IsChartListenerCollectionNeedsUpdate() const
                        { return bChartListenerCollectionNeedsUpdate; }
    void            SetChartListenerCollectionNeedsUpdate( BOOL bFlg )
                        { bChartListenerCollectionNeedsUpdate = bFlg; }

    ScChangeViewSettings* GetChangeViewSettings() const     { return pChangeViewSettings; }
    void                SetChangeViewSettings(const ScChangeViewSettings& rNew);

    ScLkUpdMode     GetLinkMode() const             { return eLinkMode ;}
    void            SetLinkMode( ScLkUpdMode nSet ) {   eLinkMode  = nSet;}


private:
    void                SetAutoFilterFlags();
    void                FindMaxRotCol( USHORT nTab, RowInfo* pRowInfo, USHORT nArrCount,
                                        USHORT nX1, USHORT nX2 ) const;

    USHORT              RowDifferences( USHORT nThisRow, USHORT nThisTab,
                                        ScDocument& rOtherDoc,
                                        USHORT nOtherRow, USHORT nOtherTab,
                                        USHORT nMaxCol, USHORT* pOtherCols );
    USHORT              ColDifferences( USHORT nThisCol, USHORT nThisTab,
                                        ScDocument& rOtherDoc,
                                        USHORT nOtherCol, USHORT nOtherTab,
                                        USHORT nMaxRow, USHORT* pOtherRows );
    void                FindOrder( USHORT* pOtherRows, USHORT nThisEndRow, USHORT nOtherEndRow,
                                        BOOL bColumns,
                                        ScDocument& rOtherDoc, USHORT nThisTab, USHORT nOtherTab,
                                        USHORT nEndCol, USHORT* pTranslate,
                                        ScProgress* pProgress, ULONG nProAdd );
    BOOL                OnlineSpellInRange( const ScRange& rSpellRange, ScAddress& rSpellPos,
                                        USHORT nMaxTest );

    DECL_LINK( TrackTimeHdl, Timer* );

public:
    void                StartListeningArea( const ScRange& rRange,
                                            SfxListener* pListener );
    void                EndListeningArea( const ScRange& rRange,
                                            SfxListener* pListener );
    void                Broadcast( ULONG nHint, const ScAddress& rAddr,
                                    ScBaseCell* pCell );
        // wrapper, ruft pCell->Broadcast() und AreaBroadcast() und
        // TrackFormulas()
    void                DelBroadcastAreasInRange( const ScRange& rRange );
    void                UpdateBroadcastAreas( UpdateRefMode eUpdateRefMode,
                                            const ScRange& rRange,
                                            short nDx, short nDy, short nDz );


    void                StartListeningCell( const ScAddress& rAddress,
                                            SfxListener* pListener );
    void                EndListeningCell( const ScAddress& rAddress,
                                            SfxListener* pListener );
    void                PutInFormulaTree( ScFormulaCell* pCell );
    void                RemoveFromFormulaTree( ScFormulaCell* pCell );
    void                CalcFormulaTree( BOOL bOnlyForced = FALSE,
                                        BOOL bNoProgressBar = FALSE );
    void                ClearFormulaTree();
    void                AppendToFormulaTrack( ScFormulaCell* pCell );
    void                RemoveFromFormulaTrack( ScFormulaCell* pCell );
    void                TrackFormulas();
    USHORT              GetFormulaTrackCount() const { return nFormulaTrackCount; }
    BOOL                IsInFormulaTree( ScFormulaCell* pCell ) const;
    BOOL                IsInFormulaTrack( ScFormulaCell* pCell ) const;
    USHORT              GetHardRecalcState() { return nHardRecalcState; }
    void                SetHardRecalcState( USHORT nVal ) { nHardRecalcState = nVal; }
    void                StartAllListeners();
    const ScFormulaCell*    GetFormulaTree() const { return pFormulaTree; }
    BOOL                HasForcedFormulas() const { return bHasForcedFormulas; }
    void                SetForcedFormulas( BOOL bVal ) { bHasForcedFormulas = bVal; }
    ULONG               GetFormulaCodeInTree() const { return nFormulaCodeInTree; }
    BOOL                IsInInterpreter() const { return nInterpretLevel != 0; }
    USHORT              GetInterpretLevel() { return nInterpretLevel; }
    void                IncInterpretLevel()
                            {
                                if ( nInterpretLevel < USHRT_MAX )
                                    nInterpretLevel++;
                            }
    void                DecInterpretLevel()
                            {
                                if ( nInterpretLevel )
                                    nInterpretLevel--;
                            }
    BOOL                IsInMacroInterpreter() const { return nMacroInterpretLevel != 0; }
    USHORT              GetMacroInterpretLevel() { return nMacroInterpretLevel; }
    void                IncMacroInterpretLevel()
                            {
                                if ( nMacroInterpretLevel < USHRT_MAX )
                                    nMacroInterpretLevel++;
                            }
    void                DecMacroInterpretLevel()
                            {
                                if ( nMacroInterpretLevel )
                                    nMacroInterpretLevel--;
                            }
    BOOL                IsInDtorClear() const { return bInDtorClear; }
    void                SetExpandRefs( BOOL bVal ) { bExpandRefs = bVal; }
    BOOL                IsExpandRefs() { return bExpandRefs; }

    void                IncSizeRecalcLevel( USHORT nTab );
    void                DecSizeRecalcLevel( USHORT nTab );

    void                StartTrackTimer();

    void            CompileDBFormula();
    void            CompileDBFormula( BOOL bCreateFormulaString );
    void            CompileNameFormula( BOOL bCreateFormulaString );
    void            CompileColRowNameFormula();

    // maximale Stringlaengen einer Column, fuer z.B. dBase Export
    xub_StrLen      GetMaxStringLen( USHORT nTab, USHORT nCol,
                                    USHORT nRowStart, USHORT nRowEnd ) const;
    xub_StrLen      GetMaxNumberStringLen( USHORT& nPrecision,
                                    USHORT nTab, USHORT nCol,
                                    USHORT nRowStart, USHORT nRowEnd ) const;

    void    KeyInput( const KeyEvent& rKEvt );      // TimerDelays etc.

    ScChangeTrack*      GetChangeTrack() const { return pChangeTrack; }

    //! only for import filter, deletes any existing ChangeTrack via
    //! EndChangeTracking() and takes ownership of new ChangeTrack pTrack
    void            SetChangeTrack( ScChangeTrack* pTrack );

    void            StartChangeTracking();
    void            EndChangeTracking();

    void            CompareDocument( ScDocument& rOtherDoc );

    void            AddUnoObject( SfxListener& rObject );
    void            RemoveUnoObject( SfxListener& rObject );
    void            BroadcastUno( const SfxHint &rHint );

    void            SetInLinkUpdate(BOOL bSet);             // TableLink or AreaLink
    BOOL            IsInLinkUpdate() const;                 // including DdeLink

    SfxItemPool*        GetEditPool() const { return pEditPool; }
    SfxItemPool*        GetEnginePool() const { return pEnginePool; }
    ScFieldEditEngine&  GetEditEngine();

private: // CLOOK-Impl-Methoden
    void    ImplLoadDocOptions( SvStream& rStream );
    void    ImplLoadViewOptions( SvStream& rStream );
    void    ImplSaveDocOptions( SvStream& rStream ) const;
    void    ImplSaveViewOptions( SvStream& rStream ) const;
    void    ImplCreateOptions(); // bei Gelegenheit auf on-demand umstellen?
    void    ImplDeleteOptions();

    void    DeleteDrawLayer();
    void    DeleteColorTable();
    void    LoadDrawLayer(SvStream& rStream);
    void    StoreDrawLayer(SvStream& rStream) const;
    BOOL    DrawGetPrintArea( ScRange& rRange, BOOL bSetHor, BOOL bSetVer ) const;
    void    DrawMovePage( USHORT nOldPos, USHORT nNewPos );
    void    DrawCopyPage( USHORT nOldPos, USHORT nNewPos );

    void    UpdateDrawPrinter();
    void    InitClipPtrs( ScDocument* pSourceDoc );

    void    LoadDdeLinks(SvStream& rStream);
    void    SaveDdeLinks(SvStream& rStream) const;
    void    LoadAreaLinks(SvStream& rStream);
    void    SaveAreaLinks(SvStream& rStream) const;

    void    UpdateRefAreaLinks( UpdateRefMode eUpdateRefMode,
                             const ScRange& r, short nDx, short nDy, short nDz );

    BOOL    HasPartOfMerged( const ScRange& rRange );
};


inline USHORT ScDocument::FastGetRowHeight( USHORT nRow, USHORT nTab ) const
{
    return ( pTab[nTab]->pRowFlags[nRow] & CR_HIDDEN ) ? 0 : pTab[nTab]->pRowHeight[nRow];
}



#endif


