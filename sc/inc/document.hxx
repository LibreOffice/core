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

#ifndef SC_DOCUMENT_HXX
#define SC_DOCUMENT_HXX


#include <vcl/prntypes.hxx>
#include <vcl/timer.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <rtl/ref.hxx>
#include "scdllapi.h"
#include "rangelst.hxx"
#include "table.hxx"
#include "brdcst.hxx"
#include "tabopparams.hxx"
#include "types.hxx"
#include "formula/grammar.hxx"
#include <com/sun/star/chart2/XChartDocument.hpp>
#include "scdllapi.h"

#include <memory>
#include <map>
#include <set>
#include <list>
#include <boost/ptr_container/ptr_vector.hpp>

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

struct ScAttrEntry;
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
class ScMacroManager;
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
struct ScSetStringParam;
class ScDocRowHeightUpdater;
struct ScColWidthParam;
struct ScCopyBlockFromClipParams;

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

// DDE link modes
const BYTE SC_DDE_DEFAULT       = 0;
const BYTE SC_DDE_ENGLISH       = 1;
const BYTE SC_DDE_TEXT          = 2;
const BYTE SC_DDE_IGNOREMODE    = 255;       /// For usage in FindDdeLink() only!

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
friend class ScDocRowHeightUpdater;

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceManager;

    rtl::Reference<ScPoolHelper> xPoolHelper;

    SfxUndoManager*     mpUndoManager;
    ScFieldEditEngine*  pEditEngine;                    // uses pEditPool from xPoolHelper
    ScNoteEditEngine*   pNoteEngine;                    // uses pEditPool from xPoolHelper
    SfxItemPool*    pNoteItemPool; // SfxItemPool to be used if pDrawLayer not created.
    SfxObjectShell*     pShell;
    SfxPrinter*         pPrinter;
    VirtualDevice*      pVirtualDevice_100th_mm;
    ScDrawLayer*        pDrawLayer;                     // SdrModel
    XColorTable*        pColorTable;
    ScConditionalFormatList* pCondFormList;             // conditional formats
    ScValidationDataList* pValidationList;              // validity
    SvNumberFormatterIndexTable*    pFormatExchangeList;    // for application of number formats
    ScTable*            pTab[MAXTABCOUNT];
    ScRangeName*        pRangeName;
    ScDBCollection*     pDBCollection;
    ScDPCollection*     pDPCollection;
    ScChartCollection*  pChartCollection;
    std::auto_ptr< ScTemporaryChartLock > apTemporaryChartLock;
    ScPatternAttr*      pSelectionAttr;                 // Attributes of a block
    mutable sfx2::LinkManager*      pLinkManager;
    ScFormulaCell*      pFormulaTree;                   // formula tree (start)
    ScFormulaCell*      pEOFormulaTree;                 // formula tree (end), last cell
    ScFormulaCell*      pFormulaTrack;                  // BroadcastTrack (start)
    ScFormulaCell*      pEOFormulaTrack;                // BrodcastTrack (end), last cell
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
    rtl::Reference<SvxForbiddenCharactersTable> xForbiddenCharacters;

    ScFieldEditEngine*  pCacheFieldEditEngine;

    ::std::auto_ptr<ScDocProtection> pDocProtection;
    ::std::auto_ptr<ScClipParam>     mpClipParam;

    ::std::auto_ptr<ScExternalRefManager> pExternalRefMgr;
    ::std::auto_ptr<ScMacroManager> mpMacroMgr;


    // mutable for lazy construction
    mutable ::std::auto_ptr< ScFormulaParserPool >
                        mxFormulaParserPool;            /// Pool for all external formula parsers used by this document.

    String              aDocName;                       // optional: name of document
    String              aDocCodeName;                       // optional: name of document (twice?)
    ScRangePairListRef  xColNameRanges;
    ScRangePairListRef  xRowNameRanges;

    ScViewOptions*      pViewOptions;                   // view options
    ScDocOptions*       pDocOptions;                    // document options
    ScExtDocOptions*    pExtDocOptions;                 // for import etc.
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
    boost::ptr_vector< ScInterpreterTableOpParams > aTableOpList; // list of ScInterpreterTableOpParams currently in use
    ScInterpreterTableOpParams  aLastTableOpParams;     // remember last params
private:

    LanguageType        eLanguage;                      // default language
    LanguageType        eCjkLanguage;                   // default language for asian text
    LanguageType        eCtlLanguage;                   // default language for complex text
    CharSet             eSrcSet;                        // during reading: source character set

    /** The compiler grammar used in document storage. GRAM_PODF for ODF 1.1
        documents, GRAM_ODFF for ODF 1.2 documents. */
    formula::FormulaGrammar::Grammar  eStorageGrammar;

    /** The compiler grammar used in ODF import after brackets had been
        stripped (which they shouldn't, but until that's fixed) by the XML
        importer. */
    formula::FormulaGrammar::Grammar  eXmlImportGrammar;

    ULONG               nFormulaCodeInTree;             // FormelRPN im Formelbaum
    ULONG               nXMLImportedFormulaCount;        // progress count during XML import
    USHORT              nInterpretLevel;                // >0 if in interpreter
    USHORT              nMacroInterpretLevel;           // >0 if macro in interpreter
    USHORT              nInterpreterTableOpLevel;       // >0 if in interpreter TableOp
    SCTAB               nMaxTableNumber;
    USHORT              nSrcVer;                        // file version (load/save)
    SCROW               nSrcMaxRow;                     // number of lines to load/save
    USHORT              nFormulaTrackCount;
    USHORT              nHardRecalcState;               // 0: soft, 1: hard-warn, 2: hard
    SCTAB               nVisibleTab;                    // for OLE etc.

    ScLkUpdMode         eLinkMode;

    BOOL                bAutoCalc;                      // calculate automatically
    BOOL                bAutoCalcShellDisabled;         // in/from/for ScDocShell disabled
    // are there ForcedFormulas which have to be calculated
    // in interaction with ScDocShell SetDocumentModified,
    // AutoCalcShellDisabled and TrackFormulas
    BOOL                bForcedFormulaPending;
    BOOL                bCalculatingFormulaTree;
    BOOL                bIsClip;
    BOOL                bIsUndo;
    BOOL                bIsVisible;                     // set from view ctor

    BOOL                bIsEmbedded;                    // display/adjust Embedded area?

    // no SetDirty for ScFormulaCell::CompileTokenArray but at the end of
    // ScDocument::CompileAll[WithFormats], CopyScenario, CopyBlockFromClip
    BOOL                bNoSetDirty;
    // no broadcast, construct no listener during insert from a different
    // Doc (per filter or the like ), not until CompileAll / CalcAfterLoad
    BOOL                bInsertingFromOtherDoc;
    bool                bLoadingMedium;
    bool                bImportingXML;      // special handling of formula text
    BOOL                bXMLFromWrapper;    // distinguish ScXMLImportWrapper from external component
    BOOL                bCalcingAfterLoad;              // in CalcAfterLoad TRUE
    // don't construct/destruct listeners temporarily
    BOOL                bNoListening;
    BOOL                bIdleDisabled;
    BOOL                bInLinkUpdate;                  // TableLink or AreaLink
    BOOL                bChartListenerCollectionNeedsUpdate;
    // are/were there RC_FORCED formula cells in the document (if set once to TRUE then set forever)
    BOOL                bHasForcedFormulas;
    // is the Doc being destroyed? (no Notify-Tracking etc. needed anymore)
    BOOL                bInDtorClear;
    // expand reference if insert column/row takes place at the border
    // of a reference
    // is fetched in each UpdateReference from InputOptions,
    // assigned, and restored at the end of UpdateReference
    BOOL                bExpandRefs;
    // for detective update, is set for each change of a formula
    BOOL                bDetectiveDirty;

    BYTE                nMacroCallMode;     // Macros per warning dialog disabled?
    BOOL                bHasMacroFunc;      // valid only after loading

    BYTE                nVisSpellState;

    BYTE                nAsianCompression;
    BYTE                nAsianKerning;
    BOOL                bSetDrawDefaults;

    BOOL                bPastingDrawFromOtherDoc;

    BYTE                nInDdeLinkUpdate;   // originating DDE links (stacked bool)

    BOOL                bInUnoBroadcast;
    BOOL                bInUnoListenerCall;
    formula::FormulaGrammar::Grammar  eGrammar;

    mutable BOOL        bStyleSheetUsageInvalid;

    bool                mbUndoEnabled;
    bool                mbAdjustHeightEnabled;
    bool                mbExecuteLinkEnabled;
    bool                mbChangeReadOnlyEnabled;    // allow changes in read-only document (for API import filters)
    bool                mbStreamValidLocked;

    sal_Int16           mnNamedRangesLockCount;

    ::std::set<ScFormulaCell*> maSubTotalCells;

