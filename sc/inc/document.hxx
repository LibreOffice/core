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
#include "rangenam.hxx"
#include "brdcst.hxx"
#include "tabopparams.hxx"
#include "sortparam.hxx"
#include "types.hxx"
#include "formula/grammar.hxx"
#include <com/sun/star/chart2/XChartDocument.hpp>
#include "scdllapi.h"
#include "typedstrdata.hxx"
#include "compressedarray.hxx"
#include <tools/fract.hxx>
#include <tools/gen.hxx>

#include <memory>
#include <map>
#include <set>
#include <vector>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/scoped_ptr.hpp>

namespace editeng { class SvxBorderLine; }

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
class SfxStyleSheetBase;
class SvMemoryStream;
class SvNumberFormatter;
class SvxBoxInfoItem;
class SvxBoxItem;
class SvxForbiddenCharactersTable;
namespace sfx2 {
    class LinkManager;
    }
class SvxSearchItem;
class Window;
class XColorList;

struct ScAttrEntry;
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
class ScChangeTrack;
class ScEditEngineDefaulter;
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
class ScRowBreakIterator;
struct ScSetStringParam;
class ScDocRowHeightUpdater;
struct ScColWidthParam;
struct ScCopyBlockFromClipParams;
class ScSheetEvents;
class ScProgress;
class SvtListener;
class ScNotes;
class ScEditDataArray;

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
    rtl::OUString  aDocName;
    SCTAB   nTableCount;
    sal_uLong   nCellCount;
    sal_uInt16  nPageCount;
};

// DDE link modes
const sal_uInt8 SC_DDE_DEFAULT       = 0;
const sal_uInt8 SC_DDE_ENGLISH       = 1;
const sal_uInt8 SC_DDE_TEXT          = 2;
const sal_uInt8 SC_DDE_IGNOREMODE    = 255;       /// For usage in FindDdeLink() only!

class ScDocument
{
friend class ScDocumentIterator;
friend class ScValueIterator;
friend class ScHorizontalValueIterator;
friend class ScDBQueryDataIterator;
friend class ScCellIterator;
friend class ScQueryCellIterator;
friend class ScHorizontalCellIterator;
friend class ScHorizontalAttrIterator;
friend class ScDocAttrIterator;
friend class ScAttrRectIterator;
friend class ScDocShell;
friend class ScDocRowHeightUpdater;

    typedef ::std::vector<ScTable*> TableContainer;
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceManager;

    rtl::Reference<ScPoolHelper> xPoolHelper;

    SfxUndoManager*     mpUndoManager;
    ScFieldEditEngine*  pEditEngine;                    // uses pEditPool from xPoolHelper
    ScNoteEditEngine*   pNoteEngine;                    // uses pEditPool from xPoolHelper
    SfxObjectShell*     pShell;
    SfxPrinter*         pPrinter;
    VirtualDevice*      pVirtualDevice_100th_mm;
    ScDrawLayer*        pDrawLayer;                     // SdrModel
    rtl::Reference<XColorList> pColorList;
    ScValidationDataList* pValidationList;              // validity
    SvNumberFormatterIndexTable*    pFormatExchangeList;    // for application of number formats
    TableContainer maTabs;
    std::vector<rtl::OUString> maTabNames;               // for undo document, we need the information tab name <-> index
    mutable ScRangeName* pRangeName;
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

    rtl::OUString       aDocName;                       // optional: name of document
    rtl::OUString       aDocCodeName;                       // optional: name of document (twice?)
    rtl::OUString       maFileURL;      // file URL for copy & paste
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

    sal_uLong               nFormulaCodeInTree;             // FormelRPN im Formelbaum
    sal_uLong               nXMLImportedFormulaCount;        // progress count during XML import
    sal_uInt16              nInterpretLevel;                // >0 if in interpreter
    sal_uInt16              nMacroInterpretLevel;           // >0 if macro in interpreter
    sal_uInt16              nInterpreterTableOpLevel;       // >0 if in interpreter TableOp
    SCTAB               nMaxTableNumber;
    sal_uInt16              nSrcVer;                        // file version (load/save)
    SCROW               nSrcMaxRow;                     // number of lines to load/save
    sal_uInt16              nFormulaTrackCount;
    bool                bHardRecalcState;               // false: soft, true: hard
    SCTAB               nVisibleTab;                    // for OLE etc.

    ScLkUpdMode         eLinkMode;

    bool                bAutoCalc;                      // calculate automatically
    bool                bAutoCalcShellDisabled;         // in/from/for ScDocShell disabled
    // are there ForcedFormulas which have to be calculated
    // in interaction with ScDocShell SetDocumentModified,
    // AutoCalcShellDisabled and TrackFormulas
    bool                bForcedFormulaPending;
    bool                bCalculatingFormulaTree;
    bool                bIsClip;
    bool                bIsUndo;
    bool                bIsVisible;                     // set from view ctor

    bool                bIsEmbedded;                    // display/adjust Embedded area?

    // no SetDirty for ScFormulaCell::CompileTokenArray but at the end of
    // ScDocument::CompileAll[WithFormats], CopyScenario, CopyBlockFromClip
    bool                bNoSetDirty;
    // no broadcast, construct no listener during insert from a different
    // Doc (per filter or the like ), not until CompileAll / CalcAfterLoad
    bool                bInsertingFromOtherDoc;
    bool                bLoadingMedium;
    bool                bImportingXML;      // special handling of formula text
    bool                bXMLFromWrapper;    // distinguish ScXMLImportWrapper from external component
    bool                bCalcingAfterLoad;              // in CalcAfterLoad TRUE
    // don't construct/destruct listeners temporarily
    bool                bNoListening;
    bool                bIdleDisabled;
    bool                bInLinkUpdate;                  // TableLink or AreaLink
    bool                bChartListenerCollectionNeedsUpdate;
    // are/were there RC_FORCED formula cells in the document (if set once to TRUE then set forever)
    bool                bHasForcedFormulas;
    // is the Doc being destroyed? (no Notify-Tracking etc. needed anymore)
    bool                bInDtorClear;
    // expand reference if insert column/row takes place at the border
    // of a reference
    // is fetched in each UpdateReference from InputOptions,
    // assigned, and restored at the end of UpdateReference
    bool                bExpandRefs;
    // for detective update, is set for each change of a formula
    bool                bDetectiveDirty;

    sal_uInt8               nMacroCallMode;     // Macros per warning dialog disabled?
    bool                bHasMacroFunc;      // valid only after loading

    sal_uInt8               nVisSpellState;

    sal_uInt8               nAsianCompression;
    sal_uInt8               nAsianKerning;
    bool                bSetDrawDefaults;

    bool                bPastingDrawFromOtherDoc;

    sal_uInt8                nInDdeLinkUpdate;   // originating DDE links (stacked bool)

    bool                bInUnoBroadcast;
    bool                bInUnoListenerCall;
    formula::FormulaGrammar::Grammar  eGrammar;

    mutable bool        bStyleSheetUsageInvalid;

    bool                mbUndoEnabled;
    bool                mbAdjustHeightEnabled;
    bool                mbExecuteLinkEnabled;
    bool                mbChangeReadOnlyEnabled;    // allow changes in read-only document (for API import filters)
    bool                mbStreamValidLocked;

    sal_Int16           mnNamedRangesLockCount;

    ::std::set<ScFormulaCell*> maSubTotalCells;

    // quick and ugly hack to fix the ScEditableTester problem in ucalc
    // write a clean fix for this as soon as possible
    bool                mbIsInTest;

public:
    SC_DLLPUBLIC sal_uLong          GetCellCount() const;       // all cells
    SCSIZE          GetCellCount(SCTAB nTab, SCCOL nCol) const;
    sal_uLong           GetCodeCount() const;       // RPN-Code in formulas
    DECL_LINK( GetUserDefinedColor, sal_uInt16 * );
                                                                // number formatter

public:
    SC_DLLPUBLIC                ScDocument( ScDocumentMode eMode = SCDOCMODE_DOCUMENT,
                                SfxObjectShell* pDocShell = NULL );
    SC_DLLPUBLIC                ~ScDocument();

    inline ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                    GetServiceManager() const { return xServiceManager; }

    SC_DLLPUBLIC const rtl::OUString&  GetName() const { return aDocName; }
    void            SetName( const rtl::OUString& r ) { aDocName = r; }
    const rtl::OUString&   GetCodeName() const { return aDocCodeName; }
    void                SetCodeName( const rtl::OUString& r ) { aDocCodeName = r; }
    const rtl::OUString& GetFileURL() const { return maFileURL; }

    void            GetDocStat( ScDocStat& rDocStat );

    SC_DLLPUBLIC void  InitDrawLayer( SfxObjectShell* pDocShell = NULL );
    rtl::Reference<XColorList>          GetColorList();

    SC_DLLPUBLIC sfx2::LinkManager*     GetLinkManager() const;

    SC_DLLPUBLIC const ScDocOptions&        GetDocOptions() const;
    SC_DLLPUBLIC void                   SetDocOptions( const ScDocOptions& rOpt );
    SC_DLLPUBLIC const ScViewOptions&   GetViewOptions() const;
    SC_DLLPUBLIC void                   SetViewOptions( const ScViewOptions& rOpt );
    void                    SetPrintOptions();

