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

#ifndef SC_DOCUMENT_HXX
#define SC_DOCUMENT_HXX


#include <vcl/prntypes.hxx>
#include <vcl/timer.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <vos/ref.hxx>
#include "scdllapi.h"
#include "table.hxx"        // FastGetRowHeight (inline)
#include "rangelst.hxx"
#include "brdcst.hxx"
#include "tabopparams.hxx"
#include "formula/grammar.hxx"
#include <com/sun/star/chart2/XChartDocument.hpp>
#include "scdllapi.h"

#include <memory>
#include <map>
#include <set>

// Wang Xu Ming -- 2009-8-17
// DataPilot Migration - Cache&&Performance
#include <list>
#include "dpobject.hxx"
#include "dptabdat.hxx"
// End Comments

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
namespace sfx2 {
    class LinkManager;
    }
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
class ScDocProtection;
class ScDocumentPool;
class ScDrawLayer;
class ScExtDocOptions;
class ScExternalRefManager;
class ScFormulaCell;
class ScMarkData;
class ScOutlineTable;
class ScPatternAttr;
class ScPrintRangeSaver;
class ScRangeData;
class ScRangeName;
class ScStyleSheet;
class ScStyleSheetPool;
class ScTable;
class ScTableProtection;
class ScTokenArray;
class ScValidationData;
class ScValidationDataList;
class ScViewOptions;
class ScStrCollection;
class TypedScStrCollection;
class ScChangeTrack;
class ScFieldEditEngine;
class ScNoteEditEngine;
struct ScConsolidateParam;
class ScDPObject;
class ScDPCollection;
class ScMatrix;
class ScScriptTypeData;
class ScPoolHelper;
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
class SfxUndoManager;
class ScFormulaParserPool;
struct ScClipParam;
struct ScClipRangeNameData;
class ScRowBreakIterator;

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
    namespace script { namespace vba {
        class XVBAEventProcessor;
    } }
    namespace sheet {
        struct TablePageBreakData;
    }
} } }

#include <svl/zforlist.hxx>
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
    sal_uLong   nCellCount;
    sal_uInt16  nPageCount;
};

// The constant parameters to CopyBlockFromClip
struct ScCopyBlockFromClipParams
{
    ScDocument* pRefUndoDoc;
    ScDocument* pClipDoc;
    sal_uInt16      nInsFlag;
    SCTAB       nTabStart;
    SCTAB       nTabEnd;
    sal_Bool        bAsLink;
    sal_Bool        bSkipAttrForEmpty;
};


// for loading of binary file format symbol string cells which need font conversion
struct ScSymbolStringCellEntry
{
    ScStringCell*   pCell;
    SCROW           nRow;
};


// -----------------------------------------------------------------------

// DDE link modes
const sal_uInt8 SC_DDE_DEFAULT       = 0;
const sal_uInt8 SC_DDE_ENGLISH       = 1;
const sal_uInt8 SC_DDE_TEXT          = 2;
const sal_uInt8 SC_DDE_IGNOREMODE    = 255;       /// For usage in FindDdeLink() only!


// -----------------------------------------------------------------------

class ScDocument
{
friend class ScDocumentIterator;
friend class ScValueIterator;
friend class ScDBQueryDataIterator;
friend class ScCellIterator;
friend class ScQueryCellIterator;
friend class ScHorizontalCellIterator;
friend class ScHorizontalAttrIterator;
friend class ScDocAttrIterator;
friend class ScAttrRectIterator;
friend class ScDocShell;

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceManager;

    vos::ORef<ScPoolHelper> xPoolHelper;

    SfxUndoManager*     mpUndoManager;
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
    ScDPCollection*     pDPCollection;
    // Wang Xu Ming -- 2009-8-17
    // DataPilot Migration - Cache&&Performance
    std::list<ScDPObject>        m_listDPObjectsInClip;
    std::list<ScDPTableDataCache*>   m_listDPObjectsCaches;
    // End Comments
    ScChartCollection*  pChartCollection;
    std::auto_ptr< ScTemporaryChartLock > apTemporaryChartLock;
    ScPatternAttr*      pSelectionAttr;                 // Attribute eines Blocks
    mutable sfx2::LinkManager*      pLinkManager;
    ScFormulaCell*      pFormulaTree;                   // Berechnungsbaum Start
    ScFormulaCell*      pEOFormulaTree;                 // Berechnungsbaum Ende, letzte Zelle
    ScFormulaCell*      pFormulaTrack;                  // BroadcastTrack Start
    ScFormulaCell*      pEOFormulaTrack;                // BrodcastTrack Ende, letzte Zelle
    ScBroadcastAreaSlotMachine* pBASM;                  // BroadcastAreas
    ScChartListenerCollection* pChartListenerCollection;
    ScStrCollection*        pOtherObjects;                  // non-chart OLE objects
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

    ::std::auto_ptr<ScDocProtection> pDocProtection;
    ::std::auto_ptr<ScClipParam>     mpClipParam;

    ::std::auto_ptr<ScExternalRefManager> pExternalRefMgr;

    // mutable for lazy construction
    mutable ::std::auto_ptr< ScFormulaParserPool >
                        mxFormulaParserPool;            /// Pool for all external formula parsers used by this document.

    String              aDocName;                       // opt: Dokumentname
    String              aDocCodeName;                       // opt: Dokumentname
    ScRangePairListRef  xColNameRanges;
    ScRangePairListRef  xRowNameRanges;

    ScViewOptions*      pViewOptions;                   // View-Optionen
    ScDocOptions*       pDocOptions;                    // Dokument-Optionen
    ScExtDocOptions*    pExtDocOptions;                 // fuer Import etc.
    ScConsolidateParam* pConsolidateDlgData;

    ScRecursionHelper*  pRecursionHelper;               // information for recursive and iterative cell formulas

    ScAutoNameCache*    pAutoNameCache;                 // for automatic name lookup during CompileXML

    ScLookupCacheMapImpl* pLookupCacheMapImpl;          // cache for lookups like VLOOKUP and MATCH

    sal_Int64           nUnoObjectId;                   // counted up for UNO objects

    sal_uInt32          nRangeOverflowType;             // used in (xml) loading for overflow warnings

    ScRange             aEmbedRange;
    ScAddress           aCurTextWidthCalcPos;
    ScAddress           aOnlineSpellPos;                // within whole document
    ScRange             aVisSpellRange;
    ScAddress           aVisSpellPos;                   // within aVisSpellRange (see nVisSpellState)

    Timer               aTrackTimer;

    com::sun::star::uno::Reference< com::sun::star::script::vba::XVBAEventProcessor >
                        mxVbaEvents;

public:
    ScTabOpList         aTableOpList;                   // list of ScInterpreterTableOpParams currently in use
    ScInterpreterTableOpParams  aLastTableOpParams;     // remember last params
private:

    LanguageType        eLanguage;                      // default language
    LanguageType        eCjkLanguage;                   // default language for asian text
    LanguageType        eCtlLanguage;                   // default language for complex text
    CharSet             eSrcSet;                        // Einlesen: Quell-Zeichensatz

    /** The compiler grammar used in document storage. GRAM_PODF for ODF 1.1
        documents, GRAM_ODFF for ODF 1.2 documents. */
    formula::FormulaGrammar::Grammar  eStorageGrammar;

    /** The compiler grammar used in ODF import after brackets had been
        stripped (which they shouldn't, but until that's fixed) by the XML
        importer. */
    formula::FormulaGrammar::Grammar  eXmlImportGrammar;

    sal_uLong               nFormulaCodeInTree;             // FormelRPN im Formelbaum
    sal_uLong               nXMLImportedFormulaCount;        // progress count during XML import
    sal_uInt16              nInterpretLevel;                // >0 wenn im Interpreter
    sal_uInt16              nMacroInterpretLevel;           // >0 wenn Macro im Interpreter
    sal_uInt16              nInterpreterTableOpLevel;       // >0 if in Interpreter TableOp
    SCTAB               nMaxTableNumber;
    sal_uInt16              nSrcVer;                        // Dateiversion (Laden/Speichern)
    SCROW               nSrcMaxRow;                     // Zeilenzahl zum Laden/Speichern
    sal_uInt16              nFormulaTrackCount;
    sal_uInt16              nHardRecalcState;               // 0: soft, 1: hard-warn, 2: hard
    SCTAB               nVisibleTab;                    // fuer OLE etc.

    ScLkUpdMode         eLinkMode;

    sal_Bool                bAutoCalc;                      // Automatisch Berechnen
    sal_Bool                bAutoCalcShellDisabled;         // in/von/fuer ScDocShell disabled
    // ob noch ForcedFormulas berechnet werden muessen,
    // im Zusammenspiel mit ScDocShell SetDocumentModified,
    // AutoCalcShellDisabled und TrackFormulas
    sal_Bool                bForcedFormulaPending;
    sal_Bool                bCalculatingFormulaTree;
    sal_Bool                bIsClip;
    sal_Bool                bIsUndo;
    sal_Bool                bIsVisible;                     // set from view ctor

    sal_Bool                bIsEmbedded;                    // Embedded-Bereich anzeigen/anpassen ?

    // kein SetDirty bei ScFormulaCell::CompileTokenArray sondern am Ende
    // von ScDocument::CompileAll[WithFormats], CopyScenario, CopyBlockFromClip
    sal_Bool                bNoSetDirty;
    // kein Broadcast, keine Listener aufbauen waehrend aus einem anderen
    // Doc (per Filter o.ae.) inserted wird, erst bei CompileAll / CalcAfterLoad
    sal_Bool                bInsertingFromOtherDoc;
    bool                bLoadingMedium;
    bool                bImportingXML;      // special handling of formula text
    sal_Bool                bXMLFromWrapper;    // distinguish ScXMLImportWrapper from external component
    sal_Bool                bCalcingAfterLoad;              // in CalcAfterLoad TRUE
    // wenn temporaer keine Listener auf/abgebaut werden sollen
    sal_Bool                bNoListening;
    sal_Bool                bIdleDisabled;
    sal_Bool                bInLinkUpdate;                  // TableLink or AreaLink
    sal_Bool                bChartListenerCollectionNeedsUpdate;
    // ob RC_FORCED Formelzellen im Dokument sind/waren (einmal an immer an)
    sal_Bool                bHasForcedFormulas;
    // ob das Doc gerade zerstoert wird (kein Notify-Tracking etc. mehr)
    sal_Bool                bInDtorClear;
    // ob bei Spalte/Zeile einfuegen am Rand einer Referenz die Referenz
    // erweitert wird, wird in jedem UpdateReference aus InputOptions geholt,
    // gesetzt und am Ende von UpdateReference zurueckgesetzt
    sal_Bool                bExpandRefs;
    // fuer Detektiv-Update, wird bei jeder Aenderung an Formeln gesetzt
    sal_Bool                bDetectiveDirty;

    sal_uInt8               nMacroCallMode;     // Makros per Warnung-Dialog disabled?
    sal_Bool                bHasMacroFunc;      // valid only after loading

    sal_uInt8               nVisSpellState;

    sal_uInt8               nAsianCompression;
    sal_uInt8               nAsianKerning;
    sal_Bool                bSetDrawDefaults;

    sal_Bool                bPastingDrawFromOtherDoc;

    sal_uInt8                nInDdeLinkUpdate;   // originating DDE links (stacked bool)

    sal_Bool                bInUnoBroadcast;
    sal_Bool                bInUnoListenerCall;
    formula::FormulaGrammar::Grammar  eGrammar;

    mutable sal_Bool        bStyleSheetUsageInvalid;

    bool                mbUndoEnabled;
    bool                mbAdjustHeightEnabled;
    bool                mbExecuteLinkEnabled;
    bool                mbChangeReadOnlyEnabled;    // allow changes in read-only document (for API import filters)
    bool                mbStreamValidLocked;

    sal_Int16           mnNamedRangesLockCount;

public:
    SC_DLLPUBLIC sal_uLong          GetCellCount() const;       // alle Zellen
    SCSIZE          GetCellCount(SCTAB nTab, SCCOL nCol) const;
    sal_uLong           GetWeightedCount() const;   // Formeln und Edit staerker gewichtet
    sal_uLong           GetCodeCount() const;       // RPN-Code in Formeln
    DECL_LINK( GetUserDefinedColor, sal_uInt16 * );
                                                                // Numberformatter

public:
    SC_DLLPUBLIC                ScDocument( ScDocumentMode eMode = SCDOCMODE_DOCUMENT,
                                SfxObjectShell* pDocShell = NULL );
    SC_DLLPUBLIC                ~ScDocument();

