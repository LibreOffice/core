/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: document.hxx,v $
 *
 *  $Revision: 1.107 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 15:15:18 $
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

#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif

#ifndef INCLUDED_SCDLLAPI_H
#include "scdllapi.h"
#endif

#ifndef SC_TABLE_HXX
#include "table.hxx"        // FastGetRowHeight (inline)
#endif

#ifndef SC_RANGELST_HXX
#include "rangelst.hxx"
#endif

#ifndef _SC_BRDCST_HXX
#include "brdcst.hxx"
#endif

#ifndef SC_TABOPPARAMS_HXX
#include "tabopparams.hxx"
#endif

#include <memory>
#include <map>

class KeyEvent;
class OutputDevice;
class SdrObject;
class SfxBroadcaster;
class SfxListener;
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
class SvxForbiddenCharactersTable;
class SvxLinkManager;
class SvxSearchItem;
class SvxShadowItem;
class Window;
class XColorTable;
class List;

class ScAutoFormatData;
class ScBaseCell;
class ScStringCell;
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
class StrCollection;
class TypedStrCollection;
class ScChangeTrack;
class ScFieldEditEngine;
class ScNoteEditEngine;
struct ScConsolidateParam;
class ScDPObject;
class ScDPCollection;
class ScMatrix;
class ScScriptTypeData;
class ScPoolHelper;
class ScImpExpLogMsg;
struct ScSortParam;
class ScRefreshTimerControl;
class ScUnoListenerCalls;
class ScUnoRefList;
class ScRecursionHelper;
struct RowInfo;
struct ScTableInfo;
struct ScTabOpParam;
class VirtualDevice;
class ScAutoNameCache;
class ScTemporaryChartLock;
class ScLookupCache;
struct ScLookupCacheMapImpl;

namespace com { namespace sun { namespace star {
    namespace lang {
        class XMultiServiceFactory;
        struct EventObject;
    }
    namespace i18n {
        class XBreakIterator;
    }
    namespace util {
        class XModifyListener;
    }
    namespace embed {
        class XEmbeddedObject;
    }
} } }

#include <svtools/zforlist.hxx>
/*
#ifdef _ZFORLIST_DECLARE_TABLE
class SvNumberFormatterIndexTable;
#else
class Table;
typedef Table SvNumberFormatterIndexTable;
#endif
*/

#define SC_DOC_NEW          0xFFFF

#define SC_MACROCALL_ALLOWED        0
#define SC_MACROCALL_NOTALLOWED     1
#define SC_MACROCALL_ASK            2

#define SC_ASIANCOMPRESSION_INVALID     0xff
#define SC_ASIANKERNING_INVALID         0xff


enum ScDocumentMode
    {
        SCDOCMODE_DOCUMENT,
        SCDOCMODE_CLIP,
        SCDOCMODE_UNDO
    };


struct ScDocStat
{
    String  aDocName;
    SCTAB   nTableCount;
    ULONG   nCellCount;
    USHORT  nPageCount;
};

// nicht 11 Parameter bei CopyBlockFromClip, konstante Werte der Schleife hier
struct ScCopyBlockFromClipParams
{
    ScDocument* pRefUndoDoc;
    ScDocument* pClipDoc;
    USHORT      nInsFlag;
    SCTAB       nTabStart;
    SCTAB       nTabEnd;
    BOOL        bAsLink;
    BOOL        bSkipAttrForEmpty;
};


// for loading of binary file format symbol string cells which need font conversion
struct ScSymbolStringCellEntry
{
    ScStringCell*   pCell;
    SCROW           nRow;
};


// -----------------------------------------------------------------------

// DDE link modes
const BYTE SC_DDE_DEFAULT       = 0;
const BYTE SC_DDE_ENGLISH       = 1;
const BYTE SC_DDE_TEXT          = 2;
const BYTE SC_DDE_IGNOREMODE    = 255;       /// For usage in FindDdeLink() only!


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

    vos::ORef<ScPoolHelper> xPoolHelper;

    ScFieldEditEngine*  pEditEngine;                    // uses pEditPool from xPoolHelper
    ScNoteEditEngine*   pNoteEngine;                    // uses pEditPool from xPoolHelper
    SfxItemPool*    pNoteItemPool; // SfxItemPool to be used if pDrawLayer not created.
    SfxObjectShell*     pShell;
    SfxPrinter*         pPrinter;
    VirtualDevice*      pVirtualDevice_100th_mm;
    ScDrawLayer*        pDrawLayer;                     // SdrModel
    XColorTable*        pColorTable;
    ScConditionalFormatList* pCondFormList;             // bedingte Formate
    ScValidationDataList* pValidationList;              // Gueltigkeit
    SvNumberFormatterIndexTable*    pFormatExchangeList;            // zum Umsetzen von Zahlenformaten
    ScTable*            pTab[MAXTABCOUNT];
    ScRangeName*        pRangeName;
    ScDBCollection*     pDBCollection;
    ScPivotCollection*  pPivotCollection;
    ScDPCollection*     pDPCollection;
    ScChartCollection*  pChartCollection;
    std::auto_ptr< ScTemporaryChartLock > apTemporaryChartLock;
    ScPatternAttr*      pSelectionAttr;                 // Attribute eines Blocks
    SvxLinkManager*     pLinkManager;
    ScFormulaCell*      pFormulaTree;                   // Berechnungsbaum Start
    ScFormulaCell*      pEOFormulaTree;                 // Berechnungsbaum Ende, letzte Zelle
    ScFormulaCell*      pFormulaTrack;                  // BroadcastTrack Start
    ScFormulaCell*      pEOFormulaTrack;                // BrodcastTrack Ende, letzte Zelle
    ScBroadcastAreaSlotMachine* pBASM;                  // BroadcastAreas
    ScChartListenerCollection* pChartListenerCollection;
    StrCollection*      pOtherObjects;                  // non-chart OLE objects
    SvMemoryStream*     pClipData;
    ScDetOpList*        pDetOpList;
    ScChangeTrack*      pChangeTrack;
    SfxBroadcaster*     pUnoBroadcaster;
    ScUnoListenerCalls* pUnoListenerCalls;
    ScUnoRefList*       pUnoRefUndoList;
    ScChangeViewSettings* pChangeViewSettings;
    ScScriptTypeData*   pScriptTypeData;
    ScRefreshTimerControl* pRefreshTimerControl;
    vos::ORef<SvxForbiddenCharactersTable> xForbiddenCharacters;

    ScFieldEditEngine*  pCacheFieldEditEngine;

    com::sun::star::uno::Sequence<sal_Int8> aProtectPass;
    String              aDocName;                       // opt: Dokumentname
    ScRangePairListRef  xColNameRanges;
    ScRangePairListRef  xRowNameRanges;

    ScViewOptions*      pViewOptions;                   // View-Optionen
    ScDocOptions*       pDocOptions;                    // Dokument-Optionen
    ScExtDocOptions*    pExtDocOptions;                 // fuer Import etc.
    ScConsolidateParam* pConsolidateDlgData;

    List*               pLoadedSymbolStringCellList;    // binary file format import of symbol font string cells

    ScRecursionHelper*  pRecursionHelper;               // information for recursive and iterative cell formulas

    ScAutoNameCache*    pAutoNameCache;                 // for automatic name lookup during CompileXML

    ScLookupCacheMapImpl* pLookupCacheMapImpl;          // cache for lookups like VLOOKUP and MATCH

    sal_Int64           nUnoObjectId;                   // counted up for UNO objects

    sal_uInt32          nRangeOverflowType;             // used in (xml) loading for overflow warnings

    ScRange             aClipRange;
    ScRange             aEmbedRange;
    ScAddress           aCurTextWidthCalcPos;
    ScAddress           aOnlineSpellPos;                // within whole document
    ScRange             aVisSpellRange;
    ScAddress           aVisSpellPos;                   // within aVisSpellRange (see nVisSpellState)

    Timer               aTrackTimer;

public:
    ScTabOpList         aTableOpList;                   // list of ScInterpreterTableOpParams currently in use
    ScInterpreterTableOpParams  aLastTableOpParams;     // remember last params
private:

    LanguageType        eLanguage;                      // default language
    LanguageType        eCjkLanguage;                   // default language for asian text
    LanguageType        eCtlLanguage;                   // default language for complex text
    CharSet             eSrcSet;                        // Einlesen: Quell-Zeichensatz

    ULONG               nFormulaCodeInTree;             // FormelRPN im Formelbaum
    ULONG               nXMLImportedFormulaCount;        // progress count during XML import
    USHORT              nInterpretLevel;                // >0 wenn im Interpreter
    USHORT              nMacroInterpretLevel;           // >0 wenn Macro im Interpreter
    USHORT              nInterpreterTableOpLevel;       // >0 if in Interpreter TableOp
    SCTAB               nMaxTableNumber;
    USHORT              nSrcVer;                        // Dateiversion (Laden/Speichern)
    SCROW               nSrcMaxRow;                     // Zeilenzahl zum Laden/Speichern
    USHORT              nFormulaTrackCount;
    USHORT              nHardRecalcState;               // 0: soft, 1: hard-warn, 2: hard
    SCTAB               nVisibleTab;                    // fuer OLE etc.

    ScLkUpdMode         eLinkMode;

    BOOL                bProtected;
    BOOL                bAutoCalc;                      // Automatisch Berechnen
    BOOL                bAutoCalcShellDisabled;         // in/von/fuer ScDocShell disabled
    // ob noch ForcedFormulas berechnet werden muessen,
    // im Zusammenspiel mit ScDocShell SetDocumentModified,
    // AutoCalcShellDisabled und TrackFormulas
    BOOL                bForcedFormulaPending;
    BOOL                bCalculatingFormulaTree;
    BOOL                bIsClip;
    BOOL                bCutMode;
    BOOL                bIsUndo;
    BOOL                bIsVisible;                     // set from view ctor

    BOOL                bIsEmbedded;                    // Embedded-Bereich anzeigen/anpassen ?

    // kein SetDirty bei ScFormulaCell::CompileTokenArray sondern am Ende
    // von ScDocument::CompileAll[WithFormats], CopyScenario, CopyBlockFromClip
    BOOL                bNoSetDirty;
    // kein Broadcast, keine Listener aufbauen waehrend aus einem anderen
    // Doc (per Filter o.ae.) inserted wird, erst bei CompileAll / CalcAfterLoad
    BOOL                bInsertingFromOtherDoc;
    BOOL                bImportingXML;      // special handling of formula text
    BOOL                bXMLFromWrapper;    // distinguish ScXMLImportWrapper from external component
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

    BYTE                nAsianCompression;
    BYTE                nAsianKerning;
    BOOL                bSetDrawDefaults;

    BOOL                bPastingDrawFromOtherDoc;

    BYTE                nInDdeLinkUpdate;   // originating DDE links (stacked bool)

    BOOL                bInUnoBroadcast;
    BOOL                bInUnoListenerCall;
    ScAddress::Convention eAddrConv;

    mutable BOOL        bStyleSheetUsageInvalid;

    BOOL                bUndoEnabled;
    bool                mbAdjustHeightEnabled;
    bool                mbExecuteLinkEnabled;
    bool                mbChangeReadOnlyEnabled;    // allow changes in read-only document (for API import filters)

    sal_Int16           mnNamedRangesLockCount;

    inline BOOL         RowHidden( SCROW nRow, SCTAB nTab );        // FillInfo