    ScExtDocOptions*        GetExtDocOptions()  { return pExtDocOptions; }
    SC_DLLPUBLIC void                   SetExtDocOptions( ScExtDocOptions* pNewOptions );

    SC_DLLPUBLIC void       GetLanguage( LanguageType& rLatin, LanguageType& rCjk, LanguageType& rCtl ) const;
    void                    SetLanguage( LanguageType eLatin, LanguageType eCjk, LanguageType eCtl );

    void                    SetDrawDefaults();

    void                        SetConsolidateDlgData( const ScConsolidateParam* pData );
    const ScConsolidateParam*   GetConsolidateDlgData() const { return pConsolidateDlgData; }

    void            Clear( bool bFromDestructor = false );

    ScFieldEditEngine*  CreateFieldEditEngine();
    void                DisposeFieldEditEngine(ScFieldEditEngine*& rpEditEngine);

    /**
     * Get all range names that are local to each table.  It only returns
     * non-empty range name set.
     */
    SC_DLLPUBLIC void GetAllTabRangeNames(ScRangeName::TabNameCopyMap& rRangeNames) const;
    SC_DLLPUBLIC void SetAllRangeNames( const boost::ptr_map<rtl::OUString, ScRangeName>& rRangeMap);
    SC_DLLPUBLIC void GetTabRangeNameMap(std::map<rtl::OUString, ScRangeName*>& rRangeName);
    SC_DLLPUBLIC void GetRangeNameMap(std::map<rtl::OUString, ScRangeName*>& rRangeName);
    SC_DLLPUBLIC ScRangeName* GetRangeName(SCTAB nTab) const;
    SC_DLLPUBLIC ScRangeName* GetRangeName() const;
    void SetRangeName(SCTAB nTab, ScRangeName* pNew);
    void SetRangeName( ScRangeName* pNewRangeName );
    SCTAB           GetMaxTableNumber() { return static_cast<SCTAB>(maTabs.size()) - 1; }
    void            SetMaxTableNumber(SCTAB nNumber) { nMaxTableNumber = nNumber; }

    ScRangePairList*    GetColNameRanges() { return &xColNameRanges; }
    ScRangePairList*    GetRowNameRanges() { return &xRowNameRanges; }
    ScRangePairListRef& GetColNameRangesRef() { return xColNameRanges; }
    ScRangePairListRef& GetRowNameRangesRef() { return xRowNameRanges; }