    inline ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                    GetServiceManager() const { return xServiceManager; }

    SC_DLLPUBLIC const String&  GetName() const { return aDocName; }
    void            SetName( const String& r ) { aDocName = r; }
    const String&   GetCodeName() const { return aDocCodeName; }
    void            SetCodeName( const String& r ) { aDocCodeName = r; }

    void            GetDocStat( ScDocStat& rDocStat );

    SC_DLLPUBLIC void           InitDrawLayer( SfxObjectShell* pDocShell = NULL );
    XColorTable*    GetColorTable();

    SC_DLLPUBLIC sfx2::LinkManager*     GetLinkManager() const;

    SC_DLLPUBLIC const ScDocOptions&        GetDocOptions() const;
    SC_DLLPUBLIC void                   SetDocOptions( const ScDocOptions& rOpt );
    SC_DLLPUBLIC const ScViewOptions&   GetViewOptions() const;
    SC_DLLPUBLIC void                   SetViewOptions( const ScViewOptions& rOpt );
    void                    SetPrintOptions();

    ScExtDocOptions*        GetExtDocOptions()  { return pExtDocOptions; }
    SC_DLLPUBLIC void                   SetExtDocOptions( ScExtDocOptions* pNewOptions );

    void                    GetLanguage( LanguageType& rLatin, LanguageType& rCjk, LanguageType& rCtl ) const;
    void                    SetLanguage( LanguageType eLatin, LanguageType eCjk, LanguageType eCtl );

    void                    SetDrawDefaults();

    void                        SetConsolidateDlgData( const ScConsolidateParam* pData );
    const ScConsolidateParam*   GetConsolidateDlgData() const { return pConsolidateDlgData; }

    void            Clear( sal_Bool bFromDestructor = sal_False );

    ScFieldEditEngine*  CreateFieldEditEngine();
    void                DisposeFieldEditEngine(ScFieldEditEngine*& rpEditEngine);

    SC_DLLPUBLIC ScRangeName*   GetRangeName();
    void            SetRangeName( ScRangeName* pNewRangeName );
    SCTAB           GetMaxTableNumber() { return nMaxTableNumber; }
    void            SetMaxTableNumber(SCTAB nNumber) { nMaxTableNumber = nNumber; }

    ScRangePairList*    GetColNameRanges() { return &xColNameRanges; }
    ScRangePairList*    GetRowNameRanges() { return &xRowNameRanges; }
    ScRangePairListRef& GetColNameRangesRef() { return xColNameRanges; }
    ScRangePairListRef& GetRowNameRangesRef() { return xRowNameRanges; }

    SC_DLLPUBLIC ScDBCollection*    GetDBCollection() const;
    void            SetDBCollection( ScDBCollection* pNewDBCollection,
                                        sal_Bool bRemoveAutoFilter = sal_False );
    ScDBData*       GetDBAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab,
                                        sal_Bool bStartOnly = sal_False) const;
    ScDBData*       GetDBAtArea(SCTAB nTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2) const;

//UNUSED2008-05  ScRangeData*   GetRangeAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab,
//UNUSED2008-05                                      sal_Bool bStartOnly = sal_False) const;
    SC_DLLPUBLIC ScRangeData*   GetRangeAtBlock( const ScRange& rBlock, String* pName=NULL ) const;

    SC_DLLPUBLIC ScDPCollection*        GetDPCollection();
    ScDPObject*         GetDPAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab) const;
    ScDPObject*         GetDPAtBlock( const ScRange& rBlock ) const;
    // Wang Xu Ming -- 2009-8-17
    // DataPilot Migration - Cache&&Performance
    SC_DLLPUBLIC ScDPTableDataCache*    GetDPObjectCache( long nID );
    SC_DLLPUBLIC ScDPTableDataCache*    GetUsedDPObjectCache ( ScRange rRange );
    SC_DLLPUBLIC long                                 AddDPObjectCache( ScDPTableDataCache* pData );
    SC_DLLPUBLIC void                                 RemoveDPObjectCache( long nID );
    SC_DLLPUBLIC void                                 RemoveUnusedDPObjectCaches();
    SC_DLLPUBLIC void                                 GetUsedDPObjectCache( std::list<ScDPTableDataCache*>& usedlist );
    SC_DLLPUBLIC long                                 GetNewDPObjectCacheId ();
    // End Comments

    SC_DLLPUBLIC ScChartCollection* GetChartCollection() const;

    void                StopTemporaryChartLock();

    void            EnsureGraphicNames();

    SdrObject*      GetObjectAtPoint( SCTAB nTab, const Point& rPos );
    sal_Bool            HasChartAtPoint( SCTAB nTab, const Point& rPos, String* pName = NULL );

    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartDocument > GetChartByName( const String& rChartName );
    SC_DLLPUBLIC void            GetChartRanges( const String& rChartName, ::std::vector< ScRangeList >& rRanges, ScDocument* pSheetNameDoc );
    void            SetChartRanges( const String& rChartName, const ::std::vector< ScRangeList >& rRanges );

    void            UpdateChartArea( const String& rChartName, const ScRange& rNewArea,
                                        sal_Bool bColHeaders, sal_Bool bRowHeaders, sal_Bool bAdd );
    void            UpdateChartArea( const String& rChartName,
                                    const ScRangeListRef& rNewList,
                                    sal_Bool bColHeaders, sal_Bool bRowHeaders, sal_Bool bAdd );
    void            GetOldChartParameters( const String& rName,
                                    ScRangeList& rRanges, sal_Bool& rColHeaders, sal_Bool& rRowHeaders );
    ::com::sun::star::uno::Reference<
            ::com::sun::star::embed::XEmbeddedObject >
                    FindOleObjectByName( const String& rName );

    SC_DLLPUBLIC void           MakeTable( SCTAB nTab,bool _bNeedsNameCheck = true );

    SCTAB           GetVisibleTab() const       { return nVisibleTab; }
    SC_DLLPUBLIC void           SetVisibleTab(SCTAB nTab)   { nVisibleTab = nTab; }

    SC_DLLPUBLIC sal_Bool           HasTable( SCTAB nTab ) const;
    SC_DLLPUBLIC sal_Bool           GetName( SCTAB nTab, String& rName ) const;
    SC_DLLPUBLIC sal_Bool           GetCodeName( SCTAB nTab, String& rName ) const;
    SC_DLLPUBLIC sal_Bool           SetCodeName( SCTAB nTab, const String& rName );
    SC_DLLPUBLIC sal_Bool           GetTable( const String& rName, SCTAB& rTab ) const;
    SC_DLLPUBLIC inline SCTAB   GetTableCount() const { return nMaxTableNumber; }
    SvNumberFormatterIndexTable* GetFormatExchangeList() const { return pFormatExchangeList; }

    SC_DLLPUBLIC ScDocProtection* GetDocProtection() const;
    SC_DLLPUBLIC void            SetDocProtection(const ScDocProtection* pProtect);
    SC_DLLPUBLIC sal_Bool           IsDocProtected() const;
    sal_Bool            IsDocEditable() const;
    SC_DLLPUBLIC sal_Bool           IsTabProtected( SCTAB nTab ) const;
    SC_DLLPUBLIC    ScTableProtection* GetTabProtection( SCTAB nTab ) const;
    SC_DLLPUBLIC void SetTabProtection(SCTAB nTab, const ScTableProtection* pProtect);
    void            CopyTabProtection(SCTAB nTabSrc, SCTAB nTabDest);

    void            LockTable(SCTAB nTab);
    void            UnlockTable(SCTAB nTab);

    sal_Bool            IsBlockEditable( SCTAB nTab, SCCOL nStartCol, SCROW nStartRow,
                                        SCCOL nEndCol, SCROW nEndRow,
                                        sal_Bool* pOnlyNotBecauseOfMatrix = NULL ) const;
    sal_Bool            IsSelectionEditable( const ScMarkData& rMark,
                                        sal_Bool* pOnlyNotBecauseOfMatrix = NULL ) const;
    sal_Bool            HasSelectedBlockMatrixFragment( SCCOL nStartCol, SCROW nStartRow,
                                            SCCOL nEndCol, SCROW nEndRow,
                                            const ScMarkData& rMark ) const;

    sal_Bool            GetMatrixFormulaRange( const ScAddress& rCellPos, ScRange& rMatrix );

    sal_Bool            IsEmbedded() const;
    void            GetEmbedded( ScRange& rRange ) const;
    void            SetEmbedded( const ScRange& rRange );
    void            ResetEmbedded();
    Rectangle       GetEmbeddedRect() const;                        // 1/100 mm
    void            SetEmbedded( const Rectangle& rRect );          // aus VisArea (1/100 mm)
    void            SnapVisArea( Rectangle& rRect ) const;          // 1/100 mm

    SC_DLLPUBLIC sal_Bool           ValidTabName( const String& rName ) const;
    SC_DLLPUBLIC sal_Bool           ValidNewTabName( const String& rName ) const;
    SC_DLLPUBLIC void           CreateValidTabName(String& rName) const;
    SC_DLLPUBLIC sal_Bool           InsertTab( SCTAB nPos, const String& rName,
                                sal_Bool bExternalDocument = sal_False );
    SC_DLLPUBLIC sal_Bool            DeleteTab( SCTAB nTab, ScDocument* pRefUndoDoc = NULL );
    SC_DLLPUBLIC sal_Bool           RenameTab( SCTAB nTab, const String& rName,
                                sal_Bool bUpdateRef = sal_True,
                                sal_Bool bExternalDocument = sal_False );
    sal_Bool            MoveTab( SCTAB nOldPos, SCTAB nNewPos );
    sal_Bool            CopyTab( SCTAB nOldPos, SCTAB nNewPos,
                                const ScMarkData* pOnlyMarked = NULL );
    SC_DLLPUBLIC sal_uLong          TransferTab(ScDocument* pSrcDoc, SCTAB nSrcPos, SCTAB nDestPos,
                                    sal_Bool bInsertNew = sal_True,
                                    sal_Bool bResultsOnly = sal_False );
    SC_DLLPUBLIC void           TransferDrawPage(ScDocument* pSrcDoc, SCTAB nSrcPos, SCTAB nDestPos);
    SC_DLLPUBLIC void           SetVisible( SCTAB nTab, sal_Bool bVisible );
    SC_DLLPUBLIC sal_Bool           IsVisible( SCTAB nTab ) const;
    sal_Bool            IsStreamValid( SCTAB nTab ) const;
    void            SetStreamValid( SCTAB nTab, sal_Bool bSet, sal_Bool bIgnoreLock = sal_False );
    void            LockStreamValid( bool bLock );
    bool            IsStreamValidLocked() const                         { return mbStreamValidLocked; }
    sal_Bool            IsPendingRowHeights( SCTAB nTab ) const;
    void            SetPendingRowHeights( SCTAB nTab, sal_Bool bSet );
    SC_DLLPUBLIC void           SetLayoutRTL( SCTAB nTab, sal_Bool bRTL );
    SC_DLLPUBLIC sal_Bool           IsLayoutRTL( SCTAB nTab ) const;
    sal_Bool            IsNegativePage( SCTAB nTab ) const;
    SC_DLLPUBLIC void           SetScenario( SCTAB nTab, sal_Bool bFlag );
    SC_DLLPUBLIC sal_Bool           IsScenario( SCTAB nTab ) const;
    SC_DLLPUBLIC void           GetScenarioData( SCTAB nTab, String& rComment,
                                        Color& rColor, sal_uInt16& rFlags ) const;
    SC_DLLPUBLIC void           SetScenarioData( SCTAB nTab, const String& rComment,
                                        const Color& rColor, sal_uInt16 nFlags );
    SC_DLLPUBLIC Color GetTabBgColor( SCTAB nTab ) const;
    SC_DLLPUBLIC void SetTabBgColor( SCTAB nTab, const Color& rColor );
    SC_DLLPUBLIC bool IsDefaultTabBgColor( SCTAB nTab ) const;
    void            GetScenarioFlags( SCTAB nTab, sal_uInt16& rFlags ) const;
    SC_DLLPUBLIC sal_Bool           IsActiveScenario( SCTAB nTab ) const;
    SC_DLLPUBLIC void           SetActiveScenario( SCTAB nTab, sal_Bool bActive );      // nur fuer Undo etc.
    SC_DLLPUBLIC formula::FormulaGrammar::AddressConvention GetAddressConvention() const;
    SC_DLLPUBLIC formula::FormulaGrammar::Grammar GetGrammar() const;
    void            SetGrammar( formula::FormulaGrammar::Grammar eGram );
    SC_DLLPUBLIC sal_uInt8          GetLinkMode( SCTAB nTab ) const;
    sal_Bool            IsLinked( SCTAB nTab ) const;
    SC_DLLPUBLIC const String&  GetLinkDoc( SCTAB nTab ) const;
    const String&   GetLinkFlt( SCTAB nTab ) const;
    const String&   GetLinkOpt( SCTAB nTab ) const;
    SC_DLLPUBLIC const String&  GetLinkTab( SCTAB nTab ) const;
    sal_uLong           GetLinkRefreshDelay( SCTAB nTab ) const;
    void            SetLink( SCTAB nTab, sal_uInt8 nMode, const String& rDoc,
                            const String& rFilter, const String& rOptions,
                            const String& rTabName, sal_uLong nRefreshDelay );
    sal_Bool            HasLink( const String& rDoc,
                            const String& rFilter, const String& rOptions ) const;
    SC_DLLPUBLIC sal_Bool           LinkExternalTab( SCTAB& nTab, const String& aDocTab,
                                    const String& aFileName,
                                    const String& aTabName );

    bool            HasExternalRefManager() const { return pExternalRefMgr.get(); }
    SC_DLLPUBLIC ScExternalRefManager* GetExternalRefManager() const;
    bool            IsInExternalReferenceMarking() const;
    void            MarkUsedExternalReferences();
    bool            MarkUsedExternalReferences( ScTokenArray & rArr );

    /** Returns the pool containing external formula parsers. Creates the pool
        on first call. */
    ScFormulaParserPool& GetFormulaParserPool() const;

    sal_Bool            HasDdeLinks() const;
    sal_Bool            HasAreaLinks() const;
    void            UpdateExternalRefLinks();
    void            UpdateDdeLinks();
    void            UpdateAreaLinks();

                    // originating DDE links
    void            IncInDdeLinkUpdate() { if ( nInDdeLinkUpdate < 255 ) ++nInDdeLinkUpdate; }
    void            DecInDdeLinkUpdate() { if ( nInDdeLinkUpdate ) --nInDdeLinkUpdate; }
    sal_Bool            IsInDdeLinkUpdate() const   { return nInDdeLinkUpdate != 0; }

    SC_DLLPUBLIC void           CopyDdeLinks( ScDocument* pDestDoc ) const;
    void            DisconnectDdeLinks();

                    // Fuer StarOne Api:
    sal_uInt16          GetDdeLinkCount() const;
    sal_Bool            UpdateDdeLink( const String& rAppl, const String& rTopic, const String& rItem );

    /** Tries to find a DDE link with the specified connection data.
        @param rnDdePos  (out-param) Returns the index of the DDE link (does not include other links from link manager).
        @return  true = DDE link found, rnDdePos valid. */
    SC_DLLPUBLIC bool            FindDdeLink( const String& rAppl, const String& rTopic, const String& rItem, sal_uInt8 nMode, sal_uInt16& rnDdePos );

    /** Returns the connection data of the specified DDE link.
        @param nDdePos  Index of the DDE link (does not include other links from link manager).
        @param rAppl  (out-param) The application name.
        @param rTopic  (out-param) The DDE topic.
        @param rItem  (out-param) The DDE item.
        @return  true = DDE link found, out-parameters valid. */
    bool            GetDdeLinkData( sal_uInt16 nDdePos, String& rAppl, String& rTopic, String& rItem ) const;
    /** Returns the link mode of the specified DDE link.
        @param nDdePos  Index of the DDE link (does not include other links from link manager).
        @param rnMode  (out-param) The link mode of the specified DDE link.
        @return  true = DDE link found, rnMode valid. */
    bool            GetDdeLinkMode( sal_uInt16 nDdePos, sal_uInt8& rnMode ) const;
    /** Returns the result matrix of the specified DDE link.
        @param nDdePos  Index of the DDE link (does not include other links from link manager).
        @return  The result matrix, if the DDE link has been found, 0 otherwise. */
    SC_DLLPUBLIC const ScMatrix* GetDdeLinkResultMatrix( sal_uInt16 nDdePos ) const;

    /** Tries to find a DDE link or creates a new, if not extant.
        @param pResults  If not 0, sets the matrix as as DDE link result matrix (also for existing links).
        @return  true = DDE link found; false = Unpredictable error occured, no DDE link created. */
    SC_DLLPUBLIC bool            CreateDdeLink( const String& rAppl, const String& rTopic, const String& rItem, sal_uInt8 nMode, ScMatrix* pResults = NULL );
    /** Sets a result matrix for the specified DDE link.
        @param nDdePos  Index of the DDE link (does not include other links from link manager).
        @param pResults  The array containing all results of the DDE link (intrusive-ref-counted, do not delete).
        @return  true = DDE link found and matrix set. */
    bool            SetDdeLinkResultMatrix( sal_uInt16 nDdePos, ScMatrix* pResults );


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

    SC_DLLPUBLIC void           UpdateAllCharts();
    void            UpdateChartRef( UpdateRefMode eUpdateRefMode,
                                    SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                                    SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                                    SCsCOL nDx, SCsROW nDy, SCsTAB nDz );
                    //! setzt nur die neue RangeList, keine ChartListener o.ae.
    void            SetChartRangeList( const String& rChartName,
                        const ScRangeListRef& rNewRangeListRef );

    sal_Bool            HasControl( SCTAB nTab, const Rectangle& rMMRect );
    void            InvalidateControls( Window* pWin, SCTAB nTab, const Rectangle& rMMRect );

    void            StartAnimations( SCTAB nTab, Window* pWin );

    sal_Bool            HasBackgroundDraw( SCTAB nTab, const Rectangle& rMMRect );
    sal_Bool            HasAnyDraw( SCTAB nTab, const Rectangle& rMMRect );

    const ScSheetEvents* GetSheetEvents( SCTAB nTab ) const;
    void            SetSheetEvents( SCTAB nTab, const ScSheetEvents* pNew );
    bool            HasSheetEventScript( SCTAB nTab, sal_Int32 nEvent, bool bWithVbaEvents = false ) const;
    bool            HasAnySheetEventScript( sal_Int32 nEvent, bool bWithVbaEvents = false ) const;  // on any sheet

    bool            HasAnyCalcNotification() const;
    sal_Bool            HasCalcNotification( SCTAB nTab ) const;
    void            SetCalcNotification( SCTAB nTab );
    void            ResetCalcNotifications();

    SC_DLLPUBLIC ScOutlineTable*    GetOutlineTable( SCTAB nTab, sal_Bool bCreate = sal_False );
    sal_Bool            SetOutlineTable( SCTAB nTab, const ScOutlineTable* pNewOutline );

    void            DoAutoOutline( SCCOL nStartCol, SCROW nStartRow,
                                    SCCOL nEndCol, SCROW nEndRow, SCTAB nTab );

    sal_Bool            DoSubTotals( SCTAB nTab, ScSubTotalParam& rParam );
    void            RemoveSubTotals( SCTAB nTab, ScSubTotalParam& rParam );
    sal_Bool            TestRemoveSubTotals( SCTAB nTab, const ScSubTotalParam& rParam );
    sal_Bool            HasSubTotalCells( const ScRange& rRange );

    SC_DLLPUBLIC void           PutCell( const ScAddress&, ScBaseCell* pCell, sal_Bool bForceTab = sal_False );