public:
    ULONG           GetCellCount() const;       // alle Zellen
    ULONG           GetWeightedCount() const;   // Formeln und Edit staerker gewichtet
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

    void                    SetDrawDefaults();

    void                        SetConsolidateDlgData( const ScConsolidateParam* pData );
    const ScConsolidateParam*   GetConsolidateDlgData() const { return pConsolidateDlgData; }

    void            Clear( sal_Bool bFromDestructor = sal_False );

    ScFieldEditEngine*  CreateFieldEditEngine();
    void                DisposeFieldEditEngine(ScFieldEditEngine*& rpEditEngine);

SC_DLLPUBLIC    ScRangeName*    GetRangeName();
    void            SetRangeName( ScRangeName* pNewRangeName );
    SCTAB           GetMaxTableNumber() { return nMaxTableNumber; }
    void            SetMaxTableNumber(SCTAB nNumber) { nMaxTableNumber = nNumber; }

    ScRangePairList*    GetColNameRanges() { return &xColNameRanges; }
    ScRangePairList*    GetRowNameRanges() { return &xRowNameRanges; }
    ScRangePairListRef& GetColNameRangesRef() { return xColNameRanges; }
    ScRangePairListRef& GetRowNameRangesRef() { return xRowNameRanges; }

SC_DLLPUBLIC    ScDBCollection* GetDBCollection() const;
    void            SetDBCollection( ScDBCollection* pNewDBCollection,
                                        BOOL bRemoveAutoFilter = FALSE );
    ScDBData*       GetDBAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab,
                                        BOOL bStartOnly = FALSE) const;
    ScDBData*       GetDBAtArea(SCTAB nTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2) const;

    ScRangeData*    GetRangeAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab,
                                        BOOL bStartOnly = FALSE) const;
    ScRangeData*    GetRangeAtBlock( const ScRange& rBlock, String* pName=NULL ) const;

    ScDPCollection*     GetDPCollection();
    ScDPObject*         GetDPAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab) const;
    ScDPObject*         GetDPAtBlock( const ScRange& rBlock ) const;

    ScPivotCollection*  GetPivotCollection() const;
    void                SetPivotCollection(ScPivotCollection* pNewPivotCollection);
    ScPivot*            GetPivotAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab) const;

    ScChartCollection*  GetChartCollection() const;
    void                SetChartCollection(ScChartCollection* pNewChartCollection);

    void                StopTemporaryChartLock();

    void            EnsureGraphicNames();

    SdrObject*      GetObjectAtPoint( SCTAB nTab, const Point& rPos );
    BOOL            HasChartAtPoint( SCTAB nTab, const Point& rPos, String* pName = NULL );
    void            UpdateChartArea( const String& rChartName, const ScRange& rNewArea,
                                        BOOL bColHeaders, BOOL bRowHeaders, BOOL bAdd );
    void            UpdateChartArea( const String& rChartName,
                                    const ScRangeListRef& rNewList,
                                    BOOL bColHeaders, BOOL bRowHeaders, BOOL bAdd );
    void            GetOldChartParameters( const String& rName,
                                    ScRangeList& rRanges, BOOL& rColHeaders, BOOL& rRowHeaders );
    ::com::sun::star::uno::Reference<
            ::com::sun::star::embed::XEmbeddedObject >
                    FindOleObjectByName( const String& rName );

    void            MakeTable( SCTAB nTab );

    SCTAB           GetVisibleTab() const       { return nVisibleTab; }
    void            SetVisibleTab(SCTAB nTab)   { nVisibleTab = nTab; }

    BOOL            HasTable( SCTAB nTab ) const;
    SC_DLLPUBLIC BOOL           GetName( SCTAB nTab, String& rName ) const;
    BOOL            GetTable( const String& rName, SCTAB& rTab ) const;
    inline SCTAB    GetTableCount() const { return nMaxTableNumber; }
    SvNumberFormatterIndexTable* GetFormatExchangeList() const { return pFormatExchangeList; }

    void            SetDocProtection( BOOL bProtect, const com::sun::star::uno::Sequence <sal_Int8>& aPass );
    void            SetTabProtection( SCTAB nTab, BOOL bProtect, const com::sun::star::uno::Sequence <sal_Int8>& aPass );
    BOOL            IsDocProtected() const;
    BOOL            IsDocEditable() const;
    BOOL            IsTabProtected( SCTAB nTab ) const;
    const com::sun::star::uno::Sequence <sal_Int8>& GetDocPassword() const;
    const com::sun::star::uno::Sequence <sal_Int8>& GetTabPassword( SCTAB nTab ) const;

    void            LockTable(SCTAB nTab);
    void            UnlockTable(SCTAB nTab);

    BOOL            IsBlockEditable( SCTAB nTab, SCCOL nStartCol, SCROW nStartRow,
                                        SCCOL nEndCol, SCROW nEndRow,
                                        BOOL* pOnlyNotBecauseOfMatrix = NULL ) const;
    BOOL            IsSelectedBlockEditable( SCCOL nStartCol, SCROW nStartRow,
                                            SCCOL nEndCol, SCROW nEndRow,
                                            const ScMarkData& rMark ) const;
    BOOL            IsSelectionEditable( const ScMarkData& rMark,
                                        BOOL* pOnlyNotBecauseOfMatrix = NULL ) const;
    BOOL            IsSelectionOrBlockEditable( SCTAB nTab, SCCOL nStartCol, SCROW nStartRow,
                                        SCCOL nEndCol, SCROW nEndRow,
                                        const ScMarkData& rMark ) const;
    BOOL            IsSelectedOrBlockEditable( SCCOL nStartCol, SCROW nStartRow,
                                            SCCOL nEndCol, SCROW nEndRow,
                                            const ScMarkData& rMark ) const;

    BOOL            HasSelectedBlockMatrixFragment( SCCOL nStartCol, SCROW nStartRow,
                                            SCCOL nEndCol, SCROW nEndRow,
                                            const ScMarkData& rMark ) const;

    BOOL            GetMatrixFormulaRange( const ScAddress& rCellPos, ScRange& rMatrix );

    BOOL            IsEmbedded() const;
    void            GetEmbedded( ScRange& rRange ) const;
    void            SetEmbedded( const ScRange& rRange );
    void            ResetEmbedded();
    Rectangle       GetEmbeddedRect() const;                        // 1/100 mm
    void            SetEmbedded( const Rectangle& rRect );          // aus VisArea (1/100 mm)
    void            SnapVisArea( Rectangle& rRect ) const;          // 1/100 mm

    SC_DLLPUBLIC BOOL           ValidTabName( const String& rName ) const;
    static SC_DLLPUBLIC void      ConvertToValidTabName( String& rName, sal_Unicode cReplaceChar );
    SC_DLLPUBLIC BOOL           ValidNewTabName( const String& rName ) const;
    SC_DLLPUBLIC void           CreateValidTabName(String& rName) const;
    BOOL            InsertTab( SCTAB nPos, const String& rName,
                                BOOL bExternalDocument = FALSE );
    BOOL            DeleteTab( SCTAB nTab, ScDocument* pRefUndoDoc = NULL );
    BOOL            RenameTab( SCTAB nTab, const String& rName,
                                BOOL bUpdateRef = TRUE,
                                BOOL bExternalDocument = FALSE );
    BOOL            MoveTab( SCTAB nOldPos, SCTAB nNewPos );
    BOOL            CopyTab( SCTAB nOldPos, SCTAB nNewPos,
                                const ScMarkData* pOnlyMarked = NULL );
    ULONG           TransferTab(ScDocument* pSrcDoc, SCTAB nSrcPos, SCTAB nDestPos,
                                    BOOL bInsertNew = TRUE,
                                    BOOL bResultsOnly = FALSE );
    void            TransferDrawPage(ScDocument* pSrcDoc, SCTAB nSrcPos, SCTAB nDestPos);
    void            ClearDrawPage(SCTAB nTab);
    void            SetVisible( SCTAB nTab, BOOL bVisible );
    BOOL            IsVisible( SCTAB nTab ) const;
    void            SetLayoutRTL( SCTAB nTab, BOOL bRTL );
    BOOL            IsLayoutRTL( SCTAB nTab ) const;
    BOOL            IsNegativePage( SCTAB nTab ) const;
    void            SetScenario( SCTAB nTab, BOOL bFlag );
    BOOL            IsScenario( SCTAB nTab ) const;
    void            GetScenarioData( SCTAB nTab, String& rComment,
                                        Color& rColor, USHORT& rFlags ) const;
    void            SetScenarioData( SCTAB nTab, const String& rComment,
                                        const Color& rColor, USHORT nFlags );
    void            GetScenarioFlags( SCTAB nTab, USHORT& rFlags ) const;
    BOOL            IsActiveScenario( SCTAB nTab ) const;
    void            SetActiveScenario( SCTAB nTab, BOOL bActive );      // nur fuer Undo etc.
    SC_DLLPUBLIC ScAddress::Convention GetAddressConvention() const;
    void            SetAddressConvention( ScAddress::Convention eConv );
    BYTE            GetLinkMode( SCTAB nTab ) const;
    BOOL            IsLinked( SCTAB nTab ) const;
    const String&   GetLinkDoc( SCTAB nTab ) const;
    const String&   GetLinkFlt( SCTAB nTab ) const;
    const String&   GetLinkOpt( SCTAB nTab ) const;
    const String&   GetLinkTab( SCTAB nTab ) const;
    ULONG           GetLinkRefreshDelay( SCTAB nTab ) const;
    void            SetLink( SCTAB nTab, BYTE nMode, const String& rDoc,
                            const String& rFilter, const String& rOptions,
                            const String& rTabName, ULONG nRefreshDelay );
    BOOL            HasLink( const String& rDoc,
                            const String& rFilter, const String& rOptions ) const;
    BOOL            LinkExternalTab( SCTAB& nTab, const String& aDocTab,
                                    const String& aFileName,
                                    const String& aTabName );

    /** Creates a new sheet, and makes it linked to the specified sheet in an external document.
        @param rnTab  (out-param) Returns the sheet index, if sheet could be inserted).
        @return  TRUE = Sheet created, rnTab contains valid sheet index. */
    BOOL            InsertLinkedEmptyTab( SCTAB& rnTab, const String& rFileName,
                        const String& rFilterName, const String& rFilterOpt, const String& rTabName );

    BOOL            HasDdeLinks() const;
    BOOL            HasAreaLinks() const;
    void            UpdateDdeLinks();
    void            UpdateAreaLinks();

                    // originating DDE links
    void            IncInDdeLinkUpdate() { if ( nInDdeLinkUpdate < 255 ) ++nInDdeLinkUpdate; }
    void            DecInDdeLinkUpdate() { if ( nInDdeLinkUpdate ) --nInDdeLinkUpdate; }
    BOOL            IsInDdeLinkUpdate() const   { return nInDdeLinkUpdate != 0; }

    void            CopyDdeLinks( ScDocument* pDestDoc ) const;
    void            DisconnectDdeLinks();

                    // Fuer StarOne Api:
    USHORT          GetDdeLinkCount() const;
    BOOL            UpdateDdeLink( const String& rAppl, const String& rTopic, const String& rItem );

    /** Tries to find a DDE link with the specified connection data.
        @param rnDdePos  (out-param) Returns the index of the DDE link (does not include other links from link manager).
        @return  true = DDE link found, rnDdePos valid. */
    bool            FindDdeLink( const String& rAppl, const String& rTopic, const String& rItem, BYTE nMode, USHORT& rnDdePos );

    /** Returns the connection data of the specified DDE link.
        @param nDdePos  Index of the DDE link (does not include other links from link manager).
        @param rAppl  (out-param) The application name.
        @param rTopic  (out-param) The DDE topic.
        @param rItem  (out-param) The DDE item.
        @return  true = DDE link found, out-parameters valid. */
    bool            GetDdeLinkData( USHORT nDdePos, String& rAppl, String& rTopic, String& rItem ) const;
    /** Returns the link mode of the specified DDE link.
        @param nDdePos  Index of the DDE link (does not include other links from link manager).
        @param rnMode  (out-param) The link mode of the specified DDE link.
        @return  true = DDE link found, rnMode valid. */
    bool            GetDdeLinkMode( USHORT nDdePos, BYTE& rnMode ) const;
    /** Returns the result matrix of the specified DDE link.
        @param nDdePos  Index of the DDE link (does not include other links from link manager).
        @return  The result matrix, if the DDE link has been found, 0 otherwise. */
    const ScMatrix* GetDdeLinkResultMatrix( USHORT nDdePos ) const;

    /** Tries to find a DDE link or creates a new, if not extant.
        @param pResults  If not 0, sets the matrix as as DDE link result matrix (also for existing links).
        @return  true = DDE link found; false = Unpredictable error occured, no DDE link created. */
    bool            CreateDdeLink( const String& rAppl, const String& rTopic, const String& rItem, BYTE nMode, ScMatrix* pResults = NULL );
    /** Sets a result matrix for the specified DDE link.
        @param nDdePos  Index of the DDE link (does not include other links from link manager).
        @param pResults  The array containing all results of the DDE link (intrusive-ref-counted, do not delete).
        @return  true = DDE link found and matrix set. */
    bool            SetDdeLinkResultMatrix( USHORT nDdePos, ScMatrix* pResults );


    SfxBindings*    GetViewBindings();
    SfxObjectShell* GetDocumentShell() const    { return pShell; }
    ScDrawLayer*    GetDrawLayer()              { return pDrawLayer; }
    SfxBroadcaster* GetDrawBroadcaster();       // zwecks Header-Vermeidung
    void            BeginDrawUndo();

    void            BeginUnoRefUndo();
    bool            HasUnoRefUndo() const       { return ( pUnoRefUndoList != NULL ); }
    ScUnoRefList*   EndUnoRefUndo();            // must be deleted by caller!
    sal_Int64       GetNewUnoId();
    void            AddUnoRefChange( sal_Int64 nId, const ScRangeList& rOldRanges );

    // #109985#
    sal_Bool IsChart( const SdrObject* pObject );

    void            UpdateAllCharts();
    void            UpdateChartRef( UpdateRefMode eUpdateRefMode,
                                    SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                                    SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                                    SCsCOL nDx, SCsROW nDy, SCsTAB nDz );
                    //! setzt nur die neue RangeList, keine ChartListener o.ae.
    void            SetChartRangeList( const String& rChartName,
                        const ScRangeListRef& rNewRangeListRef );

    BOOL            HasControl( SCTAB nTab, const Rectangle& rMMRect );
    void            InvalidateControls( Window* pWin, SCTAB nTab, const Rectangle& rMMRect );

    void            StopAnimations( SCTAB nTab, Window* pWin );
    void            StartAnimations( SCTAB nTab, Window* pWin );

    BOOL            HasBackgroundDraw( SCTAB nTab, const Rectangle& rMMRect );
    BOOL            HasAnyDraw( SCTAB nTab, const Rectangle& rMMRect );

    ScOutlineTable* GetOutlineTable( SCTAB nTab, BOOL bCreate = FALSE );
    BOOL            SetOutlineTable( SCTAB nTab, const ScOutlineTable* pNewOutline );

    void            DoAutoOutline( SCCOL nStartCol, SCROW nStartRow,
                                    SCCOL nEndCol, SCROW nEndRow, SCTAB nTab );

    BOOL            DoSubTotals( SCTAB nTab, ScSubTotalParam& rParam );
    void            RemoveSubTotals( SCTAB nTab, ScSubTotalParam& rParam );
    BOOL            TestRemoveSubTotals( SCTAB nTab, const ScSubTotalParam& rParam );
    BOOL            HasSubTotalCells( const ScRange& rRange );

    void            PutCell( const ScAddress&, ScBaseCell* pCell, BOOL bForceTab = FALSE );
    void            PutCell( const ScAddress&, ScBaseCell* pCell,
                            ULONG nFormatIndex, BOOL bForceTab = FALSE);
    void            PutCell( SCCOL nCol, SCROW nRow, SCTAB nTab, ScBaseCell* pCell,
                            BOOL bForceTab = FALSE );
    void            PutCell(SCCOL nCol, SCROW nRow, SCTAB nTab, ScBaseCell* pCell,
                            ULONG nFormatIndex, BOOL bForceTab = FALSE);
                    //  return TRUE = Zahlformat gesetzt
    BOOL            SetString( SCCOL nCol, SCROW nRow, SCTAB nTab, const String& rString );
    void            SetValue( SCCOL nCol, SCROW nRow, SCTAB nTab, const double& rVal );
    void            SetNote( SCCOL nCol, SCROW nRow, SCTAB nTab, const ScPostIt& rNote );
    void            SetError( SCCOL nCol, SCROW nRow, SCTAB nTab, const USHORT nError);

    void            InsertMatrixFormula(SCCOL nCol1, SCROW nRow1,
                                        SCCOL nCol2, SCROW nRow2,
                                        const ScMarkData& rMark,
                                        const String& rFormula,
                                        const ScTokenArray* p = NULL );
    void            InsertTableOp(const ScTabOpParam& rParam,   // Mehrfachoperation
                                  SCCOL nCol1, SCROW nRow1,
                                  SCCOL nCol2, SCROW nRow2, const ScMarkData& rMark);

    SC_DLLPUBLIC void           GetString( SCCOL nCol, SCROW nRow, SCTAB nTab, String& rString );
    void            GetInputString( SCCOL nCol, SCROW nRow, SCTAB nTab, String& rString );
    double          GetValue( const ScAddress& );
    void            GetValue( SCCOL nCol, SCROW nRow, SCTAB nTab, double& rValue );
    double          RoundValueAsShown( double fVal, ULONG nFormat );
    void            GetNumberFormat( SCCOL nCol, SCROW nRow, SCTAB nTab,
                                     sal_uInt32& rFormat );
    sal_uInt32      GetNumberFormat( const ScAddress& ) const;
                    /// if no number format attribute is set the calculated
                    /// number format of the formula cell is returned
    void            GetNumberFormatInfo( short& nType, ULONG& nIndex,
                        const ScAddress& rPos, const ScFormulaCell& rFCell ) const;
    void            GetFormula( SCCOL nCol, SCROW nRow, SCTAB nTab, String& rFormula,
                                BOOL bAsciiExport = FALSE ) const;
    BOOL            GetNote( SCCOL nCol, SCROW nRow, SCTAB nTab, ScPostIt& rNote);
    void            GetCellType( SCCOL nCol, SCROW nRow, SCTAB nTab, CellType& rCellType ) const;
    CellType        GetCellType( const ScAddress& rPos ) const;
    void            GetCell( SCCOL nCol, SCROW nRow, SCTAB nTab, ScBaseCell*& rpCell ) const;
    ScBaseCell*     GetCell( const ScAddress& rPos ) const;

    void            RefreshNoteFlags();
    BOOL            HasNoteObject( SCCOL nCol, SCROW nRow, SCTAB nTab ) const;

    BOOL            HasData( SCCOL nCol, SCROW nRow, SCTAB nTab );
    SC_DLLPUBLIC BOOL           HasStringData( SCCOL nCol, SCROW nRow, SCTAB nTab ) const;
    BOOL            HasValueData( SCCOL nCol, SCROW nRow, SCTAB nTab ) const;
    USHORT          GetErrorData(SCCOL nCol, SCROW nRow, SCTAB nTab) const;
    BOOL            HasStringCells( const ScRange& rRange ) const;

    /** Returns true, if there is any data to create a selection list for rPos. */
    BOOL            HasSelectionData( SCCOL nCol, SCROW nRow, SCTAB nTab ) const;

    BOOL            ExtendMergeSel( SCCOL nStartCol, SCROW nStartRow,
                                SCCOL& rEndCol, SCROW& rEndRow, const ScMarkData& rMark,
                                BOOL bRefresh = FALSE, BOOL bAttrs = FALSE );
    BOOL            ExtendMerge( SCCOL nStartCol, SCROW nStartRow,
                                SCCOL& rEndCol, SCROW& rEndRow, SCTAB nTab,
                                BOOL bRefresh = FALSE, BOOL bAttrs = FALSE );
    BOOL            ExtendMerge( ScRange& rRange, BOOL bRefresh = FALSE, BOOL bAttrs = FALSE );
    BOOL            ExtendTotalMerge( ScRange& rRange );
    BOOL            ExtendOverlapped( SCCOL& rStartCol, SCROW& rStartRow,
                                SCCOL nEndCol, SCROW nEndRow, SCTAB nTab );
    BOOL            ExtendOverlapped( ScRange& rRange );

    BOOL            RefreshAutoFilter( SCCOL nStartCol, SCROW nStartRow,
                                SCCOL nEndCol, SCROW nEndRow, SCTAB nTab );

    void            DoMergeContents( SCTAB nTab, SCCOL nStartCol, SCROW nStartRow,
                                    SCCOL nEndCol, SCROW nEndRow );
                    //  ohne Ueberpruefung:
    void            DoMerge( SCTAB nTab, SCCOL nStartCol, SCROW nStartRow,
                                    SCCOL nEndCol, SCROW nEndRow );
    void            RemoveMerge( SCCOL nCol, SCROW nRow, SCTAB nTab );

    BOOL            IsBlockEmpty( SCTAB nTab, SCCOL nStartCol, SCROW nStartRow,
                                                SCCOL nEndCol, SCROW nEndRow ) const;
    BOOL            IsPrintEmpty( SCTAB nTab, SCCOL nStartCol, SCROW nStartRow,
                                                SCCOL nEndCol, SCROW nEndRow,
                                                BOOL bLeftIsEmpty = FALSE,
                                                ScRange* pLastRange = NULL,
                                                Rectangle* pLastMM = NULL ) const;

    BOOL            IsOverlapped( SCCOL nCol, SCROW nRow, SCTAB nTab ) const;
    BOOL            IsHorOverlapped( SCCOL nCol, SCROW nRow, SCTAB nTab ) const;
    BOOL            IsVerOverlapped( SCCOL nCol, SCROW nRow, SCTAB nTab ) const;

    BOOL            HasAttrib( SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                            SCCOL nCol2, SCROW nRow2, SCTAB nTab2, USHORT nMask );
    BOOL            HasAttrib( const ScRange& rRange, USHORT nMask );

    BOOL            HasLines( const ScRange& rRange, Rectangle& rSizes ) const;

    void            GetBorderLines( SCCOL nCol, SCROW nRow, SCTAB nTab,
                                    const SvxBorderLine** ppLeft,
                                    const SvxBorderLine** ppTop,
                                    const SvxBorderLine** ppRight,
                                    const SvxBorderLine** ppBottom ) const;

    void            ResetChanged( const ScRange& rRange );

    void            SetDirty();
    void            SetDirty( const ScRange& );
    void            SetDirtyVar();
    void            SetTableOpDirty( const ScRange& );  // for Interpreter TableOp
    void            CalcAll();
    void            CalcAfterLoad();
    void            CompileAll();
    void            CompileXML();

    ScAutoNameCache* GetAutoNameCache()     { return pAutoNameCache; }

                    // Creates a ScLookupCache cache for the range if it doesn't already exist.
    ScLookupCache & GetLookupCache( const ScRange & rRange );
                    // Only ScLookupCache ctor/dtor use Add/Remove.
    void            AddLookupCache( ScLookupCache & rCache );
    void            RemoveLookupCache( ScLookupCache & rCache );

                    // Automatisch Berechnen
    void            SetAutoCalc( BOOL bNewAutoCalc );
    BOOL            GetAutoCalc() const { return bAutoCalc; }
                    // Automatisch Berechnen in/von/fuer ScDocShell disabled
    void            SetAutoCalcShellDisabled( BOOL bNew ) { bAutoCalcShellDisabled = bNew; }
    BOOL            IsAutoCalcShellDisabled() const { return bAutoCalcShellDisabled; }
                    // ForcedFormulas zu berechnen
    void            SetForcedFormulaPending( BOOL bNew ) { bForcedFormulaPending = bNew; }
    BOOL            IsForcedFormulaPending() const { return bForcedFormulaPending; }
                    // if CalcFormulaTree() is currently running
    BOOL            IsCalculatingFormulaTree() { return bCalculatingFormulaTree; }

    void            GetErrCode( SCCOL nCol, SCROW nRow, SCTAB nTab, USHORT& rErrCode );
    USHORT          GetErrCode( const ScAddress& ) const;

    void            GetDataArea( SCTAB nTab, SCCOL& rStartCol, SCROW& rStartRow,
                                    SCCOL& rEndCol, SCROW& rEndRow, BOOL bIncludeOld );
    BOOL            GetCellArea( SCTAB nTab, SCCOL& rEndCol, SCROW& rEndRow ) const;
    BOOL            GetTableArea( SCTAB nTab, SCCOL& rEndCol, SCROW& rEndRow ) const;
    BOOL            GetPrintArea( SCTAB nTab, SCCOL& rEndCol, SCROW& rEndRow,
                                    BOOL bNotes = TRUE ) const;
    BOOL            GetPrintAreaHor( SCTAB nTab, SCROW nStartRow, SCROW nEndRow,
                                        SCCOL& rEndCol, BOOL bNotes = TRUE ) const;
    BOOL            GetPrintAreaVer( SCTAB nTab, SCCOL nStartCol, SCCOL nEndCol,
                                        SCROW& rEndRow, BOOL bNotes = TRUE ) const;
    void            InvalidateTableArea();

    BOOL            GetDataStart( SCTAB nTab, SCCOL& rStartCol, SCROW& rStartRow ) const;

    void            ExtendPrintArea( OutputDevice* pDev, SCTAB nTab,
                                    SCCOL nStartCol, SCROW nStartRow,
                                    SCCOL& rEndCol, SCROW nEndRow );

    SCSIZE          GetEmptyLinesInBlock( SCCOL nStartCol, SCROW nStartRow, SCTAB nStartTab,
                                            SCCOL nEndCol, SCROW nEndRow, SCTAB nEndTab,
                                            ScDirection eDir );

    void            FindAreaPos( SCCOL& rCol, SCROW& rRow, SCTAB nTab, SCsCOL nMovX, SCsROW nMovY );
    SC_DLLPUBLIC void           GetNextPos( SCCOL& rCol, SCROW& rRow, SCTAB nTab, SCsCOL nMovX, SCsROW nMovY,
                                BOOL bMarked, BOOL bUnprotected, const ScMarkData& rMark );

    BOOL            GetNextMarkedCell( SCCOL& rCol, SCROW& rRow, SCTAB nTab,
                                        const ScMarkData& rMark );

    void            LimitChartArea( SCTAB nTab, SCCOL& rStartCol, SCROW& rStartRow,
                                                    SCCOL& rEndCol, SCROW& rEndRow );
    void            LimitChartIfAll( ScRangeListRef& rRangeList );

    BOOL            InsertRow( SCCOL nStartCol, SCTAB nStartTab,
                               SCCOL nEndCol,   SCTAB nEndTab,
                               SCROW nStartRow, SCSIZE nSize, ScDocument* pRefUndoDoc = NULL );
    BOOL            InsertRow( const ScRange& rRange, ScDocument* pRefUndoDoc = NULL );
    void            DeleteRow( SCCOL nStartCol, SCTAB nStartTab,
                               SCCOL nEndCol,   SCTAB nEndTab,
                               SCROW nStartRow, SCSIZE nSize,
                               ScDocument* pRefUndoDoc = NULL, BOOL* pUndoOutline = NULL );
    void            DeleteRow( const ScRange& rRange,
                               ScDocument* pRefUndoDoc = NULL, BOOL* pUndoOutline = NULL );
    BOOL            InsertCol( SCROW nStartRow, SCTAB nStartTab,
                               SCROW nEndRow,   SCTAB nEndTab,
                               SCCOL nStartCol, SCSIZE nSize, ScDocument* pRefUndoDoc = NULL );
    BOOL            InsertCol( const ScRange& rRange, ScDocument* pRefUndoDoc = NULL );
    void            DeleteCol( SCROW nStartRow, SCTAB nStartTab,
                               SCROW nEndRow, SCTAB nEndTab,
                               SCCOL nStartCol, SCSIZE nSize,
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
    BOOL            IsUndoEnabled() const                       { return bUndoEnabled; }
    void            EnableUndo( BOOL bVal )                     { bUndoEnabled = bVal; }
    bool            IsAdjustHeightEnabled() const               { return mbAdjustHeightEnabled; }
    void            EnableAdjustHeight( bool bVal )             { mbAdjustHeightEnabled = bVal; }
    bool            IsExecuteLinkEnabled() const                { return mbExecuteLinkEnabled; }
    void            EnableExecuteLink( bool bVal )              { mbExecuteLinkEnabled = bVal; }
    bool            IsChangeReadOnlyEnabled() const             { return mbChangeReadOnlyEnabled; }
    void            EnableChangeReadOnly( bool bVal )           { mbChangeReadOnlyEnabled = bVal; }
    sal_Int16       GetNamedRangesLockCount() const             { return mnNamedRangesLockCount; }
    void            SetNamedRangesLockCount( sal_Int16 nCount ) { mnNamedRangesLockCount = nCount; }
    void            ResetClip( ScDocument* pSourceDoc, const ScMarkData* pMarks );
    void            ResetClip( ScDocument* pSourceDoc, SCTAB nTab );
    void            SetCutMode( BOOL bCut );
    BOOL            IsCutMode();
    void            SetClipArea( const ScRange& rArea, BOOL bCut = FALSE );

    BOOL            IsDocVisible() const                        { return bIsVisible; }
    void            SetDocVisible( BOOL bSet );

    BOOL            HasOLEObjectsInArea( const ScRange& rRange, const ScMarkData* pTabMark = NULL );

    void            DeleteObjectsInArea( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                                        const ScMarkData& rMark );
    void            DeleteObjectsInSelection( const ScMarkData& rMark );
    void            DeleteObjects( SCTAB nTab );

    void            DeleteArea(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                            const ScMarkData& rMark, USHORT nDelFlag);
    void            DeleteAreaTab(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                                SCTAB nTab, USHORT nDelFlag);
    void            DeleteAreaTab(const ScRange& rRange, USHORT nDelFlag);
    void            CopyToClip(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                                BOOL bCut, ScDocument* pClipDoc, BOOL bAllTabs,
                                const ScMarkData* pMarks = NULL,
                                BOOL bKeepScenarioFlags = FALSE, BOOL bIncludeObjects = FALSE);
    void            CopyTabToClip(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                                SCTAB nTab, ScDocument* pClipDoc = NULL);
    void            CopyBlockFromClip( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                                    const ScMarkData& rMark, SCsCOL nDx, SCsROW nDy,
                                    const ScCopyBlockFromClipParams* pCBFCP );
    void            CopyNonFilteredFromClip( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                                    const ScMarkData& rMark, SCsCOL nDx, SCsROW nDy,
                                    const ScCopyBlockFromClipParams* pCBFCP );
    void            StartListeningFromClip( SCCOL nCol1, SCROW nRow1,
                                        SCCOL nCol2, SCROW nRow2,
                                        const ScMarkData& rMark, USHORT nInsFlag );
    void            BroadcastFromClip( SCCOL nCol1, SCROW nRow1,
                                    SCCOL nCol2, SCROW nRow2,
                                    const ScMarkData& rMark, USHORT nInsFlag );
    void            CopyFromClip( const ScRange& rDestRange, const ScMarkData& rMark,
                                    USHORT nInsFlag,
                                    ScDocument* pRefUndoDoc = NULL,
                                    ScDocument* pClipDoc = NULL,
                                    BOOL bResetCut = TRUE,
                                    BOOL bAsLink = FALSE,
                                    BOOL bIncludeFiltered = TRUE,
                                    BOOL bSkipAttrForEmpty = FALSE );

    void            GetClipArea(SCCOL& nClipX, SCROW& nClipY, BOOL bIncludeFiltered);
    void            GetClipStart(SCCOL& nClipX, SCROW& nClipY);

    BOOL            HasClipFilteredRows();

    BOOL            IsClipboardSource() const;

    void            TransposeClip( ScDocument* pTransClip, USHORT nFlags, BOOL bAsLink );

    void            MixDocument( const ScRange& rRange, USHORT nFunction, BOOL bSkipEmpty,
                                    ScDocument* pSrcDoc );

    void            FillTab( const ScRange& rSrcArea, const ScMarkData& rMark,
                                USHORT nFlags, USHORT nFunction,
                                BOOL bSkipEmpty, BOOL bAsLink );
    void            FillTabMarked( SCTAB nSrcTab, const ScMarkData& rMark,
                                USHORT nFlags, USHORT nFunction,
                                BOOL bSkipEmpty, BOOL bAsLink );

    void            TransliterateText( const ScMarkData& rMultiMark, sal_Int32 nType );

    void            InitUndo( ScDocument* pSrcDoc, SCTAB nTab1, SCTAB nTab2,
                                BOOL bColInfo = FALSE, BOOL bRowInfo = FALSE );
    void            AddUndoTab( SCTAB nTab1, SCTAB nTab2,
                                BOOL bColInfo = FALSE, BOOL bRowInfo = FALSE );
    void            InitUndoSelected( ScDocument* pSrcDoc, const ScMarkData& rTabSelection,
                                BOOL bColInfo = FALSE, BOOL bRowInfo = FALSE );

                    //  nicht mehr benutzen:
    void            CopyToDocument(SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                                SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                                USHORT nFlags, BOOL bMarked, ScDocument* pDestDoc,
                                const ScMarkData* pMarks = NULL, BOOL bColRowFlags = TRUE);
    void            UndoToDocument(SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                                SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                                USHORT nFlags, BOOL bMarked, ScDocument* pDestDoc,
                                const ScMarkData* pMarks = NULL);

    void            CopyToDocument(const ScRange& rRange,
                                USHORT nFlags, BOOL bMarked, ScDocument* pDestDoc,
                                const ScMarkData* pMarks = NULL, BOOL bColRowFlags = TRUE);
    void            UndoToDocument(const ScRange& rRange,
                                USHORT nFlags, BOOL bMarked, ScDocument* pDestDoc,
                                const ScMarkData* pMarks = NULL);

    void            CopyScenario( SCTAB nSrcTab, SCTAB nDestTab, BOOL bNewScenario = FALSE );
    BOOL            TestCopyScenario( SCTAB nSrcTab, SCTAB nDestTab ) const;
    void            MarkScenario( SCTAB nSrcTab, SCTAB nDestTab,
                                    ScMarkData& rDestMark, BOOL bResetMark = TRUE,
                                    USHORT nNeededBits = 0 ) const;
    BOOL            HasScenarioRange( SCTAB nTab, const ScRange& rRange ) const;
    const ScRangeList* GetScenarioRanges( SCTAB nTab ) const;

    void            CopyUpdated( ScDocument* pPosDoc, ScDocument* pDestDoc );

    void            UpdateReference( UpdateRefMode eUpdateRefMode, SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                                     SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                                     SCsCOL nDx, SCsROW nDy, SCsTAB nDz,
                                     ScDocument* pUndoDoc = NULL, BOOL bIncludeDraw = TRUE );

    void            UpdateTranspose( const ScAddress& rDestPos, ScDocument* pClipDoc,
                                        const ScMarkData& rMark, ScDocument* pUndoDoc = NULL );

    void            UpdateGrow( const ScRange& rArea, SCCOL nGrowX, SCROW nGrowY );

    void            Fill(   SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                            const ScMarkData& rMark,
                            ULONG nFillCount, FillDir eFillDir = FILL_TO_BOTTOM,
                            FillCmd eFillCmd = FILL_LINEAR, FillDateCmd eFillDateCmd = FILL_DAY,
                            double nStepValue = 1.0, double nMaxValue = 1E307);
    String          GetAutoFillPreview( const ScRange& rSource, SCCOL nEndX, SCROW nEndY );

    BOOL            GetSelectionFunction( ScSubTotalFunc eFunc,
                                            const ScAddress& rCursor, const ScMarkData& rMark,
                                            double& rResult );

    SC_DLLPUBLIC const SfxPoolItem*         GetAttr( SCCOL nCol, SCROW nRow, SCTAB nTab, USHORT nWhich ) const;
    const ScPatternAttr*    GetPattern( SCCOL nCol, SCROW nRow, SCTAB nTab ) const;
    const ScPatternAttr*    GetMostUsedPattern( SCCOL nCol, SCROW nStartRow, SCROW nEndRow, SCTAB nTab ) const;
    const ScPatternAttr*    GetSelectionPattern( const ScMarkData& rMark, BOOL bDeep = TRUE );
    ScPatternAttr*          CreateSelectionPattern( const ScMarkData& rMark, BOOL bDeep = TRUE );

    const ScConditionalFormat* GetCondFormat( SCCOL nCol, SCROW nRow, SCTAB nTab ) const;
    const SfxItemSet*   GetCondResult( SCCOL nCol, SCROW nRow, SCTAB nTab ) const;
    const SfxPoolItem*  GetEffItem( SCCOL nCol, SCROW nRow, SCTAB nTab, USHORT nWhich ) const;

    const ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XBreakIterator >& GetBreakIterator();
    BOOL            HasStringWeakCharacters( const String& rString );
    BYTE            GetStringScriptType( const String& rString );
    BYTE            GetCellScriptType( ScBaseCell* pCell, ULONG nNumberFormat );
    BYTE            GetScriptType( SCCOL nCol, SCROW nRow, SCTAB nTab, ScBaseCell* pCell = NULL );

    BOOL            HasDetectiveOperations() const;
    void            AddDetectiveOperation( const ScDetOpData& rData );
    void            ClearDetectiveOperations();
    ScDetOpList*    GetDetOpList() const                { return pDetOpList; }
    void            SetDetOpList(ScDetOpList* pNew);

    BOOL            HasDetectiveObjects(SCTAB nTab) const;

    void            GetSelectionFrame( const ScMarkData& rMark,
                                       SvxBoxItem&      rLineOuter,
                                       SvxBoxInfoItem&  rLineInner );
    void            ApplySelectionFrame( const ScMarkData& rMark,
                                         const SvxBoxItem* pLineOuter,
                                         const SvxBoxInfoItem* pLineInner );
    void            ApplyFrameAreaTab( const ScRange& rRange,
                                         const SvxBoxItem* pLineOuter,
                                         const SvxBoxInfoItem* pLineInner );

    void            ClearSelectionItems( const USHORT* pWhich, const ScMarkData& rMark );
    void            ChangeSelectionIndent( BOOL bIncrement, const ScMarkData& rMark );

    ULONG           AddCondFormat( const ScConditionalFormat& rNew );
    void            FindConditionalFormat( ULONG nKey, ScRangeList& rRanges );
    void            FindConditionalFormat( ULONG nKey, ScRangeList& rRanges, SCTAB nTab );
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

    SC_DLLPUBLIC void           ApplyAttr( SCCOL nCol, SCROW nRow, SCTAB nTab,
                                const SfxPoolItem& rAttr );
    void            ApplyPattern( SCCOL nCol, SCROW nRow, SCTAB nTab,
                                    const ScPatternAttr& rAttr );
    void            ApplyPatternArea( SCCOL nStartCol, SCROW nStartRow,
                                        SCCOL nEndCol, SCROW nEndRow,
                                        const ScMarkData& rMark, const ScPatternAttr& rAttr );
    void            ApplyPatternAreaTab( SCCOL nStartCol, SCROW nStartRow,
                                            SCCOL nEndCol, SCROW nEndRow, SCTAB nTab,
                                            const ScPatternAttr& rAttr );
    void            ApplyPatternIfNumberformatIncompatible(
                            const ScRange& rRange, const ScMarkData& rMark,
                            const ScPatternAttr& rPattern, short nNewType );

    void            ApplyStyle( SCCOL nCol, SCROW nRow, SCTAB nTab,
                                const ScStyleSheet& rStyle);
    void            ApplyStyleArea( SCCOL nStartCol, SCROW nStartRow,
                                    SCCOL nEndCol, SCROW nEndRow,
                                    const ScMarkData& rMark, const ScStyleSheet& rStyle);
    void            ApplyStyleAreaTab( SCCOL nStartCol, SCROW nStartRow,
                                        SCCOL nEndCol, SCROW nEndRow, SCTAB nTab,
                                        const ScStyleSheet& rStyle);

    void            ApplySelectionStyle( const ScStyleSheet& rStyle, const ScMarkData& rMark );
    void            ApplySelectionLineStyle( const ScMarkData& rMark,
                                            const SvxBorderLine* pLine, BOOL bColorOnly );

    const ScStyleSheet* GetStyle( SCCOL nCol, SCROW nRow, SCTAB nTab ) const;
    const ScStyleSheet* GetSelectionStyle( const ScMarkData& rMark ) const;

    void            StyleSheetChanged( const SfxStyleSheetBase* pStyleSheet, BOOL bRemoved,
                                        OutputDevice* pDev,
                                        double nPPTX, double nPPTY,
                                        const Fraction& rZoomX, const Fraction& rZoomY );

    BOOL            IsStyleSheetUsed( const ScStyleSheet& rStyle, BOOL bGatherAllStyles ) const;

                    // Rueckgabe TRUE bei ApplyFlags: Wert geaendert
    BOOL            ApplyFlags( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                                    const ScMarkData& rMark, INT16 nFlags );
    BOOL            ApplyFlagsTab( SCCOL nStartCol, SCROW nStartRow,
                                            SCCOL nEndCol, SCROW nEndRow,
                                            SCTAB nTab, INT16 nFlags );
    BOOL            RemoveFlags( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                                    const ScMarkData& rMark, INT16 nFlags );
    BOOL            RemoveFlagsTab( SCCOL nStartCol, SCROW nStartRow,
                                            SCCOL nEndCol, SCROW nEndRow,
                                            SCTAB nTab, INT16 nFlags );

    void            SetPattern( const ScAddress&, const ScPatternAttr& rAttr,
                                    BOOL bPutToPool = FALSE );
    void            SetPattern( SCCOL nCol, SCROW nRow, SCTAB nTab, const ScPatternAttr& rAttr,
                                    BOOL bPutToPool = FALSE );
    void            DeleteNumberFormat( const sal_uInt32* pDelKeys, sal_uInt32 nCount );

    void            AutoFormat( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                                    USHORT nFormatNo, const ScMarkData& rMark );
    void            GetAutoFormatData( SCTAB nTab, SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                                        ScAutoFormatData& rData );
    BOOL            SearchAndReplace( const SvxSearchItem& rSearchItem,
                                        SCCOL& rCol, SCROW& rRow, SCTAB& rTab,
                                        ScMarkData& rMark,
                                        String& rUndoStr, ScDocument* pUndoDoc = NULL );

                    // Col/Row von Folgeaufrufen bestimmen
                    // (z.B. nicht gefunden von Anfang, oder folgende Tabellen)
    static void     GetSearchAndReplaceStart( const SvxSearchItem& rSearchItem,
                        SCCOL& rCol, SCROW& rRow );

    BOOL            Solver(SCCOL nFCol, SCROW nFRow, SCTAB nFTab,
                            SCCOL nVCol, SCROW nVRow, SCTAB nVTab,
                            const String& sValStr, double& nX);

    void            ApplySelectionPattern( const ScPatternAttr& rAttr, const ScMarkData& rMark );
    void            DeleteSelection( USHORT nDelFlag, const ScMarkData& rMark );
    void            DeleteSelectionTab( SCTAB nTab, USHORT nDelFlag, const ScMarkData& rMark );

                    //

    void            SetColWidth( SCCOL nCol, SCTAB nTab, USHORT nNewWidth );
    void            SetRowHeight( SCROW nRow, SCTAB nTab, USHORT nNewHeight );
    void            SetRowHeightRange( SCROW nStartRow, SCROW nEndRow, SCTAB nTab,
                                            USHORT nNewHeight );
    void            SetManualHeight( SCROW nStartRow, SCROW nEndRow, SCTAB nTab, BOOL bManual );

    USHORT          GetColWidth( SCCOL nCol, SCTAB nTab ) const;
    USHORT          GetRowHeight( SCROW nRow, SCTAB nTab ) const;
    ULONG           GetRowHeight( SCROW nStartRow, SCROW nEndRow, SCTAB nTab ) const;
    ULONG           GetScaledRowHeight( SCROW nStartRow, SCROW nEndRow, SCTAB nTab, double fScale ) const;
    const ScSummableCompressedArray< SCROW, USHORT> & GetRowHeightArray( SCTAB nTab ) const;
    ULONG           GetColOffset( SCCOL nCol, SCTAB nTab ) const;
    ULONG           GetRowOffset( SCROW nRow, SCTAB nTab ) const;

    SC_DLLPUBLIC USHORT         GetOriginalWidth( SCCOL nCol, SCTAB nTab ) const;
    SC_DLLPUBLIC USHORT         GetOriginalHeight( SCROW nRow, SCTAB nTab ) const;

    USHORT          GetCommonWidth( SCCOL nEndCol, SCTAB nTab ) const;

                    // All FastGet...() methods have no check for valid nTab!
                    // They access ScCompressedArray objects, so using the
                    // single row taking ones in loops to access a sequence of
                    // single rows is no good idea! Use specialized range
                    // taking methods instead, or iterators.
    inline ULONG    FastGetRowHeight( SCROW nStartRow, SCROW nEndRow,
                        SCTAB nTab ) const;
    inline ULONG    FastGetScaledRowHeight( SCROW nStartRow, SCROW nEndRow,
                        SCTAB nTab, double fScale ) const;
    inline USHORT   FastGetRowHeight( SCROW nRow, SCTAB nTab ) const;
    inline SCROW    FastGetRowForHeight( SCTAB nTab, ULONG nHeight ) const;
    inline SCROW    FastGetFirstNonHiddenRow( SCROW nStartRow, SCTAB nTab ) const;
                    /** No check for flags whether row is hidden, height value
                        is returned unconditionally. */
    inline USHORT   FastGetOriginalRowHeight( SCROW nRow, SCTAB nTab ) const;

    SCROW           GetHiddenRowCount( SCROW nRow, SCTAB nTab ) const;

    USHORT          GetOptimalColWidth( SCCOL nCol, SCTAB nTab, OutputDevice* pDev,
                                        double nPPTX, double nPPTY,
                                        const Fraction& rZoomX, const Fraction& rZoomY,
                                        BOOL bFormula,
                                        const ScMarkData* pMarkData = NULL,
                                        BOOL bSimpleTextImport = FALSE );
    BOOL            SetOptimalHeight( SCROW nStartRow, SCROW nEndRow, SCTAB nTab, USHORT nExtra,
                                        OutputDevice* pDev,
                                        double nPPTX, double nPPTY,
                                        const Fraction& rZoomX, const Fraction& rZoomY,
                                        BOOL bShrink );
    long            GetNeededSize( SCCOL nCol, SCROW nRow, SCTAB nTab,
                                    OutputDevice* pDev,
                                    double nPPTX, double nPPTY,
                                    const Fraction& rZoomX, const Fraction& rZoomY,
                                    BOOL bWidth, BOOL bTotalSize = FALSE );

    void            ShowCol(SCCOL nCol, SCTAB nTab, BOOL bShow);
    void            ShowRow(SCROW nRow, SCTAB nTab, BOOL bShow);
    void            ShowRows(SCROW nRow1, SCROW nRow2, SCTAB nTab, BOOL bShow);
    void            SetColFlags( SCCOL nCol, SCTAB nTab, BYTE nNewFlags );
    void            SetRowFlags( SCROW nRow, SCTAB nTab, BYTE nNewFlags );
    void            SetRowFlags( SCROW nStartRow, SCROW nEndRow, SCTAB nTab, BYTE nNewFlags );

    SC_DLLPUBLIC BYTE           GetColFlags( SCCOL nCol, SCTAB nTab ) const;
    SC_DLLPUBLIC BYTE           GetRowFlags( SCROW nRow, SCTAB nTab ) const;

    const ScBitMaskCompressedArray< SCROW, BYTE> & GetRowFlagsArray( SCTAB nTab ) const;
          ScBitMaskCompressedArray< SCROW, BYTE> & GetRowFlagsArrayModifiable( SCTAB nTab );

                    /// @return  the index of the last column with any set flags (auto-pagebreak is ignored).
    SCCOL           GetLastFlaggedCol( SCTAB nTab ) const;
                    /// @return  the index of the last row with any set flags (auto-pagebreak is ignored).
    SCROW           GetLastFlaggedRow( SCTAB nTab ) const;

                    /// @return  the index of the last changed column (flags and column width, auto pagebreak is ignored).
    SCCOL           GetLastChangedCol( SCTAB nTab ) const;
                    /// @return  the index of the last changed row (flags and row height, auto pagebreak is ignored).
    SCROW           GetLastChangedRow( SCTAB nTab ) const;

    SCCOL           GetNextDifferentChangedCol( SCTAB nTab, SCCOL nStart) const;

                    // #108550#; if bCareManualSize is set then the row
                    // heights are compared only if the manual size flag for
                    // the row is set. If the bCareManualSize is not set then
                    // the row heights are always compared.
    SCROW           GetNextDifferentChangedRow( SCTAB nTab, SCROW nStart, bool bCareManualSize = true) const;

    // returns whether to export a Default style for this col/row or not
    // nDefault is setted to one possition in the current row/col where the Default style is
    BOOL            GetColDefault( SCTAB nTab, SCCOL nCol, SCROW nLastRow, SCROW& nDefault);
    BOOL            GetRowDefault( SCTAB nTab, SCROW nRow, SCCOL nLastCol, SCCOL& nDefault);

    BOOL            IsFiltered( SCROW nRow, SCTAB nTab ) const;

    BOOL            UpdateOutlineCol( SCCOL nStartCol, SCCOL nEndCol, SCTAB nTab, BOOL bShow );
    BOOL            UpdateOutlineRow( SCROW nStartRow, SCROW nEndRow, SCTAB nTab, BOOL bShow );

    void            StripHidden( SCCOL& rX1, SCROW& rY1, SCCOL& rX2, SCROW& rY2, SCTAB nTab );
    void            ExtendHidden( SCCOL& rX1, SCROW& rY1, SCCOL& rX2, SCROW& rY2, SCTAB nTab );

    ScPatternAttr*      GetDefPattern() const;
    ScDocumentPool*     GetPool();
    ScStyleSheetPool*   GetStyleSheetPool() const;

    // PageStyle:
    SC_DLLPUBLIC const String&  GetPageStyle( SCTAB nTab ) const;
    void            SetPageStyle( SCTAB nTab, const String& rName );
    Size            GetPageSize( SCTAB nTab ) const;
    void            SetPageSize( SCTAB nTab, const Size& rSize );
    void            SetRepeatArea( SCTAB nTab, SCCOL nStartCol, SCCOL nEndCol, SCROW nStartRow, SCROW nEndRow );
    void            UpdatePageBreaks();
    void            UpdatePageBreaks( SCTAB nTab, const ScRange* pUserArea = NULL );
    void            RemoveManualBreaks( SCTAB nTab );
    BOOL            HasManualBreaks( SCTAB nTab ) const;

    BOOL            IsPageStyleInUse( const String& rStrPageStyle, SCTAB* pInTab = NULL );
    BOOL            RemovePageStyleInUse( const String& rStrPageStyle );
    BOOL            RenamePageStyleInUse( const String& rOld, const String& rNew );
    void            ModifyStyleSheet( SfxStyleSheetBase& rPageStyle,
                                      const SfxItemSet&  rChanges );

    void            PageStyleModified( SCTAB nTab, const String& rNewName );

    BOOL            NeedPageResetAfterTab( SCTAB nTab ) const;

    // war vorher im PageStyle untergracht. Jetzt an jeder Tabelle:
    BOOL            HasPrintRange();
    USHORT          GetPrintRangeCount( SCTAB nTab );
    const ScRange*  GetPrintRange( SCTAB nTab, USHORT nPos );
    const ScRange*  GetRepeatColRange( SCTAB nTab );
    const ScRange*  GetRepeatRowRange( SCTAB nTab );
    /** Returns true, if the specified sheet is always printed. */
    BOOL            IsPrintEntireSheet( SCTAB nTab ) const;

    /** Removes all print ranges. */
    void            ClearPrintRanges( SCTAB nTab );
    /** Adds a new print ranges. */
    void            AddPrintRange( SCTAB nTab, const ScRange& rNew );
    /** Removes all old print ranges and sets the passed print ranges. */
    void            SetPrintRange( SCTAB nTab, const ScRange& rNew );
    /** Marks the specified sheet to be printed completely. Deletes old print ranges on the sheet! */
    void            SetPrintEntireSheet( SCTAB nTab );
    void            SetRepeatColRange( SCTAB nTab, const ScRange* pNew );
    void            SetRepeatRowRange( SCTAB nTab, const ScRange* pNew );
    ScPrintRangeSaver* CreatePrintRangeSaver() const;
    void            RestorePrintRanges( const ScPrintRangeSaver& rSaver );

    Rectangle       GetMMRect( SCCOL nStartCol, SCROW nStartRow,
                                SCCOL nEndCol, SCROW nEndRow, SCTAB nTab );
    ScRange         GetRange( SCTAB nTab, const Rectangle& rMMRect );

    BOOL            LoadPool( SvStream& rStream, BOOL bLoadRefCounts );
    BOOL            SavePool( SvStream& rStream ) const;

    BOOL            Load( SvStream& rStream, ScProgress* pProgress );
    BOOL            Save( SvStream& rStream, ScProgress* pProgress ) const;

    void            UpdStlShtPtrsFrmNms();
    void            StylesToNames();

    void            CopyStdStylesFrom( ScDocument* pSrcDoc );

    CharSet         GetSrcCharSet() const   { return eSrcSet; }
    ULONG           GetSrcVersion() const   { return nSrcVer; }
    SCROW           GetSrcMaxRow() const    { return nSrcMaxRow; }

    void            SetLostData();
    BOOL            HasLostData() const     { return bLostData; }

    void            SetSrcCharSet( CharSet eNew )   { eSrcSet = eNew; }
    void            UpdateFontCharSet();

    friend SvStream& operator>>( SvStream& rStream, ScDocument& rDocument );
    friend SvStream& operator<<( SvStream& rStream, const ScDocument& rDocument );

    void            FillInfo( ScTableInfo& rTabInfo, SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2,
                        SCTAB nTab, double nScaleX, double nScaleY,
                        BOOL bPageMode, BOOL bFormulaMode,
                        const ScMarkData* pMarkData = NULL );

SC_DLLPUBLIC    SvNumberFormatter*  GetFormatTable() const;

    void            Sort( SCTAB nTab, const ScSortParam& rSortParam, BOOL bKeepQuery );
    SCSIZE          Query( SCTAB nTab, const ScQueryParam& rQueryParam, BOOL bKeepSub );
    BOOL            ValidQuery( SCROW nRow, SCTAB nTab, const ScQueryParam& rQueryParam, BOOL* pSpecial = NULL );
    BOOL            CreateQueryParam( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                                        SCTAB nTab, ScQueryParam& rQueryParam );
    void            GetUpperCellString(SCCOL nCol, SCROW nRow, SCTAB nTab, String& rStr);

    BOOL            GetFilterEntries( SCCOL nCol, SCROW nRow, SCTAB nTab,
                                TypedStrCollection& rStrings, bool bFilter = false );
    SC_DLLPUBLIC BOOL           GetFilterEntriesArea( SCCOL nCol, SCROW nStartRow, SCROW nEndRow,
                                SCTAB nTab, TypedStrCollection& rStrings );
    BOOL            GetDataEntries( SCCOL nCol, SCROW nRow, SCTAB nTab,
                                TypedStrCollection& rStrings, BOOL bLimit = FALSE );
    BOOL            GetFormulaEntries( TypedStrCollection& rStrings );

    BOOL            HasAutoFilter( SCCOL nCol, SCROW nRow, SCTAB nTab );

    SC_DLLPUBLIC BOOL           HasColHeader( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                                    SCTAB nTab );
    SC_DLLPUBLIC BOOL           HasRowHeader( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                                    SCTAB nTab );

    SfxPrinter*     GetPrinter( BOOL bCreateIfNotExist = TRUE );
    void            SetPrinter( SfxPrinter* pNewPrinter );
    VirtualDevice*  GetVirtualDevice_100th_mm();
    OutputDevice*   GetRefDevice(); // WYSIWYG: Printer, otherwise VirtualDevice...

    void            EraseNonUsedSharedNames(USHORT nLevel);
    BOOL            GetNextSpellingCell(SCCOL& nCol, SCROW& nRow, SCTAB nTab,
                                        BOOL bInSel, const ScMarkData& rMark) const;

    BOOL            ReplaceStyle(const SvxSearchItem& rSearchItem,
                                 SCCOL nCol, SCROW nRow, SCTAB nTab,
                                 ScMarkData& rMark, BOOL bIsUndo);

    void            DoColResize( SCTAB nTab, SCCOL nCol1, SCCOL nCol2, SCSIZE nAdd );

    // Idleberechnung der OutputDevice-Zelltextbreite
    BOOL            IsLoadingDone() const { return bLoadingDone; }
    void            InvalidateTextWidth( const String& rStyleName );
    void            InvalidateTextWidth( SCTAB nTab );
    void            InvalidateTextWidth( const ScAddress* pAdrFrom, const ScAddress* pAdrTo, BOOL bNumFormatChanged );

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

    void            SetRangeOverflowType(sal_uInt32 nType)  { nRangeOverflowType = nType; }
    sal_Bool        HasRangeOverflow() const                { return nRangeOverflowType != 0; }
    sal_uInt32      GetRangeOverflowType() const            { return nRangeOverflowType; }

    // fuer Broadcasting/Listening
    void            SetNoSetDirty( BOOL bVal ) { bNoSetDirty = bVal; }
    BOOL            GetNoSetDirty() const { return bNoSetDirty; }
    void            SetInsertingFromOtherDoc( BOOL bVal ) { bInsertingFromOtherDoc = bVal; }
    BOOL            IsInsertingFromOtherDoc() const { return bInsertingFromOtherDoc; }
    void            SetImportingXML( BOOL bVal );
    BOOL            IsImportingXML() const { return bImportingXML; }
    void            SetXMLFromWrapper( BOOL bVal );
    BOOL            IsXMLFromWrapper() const { return bXMLFromWrapper; }
    void            SetCalcingAfterLoad( BOOL bVal ) { bCalcingAfterLoad = bVal; }
    BOOL            IsCalcingAfterLoad() const { return bCalcingAfterLoad; }
    void            SetNoListening( BOOL bVal ) { bNoListening = bVal; }
    BOOL            GetNoListening() const { return bNoListening; }
    ScBroadcastAreaSlotMachine* GetBASM() const { return pBASM; }

    ScChartListenerCollection* GetChartListenerCollection() const
                        { return pChartListenerCollection; }
    void            SetChartListenerCollection( ScChartListenerCollection*,
                        BOOL bSetChartRangeLists = FALSE );
    void            UpdateChart( const String& rName );
    void            RestoreChartListener( const String& rName );
    void            UpdateChartListenerCollection();
    BOOL            IsChartListenerCollectionNeedsUpdate() const
                        { return bChartListenerCollectionNeedsUpdate; }
    void            SetChartListenerCollectionNeedsUpdate( BOOL bFlg )
                        { bChartListenerCollectionNeedsUpdate = bFlg; }
    void            AddOLEObjectToCollection(const String& rName);

    ScChangeViewSettings* GetChangeViewSettings() const     { return pChangeViewSettings; }
    void                SetChangeViewSettings(const ScChangeViewSettings& rNew);

    vos::ORef<SvxForbiddenCharactersTable> GetForbiddenCharacters();
    void            SetForbiddenCharacters( const vos::ORef<SvxForbiddenCharactersTable> xNew );

    BYTE            GetAsianCompression() const;        // CharacterCompressionType values
    BOOL            IsValidAsianCompression() const;
    void            SetAsianCompression(BYTE nNew);

    BOOL            GetAsianKerning() const;
    BOOL            IsValidAsianKerning() const;
    void            SetAsianKerning(BOOL bNew);

    BYTE            GetEditTextDirection(SCTAB nTab) const; // EEHorizontalTextDirection values

    ScLkUpdMode     GetLinkMode() const             { return eLinkMode ;}
    void            SetLinkMode( ScLkUpdMode nSet ) {   eLinkMode  = nSet;}


private:
    void                SetAutoFilterFlags();
    void                FindMaxRotCol( SCTAB nTab, RowInfo* pRowInfo, SCSIZE nArrCount,
                                        SCCOL nX1, SCCOL nX2 ) const;

    USHORT              RowDifferences( SCROW nThisRow, SCTAB nThisTab,
                                        ScDocument& rOtherDoc,
                                        SCROW nOtherRow, SCTAB nOtherTab,
                                        SCCOL nMaxCol, SCCOLROW* pOtherCols );
    USHORT              ColDifferences( SCCOL nThisCol, SCTAB nThisTab,
                                        ScDocument& rOtherDoc,
                                        SCCOL nOtherCol, SCTAB nOtherTab,
                                        SCROW nMaxRow, SCCOLROW* pOtherRows );
    void                FindOrder( SCCOLROW* pOtherRows, SCCOLROW nThisEndRow, SCCOLROW nOtherEndRow,
                                        BOOL bColumns,
                                        ScDocument& rOtherDoc, SCTAB nThisTab, SCTAB nOtherTab,
                                        SCCOLROW nEndCol, SCCOLROW* pTranslate,
                                        ScProgress* pProgress, ULONG nProAdd );
    BOOL                OnlineSpellInRange( const ScRange& rSpellRange, ScAddress& rSpellPos,
                                        USHORT nMaxTest );

    DECL_LINK( TrackTimeHdl, Timer* );

    static ScRecursionHelper*   CreateRecursionHelperInstance();

public:
    void                StartListeningArea( const ScRange& rRange,
                                            SvtListener* pListener );
    void                EndListeningArea( const ScRange& rRange,
                                            SvtListener* pListener );
                        /** Broadcast wrapper, calls
                            rHint.GetCell()->Broadcast() and AreaBroadcast()
                            and TrackFormulas() and conditional format list
                            SourceChanged().
                            Preferred.
                         */
    void                Broadcast( const ScHint& rHint );
                        /// deprecated
    void                Broadcast( ULONG nHint, const ScAddress& rAddr,
                                    ScBaseCell* pCell );
                        /// only area, no cell broadcast
    void                AreaBroadcast( const ScHint& rHint );
                        /// only areas in range, no cell broadcasts
    void                AreaBroadcastInRange( const ScRange& rRange,
                                              const ScHint& rHint );
    void                DelBroadcastAreasInRange( const ScRange& rRange );
    void                UpdateBroadcastAreas( UpdateRefMode eUpdateRefMode,
                                            const ScRange& rRange,
                                            SCsCOL nDx, SCsROW nDy, SCsTAB nDz );


    void                StartListeningCell( const ScAddress& rAddress,
                                            SvtListener* pListener );
    void                EndListeningCell( const ScAddress& rAddress,
                                            SvtListener* pListener );
    void                PutInFormulaTree( ScFormulaCell* pCell );
    void                RemoveFromFormulaTree( ScFormulaCell* pCell );
    void                CalcFormulaTree( BOOL bOnlyForced = FALSE,
                                        BOOL bNoProgressBar = FALSE );
    void                ClearFormulaTree();
    void                AppendToFormulaTrack( ScFormulaCell* pCell );
    void                RemoveFromFormulaTrack( ScFormulaCell* pCell );
    void                TrackFormulas( ULONG nHintId = SC_HINT_DATACHANGED );
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
    BOOL                IsInInterpreterTableOp() const { return nInterpreterTableOpLevel != 0; }
    USHORT              GetInterpreterTableOpLevel() { return nInterpreterTableOpLevel; }
    void                IncInterpreterTableOpLevel()
                            {
                                if ( nInterpreterTableOpLevel < USHRT_MAX )
                                    nInterpreterTableOpLevel++;
                            }
    void                DecInterpreterTableOpLevel()
                            {
                                if ( nInterpreterTableOpLevel )
                                    nInterpreterTableOpLevel--;
                            }
                        // add a formula to be remembered for TableOp broadcasts
    void                AddTableOpFormulaCell( ScFormulaCell* );
    void                InvalidateLastTableOpParams() { aLastTableOpParams.bValid = FALSE; }
    ScRecursionHelper&  GetRecursionHelper()
                            {
                                if (!pRecursionHelper)
                                    pRecursionHelper = CreateRecursionHelperInstance();
                                return *pRecursionHelper;
                            }
    BOOL                IsInDtorClear() const { return bInDtorClear; }
    void                SetExpandRefs( BOOL bVal ) { bExpandRefs = bVal; }
    BOOL                IsExpandRefs() { return bExpandRefs; }

    void                IncSizeRecalcLevel( SCTAB nTab );
    void                DecSizeRecalcLevel( SCTAB nTab );

    ULONG               GetXMLImportedFormulaCount() const { return nXMLImportedFormulaCount; }
    void                IncXMLImportedFormulaCount( ULONG nVal )
                            {
                                if ( nXMLImportedFormulaCount + nVal > nXMLImportedFormulaCount )
                                    nXMLImportedFormulaCount += nVal;
                            }
    void                DecXMLImportedFormulaCount( ULONG nVal )
                            {
                                if ( nVal <= nXMLImportedFormulaCount )
                                    nXMLImportedFormulaCount -= nVal;
                                else
                                    nXMLImportedFormulaCount = 0;
                            }

    void                StartTrackTimer();

    void            CompileDBFormula();
    void            CompileDBFormula( BOOL bCreateFormulaString );
    void            CompileNameFormula( BOOL bCreateFormulaString );
    void            CompileColRowNameFormula();

    /** Maximum string length of a column, e.g. for dBase export.
        @return String length in octets (!) of the destination encoding. In
                case of non-octet encodings (e.g. UCS2) the length in code
                points times sizeof(sal_Unicode) is returned. */
    sal_Int32       GetMaxStringLen( SCTAB nTab, SCCOL nCol,
                                     SCROW nRowStart, SCROW nRowEnd,
                                     CharSet eCharSet ) const;
    /** Maximum string length of numerical cells of a column, e.g. for dBase export.
        @return String length in characters (!) including the decimal
                separator, and the decimal precision needed. */
    xub_StrLen      GetMaxNumberStringLen( USHORT& nPrecision,
                                    SCTAB nTab, SCCOL nCol,
                                    SCROW nRowStart, SCROW nRowEnd ) const;

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
    void            AddUnoListenerCall( const ::com::sun::star::uno::Reference<
                                            ::com::sun::star::util::XModifyListener >& rListener,
                                        const ::com::sun::star::lang::EventObject& rEvent );

    void            SetInLinkUpdate(BOOL bSet);             // TableLink or AreaLink
    BOOL            IsInLinkUpdate() const;                 // including DdeLink

    SfxItemPool*        GetEditPool() const;
    SfxItemPool*        GetEnginePool() const;
    ScFieldEditEngine&  GetEditEngine();
    ScNoteEditEngine&   GetNoteEngine();
    SfxItemPool&            GetNoteItemPool();

    void            AddToImpExpLog( const ScImpExpLogMsg& rMsg );
    void            AddToImpExpLog( ScImpExpLogMsg* pMsg );

    ScRefreshTimerControl*  GetRefreshTimerControl() const
        { return pRefreshTimerControl; }
    ScRefreshTimerControl * const * GetRefreshTimerControlAddress() const
        { return &pRefreshTimerControl; }

                    /// if symbol string cells of old binary file format are in list
    BOOL            SymbolStringCellsPending() const;
                    /// get list of ScSymbolStringCellEntry, create if necessary
    List&           GetLoadedSymbolStringCellsList();

    void            SetPastingDrawFromOtherDoc( BOOL bVal )
                        { bPastingDrawFromOtherDoc = bVal; }
    BOOL            PastingDrawFromOtherDoc() const
                        { return bPastingDrawFromOtherDoc; }

                    /// an ID unique to each document instance
    sal_uInt32      GetDocumentID() const;

    void            InvalidateStyleSheetUsage()
                        { bStyleSheetUsageInvalid = TRUE; }
    void GetSortParam( ScSortParam& rParam, SCTAB nTab );
    void SetSortParam( ScSortParam& rParam, SCTAB nTab );
private: // CLOOK-Impl-Methoden
    void    ImplLoadDocOptions( SvStream& rStream );
    void    ImplLoadViewOptions( SvStream& rStream );
    void    ImplSaveDocOptions( SvStream& rStream ) const;
    void    ImplSaveViewOptions( SvStream& rStream ) const;
    void    ImplCreateOptions(); // bei Gelegenheit auf on-demand umstellen?
    void    ImplDeleteOptions();

    void    DeleteDrawLayer();
    void    DeleteColorTable();
    BOOL    DrawGetPrintArea( ScRange& rRange, BOOL bSetHor, BOOL bSetVer ) const;
    void    DrawMovePage( USHORT nOldPos, USHORT nNewPos );
    void    DrawCopyPage( USHORT nOldPos, USHORT nNewPos );

    void    UpdateDrawPrinter();
    void    UpdateDrawLanguages();
    void    UpdateDrawDefaults();
    void    InitClipPtrs( ScDocument* pSourceDoc );

    void    LoadDdeLinks(SvStream& rStream);
    void    SaveDdeLinks(SvStream& rStream) const;
    void    LoadAreaLinks(SvStream& rStream);
    void    SaveAreaLinks(SvStream& rStream) const;

    void    DeleteAreaLinksOnTab( SCTAB nTab );
    void    UpdateRefAreaLinks( UpdateRefMode eUpdateRefMode,
                             const ScRange& r, SCsCOL nDx, SCsROW nDy, SCsTAB nDz );

    BOOL    HasPartOfMerged( const ScRange& rRange );

    std::map< SCTAB, ScSortParam > mSheetSortParams;

};
inline void ScDocument::GetSortParam( ScSortParam& rParam, SCTAB nTab )
{
    rParam = mSheetSortParams[ nTab ];
}