    SC_DLLPUBLIC ScDBCollection*    GetDBCollection() const;
    void            SetDBCollection( ScDBCollection* pNewDBCollection,
                                        bool bRemoveAutoFilter = false );
    const ScDBData* GetDBAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab, bool bStartOnly = false) const;
    ScDBData* GetDBAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab, bool bStartOnly = false);
    const ScDBData* GetDBAtArea(SCTAB nTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2) const;
    ScDBData* GetDBAtArea(SCTAB nTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2);

    SC_DLLPUBLIC const ScRangeData* GetRangeAtBlock( const ScRange& rBlock, rtl::OUString* pName=NULL ) const;

    SC_DLLPUBLIC ScDPCollection*        GetDPCollection();
    ScDPObject*         GetDPAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab) const;
    ScDPObject*         GetDPAtBlock( const ScRange& rBlock ) const;

    SC_DLLPUBLIC ScChartCollection* GetChartCollection() const;

    void                StopTemporaryChartLock();

    void            EnsureGraphicNames();

    SdrObject*      GetObjectAtPoint( SCTAB nTab, const Point& rPos );
    bool            HasChartAtPoint( SCTAB nTab, const Point& rPos, rtl::OUString& rName );

    ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartDocument > GetChartByName( const rtl::OUString& rChartName );
    SC_DLLPUBLIC void            GetChartRanges( const rtl::OUString& rChartName, ::std::vector< ScRangeList >& rRanges, ScDocument* pSheetNameDoc );
    void            SetChartRanges( const rtl::OUString& rChartName, const ::std::vector< ScRangeList >& rRanges );

    void            UpdateChartArea( const rtl::OUString& rChartName, const ScRange& rNewArea,
                                        bool bColHeaders, bool bRowHeaders, bool bAdd );
    void            UpdateChartArea( const rtl::OUString& rChartName,
                                    const ScRangeListRef& rNewList,
                                    bool bColHeaders, bool bRowHeaders, bool bAdd );
    void            GetOldChartParameters( const rtl::OUString& rName,
                                    ScRangeList& rRanges, bool& rColHeaders, bool& rRowHeaders );
    ::com::sun::star::uno::Reference<
            ::com::sun::star::embed::XEmbeddedObject >
                    FindOleObjectByName( const rtl::OUString& rName );

    SC_DLLPUBLIC void           MakeTable( SCTAB nTab,bool _bNeedsNameCheck = true );

    SCTAB           GetVisibleTab() const       { return nVisibleTab; }
    SC_DLLPUBLIC void           SetVisibleTab(SCTAB nTab)   { nVisibleTab = nTab; }

    SC_DLLPUBLIC bool HasTable( SCTAB nTab ) const;
    SC_DLLPUBLIC bool GetName( SCTAB nTab, rtl::OUString& rName ) const;
    SC_DLLPUBLIC bool GetCodeName( SCTAB nTab, rtl::OUString& rName ) const;
    SC_DLLPUBLIC bool SetCodeName( SCTAB nTab, const rtl::OUString& rName );
    SC_DLLPUBLIC bool GetTable( const rtl::OUString& rName, SCTAB& rTab ) const;
    rtl::OUString   GetCopyTabName(SCTAB nTab) const;

    SC_DLLPUBLIC void SetAnonymousDBData(SCTAB nTab, ScDBData* pDBData);
    SC_DLLPUBLIC ScDBData* GetAnonymousDBData(SCTAB nTab);

    SC_DLLPUBLIC inline SCTAB   GetTableCount() const { return static_cast<SCTAB>(maTabs.size()); }
    SvNumberFormatterIndexTable* GetFormatExchangeList() const { return pFormatExchangeList; }

    SC_DLLPUBLIC ScDocProtection* GetDocProtection() const;
    SC_DLLPUBLIC void           SetDocProtection(const ScDocProtection* pProtect);
    SC_DLLPUBLIC bool           IsDocProtected() const;
    bool            IsDocEditable() const;
    SC_DLLPUBLIC bool           IsTabProtected( SCTAB nTab ) const;
    SC_DLLPUBLIC    ScTableProtection* GetTabProtection( SCTAB nTab ) const;
    SC_DLLPUBLIC void SetTabProtection(SCTAB nTab, const ScTableProtection* pProtect);
    void            CopyTabProtection(SCTAB nTabSrc, SCTAB nTabDest);

    void            LockTable(SCTAB nTab);
    void            UnlockTable(SCTAB nTab);

    bool            IsBlockEditable( SCTAB nTab, SCCOL nStartCol, SCROW nStartRow,
                                        SCCOL nEndCol, SCROW nEndRow,
                                        bool* pOnlyNotBecauseOfMatrix = NULL ) const;
    bool            IsSelectionEditable( const ScMarkData& rMark,
                                        bool* pOnlyNotBecauseOfMatrix = NULL ) const;
    bool            HasSelectedBlockMatrixFragment( SCCOL nStartCol, SCROW nStartRow,
                                            SCCOL nEndCol, SCROW nEndRow,
                                            const ScMarkData& rMark ) const;
    bool            HasSelectedBlockMatrixFragment( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow, SCTAB nTAB ) const;

    bool            GetMatrixFormulaRange( const ScAddress& rCellPos, ScRange& rMatrix );

    bool            IsEmbedded() const;
    void            GetEmbedded( ScRange& rRange ) const;
    void            SetEmbedded( const ScRange& rRange );
    void            ResetEmbedded();
    Rectangle       GetEmbeddedRect() const;                        // 1/100 mm
    void            SetEmbedded( const Rectangle& rRect );          // from VisArea (1/100 mm)
    void            SnapVisArea( Rectangle& rRect ) const;          // 1/100 mm

    static SC_DLLPUBLIC bool ValidTabName( const rtl::OUString& rName );

    SC_DLLPUBLIC bool           ValidNewTabName( const rtl::OUString& rName ) const;
    SC_DLLPUBLIC void           CreateValidTabName(rtl::OUString& rName) const;
    SC_DLLPUBLIC void           CreateValidTabNames(std::vector<rtl::OUString>& aNames, SCTAB nCount) const;

    void AppendTabOnLoad(const rtl::OUString& rName);
    void SetTabNameOnLoad(SCTAB nTab, const rtl::OUString& rName);

    SC_DLLPUBLIC bool           InsertTab( SCTAB nPos, const rtl::OUString& rName,
                                        bool bExternalDocument = false );
    SC_DLLPUBLIC bool           InsertTabs( SCTAB nPos, const std::vector<rtl::OUString>& rNames,
                                bool bExternalDocument = false, bool bNamesValid = false );
    SC_DLLPUBLIC bool           DeleteTabs( SCTAB nTab, SCTAB nSheets, ScDocument* pRefUndoDoc = NULL );
    SC_DLLPUBLIC bool           DeleteTab( SCTAB nTab, ScDocument* pRefUndoDoc = NULL );
    SC_DLLPUBLIC bool           RenameTab( SCTAB nTab, const rtl::OUString& rName,
                                bool bUpdateRef = true,
                                bool bExternalDocument = false );
    bool                        MoveTab( SCTAB nOldPos, SCTAB nNewPos, ScProgress* pProgress = NULL );
    bool                        CopyTab( SCTAB nOldPos, SCTAB nNewPos,
                                const ScMarkData* pOnlyMarked = NULL );
    SC_DLLPUBLIC sal_uLong          TransferTab(ScDocument* pSrcDoc, SCTAB nSrcPos, SCTAB nDestPos,
                                    bool bInsertNew = true,
                                    bool bResultsOnly = false );
    SC_DLLPUBLIC void           TransferDrawPage(ScDocument* pSrcDoc, SCTAB nSrcPos, SCTAB nDestPos);
    SC_DLLPUBLIC void           SetVisible( SCTAB nTab, bool bVisible );
    SC_DLLPUBLIC bool           IsVisible( SCTAB nTab ) const;
    bool            IsStreamValid( SCTAB nTab ) const;
    void            SetStreamValid( SCTAB nTab, bool bSet, bool bIgnoreLock = false );
    void            LockStreamValid( bool bLock );
    bool            IsStreamValidLocked() const                         { return mbStreamValidLocked; }
    bool            IsPendingRowHeights( SCTAB nTab ) const;
    void            SetPendingRowHeights( SCTAB nTab, bool bSet );
    SC_DLLPUBLIC void           SetLayoutRTL( SCTAB nTab, bool bRTL );
    SC_DLLPUBLIC bool           IsLayoutRTL( SCTAB nTab ) const;
    bool            IsNegativePage( SCTAB nTab ) const;
    SC_DLLPUBLIC void           SetScenario( SCTAB nTab, bool bFlag );
    SC_DLLPUBLIC bool           IsScenario( SCTAB nTab ) const;
    SC_DLLPUBLIC void GetScenarioData( SCTAB nTab, rtl::OUString& rComment,
                                       Color& rColor, sal_uInt16& rFlags ) const;
    SC_DLLPUBLIC void           SetScenarioData( SCTAB nTab, const rtl::OUString& rComment,
                                        const Color& rColor, sal_uInt16 nFlags );
    SC_DLLPUBLIC Color GetTabBgColor( SCTAB nTab ) const;
    SC_DLLPUBLIC void SetTabBgColor( SCTAB nTab, const Color& rColor );
    SC_DLLPUBLIC bool IsDefaultTabBgColor( SCTAB nTab ) const;
    void            GetScenarioFlags( SCTAB nTab, sal_uInt16& rFlags ) const;
    SC_DLLPUBLIC bool           IsActiveScenario( SCTAB nTab ) const;
    SC_DLLPUBLIC void           SetActiveScenario( SCTAB nTab, bool bActive );      // only for Undo etc.
    SC_DLLPUBLIC formula::FormulaGrammar::AddressConvention GetAddressConvention() const;
    SC_DLLPUBLIC formula::FormulaGrammar::Grammar GetGrammar() const;
    void            SetGrammar( formula::FormulaGrammar::Grammar eGram );
    SC_DLLPUBLIC sal_uInt8          GetLinkMode( SCTAB nTab ) const;
    bool            IsLinked( SCTAB nTab ) const;
    SC_DLLPUBLIC const rtl::OUString  GetLinkDoc( SCTAB nTab ) const;
    const rtl::OUString   GetLinkFlt( SCTAB nTab ) const;
    const rtl::OUString   GetLinkOpt( SCTAB nTab ) const;
    SC_DLLPUBLIC const rtl::OUString  GetLinkTab( SCTAB nTab ) const;
    sal_uLong       GetLinkRefreshDelay( SCTAB nTab ) const;
    void            SetLink( SCTAB nTab, sal_uInt8 nMode, const rtl::OUString& rDoc,
                            const rtl::OUString& rFilter, const rtl::OUString& rOptions,
                            const rtl::OUString& rTabName, sal_uLong nRefreshDelay );
    bool            HasLink( const rtl::OUString& rDoc,
                            const rtl::OUString& rFilter, const rtl::OUString& rOptions ) const;
    SC_DLLPUBLIC bool           LinkExternalTab( SCTAB& nTab, const rtl::OUString& aDocTab,
                                    const rtl::OUString& aFileName,
                                    const rtl::OUString& aTabName );

    bool            HasExternalRefManager() const { return pExternalRefMgr.get(); }
    SC_DLLPUBLIC ScExternalRefManager* GetExternalRefManager() const;
    bool            IsInExternalReferenceMarking() const;
    void            MarkUsedExternalReferences();
    bool            MarkUsedExternalReferences( ScTokenArray & rArr );

    /** Returns the pool containing external formula parsers. Creates the pool
        on first call. */
    ScFormulaParserPool& GetFormulaParserPool() const;

    bool            HasDdeLinks() const;
    bool            HasAreaLinks() const;
    void            UpdateExternalRefLinks(Window* pWin);
    void            UpdateDdeLinks(Window* pWin);
    void            UpdateAreaLinks();

                    // originating DDE links
    void            IncInDdeLinkUpdate() { if ( nInDdeLinkUpdate < 255 ) ++nInDdeLinkUpdate; }
    void            DecInDdeLinkUpdate() { if ( nInDdeLinkUpdate ) --nInDdeLinkUpdate; }
    bool            IsInDdeLinkUpdate() const   { return nInDdeLinkUpdate != 0; }

    SC_DLLPUBLIC void           CopyDdeLinks( ScDocument* pDestDoc ) const;
    void            DisconnectDdeLinks();

                    // for StarOne Api:
    sal_uInt16          GetDdeLinkCount() const;
    bool            UpdateDdeLink( const rtl::OUString& rAppl, const rtl::OUString& rTopic, const rtl::OUString& rItem );

    /** Tries to find a DDE link with the specified connection data.
        @param rnDdePos  (out-param) Returns the index of the DDE link (does not include other links from link manager).
        @return  true = DDE link found, rnDdePos valid. */
    SC_DLLPUBLIC bool            FindDdeLink( const rtl::OUString& rAppl, const rtl::OUString& rTopic, const rtl::OUString& rItem, sal_uInt8 nMode, sal_uInt16& rnDdePos );

    /** Returns the connection data of the specified DDE link.
        @param nDdePos  Index of the DDE link (does not include other links from link manager).
        @param rAppl  (out-param) The application name.
        @param rTopic  (out-param) The DDE topic.
        @param rItem  (out-param) The DDE item.
        @return  true = DDE link found, out-parameters valid. */
    bool            GetDdeLinkData( sal_uInt16 nDdePos, rtl::OUString& rAppl, rtl::OUString& rTopic, rtl::OUString& rItem ) const;
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
        @return  true = DDE link found; false = Unpredictable error occurred, no DDE link created. */
    SC_DLLPUBLIC bool            CreateDdeLink( const rtl::OUString& rAppl, const rtl::OUString& rTopic, const rtl::OUString& rItem, sal_uInt8 nMode, ScMatrixRef pResults );
    /** Sets a result matrix for the specified DDE link.
        @param nDdePos  Index of the DDE link (does not include other links from link manager).
        @param pResults  The array containing all results of the DDE link (intrusive-ref-counted, do not delete).
        @return  true = DDE link found and matrix set. */
    bool            SetDdeLinkResultMatrix( sal_uInt16 nDdePos, ScMatrixRef pResults );


    SfxBindings*    GetViewBindings();
    SfxObjectShell* GetDocumentShell() const    { return pShell; }
    SC_DLLPUBLIC ScDrawLayer* GetDrawLayer();
    SfxBroadcaster* GetDrawBroadcaster();       // to avoid header
    void            BeginDrawUndo();

    void            BeginUnoRefUndo();
    bool            HasUnoRefUndo() const       { return ( pUnoRefUndoList != NULL ); }
    ScUnoRefList*   EndUnoRefUndo();            // must be deleted by caller!
    sal_Int64       GetNewUnoId();
    void            AddUnoRefChange( sal_Int64 nId, const ScRangeList& rOldRanges );

    bool IsChart( const SdrObject* pObject );

    SC_DLLPUBLIC void           UpdateAllCharts();
    void            UpdateChartRef( UpdateRefMode eUpdateRefMode,
                                    SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                                    SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                                    SCsCOL nDx, SCsROW nDy, SCsTAB nDz );
                    //! only assigns the new RangeList, no ChartListener or the like
    void            SetChartRangeList( const rtl::OUString& rChartName,
                        const ScRangeListRef& rNewRangeListRef );

    void            StartAnimations( SCTAB nTab, Window* pWin );

    bool            HasBackgroundDraw( SCTAB nTab, const Rectangle& rMMRect ) const;
    bool            HasAnyDraw( SCTAB nTab, const Rectangle& rMMRect ) const;

    const ScSheetEvents* GetSheetEvents( SCTAB nTab ) const;
    void            SetSheetEvents( SCTAB nTab, const ScSheetEvents* pNew );
    bool            HasSheetEventScript( SCTAB nTab, sal_Int32 nEvent, bool bWithVbaEvents = false ) const;
    bool            HasAnySheetEventScript( sal_Int32 nEvent, bool bWithVbaEvents = false ) const;  // on any sheet

    bool            HasAnyCalcNotification() const;
    bool            HasCalcNotification( SCTAB nTab ) const;
    void            SetCalcNotification( SCTAB nTab );
    void            ResetCalcNotifications();

    SC_DLLPUBLIC ScOutlineTable*    GetOutlineTable( SCTAB nTab, bool bCreate = false );
    bool            SetOutlineTable( SCTAB nTab, const ScOutlineTable* pNewOutline );

    void            DoAutoOutline( SCCOL nStartCol, SCROW nStartRow,
                                    SCCOL nEndCol, SCROW nEndRow, SCTAB nTab );

    bool            DoSubTotals( SCTAB nTab, ScSubTotalParam& rParam );
    void            RemoveSubTotals( SCTAB nTab, ScSubTotalParam& rParam );
    bool            TestRemoveSubTotals( SCTAB nTab, const ScSubTotalParam& rParam );
    bool            HasSubTotalCells( const ScRange& rRange );

    SC_DLLPUBLIC void           PutCell( const ScAddress&, ScBaseCell* pCell, bool bForceTab = false );
    SC_DLLPUBLIC void           PutCell( SCCOL nCol, SCROW nRow, SCTAB nTab, ScBaseCell* pCell,
                            bool bForceTab = false );
    SC_DLLPUBLIC void           PutCell(SCCOL nCol, SCROW nRow, SCTAB nTab, ScBaseCell* pCell,
                            sal_uLong nFormatIndex, bool bForceTab = false);
                    //  return TRUE = number format is set
    SC_DLLPUBLIC bool           SetString(
        SCCOL nCol, SCROW nRow, SCTAB nTab, const rtl::OUString& rString,
        ScSetStringParam* pParam = NULL );
    SC_DLLPUBLIC void           SetValue( SCCOL nCol, SCROW nRow, SCTAB nTab, const double& rVal );
    void            SetError( SCCOL nCol, SCROW nRow, SCTAB nTab, const sal_uInt16 nError);

    SC_DLLPUBLIC void           InsertMatrixFormula(SCCOL nCol1, SCROW nRow1,
                                        SCCOL nCol2, SCROW nRow2,
                                        const ScMarkData& rMark,
                                        const rtl::OUString& rFormula,
                                        const ScTokenArray* p = NULL,
                                        const formula::FormulaGrammar::Grammar = formula::FormulaGrammar::GRAM_DEFAULT );
    SC_DLLPUBLIC void           InsertTableOp(const ScTabOpParam& rParam,   // multi-operation
                                  SCCOL nCol1, SCROW nRow1,
                                  SCCOL nCol2, SCROW nRow2, const ScMarkData& rMark);

    SC_DLLPUBLIC void           GetString( SCCOL nCol, SCROW nRow, SCTAB nTab, String& rString );
    SC_DLLPUBLIC void           GetString( SCCOL nCol, SCROW nRow, SCTAB nTab, rtl::OUString& rString );
    SC_DLLPUBLIC rtl::OUString  GetString( SCCOL nCol, SCROW nRow, SCTAB nTab) { rtl::OUString aString; GetString(nCol, nRow, nTab, aString); return aString;}
    SC_DLLPUBLIC void           GetInputString( SCCOL nCol, SCROW nRow, SCTAB nTab, String& rString );
    SC_DLLPUBLIC void           GetInputString( SCCOL nCol, SCROW nRow, SCTAB nTab, rtl::OUString& rString );
    sal_uInt16                  GetStringForFormula( const ScAddress& rPos, rtl::OUString& rString );
    SC_DLLPUBLIC double         GetValue( const ScAddress& );
    SC_DLLPUBLIC double         GetValue( const SCCOL nCol, SCROW nRow, SCTAB nTab) { ScAddress aAdr(nCol, nRow, nTab); return GetValue(aAdr);}
    SC_DLLPUBLIC void           GetValue( SCCOL nCol, SCROW nRow, SCTAB nTab, double& rValue );
    SC_DLLPUBLIC double         RoundValueAsShown( double fVal, sal_uLong nFormat );
    SC_DLLPUBLIC void           GetNumberFormat( SCCOL nCol, SCROW nRow, SCTAB nTab,
                                     sal_uInt32& rFormat ) const;
    sal_uInt32      GetNumberFormat( const ScRange& rRange ) const;
    SC_DLLPUBLIC sal_uInt32     GetNumberFormat( const ScAddress& ) const;
                    /** If no number format attribute is set and the cell
                        pointer passed is of type formula cell, the calculated
                        number format of the formula cell is returned. pCell
                        may be NULL. */
    SC_DLLPUBLIC void           GetNumberFormatInfo( short& nType, sal_uLong& nIndex,
                        const ScAddress& rPos, const ScBaseCell* pCell ) const;
    void            GetFormula( SCCOL nCol, SCROW nRow, SCTAB nTab, String& rFormula ) const;
    SC_DLLPUBLIC void           GetFormula( SCCOL nCol, SCROW nRow, SCTAB nTab, rtl::OUString& rFormula ) const;
    SC_DLLPUBLIC void           GetCellType( SCCOL nCol, SCROW nRow, SCTAB nTab, CellType& rCellType ) const;
    SC_DLLPUBLIC CellType       GetCellType( const ScAddress& rPos ) const;
    SC_DLLPUBLIC void           GetCell( SCCOL nCol, SCROW nRow, SCTAB nTab, ScBaseCell*& rpCell ) const;
    SC_DLLPUBLIC ScBaseCell*        GetCell( const ScAddress& rPos ) const;

    SC_DLLPUBLIC bool           HasData( SCCOL nCol, SCROW nRow, SCTAB nTab );
    SC_DLLPUBLIC bool           HasStringData( SCCOL nCol, SCROW nRow, SCTAB nTab ) const;
    SC_DLLPUBLIC bool           HasValueData( SCCOL nCol, SCROW nRow, SCTAB nTab ) const;
    bool            HasStringCells( const ScRange& rRange ) const;

    /** Returns true, if there is any data to create a selection list for rPos. */
    bool            HasSelectionData( SCCOL nCol, SCROW nRow, SCTAB nTab ) const;

    /** Returns a table notes container. */
    SC_DLLPUBLIC ScNotes*       GetNotes(SCTAB nTab);

    /** Creates the captions of all uninitialized cell notes in the specified sheet.
        @param bForced  True = always create all captions, false = skip when Undo is disabled. */
    void            InitializeNoteCaptions( SCTAB nTab, bool bForced = false );
    /** Creates the captions of all uninitialized cell notes in all sheets.
        @param bForced  True = always create all captions, false = skip when Undo is disabled. */
    void            InitializeAllNoteCaptions( bool bForced = false );

    void            SetDrawPageSize(SCTAB nTab);

    bool            ExtendMergeSel( SCCOL nStartCol, SCROW nStartRow,
                                SCCOL& rEndCol, SCROW& rEndRow, const ScMarkData& rMark,
                                bool bRefresh = false );
    SC_DLLPUBLIC bool            ExtendMerge( SCCOL nStartCol, SCROW nStartRow,
                                SCCOL& rEndCol, SCROW& rEndRow, SCTAB nTab,
                                bool bRefresh = false );
    bool            ExtendMerge( ScRange& rRange, bool bRefresh = false );
    bool            ExtendTotalMerge( ScRange& rRange ) const;
    SC_DLLPUBLIC bool           ExtendOverlapped( SCCOL& rStartCol, SCROW& rStartRow,
                                SCCOL nEndCol, SCROW nEndRow, SCTAB nTab ) const;
    SC_DLLPUBLIC bool           ExtendOverlapped( ScRange& rRange ) const;

    bool            RefreshAutoFilter( SCCOL nStartCol, SCROW nStartRow,
                                SCCOL nEndCol, SCROW nEndRow, SCTAB nTab );

    SC_DLLPUBLIC void           DoMergeContents( SCTAB nTab, SCCOL nStartCol, SCROW nStartRow,
                                    SCCOL nEndCol, SCROW nEndRow );
                    //  without checking:
    SC_DLLPUBLIC void           DoMerge( SCTAB nTab, SCCOL nStartCol, SCROW nStartRow,
                                    SCCOL nEndCol, SCROW nEndRow, bool bDeleteCaptions = true );
    void            RemoveMerge( SCCOL nCol, SCROW nRow, SCTAB nTab );

    bool            IsBlockEmpty( SCTAB nTab, SCCOL nStartCol, SCROW nStartRow,
                                                SCCOL nEndCol, SCROW nEndRow, bool bIgnoreNotes = false ) const;
    bool            IsPrintEmpty( SCTAB nTab, SCCOL nStartCol, SCROW nStartRow,
                                                SCCOL nEndCol, SCROW nEndRow,
                                                bool bLeftIsEmpty = false,
                                                ScRange* pLastRange = NULL,
                                                Rectangle* pLastMM = NULL ) const;

    void            SkipOverlapped( SCCOL& rCol, SCROW& rRow, SCTAB nTab ) const;
    bool            IsHorOverlapped( SCCOL nCol, SCROW nRow, SCTAB nTab ) const;
    bool            IsVerOverlapped( SCCOL nCol, SCROW nRow, SCTAB nTab ) const;

    SC_DLLPUBLIC bool HasAttrib( SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                                 SCCOL nCol2, SCROW nRow2, SCTAB nTab2, sal_uInt16 nMask ) const;
    SC_DLLPUBLIC bool HasAttrib( const ScRange& rRange, sal_uInt16 nMask ) const;

    SC_DLLPUBLIC void GetBorderLines( SCCOL nCol, SCROW nRow, SCTAB nTab,
                                    const ::editeng::SvxBorderLine** ppLeft,
                                    const ::editeng::SvxBorderLine** ppTop,
                                    const ::editeng::SvxBorderLine** ppRight,
                                    const ::editeng::SvxBorderLine** ppBottom ) const;

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
    SC_DLLPUBLIC  void             SetAutoNameCache(  ScAutoNameCache* pCache );

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
    void            SetAutoCalc( bool bNewAutoCalc );
    bool            GetAutoCalc() const { return bAutoCalc; }
                    // calculate automatically in/from/for ScDocShell disabled
    void            SetAutoCalcShellDisabled( bool bNew ) { bAutoCalcShellDisabled = bNew; }
    bool            IsAutoCalcShellDisabled() const { return bAutoCalcShellDisabled; }
                    // ForcedFormulas are to be calculated
    void            SetForcedFormulaPending( bool bNew ) { bForcedFormulaPending = bNew; }
    bool            IsForcedFormulaPending() const { return bForcedFormulaPending; }
                    // if CalcFormulaTree() is currently running
    bool            IsCalculatingFormulaTree() { return bCalculatingFormulaTree; }

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
                               Out parameter, true if area was shrunk, false if not.

                        @returns true if there is any data, false if not.
                     */
    bool            ShrinkToUsedDataArea( bool& o_bShrunk,
                                          SCTAB nTab, SCCOL& rStartCol, SCROW& rStartRow,
                                          SCCOL& rEndCol, SCROW& rEndRow, bool bColumnsOnly ) const;

    SC_DLLPUBLIC void           GetDataArea( SCTAB nTab, SCCOL& rStartCol, SCROW& rStartRow,
                                    SCCOL& rEndCol, SCROW& rEndRow, bool bIncludeOld, bool bOnlyDown ) const;
    SC_DLLPUBLIC bool           GetCellArea( SCTAB nTab, SCCOL& rEndCol, SCROW& rEndRow ) const;
    SC_DLLPUBLIC bool           GetTableArea( SCTAB nTab, SCCOL& rEndCol, SCROW& rEndRow ) const;
    SC_DLLPUBLIC void           GetFormattedAndUsedArea( SCTAB nTab, SCCOL& rEndCol, SCROW& rEndRow ) const;
    SC_DLLPUBLIC bool           GetPrintArea( SCTAB nTab, SCCOL& rEndCol, SCROW& rEndRow,
                                    bool bNotes = true ) const;
    SC_DLLPUBLIC bool           GetPrintAreaHor( SCTAB nTab, SCROW nStartRow, SCROW nEndRow,
                                        SCCOL& rEndCol, bool bNotes = true ) const;
    SC_DLLPUBLIC bool           GetPrintAreaVer( SCTAB nTab, SCCOL nStartCol, SCCOL nEndCol,
                                        SCROW& rEndRow, bool bNotes = true ) const;
    void            InvalidateTableArea();


    SC_DLLPUBLIC bool           GetDataStart( SCTAB nTab, SCCOL& rStartCol, SCROW& rStartRow ) const;

    /**
     * Find the maximum column position that contains printable data for the
     * specified row range.  The final column position must be equal or less
     * than the initial value of rEndCol.
     */
    void            ExtendPrintArea( OutputDevice* pDev, SCTAB nTab,
                                    SCCOL nStartCol, SCROW nStartRow,
                                    SCCOL& rEndCol, SCROW nEndRow ) const;
    SC_DLLPUBLIC SCSIZE         GetEmptyLinesInBlock( SCCOL nStartCol, SCROW nStartRow, SCTAB nStartTab,
                                            SCCOL nEndCol, SCROW nEndRow, SCTAB nEndTab,
                                            ScDirection eDir );

    SC_DLLPUBLIC void           FindAreaPos( SCCOL& rCol, SCROW& rRow, SCTAB nTab, ScMoveDirection eDirection );
    SC_DLLPUBLIC void           GetNextPos( SCCOL& rCol, SCROW& rRow, SCTAB nTab, SCsCOL nMovX, SCsROW nMovY,
                                bool bMarked, bool bUnprotected, const ScMarkData& rMark );

    bool            GetNextMarkedCell( SCCOL& rCol, SCROW& rRow, SCTAB nTab,
                                        const ScMarkData& rMark );

    void            LimitChartArea( SCTAB nTab, SCCOL& rStartCol, SCROW& rStartRow,
                                                    SCCOL& rEndCol, SCROW& rEndRow );
    void            LimitChartIfAll( ScRangeListRef& rRangeList );

    bool            InsertRow( SCCOL nStartCol, SCTAB nStartTab,
                               SCCOL nEndCol,   SCTAB nEndTab,
                               SCROW nStartRow, SCSIZE nSize, ScDocument* pRefUndoDoc = NULL,
                               const ScMarkData* pTabMark = NULL );
    SC_DLLPUBLIC bool           InsertRow( const ScRange& rRange, ScDocument* pRefUndoDoc = NULL );
    void            DeleteRow( SCCOL nStartCol, SCTAB nStartTab,
                               SCCOL nEndCol,   SCTAB nEndTab,
                               SCROW nStartRow, SCSIZE nSize,
                               ScDocument* pRefUndoDoc = NULL, bool* pUndoOutline = NULL,
                               const ScMarkData* pTabMark = NULL );
    SC_DLLPUBLIC void   DeleteRow( const ScRange& rRange,
                               ScDocument* pRefUndoDoc = NULL, bool* pUndoOutline = NULL );
    bool            InsertCol( SCROW nStartRow, SCTAB nStartTab,
                               SCROW nEndRow,   SCTAB nEndTab,
                               SCCOL nStartCol, SCSIZE nSize, ScDocument* pRefUndoDoc = NULL,
                               const ScMarkData* pTabMark = NULL );
    SC_DLLPUBLIC bool           InsertCol( const ScRange& rRange, ScDocument* pRefUndoDoc = NULL );
    void            DeleteCol( SCROW nStartRow, SCTAB nStartTab,
                               SCROW nEndRow, SCTAB nEndTab,
                               SCCOL nStartCol, SCSIZE nSize,
                               ScDocument* pRefUndoDoc = NULL, bool* pUndoOutline = NULL,
                               const ScMarkData* pTabMark = NULL );
    void            DeleteCol( const ScRange& rRange,
                               ScDocument* pRefUndoDoc = NULL, bool* pUndoOutline = NULL );

    bool            CanInsertRow( const ScRange& rRange ) const;
    bool            CanInsertCol( const ScRange& rRange ) const;

    void            FitBlock( const ScRange& rOld, const ScRange& rNew, bool bClear = true );
    bool            CanFitBlock( const ScRange& rOld, const ScRange& rNew );

    bool            IsClipOrUndo() const                        { return bIsClip || bIsUndo; }
    bool            IsUndo() const                              { return bIsUndo; }
    bool            IsClipboard() const                         { return bIsClip; }
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
    void            SetCutMode( bool bCut );
    SC_DLLPUBLIC bool           IsCutMode();
    void            SetClipArea( const ScRange& rArea, bool bCut = false );

    SC_DLLPUBLIC bool           IsDocVisible() const                        { return bIsVisible; }
    void            SetDocVisible( bool bSet );

    bool            HasOLEObjectsInArea( const ScRange& rRange, const ScMarkData* pTabMark = NULL );

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
                               bool bIncludeObjects = false, bool bCloneNoteCaptions = true, bool bUseRangeForVBA = false );

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
                                    bool bResetCut = true,
                                    bool bAsLink = false,
                                    bool bIncludeFiltered = true,
                                    bool bSkipAttrForEmpty = false,
                                    const ScRangeList * pDestRanges = NULL );

    void            CopyMultiRangeFromClip(const ScAddress& rDestPos, const ScMarkData& rMark,
                                           sal_uInt16 nInsFlag, ScDocument* pClipDoc,
                                           bool bResetCut = true, bool bAsLink = false,
                                           bool bIncludeFiltered = true,
                                           bool bSkipAttrForEmpty = false);

    void            GetClipArea(SCCOL& nClipX, SCROW& nClipY, bool bIncludeFiltered);
    void            GetClipStart(SCCOL& nClipX, SCROW& nClipY);

    bool            HasClipFilteredRows();

    bool            IsClipboardSource() const;

    SC_DLLPUBLIC void           TransposeClip( ScDocument* pTransClip, sal_uInt16 nFlags, bool bAsLink );

    ScClipParam&    GetClipParam();
    void            SetClipParam(const ScClipParam& rParam);

    void            MixDocument( const ScRange& rRange, sal_uInt16 nFunction, bool bSkipEmpty,
                                    ScDocument* pSrcDoc );

    void            FillTab( const ScRange& rSrcArea, const ScMarkData& rMark,
                                sal_uInt16 nFlags, sal_uInt16 nFunction,
                                bool bSkipEmpty, bool bAsLink );
    void            FillTabMarked( SCTAB nSrcTab, const ScMarkData& rMark,
                                sal_uInt16 nFlags, sal_uInt16 nFunction,
                                bool bSkipEmpty, bool bAsLink );

    void            TransliterateText( const ScMarkData& rMultiMark, sal_Int32 nType );

    SC_DLLPUBLIC void           InitUndo( ScDocument* pSrcDoc, SCTAB nTab1, SCTAB nTab2,
                                bool bColInfo = false, bool bRowInfo = false );
    void            AddUndoTab( SCTAB nTab1, SCTAB nTab2,
                                bool bColInfo = false, bool bRowInfo = false );
    SC_DLLPUBLIC void           InitUndoSelected( ScDocument* pSrcDoc, const ScMarkData& rTabSelection,
                                bool bColInfo = false, bool bRowInfo = false );
    void            SetInTest() { mbIsInTest = true; }

                    //  don't use anymore:
    void            CopyToDocument(SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                                SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                                sal_uInt16 nFlags, bool bMarked, ScDocument* pDestDoc,
                                const ScMarkData* pMarks = NULL, bool bColRowFlags = true);
    void            UndoToDocument(SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                                SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                                sal_uInt16 nFlags, bool bMarked, ScDocument* pDestDoc,
                                const ScMarkData* pMarks = NULL);

    void            CopyToDocument(const ScRange& rRange,
                                sal_uInt16 nFlags, bool bMarked, ScDocument* pDestDoc,
                                const ScMarkData* pMarks = NULL, bool bColRowFlags = true);
    void            UndoToDocument(const ScRange& rRange,
                                sal_uInt16 nFlags, bool bMarked, ScDocument* pDestDoc,
                                const ScMarkData* pMarks = NULL);

    void            CopyScenario( SCTAB nSrcTab, SCTAB nDestTab, bool bNewScenario = false );
    bool            TestCopyScenario( SCTAB nSrcTab, SCTAB nDestTab ) const;
    void            MarkScenario( SCTAB nSrcTab, SCTAB nDestTab,
                                    ScMarkData& rDestMark, bool bResetMark = true,
                                    sal_uInt16 nNeededBits = 0 ) const;
    bool            HasScenarioRange( SCTAB nTab, const ScRange& rRange ) const;
    SC_DLLPUBLIC const ScRangeList* GetScenarioRanges( SCTAB nTab ) const;

    SC_DLLPUBLIC void           CopyUpdated( ScDocument* pPosDoc, ScDocument* pDestDoc );

    void            UpdateReference( UpdateRefMode eUpdateRefMode, SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                                     SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                                     SCsCOL nDx, SCsROW nDy, SCsTAB nDz,
                                     ScDocument* pUndoDoc = NULL, bool bIncludeDraw = true,
                                     bool bUpdateNoteCaptionPos = true );

    SC_DLLPUBLIC void           UpdateTranspose( const ScAddress& rDestPos, ScDocument* pClipDoc,
                                        const ScMarkData& rMark, ScDocument* pUndoDoc = NULL );

    void            UpdateGrow( const ScRange& rArea, SCCOL nGrowX, SCROW nGrowY );

    SC_DLLPUBLIC void Fill(   SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                            ScProgress* pProgress, const ScMarkData& rMark,
                            sal_uLong nFillCount, FillDir eFillDir = FILL_TO_BOTTOM,
                            FillCmd eFillCmd = FILL_LINEAR, FillDateCmd eFillDateCmd = FILL_DAY,
                            double nStepValue = 1.0, double nMaxValue = 1E307 );
    rtl::OUString          GetAutoFillPreview( const ScRange& rSource, SCCOL nEndX, SCROW nEndY );

    bool            GetSelectionFunction( ScSubTotalFunc eFunc,
                                            const ScAddress& rCursor, const ScMarkData& rMark,
                                            double& rResult );

    SC_DLLPUBLIC const SfxPoolItem*         GetAttr( SCCOL nCol, SCROW nRow, SCTAB nTab, sal_uInt16 nWhich ) const;
    SC_DLLPUBLIC const ScPatternAttr*   GetPattern( SCCOL nCol, SCROW nRow, SCTAB nTab ) const;
    SC_DLLPUBLIC const ScPatternAttr*    GetMostUsedPattern( SCCOL nCol, SCROW nStartRow, SCROW nEndRow, SCTAB nTab ) const;
    const ScPatternAttr*    GetSelectionPattern( const ScMarkData& rMark, bool bDeep = true );
    ScPatternAttr*          CreateSelectionPattern( const ScMarkData& rMark, bool bDeep = true );

    SC_DLLPUBLIC ScConditionalFormat* GetCondFormat( SCCOL nCol, SCROW nRow, SCTAB nTab ) const;
    SC_DLLPUBLIC const SfxItemSet*  GetCondResult( SCCOL nCol, SCROW nRow, SCTAB nTab ) const;
    const SfxPoolItem*  GetEffItem( SCCOL nCol, SCROW nRow, SCTAB nTab, sal_uInt16 nWhich ) const;

    SC_DLLPUBLIC const ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XBreakIterator >& GetBreakIterator();
    bool            HasStringWeakCharacters( const rtl::OUString& rString );
    SC_DLLPUBLIC sal_uInt8          GetStringScriptType( const rtl::OUString& rString );
    SC_DLLPUBLIC sal_uInt8          GetCellScriptType( ScBaseCell* pCell, sal_uLong nNumberFormat );
    SC_DLLPUBLIC sal_uInt8          GetScriptType( SCCOL nCol, SCROW nRow, SCTAB nTab, ScBaseCell* pCell = NULL );

    bool            HasDetectiveOperations() const;
    void            AddDetectiveOperation( const ScDetOpData& rData );
    void            ClearDetectiveOperations();
    ScDetOpList*    GetDetOpList() const                { return pDetOpList; }
    void            SetDetOpList(ScDetOpList* pNew);

    bool            HasDetectiveObjects(SCTAB nTab) const;

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
    void            ChangeSelectionIndent( bool bIncrement, const ScMarkData& rMark );

    SC_DLLPUBLIC sal_uLong AddCondFormat( ScConditionalFormat* pNew, SCTAB nTab );
    void DeleteConditionalFormat( sal_uLong nIndex, SCTAB nTab );
    SC_DLLPUBLIC void           FindConditionalFormat( sal_uLong nKey, ScRangeList& rRanges, SCTAB nTab );
    void            ConditionalChanged( sal_uLong nKey, SCTAB nTab );

    void SetCondFormList( ScConditionalFormatList* pList, SCTAB nTab );

    SC_DLLPUBLIC sal_uLong          AddValidationEntry( const ScValidationData& rNew );

    SC_DLLPUBLIC const ScValidationData*    GetValidationEntry( sal_uLong nIndex ) const;

    SC_DLLPUBLIC ScConditionalFormatList* GetCondFormList( SCTAB nTab ) const;

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
                                            const ::editeng::SvxBorderLine* pLine, bool bColorOnly );

    const ScStyleSheet* GetStyle( SCCOL nCol, SCROW nRow, SCTAB nTab ) const;
    const ScStyleSheet* GetSelectionStyle( const ScMarkData& rMark ) const;

    void            StyleSheetChanged( const SfxStyleSheetBase* pStyleSheet, bool bRemoved,
                                        OutputDevice* pDev,
                                        double nPPTX, double nPPTY,
                                        const Fraction& rZoomX, const Fraction& rZoomY );

    bool            IsStyleSheetUsed( const ScStyleSheet& rStyle, bool bGatherAllStyles ) const;

    SC_DLLPUBLIC bool           ApplyFlagsTab( SCCOL nStartCol, SCROW nStartRow,
                                            SCCOL nEndCol, SCROW nEndRow,
                                            SCTAB nTab, sal_Int16 nFlags );
    SC_DLLPUBLIC bool           RemoveFlagsTab( SCCOL nStartCol, SCROW nStartRow,
                                            SCCOL nEndCol, SCROW nEndRow,
                                            SCTAB nTab, sal_Int16 nFlags );

    SC_DLLPUBLIC void           SetPattern( const ScAddress&, const ScPatternAttr& rAttr,
                                    bool bPutToPool = false );
    SC_DLLPUBLIC void           SetPattern( SCCOL nCol, SCROW nRow, SCTAB nTab, const ScPatternAttr& rAttr,
                                    bool bPutToPool = false );

    void            AutoFormat( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                                    sal_uInt16 nFormatNo, const ScMarkData& rMark );
    void            GetAutoFormatData( SCTAB nTab, SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                                        ScAutoFormatData& rData );
    bool            SearchAndReplace( const SvxSearchItem& rSearchItem,
                                      SCCOL& rCol, SCROW& rRow, SCTAB& rTab,
                                      const ScMarkData& rMark, ScRangeList& rMatchedRanges,
                                      rtl::OUString& rUndoStr, ScDocument* pUndoDoc = NULL );

                    // determine Col/Row of subsequent calls
                    // (e.g. not found from the beginning, or subsequent tables)
                    // meaning of explanation in "()" was already unclear in German
    static void     GetSearchAndReplaceStart( const SvxSearchItem& rSearchItem,
                        SCCOL& rCol, SCROW& rRow );

    bool            Solver(SCCOL nFCol, SCROW nFRow, SCTAB nFTab,
                            SCCOL nVCol, SCROW nVRow, SCTAB nVTab,
                            const rtl::OUString& sValStr, double& nX);

    SC_DLLPUBLIC void            ApplySelectionPattern( const ScPatternAttr& rAttr, const ScMarkData& rMark,
                                           ScEditDataArray* pDataArray = NULL );
    void            DeleteSelection( sal_uInt16 nDelFlag, const ScMarkData& rMark );
    void            DeleteSelectionTab( SCTAB nTab, sal_uInt16 nDelFlag, const ScMarkData& rMark );

    SC_DLLPUBLIC void           SetColWidth( SCCOL nCol, SCTAB nTab, sal_uInt16 nNewWidth );
    SC_DLLPUBLIC void           SetColWidthOnly( SCCOL nCol, SCTAB nTab, sal_uInt16 nNewWidth );
    SC_DLLPUBLIC void           SetRowHeight( SCROW nRow, SCTAB nTab, sal_uInt16 nNewHeight );
    SC_DLLPUBLIC void           SetRowHeightRange( SCROW nStartRow, SCROW nEndRow, SCTAB nTab,
                                            sal_uInt16 nNewHeight );

    SC_DLLPUBLIC void           SetRowHeightOnly( SCROW nStartRow, SCROW nEndRow, SCTAB nTab,
                                                  sal_uInt16 nNewHeight );
    SC_DLLPUBLIC void           SetManualHeight( SCROW nStartRow, SCROW nEndRow, SCTAB nTab, bool bManual );

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
                                        bool bFormula,
                                        const ScMarkData* pMarkData = NULL,
                                        const ScColWidthParam* pParam = NULL );
    SC_DLLPUBLIC bool           SetOptimalHeight( SCROW nStartRow, SCROW nEndRow, SCTAB nTab, sal_uInt16 nExtra,
                                        OutputDevice* pDev,
                                        double nPPTX, double nPPTY,
                                        const Fraction& rZoomX, const Fraction& rZoomY,
                                        bool bShrink );
    void            UpdateAllRowHeights( OutputDevice* pDev,
                                        double nPPTX, double nPPTY,
                                        const Fraction& rZoomX, const Fraction& rZoomY,
                                        const ScMarkData* pTabMark = NULL );
    long            GetNeededSize( SCCOL nCol, SCROW nRow, SCTAB nTab,
                                    OutputDevice* pDev,
                                    double nPPTX, double nPPTY,
                                    const Fraction& rZoomX, const Fraction& rZoomY,
                                    bool bWidth, bool bTotalSize = false );

    SC_DLLPUBLIC void           ShowCol(SCCOL nCol, SCTAB nTab, bool bShow);
    SC_DLLPUBLIC void           ShowRow(SCROW nRow, SCTAB nTab, bool bShow);
    SC_DLLPUBLIC void           ShowRows(SCROW nRow1, SCROW nRow2, SCTAB nTab, bool bShow);
    SC_DLLPUBLIC void           SetRowFlags( SCROW nRow, SCTAB nTab, sal_uInt8 nNewFlags );
    SC_DLLPUBLIC void           SetRowFlags( SCROW nStartRow, SCROW nEndRow, SCTAB nTab, sal_uInt8 nNewFlags );

    SC_DLLPUBLIC sal_uInt8          GetColFlags( SCCOL nCol, SCTAB nTab ) const;
    SC_DLLPUBLIC sal_uInt8          GetRowFlags( SCROW nRow, SCTAB nTab ) const;

    SC_DLLPUBLIC const ScBitMaskCompressedArray< SCROW, sal_uInt8> & GetRowFlagsArray( SCTAB nTab ) const;

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

    bool IsManualRowHeight(SCROW nRow, SCTAB nTab) const;

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
    bool            GetColDefault( SCTAB nTab, SCCOL nCol, SCROW nLastRow, SCROW& nDefault);
    bool            GetRowDefault( SCTAB nTab, SCROW nRow, SCCOL nLastCol, SCCOL& nDefault);

    bool            UpdateOutlineCol( SCCOL nStartCol, SCCOL nEndCol, SCTAB nTab, bool bShow );
    bool            UpdateOutlineRow( SCROW nStartRow, SCROW nEndRow, SCTAB nTab, bool bShow );

    void            StripHidden( SCCOL& rX1, SCROW& rY1, SCCOL& rX2, SCROW& rY2, SCTAB nTab );
    void            ExtendHidden( SCCOL& rX1, SCROW& rY1, SCCOL& rX2, SCROW& rY2, SCTAB nTab );

    SC_DLLPUBLIC ScPatternAttr*     GetDefPattern() const;
    SC_DLLPUBLIC ScDocumentPool*        GetPool();
    SC_DLLPUBLIC ScStyleSheetPool*  GetStyleSheetPool() const;

    // PageStyle:
    SC_DLLPUBLIC const rtl::OUString  GetPageStyle( SCTAB nTab ) const;
    SC_DLLPUBLIC void           SetPageStyle( SCTAB nTab, const rtl::OUString& rName );
    Size            GetPageSize( SCTAB nTab ) const;
    void            SetPageSize( SCTAB nTab, const Size& rSize );
    void            SetRepeatArea( SCTAB nTab, SCCOL nStartCol, SCCOL nEndCol, SCROW nStartRow, SCROW nEndRow );
    void            InvalidatePageBreaks(SCTAB nTab);
    void            UpdatePageBreaks( SCTAB nTab, const ScRange* pUserArea = NULL );
    void            RemoveManualBreaks( SCTAB nTab );
    bool            HasManualBreaks( SCTAB nTab ) const;

    bool            IsPageStyleInUse( const rtl::OUString& rStrPageStyle, SCTAB* pInTab = NULL );
    bool            RemovePageStyleInUse( const rtl::OUString& rStrPageStyle );
    bool            RenamePageStyleInUse( const rtl::OUString& rOld, const rtl::OUString& rNew );
    void            ModifyStyleSheet( SfxStyleSheetBase& rPageStyle,
                                      const SfxItemSet&  rChanges );

    void            PageStyleModified( SCTAB nTab, const rtl::OUString& rNewName );

    SC_DLLPUBLIC bool           NeedPageResetAfterTab( SCTAB nTab ) const;

    // Was stored in PageStyle previously. Now it exists for every table:
    SC_DLLPUBLIC bool           HasPrintRange();
    SC_DLLPUBLIC sal_uInt16         GetPrintRangeCount( SCTAB nTab );
    SC_DLLPUBLIC const ScRange* GetPrintRange( SCTAB nTab, sal_uInt16 nPos );
    SC_DLLPUBLIC const ScRange* GetRepeatColRange( SCTAB nTab );
    SC_DLLPUBLIC const ScRange* GetRepeatRowRange( SCTAB nTab );
    /** Returns true, if the specified sheet is always printed. */
    bool            IsPrintEntireSheet( SCTAB nTab ) const;

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
    sal_uLong           GetSrcVersion() const   { return nSrcVer; }
    SCROW           GetSrcMaxRow() const    { return nSrcMaxRow; }

    void            SetSrcCharSet( CharSet eNew )   { eSrcSet = eNew; }
    void            UpdateFontCharSet();

    void            FillInfo( ScTableInfo& rTabInfo, SCCOL nX1, SCROW nY1, SCCOL nX2, SCROW nY2,
                        SCTAB nTab, double nScaleX, double nScaleY,
                        bool bPageMode, bool bFormulaMode,
                        const ScMarkData* pMarkData = NULL );

    SC_DLLPUBLIC SvNumberFormatter* GetFormatTable() const;

    void            Sort( SCTAB nTab, const ScSortParam& rSortParam, bool bKeepQuery );
    SCSIZE          Query( SCTAB nTab, const ScQueryParam& rQueryParam, bool bKeepSub );
    SC_DLLPUBLIC bool           CreateQueryParam( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                                        SCTAB nTab, ScQueryParam& rQueryParam );
    void            GetUpperCellString(SCCOL nCol, SCROW nRow, SCTAB nTab, rtl::OUString& rStr);

    /**
     * Get a list of unique strings to use in filtering criteria.  The string
     * values are sorted, and there are no duplicate values in the list.  The
     * data range to use to populate the filter entries is inferred from the
     * database range that contains the specified cell position.
     */
    bool GetFilterEntries(
        SCCOL nCol, SCROW nRow, SCTAB nTab, bool bFilter, std::vector<ScTypedStrData>& rStrings, bool& rHasDates);

    SC_DLLPUBLIC bool GetFilterEntriesArea(
        SCCOL nCol, SCROW nStartRow, SCROW nEndRow, SCTAB nTab, bool bCaseSens,
        std::vector<ScTypedStrData>& rStrings, bool& rHasDates);

    bool GetDataEntries(
        SCCOL nCol, SCROW nRow, SCTAB nTab, bool bCaseSens,
        std::vector<ScTypedStrData>& rStrings, bool bLimit = false );
    bool GetFormulaEntries( ScTypedCaseStrSet& rStrings );

    bool HasAutoFilter( SCCOL nCol, SCROW nRow, SCTAB nTab );

    SC_DLLPUBLIC bool           HasColHeader( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                                    SCTAB nTab );
    SC_DLLPUBLIC bool           HasRowHeader( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                                    SCTAB nTab );

    SfxPrinter*     GetPrinter( bool bCreateIfNotExist = true );
    void            SetPrinter( SfxPrinter* pNewPrinter );
    VirtualDevice*  GetVirtualDevice_100th_mm();
    SC_DLLPUBLIC OutputDevice*  GetRefDevice(); // WYSIWYG: Printer, otherwise VirtualDevice...

    bool            GetNextSpellingCell(SCCOL& nCol, SCROW& nRow, SCTAB nTab,
                                        bool bInSel, const ScMarkData& rMark) const;

    bool            ReplaceStyle(const SvxSearchItem& rSearchItem,
                                 SCCOL nCol, SCROW nRow, SCTAB nTab,
                                 ScMarkData& rMark, bool bIsUndo);

    void            DoColResize( SCTAB nTab, SCCOL nCol1, SCCOL nCol2, SCSIZE nAdd );

    void            InvalidateTextWidth( const rtl::OUString& rStyleName );
    void            InvalidateTextWidth( SCTAB nTab );
    void            InvalidateTextWidth( const ScAddress* pAdrFrom, const ScAddress* pAdrTo, bool bNumFormatChanged );

    bool            IdleCalcTextWidth();
    bool            IdleCheckLinks();

    bool            ContinueOnlineSpelling();   // TRUE = found s.th.

    void            RepaintRange( const ScRange& rRange );

    bool        IsIdleDisabled() const      { return bIdleDisabled; }
    void            DisableIdle(bool bDo)   { bIdleDisabled = bDo; }

    bool            IsDetectiveDirty() const     { return bDetectiveDirty; }
    void            SetDetectiveDirty(bool bSet) { bDetectiveDirty = bSet; }

    void            RemoveAutoSpellObj();
    void            SetOnlineSpellPos( const ScAddress& rPos );
    SC_DLLPUBLIC bool           SetVisibleSpellRange( const ScRange& rRange );  // true = changed

    sal_uInt8           GetMacroCallMode() const     { return nMacroCallMode; }
    void            SetMacroCallMode(sal_uInt8 nNew)     { nMacroCallMode = nNew; }

    bool            GetHasMacroFunc() const      { return bHasMacroFunc; }
    void            SetHasMacroFunc(bool bSet)   { bHasMacroFunc = bSet; }

    bool            CheckMacroWarn();

    void            SetRangeOverflowType(sal_uInt32 nType)  { nRangeOverflowType = nType; }
    bool        HasRangeOverflow() const                { return nRangeOverflowType != 0; }
    SC_DLLPUBLIC sal_uInt32      GetRangeOverflowType() const            { return nRangeOverflowType; }

    // for broadcasting/listening
    void            SetNoSetDirty( bool bVal ) { bNoSetDirty = bVal; }
    bool            GetNoSetDirty() const { return bNoSetDirty; }
    void            SetInsertingFromOtherDoc( bool bVal ) { bInsertingFromOtherDoc = bVal; }
    bool            IsInsertingFromOtherDoc() const { return bInsertingFromOtherDoc; }
    void            SetLoadingMedium( bool bVal );
    void            SetImportingXML( bool bVal );
    bool            IsImportingXML() const { return bImportingXML; }
    void            SetXMLFromWrapper( bool bVal );
    bool            IsXMLFromWrapper() const { return bXMLFromWrapper; }
    void            SetCalcingAfterLoad( bool bVal ) { bCalcingAfterLoad = bVal; }
    bool            IsCalcingAfterLoad() const { return bCalcingAfterLoad; }
    void            SetNoListening( bool bVal ) { bNoListening = bVal; }
    bool            GetNoListening() const { return bNoListening; }
    ScBroadcastAreaSlotMachine* GetBASM() const { return pBASM; }

    SC_DLLPUBLIC ScChartListenerCollection* GetChartListenerCollection() const;
    void            SetChartListenerCollection( ScChartListenerCollection*,
                        bool bSetChartRangeLists = false );
    void            UpdateChart( const rtl::OUString& rName );
    void            RestoreChartListener( const rtl::OUString& rName );
    SC_DLLPUBLIC void           UpdateChartListenerCollection();
    bool            IsChartListenerCollectionNeedsUpdate() const
                        { return bChartListenerCollectionNeedsUpdate; }
    void            SetChartListenerCollectionNeedsUpdate( bool bFlg )
                        { bChartListenerCollectionNeedsUpdate = bFlg; }
    void            AddOLEObjectToCollection(const rtl::OUString& rName);

    ScChangeViewSettings* GetChangeViewSettings() const     { return pChangeViewSettings; }
    SC_DLLPUBLIC void               SetChangeViewSettings(const ScChangeViewSettings& rNew);

    rtl::Reference<SvxForbiddenCharactersTable> GetForbiddenCharacters();
    void            SetForbiddenCharacters( const rtl::Reference<SvxForbiddenCharactersTable> xNew );

    sal_uInt8           GetAsianCompression() const;        // CharacterCompressionType values
    bool            IsValidAsianCompression() const;
    void            SetAsianCompression(sal_uInt8 nNew);

    bool            GetAsianKerning() const;
    bool            IsValidAsianKerning() const;
    void            SetAsianKerning(bool bNew);
    void            ApplyAsianEditSettings(ScEditEngineDefaulter& rEngine);

    sal_uInt8           GetEditTextDirection(SCTAB nTab) const; // EEHorizontalTextDirection values

    SC_DLLPUBLIC ScLkUpdMode        GetLinkMode() const             { return eLinkMode ;}
    void            SetLinkMode( ScLkUpdMode nSet ) {   eLinkMode  = nSet;}

    SC_DLLPUBLIC ScMacroManager* GetMacroManager();

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
                                        bool bColumns,
                                        ScDocument& rOtherDoc, SCTAB nThisTab, SCTAB nOtherTab,
                                        SCCOLROW nEndCol, SCCOLROW* pTranslate,
                                        ScProgress* pProgress, sal_uLong nProAdd );
    bool                OnlineSpellInRange( const ScRange& rSpellRange, ScAddress& rSpellPos,
                                        sal_uInt16 nMaxTest );

    DECL_LINK(TrackTimeHdl, void *);

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
    void                CalcFormulaTree( bool bOnlyForced = false,
                                        bool bNoProgressBar = false );
    void                ClearFormulaTree();
    void                AppendToFormulaTrack( ScFormulaCell* pCell );
    void                RemoveFromFormulaTrack( ScFormulaCell* pCell );
    void                TrackFormulas( sal_uLong nHintId = SC_HINT_DATACHANGED );
    sal_uInt16              GetFormulaTrackCount() const { return nFormulaTrackCount; }
    bool                IsInFormulaTree( ScFormulaCell* pCell ) const;
    bool                IsInFormulaTrack( ScFormulaCell* pCell ) const;
    bool                GetHardRecalcState() { return bHardRecalcState; }
    void                SetHardRecalcState( bool bVal ) { bHardRecalcState = bVal; }
    void                StartAllListeners();
    const ScFormulaCell*    GetFormulaTree() const { return pFormulaTree; }
    bool                HasForcedFormulas() const { return bHasForcedFormulas; }
    void                SetForcedFormulas( bool bVal ) { bHasForcedFormulas = bVal; }
    sal_uLong               GetFormulaCodeInTree() const { return nFormulaCodeInTree; }
    bool                IsInInterpreter() const { return nInterpretLevel != 0; }
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
    bool                IsInMacroInterpreter() const { return nMacroInterpretLevel != 0; }
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
    bool                IsInInterpreterTableOp() const { return nInterpreterTableOpLevel != 0; }
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
    void                InvalidateLastTableOpParams() { aLastTableOpParams.bValid = false; }
    ScRecursionHelper&  GetRecursionHelper()
                            {
                                if (!pRecursionHelper)
                                    pRecursionHelper = CreateRecursionHelperInstance();
                                return *pRecursionHelper;
                            }
    bool                IsInDtorClear() const { return bInDtorClear; }
    void                SetExpandRefs( bool bVal ) { bExpandRefs = bVal; }
    bool                IsExpandRefs() { return bExpandRefs; }

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
    void            CompileDBFormula( bool bCreateFormulaString );
    void            CompileNameFormula( bool bCreateFormulaString );
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

    void            SetInLinkUpdate(bool bSet);             // TableLink or AreaLink
    bool            IsInLinkUpdate() const;                 // including DdeLink

    SC_DLLPUBLIC SfxItemPool*       GetEditPool() const;
    SC_DLLPUBLIC SfxItemPool*       GetEnginePool() const;
    SC_DLLPUBLIC ScFieldEditEngine& GetEditEngine();
    SC_DLLPUBLIC ScNoteEditEngine&  GetNoteEngine();

    ScRefreshTimerControl*  GetRefreshTimerControl() const
        { return pRefreshTimerControl; }
    ScRefreshTimerControl * const * GetRefreshTimerControlAddress() const
        { return &pRefreshTimerControl; }

    void            SetPastingDrawFromOtherDoc( bool bVal )
                        { bPastingDrawFromOtherDoc = bVal; }
    bool            PastingDrawFromOtherDoc() const
                        { return bPastingDrawFromOtherDoc; }

                    /// an ID unique to each document instance
    sal_uInt32      GetDocumentID() const;

    void            InvalidateStyleSheetUsage()
                        { bStyleSheetUsageInvalid = true; }
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
    SC_DLLPUBLIC bool   DrawGetPrintArea( ScRange& rRange, bool bSetHor, bool bSetVer ) const;
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
    void    CopyRangeNamesToClip(ScDocument* pClipDoc, const ScRange& rClipRange, SCTAB nTab);

    bool    HasPartOfMerged( const ScRange& rRange );

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