//UNUSED2009-05 SC_DLLPUBLIC void           PutCell( const ScAddress&, ScBaseCell* pCell,
//UNUSED2009-05                         sal_uLong nFormatIndex, sal_Bool bForceTab = sal_False);
    SC_DLLPUBLIC void           PutCell( SCCOL nCol, SCROW nRow, SCTAB nTab, ScBaseCell* pCell,
                            sal_Bool bForceTab = sal_False );
    SC_DLLPUBLIC void           PutCell(SCCOL nCol, SCROW nRow, SCTAB nTab, ScBaseCell* pCell,
                            sal_uLong nFormatIndex, sal_Bool bForceTab = sal_False);
                    //  return sal_True = Zahlformat gesetzt
    SC_DLLPUBLIC sal_Bool           SetString(
        SCCOL nCol, SCROW nRow, SCTAB nTab, const String& rString,
        SvNumberFormatter* pFormatter = NULL, bool bDetectNumberFormat = true );
    SC_DLLPUBLIC void           SetValue( SCCOL nCol, SCROW nRow, SCTAB nTab, const double& rVal );
    void            SetError( SCCOL nCol, SCROW nRow, SCTAB nTab, const sal_uInt16 nError);

    SC_DLLPUBLIC void           InsertMatrixFormula(SCCOL nCol1, SCROW nRow1,
                                        SCCOL nCol2, SCROW nRow2,
                                        const ScMarkData& rMark,
                                        const String& rFormula,
                                        const ScTokenArray* p = NULL,
                                        const formula::FormulaGrammar::Grammar = formula::FormulaGrammar::GRAM_DEFAULT );
    SC_DLLPUBLIC void           InsertTableOp(const ScTabOpParam& rParam,   // Mehrfachoperation
                                  SCCOL nCol1, SCROW nRow1,
                                  SCCOL nCol2, SCROW nRow2, const ScMarkData& rMark);

    SC_DLLPUBLIC void           GetString( SCCOL nCol, SCROW nRow, SCTAB nTab, String& rString );
    SC_DLLPUBLIC void           GetInputString( SCCOL nCol, SCROW nRow, SCTAB nTab, String& rString );
    SC_DLLPUBLIC double         GetValue( const ScAddress& );
    SC_DLLPUBLIC void           GetValue( SCCOL nCol, SCROW nRow, SCTAB nTab, double& rValue );
    SC_DLLPUBLIC double         RoundValueAsShown( double fVal, sal_uLong nFormat );
    SC_DLLPUBLIC void           GetNumberFormat( SCCOL nCol, SCROW nRow, SCTAB nTab,
                                     sal_uInt32& rFormat );
    SC_DLLPUBLIC sal_uInt32     GetNumberFormat( const ScAddress& ) const;
                    /** If no number format attribute is set and the cell
                        pointer passed is of type formula cell, the calculated
                        number format of the formula cell is returned. pCell
                        may be NULL. */
    SC_DLLPUBLIC void           GetNumberFormatInfo( short& nType, sal_uLong& nIndex,
                        const ScAddress& rPos, const ScBaseCell* pCell ) const;
    void            GetFormula( SCCOL nCol, SCROW nRow, SCTAB nTab, String& rFormula,
                                sal_Bool bAsciiExport = sal_False ) const;
    SC_DLLPUBLIC void           GetCellType( SCCOL nCol, SCROW nRow, SCTAB nTab, CellType& rCellType ) const;
    SC_DLLPUBLIC CellType       GetCellType( const ScAddress& rPos ) const;
    SC_DLLPUBLIC void           GetCell( SCCOL nCol, SCROW nRow, SCTAB nTab, ScBaseCell*& rpCell ) const;
    SC_DLLPUBLIC ScBaseCell*        GetCell( const ScAddress& rPos ) const;