inline void ScDocument::SetSortParam( ScSortParam& rParam, SCTAB nTab )
{
    mSheetSortParams[ nTab ] = rParam;
}


inline ULONG ScDocument::FastGetRowHeight( SCROW nStartRow, SCROW nEndRow,
        SCTAB nTab ) const
{
    return pTab[nTab]->pRowFlags->SumCoupledArrayForCondition( nStartRow,
            nEndRow, CR_HIDDEN, 0, *(pTab[nTab]->pRowHeight));
}

inline ULONG ScDocument::FastGetScaledRowHeight( SCROW nStartRow, SCROW nEndRow,
        SCTAB nTab, double fScale ) const
{
    return pTab[nTab]->pRowFlags->SumScaledCoupledArrayForCondition( nStartRow,
            nEndRow, CR_HIDDEN, 0, *(pTab[nTab]->pRowHeight), fScale);
}

inline USHORT ScDocument::FastGetRowHeight( SCROW nRow, SCTAB nTab ) const
{
    return ( pTab[nTab]->pRowFlags->GetValue(nRow) & CR_HIDDEN ) ? 0 :
        pTab[nTab]->pRowHeight->GetValue(nRow);
}

inline SCROW ScDocument::FastGetRowForHeight( SCTAB nTab, ULONG nHeight ) const
{
    ScCoupledCompressedArrayIterator< SCROW, BYTE, USHORT> aIter(
            *(pTab[nTab]->pRowFlags), 0, MAXROW, CR_HIDDEN, 0,
            *(pTab[nTab]->pRowHeight));
    ULONG nSum = 0;
    for ( ; aIter; aIter.NextRange() )
    {
        ULONG nNew = *aIter * (aIter.GetRangeEnd() - aIter.GetRangeStart() + 1);
        if (nSum + nNew > nHeight)
        {
            for ( ; aIter && nSum <= nHeight; ++aIter )
            {
                nSum += *aIter;
            }
            return aIter.GetPos();
        }
        nSum += nNew;
    }
    return aIter.GetPos();
}

inline SCROW ScDocument::FastGetFirstNonHiddenRow( SCROW nStartRow, SCTAB nTab) const
{
    return pTab[nTab]->pRowFlags->GetFirstForCondition( nStartRow, MAXROW,
            CR_HIDDEN, 0);
}

inline USHORT ScDocument::FastGetOriginalRowHeight( SCROW nRow, SCTAB nTab ) const
{
    return pTab[nTab]->pRowHeight->GetValue(nRow);
}

#endif