public:
    SC_DLLPUBLIC ULONG          GetCellCount() const;       // all cells
    SCSIZE          GetCellCount(SCTAB nTab, SCCOL nCol) const;
    ULONG           GetCodeCount() const;       // RPN-Code in formulas
    DECL_LINK( GetUserDefinedColor, USHORT * );
                                                                // number formatter

public:
    SC_DLLPUBLIC                ScDocument( ScDocumentMode eMode = SCDOCMODE_DOCUMENT,
                                SfxObjectShell* pDocShell = NULL );
    SC_DLLPUBLIC                ~ScDocument();

    inline ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                    GetServiceManager() const { return xServiceManager; }

    SC_DLLPUBLIC const String&  GetName() const { return aDocName; }
    void            SetName( const String& r ) { aDocName = r; }
    const String&   GetCodeName() const { return aDocCodeName; }
    void                SetCodeName( const String& r ) { aDocCodeName = r; }

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

    SC_DLLPUBLIC ScRangeName*   GetRangeName(SCTAB nTab) const;
    SC_DLLPUBLIC ScRangeName*   GetRangeName() const;
    void            SetRangeName( ScRangeName* pNewRangeName );
    SCTAB           GetMaxTableNumber() { return nMaxTableNumber; }
    void            SetMaxTableNumber(SCTAB nNumber) { nMaxTableNumber = nNumber; }

    ScRangePairList*    GetColNameRanges() { return &xColNameRanges; }
    ScRangePairList*    GetRowNameRanges() { return &xRowNameRanges; }
    ScRangePairListRef& GetColNameRangesRef() { return xColNameRanges; }
    ScRangePairListRef& GetRowNameRangesRef() { return xRowNameRanges; }

    SC_DLLPUBLIC ScDBCollection*    GetDBCollection() const;
    void            SetDBCollection( ScDBCollection* pNewDBCollection,
                                        BOOL bRemoveAutoFilter = FALSE );
    ScDBData*       GetDBAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab,
                                        BOOL bStartOnly = FALSE) const;
    ScDBData*       GetDBAtArea(SCTAB nTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2) const;

    SC_DLLPUBLIC const ScRangeData* GetRangeAtBlock( const ScRange& rBlock, String* pName=NULL ) const;

    SC_DLLPUBLIC ScDPCollection*        GetDPCollection();
    ScDPObject*         GetDPAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab) const;
    ScDPObject*         GetDPAtBlock( const ScRange& rBlock ) const;

    SC_DLLPUBLIC ScChartCollection* GetChartCollection() const;

    void                StopTemporaryChartLock();

    void            EnsureGraphicNames();

    SdrObject*      GetObjectAtPoint( SCTAB nTab, const Point& rPos );
    BOOL            HasChartAtPoint( SCTAB nTab, const Point& rPos, String* pName = NULL );

    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartDocument > GetChartByName( const String& rChartName );
    SC_DLLPUBLIC void            GetChartRanges( const String& rChartName, ::std::vector< ScRangeList >& rRanges, ScDocument* pSheetNameDoc );
    void            SetChartRanges( const String& rChartName, const ::std::vector< ScRangeList >& rRanges );

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

    SC_DLLPUBLIC void           MakeTable( SCTAB nTab,bool _bNeedsNameCheck = true );

    SCTAB           GetVisibleTab() const       { return nVisibleTab; }
    SC_DLLPUBLIC void           SetVisibleTab(SCTAB nTab)   { nVisibleTab = nTab; }

    SC_DLLPUBLIC BOOL           HasTable( SCTAB nTab ) const;
    SC_DLLPUBLIC BOOL           GetName( SCTAB nTab, String& rName ) const;
    SC_DLLPUBLIC BOOL           GetCodeName( SCTAB nTab, String& rName ) const;
    SC_DLLPUBLIC BOOL                   SetCodeName( SCTAB nTab, const String& rName );
    SC_DLLPUBLIC BOOL           GetTable( const String& rName, SCTAB& rTab ) const;
    SC_DLLPUBLIC inline SCTAB   GetTableCount() const { return nMaxTableNumber; }
    SvNumberFormatterIndexTable* GetFormatExchangeList() const { return pFormatExchangeList; }

    SC_DLLPUBLIC ScDocProtection* GetDocProtection() const;
    SC_DLLPUBLIC void            SetDocProtection(const ScDocProtection* pProtect);
    SC_DLLPUBLIC BOOL           IsDocProtected() const;
    BOOL            IsDocEditable() const;
    SC_DLLPUBLIC BOOL           IsTabProtected( SCTAB nTab ) const;
    SC_DLLPUBLIC    ScTableProtection* GetTabProtection( SCTAB nTab ) const;
    SC_DLLPUBLIC void SetTabProtection(SCTAB nTab, const ScTableProtection* pProtect);
    void            CopyTabProtection(SCTAB nTabSrc, SCTAB nTabDest);

    void            LockTable(SCTAB nTab);
    void            UnlockTable(SCTAB nTab);

    BOOL            IsBlockEditable( SCTAB nTab, SCCOL nStartCol, SCROW nStartRow,
                                        SCCOL nEndCol, SCROW nEndRow,
                                        BOOL* pOnlyNotBecauseOfMatrix = NULL ) const;
    BOOL            IsSelectionEditable( const ScMarkData& rMark,
                                        BOOL* pOnlyNotBecauseOfMatrix = NULL ) const;
    BOOL            HasSelectedBlockMatrixFragment( SCCOL nStartCol, SCROW nStartRow,
                                            SCCOL nEndCol, SCROW nEndRow,
                                            const ScMarkData& rMark ) const;
    BOOL            HasSelectedBlockMatrixFragment( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow, SCTAB nTAB ) const;

    BOOL            GetMatrixFormulaRange( const ScAddress& rCellPos, ScRange& rMatrix );

    BOOL            IsEmbedded() const;
    void            GetEmbedded( ScRange& rRange ) const;
    void            SetEmbedded( const ScRange& rRange );
    void            ResetEmbedded();
    Rectangle       GetEmbeddedRect() const;                        // 1/100 mm
    void            SetEmbedded( const Rectangle& rRect );          // from VisArea (1/100 mm)
    void            SnapVisArea( Rectangle& rRect ) const;          // 1/100 mm

    static SC_DLLPUBLIC bool ValidTabName( const String& rName );

    SC_DLLPUBLIC BOOL           ValidNewTabName( const String& rName ) const;
    SC_DLLPUBLIC void           CreateValidTabName(String& rName) const;
    SC_DLLPUBLIC BOOL           InsertTab( SCTAB nPos, const String& rName,
                                BOOL bExternalDocument = FALSE );
    SC_DLLPUBLIC BOOL            DeleteTab( SCTAB nTab, ScDocument* pRefUndoDoc = NULL );
    SC_DLLPUBLIC BOOL           RenameTab( SCTAB nTab, const String& rName,
                                BOOL bUpdateRef = TRUE,
                                BOOL bExternalDocument = FALSE );
    BOOL            MoveTab( SCTAB nOldPos, SCTAB nNewPos );
    BOOL            CopyTab( SCTAB nOldPos, SCTAB nNewPos,
                                const ScMarkData* pOnlyMarked = NULL );
    SC_DLLPUBLIC ULONG          TransferTab(ScDocument* pSrcDoc, SCTAB nSrcPos, SCTAB nDestPos,
                                    BOOL bInsertNew = TRUE,
                                    BOOL bResultsOnly = FALSE );
    SC_DLLPUBLIC void           TransferDrawPage(ScDocument* pSrcDoc, SCTAB nSrcPos, SCTAB nDestPos);
    SC_DLLPUBLIC void           SetVisible( SCTAB nTab, BOOL bVisible );
    SC_DLLPUBLIC BOOL           IsVisible( SCTAB nTab ) const;
    BOOL            IsStreamValid( SCTAB nTab ) const;
    void            SetStreamValid( SCTAB nTab, BOOL bSet, BOOL bIgnoreLock = FALSE );
    void            LockStreamValid( bool bLock );
    bool            IsStreamValidLocked() const                         { return mbStreamValidLocked; }
    BOOL            IsPendingRowHeights( SCTAB nTab ) const;
    void            SetPendingRowHeights( SCTAB nTab, BOOL bSet );
    SC_DLLPUBLIC void           SetLayoutRTL( SCTAB nTab, BOOL bRTL );
    SC_DLLPUBLIC BOOL           IsLayoutRTL( SCTAB nTab ) const;
    BOOL            IsNegativePage( SCTAB nTab ) const;
    SC_DLLPUBLIC void           SetScenario( SCTAB nTab, BOOL bFlag );
    SC_DLLPUBLIC BOOL           IsScenario( SCTAB nTab ) const;
    SC_DLLPUBLIC void           GetScenarioData( SCTAB nTab, String& rComment,
                                        Color& rColor, USHORT& rFlags ) const;
    SC_DLLPUBLIC void           SetScenarioData( SCTAB nTab, const String& rComment,
                                        const Color& rColor, USHORT nFlags );
    SC_DLLPUBLIC Color GetTabBgColor( SCTAB nTab ) const;
    SC_DLLPUBLIC void SetTabBgColor( SCTAB nTab, const Color& rColor );
    SC_DLLPUBLIC bool IsDefaultTabBgColor( SCTAB nTab ) const;
    void            GetScenarioFlags( SCTAB nTab, USHORT& rFlags ) const;
    SC_DLLPUBLIC BOOL           IsActiveScenario( SCTAB nTab ) const;
    SC_DLLPUBLIC void           SetActiveScenario( SCTAB nTab, BOOL bActive );      // only for Undo etc.
    SC_DLLPUBLIC formula::FormulaGrammar::AddressConvention GetAddressConvention() const;
    SC_DLLPUBLIC formula::FormulaGrammar::Grammar GetGrammar() const;
    void            SetGrammar( formula::FormulaGrammar::Grammar eGram );
    SC_DLLPUBLIC BYTE           GetLinkMode( SCTAB nTab ) const;
    BOOL            IsLinked( SCTAB nTab ) const;
    SC_DLLPUBLIC const String&  GetLinkDoc( SCTAB nTab ) const;
    const String&   GetLinkFlt( SCTAB nTab ) const;
    const String&   GetLinkOpt( SCTAB nTab ) const;
    SC_DLLPUBLIC const String&  GetLinkTab( SCTAB nTab ) const;
    ULONG           GetLinkRefreshDelay( SCTAB nTab ) const;
    void            SetLink( SCTAB nTab, BYTE nMode, const String& rDoc,
                            const String& rFilter, const String& rOptions,
                            const String& rTabName, ULONG nRefreshDelay );
    BOOL            HasLink( const String& rDoc,
                            const String& rFilter, const String& rOptions ) const;
    SC_DLLPUBLIC BOOL           LinkExternalTab( SCTAB& nTab, const String& aDocTab,
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

    BOOL            HasDdeLinks() const;
    BOOL            HasAreaLinks() const;
    void            UpdateExternalRefLinks();
    void            UpdateDdeLinks();
    void            UpdateAreaLinks();

                    // originating DDE links
    void            IncInDdeLinkUpdate() { if ( nInDdeLinkUpdate < 255 ) ++nInDdeLinkUpdate; }
    void            DecInDdeLinkUpdate() { if ( nInDdeLinkUpdate ) --nInDdeLinkUpdate; }
    BOOL            IsInDdeLinkUpdate() const   { return nInDdeLinkUpdate != 0; }

    SC_DLLPUBLIC void           CopyDdeLinks( ScDocument* pDestDoc ) const;
    void            DisconnectDdeLinks();

                    // for StarOne Api:
    USHORT          GetDdeLinkCount() const;
    BOOL            UpdateDdeLink( const String& rAppl, const String& rTopic, const String& rItem );

    /** Tries to find a DDE link with the specified connection data.
        @param rnDdePos  (out-param) Returns the index of the DDE link (does not include other links from link manager).
        @return  true = DDE link found, rnDdePos valid. */
    SC_DLLPUBLIC bool            FindDdeLink( const String& rAppl, const String& rTopic, const String& rItem, BYTE nMode, USHORT& rnDdePos );

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
    SC_DLLPUBLIC const ScMatrix* GetDdeLinkResultMatrix( USHORT nDdePos ) const;

    /** Tries to find a DDE link or creates a new, if not extant.
        @param pResults  If not 0, sets the matrix as as DDE link result matrix (also for existing links).
        @return  true = DDE link found; false = Unpredictable error occurred, no DDE link created. */
    SC_DLLPUBLIC bool            CreateDdeLink( const String& rAppl, const String& rTopic, const String& rItem, BYTE nMode, ScMatrixRef pResults );
    /** Sets a result matrix for the specified DDE link.
        @param nDdePos  Index of the DDE link (does not include other links from link manager).
        @param pResults  The array containing all results of the DDE link (intrusive-ref-counted, do not delete).
        @return  true = DDE link found and matrix set. */
    bool            SetDdeLinkResultMatrix( USHORT nDdePos, ScMatrixRef pResults );


    SfxBindings*    GetViewBindings();
    SfxObjectShell* GetDocumentShell() const    { return pShell; }
    ScDrawLayer*    GetDrawLayer()              { return pDrawLayer; }
    SfxBroadcaster* GetDrawBroadcaster();       // to avoid header
    void            BeginDrawUndo();

    void            BeginUnoRefUndo();
    bool            HasUnoRefUndo() const       { return ( pUnoRefUndoList != NULL ); }
    ScUnoRefList*   EndUnoRefUndo();            // must be deleted by caller!
    sal_Int64       GetNewUnoId();
    void            AddUnoRefChange( sal_Int64 nId, const ScRangeList& rOldRanges );

    sal_Bool IsChart( const SdrObject* pObject );

    SC_DLLPUBLIC void           UpdateAllCharts();
    void            UpdateChartRef( UpdateRefMode eUpdateRefMode,
                                    SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                                    SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                                    SCsCOL nDx, SCsROW nDy, SCsTAB nDz );
                    //! only assigns the new RangeList, no ChartListener or the like
    void            SetChartRangeList( const String& rChartName,
                        const ScRangeListRef& rNewRangeListRef );

    BOOL            HasControl( SCTAB nTab, const Rectangle& rMMRect );
    void            InvalidateControls( Window* pWin, SCTAB nTab, const Rectangle& rMMRect );

    void            StartAnimations( SCTAB nTab, Window* pWin );

    BOOL            HasBackgroundDraw( SCTAB nTab, const Rectangle& rMMRect );
    BOOL            HasAnyDraw( SCTAB nTab, const Rectangle& rMMRect );

    const ScSheetEvents* GetSheetEvents( SCTAB nTab ) const;
    void            SetSheetEvents( SCTAB nTab, const ScSheetEvents* pNew );
    bool            HasSheetEventScript( SCTAB nTab, sal_Int32 nEvent, bool bWithVbaEvents = false ) const;
    bool            HasAnySheetEventScript( sal_Int32 nEvent, bool bWithVbaEvents = false ) const;  // on any sheet

    BOOL            HasCalcNotification( SCTAB nTab ) const;
    void            SetCalcNotification( SCTAB nTab );
    void            ResetCalcNotifications();

    SC_DLLPUBLIC ScOutlineTable*    GetOutlineTable( SCTAB nTab, BOOL bCreate = FALSE );
    BOOL            SetOutlineTable( SCTAB nTab, const ScOutlineTable* pNewOutline );

    void            DoAutoOutline( SCCOL nStartCol, SCROW nStartRow,
                                    SCCOL nEndCol, SCROW nEndRow, SCTAB nTab );

    BOOL            DoSubTotals( SCTAB nTab, ScSubTotalParam& rParam );
    void            RemoveSubTotals( SCTAB nTab, ScSubTotalParam& rParam );
    BOOL            TestRemoveSubTotals( SCTAB nTab, const ScSubTotalParam& rParam );
    BOOL            HasSubTotalCells( const ScRange& rRange );

    SC_DLLPUBLIC void           PutCell( const ScAddress&, ScBaseCell* pCell, BOOL bForceTab = FALSE );
    SC_DLLPUBLIC void           PutCell( SCCOL nCol, SCROW nRow, SCTAB nTab, ScBaseCell* pCell,
                            BOOL bForceTab = FALSE );
    SC_DLLPUBLIC void           PutCell(SCCOL nCol, SCROW nRow, SCTAB nTab, ScBaseCell* pCell,
                            ULONG nFormatIndex, BOOL bForceTab = FALSE);
                    //  return TRUE = number format is set
    SC_DLLPUBLIC BOOL           SetString(
        SCCOL nCol, SCROW nRow, SCTAB nTab, const String& rString,
        ScSetStringParam* pParam = NULL );
    SC_DLLPUBLIC void           SetValue( SCCOL nCol, SCROW nRow, SCTAB nTab, const double& rVal );
    void            SetError( SCCOL nCol, SCROW nRow, SCTAB nTab, const USHORT nError);

    SC_DLLPUBLIC void           InsertMatrixFormula(SCCOL nCol1, SCROW nRow1,
                                        SCCOL nCol2, SCROW nRow2,
                                        const ScMarkData& rMark,
                                        const String& rFormula,
                                        const ScTokenArray* p = NULL,
                                        const formula::FormulaGrammar::Grammar = formula::FormulaGrammar::GRAM_DEFAULT );
    SC_DLLPUBLIC void           InsertTableOp(const ScTabOpParam& rParam,   // multi-operation
                                  SCCOL nCol1, SCROW nRow1,
                                  SCCOL nCol2, SCROW nRow2, const ScMarkData& rMark);

    SC_DLLPUBLIC void           GetString( SCCOL nCol, SCROW nRow, SCTAB nTab, String& rString );
    SC_DLLPUBLIC void           GetString( SCCOL nCol, SCROW nRow, SCTAB nTab, rtl::OUString& rString );
    SC_DLLPUBLIC void           GetInputString( SCCOL nCol, SCROW nRow, SCTAB nTab, String& rString );
    SC_DLLPUBLIC double         GetValue( const ScAddress& );
    SC_DLLPUBLIC void           GetValue( SCCOL nCol, SCROW nRow, SCTAB nTab, double& rValue );
    SC_DLLPUBLIC double         RoundValueAsShown( double fVal, ULONG nFormat );
    SC_DLLPUBLIC void           GetNumberFormat( SCCOL nCol, SCROW nRow, SCTAB nTab,
                                     sal_uInt32& rFormat ) const;
    sal_uInt32      GetNumberFormat( const ScRange& rRange ) const;
    SC_DLLPUBLIC sal_uInt32     GetNumberFormat( const ScAddress& ) const;
                    /** If no number format attribute is set and the cell
                        pointer passed is of type formula cell, the calculated
                        number format of the formula cell is returned. pCell
                        may be NULL. */
    SC_DLLPUBLIC void           GetNumberFormatInfo( short& nType, ULONG& nIndex,
                        const ScAddress& rPos, const ScBaseCell* pCell ) const;
    void            GetFormula( SCCOL nCol, SCROW nRow, SCTAB nTab, String& rFormula ) const;
    SC_DLLPUBLIC void           GetCellType( SCCOL nCol, SCROW nRow, SCTAB nTab, CellType& rCellType ) const;
    SC_DLLPUBLIC CellType       GetCellType( const ScAddress& rPos ) const;
    SC_DLLPUBLIC void           GetCell( SCCOL nCol, SCROW nRow, SCTAB nTab, ScBaseCell*& rpCell ) const;
    SC_DLLPUBLIC ScBaseCell*        GetCell( const ScAddress& rPos ) const;

    SC_DLLPUBLIC BOOL           HasData( SCCOL nCol, SCROW nRow, SCTAB nTab );
    SC_DLLPUBLIC BOOL           HasStringData( SCCOL nCol, SCROW nRow, SCTAB nTab ) const;
    SC_DLLPUBLIC BOOL           HasValueData( SCCOL nCol, SCROW nRow, SCTAB nTab ) const;
    BOOL            HasStringCells( const ScRange& rRange ) const;

    /** Returns true, if there is any data to create a selection list for rPos. */
    BOOL            HasSelectionData( SCCOL nCol, SCROW nRow, SCTAB nTab ) const;

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

    void            SetDrawPageSize(SCTAB nTab);

    BOOL            ExtendMergeSel( SCCOL nStartCol, SCROW nStartRow,
                                SCCOL& rEndCol, SCROW& rEndRow, const ScMarkData& rMark,
                                BOOL bRefresh = FALSE, BOOL bAttrs = FALSE );
    BOOL            ExtendMerge( SCCOL nStartCol, SCROW nStartRow,
                                SCCOL& rEndCol, SCROW& rEndRow, SCTAB nTab,
                                BOOL bRefresh = FALSE, BOOL bAttrs = FALSE );
    BOOL            ExtendMerge( ScRange& rRange, BOOL bRefresh = FALSE, BOOL bAttrs = FALSE );
    BOOL            ExtendTotalMerge( ScRange& rRange );
    SC_DLLPUBLIC BOOL           ExtendOverlapped( SCCOL& rStartCol, SCROW& rStartRow,
                                SCCOL nEndCol, SCROW nEndRow, SCTAB nTab );
    SC_DLLPUBLIC BOOL           ExtendOverlapped( ScRange& rRange );

    BOOL            RefreshAutoFilter( SCCOL nStartCol, SCROW nStartRow,
                                SCCOL nEndCol, SCROW nEndRow, SCTAB nTab );

    SC_DLLPUBLIC void           DoMergeContents( SCTAB nTab, SCCOL nStartCol, SCROW nStartRow,
                                    SCCOL nEndCol, SCROW nEndRow );
                    //  without checking:
    SC_DLLPUBLIC void           DoMerge( SCTAB nTab, SCCOL nStartCol, SCROW nStartRow,
                                    SCCOL nEndCol, SCROW nEndRow, bool bDeleteCaptions = true );
    void            RemoveMerge( SCCOL nCol, SCROW nRow, SCTAB nTab );

    BOOL            IsBlockEmpty( SCTAB nTab, SCCOL nStartCol, SCROW nStartRow,
                                                SCCOL nEndCol, SCROW nEndRow, bool bIgnoreNotes = false ) const;
    BOOL            IsPrintEmpty( SCTAB nTab, SCCOL nStartCol, SCROW nStartRow,
                                                SCCOL nEndCol, SCROW nEndRow,
                                                BOOL bLeftIsEmpty = FALSE,
                                                ScRange* pLastRange = NULL,
                                                Rectangle* pLastMM = NULL ) const;

    void            SkipOverlapped( SCCOL& rCol, SCROW& rRow, SCTAB nTab ) const;
    BOOL            IsHorOverlapped( SCCOL nCol, SCROW nRow, SCTAB nTab ) const;
    BOOL            IsVerOverlapped( SCCOL nCol, SCROW nRow, SCTAB nTab ) const;

    SC_DLLPUBLIC bool           HasAttrib( SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                                           SCCOL nCol2, SCROW nRow2, SCTAB nTab2, USHORT nMask );
    SC_DLLPUBLIC bool           HasAttrib( const ScRange& rRange, USHORT nMask );

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

                    // calculate automatically
    void            SetAutoCalc( BOOL bNewAutoCalc );
    BOOL            GetAutoCalc() const { return bAutoCalc; }
                    // calculate automatically in/from/for ScDocShell disabled
    void            SetAutoCalcShellDisabled( BOOL bNew ) { bAutoCalcShellDisabled = bNew; }
    BOOL            IsAutoCalcShellDisabled() const { return bAutoCalcShellDisabled; }
                    // ForcedFormulas are to be calculated
    void            SetForcedFormulaPending( BOOL bNew ) { bForcedFormulaPending = bNew; }
    BOOL            IsForcedFormulaPending() const { return bForcedFormulaPending; }
                    // if CalcFormulaTree() is currently running
    BOOL            IsCalculatingFormulaTree() { return bCalculatingFormulaTree; }

    USHORT          GetErrCode( const ScAddress& ) const;

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

    void            GetDataArea( SCTAB nTab, SCCOL& rStartCol, SCROW& rStartRow,
                                 SCCOL& rEndCol, SCROW& rEndRow, BOOL bIncludeOld, bool bOnlyDown ) const;
    SC_DLLPUBLIC BOOL           GetCellArea( SCTAB nTab, SCCOL& rEndCol, SCROW& rEndRow ) const;
    SC_DLLPUBLIC BOOL           GetTableArea( SCTAB nTab, SCCOL& rEndCol, SCROW& rEndRow ) const;
    SC_DLLPUBLIC BOOL           GetPrintArea( SCTAB nTab, SCCOL& rEndCol, SCROW& rEndRow,
                                    BOOL bNotes = TRUE ) const;
    SC_DLLPUBLIC BOOL           GetPrintAreaHor( SCTAB nTab, SCROW nStartRow, SCROW nEndRow,
                                        SCCOL& rEndCol, BOOL bNotes = TRUE ) const;
    SC_DLLPUBLIC BOOL           GetPrintAreaVer( SCTAB nTab, SCCOL nStartCol, SCCOL nEndCol,
                                        SCROW& rEndRow, BOOL bNotes = TRUE ) const;
    void            InvalidateTableArea();


    SC_DLLPUBLIC BOOL           GetDataStart( SCTAB nTab, SCCOL& rStartCol, SCROW& rStartRow ) const;

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

    SC_DLLPUBLIC void           FindAreaPos( SCCOL& rCol, SCROW& rRow, SCTAB nTab, SCsCOL nMovX, SCsROW nMovY );
    SC_DLLPUBLIC void           GetNextPos( SCCOL& rCol, SCROW& rRow, SCTAB nTab, SCsCOL nMovX, SCsROW nMovY,
                                BOOL bMarked, BOOL bUnprotected, const ScMarkData& rMark );

    BOOL            GetNextMarkedCell( SCCOL& rCol, SCROW& rRow, SCTAB nTab,
                                        const ScMarkData& rMark );

    void            LimitChartArea( SCTAB nTab, SCCOL& rStartCol, SCROW& rStartRow,
                                                    SCCOL& rEndCol, SCROW& rEndRow );
    void            LimitChartIfAll( ScRangeListRef& rRangeList );

    BOOL            InsertRow( SCCOL nStartCol, SCTAB nStartTab,
                               SCCOL nEndCol,   SCTAB nEndTab,
                               SCROW nStartRow, SCSIZE nSize, ScDocument* pRefUndoDoc = NULL,
                               const ScMarkData* pTabMark = NULL );
    SC_DLLPUBLIC BOOL           InsertRow( const ScRange& rRange, ScDocument* pRefUndoDoc = NULL );
    void            DeleteRow( SCCOL nStartCol, SCTAB nStartTab,
                               SCCOL nEndCol,   SCTAB nEndTab,
                               SCROW nStartRow, SCSIZE nSize,
                               ScDocument* pRefUndoDoc = NULL, BOOL* pUndoOutline = NULL,
                               const ScMarkData* pTabMark = NULL );
    SC_DLLPUBLIC void   DeleteRow( const ScRange& rRange,
                               ScDocument* pRefUndoDoc = NULL, BOOL* pUndoOutline = NULL );
    BOOL            InsertCol( SCROW nStartRow, SCTAB nStartTab,
                               SCROW nEndRow,   SCTAB nEndTab,
                               SCCOL nStartCol, SCSIZE nSize, ScDocument* pRefUndoDoc = NULL,
                               const ScMarkData* pTabMark = NULL );
    SC_DLLPUBLIC BOOL           InsertCol( const ScRange& rRange, ScDocument* pRefUndoDoc = NULL );
    void            DeleteCol( SCROW nStartRow, SCTAB nStartTab,
                               SCROW nEndRow, SCTAB nEndTab,
                               SCCOL nStartCol, SCSIZE nSize,
                               ScDocument* pRefUndoDoc = NULL, BOOL* pUndoOutline = NULL,
                               const ScMarkData* pTabMark = NULL );
    void            DeleteCol( const ScRange& rRange,
                               ScDocument* pRefUndoDoc = NULL, BOOL* pUndoOutline = NULL );

    BOOL            CanInsertRow( const ScRange& rRange ) const;
    BOOL            CanInsertCol( const ScRange& rRange ) const;

    void            FitBlock( const ScRange& rOld, const ScRange& rNew, BOOL bClear = TRUE );
    BOOL            CanFitBlock( const ScRange& rOld, const ScRange& rNew );

    BOOL            IsClipOrUndo() const                        { return bIsClip || bIsUndo; }
    BOOL            IsUndo() const                              { return bIsUndo; }
    BOOL            IsClipboard() const                         { return bIsClip; }
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
    void            SetCutMode( BOOL bCut );
    SC_DLLPUBLIC BOOL           IsCutMode();
    void            SetClipArea( const ScRange& rArea, BOOL bCut = FALSE );

    SC_DLLPUBLIC BOOL           IsDocVisible() const                        { return bIsVisible; }
    void            SetDocVisible( BOOL bSet );

    BOOL            HasOLEObjectsInArea( const ScRange& rRange, const ScMarkData* pTabMark = NULL );

    void            DeleteObjectsInArea( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                                        const ScMarkData& rMark );
    void            DeleteObjectsInSelection( const ScMarkData& rMark );

    void            DeleteArea(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                            const ScMarkData& rMark, USHORT nDelFlag);
    void            DeleteAreaTab(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                                SCTAB nTab, USHORT nDelFlag);
    void            DeleteAreaTab(const ScRange& rRange, USHORT nDelFlag);

    void            CopyToClip(const ScClipParam& rClipParam, ScDocument* pClipDoc,
                               const ScMarkData* pMarks = NULL, bool bAllTabs = false, bool bKeepScenarioFlags = false,
                               bool bIncludeObjects = false, bool bCloneNoteCaptions = true);

    void            CopyToClip4VBA(const ScClipParam& rClipParam, ScDocument* pClipDoc, bool bKeepScenarioFlags = false,
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
                                        const ScMarkData& rMark, USHORT nInsFlag );
    void            BroadcastFromClip( SCCOL nCol1, SCROW nRow1,
                                    SCCOL nCol2, SCROW nRow2,
                                    const ScMarkData& rMark, USHORT nInsFlag );
    /** If pDestRanges is given it overrides rDestRange, rDestRange in this
        case is the overall encompassing range. */
    void            CopyFromClip( const ScRange& rDestRange, const ScMarkData& rMark,
                                    USHORT nInsFlag,
                                    ScDocument* pRefUndoDoc = NULL,
                                    ScDocument* pClipDoc = NULL,
                                    BOOL bResetCut = TRUE,
                                    BOOL bAsLink = FALSE,
                                    BOOL bIncludeFiltered = TRUE,
                                    BOOL bSkipAttrForEmpty = FALSE,
                                    const ScRangeList * pDestRanges = NULL );

    void            CopyMultiRangeFromClip(const ScAddress& rDestPos, const ScMarkData& rMark,
                                           sal_uInt16 nInsFlag, ScDocument* pClipDoc,
                                           bool bResetCut = true, bool bAsLink = false,
                                           bool bIncludeFiltered = true,
                                           bool bSkipAttrForEmpty = false);

    void            GetClipArea(SCCOL& nClipX, SCROW& nClipY, BOOL bIncludeFiltered);
    void            GetClipStart(SCCOL& nClipX, SCROW& nClipY);

    BOOL            HasClipFilteredRows();

    BOOL            IsClipboardSource() const;

    SC_DLLPUBLIC void           TransposeClip( ScDocument* pTransClip, USHORT nFlags, BOOL bAsLink );

    ScClipParam&    GetClipParam();
    void            SetClipParam(const ScClipParam& rParam);

    void            MixDocument( const ScRange& rRange, USHORT nFunction, BOOL bSkipEmpty,
                                    ScDocument* pSrcDoc );

    void            FillTab( const ScRange& rSrcArea, const ScMarkData& rMark,
                                USHORT nFlags, USHORT nFunction,
                                BOOL bSkipEmpty, BOOL bAsLink );
    void            FillTabMarked( SCTAB nSrcTab, const ScMarkData& rMark,
                                USHORT nFlags, USHORT nFunction,
                                BOOL bSkipEmpty, BOOL bAsLink );

    void            TransliterateText( const ScMarkData& rMultiMark, sal_Int32 nType );

    SC_DLLPUBLIC void           InitUndo( ScDocument* pSrcDoc, SCTAB nTab1, SCTAB nTab2,
                                BOOL bColInfo = FALSE, BOOL bRowInfo = FALSE );
    void            AddUndoTab( SCTAB nTab1, SCTAB nTab2,
                                BOOL bColInfo = FALSE, BOOL bRowInfo = FALSE );
    SC_DLLPUBLIC void           InitUndoSelected( ScDocument* pSrcDoc, const ScMarkData& rTabSelection,
                                BOOL bColInfo = FALSE, BOOL bRowInfo = FALSE );

                    //  don't use anymore:
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
    SC_DLLPUBLIC const ScRangeList* GetScenarioRanges( SCTAB nTab ) const;

    SC_DLLPUBLIC void           CopyUpdated( ScDocument* pPosDoc, ScDocument* pDestDoc );

    void            UpdateReference( UpdateRefMode eUpdateRefMode, SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                                     SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                                     SCsCOL nDx, SCsROW nDy, SCsTAB nDz,
                                     ScDocument* pUndoDoc = NULL, BOOL bIncludeDraw = TRUE,
                                     bool bUpdateNoteCaptionPos = true );

    SC_DLLPUBLIC void           UpdateTranspose( const ScAddress& rDestPos, ScDocument* pClipDoc,
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
    SC_DLLPUBLIC const ScPatternAttr*   GetPattern( SCCOL nCol, SCROW nRow, SCTAB nTab ) const;
    SC_DLLPUBLIC const ScPatternAttr*    GetMostUsedPattern( SCCOL nCol, SCROW nStartRow, SCROW nEndRow, SCTAB nTab ) const;
    const ScPatternAttr*    GetSelectionPattern( const ScMarkData& rMark, BOOL bDeep = TRUE );
    ScPatternAttr*          CreateSelectionPattern( const ScMarkData& rMark, BOOL bDeep = TRUE );

    const ScConditionalFormat* GetCondFormat( SCCOL nCol, SCROW nRow, SCTAB nTab ) const;
    SC_DLLPUBLIC const SfxItemSet*  GetCondResult( SCCOL nCol, SCROW nRow, SCTAB nTab ) const;
    const SfxPoolItem*  GetEffItem( SCCOL nCol, SCROW nRow, SCTAB nTab, USHORT nWhich ) const;

    SC_DLLPUBLIC const ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XBreakIterator >& GetBreakIterator();
    BOOL            HasStringWeakCharacters( const String& rString );
    SC_DLLPUBLIC BYTE           GetStringScriptType( const String& rString );
    SC_DLLPUBLIC BYTE           GetCellScriptType( ScBaseCell* pCell, ULONG nNumberFormat );
    SC_DLLPUBLIC BYTE           GetScriptType( SCCOL nCol, SCROW nRow, SCTAB nTab, ScBaseCell* pCell = NULL );

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

    SC_DLLPUBLIC ULONG          AddCondFormat( const ScConditionalFormat& rNew );
    SC_DLLPUBLIC void           FindConditionalFormat( ULONG nKey, ScRangeList& rRanges );
    SC_DLLPUBLIC void           FindConditionalFormat( ULONG nKey, ScRangeList& rRanges, SCTAB nTab );
    void            ConditionalChanged( ULONG nKey );

    SC_DLLPUBLIC ULONG          AddValidationEntry( const ScValidationData& rNew );

    SC_DLLPUBLIC const ScValidationData*    GetValidationEntry( ULONG nIndex ) const;

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
                                        const ScMarkData& rMark, const ScPatternAttr& rAttr,
                                        ScEditDataArray* pDataArray = NULL );
    SC_DLLPUBLIC void           ApplyPatternAreaTab( SCCOL nStartCol, SCROW nStartRow,
                                            SCCOL nEndCol, SCROW nEndRow, SCTAB nTab,
                                            const ScPatternAttr& rAttr );

    SC_DLLPUBLIC bool SetAttrEntries(SCCOL nCol, SCTAB nTab, ScAttrEntry* pData, SCSIZE nSize);

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
                                            const SvxBorderLine* pLine, BOOL bColorOnly );

    const ScStyleSheet* GetStyle( SCCOL nCol, SCROW nRow, SCTAB nTab ) const;
    const ScStyleSheet* GetSelectionStyle( const ScMarkData& rMark ) const;

    void            StyleSheetChanged( const SfxStyleSheetBase* pStyleSheet, BOOL bRemoved,
                                        OutputDevice* pDev,
                                        double nPPTX, double nPPTY,
                                        const Fraction& rZoomX, const Fraction& rZoomY );

    BOOL            IsStyleSheetUsed( const ScStyleSheet& rStyle, BOOL bGatherAllStyles ) const;

    SC_DLLPUBLIC BOOL           ApplyFlagsTab( SCCOL nStartCol, SCROW nStartRow,
                                            SCCOL nEndCol, SCROW nEndRow,
                                            SCTAB nTab, INT16 nFlags );
    BOOL            RemoveFlagsTab( SCCOL nStartCol, SCROW nStartRow,
                                            SCCOL nEndCol, SCROW nEndRow,
                                            SCTAB nTab, INT16 nFlags );

    SC_DLLPUBLIC void           SetPattern( const ScAddress&, const ScPatternAttr& rAttr,
                                    BOOL bPutToPool = FALSE );
    SC_DLLPUBLIC void           SetPattern( SCCOL nCol, SCROW nRow, SCTAB nTab, const ScPatternAttr& rAttr,
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

                    // determine Col/Row of subsequent calls
                    // (e.g. not found from the beginning, or subsequent tables)
                    // meaning of explanation in "()" was already unclear in German
    static void     GetSearchAndReplaceStart( const SvxSearchItem& rSearchItem,
                        SCCOL& rCol, SCROW& rRow );

    BOOL            Solver(SCCOL nFCol, SCROW nFRow, SCTAB nFTab,
                            SCCOL nVCol, SCROW nVRow, SCTAB nVTab,
                            const String& sValStr, double& nX);

    void            ApplySelectionPattern( const ScPatternAttr& rAttr, const ScMarkData& rMark,
                                           ScEditDataArray* pDataArray = NULL );
    void            DeleteSelection( USHORT nDelFlag, const ScMarkData& rMark );
    void            DeleteSelectionTab( SCTAB nTab, USHORT nDelFlag, const ScMarkData& rMark );

    SC_DLLPUBLIC void           SetColWidth( SCCOL nCol, SCTAB nTab, USHORT nNewWidth );
    SC_DLLPUBLIC void           SetColWidthOnly( SCCOL nCol, SCTAB nTab, USHORT nNewWidth );
    SC_DLLPUBLIC void           SetRowHeight( SCROW nRow, SCTAB nTab, USHORT nNewHeight );
    SC_DLLPUBLIC void           SetRowHeightRange( SCROW nStartRow, SCROW nEndRow, SCTAB nTab,
                                            USHORT nNewHeight );

    SC_DLLPUBLIC void           SetRowHeightOnly( SCROW nStartRow, SCROW nEndRow, SCTAB nTab,
                                                  USHORT nNewHeight );
    SC_DLLPUBLIC void           SetManualHeight( SCROW nStartRow, SCROW nEndRow, SCTAB nTab, BOOL bManual );

    SC_DLLPUBLIC USHORT         GetColWidth( SCCOL nCol, SCTAB nTab ) const;
    SC_DLLPUBLIC USHORT         GetRowHeight( SCROW nRow, SCTAB nTab, bool bHiddenAsZero = true ) const;
    SC_DLLPUBLIC USHORT         GetRowHeight( SCROW nRow, SCTAB nTab, SCROW* pStartRow, SCROW* pEndRow, bool bHiddenAsZero = true ) const;
    SC_DLLPUBLIC ULONG          GetRowHeight( SCROW nStartRow, SCROW nEndRow, SCTAB nTab ) const;
    SCROW                       GetRowForHeight( SCTAB nTab, ULONG nHeight ) const;
    ULONG                       GetScaledRowHeight( SCROW nStartRow, SCROW nEndRow, SCTAB nTab, double fScale ) const;
    SC_DLLPUBLIC ULONG          GetColOffset( SCCOL nCol, SCTAB nTab ) const;
    SC_DLLPUBLIC ULONG          GetRowOffset( SCROW nRow, SCTAB nTab ) const;

    SC_DLLPUBLIC USHORT         GetOriginalWidth( SCCOL nCol, SCTAB nTab ) const;
    SC_DLLPUBLIC USHORT         GetOriginalHeight( SCROW nRow, SCTAB nTab ) const;

    USHORT          GetCommonWidth( SCCOL nEndCol, SCTAB nTab ) const;

    SCROW           GetHiddenRowCount( SCROW nRow, SCTAB nTab ) const;

    USHORT          GetOptimalColWidth( SCCOL nCol, SCTAB nTab, OutputDevice* pDev,
                                        double nPPTX, double nPPTY,
                                        const Fraction& rZoomX, const Fraction& rZoomY,
                                        BOOL bFormula,
                                        const ScMarkData* pMarkData = NULL,
                                        const ScColWidthParam* pParam = NULL );
    SC_DLLPUBLIC BOOL           SetOptimalHeight( SCROW nStartRow, SCROW nEndRow, SCTAB nTab, USHORT nExtra,
                                        OutputDevice* pDev,
                                        double nPPTX, double nPPTY,
                                        const Fraction& rZoomX, const Fraction& rZoomY,
                                        BOOL bShrink );
    void            UpdateAllRowHeights( OutputDevice* pDev,
                                        double nPPTX, double nPPTY,
                                        const Fraction& rZoomX, const Fraction& rZoomY,
                                        const ScMarkData* pTabMark = NULL );
    long            GetNeededSize( SCCOL nCol, SCROW nRow, SCTAB nTab,
                                    OutputDevice* pDev,
                                    double nPPTX, double nPPTY,
                                    const Fraction& rZoomX, const Fraction& rZoomY,
                                    BOOL bWidth, BOOL bTotalSize = FALSE );

    SC_DLLPUBLIC void           ShowCol(SCCOL nCol, SCTAB nTab, BOOL bShow);
    SC_DLLPUBLIC void           ShowRow(SCROW nRow, SCTAB nTab, BOOL bShow);
    SC_DLLPUBLIC void           ShowRows(SCROW nRow1, SCROW nRow2, SCTAB nTab, BOOL bShow);
    SC_DLLPUBLIC void           SetRowFlags( SCROW nRow, SCTAB nTab, BYTE nNewFlags );
    SC_DLLPUBLIC void           SetRowFlags( SCROW nStartRow, SCROW nEndRow, SCTAB nTab, BYTE nNewFlags );

    SC_DLLPUBLIC BYTE           GetColFlags( SCCOL nCol, SCTAB nTab ) const;
    SC_DLLPUBLIC BYTE           GetRowFlags( SCROW nRow, SCTAB nTab ) const;

    SC_DLLPUBLIC const ScBitMaskCompressedArray< SCROW, BYTE> & GetRowFlagsArray( SCTAB nTab ) const;
    SC_DLLPUBLIC       ScBitMaskCompressedArray< SCROW, BYTE> & GetRowFlagsArrayModifiable( SCTAB nTab );

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
    SC_DLLPUBLIC bool           HasHiddenRows(SCROW nStartRow, SCROW nEndRow, SCTAB nTab);
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

                    // if bCareManualSize is set then the row
                    // heights are compared only if the manual size flag for
                    // the row is set. If the bCareManualSize is not set then
                    // the row heights are always compared.
    SCROW           GetNextDifferentChangedRow( SCTAB nTab, SCROW nStart, bool bCareManualSize = true) const;

    // returns whether to export a Default style for this col/row or not
    // nDefault is setted to one possition in the current row/col where the Default style is
    BOOL            GetColDefault( SCTAB nTab, SCCOL nCol, SCROW nLastRow, SCROW& nDefault);
    BOOL            GetRowDefault( SCTAB nTab, SCROW nRow, SCCOL nLastCol, SCCOL& nDefault);

    BOOL            UpdateOutlineCol( SCCOL nStartCol, SCCOL nEndCol, SCTAB nTab, BOOL bShow );
    BOOL            UpdateOutlineRow( SCROW nStartRow, SCROW nEndRow, SCTAB nTab, BOOL bShow );

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
    BOOL            HasManualBreaks( SCTAB nTab ) const;

    BOOL            IsPageStyleInUse( const String& rStrPageStyle, SCTAB* pInTab = NULL );
    BOOL            RemovePageStyleInUse( const String& rStrPageStyle );
    BOOL            RenamePageStyleInUse( const String& rOld, const String& rNew );
    void            ModifyStyleSheet( SfxStyleSheetBase& rPageStyle,
                                      const SfxItemSet&  rChanges );

    void            PageStyleModified( SCTAB nTab, const String& rNewName );

    SC_DLLPUBLIC BOOL           NeedPageResetAfterTab( SCTAB nTab ) const;

    // Was stored in PageStyle previously. Now it exists for every table:
    SC_DLLPUBLIC BOOL           HasPrintRange();
    SC_DLLPUBLIC USHORT         GetPrintRangeCount( SCTAB nTab );
    SC_DLLPUBLIC const ScRange* GetPrintRange( SCTAB nTab, USHORT nPos );
    SC_DLLPUBLIC const ScRange* GetRepeatColRange( SCTAB nTab );
    SC_DLLPUBLIC const ScRange* GetRepeatRowRange( SCTAB nTab );
    /** Returns true, if the specified sheet is always printed. */
    BOOL            IsPrintEntireSheet( SCTAB nTab ) const;

    /** Removes all print ranges. */
    SC_DLLPUBLIC void            ClearPrintRanges( SCTAB nTab );
    /** Adds a new print ranges. */
    SC_DLLPUBLIC void            AddPrintRange( SCTAB nTab, const ScRange& rNew );
    /** Marks the specified sheet to be printed completely. Deletes old print ranges on the sheet! */
    SC_DLLPUBLIC void            SetPrintEntireSheet( SCTAB nTab );
    SC_DLLPUBLIC void           SetRepeatColRange( SCTAB nTab, const ScRange* pNew );
    SC_DLLPUBLIC void           SetRepeatRowRange( SCTAB nTab, const ScRange* pNew );
    ScPrintRangeSaver* CreatePrintRangeSaver() const;
    void            RestorePrintRanges( const ScPrintRangeSaver& rSaver );

    SC_DLLPUBLIC Rectangle      GetMMRect( SCCOL nStartCol, SCROW nStartRow,
                                           SCCOL nEndCol, SCROW nEndRow, SCTAB nTab ) const;
    SC_DLLPUBLIC ScRange            GetRange( SCTAB nTab, const Rectangle& rMMRect ) const;

    void            UpdStlShtPtrsFrmNms();
    void            StylesToNames();

    SC_DLLPUBLIC void           CopyStdStylesFrom( ScDocument* pSrcDoc );

    CharSet         GetSrcCharSet() const   { return eSrcSet; }
    ULONG           GetSrcVersion() const   { return nSrcVer; }
    SCROW           GetSrcMaxRow() const    { return nSrcMaxRow; }

    void            SetSrcCharSet( CharSet eNew )   { eSrcSet = eNew; }
    void            UpdateFontCharSet();

    void            FillInfo( ScTableInfo& rTabInfo, SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2,
                        SCTAB nTab, double nScaleX, double nScaleY,
                        BOOL bPageMode, BOOL bFormulaMode,
                        const ScMarkData* pMarkData = NULL );

    SC_DLLPUBLIC SvNumberFormatter* GetFormatTable() const;

    void            Sort( SCTAB nTab, const ScSortParam& rSortParam, BOOL bKeepQuery );
    SCSIZE          Query( SCTAB nTab, const ScQueryParam& rQueryParam, BOOL bKeepSub );
    SC_DLLPUBLIC BOOL           CreateQueryParam( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                                        SCTAB nTab, ScQueryParam& rQueryParam );
    void            GetUpperCellString(SCCOL nCol, SCROW nRow, SCTAB nTab, String& rStr);

    /** Update the dynamic end row position of a current data area. */
    void            UpdateDynamicEndRow(ScDBData& rDBData) const;

    BOOL            GetFilterEntries( SCCOL nCol, SCROW nRow, SCTAB nTab,
                                bool bFilter, TypedScStrCollection& rStrings, bool& rHasDates);
    SC_DLLPUBLIC BOOL           GetFilterEntriesArea( SCCOL nCol, SCROW nStartRow, SCROW nEndRow,
                                SCTAB nTab, TypedScStrCollection& rStrings, bool& rHasDates );
    BOOL            GetDataEntries( SCCOL nCol, SCROW nRow, SCTAB nTab,
                                TypedScStrCollection& rStrings, BOOL bLimit = FALSE );
    BOOL            GetFormulaEntries( TypedScStrCollection& rStrings );

    BOOL            HasAutoFilter( SCCOL nCol, SCROW nRow, SCTAB nTab );

    SC_DLLPUBLIC BOOL           HasColHeader( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                                    SCTAB nTab );
    SC_DLLPUBLIC BOOL           HasRowHeader( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                                    SCTAB nTab );

    SfxPrinter*     GetPrinter( BOOL bCreateIfNotExist = TRUE );
    void            SetPrinter( SfxPrinter* pNewPrinter );
    VirtualDevice*  GetVirtualDevice_100th_mm();
    SC_DLLPUBLIC OutputDevice*  GetRefDevice(); // WYSIWYG: Printer, otherwise VirtualDevice...

    void            EraseNonUsedSharedNames(USHORT nLevel);
    BOOL            GetNextSpellingCell(SCCOL& nCol, SCROW& nRow, SCTAB nTab,
                                        BOOL bInSel, const ScMarkData& rMark) const;

    BOOL            ReplaceStyle(const SvxSearchItem& rSearchItem,
                                 SCCOL nCol, SCROW nRow, SCTAB nTab,
                                 ScMarkData& rMark, BOOL bIsUndo);

    void            DoColResize( SCTAB nTab, SCCOL nCol1, SCCOL nCol2, SCSIZE nAdd );

    void            InvalidateTextWidth( const String& rStyleName );
    void            InvalidateTextWidth( SCTAB nTab );
    void            InvalidateTextWidth( const ScAddress* pAdrFrom, const ScAddress* pAdrTo, BOOL bNumFormatChanged );

    BOOL            IdleCalcTextWidth();
    BOOL            IdleCheckLinks();

    BOOL            ContinueOnlineSpelling();   // TRUE = found s.th.

    BOOL            IsIdleDisabled() const      { return bIdleDisabled; }
    void            DisableIdle(BOOL bDo)       { bIdleDisabled = bDo; }

    BOOL            IsDetectiveDirty() const     { return bDetectiveDirty; }
    void            SetDetectiveDirty(BOOL bSet) { bDetectiveDirty = bSet; }

    void            RemoveAutoSpellObj();
    void            SetOnlineSpellPos( const ScAddress& rPos );
    SC_DLLPUBLIC BOOL           SetVisibleSpellRange( const ScRange& rRange );  // TRUE = changed

    BYTE            GetMacroCallMode() const     { return nMacroCallMode; }
    void            SetMacroCallMode(BYTE nNew)  { nMacroCallMode = nNew; }

    BOOL            GetHasMacroFunc() const      { return bHasMacroFunc; }
    void            SetHasMacroFunc(BOOL bSet)   { bHasMacroFunc = bSet; }

    BOOL            CheckMacroWarn();

    void            SetRangeOverflowType(sal_uInt32 nType)  { nRangeOverflowType = nType; }
    sal_Bool        HasRangeOverflow() const                { return nRangeOverflowType != 0; }
    SC_DLLPUBLIC sal_uInt32      GetRangeOverflowType() const            { return nRangeOverflowType; }

    // for broadcasting/listening
    void            SetNoSetDirty( BOOL bVal ) { bNoSetDirty = bVal; }
    BOOL            GetNoSetDirty() const { return bNoSetDirty; }
    void            SetInsertingFromOtherDoc( BOOL bVal ) { bInsertingFromOtherDoc = bVal; }
    BOOL            IsInsertingFromOtherDoc() const { return bInsertingFromOtherDoc; }
    bool            IsLoadingMedium() const;
    void            SetLoadingMedium( bool bVal );
    void            SetImportingXML( bool bVal );
    bool            IsImportingXML() const { return bImportingXML; }
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
    SC_DLLPUBLIC void           UpdateChartListenerCollection();
    BOOL            IsChartListenerCollectionNeedsUpdate() const
                        { return bChartListenerCollectionNeedsUpdate; }
    void            SetChartListenerCollectionNeedsUpdate( BOOL bFlg )
                        { bChartListenerCollectionNeedsUpdate = bFlg; }
    void            AddOLEObjectToCollection(const String& rName);

    ScChangeViewSettings* GetChangeViewSettings() const     { return pChangeViewSettings; }
    SC_DLLPUBLIC void               SetChangeViewSettings(const ScChangeViewSettings& rNew);

    rtl::Reference<SvxForbiddenCharactersTable> GetForbiddenCharacters();
    void            SetForbiddenCharacters( const rtl::Reference<SvxForbiddenCharactersTable> xNew );

    BYTE            GetAsianCompression() const;        // CharacterCompressionType values
    BOOL            IsValidAsianCompression() const;
    void            SetAsianCompression(BYTE nNew);

    BOOL            GetAsianKerning() const;
    BOOL            IsValidAsianKerning() const;
    void            SetAsianKerning(BOOL bNew);

    BYTE            GetEditTextDirection(SCTAB nTab) const; // EEHorizontalTextDirection values

    SC_DLLPUBLIC ScLkUpdMode        GetLinkMode() const             { return eLinkMode ;}
    void            SetLinkMode( ScLkUpdMode nSet ) {   eLinkMode  = nSet;}

    SC_DLLPUBLIC ScMacroManager* GetMacroManager();

private:
    ScDocument(const ScDocument& r); // disabled with no definition

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
    SC_DLLPUBLIC                         rHint.GetCell()->Broadcast() and AreaBroadcast()
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

    SC_DLLPUBLIC void               IncSizeRecalcLevel( SCTAB nTab );
    SC_DLLPUBLIC void               DecSizeRecalcLevel( SCTAB nTab, bool bUpdateNoteCaptionPos = true );

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

    void            SetInLinkUpdate(BOOL bSet);             // TableLink or AreaLink
    BOOL            IsInLinkUpdate() const;                 // including DdeLink

    SC_DLLPUBLIC SfxItemPool*       GetEditPool() const;
    SC_DLLPUBLIC SfxItemPool*       GetEnginePool() const;
    SC_DLLPUBLIC ScFieldEditEngine& GetEditEngine();
    SC_DLLPUBLIC ScNoteEditEngine&  GetNoteEngine();

    ScRefreshTimerControl*  GetRefreshTimerControl() const
        { return pRefreshTimerControl; }
    ScRefreshTimerControl * const * GetRefreshTimerControlAddress() const
        { return &pRefreshTimerControl; }

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

    void AddSubTotalCell(ScFormulaCell* pCell);
    void RemoveSubTotalCell(ScFormulaCell* pCell);
    void SetSubTotalCellsDirty(const ScRange& rDirtyRange);

private: // CLOOK-Impl-methods

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

    void    ImplCreateOptions(); // Suggestion: switch to on-demand?
    void    ImplDeleteOptions();

    void    DeleteDrawLayer();
    void    DeleteColorTable();
    SC_DLLPUBLIC BOOL   DrawGetPrintArea( ScRange& rRange, BOOL bSetHor, BOOL bSetVer ) const;
    void    DrawMovePage( USHORT nOldPos, USHORT nNewPos );
    void    DrawCopyPage( USHORT nOldPos, USHORT nNewPos );

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
    void    CopyRangeNamesToClip(ScDocument* pClipDoc, const ScRange& rClipRange, SCTAB nTab);
    void    CopyRangeNamesFromClip(ScDocument* pClipDoc, ScClipRangeNameData& rRangeNames);
    void    UpdateRangeNamesInFormulas(
        ScClipRangeNameData& rRangeNames, const ScRangeList& rDestRanges, const ScMarkData& rMark,
        SCCOL nXw, SCROW nYw);

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

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