//UNUSED2008-05  void           RefreshNoteFlags();

    SC_DLLPUBLIC sal_Bool           HasData( SCCOL nCol, SCROW nRow, SCTAB nTab );
    SC_DLLPUBLIC sal_Bool           HasStringData( SCCOL nCol, SCROW nRow, SCTAB nTab ) const;
    SC_DLLPUBLIC sal_Bool           HasValueData( SCCOL nCol, SCROW nRow, SCTAB nTab ) const;
    sal_Bool            HasStringCells( const ScRange& rRange ) const;

    /** Returns true, if there is any data to create a selection list for rPos. */
    sal_Bool            HasSelectionData( SCCOL nCol, SCROW nRow, SCTAB nTab ) const;

    /** Returns the pointer to a cell note object at the passed cell address. */
    ScPostIt*       GetNote( const ScAddress& rPos );
    /** Sets the passed note at the cell with the passed cell address. */
    void            TakeNote( const ScAddress& rPos, ScPostIt*& rpNote );
    /** Returns and forgets the cell note object at the passed cell address. */
    ScPostIt*       ReleaseNote( const ScAddress& rPos );
    /** Returns the pointer to an existing or created cell note object at the passed cell address. */
    SC_DLLPUBLIC ScPostIt* GetOrCreateNote( const ScAddress& rPos );
    /** Deletes the note at the passed cell address. */
    void            DeleteNote( const ScAddress& rPos );
    /** Creates the captions of all uninitialized cell notes in the specified sheet.
        @param bForced  True = always create all captions, false = skip when Undo is disabled. */
    void            InitializeNoteCaptions( SCTAB nTab, bool bForced = false );
    /** Creates the captions of all uninitialized cell notes in all sheets.
        @param bForced  True = always create all captions, false = skip when Undo is disabled. */
    void            InitializeAllNoteCaptions( bool bForced = false );

    sal_Bool            ExtendMergeSel( SCCOL nStartCol, SCROW nStartRow,
                                SCCOL& rEndCol, SCROW& rEndRow, const ScMarkData& rMark,
                                sal_Bool bRefresh = sal_False, sal_Bool bAttrs = sal_False );
    sal_Bool            ExtendMerge( SCCOL nStartCol, SCROW nStartRow,
                                SCCOL& rEndCol, SCROW& rEndRow, SCTAB nTab,
                                sal_Bool bRefresh = sal_False, sal_Bool bAttrs = sal_False );
    sal_Bool            ExtendMerge( ScRange& rRange, sal_Bool bRefresh = sal_False, sal_Bool bAttrs = sal_False );
    sal_Bool            ExtendTotalMerge( ScRange& rRange );
    SC_DLLPUBLIC sal_Bool           ExtendOverlapped( SCCOL& rStartCol, SCROW& rStartRow,
                                SCCOL nEndCol, SCROW nEndRow, SCTAB nTab );
    SC_DLLPUBLIC sal_Bool           ExtendOverlapped( ScRange& rRange );

    sal_Bool            RefreshAutoFilter( SCCOL nStartCol, SCROW nStartRow,
                                SCCOL nEndCol, SCROW nEndRow, SCTAB nTab );

    SC_DLLPUBLIC void           DoMergeContents( SCTAB nTab, SCCOL nStartCol, SCROW nStartRow,
                                    SCCOL nEndCol, SCROW nEndRow );
                    //  ohne Ueberpruefung:
    SC_DLLPUBLIC void           DoMerge( SCTAB nTab, SCCOL nStartCol, SCROW nStartRow,
                                    SCCOL nEndCol, SCROW nEndRow, bool bDeleteCaptions = true );
    void            RemoveMerge( SCCOL nCol, SCROW nRow, SCTAB nTab );

    sal_Bool            IsBlockEmpty( SCTAB nTab, SCCOL nStartCol, SCROW nStartRow,
                                                SCCOL nEndCol, SCROW nEndRow, bool bIgnoreNotes = false ) const;
    sal_Bool            IsPrintEmpty( SCTAB nTab, SCCOL nStartCol, SCROW nStartRow,
                                                SCCOL nEndCol, SCROW nEndRow,
                                                sal_Bool bLeftIsEmpty = sal_False,
                                                ScRange* pLastRange = NULL,
                                                Rectangle* pLastMM = NULL ) const;

    sal_Bool            IsHorOverlapped( SCCOL nCol, SCROW nRow, SCTAB nTab ) const;
    sal_Bool            IsVerOverlapped( SCCOL nCol, SCROW nRow, SCTAB nTab ) const;

    SC_DLLPUBLIC bool           HasAttrib( SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                                           SCCOL nCol2, SCROW nRow2, SCTAB nTab2, sal_uInt16 nMask );
    SC_DLLPUBLIC bool           HasAttrib( const ScRange& rRange, sal_uInt16 nMask );

    void            GetBorderLines( SCCOL nCol, SCROW nRow, SCTAB nTab,
                                    const SvxBorderLine** ppLeft,
                                    const SvxBorderLine** ppTop,
                                    const SvxBorderLine** ppRight,
                                    const SvxBorderLine** ppBottom ) const;

    void            ResetChanged( const ScRange& rRange );

    void            SetDirty();
    void            SetDirty( const ScRange& );
    void            SetTableOpDirty( const ScRange& );  // for Interpreter TableOp
    void            InterpretDirtyCells( const ScRangeList& rRanges );
    void            CalcAll();
    SC_DLLPUBLIC void           CalcAfterLoad();
    void            CompileAll();
    void            CompileXML();

    ScAutoNameCache* GetAutoNameCache()     { return pAutoNameCache; }

                    /** Creates a ScLookupCache cache for the range if it
                        doesn't already exist. */
    ScLookupCache & GetLookupCache( const ScRange & rRange );
                    /** Only ScLookupCache ctor uses AddLookupCache(), do not
                        use elsewhere! */
    void            AddLookupCache( ScLookupCache & rCache );
                    /** Only ScLookupCache dtor uses RemoveLookupCache(), do
                        not use elsewhere! */
    void            RemoveLookupCache( ScLookupCache & rCache );
                    /** Zap all caches. */
    void            ClearLookupCaches();

                    // Automatisch Berechnen
    void            SetAutoCalc( sal_Bool bNewAutoCalc );
    sal_Bool            GetAutoCalc() const { return bAutoCalc; }
                    // Automatisch Berechnen in/von/fuer ScDocShell disabled
    void            SetAutoCalcShellDisabled( sal_Bool bNew ) { bAutoCalcShellDisabled = bNew; }
    sal_Bool            IsAutoCalcShellDisabled() const { return bAutoCalcShellDisabled; }
                    // ForcedFormulas zu berechnen
    void            SetForcedFormulaPending( sal_Bool bNew ) { bForcedFormulaPending = bNew; }
    sal_Bool            IsForcedFormulaPending() const { return bForcedFormulaPending; }
                    // if CalcFormulaTree() is currently running
    sal_Bool            IsCalculatingFormulaTree() { return bCalculatingFormulaTree; }

    sal_uInt16          GetErrCode( const ScAddress& ) const;

                    /** Shrink a range to only include data area.

                        This is not the actually used area within the
                        selection, but the bounds of the sheet's data area
                        instead.

                        @returns TRUE if the area passed intersected the data
                                 area, FALSE if not, in which case the values
                                 obtained may be out of bounds, not in order or
                                 unmodified. TRUE does not mean that there
                                 actually is any data within the selection.
                     */
    bool            ShrinkToDataArea( SCTAB nTab, SCCOL& rStartCol, SCROW& rStartRow, SCCOL& rEndCol, SCROW& rEndRow ) const;

                    /** Shrink a range to only include used data area.

                        @param o_bShrunk
                               Out parameter, TRUE if area was shrunk, FALSE if not.

                        @returns TRUE if there is any data, FALSE if not.
                     */
    bool            ShrinkToUsedDataArea( bool& o_bShrunk,
                                          SCTAB nTab, SCCOL& rStartCol, SCROW& rStartRow,
                                          SCCOL& rEndCol, SCROW& rEndRow, bool bColumnsOnly ) const;

    SC_DLLPUBLIC void           GetDataArea( SCTAB nTab, SCCOL& rStartCol, SCROW& rStartRow,
                                    SCCOL& rEndCol, SCROW& rEndRow, sal_Bool bIncludeOld, bool bOnlyDown ) const;
    SC_DLLPUBLIC sal_Bool           GetCellArea( SCTAB nTab, SCCOL& rEndCol, SCROW& rEndRow ) const;
    SC_DLLPUBLIC sal_Bool           GetTableArea( SCTAB nTab, SCCOL& rEndCol, SCROW& rEndRow ) const;
    SC_DLLPUBLIC sal_Bool           GetPrintArea( SCTAB nTab, SCCOL& rEndCol, SCROW& rEndRow,
                                    sal_Bool bNotes = sal_True ) const;
    SC_DLLPUBLIC sal_Bool           GetPrintAreaHor( SCTAB nTab, SCROW nStartRow, SCROW nEndRow,
                                        SCCOL& rEndCol, sal_Bool bNotes = sal_True ) const;
    SC_DLLPUBLIC sal_Bool           GetPrintAreaVer( SCTAB nTab, SCCOL nStartCol, SCCOL nEndCol,
                                        SCROW& rEndRow, sal_Bool bNotes = sal_True ) const;
    void            InvalidateTableArea();


    SC_DLLPUBLIC sal_Bool           GetDataStart( SCTAB nTab, SCCOL& rStartCol, SCROW& rStartRow ) const;

    /**
     * Find the maximum column position that contains printable data for the
     * specified row range.  The final column position must be equal or less
     * than the initial value of rEndCol.
     */
    void            ExtendPrintArea( OutputDevice* pDev, SCTAB nTab,
                                    SCCOL nStartCol, SCROW nStartRow,
                                    SCCOL& rEndCol, SCROW nEndRow );
    SC_DLLPUBLIC SCSIZE         GetEmptyLinesInBlock( SCCOL nStartCol, SCROW nStartRow, SCTAB nStartTab,
                                            SCCOL nEndCol, SCROW nEndRow, SCTAB nEndTab,
                                            ScDirection eDir );

    void            FindAreaPos( SCCOL& rCol, SCROW& rRow, SCTAB nTab, SCsCOL nMovX, SCsROW nMovY );
    SC_DLLPUBLIC void           GetNextPos( SCCOL& rCol, SCROW& rRow, SCTAB nTab, SCsCOL nMovX, SCsROW nMovY,
                                sal_Bool bMarked, sal_Bool bUnprotected, const ScMarkData& rMark );

    sal_Bool            GetNextMarkedCell( SCCOL& rCol, SCROW& rRow, SCTAB nTab,
                                        const ScMarkData& rMark );

    void            LimitChartArea( SCTAB nTab, SCCOL& rStartCol, SCROW& rStartRow,
                                                    SCCOL& rEndCol, SCROW& rEndRow );
    void            LimitChartIfAll( ScRangeListRef& rRangeList );

    sal_Bool            InsertRow( SCCOL nStartCol, SCTAB nStartTab,
                               SCCOL nEndCol,   SCTAB nEndTab,
                               SCROW nStartRow, SCSIZE nSize, ScDocument* pRefUndoDoc = NULL,
                               const ScMarkData* pTabMark = NULL );
    SC_DLLPUBLIC sal_Bool           InsertRow( const ScRange& rRange, ScDocument* pRefUndoDoc = NULL );
    void            DeleteRow( SCCOL nStartCol, SCTAB nStartTab,
                               SCCOL nEndCol,   SCTAB nEndTab,
                               SCROW nStartRow, SCSIZE nSize,
                               ScDocument* pRefUndoDoc = NULL, sal_Bool* pUndoOutline = NULL,
                               const ScMarkData* pTabMark = NULL );
    void            DeleteRow( const ScRange& rRange,
                               ScDocument* pRefUndoDoc = NULL, sal_Bool* pUndoOutline = NULL );
    sal_Bool            InsertCol( SCROW nStartRow, SCTAB nStartTab,
                               SCROW nEndRow,   SCTAB nEndTab,
                               SCCOL nStartCol, SCSIZE nSize, ScDocument* pRefUndoDoc = NULL,
                               const ScMarkData* pTabMark = NULL );
    SC_DLLPUBLIC sal_Bool           InsertCol( const ScRange& rRange, ScDocument* pRefUndoDoc = NULL );
    void            DeleteCol( SCROW nStartRow, SCTAB nStartTab,
                               SCROW nEndRow, SCTAB nEndTab,
                               SCCOL nStartCol, SCSIZE nSize,
                               ScDocument* pRefUndoDoc = NULL, sal_Bool* pUndoOutline = NULL,
                               const ScMarkData* pTabMark = NULL );
    void            DeleteCol( const ScRange& rRange,
                               ScDocument* pRefUndoDoc = NULL, sal_Bool* pUndoOutline = NULL );

    sal_Bool            CanInsertRow( const ScRange& rRange ) const;
    sal_Bool            CanInsertCol( const ScRange& rRange ) const;

    void            FitBlock( const ScRange& rOld, const ScRange& rNew, sal_Bool bClear = sal_True );
    sal_Bool            CanFitBlock( const ScRange& rOld, const ScRange& rNew );

    sal_Bool            IsClipOrUndo() const                        { return bIsClip || bIsUndo; }
    sal_Bool            IsUndo() const                              { return bIsUndo; }
    sal_Bool            IsClipboard() const                         { return bIsClip; }
    bool            IsUndoEnabled() const                       { return mbUndoEnabled; }
    void            EnableUndo( bool bVal );

    bool            IsAdjustHeightEnabled() const               { return mbAdjustHeightEnabled; }
    void            EnableAdjustHeight( bool bVal )             { mbAdjustHeightEnabled = bVal; }
    bool            IsExecuteLinkEnabled() const                { return mbExecuteLinkEnabled; }
    void            EnableExecuteLink( bool bVal )              { mbExecuteLinkEnabled = bVal; }
    bool            IsChangeReadOnlyEnabled() const             { return mbChangeReadOnlyEnabled; }
    void            EnableChangeReadOnly( bool bVal )           { mbChangeReadOnlyEnabled = bVal; }
    SC_DLLPUBLIC sal_Int16       GetNamedRangesLockCount() const             { return mnNamedRangesLockCount; }
    void            SetNamedRangesLockCount( sal_Int16 nCount ) { mnNamedRangesLockCount = nCount; }
    SC_DLLPUBLIC void           ResetClip( ScDocument* pSourceDoc, const ScMarkData* pMarks );
    SC_DLLPUBLIC void           ResetClip( ScDocument* pSourceDoc, SCTAB nTab );
    void            SetCutMode( sal_Bool bCut );
    sal_Bool            IsCutMode();
    void            SetClipArea( const ScRange& rArea, sal_Bool bCut = sal_False );

    SC_DLLPUBLIC sal_Bool           IsDocVisible() const                        { return bIsVisible; }
    void            SetDocVisible( sal_Bool bSet );

    sal_Bool            HasOLEObjectsInArea( const ScRange& rRange, const ScMarkData* pTabMark = NULL );

    void            DeleteObjectsInArea( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                                        const ScMarkData& rMark );
    void            DeleteObjectsInSelection( const ScMarkData& rMark );

    void            DeleteArea(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                            const ScMarkData& rMark, sal_uInt16 nDelFlag);
    void            DeleteAreaTab(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                                SCTAB nTab, sal_uInt16 nDelFlag);
    void            DeleteAreaTab(const ScRange& rRange, sal_uInt16 nDelFlag);

    void            CopyToClip(const ScClipParam& rClipParam, ScDocument* pClipDoc,
                               const ScMarkData* pMarks = NULL, bool bAllTabs = false, bool bKeepScenarioFlags = false,
                               bool bIncludeObjects = false, bool bCloneNoteCaptions = true);

    void            CopyTabToClip(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                                SCTAB nTab, ScDocument* pClipDoc = NULL);
    void            CopyBlockFromClip( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                                    const ScMarkData& rMark, SCsCOL nDx, SCsROW nDy,
                                    const ScCopyBlockFromClipParams* pCBFCP );
    void            CopyNonFilteredFromClip( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                                    const ScMarkData& rMark, SCsCOL nDx, SCsROW nDy,
                                    const ScCopyBlockFromClipParams* pCBFCP,
                                    SCROW & rClipStartRow );
    void            StartListeningFromClip( SCCOL nCol1, SCROW nRow1,
                                        SCCOL nCol2, SCROW nRow2,
                                        const ScMarkData& rMark, sal_uInt16 nInsFlag );
    void            BroadcastFromClip( SCCOL nCol1, SCROW nRow1,
                                    SCCOL nCol2, SCROW nRow2,
                                    const ScMarkData& rMark, sal_uInt16 nInsFlag );
    /** If pDestRanges is given it overrides rDestRange, rDestRange in this
        case is the overall encompassing range. */
    void            CopyFromClip( const ScRange& rDestRange, const ScMarkData& rMark,
                                    sal_uInt16 nInsFlag,
                                    ScDocument* pRefUndoDoc = NULL,
                                    ScDocument* pClipDoc = NULL,
                                    sal_Bool bResetCut = sal_True,
                                    sal_Bool bAsLink = sal_False,
                                    sal_Bool bIncludeFiltered = sal_True,
                                    sal_Bool bSkipAttrForEmpty = sal_False,
                                    const ScRangeList * pDestRanges = NULL );

    void            CopyMultiRangeFromClip(const ScAddress& rDestPos, const ScMarkData& rMark,
                                           sal_uInt16 nInsFlag, ScDocument* pClipDoc,
                                           bool bResetCut = true, bool bAsLink = false,
                                           bool bIncludeFiltered = true,
                                           bool bSkipAttrForEmpty = false);

    void            GetClipArea(SCCOL& nClipX, SCROW& nClipY, sal_Bool bIncludeFiltered);
    void            GetClipStart(SCCOL& nClipX, SCROW& nClipY);

    sal_Bool            HasClipFilteredRows();

    sal_Bool            IsClipboardSource() const;

    SC_DLLPUBLIC void           TransposeClip( ScDocument* pTransClip, sal_uInt16 nFlags, sal_Bool bAsLink );

    ScClipParam&    GetClipParam();
    void            SetClipParam(const ScClipParam& rParam);

    void            MixDocument( const ScRange& rRange, sal_uInt16 nFunction, sal_Bool bSkipEmpty,
                                    ScDocument* pSrcDoc );

    void            FillTab( const ScRange& rSrcArea, const ScMarkData& rMark,
                                sal_uInt16 nFlags, sal_uInt16 nFunction,
                                sal_Bool bSkipEmpty, sal_Bool bAsLink );
    void            FillTabMarked( SCTAB nSrcTab, const ScMarkData& rMark,
                                sal_uInt16 nFlags, sal_uInt16 nFunction,
                                sal_Bool bSkipEmpty, sal_Bool bAsLink );

    void            TransliterateText( const ScMarkData& rMultiMark, sal_Int32 nType );

    SC_DLLPUBLIC void           InitUndo( ScDocument* pSrcDoc, SCTAB nTab1, SCTAB nTab2,
                                sal_Bool bColInfo = sal_False, sal_Bool bRowInfo = sal_False );
    void            AddUndoTab( SCTAB nTab1, SCTAB nTab2,
                                sal_Bool bColInfo = sal_False, sal_Bool bRowInfo = sal_False );
    SC_DLLPUBLIC void           InitUndoSelected( ScDocument* pSrcDoc, const ScMarkData& rTabSelection,
                                sal_Bool bColInfo = sal_False, sal_Bool bRowInfo = sal_False );

                    //  nicht mehr benutzen:
    void            CopyToDocument(SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                                SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                                sal_uInt16 nFlags, sal_Bool bMarked, ScDocument* pDestDoc,
                                const ScMarkData* pMarks = NULL, sal_Bool bColRowFlags = sal_True);
    void            UndoToDocument(SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                                SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                                sal_uInt16 nFlags, sal_Bool bMarked, ScDocument* pDestDoc,
                                const ScMarkData* pMarks = NULL);

    void            CopyToDocument(const ScRange& rRange,
                                sal_uInt16 nFlags, sal_Bool bMarked, ScDocument* pDestDoc,
                                const ScMarkData* pMarks = NULL, sal_Bool bColRowFlags = sal_True);
    void            UndoToDocument(const ScRange& rRange,
                                sal_uInt16 nFlags, sal_Bool bMarked, ScDocument* pDestDoc,
                                const ScMarkData* pMarks = NULL);

    void            CopyScenario( SCTAB nSrcTab, SCTAB nDestTab, sal_Bool bNewScenario = sal_False );
    sal_Bool            TestCopyScenario( SCTAB nSrcTab, SCTAB nDestTab ) const;
    void            MarkScenario( SCTAB nSrcTab, SCTAB nDestTab,
                                    ScMarkData& rDestMark, sal_Bool bResetMark = sal_True,
                                    sal_uInt16 nNeededBits = 0 ) const;
    sal_Bool            HasScenarioRange( SCTAB nTab, const ScRange& rRange ) const;
    SC_DLLPUBLIC const ScRangeList* GetScenarioRanges( SCTAB nTab ) const;

    SC_DLLPUBLIC void           CopyUpdated( ScDocument* pPosDoc, ScDocument* pDestDoc );

    void            UpdateReference( UpdateRefMode eUpdateRefMode, SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                                     SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                                     SCsCOL nDx, SCsROW nDy, SCsTAB nDz,
                                     ScDocument* pUndoDoc = NULL, sal_Bool bIncludeDraw = sal_True,
                                     bool bUpdateNoteCaptionPos = true );

    SC_DLLPUBLIC void           UpdateTranspose( const ScAddress& rDestPos, ScDocument* pClipDoc,
                                        const ScMarkData& rMark, ScDocument* pUndoDoc = NULL );

    void            UpdateGrow( const ScRange& rArea, SCCOL nGrowX, SCROW nGrowY );

    void            Fill(   SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                            const ScMarkData& rMark,
                            sal_uLong nFillCount, FillDir eFillDir = FILL_TO_BOTTOM,
                            FillCmd eFillCmd = FILL_LINEAR, FillDateCmd eFillDateCmd = FILL_DAY,
                            double nStepValue = 1.0, double nMaxValue = 1E307);
    String          GetAutoFillPreview( const ScRange& rSource, SCCOL nEndX, SCROW nEndY );

    sal_Bool            GetSelectionFunction( ScSubTotalFunc eFunc,
                                            const ScAddress& rCursor, const ScMarkData& rMark,
                                            double& rResult );

    SC_DLLPUBLIC const SfxPoolItem*         GetAttr( SCCOL nCol, SCROW nRow, SCTAB nTab, sal_uInt16 nWhich ) const;
    SC_DLLPUBLIC const ScPatternAttr*   GetPattern( SCCOL nCol, SCROW nRow, SCTAB nTab ) const;
    SC_DLLPUBLIC const ScPatternAttr*    GetMostUsedPattern( SCCOL nCol, SCROW nStartRow, SCROW nEndRow, SCTAB nTab ) const;
    const ScPatternAttr*    GetSelectionPattern( const ScMarkData& rMark, sal_Bool bDeep = sal_True );
    ScPatternAttr*          CreateSelectionPattern( const ScMarkData& rMark, sal_Bool bDeep = sal_True );

    const ScConditionalFormat* GetCondFormat( SCCOL nCol, SCROW nRow, SCTAB nTab ) const;
    SC_DLLPUBLIC const SfxItemSet*  GetCondResult( SCCOL nCol, SCROW nRow, SCTAB nTab ) const;
    const SfxPoolItem*  GetEffItem( SCCOL nCol, SCROW nRow, SCTAB nTab, sal_uInt16 nWhich ) const;

    SC_DLLPUBLIC const ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XBreakIterator >& GetBreakIterator();
    sal_Bool            HasStringWeakCharacters( const String& rString );
    SC_DLLPUBLIC sal_uInt8          GetStringScriptType( const String& rString );
    SC_DLLPUBLIC sal_uInt8          GetCellScriptType( ScBaseCell* pCell, sal_uLong nNumberFormat );
    SC_DLLPUBLIC sal_uInt8          GetScriptType( SCCOL nCol, SCROW nRow, SCTAB nTab, ScBaseCell* pCell = NULL );

    sal_Bool            HasDetectiveOperations() const;
    void            AddDetectiveOperation( const ScDetOpData& rData );
    void            ClearDetectiveOperations();
    ScDetOpList*    GetDetOpList() const                { return pDetOpList; }
    void            SetDetOpList(ScDetOpList* pNew);

    sal_Bool            HasDetectiveObjects(SCTAB nTab) const;

    void            GetSelectionFrame( const ScMarkData& rMark,
                                       SvxBoxItem&      rLineOuter,
                                       SvxBoxInfoItem&  rLineInner );
    void            ApplySelectionFrame( const ScMarkData& rMark,
                                         const SvxBoxItem* pLineOuter,
                                         const SvxBoxInfoItem* pLineInner );
    void            ApplyFrameAreaTab( const ScRange& rRange,
                                         const SvxBoxItem* pLineOuter,
                                         const SvxBoxInfoItem* pLineInner );

    void            ClearSelectionItems( const sal_uInt16* pWhich, const ScMarkData& rMark );
    void            ChangeSelectionIndent( sal_Bool bIncrement, const ScMarkData& rMark );

    SC_DLLPUBLIC sal_uLong          AddCondFormat( const ScConditionalFormat& rNew );
    SC_DLLPUBLIC void           FindConditionalFormat( sal_uLong nKey, ScRangeList& rRanges );
    SC_DLLPUBLIC void           FindConditionalFormat( sal_uLong nKey, ScRangeList& rRanges, SCTAB nTab );
    void            ConditionalChanged( sal_uLong nKey );

    SC_DLLPUBLIC sal_uLong          AddValidationEntry( const ScValidationData& rNew );

    SC_DLLPUBLIC const ScValidationData*    GetValidationEntry( sal_uLong nIndex ) const;

    ScConditionalFormatList* GetCondFormList() const        // Ref-Undo
                    { return pCondFormList; }
    void            SetCondFormList(ScConditionalFormatList* pNew);

    ScValidationDataList* GetValidationList() const
                    { return pValidationList; }

    SC_DLLPUBLIC void           ApplyAttr( SCCOL nCol, SCROW nRow, SCTAB nTab,
                                const SfxPoolItem& rAttr );
    SC_DLLPUBLIC void           ApplyPattern( SCCOL nCol, SCROW nRow, SCTAB nTab,
                                    const ScPatternAttr& rAttr );
    SC_DLLPUBLIC void           ApplyPatternArea( SCCOL nStartCol, SCROW nStartRow,
                                        SCCOL nEndCol, SCROW nEndRow,
                                        const ScMarkData& rMark, const ScPatternAttr& rAttr );
    SC_DLLPUBLIC void           ApplyPatternAreaTab( SCCOL nStartCol, SCROW nStartRow,
                                            SCCOL nEndCol, SCROW nEndRow, SCTAB nTab,
                                            const ScPatternAttr& rAttr );
    SC_DLLPUBLIC void           ApplyPatternIfNumberformatIncompatible(
                            const ScRange& rRange, const ScMarkData& rMark,
                            const ScPatternAttr& rPattern, short nNewType );

    void            ApplyStyle( SCCOL nCol, SCROW nRow, SCTAB nTab,
                                const ScStyleSheet& rStyle);
    void            ApplyStyleArea( SCCOL nStartCol, SCROW nStartRow,
                                    SCCOL nEndCol, SCROW nEndRow,
                                    const ScMarkData& rMark, const ScStyleSheet& rStyle);
    SC_DLLPUBLIC void           ApplyStyleAreaTab( SCCOL nStartCol, SCROW nStartRow,
                                        SCCOL nEndCol, SCROW nEndRow, SCTAB nTab,
                                        const ScStyleSheet& rStyle);

    void            ApplySelectionStyle( const ScStyleSheet& rStyle, const ScMarkData& rMark );
    void            ApplySelectionLineStyle( const ScMarkData& rMark,
                                            const SvxBorderLine* pLine, sal_Bool bColorOnly );

    const ScStyleSheet* GetStyle( SCCOL nCol, SCROW nRow, SCTAB nTab ) const;
    const ScStyleSheet* GetSelectionStyle( const ScMarkData& rMark ) const;

    void            StyleSheetChanged( const SfxStyleSheetBase* pStyleSheet, sal_Bool bRemoved,
                                        OutputDevice* pDev,
                                        double nPPTX, double nPPTY,
                                        const Fraction& rZoomX, const Fraction& rZoomY );

    sal_Bool            IsStyleSheetUsed( const ScStyleSheet& rStyle, sal_Bool bGatherAllStyles ) const;

    SC_DLLPUBLIC sal_Bool           ApplyFlagsTab( SCCOL nStartCol, SCROW nStartRow,
                                            SCCOL nEndCol, SCROW nEndRow,
                                            SCTAB nTab, sal_Int16 nFlags );
    sal_Bool            RemoveFlagsTab( SCCOL nStartCol, SCROW nStartRow,
                                            SCCOL nEndCol, SCROW nEndRow,
                                            SCTAB nTab, sal_Int16 nFlags );

    SC_DLLPUBLIC void           SetPattern( const ScAddress&, const ScPatternAttr& rAttr,
                                    sal_Bool bPutToPool = sal_False );
    SC_DLLPUBLIC void           SetPattern( SCCOL nCol, SCROW nRow, SCTAB nTab, const ScPatternAttr& rAttr,
                                    sal_Bool bPutToPool = sal_False );
    void            DeleteNumberFormat( const sal_uInt32* pDelKeys, sal_uInt32 nCount );

    void            AutoFormat( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                                    sal_uInt16 nFormatNo, const ScMarkData& rMark );
    void            GetAutoFormatData( SCTAB nTab, SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                                        ScAutoFormatData& rData );
    sal_Bool            SearchAndReplace( const SvxSearchItem& rSearchItem,
                                        SCCOL& rCol, SCROW& rRow, SCTAB& rTab,
                                        ScMarkData& rMark,
                                        String& rUndoStr, ScDocument* pUndoDoc = NULL );

                    // Col/Row von Folgeaufrufen bestimmen
                    // (z.B. nicht gefunden von Anfang, oder folgende Tabellen)
    static void     GetSearchAndReplaceStart( const SvxSearchItem& rSearchItem,
                        SCCOL& rCol, SCROW& rRow );

    sal_Bool            Solver(SCCOL nFCol, SCROW nFRow, SCTAB nFTab,
                            SCCOL nVCol, SCROW nVRow, SCTAB nVTab,
                            const String& sValStr, double& nX);

    void            ApplySelectionPattern( const ScPatternAttr& rAttr, const ScMarkData& rMark );
    void            DeleteSelection( sal_uInt16 nDelFlag, const ScMarkData& rMark );
    void            DeleteSelectionTab( SCTAB nTab, sal_uInt16 nDelFlag, const ScMarkData& rMark );

    SC_DLLPUBLIC void           SetColWidth( SCCOL nCol, SCTAB nTab, sal_uInt16 nNewWidth );
    SC_DLLPUBLIC void           SetRowHeight( SCROW nRow, SCTAB nTab, sal_uInt16 nNewHeight );
    SC_DLLPUBLIC void           SetRowHeightRange( SCROW nStartRow, SCROW nEndRow, SCTAB nTab,
                                            sal_uInt16 nNewHeight );

    SC_DLLPUBLIC void           SetRowHeightOnly( SCROW nStartRow, SCROW nEndRow, SCTAB nTab,
                                                  sal_uInt16 nNewHeight );
    void                        SetManualHeight( SCROW nStartRow, SCROW nEndRow, SCTAB nTab, sal_Bool bManual );

    SC_DLLPUBLIC sal_uInt16         GetColWidth( SCCOL nCol, SCTAB nTab ) const;
    SC_DLLPUBLIC sal_uInt16         GetRowHeight( SCROW nRow, SCTAB nTab, bool bHiddenAsZero = true ) const;
    SC_DLLPUBLIC sal_uInt16         GetRowHeight( SCROW nRow, SCTAB nTab, SCROW* pStartRow, SCROW* pEndRow, bool bHiddenAsZero = true ) const;
    SC_DLLPUBLIC sal_uLong          GetRowHeight( SCROW nStartRow, SCROW nEndRow, SCTAB nTab ) const;
    SCROW                       GetRowForHeight( SCTAB nTab, sal_uLong nHeight ) const;
    sal_uLong                       GetScaledRowHeight( SCROW nStartRow, SCROW nEndRow, SCTAB nTab, double fScale ) const;
    SC_DLLPUBLIC sal_uLong          GetColOffset( SCCOL nCol, SCTAB nTab ) const;
    SC_DLLPUBLIC sal_uLong          GetRowOffset( SCROW nRow, SCTAB nTab ) const;

    SC_DLLPUBLIC sal_uInt16         GetOriginalWidth( SCCOL nCol, SCTAB nTab ) const;
    SC_DLLPUBLIC sal_uInt16         GetOriginalHeight( SCROW nRow, SCTAB nTab ) const;

    sal_uInt16          GetCommonWidth( SCCOL nEndCol, SCTAB nTab ) const;

    SCROW           GetHiddenRowCount( SCROW nRow, SCTAB nTab ) const;

    sal_uInt16          GetOptimalColWidth( SCCOL nCol, SCTAB nTab, OutputDevice* pDev,
                                        double nPPTX, double nPPTY,
                                        const Fraction& rZoomX, const Fraction& rZoomY,
                                        sal_Bool bFormula,
                                        const ScMarkData* pMarkData = NULL,
                                        sal_Bool bSimpleTextImport = sal_False );
    SC_DLLPUBLIC sal_Bool           SetOptimalHeight( SCROW nStartRow, SCROW nEndRow, SCTAB nTab, sal_uInt16 nExtra,
                                        OutputDevice* pDev,
                                        double nPPTX, double nPPTY,
                                        const Fraction& rZoomX, const Fraction& rZoomY,
                                        sal_Bool bShrink );
    void            UpdateAllRowHeights( OutputDevice* pDev,
                                        double nPPTX, double nPPTY,
                                        const Fraction& rZoomX, const Fraction& rZoomY,
                                        const ScMarkData* pTabMark = NULL );
    long            GetNeededSize( SCCOL nCol, SCROW nRow, SCTAB nTab,
                                    OutputDevice* pDev,
                                    double nPPTX, double nPPTY,
                                    const Fraction& rZoomX, const Fraction& rZoomY,
                                    sal_Bool bWidth, sal_Bool bTotalSize = sal_False );

    SC_DLLPUBLIC void           ShowCol(SCCOL nCol, SCTAB nTab, sal_Bool bShow);
    SC_DLLPUBLIC void           ShowRow(SCROW nRow, SCTAB nTab, sal_Bool bShow);
    SC_DLLPUBLIC void           ShowRows(SCROW nRow1, SCROW nRow2, SCTAB nTab, sal_Bool bShow);
    SC_DLLPUBLIC void           SetColFlags( SCCOL nCol, SCTAB nTab, sal_uInt8 nNewFlags );
    SC_DLLPUBLIC void           SetRowFlags( SCROW nRow, SCTAB nTab, sal_uInt8 nNewFlags );
    SC_DLLPUBLIC void           SetRowFlags( SCROW nStartRow, SCROW nEndRow, SCTAB nTab, sal_uInt8 nNewFlags );

    SC_DLLPUBLIC sal_uInt8          GetColFlags( SCCOL nCol, SCTAB nTab ) const;
    SC_DLLPUBLIC sal_uInt8          GetRowFlags( SCROW nRow, SCTAB nTab ) const;

    SC_DLLPUBLIC const ScBitMaskCompressedArray< SCROW, sal_uInt8> & GetRowFlagsArray( SCTAB nTab ) const;
    SC_DLLPUBLIC       ScBitMaskCompressedArray< SCROW, sal_uInt8> & GetRowFlagsArrayModifiable( SCTAB nTab );

    SC_DLLPUBLIC void           GetAllRowBreaks(::std::set<SCROW>& rBreaks, SCTAB nTab, bool bPage, bool bManual) const;
    SC_DLLPUBLIC void           GetAllColBreaks(::std::set<SCCOL>& rBreaks, SCTAB nTab, bool bPage, bool bManual) const;
    SC_DLLPUBLIC ScBreakType    HasRowBreak(SCROW nRow, SCTAB nTab) const;
    SC_DLLPUBLIC ScBreakType    HasColBreak(SCCOL nCol, SCTAB nTab) const;
    SC_DLLPUBLIC void           SetRowBreak(SCROW nRow, SCTAB nTab, bool bPage, bool bManual);
    SC_DLLPUBLIC void           SetColBreak(SCCOL nCol, SCTAB nTab, bool bPage, bool bManual);
    void                        RemoveRowBreak(SCROW nRow, SCTAB nTab, bool bPage, bool bManual);
    void                        RemoveColBreak(SCCOL nCol, SCTAB nTab, bool bPage, bool bManual);
    ::com::sun::star::uno::Sequence<
        ::com::sun::star::sheet::TablePageBreakData> GetRowBreakData(SCTAB nTab) const;

    SC_DLLPUBLIC bool           RowHidden(SCROW nRow, SCTAB nTab, SCROW* pFirstRow = NULL, SCROW* pLastRow = NULL);
    SC_DLLPUBLIC bool           RowHidden(SCROW nRow, SCTAB nTab, SCROW& rLastRow);
    SC_DLLPUBLIC bool           HasHiddenRows(SCROW nStartRow, SCROW nEndRow, SCTAB nTab);
    SC_DLLPUBLIC bool           ColHidden(SCCOL nCol, SCTAB nTab, SCCOL& rLastCol);
    SC_DLLPUBLIC bool           ColHidden(SCCOL nCol, SCTAB nTab, SCCOL* pFirstCol = NULL, SCCOL* pLastCol = NULL);
    SC_DLLPUBLIC void           SetRowHidden(SCROW nStartRow, SCROW nEndRow, SCTAB nTab, bool bHidden);
    SC_DLLPUBLIC void           SetColHidden(SCCOL nStartCol, SCCOL nEndCol, SCTAB nTab, bool bHidden);
    SC_DLLPUBLIC SCROW          FirstVisibleRow(SCROW nStartRow, SCROW nEndRow, SCTAB nTab);
    SC_DLLPUBLIC SCROW          LastVisibleRow(SCROW nStartRow, SCROW nEndRow, SCTAB nTab);
    SCROW                       CountVisibleRows(SCROW nStartRow, SCROW nEndRow, SCTAB nTab);

    bool                        RowFiltered(SCROW nRow, SCTAB nTab, SCROW* pFirstRow = NULL, SCROW* pLastRow = NULL);
    bool                        HasFilteredRows(SCROW nStartRow, SCROW nEndRow, SCTAB nTab);
    bool                        ColFiltered(SCCOL nCol, SCTAB nTab, SCCOL* pFirstCol = NULL, SCCOL* pLastCol = NULL);
    SC_DLLPUBLIC void           SetRowFiltered(SCROW nStartRow, SCROW nEndRow, SCTAB nTab, bool bFiltered);
    SC_DLLPUBLIC void           SetColFiltered(SCCOL nStartCol, SCCOL nEndCol, SCTAB nTab, bool bFiltered);
    SCROW                       FirstNonFilteredRow(SCROW nStartRow, SCROW nEndRow, SCTAB nTab);
    SCROW                       LastNonFilteredRow(SCROW nStartRow, SCROW nEndRow, SCTAB nTab);
    SCROW                       CountNonFilteredRows(SCROW nStartRow, SCROW nEndRow, SCTAB nTab);

    /**
     * Write all column row flags to table's flag data, because not all column
     * row attributes are stored in the flag data members.  This is necessary
     * for ods export.
     */
    void                        SyncColRowFlags();

                    /// @return  the index of the last row with any set flags (auto-pagebreak is ignored).
    SC_DLLPUBLIC SCROW          GetLastFlaggedRow( SCTAB nTab ) const;

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
    sal_Bool            GetColDefault( SCTAB nTab, SCCOL nCol, SCROW nLastRow, SCROW& nDefault);
    sal_Bool            GetRowDefault( SCTAB nTab, SCROW nRow, SCCOL nLastCol, SCCOL& nDefault);

    sal_Bool            UpdateOutlineCol( SCCOL nStartCol, SCCOL nEndCol, SCTAB nTab, sal_Bool bShow );
    sal_Bool            UpdateOutlineRow( SCROW nStartRow, SCROW nEndRow, SCTAB nTab, sal_Bool bShow );

    void            StripHidden( SCCOL& rX1, SCROW& rY1, SCCOL& rX2, SCROW& rY2, SCTAB nTab );
    void            ExtendHidden( SCCOL& rX1, SCROW& rY1, SCCOL& rX2, SCROW& rY2, SCTAB nTab );

    SC_DLLPUBLIC ScPatternAttr*     GetDefPattern() const;
    SC_DLLPUBLIC ScDocumentPool*        GetPool();
    SC_DLLPUBLIC ScStyleSheetPool*  GetStyleSheetPool() const;

    // PageStyle:
    SC_DLLPUBLIC const String&  GetPageStyle( SCTAB nTab ) const;
    SC_DLLPUBLIC void           SetPageStyle( SCTAB nTab, const String& rName );
    Size            GetPageSize( SCTAB nTab ) const;
    void            SetPageSize( SCTAB nTab, const Size& rSize );
    void            SetRepeatArea( SCTAB nTab, SCCOL nStartCol, SCCOL nEndCol, SCROW nStartRow, SCROW nEndRow );
    void            InvalidatePageBreaks(SCTAB nTab);
    void            UpdatePageBreaks( SCTAB nTab, const ScRange* pUserArea = NULL );
    void            RemoveManualBreaks( SCTAB nTab );
    sal_Bool            HasManualBreaks( SCTAB nTab ) const;

    sal_Bool            IsPageStyleInUse( const String& rStrPageStyle, SCTAB* pInTab = NULL );
    sal_Bool            RemovePageStyleInUse( const String& rStrPageStyle );
    sal_Bool            RenamePageStyleInUse( const String& rOld, const String& rNew );
    void            ModifyStyleSheet( SfxStyleSheetBase& rPageStyle,
                                      const SfxItemSet&  rChanges );

    void            PageStyleModified( SCTAB nTab, const String& rNewName );

    SC_DLLPUBLIC sal_Bool           NeedPageResetAfterTab( SCTAB nTab ) const;

    // war vorher im PageStyle untergracht. Jetzt an jeder Tabelle:
    SC_DLLPUBLIC sal_Bool           HasPrintRange();
    SC_DLLPUBLIC sal_uInt16         GetPrintRangeCount( SCTAB nTab );
    SC_DLLPUBLIC const ScRange* GetPrintRange( SCTAB nTab, sal_uInt16 nPos );
    SC_DLLPUBLIC const ScRange* GetRepeatColRange( SCTAB nTab );
    SC_DLLPUBLIC const ScRange* GetRepeatRowRange( SCTAB nTab );
    /** Returns true, if the specified sheet is always printed. */
    sal_Bool            IsPrintEntireSheet( SCTAB nTab ) const;

    /** Removes all print ranges. */
    SC_DLLPUBLIC void            ClearPrintRanges( SCTAB nTab );
    /** Adds a new print ranges. */
    SC_DLLPUBLIC void            AddPrintRange( SCTAB nTab, const ScRange& rNew );
//UNUSED2009-05 /** Removes all old print ranges and sets the passed print ranges. */
//UNUSED2009-05 void            SetPrintRange( SCTAB nTab, const ScRange& rNew );
    /** Marks the specified sheet to be printed completely. Deletes old print ranges on the sheet! */
    SC_DLLPUBLIC void            SetPrintEntireSheet( SCTAB nTab );
    SC_DLLPUBLIC void           SetRepeatColRange( SCTAB nTab, const ScRange* pNew );
    SC_DLLPUBLIC void           SetRepeatRowRange( SCTAB nTab, const ScRange* pNew );
    ScPrintRangeSaver* CreatePrintRangeSaver() const;
    void            RestorePrintRanges( const ScPrintRangeSaver& rSaver );

    SC_DLLPUBLIC Rectangle      GetMMRect( SCCOL nStartCol, SCROW nStartRow,
                                SCCOL nEndCol, SCROW nEndRow, SCTAB nTab );
    SC_DLLPUBLIC ScRange            GetRange( SCTAB nTab, const Rectangle& rMMRect );

    void            UpdStlShtPtrsFrmNms();
    void            StylesToNames();

    SC_DLLPUBLIC void           CopyStdStylesFrom( ScDocument* pSrcDoc );

    CharSet         GetSrcCharSet() const   { return eSrcSet; }
    sal_uLong           GetSrcVersion() const   { return nSrcVer; }
    SCROW           GetSrcMaxRow() const    { return nSrcMaxRow; }

    void            SetSrcCharSet( CharSet eNew )   { eSrcSet = eNew; }
    void            UpdateFontCharSet();

    void            FillInfo( ScTableInfo& rTabInfo, SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2,
                        SCTAB nTab, double nScaleX, double nScaleY,
                        sal_Bool bPageMode, sal_Bool bFormulaMode,
                        const ScMarkData* pMarkData = NULL );

    SC_DLLPUBLIC SvNumberFormatter* GetFormatTable() const;

    void            Sort( SCTAB nTab, const ScSortParam& rSortParam, sal_Bool bKeepQuery );
    SCSIZE          Query( SCTAB nTab, const ScQueryParam& rQueryParam, sal_Bool bKeepSub );
    sal_Bool            ValidQuery( SCROW nRow, SCTAB nTab, const ScQueryParam& rQueryParam, sal_Bool* pSpecial = NULL );
    SC_DLLPUBLIC sal_Bool           CreateQueryParam( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                                        SCTAB nTab, ScQueryParam& rQueryParam );
    void            GetUpperCellString(SCCOL nCol, SCROW nRow, SCTAB nTab, String& rStr);

    sal_Bool            GetFilterEntries( SCCOL nCol, SCROW nRow, SCTAB nTab,
                                bool bFilter, TypedScStrCollection& rStrings, bool& rHasDates);
    SC_DLLPUBLIC sal_Bool           GetFilterEntriesArea( SCCOL nCol, SCROW nStartRow, SCROW nEndRow,
                                SCTAB nTab, TypedScStrCollection& rStrings, bool& rHasDates );
    sal_Bool            GetDataEntries( SCCOL nCol, SCROW nRow, SCTAB nTab,
                                TypedScStrCollection& rStrings, sal_Bool bLimit = sal_False );
    sal_Bool            GetFormulaEntries( TypedScStrCollection& rStrings );

    sal_Bool            HasAutoFilter( SCCOL nCol, SCROW nRow, SCTAB nTab );

    SC_DLLPUBLIC sal_Bool           HasColHeader( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                                    SCTAB nTab );
    SC_DLLPUBLIC sal_Bool           HasRowHeader( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                                    SCTAB nTab );

    SfxPrinter*     GetPrinter( sal_Bool bCreateIfNotExist = sal_True );
    void            SetPrinter( SfxPrinter* pNewPrinter );
    VirtualDevice*  GetVirtualDevice_100th_mm();
    SC_DLLPUBLIC OutputDevice*  GetRefDevice(); // WYSIWYG: Printer, otherwise VirtualDevice...

    void            EraseNonUsedSharedNames(sal_uInt16 nLevel);
    sal_Bool            GetNextSpellingCell(SCCOL& nCol, SCROW& nRow, SCTAB nTab,
                                        sal_Bool bInSel, const ScMarkData& rMark) const;

    sal_Bool            ReplaceStyle(const SvxSearchItem& rSearchItem,
                                 SCCOL nCol, SCROW nRow, SCTAB nTab,
                                 ScMarkData& rMark, sal_Bool bIsUndo);

    void            DoColResize( SCTAB nTab, SCCOL nCol1, SCCOL nCol2, SCSIZE nAdd );

    void            InvalidateTextWidth( const String& rStyleName );
    void            InvalidateTextWidth( SCTAB nTab );
    void            InvalidateTextWidth( const ScAddress* pAdrFrom, const ScAddress* pAdrTo, sal_Bool bNumFormatChanged );

    sal_Bool            IdleCalcTextWidth();
    sal_Bool            IdleCheckLinks();

    sal_Bool            ContinueOnlineSpelling();   // sal_True = etwas gefunden

    void            RepaintRange( const ScRange& rRange );

    sal_Bool        IsIdleDisabled() const      { return bIdleDisabled; }
    void            DisableIdle(sal_Bool bDo)   { bIdleDisabled = bDo; }

    sal_Bool            IsDetectiveDirty() const     { return bDetectiveDirty; }
    void            SetDetectiveDirty(sal_Bool bSet) { bDetectiveDirty = bSet; }

    void            RemoveAutoSpellObj();
    void            SetOnlineSpellPos( const ScAddress& rPos );
    SC_DLLPUBLIC sal_Bool           SetVisibleSpellRange( const ScRange& rRange );  // sal_True = changed

    sal_uInt8           GetMacroCallMode() const     { return nMacroCallMode; }
    void            SetMacroCallMode(sal_uInt8 nNew)     { nMacroCallMode = nNew; }

    sal_Bool            GetHasMacroFunc() const      { return bHasMacroFunc; }
    void            SetHasMacroFunc(sal_Bool bSet)   { bHasMacroFunc = bSet; }

    sal_Bool            CheckMacroWarn();

    void            SetRangeOverflowType(sal_uInt32 nType)  { nRangeOverflowType = nType; }
    sal_Bool        HasRangeOverflow() const                { return nRangeOverflowType != 0; }
    SC_DLLPUBLIC sal_uInt32      GetRangeOverflowType() const            { return nRangeOverflowType; }

    // fuer Broadcasting/Listening
    void            SetNoSetDirty( sal_Bool bVal ) { bNoSetDirty = bVal; }
    sal_Bool            GetNoSetDirty() const { return bNoSetDirty; }
    void            SetInsertingFromOtherDoc( sal_Bool bVal ) { bInsertingFromOtherDoc = bVal; }
    sal_Bool            IsInsertingFromOtherDoc() const { return bInsertingFromOtherDoc; }
    void            SetLoadingMedium( bool bVal );
    void            SetImportingXML( bool bVal );
    bool            IsImportingXML() const { return bImportingXML; }
    void            SetXMLFromWrapper( sal_Bool bVal );
    sal_Bool            IsXMLFromWrapper() const { return bXMLFromWrapper; }
    void            SetCalcingAfterLoad( sal_Bool bVal ) { bCalcingAfterLoad = bVal; }
    sal_Bool            IsCalcingAfterLoad() const { return bCalcingAfterLoad; }
    void            SetNoListening( sal_Bool bVal ) { bNoListening = bVal; }
    sal_Bool            GetNoListening() const { return bNoListening; }
    ScBroadcastAreaSlotMachine* GetBASM() const { return pBASM; }

    ScChartListenerCollection* GetChartListenerCollection() const
                        { return pChartListenerCollection; }
    void            SetChartListenerCollection( ScChartListenerCollection*,
                        sal_Bool bSetChartRangeLists = sal_False );
    void            UpdateChart( const String& rName );
    void            RestoreChartListener( const String& rName );
    SC_DLLPUBLIC void           UpdateChartListenerCollection();
    sal_Bool            IsChartListenerCollectionNeedsUpdate() const
                        { return bChartListenerCollectionNeedsUpdate; }
    void            SetChartListenerCollectionNeedsUpdate( sal_Bool bFlg )
                        { bChartListenerCollectionNeedsUpdate = bFlg; }
    void            AddOLEObjectToCollection(const String& rName);

    ScChangeViewSettings* GetChangeViewSettings() const     { return pChangeViewSettings; }
    SC_DLLPUBLIC void               SetChangeViewSettings(const ScChangeViewSettings& rNew);

    vos::ORef<SvxForbiddenCharactersTable> GetForbiddenCharacters();
    void            SetForbiddenCharacters( const vos::ORef<SvxForbiddenCharactersTable> xNew );

    sal_uInt8           GetAsianCompression() const;        // CharacterCompressionType values
    sal_Bool            IsValidAsianCompression() const;
    void            SetAsianCompression(sal_uInt8 nNew);

    sal_Bool            GetAsianKerning() const;
    sal_Bool            IsValidAsianKerning() const;
    void            SetAsianKerning(sal_Bool bNew);

    sal_uInt8           GetEditTextDirection(SCTAB nTab) const; // EEHorizontalTextDirection values

    SC_DLLPUBLIC ScLkUpdMode        GetLinkMode() const             { return eLinkMode ;}
    void            SetLinkMode( ScLkUpdMode nSet ) {   eLinkMode  = nSet;}


private:
    ScDocument(const ScDocument& r); // disabled with no definition

    void                FindMaxRotCol( SCTAB nTab, RowInfo* pRowInfo, SCSIZE nArrCount,
                                        SCCOL nX1, SCCOL nX2 ) const;

    sal_uInt16              RowDifferences( SCROW nThisRow, SCTAB nThisTab,
                                        ScDocument& rOtherDoc,
                                        SCROW nOtherRow, SCTAB nOtherTab,
                                        SCCOL nMaxCol, SCCOLROW* pOtherCols );
    sal_uInt16              ColDifferences( SCCOL nThisCol, SCTAB nThisTab,
                                        ScDocument& rOtherDoc,
                                        SCCOL nOtherCol, SCTAB nOtherTab,
                                        SCROW nMaxRow, SCCOLROW* pOtherRows );
    void                FindOrder( SCCOLROW* pOtherRows, SCCOLROW nThisEndRow, SCCOLROW nOtherEndRow,
                                        sal_Bool bColumns,
                                        ScDocument& rOtherDoc, SCTAB nThisTab, SCTAB nOtherTab,
                                        SCCOLROW nEndCol, SCCOLROW* pTranslate,
                                        ScProgress* pProgress, sal_uLong nProAdd );
    sal_Bool                OnlineSpellInRange( const ScRange& rSpellRange, ScAddress& rSpellPos,
                                        sal_uInt16 nMaxTest );

    DECL_LINK( TrackTimeHdl, Timer* );

    static ScRecursionHelper*   CreateRecursionHelperInstance();

public:
    void                StartListeningArea( const ScRange& rRange,
                                            SvtListener* pListener );
    void                EndListeningArea( const ScRange& rRange,
                                            SvtListener* pListener );
                        /** Broadcast wrapper, calls
    SC_DLLPUBLIC                         rHint.GetCell()->Broadcast() and AreaBroadcast()
                            and TrackFormulas() and conditional format list
                            SourceChanged().
                            Preferred.
                         */
    void                Broadcast( const ScHint& rHint );
                        /// deprecated
    void                Broadcast( sal_uLong nHint, const ScAddress& rAddr,
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
    void                CalcFormulaTree( sal_Bool bOnlyForced = sal_False,
                                        sal_Bool bNoProgressBar = sal_False );
    void                ClearFormulaTree();
    void                AppendToFormulaTrack( ScFormulaCell* pCell );
    void                RemoveFromFormulaTrack( ScFormulaCell* pCell );
    void                TrackFormulas( sal_uLong nHintId = SC_HINT_DATACHANGED );
    sal_uInt16              GetFormulaTrackCount() const { return nFormulaTrackCount; }
    sal_Bool                IsInFormulaTree( ScFormulaCell* pCell ) const;
    sal_Bool                IsInFormulaTrack( ScFormulaCell* pCell ) const;
    sal_uInt16              GetHardRecalcState() { return nHardRecalcState; }
    void                SetHardRecalcState( sal_uInt16 nVal ) { nHardRecalcState = nVal; }
    void                StartAllListeners();
    const ScFormulaCell*    GetFormulaTree() const { return pFormulaTree; }
    sal_Bool                HasForcedFormulas() const { return bHasForcedFormulas; }
    void                SetForcedFormulas( sal_Bool bVal ) { bHasForcedFormulas = bVal; }
    sal_uLong               GetFormulaCodeInTree() const { return nFormulaCodeInTree; }
    sal_Bool                IsInInterpreter() const { return nInterpretLevel != 0; }
    sal_uInt16              GetInterpretLevel() { return nInterpretLevel; }
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
    sal_Bool                IsInMacroInterpreter() const { return nMacroInterpretLevel != 0; }
    sal_uInt16              GetMacroInterpretLevel() { return nMacroInterpretLevel; }
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
    sal_Bool                IsInInterpreterTableOp() const { return nInterpreterTableOpLevel != 0; }
    sal_uInt16              GetInterpreterTableOpLevel() { return nInterpreterTableOpLevel; }
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
    void                InvalidateLastTableOpParams() { aLastTableOpParams.bValid = sal_False; }
    ScRecursionHelper&  GetRecursionHelper()
                            {
                                if (!pRecursionHelper)
                                    pRecursionHelper = CreateRecursionHelperInstance();
                                return *pRecursionHelper;
                            }
    sal_Bool                IsInDtorClear() const { return bInDtorClear; }
    void                SetExpandRefs( sal_Bool bVal ) { bExpandRefs = bVal; }
    sal_Bool                IsExpandRefs() { return bExpandRefs; }

    SC_DLLPUBLIC void               IncSizeRecalcLevel( SCTAB nTab );
    SC_DLLPUBLIC void               DecSizeRecalcLevel( SCTAB nTab, bool bUpdateNoteCaptionPos = true );

    sal_uLong               GetXMLImportedFormulaCount() const { return nXMLImportedFormulaCount; }
    void                IncXMLImportedFormulaCount( sal_uLong nVal )
                            {
                                if ( nXMLImportedFormulaCount + nVal > nXMLImportedFormulaCount )
                                    nXMLImportedFormulaCount += nVal;
                            }
    void                DecXMLImportedFormulaCount( sal_uLong nVal )
                            {
                                if ( nVal <= nXMLImportedFormulaCount )
                                    nXMLImportedFormulaCount -= nVal;
                                else
                                    nXMLImportedFormulaCount = 0;
                            }

    void                StartTrackTimer();

    void            CompileDBFormula();
    void            CompileDBFormula( sal_Bool bCreateFormulaString );
    void            CompileNameFormula( sal_Bool bCreateFormulaString );
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
    xub_StrLen      GetMaxNumberStringLen( sal_uInt16& nPrecision,
                                           SCTAB nTab, SCCOL nCol,
                                           SCROW nRowStart, SCROW nRowEnd ) const;

    void    KeyInput( const KeyEvent& rKEvt );      // TimerDelays etc.

    ScChangeTrack*      GetChangeTrack() const { return pChangeTrack; }

    //! only for import filter, deletes any existing ChangeTrack via
    //! EndChangeTracking() and takes ownership of new ChangeTrack pTrack
    SC_DLLPUBLIC void           SetChangeTrack( ScChangeTrack* pTrack );

    void            StartChangeTracking();
    void            EndChangeTracking();

    SC_DLLPUBLIC void           CompareDocument( ScDocument& rOtherDoc );

    void            AddUnoObject( SfxListener& rObject );
    void            RemoveUnoObject( SfxListener& rObject );
    void            BroadcastUno( const SfxHint &rHint );
    void            AddUnoListenerCall( const ::com::sun::star::uno::Reference<
                                            ::com::sun::star::util::XModifyListener >& rListener,
                                        const ::com::sun::star::lang::EventObject& rEvent );

    void            SetInLinkUpdate(sal_Bool bSet);             // TableLink or AreaLink
    sal_Bool            IsInLinkUpdate() const;                 // including DdeLink

    SC_DLLPUBLIC SfxItemPool*       GetEditPool() const;
    SC_DLLPUBLIC SfxItemPool*       GetEnginePool() const;
    SC_DLLPUBLIC ScFieldEditEngine& GetEditEngine();
    SC_DLLPUBLIC ScNoteEditEngine&  GetNoteEngine();
//UNUSED2009-05 SfxItemPool&            GetNoteItemPool();

    ScRefreshTimerControl*  GetRefreshTimerControl() const
        { return pRefreshTimerControl; }
    ScRefreshTimerControl * const * GetRefreshTimerControlAddress() const
        { return &pRefreshTimerControl; }

    void            SetPastingDrawFromOtherDoc( sal_Bool bVal )
                        { bPastingDrawFromOtherDoc = bVal; }
    sal_Bool            PastingDrawFromOtherDoc() const
                        { return bPastingDrawFromOtherDoc; }

                    /// an ID unique to each document instance
    sal_uInt32      GetDocumentID() const;

    void            InvalidateStyleSheetUsage()
                        { bStyleSheetUsageInvalid = sal_True; }
    void GetSortParam( ScSortParam& rParam, SCTAB nTab );
    void SetSortParam( ScSortParam& rParam, SCTAB nTab );

    inline void     SetVbaEventProcessor( const com::sun::star::uno::Reference< com::sun::star::script::vba::XVBAEventProcessor >& rxVbaEvents )
                        { mxVbaEvents = rxVbaEvents; }
    inline com::sun::star::uno::Reference< com::sun::star::script::vba::XVBAEventProcessor >
                    GetVbaEventProcessor() const { return mxVbaEvents; }

    /** Should only be GRAM_PODF or GRAM_ODFF. */
    void                SetStorageGrammar( formula::FormulaGrammar::Grammar eGrammar );
    formula::FormulaGrammar::Grammar  GetStorageGrammar() const
                            { return eStorageGrammar; }

    SfxUndoManager*     GetUndoManager();
    bool IsInVBAMode() const;
    ScRowBreakIterator* GetRowBreakIterator(SCTAB nTab) const;

private: // CLOOK-Impl-Methoden

    /**
     * Use this class as a locale variable to merge number formatter from
     * another document, and set NULL pointer to pFormatExchangeList when
     * done.
     */
    class NumFmtMergeHandler
    {
    public:
        explicit NumFmtMergeHandler(ScDocument* pDoc, ScDocument* pSrcDoc);
        ~NumFmtMergeHandler();

    private:
        ScDocument* mpDoc;
    };

    void    MergeNumberFormatter(ScDocument* pSrcDoc);

    void    ImplCreateOptions(); // bei Gelegenheit auf on-demand umstellen?
    void    ImplDeleteOptions();

    void    DeleteDrawLayer();
    void    DeleteColorTable();
    SC_DLLPUBLIC sal_Bool   DrawGetPrintArea( ScRange& rRange, sal_Bool bSetHor, sal_Bool bSetVer ) const;
    void    DrawMovePage( sal_uInt16 nOldPos, sal_uInt16 nNewPos );
    void    DrawCopyPage( sal_uInt16 nOldPos, sal_uInt16 nNewPos );

    void    UpdateDrawPrinter();
    void    UpdateDrawLanguages();
    void    UpdateDrawDefaults();
    SC_DLLPUBLIC void   InitClipPtrs( ScDocument* pSourceDoc );

    void    LoadDdeLinks(SvStream& rStream);
    void    SaveDdeLinks(SvStream& rStream) const;

    void    DeleteAreaLinksOnTab( SCTAB nTab );
    void    UpdateRefAreaLinks( UpdateRefMode eUpdateRefMode,
                             const ScRange& r, SCsCOL nDx, SCsROW nDy, SCsTAB nDz );

    void    CopyRangeNamesToClip(ScDocument* pClipDoc, const ScRange& rClipRange, const ScMarkData* pMarks, bool bAllTabs);
    void    CopyRangeNamesFromClip(ScDocument* pClipDoc, ScClipRangeNameData& rRangeNames);
    void    UpdateRangeNamesInFormulas(
        ScClipRangeNameData& rRangeNames, const ScRangeList& rDestRanges, const ScMarkData& rMark,
        SCCOL nXw, SCROW nYw);

    sal_Bool    HasPartOfMerged( const ScRange& rRange );

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

#endif


