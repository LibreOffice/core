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

#ifndef INCLUDED_SC_INC_DOCUMENT_HXX
#define INCLUDED_SC_INC_DOCUMENT_HXX

#include <vcl/prntypes.hxx>
#include <vcl/timer.hxx>
#include <vcl/idle.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <vcl/vclptr.hxx>
#include "scdllapi.h"
#include "rangelst.hxx"
#include "rangenam.hxx"
#include "tabopparams.hxx"
#include "types.hxx"
#include <formula/grammar.hxx>
#include <formula/types.hxx>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include "typedstrdata.hxx"
#include "calcmacros.hxx"
#include "calcconfig.hxx"
#include <simplehintids.hxx>
#include <tools/gen.hxx>
#include <svl/zforlist.hxx>

#include <memory>
#include <map>
#include <set>
#include <vector>

#include "markdata.hxx"

enum class SvtScriptType;
enum class ScMF;
namespace editeng { class SvxBorderLine; }
namespace formula { struct VectorRefArray; }
namespace svl {

class SharedString;
class SharedStringPool;

}

namespace sc {

struct FormulaGroupContext;
class StartListeningContext;
class EndListeningContext;
class CopyFromClipContext;
class ColumnSpanSet;
struct ColumnBlockPosition;
struct RefUpdateContext;
class EditTextIterator;
struct NoteEntry;
struct FormulaGroupContext;
class DocumentStreamAccess;
class DocumentLinkManager;
class CellValues;
class TableValues;
class RowHeightContext;
struct SetFormulaDirtyContext;
class RefMovedHint;
struct SortUndoParam;
struct ReorderParam;
class FormulaGroupAreaListener;
class ColumnSet;
class UpdatedRangeNames;

}

class Fraction;
class SvxFontItem;

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
namespace vcl { class Window; }
class XColorList;

struct ScAttrEntry;
class ScAutoFormatData;
class ScBroadcastAreaSlotMachine;
class ScChangeViewSettings;
class ScChartCollection;
class ScChartListenerCollection;
class ScClipOptions;
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
class ScSheetEvents;
class ScProgress;
class SvtListener;
class ScEditDataArray;
class EditTextObject;
struct ScRefCellValue;
class ScDocumentImport;
class ScPostIt;
struct ScSubTotalParam;
struct ScQueryParam;
class ScHint;
class SvtBroadcaster;
enum class ScDBDataPortion;
enum class ScSheetEventId;
class BitmapEx;

namespace sc {

typedef std::map<sal_Int32, BitmapEx> IconSetBitmapMap;

}

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

#define SC_DOC_NEW          0xFFFF

#define SC_MACROCALL_ALLOWED        0

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
    OUString  aDocName;
    SCTAB   nTableCount;
    sal_uLong   nCellCount;
    sal_uLong   nFormulaCount;
    sal_uInt16  nPageCount;
    ScDocStat()
        : nTableCount(0)
        , nCellCount(0)
        , nFormulaCount(0)
        , nPageCount(0)
    {
    }
};

// DDE link modes
const sal_uInt8 SC_DDE_DEFAULT       = 0;
const sal_uInt8 SC_DDE_ENGLISH       = 1;
const sal_uInt8 SC_DDE_TEXT          = 2;
const sal_uInt8 SC_DDE_IGNOREMODE    = 255;       /// For usage in FindDdeLink() only!

class ScDocument
{
friend class ScValueIterator;
friend class ScHorizontalValueIterator;
friend class ScDBQueryDataIterator;
friend class ScFormulaGroupIterator;
friend class ScCellIterator;
friend class ScQueryCellIterator;
friend class ScHorizontalCellIterator;
friend class ScHorizontalAttrIterator;
friend class ScDocAttrIterator;
friend class ScAccessibleTableBase;
friend class ScAttrRectIterator;
friend class ScDocShell;
friend class ScDocRowHeightUpdater;
friend class ScColumnTextWidthIterator;
friend class ScFormulaCell;
friend class ScTable;
friend class ScColumn;
friend struct ScRefCellValue;
friend class ScDocumentImport;
friend class sc::DocumentStreamAccess;
friend class sc::ColumnSpanSet;
friend class sc::EditTextIterator;
friend class sc::FormulaGroupAreaListener;

    typedef std::vector<ScTable*> TableContainer;

public:
    enum HardRecalcState
    {
        HARDRECALCSTATE_OFF = 0,    /// normal calculation of dependencies
        HARDRECALCSTATE_TEMPORARY,  /// CalcAll() without broadcast/notify but setting up new listeners
        HARDRECALCSTATE_ETERNAL     /// no new listeners are setup, no broadcast/notify
    };

private:

    rtl::Reference<ScPoolHelper> xPoolHelper;

    std::shared_ptr<svl::SharedStringPool> mpCellStringPool;
    std::shared_ptr<sc::FormulaGroupContext> mpFormulaGroupCxt;
    mutable std::unique_ptr<sc::DocumentLinkManager> mpDocLinkMgr;

    ScCalcConfig        maCalcConfig;

    SfxUndoManager*     mpUndoManager;
    ScFieldEditEngine*  pEditEngine;                    // uses pEditPool from xPoolHelper
    ScNoteEditEngine*   pNoteEngine;                    // uses pEditPool from xPoolHelper
    SfxObjectShell*     pShell;
    VclPtr<SfxPrinter>    pPrinter;
    VclPtr<VirtualDevice> pVirtualDevice_100th_mm;
    ScDrawLayer*        pDrawLayer;                     // SdrModel
    rtl::Reference<XColorList> pColorList;
    ScValidationDataList* pValidationList;              // validity
    SvNumberFormatterIndexTable*    pFormatExchangeList;    // for application of number formats
    TableContainer maTabs;
    std::vector<OUString> maTabNames;               // for undo document, we need the information tab name <-> index
    mutable ScRangeName* pRangeName;
    ScDBCollection*     pDBCollection;
    ScDPCollection*     pDPCollection;
    ScChartCollection*  pChartCollection;
    std::unique_ptr< ScTemporaryChartLock > apTemporaryChartLock;
    ScPatternAttr*      pSelectionAttr;                 // Attributes of a block
    ScFormulaCell*      pFormulaTree;                   // formula tree (start)
    ScFormulaCell*      pEOFormulaTree;                 // formula tree (end), last cell
    ScFormulaCell*      pFormulaTrack;                  // BroadcastTrack (start)
    ScFormulaCell*      pEOFormulaTrack;                // BroadcastTrack (end), last cell
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
    ScDBData*           mpAnonymousDBData;

    ScFieldEditEngine*  pCacheFieldEditEngine;

    std::unique_ptr<ScDocProtection> pDocProtection;
    std::unique_ptr<ScClipParam>     mpClipParam;

    std::unique_ptr<ScExternalRefManager> pExternalRefMgr;
    std::unique_ptr<ScMacroManager> mpMacroMgr;

    // mutable for lazy construction
    mutable std::unique_ptr< ScFormulaParserPool >
                        mxFormulaParserPool;            /// Pool for all external formula parsers used by this document.

    OUString       aDocName;                       // optional: name of document
    OUString       aDocCodeName;                       // optional: name of document (twice?)
    OUString       maFileURL;      // file URL for copy & paste
    ScRangePairListRef  xColNameRanges;
    ScRangePairListRef  xRowNameRanges;

    ScViewOptions*      pViewOptions;                   // view options
    ScDocOptions*       pDocOptions;                    // document options
    ScExtDocOptions*    pExtDocOptions;                 // for import etc.
    std::unique_ptr<ScClipOptions> mpClipOptions;       // clipboard options
    ScConsolidateParam* pConsolidateDlgData;

    ScRecursionHelper*  pRecursionHelper;               // information for recursive and iterative cell formulas

    ScAutoNameCache*    pAutoNameCache;                 // for automatic name lookup during CompileXML

    ScLookupCacheMapImpl* pLookupCacheMapImpl;          // cache for lookups like VLOOKUP and MATCH

    SfxItemSet*         pPreviewFont; // convert to std::unique_ptr or whatever
    ScStyleSheet*       pPreviewCellStyle;
    ScMarkData          maPreviewSelection;
    sal_Int64           nUnoObjectId;                   // counted up for UNO objects

    sal_uInt32          nRangeOverflowType;             // used in (xml) loading for overflow warnings

    ScRange             aEmbedRange;
    ScAddress           aCurTextWidthCalcPos;

    Idle                aTrackIdle;

    css::uno::Reference< css::script::vba::XVBAEventProcessor >
                        mxVbaEvents;
public:
    /// list of ScInterpreterTableOpParams currently in use
    std::vector<std::unique_ptr<ScInterpreterTableOpParams>> m_TableOpList;
    ScInterpreterTableOpParams  aLastTableOpParams;     // remember last params
private:

    LanguageType        eLanguage;                      // default language
    LanguageType        eCjkLanguage;                   // default language for asian text
    LanguageType        eCtlLanguage;                   // default language for complex text
    rtl_TextEncoding    eSrcSet;                        // during reading: source character set

    /** The compiler grammar used in document storage. GRAM_PODF for ODF 1.1
        documents, GRAM_ODFF for ODF 1.2 documents. */
    formula::FormulaGrammar::Grammar  eStorageGrammar;

    sal_uLong               nFormulaCodeInTree;             // FormelRPN im Formelbaum
    sal_uLong               nXMLImportedFormulaCount;        // progress count during XML import
    sal_uInt16              nInterpretLevel;                // >0 if in interpreter
    sal_uInt16              nMacroInterpretLevel;           // >0 if macro in interpreter
    sal_uInt16              nInterpreterTableOpLevel;       // >0 if in interpreter TableOp
    sal_uInt16              nSrcVer;                        // file version (load/save)
    sal_uInt16              nFormulaTrackCount;
    HardRecalcState         eHardRecalcState;               // off, temporary, eternal
    SCTAB                   nVisibleTab;                    // for OLE etc., don't use inside ScDocument

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

    // no broadcast, construct no listener during insert from a different
    // Doc (per filter or the like ), not until CompileAll / CalcAfterLoad
    bool                bInsertingFromOtherDoc;
    bool                bLoadingMedium;
    bool                bImportingXML;      // special handling of formula text
    bool                bCalcingAfterLoad;              // in CalcAfterLoad TRUE
    // don't construct/destruct listeners temporarily
    bool                bNoListening;
    bool                mbIdleEnabled;
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

    bool                bHasMacroFunc;      // valid only after loading

    sal_uInt8               nAsianCompression;
    sal_uInt8               nAsianKerning;

    bool                bPastingDrawFromOtherDoc;

    sal_uInt8                nInDdeLinkUpdate;   // originating DDE links (stacked bool)

    bool                bInUnoBroadcast;
    bool                bInUnoListenerCall;
    formula::FormulaGrammar::Grammar  eGrammar;

    mutable bool        bStyleSheetUsageInvalid;

    bool                mbUndoEnabled:1;
    bool                mbAdjustHeightEnabled:1;
    bool                mbExecuteLinkEnabled:1;
    bool                mbChangeReadOnlyEnabled:1;    // allow changes in read-only document (for API import filters)
    bool                mbStreamValidLocked:1;
    bool                mbUserInteractionEnabled:1;  // whether or not to launch any kind of interactive dialogs.

    sal_Int16           mnNamedRangesLockCount;

    std::set<ScFormulaCell*> maSubTotalCells;

    bool                mbUseEmbedFonts;

    std::unique_ptr<sc::IconSetBitmapMap> m_pIconSetBitmapMap;

public:
    bool IsCellInChangeTrack(const ScAddress &cell,Color *pColCellBoder);
    void GetCellChangeTrackNote(const ScAddress &cell, OUString &strTrackText, bool &pbLeftEdge);
    bool              IsUsingEmbededFonts() { return mbUseEmbedFonts; }
    void              SetIsUsingEmbededFonts( bool bUse ) { mbUseEmbedFonts = bUse; }
    SC_DLLPUBLIC sal_uLong          GetCellCount() const;       // all cells
    SC_DLLPUBLIC sal_uLong          GetFormulaGroupCount() const;       // all cells
    sal_uLong           GetCodeCount() const;       // RPN-Code in formulas
    DECL_LINK_TYPED( GetUserDefinedColor, sal_uInt16, Color* );
                                                                // number formatter
public:
    SC_DLLPUBLIC                ScDocument( ScDocumentMode eMode = SCDOCMODE_DOCUMENT,
                                SfxObjectShell* pDocShell = nullptr );
    SC_DLLPUBLIC                ~ScDocument();

    void            SetName( const OUString& r ) { aDocName = r; }
    const OUString&   GetCodeName() const { return aDocCodeName; }
    void                SetCodeName( const OUString& r ) { aDocCodeName = r; }
    const OUString& GetFileURL() const { return maFileURL; }

    void            GetDocStat( ScDocStat& rDocStat );

    SC_DLLPUBLIC void  InitDrawLayer( SfxObjectShell* pDocShell = nullptr );
    rtl::Reference<XColorList>          GetColorList();

    SC_DLLPUBLIC sfx2::LinkManager* GetLinkManager();
    SC_DLLPUBLIC const sfx2::LinkManager* GetLinkManager() const;

    sc::DocumentLinkManager& GetDocLinkManager();
    const sc::DocumentLinkManager& GetDocLinkManager() const;

    SC_DLLPUBLIC const ScDocOptions&        GetDocOptions() const;
    SC_DLLPUBLIC void                   SetDocOptions( const ScDocOptions& rOpt );
    SC_DLLPUBLIC const ScViewOptions&   GetViewOptions() const;
    SC_DLLPUBLIC void                   SetViewOptions( const ScViewOptions& rOpt );
    void                    SetPrintOptions();

    ScExtDocOptions*        GetExtDocOptions()  { return pExtDocOptions; }
    SC_DLLPUBLIC void                   SetExtDocOptions( ScExtDocOptions* pNewOptions );

    ScClipOptions*          GetClipOptions()    { return mpClipOptions.get(); }
    void                    SetClipOptions(const ScClipOptions& rClipOptions);

    SC_DLLPUBLIC void       GetLanguage( LanguageType& rLatin, LanguageType& rCjk, LanguageType& rCtl ) const;
    void                    SetLanguage( LanguageType eLatin, LanguageType eCjk, LanguageType eCtl );

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
    SC_DLLPUBLIC void SetAllRangeNames(const std::map<OUString, std::unique_ptr<ScRangeName>>& rRangeMap);
    SC_DLLPUBLIC void GetTabRangeNameMap(std::map<OUString, ScRangeName*>& rRangeName);
    SC_DLLPUBLIC void GetRangeNameMap(std::map<OUString, ScRangeName*>& rRangeName);
    SC_DLLPUBLIC ScRangeName* GetRangeName(SCTAB nTab) const;
    SC_DLLPUBLIC ScRangeName* GetRangeName() const;
    void SetRangeName(SCTAB nTab, ScRangeName* pNew);
    void SetRangeName( ScRangeName* pNewRangeName );

    /** Find a named expression / range name in either global or a local scope.
        @param  nTab
                If <0 search nIndex in global scope, if >=0 search nIndex in scope of nTab.
        @param  nIndex
                Index of named expression / range name.
        @return nullptr if indexed name not found.
     */
    ScRangeData* FindRangeNameBySheetAndIndex( SCTAB nTab, sal_uInt16 nIndex ) const;

    /** Recursively find all named expressions that directly or indirectly
        (nested) reference a given sheet, starting from a given named
        expression nTokenTab/nTokenIndex.

        Used to collect all named expressions/ranges that will need to be
        copied along when copying sheets.

        The different tab/sheets passed cater for the situation that a sheet is
        copied and was already inserted and global names are already adjusted
        but the sheet-local names of the shifted original sheet are not yet. If
        no sheet was inserted and global and local names' references not
        updated yet, then all 4 tab arguments would be identical.

        @param  nTokenTab
                Tab/sheet on which to find the name, -1 if global scope.
                For example obtained from ocName token.

        @param  nTokenIndex
                Index of named expression. For example obtained from ocName token.

        @param  nGlobalRefTab
                Tab to check if used in global names.

        @param  nLocalRefTab
                Tab to check if used in sheet-local names.

        @param  nOldTokenTab
                The original tab of the copied sheet, used as sheet-local
                base position for relative references.

        @param  nOldTokenTabReplacement
                The replacement to use for relative references if the name
                encountered uses nOldTokenTab as base position.

        @param  bSameDoc
                FALSE if collecting names for a sheet to be copied to another
                document. Then all names encountered are considered to be
                referencing the sheet. Else TRUE if collecting names to be
                copied into the same document.

        @param  nRecursion
                Recursion guard, initialize with 0.
      */
    bool FindRangeNamesReferencingSheet( sc::UpdatedRangeNames& rIndexes,
            SCTAB nTokenTab, const sal_uInt16 nTokenIndex,
            SCTAB nGlobalRefTab, SCTAB nLocalRefTab, SCTAB nOldTokenTab, SCTAB nOldTokenTabReplacement,
            bool bSameDoc, int nRecursion ) const;

    /** If necessary (name references sheet rOldPos.Tab()) copy and adjust
        named expression/range from sheet-local to sheet-local, or global to
        sheet-local if bGlobalNamesToLocal==true.

        Also copies nested names and adjusts the ocName tokens of the calling name.

        @param  rSheet
                On entry, the original sheet of the named expression/range, <0 global.
                On return TRUE, the new sheet. Else unchanged.

        @param  rIndex
                On entry, the original index of the named expression/range.
                On return TRUE, the new index, or 0 if a new copied name couldn't be inserted. Else unchanged.

        @param  rpRangeData
                On entry, the pointer to the original named expression/range.
                On return TRUE, the pointer to the new copied name, or nullptr if hit shappened.

        @param  rNewPos
                New position of formula cell if called for that, else new base
                position of a to be created new name adjusted for Tab.
                rNewPos.nTab MUST point to the new sheet copied to.

        @param  rOldPos
                Old position of formula cell if called for that, else base
                position of the existing name adjusted for Tab.
                rOldPos.nTab MUST point to the old sheet copied from.

        @param  bGlobalNamesToLocal
                If TRUE, affected global names are copied to sheet-local names.
                If FALSE, global names are copied to global names in another document.

        @param  bUsedByFormula
                If TRUE, forces a global name to be affected/used.
                If FALSE, a global name is only affected if it evaluates to be
                referencing the sheet.

        @return TRUE if copied and caller may need to evaluate rpRangeData and rSheet and rIndex.
                FALSE if nothing to be done.
     */
    bool CopyAdjustRangeName( SCTAB& rSheet, sal_uInt16& rIndex, ScRangeData*& rpRangeData, ScDocument& rNewDoc,
            const ScAddress& rNewPos, const ScAddress& rOldPos, const bool bGlobalNamesToLocal,
            const bool bUsedByFormula ) const;

    /**
     * Call this immediately before updating all named ranges.
     */
    SC_DLLPUBLIC void PreprocessRangeNameUpdate();
    SC_DLLPUBLIC void PreprocessDBDataUpdate();
    SC_DLLPUBLIC void CompileHybridFormula();

    /**
     * Insert a new named expression to the global scope.
     *
     * @param rName name for the expression.
     * @param rPos base position.
     * @param rExpr formula expression to be associated with the name.  The
     *              current grammar is used to compile this expression.
     *
     * @return true if inserted successfully, false otherwise.
     */
    bool InsertNewRangeName( const OUString& rName, const ScAddress& rPos, const OUString& rExpr );

    /**
     * Insert a new named expression to a sheet-local scope.
     *
     * @param nTab  sheet for local scope.
     * @param rName name for the expression.
     * @param rPos base position.
     * @param rExpr formula expression to be associated with the name.  The
     *              current grammar is used to compile this expression.
     *
     * @return true if inserted successfully, false otherwise.
     */
    bool InsertNewRangeName( SCTAB nTab, const OUString& rName, const ScAddress& rPos, const OUString& rExpr );

    SCTAB           GetMaxTableNumber() { return static_cast<SCTAB>(maTabs.size()) - 1; }

    ScRangePairList*    GetColNameRanges() { return &xColNameRanges; }
    ScRangePairList*    GetRowNameRanges() { return &xRowNameRanges; }
    ScRangePairListRef& GetColNameRangesRef() { return xColNameRanges; }
    ScRangePairListRef& GetRowNameRangesRef() { return xRowNameRanges; }

    SC_DLLPUBLIC ScDBCollection*    GetDBCollection() const { return pDBCollection;}
    void            SetDBCollection( ScDBCollection* pNewDBCollection,
                                        bool bRemoveAutoFilter = false );
    const ScDBData* GetDBAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab, ScDBDataPortion ePortion) const;
    ScDBData* GetDBAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab, ScDBDataPortion ePortion);
    const ScDBData* GetDBAtArea(SCTAB nTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2) const;
    ScDBData* GetDBAtArea(SCTAB nTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2);
    void RefreshDirtyTableColumnNames();

    SC_DLLPUBLIC const ScRangeData* GetRangeAtBlock( const ScRange& rBlock, OUString* pName=nullptr ) const;

    SC_DLLPUBLIC bool HasPivotTable() const;
    SC_DLLPUBLIC ScDPCollection*       GetDPCollection();
    SC_DLLPUBLIC const ScDPCollection* GetDPCollection() const;
    SC_DLLPUBLIC ScDPObject* GetDPAtCursor(SCCOL nCol, SCROW nRow, SCTAB nTab) const;
    ScDPObject*         GetDPAtBlock( const ScRange& rBlock ) const;

    SC_DLLPUBLIC ScChartCollection* GetChartCollection() const { return pChartCollection;}

    void                StopTemporaryChartLock();

    void            EnsureGraphicNames();

    SdrObject*      GetObjectAtPoint( SCTAB nTab, const Point& rPos );
    bool            HasChartAtPoint( SCTAB nTab, const Point& rPos, OUString& rName );

    css::uno::Reference< css::chart2::XChartDocument > GetChartByName( const OUString& rChartName );

    SC_DLLPUBLIC void            GetChartRanges( const OUString& rChartName, std::vector< ScRangeList >& rRanges, ScDocument* pSheetNameDoc );
    void            SetChartRanges( const OUString& rChartName, const std::vector< ScRangeList >& rRanges );

    void            UpdateChartArea( const OUString& rChartName, const ScRange& rNewArea,
                                        bool bColHeaders, bool bRowHeaders, bool bAdd );
    void            UpdateChartArea( const OUString& rChartName,
                                    const ScRangeListRef& rNewList,
                                    bool bColHeaders, bool bRowHeaders, bool bAdd );
    void            GetOldChartParameters( const OUString& rName,
                                    ScRangeList& rRanges, bool& rColHeaders, bool& rRowHeaders );
    css::uno::Reference<
            css::embed::XEmbeddedObject >
                    FindOleObjectByName( const OUString& rName );

    SC_DLLPUBLIC void           MakeTable( SCTAB nTab,bool _bNeedsNameCheck = true );

    SCTAB           GetVisibleTab() const       { return nVisibleTab; }
    SC_DLLPUBLIC void           SetVisibleTab(SCTAB nTab)   { nVisibleTab = nTab; }

    SC_DLLPUBLIC bool HasTable( SCTAB nTab ) const;
    SC_DLLPUBLIC bool GetHashCode( SCTAB nTab, sal_Int64& rHashCode) const;
    SC_DLLPUBLIC bool GetName( SCTAB nTab, OUString& rName ) const;
    SC_DLLPUBLIC bool GetCodeName( SCTAB nTab, OUString& rName ) const;
    SC_DLLPUBLIC bool SetCodeName( SCTAB nTab, const OUString& rName );
    SC_DLLPUBLIC bool GetTable( const OUString& rName, SCTAB& rTab ) const;

    SC_DLLPUBLIC std::vector<OUString> GetAllTableNames() const;

    OUString   GetCopyTabName(SCTAB nTab) const;

    SC_DLLPUBLIC void SetAnonymousDBData(SCTAB nTab, ScDBData* pDBData);
    SC_DLLPUBLIC ScDBData* GetAnonymousDBData(SCTAB nTab);

    /** One document global anonymous database range for temporary operations,
        used if the corresponding sheet-local anonymous database range is
        already used with AutoFilter and range differs. Not stored in document
        files. */
    SC_DLLPUBLIC void SetAnonymousDBData(ScDBData* pDBData);
    SC_DLLPUBLIC ScDBData* GetAnonymousDBData();

    SC_DLLPUBLIC SCTAB GetTableCount() const;
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
                                        bool* pOnlyNotBecauseOfMatrix = nullptr ) const;
    bool            IsSelectionEditable( const ScMarkData& rMark,
                                        bool* pOnlyNotBecauseOfMatrix = nullptr ) const;
    bool            HasSelectedBlockMatrixFragment( SCCOL nStartCol, SCROW nStartRow,
                                            SCCOL nEndCol, SCROW nEndRow,
                                            const ScMarkData& rMark ) const;

    SC_DLLPUBLIC bool GetMatrixFormulaRange( const ScAddress& rCellPos, ScRange& rMatrix );

    bool            IsEmbedded() const { return bIsEmbedded;}
    void            GetEmbedded( ScRange& rRange ) const;
    void            SetEmbedded( const ScRange& rRange );
    void            ResetEmbedded();
    Rectangle       GetEmbeddedRect() const;                        // 1/100 mm
    void            SetEmbedded( SCTAB nTab, const Rectangle& rRect );          // from VisArea (1/100 mm)

    static SC_DLLPUBLIC bool ValidTabName( const OUString& rName );

    SC_DLLPUBLIC bool           ValidNewTabName( const OUString& rName ) const;
    SC_DLLPUBLIC void           CreateValidTabName(OUString& rName) const;
    SC_DLLPUBLIC void           CreateValidTabNames(std::vector<OUString>& aNames, SCTAB nCount) const;

    void AppendTabOnLoad(const OUString& rName);
    void SetTabNameOnLoad(SCTAB nTab, const OUString& rName);
    void InvalidateStreamOnSave();

    SC_DLLPUBLIC bool InsertTab(
        SCTAB nPos, const OUString& rName, bool bExternalDocument = false, bool bUndoDeleteTab = false );

    SC_DLLPUBLIC bool           InsertTabs( SCTAB nPos, const std::vector<OUString>& rNames,
                                            bool bNamesValid = false );
    SC_DLLPUBLIC bool DeleteTabs( SCTAB nTab, SCTAB nSheets );
    SC_DLLPUBLIC bool DeleteTab( SCTAB nTab );
    SC_DLLPUBLIC bool           RenameTab( SCTAB nTab, const OUString& rName,
                                bool bUpdateRef = true,
                                bool bExternalDocument = false );
    bool                        MoveTab( SCTAB nOldPos, SCTAB nNewPos, ScProgress* pProgress = nullptr );
    bool                        CopyTab( SCTAB nOldPos, SCTAB nNewPos,
                                const ScMarkData* pOnlyMarked = nullptr );
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
    SC_DLLPUBLIC bool           IsNegativePage( SCTAB nTab ) const;
    SC_DLLPUBLIC void           SetScenario( SCTAB nTab, bool bFlag );
    SC_DLLPUBLIC bool           IsScenario( SCTAB nTab ) const;
    SC_DLLPUBLIC void           GetScenarioData(SCTAB nTab, OUString& rComment,
                                                Color& rColor, ScScenarioFlags &rFlags) const;
    SC_DLLPUBLIC void           SetScenarioData(SCTAB nTab, const OUString& rComment,
                                          const Color& rColor, ScScenarioFlags nFlags);
    SC_DLLPUBLIC Color GetTabBgColor( SCTAB nTab ) const;
    SC_DLLPUBLIC void SetTabBgColor( SCTAB nTab, const Color& rColor );
    SC_DLLPUBLIC bool IsDefaultTabBgColor( SCTAB nTab ) const;
    void                        GetScenarioFlags(SCTAB nTab, ScScenarioFlags &rFlags) const;
    SC_DLLPUBLIC bool           IsActiveScenario( SCTAB nTab ) const;
    SC_DLLPUBLIC void           SetActiveScenario( SCTAB nTab, bool bActive );      // only for Undo etc.
    SC_DLLPUBLIC formula::FormulaGrammar::AddressConvention GetAddressConvention() const;
    SC_DLLPUBLIC formula::FormulaGrammar::Grammar GetGrammar() const { return eGrammar;}
    SC_DLLPUBLIC void SetGrammar( formula::FormulaGrammar::Grammar eGram );
    SC_DLLPUBLIC ScLinkMode GetLinkMode( SCTAB nTab ) const;
    bool             IsLinked( SCTAB nTab ) const;
    SC_DLLPUBLIC const OUString  GetLinkDoc( SCTAB nTab ) const;
    const OUString   GetLinkFlt( SCTAB nTab ) const;
    const OUString   GetLinkOpt( SCTAB nTab ) const;
    SC_DLLPUBLIC const OUString  GetLinkTab( SCTAB nTab ) const;
    sal_uLong       GetLinkRefreshDelay( SCTAB nTab ) const;
    void            SetLink( SCTAB nTab, ScLinkMode nMode, const OUString& rDoc,
                            const OUString& rFilter, const OUString& rOptions,
                            const OUString& rTabName, sal_uLong nRefreshDelay );
    bool            HasLink( const OUString& rDoc,
                            const OUString& rFilter, const OUString& rOptions ) const;
    SC_DLLPUBLIC bool LinkExternalTab( SCTAB& nTab, const OUString& aDocTab,
                                    const OUString& aFileName,
                                    const OUString& aTabName );

    bool            HasExternalRefManager() const { return pExternalRefMgr.get(); }
    SC_DLLPUBLIC ScExternalRefManager* GetExternalRefManager() const;
    bool            IsInExternalReferenceMarking() const;
    void            MarkUsedExternalReferences();
    bool            MarkUsedExternalReferences( ScTokenArray& rArr, const ScAddress& rPos );

    /** Returns the pool containing external formula parsers. Creates the pool
        on first call. */
    ScFormulaParserPool& GetFormulaParserPool() const;

    bool            HasAreaLinks() const;
    void            UpdateExternalRefLinks(vcl::Window* pWin);
    void            UpdateAreaLinks();

                    // originating DDE links
    void            IncInDdeLinkUpdate() { if ( nInDdeLinkUpdate < 255 ) ++nInDdeLinkUpdate; }
    void            DecInDdeLinkUpdate() { if ( nInDdeLinkUpdate ) --nInDdeLinkUpdate; }
    bool            IsInDdeLinkUpdate() const   { return nInDdeLinkUpdate != 0; }

    SC_DLLPUBLIC void           CopyDdeLinks( ScDocument* pDestDoc ) const;

    /** Tries to find a DDE link with the specified connection data.
        @param rnDdePos  (out-param) Returns the index of the DDE link (does not include other links from link manager).
        @return  true = DDE link found, rnDdePos valid. */
    SC_DLLPUBLIC bool            FindDdeLink( const OUString& rAppl, const OUString& rTopic,
            const OUString& rItem, sal_uInt8 nMode, size_t& rnDdePos );

    /** Returns the connection data of the specified DDE link.
        @param nDdePos  Index of the DDE link (does not include other links from link manager).
        @param rAppl  (out-param) The application name.
        @param rTopic  (out-param) The DDE topic.
        @param rItem  (out-param) The DDE item.
        @return  true = DDE link found, out-parameters valid. */
    bool            GetDdeLinkData( size_t nDdePos, OUString& rAppl, OUString& rTopic, OUString& rItem ) const;
    /** Returns the link mode of the specified DDE link.
        @param nDdePos  Index of the DDE link (does not include other links from link manager).
        @param rnMode  (out-param) The link mode of the specified DDE link.
        @return  true = DDE link found, rnMode valid. */
    bool            GetDdeLinkMode( size_t nDdePos, sal_uInt8& rnMode ) const;
    /** Returns the result matrix of the specified DDE link.
        @param nDdePos  Index of the DDE link (does not include other links from link manager).
        @return  The result matrix, if the DDE link has been found, 0 otherwise. */
    SC_DLLPUBLIC const ScMatrix* GetDdeLinkResultMatrix( size_t nDdePos ) const;

    /** Tries to find a DDE link or creates a new, if not extant.
        @param pResults  If not 0, sets the matrix as DDE link result matrix (also for existing links).
        @return  true = DDE link found; false = Unpredictable error occurred, no DDE link created. */
    SC_DLLPUBLIC bool            CreateDdeLink( const OUString& rAppl, const OUString& rTopic, const OUString& rItem, sal_uInt8 nMode, const ScMatrixRef& pResults );
    /** Sets a result matrix for the specified DDE link.
        @param nDdePos  Index of the DDE link (does not include other links from link manager).
        @param pResults  The array containing all results of the DDE link (intrusive-ref-counted, do not delete).
        @return  true = DDE link found and matrix set. */
    bool            SetDdeLinkResultMatrix( size_t nDdePos, const ScMatrixRef& pResults );

    SfxBindings*    GetViewBindings();
    SfxObjectShell* GetDocumentShell() const    { return pShell; }
    SC_DLLPUBLIC ScDrawLayer* GetDrawLayer() { return pDrawLayer;  }
    SC_DLLPUBLIC const ScDrawLayer* GetDrawLayer() const { return pDrawLayer;  }
    SfxBroadcaster* GetDrawBroadcaster();       // to avoid header
    void            BeginDrawUndo();

    void            BeginUnoRefUndo();
    bool            HasUnoRefUndo() const       { return ( pUnoRefUndoList != nullptr ); }
    ScUnoRefList*   EndUnoRefUndo();            // must be deleted by caller!
    sal_Int64       GetNewUnoId() { return ++nUnoObjectId; }
    void            AddUnoRefChange( sal_Int64 nId, const ScRangeList& rOldRanges );

    static bool IsChart( const SdrObject* pObject );

    SC_DLLPUBLIC void           UpdateAllCharts();
    void            UpdateChartRef( UpdateRefMode eUpdateRefMode,
                                    SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                                    SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                                    SCsCOL nDx, SCsROW nDy, SCsTAB nDz );
                    //! only assigns the new RangeList, no ChartListener or the like
    void            SetChartRangeList( const OUString& rChartName,
                        const ScRangeListRef& rNewRangeListRef );

    void            StartAnimations( SCTAB nTab, vcl::Window* pWin );

    bool            HasBackgroundDraw( SCTAB nTab, const Rectangle& rMMRect ) const;
    bool            HasAnyDraw( SCTAB nTab, const Rectangle& rMMRect ) const;

    const ScSheetEvents* GetSheetEvents( SCTAB nTab ) const;
    void            SetSheetEvents( SCTAB nTab, const ScSheetEvents* pNew );
    bool            HasSheetEventScript( SCTAB nTab, ScSheetEventId nEvent, bool bWithVbaEvents = false ) const;
    bool            HasAnySheetEventScript( ScSheetEventId nEvent, bool bWithVbaEvents = false ) const;  // on any sheet

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

    SC_DLLPUBLIC void EnsureTable( SCTAB nTab );

                    //  return TRUE = number format is set
    SC_DLLPUBLIC bool SetString(
        SCCOL nCol, SCROW nRow, SCTAB nTab, const OUString& rString,
        ScSetStringParam* pParam = nullptr );
    SC_DLLPUBLIC bool SetString( const ScAddress& rPos, const OUString& rString, ScSetStringParam* pParam = nullptr );

    /**
     * This method manages the lifecycle of the passed edit text object. When
     * the text is successfully inserted, the cell takes over the ownership of
     * the text object. If not, the text object gets deleted.
     *
     * <p>The caller must ensure that the passed edit text object <i>uses the
     * SfxItemPool instance returned from ScDocument::GetEditPool()</i>.
     * This is very important.</p>
     */
    SC_DLLPUBLIC bool SetEditText( const ScAddress& rPos, EditTextObject* pEditText );

    void SetEditText( const ScAddress& rPos, const EditTextObject& rEditText, const SfxItemPool* pEditPool );

    SC_DLLPUBLIC void SetEditText( const ScAddress& rPos, const OUString& rStr );

    SC_DLLPUBLIC SCROW GetFirstEditTextRow( const ScRange& rRange ) const;

    /**
     * Call this if you are not sure whether to put this as an edit text or a
     * simple text.
     */
    SC_DLLPUBLIC void SetTextCell( const ScAddress& rPos, const OUString& rStr );

    void SetEmptyCell( const ScAddress& rPos );

    SC_DLLPUBLIC void SetValue( SCCOL nCol, SCROW nRow, SCTAB nTab, const double& rVal );
    SC_DLLPUBLIC void SetValue( const ScAddress& rPos, double fVal );

    void SetValues( const ScAddress& rPos, const std::vector<double>& rVals );

    void SetError( SCCOL nCol, SCROW nRow, SCTAB nTab, const sal_uInt16 nError);

    SC_DLLPUBLIC void SetFormula(
        const ScAddress& rPos, const ScTokenArray& rArray );

    SC_DLLPUBLIC void SetFormula(
        const ScAddress& rPos, const OUString& rFormula,
        formula::FormulaGrammar::Grammar eGram = formula::FormulaGrammar::GRAM_DEFAULT );

    /**
     * Set formula cell, and transfer its ownership to the document. This call
     * attempts to group the passed formula cell with the adjacent cells or
     * cell groups if appropriate.
     *
     * @return pCell if it was successfully inserted, NULL otherwise. pCell
     *         is deleted automatically on failure to insert.
     */
    SC_DLLPUBLIC ScFormulaCell* SetFormulaCell( const ScAddress& rPos, ScFormulaCell* pCell );

    bool SetFormulaCells( const ScAddress& rPos, std::vector<ScFormulaCell*>& rCells );

    /**
     * Check if there is at least one formula cell in specified range.
     */
    bool HasFormulaCell( const ScRange& rRange ) const;

    SC_DLLPUBLIC void InsertMatrixFormula(SCCOL nCol1, SCROW nRow1,
                                        SCCOL nCol2, SCROW nRow2,
                                        const ScMarkData& rMark,
                                        const OUString& rFormula,
                                        const ScTokenArray* p = nullptr,
                                        const formula::FormulaGrammar::Grammar = formula::FormulaGrammar::GRAM_DEFAULT );
    SC_DLLPUBLIC void InsertTableOp(const ScTabOpParam& rParam,   // multi-operation
                                  SCCOL nCol1, SCROW nRow1,
                                  SCCOL nCol2, SCROW nRow2, const ScMarkData& rMark);

    SC_DLLPUBLIC OUString GetString( SCCOL nCol, SCROW nRow, SCTAB nTab ) const;
    SC_DLLPUBLIC OUString GetString( const ScAddress& rPos ) const;

    /**
     * Return a pointer to the double value stored in value cell.
     *
     * @param rPos cell position
     *
     * @return pointer to the double value stored in a numeric cell, or NULL
     *         if the cell at specified position is not a numeric cell.
     */
    double* GetValueCell( const ScAddress& rPos );

    SC_DLLPUBLIC svl::SharedStringPool& GetSharedStringPool();
    const svl::SharedStringPool& GetSharedStringPool() const;

    svl::SharedString GetSharedString( const ScAddress& rPos ) const;

    std::shared_ptr<sc::FormulaGroupContext>& GetFormulaGroupContext();

    SC_DLLPUBLIC void GetInputString( SCCOL nCol, SCROW nRow, SCTAB nTab, OUString& rString );
    sal_uInt16 GetStringForFormula( const ScAddress& rPos, OUString& rString );
    SC_DLLPUBLIC double GetValue( const ScAddress& rPos ) const;
    SC_DLLPUBLIC double GetValue( SCCOL nCol, SCROW nRow, SCTAB nTab ) const;
    SC_DLLPUBLIC void GetValue( SCCOL nCol, SCROW nRow, SCTAB nTab, double& rValue ) const;
    SC_DLLPUBLIC const EditTextObject* GetEditText( const ScAddress& rPos ) const;
    void RemoveEditTextCharAttribs( const ScAddress& rPos, const ScPatternAttr& rAttr );
    SC_DLLPUBLIC double RoundValueAsShown( double fVal, sal_uInt32 nFormat ) const;
    SC_DLLPUBLIC void GetNumberFormat( SCCOL nCol, SCROW nRow, SCTAB nTab,
                                     sal_uInt32& rFormat ) const;
    sal_uInt32      GetNumberFormat( const ScRange& rRange ) const;
    SC_DLLPUBLIC sal_uInt32 GetNumberFormat( const ScAddress& ) const;
    void SetNumberFormat( const ScAddress& rPos, sal_uInt32 nNumberFormat );

    void GetNumberFormatInfo( short& nType, sal_uLong& nIndex, const ScAddress& rPos ) const;
    SC_DLLPUBLIC const ScFormulaCell* GetFormulaCell( const ScAddress& rPos ) const;
    SC_DLLPUBLIC ScFormulaCell* GetFormulaCell( const ScAddress& rPos );
    SC_DLLPUBLIC void           GetFormula( SCCOL nCol, SCROW nRow, SCTAB nTab, OUString& rFormula ) const;
    SC_DLLPUBLIC void           GetCellType( SCCOL nCol, SCROW nRow, SCTAB nTab, CellType& rCellType ) const;
    SC_DLLPUBLIC CellType       GetCellType( const ScAddress& rPos ) const;

    SC_DLLPUBLIC bool           HasData( SCCOL nCol, SCROW nRow, SCTAB nTab );
    SC_DLLPUBLIC bool           HasStringData( SCCOL nCol, SCROW nRow, SCTAB nTab ) const;
    SC_DLLPUBLIC bool           HasValueData( SCCOL nCol, SCROW nRow, SCTAB nTab ) const;
    SC_DLLPUBLIC bool HasValueData( const ScAddress& rPos ) const;
    bool            HasStringCells( const ScRange& rRange ) const;

    /** Returns true, if there is any data to create a selection list for rPos. */
    bool            HasSelectionData( SCCOL nCol, SCROW nRow, SCTAB nTab ) const;

    /** Notes **/
    SC_DLLPUBLIC ScPostIt*       GetNote(const ScAddress& rPos);
    SC_DLLPUBLIC ScPostIt*       GetNote(SCCOL nCol, SCROW nRow, SCTAB nTab);
    void                         SetNote(const ScAddress& rPos, ScPostIt* pNote);
    void                         SetNote(SCCOL nCol, SCROW nRow, SCTAB nTab, ScPostIt* pNote);
    SC_DLLPUBLIC bool HasNote(const ScAddress& rPos) const;
    bool HasNote(SCCOL nCol, SCROW nRow, SCTAB nTab) const;
    SC_DLLPUBLIC bool HasColNotes(SCCOL nCol, SCTAB nTab) const;
    SC_DLLPUBLIC bool HasTabNotes(SCTAB nTab) const;
    bool HasNotes() const;
    SC_DLLPUBLIC ScPostIt*       ReleaseNote(const ScAddress& rPos);
    SC_DLLPUBLIC ScPostIt*       GetOrCreateNote(const ScAddress& rPos);
    SC_DLLPUBLIC ScPostIt*       CreateNote(const ScAddress& rPos);
    size_t GetNoteCount( SCTAB nTab, SCCOL nCol ) const;

    /**
     * Ensure that all note objects have an associated sdr object.  The export
     * code uses sdr objects to export note data.
     */
    void CreateAllNoteCaptions();
    void ForgetNoteCaptions( const ScRangeList& rRanges );

    ScAddress GetNotePosition( size_t nIndex ) const;
    ScAddress GetNotePosition( size_t nIndex, SCTAB nTab ) const;
    SCROW GetNotePosition( SCTAB nTab, SCCOL nCol, size_t nIndex ) const;

    SC_DLLPUBLIC void GetAllNoteEntries( std::vector<sc::NoteEntry>& rNotes ) const;
    void GetNotesInRange( const ScRangeList& rRange, std::vector<sc::NoteEntry>& rNotes ) const;
    bool ContainsNotesInRange( const ScRangeList& rRange ) const;

    SC_DLLPUBLIC void            SetDrawPageSize(SCTAB nTab);

    bool IsMerged( const ScAddress& rPos ) const;

    void            ExtendMergeSel( SCCOL nStartCol, SCROW nStartRow,
                                SCCOL& rEndCol, SCROW& rEndRow, const ScMarkData& rMark,
                                bool bRefresh = false );
    SC_DLLPUBLIC bool            ExtendMerge( SCCOL nStartCol, SCROW nStartRow,
                                SCCOL& rEndCol, SCROW& rEndRow, SCTAB nTab,
                                bool bRefresh = false );
    bool            ExtendMerge( ScRange& rRange, bool bRefresh = false );
    void            ExtendTotalMerge( ScRange& rRange ) const;
    SC_DLLPUBLIC void           ExtendOverlapped( SCCOL& rStartCol, SCROW& rStartRow,
                                SCCOL nEndCol, SCROW nEndRow, SCTAB nTab ) const;
    SC_DLLPUBLIC void           ExtendOverlapped( ScRange& rRange ) const;

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
                                                ScRange* pLastRange = nullptr,
                                                Rectangle* pLastMM = nullptr ) const;

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

    void CheckVectorizationState();
    void SetAllFormulasDirty( const sc::SetFormulaDirtyContext& rCxt );
    void            SetDirty( const ScRange&, bool bIncludeEmptyCells );
    void            SetTableOpDirty( const ScRange& );  // for Interpreter TableOp
    void            InterpretDirtyCells( const ScRangeList& rRanges );
    SC_DLLPUBLIC void CalcAll();
    SC_DLLPUBLIC void CalcAfterLoad( bool bStartListening = true );
    void            CompileAll();
    void            CompileXML();

    /**
     * Re-compile formula cells with error.
     *
     * @param nErrCode specified error code to match. Only those cells with
     *                 this error code will be re-compiled.  If this value is
     *                 0, cells with any error values will be re-compiled.
     *
     * @return true if at least one cell is re-compiled, false if no cells are
     *         re-compiled.
     */
    bool CompileErrorCells(sal_uInt16 nErrCode);

    ScAutoNameCache* GetAutoNameCache()     { return pAutoNameCache; }
    void             SetPreviewFont( SfxItemSet* pFontSet );
    SfxItemSet*      GetPreviewFont() { return pPreviewFont; }
    SfxItemSet*      GetPreviewFont( SCCOL nCol, SCROW nRow, SCTAB nTab );
    const ScMarkData& GetPreviewSelection() const { return maPreviewSelection; }
    void             SetPreviewSelection( ScMarkData& rSel );
    ScStyleSheet*    GetPreviewCellStyle() { return pPreviewCellStyle; }
    ScStyleSheet*    GetPreviewCellStyle( SCCOL nCol, SCROW nRow, SCTAB nTab );
    void             SetPreviewCellStyle( ScStyleSheet* pStyle ) { pPreviewCellStyle = pStyle; }
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
    SC_DLLPUBLIC void SetAutoCalc( bool bNewAutoCalc );
    SC_DLLPUBLIC bool GetAutoCalc() const { return bAutoCalc; }
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
                        @param  bStickyTopRow
                                If TRUE, do not adjust the top row.
                        @param  bStickyLeftCol
                                If TRUE, do not adjust the left column.

                        @returns true if there is any data, false if not.
                     */
    bool            ShrinkToUsedDataArea( bool& o_bShrunk,
                                          SCTAB nTab, SCCOL& rStartCol, SCROW& rStartRow,
                                          SCCOL& rEndCol, SCROW& rEndRow, bool bColumnsOnly,
                                          bool bStickyTopRow = false, bool bStickyLeftCol = false ) const;

    /**
     * Return the last non-empty row position in given columns that's no
     * greater than the initial last row position, or 0 if the columns are
     * empty. A negative value is returned if the given sheet or column
     * positions are invalid.
     *
     * <p>It starts from the specified last row position, and finds the first
     * non-empty row position in the upward direction if the start row
     * position is empty.</p>
     */
    SCROW GetLastDataRow( SCTAB nTab, SCCOL nCol1, SCCOL nCol2, SCROW nLastRow ) const;

    /**
     * Return the smallest area containing at least all contiguous cells
     * having data. This area is a square containing also empty cells. It may
     * shrink or extend the area given as input Flags as modifiers:
     *
     * @param bIncludeOld when true, ensure that the returned area contains at
     *                   least the initial area even if the actual data area
     *                   is smaller than the initial area.
     *
     * @param bOnlyDown when true, extend / shrink the data area only in a
     *                  downward direction i.e. only modify the end row
     *                  position.
     */
    SC_DLLPUBLIC void GetDataArea(
        SCTAB nTab, SCCOL& rStartCol, SCROW& rStartRow, SCCOL& rEndCol, SCROW& rEndRow,
        bool bIncludeOld, bool bOnlyDown ) const;

    SC_DLLPUBLIC bool           GetCellArea( SCTAB nTab, SCCOL& rEndCol, SCROW& rEndRow ) const;
    SC_DLLPUBLIC bool           GetTableArea( SCTAB nTab, SCCOL& rEndCol, SCROW& rEndRow ) const;
    SC_DLLPUBLIC bool           GetPrintArea( SCTAB nTab, SCCOL& rEndCol, SCROW& rEndRow,
                                    bool bNotes = true ) const;
    SC_DLLPUBLIC bool           GetPrintAreaHor( SCTAB nTab, SCROW nStartRow, SCROW nEndRow,
                                        SCCOL& rEndCol, bool bNotes = true ) const;
    SC_DLLPUBLIC bool           GetPrintAreaVer( SCTAB nTab, SCCOL nStartCol, SCCOL nEndCol,
                                        SCROW& rEndRow, bool bNotes = true ) const;
    void            InvalidateTableArea();

    /// Return the number of columns / rows that should be visible for the tiled rendering.
    SC_DLLPUBLIC void           GetTiledRenderingArea(SCTAB nTab, SCCOL& rEndCol, SCROW& rEndRow) const;

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

    void           FindAreaPos( SCCOL& rCol, SCROW& rRow, SCTAB nTab, ScMoveDirection eDirection ) const;
    SC_DLLPUBLIC void           GetNextPos( SCCOL& rCol, SCROW& rRow, SCTAB nTab, SCsCOL nMovX, SCsROW nMovY,
                                bool bMarked, bool bUnprotected, const ScMarkData& rMark ) const;

    bool            GetNextMarkedCell( SCCOL& rCol, SCROW& rRow, SCTAB nTab,
                                        const ScMarkData& rMark );

    void            LimitChartArea( SCTAB nTab, SCCOL& rStartCol, SCROW& rStartRow,
                                                    SCCOL& rEndCol, SCROW& rEndRow );
    void            LimitChartIfAll( ScRangeListRef& rRangeList );

    bool            InsertRow( SCCOL nStartCol, SCTAB nStartTab,
                               SCCOL nEndCol,   SCTAB nEndTab,
                               SCROW nStartRow, SCSIZE nSize, ScDocument* pRefUndoDoc = nullptr,
                               const ScMarkData* pTabMark = nullptr );
    SC_DLLPUBLIC bool           InsertRow( const ScRange& rRange );
    void            DeleteRow( SCCOL nStartCol, SCTAB nStartTab,
                               SCCOL nEndCol,   SCTAB nEndTab,
                               SCROW nStartRow, SCSIZE nSize,
                               ScDocument* pRefUndoDoc = nullptr, bool* pUndoOutline = nullptr,
                               const ScMarkData* pTabMark = nullptr );
    SC_DLLPUBLIC void   DeleteRow( const ScRange& rRange );
    bool            InsertCol( SCROW nStartRow, SCTAB nStartTab,
                               SCROW nEndRow,   SCTAB nEndTab,
                               SCCOL nStartCol, SCSIZE nSize, ScDocument* pRefUndoDoc = nullptr,
                               const ScMarkData* pTabMark = nullptr );
    SC_DLLPUBLIC bool           InsertCol( const ScRange& rRange );
    void            DeleteCol( SCROW nStartRow, SCTAB nStartTab,
                               SCROW nEndRow, SCTAB nEndTab,
                               SCCOL nStartCol, SCSIZE nSize,
                               ScDocument* pRefUndoDoc = nullptr, bool* pUndoOutline = nullptr,
                               const ScMarkData* pTabMark = nullptr );
    void            DeleteCol( const ScRange& rRange );

    bool            CanInsertRow( const ScRange& rRange ) const;
    bool            CanInsertCol( const ScRange& rRange ) const;

    void            FitBlock( const ScRange& rOld, const ScRange& rNew, bool bClear = true );
    bool            CanFitBlock( const ScRange& rOld, const ScRange& rNew );

    bool            IsClipOrUndo() const                        { return bIsClip || bIsUndo; }
    bool            IsUndo() const                              { return bIsUndo; }
    bool            IsClipboard() const                         { return bIsClip; }
    bool            IsUndoEnabled() const                       { return mbUndoEnabled; }
    SC_DLLPUBLIC void EnableUndo( bool bVal );

    bool            IsAdjustHeightEnabled() const               { return mbAdjustHeightEnabled; }
    void            EnableAdjustHeight( bool bVal )             { mbAdjustHeightEnabled = bVal; }
    bool            IsExecuteLinkEnabled() const                { return mbExecuteLinkEnabled; }
    void            EnableExecuteLink( bool bVal )              { mbExecuteLinkEnabled = bVal; }
    bool            IsChangeReadOnlyEnabled() const             { return mbChangeReadOnlyEnabled; }
    void            EnableChangeReadOnly( bool bVal )           { mbChangeReadOnlyEnabled = bVal; }
    SC_DLLPUBLIC bool IsUserInteractionEnabled() const { return mbUserInteractionEnabled;}
    SC_DLLPUBLIC void EnableUserInteraction( bool bVal );
    SC_DLLPUBLIC sal_Int16       GetNamedRangesLockCount() const             { return mnNamedRangesLockCount; }
    void            SetNamedRangesLockCount( sal_Int16 nCount ) { mnNamedRangesLockCount = nCount; }
    SC_DLLPUBLIC void           ResetClip( ScDocument* pSourceDoc, const ScMarkData* pMarks );
    SC_DLLPUBLIC void           ResetClip( ScDocument* pSourceDoc, SCTAB nTab );
    void            SetCutMode( bool bCut );
    bool            IsCutMode();
    void            SetClipArea( const ScRange& rArea, bool bCut = false );

    SC_DLLPUBLIC bool           IsDocVisible() const                        { return bIsVisible; }
    void            SetDocVisible( bool bSet );

    bool            HasOLEObjectsInArea( const ScRange& rRange, const ScMarkData* pTabMark = nullptr );

    void            DeleteObjectsInArea( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                                        const ScMarkData& rMark );
    void            DeleteObjectsInSelection( const ScMarkData& rMark );

    void DeleteArea(
        SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, const ScMarkData& rMark,
        InsertDeleteFlags nDelFlag, bool bBroadcast = true,
        sc::ColumnSpanSet* pBroadcastSpans = nullptr );

    SC_DLLPUBLIC void DeleteAreaTab(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                                SCTAB nTab, InsertDeleteFlags nDelFlag);
    void            DeleteAreaTab(const ScRange& rRange, InsertDeleteFlags nDelFlag);

    void            CopyToClip(const ScClipParam& rClipParam, ScDocument* pClipDoc,
                               const ScMarkData* pMarks, bool bKeepScenarioFlags,
                               bool bIncludeObjects );

    /**
     * Copy only raw cell values to another document.  Formula cells are
     * converted to raw cells.  No formatting info are copied except for
     * number formats.
     *
     * @param rSrcRange source range in the source document
     * @param nDestTab table in the clip document to copy to.
     * @param pDestDoc document to copy to
     */
    SC_DLLPUBLIC void CopyStaticToDocument(const ScRange& rSrcRange, SCTAB nDestTab, ScDocument* pDestDoc);

    /**
     * Copy only cell, nothing but cell to another document.
     *
     * @param rSrcPos source cell position
     * @param rDestPos destination cell position
     * @param rDestDoc destination document
     */
    void CopyCellToDocument( const ScAddress& rSrcPos, const ScAddress& rDestPos, ScDocument& rDestDoc );

    void            CopyTabToClip(SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                                SCTAB nTab, ScDocument* pClipDoc = nullptr);

    bool InitColumnBlockPosition( sc::ColumnBlockPosition& rBlokPos, SCTAB nTab, SCCOL nCol );

    void DeleteBeforeCopyFromClip(
        sc::CopyFromClipContext& rCxt, const ScMarkData& rMark, sc::ColumnSpanSet& rBroadcastSpans );

    bool CopyOneCellFromClip(
        sc::CopyFromClipContext& rCxt, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2 );
    void CopyBlockFromClip(
        sc::CopyFromClipContext& rCxt, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
        const ScMarkData& rMark, SCsCOL nDx, SCsROW nDy );
    void CopyNonFilteredFromClip(
        sc::CopyFromClipContext& rCxt, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
        const ScMarkData& rMark, SCsCOL nDx, SCROW & rClipStartRow );

    void            StartListeningFromClip( SCCOL nCol1, SCROW nRow1,
                                        SCCOL nCol2, SCROW nRow2,
                                        const ScMarkData& rMark, InsertDeleteFlags nInsFlag );

    void SetDirtyFromClip(
        SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, const ScMarkData& rMark,
        InsertDeleteFlags nInsFlag, sc::ColumnSpanSet& rBroadcastSpans );

    /** If pDestRanges is given it overrides rDestRange, rDestRange in this
        case is the overall encompassing range. */
    void            CopyFromClip( const ScRange& rDestRange, const ScMarkData& rMark,
                                    InsertDeleteFlags nInsFlag,
                                    ScDocument* pRefUndoDoc = nullptr,
                                    ScDocument* pClipDoc = nullptr,
                                    bool bResetCut = true,
                                    bool bAsLink = false,
                                    bool bIncludeFiltered = true,
                                    bool bSkipAttrForEmpty = false,
                                    const ScRangeList * pDestRanges = nullptr );

    void            CopyMultiRangeFromClip(const ScAddress& rDestPos, const ScMarkData& rMark,
                                           InsertDeleteFlags nInsFlag, ScDocument* pClipDoc,
                                           bool bResetCut = true, bool bAsLink = false,
                                           bool bIncludeFiltered = true,
                                           bool bSkipAttrForEmpty = false);

    void            GetClipArea(SCCOL& nClipX, SCROW& nClipY, bool bIncludeFiltered);
    void            GetClipStart(SCCOL& nClipX, SCROW& nClipY);

    bool            HasClipFilteredRows();

    bool            IsClipboardSource() const;

    SC_DLLPUBLIC void           TransposeClip( ScDocument* pTransClip, InsertDeleteFlags nFlags, bool bAsLink );

    ScClipParam&    GetClipParam();
    void            SetClipParam(const ScClipParam& rParam);

    void            MixDocument( const ScRange& rRange, ScPasteFunc nFunction, bool bSkipEmpty,
                                    ScDocument* pSrcDoc );

    void            FillTab( const ScRange& rSrcArea, const ScMarkData& rMark,
                                InsertDeleteFlags nFlags, ScPasteFunc nFunction,
                                bool bSkipEmpty, bool bAsLink );
    void            FillTabMarked( SCTAB nSrcTab, const ScMarkData& rMark,
                                InsertDeleteFlags nFlags, ScPasteFunc nFunction,
                                bool bSkipEmpty, bool bAsLink );

    void            TransliterateText( const ScMarkData& rMultiMark, sal_Int32 nType );

    SC_DLLPUBLIC void           InitUndo( ScDocument* pSrcDoc, SCTAB nTab1, SCTAB nTab2,
                                bool bColInfo = false, bool bRowInfo = false );
    void            AddUndoTab( SCTAB nTab1, SCTAB nTab2,
                                bool bColInfo = false, bool bRowInfo = false );
    SC_DLLPUBLIC void           InitUndoSelected( ScDocument* pSrcDoc, const ScMarkData& rTabSelection,
                                bool bColInfo = false, bool bRowInfo = false );

                    //  don't use anymore:
    void            CopyToDocument(SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                                SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                                InsertDeleteFlags nFlags, bool bMarked, ScDocument* pDestDoc,
                                const ScMarkData* pMarks = nullptr, bool bColRowFlags = true);
    void            UndoToDocument(SCCOL nCol1, SCROW nRow1, SCTAB nTab1,
                                SCCOL nCol2, SCROW nRow2, SCTAB nTab2,
                                InsertDeleteFlags nFlags, bool bMarked, ScDocument* pDestDoc);

    void            CopyToDocument(const ScRange& rRange,
                                InsertDeleteFlags nFlags, bool bMarked, ScDocument* pDestDoc,
                                const ScMarkData* pMarks = nullptr, bool bColRowFlags = true);
    void            UndoToDocument(const ScRange& rRange,
                                InsertDeleteFlags nFlags, bool bMarked, ScDocument* pDestDoc);

    void            CopyScenario( SCTAB nSrcTab, SCTAB nDestTab, bool bNewScenario = false );
    bool            TestCopyScenario( SCTAB nSrcTab, SCTAB nDestTab ) const;
    void            MarkScenario(SCTAB nSrcTab, SCTAB nDestTab,
                                ScMarkData& rDestMark, bool bResetMark = true,
                                ScScenarioFlags nNeededBits = ScScenarioFlags::NONE) const;
    bool            HasScenarioRange( SCTAB nTab, const ScRange& rRange ) const;
    SC_DLLPUBLIC const ScRangeList* GetScenarioRanges( SCTAB nTab ) const;

    SC_DLLPUBLIC void           CopyUpdated( ScDocument* pPosDoc, ScDocument* pDestDoc );

    void UpdateReference(
        sc::RefUpdateContext& rCxt,  ScDocument* pUndoDoc = nullptr, bool bIncludeDraw = true,
        bool bUpdateNoteCaptionPos = true );

    SC_DLLPUBLIC void           UpdateTranspose( const ScAddress& rDestPos, ScDocument* pClipDoc,
                                        const ScMarkData& rMark, ScDocument* pUndoDoc = nullptr );

    void            UpdateGrow( const ScRange& rArea, SCCOL nGrowX, SCROW nGrowY );

    SC_DLLPUBLIC void Fill(   SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                            ScProgress* pProgress, const ScMarkData& rMark,
                            sal_uLong nFillCount, FillDir eFillDir = FILL_TO_BOTTOM,
                            FillCmd eFillCmd = FILL_LINEAR, FillDateCmd eFillDateCmd = FILL_DAY,
                            double nStepValue = 1.0, double nMaxValue = 1E307 );
    OUString          GetAutoFillPreview( const ScRange& rSource, SCCOL nEndX, SCROW nEndY );

    bool            GetSelectionFunction( ScSubTotalFunc eFunc,
                                            const ScAddress& rCursor, const ScMarkData& rMark,
                                            double& rResult );

    SC_DLLPUBLIC const SfxPoolItem*         GetAttr( SCCOL nCol, SCROW nRow, SCTAB nTab, sal_uInt16 nWhich ) const;
    SC_DLLPUBLIC const SfxPoolItem* GetAttr( const ScAddress& rPos, sal_uInt16 nWhich ) const;
    SC_DLLPUBLIC const ScPatternAttr*   GetPattern( SCCOL nCol, SCROW nRow, SCTAB nTab ) const;
    SC_DLLPUBLIC const ScPatternAttr* GetPattern( const ScAddress& rPos ) const;
    SC_DLLPUBLIC const ScPatternAttr*    GetMostUsedPattern( SCCOL nCol, SCROW nStartRow, SCROW nEndRow, SCTAB nTab ) const;
    const ScPatternAttr*    GetSelectionPattern( const ScMarkData& rMark );
    ScPatternAttr*          CreateSelectionPattern( const ScMarkData& rMark, bool bDeep = true );
    SC_DLLPUBLIC void AddCondFormatData( const ScRangeList& rRange, SCTAB nTab, sal_uInt32 nIndex );
    void RemoveCondFormatData( const ScRangeList& rRange, SCTAB nTab, sal_uInt32 nIndex );

    SC_DLLPUBLIC ScConditionalFormat* GetCondFormat( SCCOL nCol, SCROW nRow, SCTAB nTab ) const;
    SC_DLLPUBLIC const SfxItemSet*  GetCondResult( SCCOL nCol, SCROW nRow, SCTAB nTab ) const;
    const SfxItemSet* GetCondResult(
        ScRefCellValue& rCell, const ScAddress& rPos, const ScConditionalFormatList& rList,
        const std::vector<sal_uInt32>& rIndex ) const;
    const SfxPoolItem*  GetEffItem( SCCOL nCol, SCROW nRow, SCTAB nTab, sal_uInt16 nWhich ) const;

    SC_DLLPUBLIC const css::uno::Reference< css::i18n::XBreakIterator >& GetBreakIterator();
    bool            HasStringWeakCharacters( const OUString& rString );
    SC_DLLPUBLIC SvtScriptType  GetStringScriptType( const OUString& rString );
    SC_DLLPUBLIC SvtScriptType  GetCellScriptType( const ScAddress& rPos, sal_uLong nNumberFormat );
    SC_DLLPUBLIC SvtScriptType  GetScriptType( SCCOL nCol, SCROW nRow, SCTAB nTab );
    SvtScriptType   GetRangeScriptType(
        sc::ColumnBlockPosition& rBlockPos, const ScAddress& rPos, SCROW nLength );
    SvtScriptType   GetRangeScriptType( const ScRangeList& rRanges );

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

    void SetCondFormList( ScConditionalFormatList* pList, SCTAB nTab );

    SC_DLLPUBLIC sal_uLong          AddValidationEntry( const ScValidationData& rNew );

    SC_DLLPUBLIC const ScValidationData*    GetValidationEntry( sal_uLong nIndex ) const;

    SC_DLLPUBLIC ScConditionalFormatList* GetCondFormList( SCTAB nTab ) const;

    const ScValidationDataList* GetValidationList() const { return pValidationList;}
    ScValidationDataList* GetValidationList() { return pValidationList;}

    SC_DLLPUBLIC void           ApplyAttr( SCCOL nCol, SCROW nRow, SCTAB nTab,
                                const SfxPoolItem& rAttr );
    SC_DLLPUBLIC void           ApplyPattern( SCCOL nCol, SCROW nRow, SCTAB nTab,
                                    const ScPatternAttr& rAttr );
    SC_DLLPUBLIC void           ApplyPatternArea( SCCOL nStartCol, SCROW nStartRow,
                                        SCCOL nEndCol, SCROW nEndRow,
                                        const ScMarkData& rMark, const ScPatternAttr& rAttr,
                                        ScEditDataArray* pDataArray = nullptr );
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

    SC_DLLPUBLIC void            ApplySelectionStyle( const ScStyleSheet& rStyle, const ScMarkData& rMark );
    void            ApplySelectionLineStyle( const ScMarkData& rMark,
                                            const ::editeng::SvxBorderLine* pLine, bool bColorOnly );

    const ScStyleSheet* GetStyle( SCCOL nCol, SCROW nRow, SCTAB nTab ) const;
    const ScStyleSheet* GetSelectionStyle( const ScMarkData& rMark ) const;

    void            StyleSheetChanged( const SfxStyleSheetBase* pStyleSheet, bool bRemoved,
                                        OutputDevice* pDev,
                                        double nPPTX, double nPPTY,
                                        const Fraction& rZoomX, const Fraction& rZoomY );

    bool            IsStyleSheetUsed( const ScStyleSheet& rStyle ) const;

    SC_DLLPUBLIC bool           ApplyFlagsTab( SCCOL nStartCol, SCROW nStartRow,
                                            SCCOL nEndCol, SCROW nEndRow,
                                            SCTAB nTab, ScMF nFlags );
    SC_DLLPUBLIC bool           RemoveFlagsTab( SCCOL nStartCol, SCROW nStartRow,
                                            SCCOL nEndCol, SCROW nEndRow,
                                            SCTAB nTab, ScMF nFlags );

    SC_DLLPUBLIC void           SetPattern( const ScAddress&, const ScPatternAttr& rAttr );
    SC_DLLPUBLIC void           SetPattern( SCCOL nCol, SCROW nRow, SCTAB nTab, const ScPatternAttr& rAttr );

    void            AutoFormat( SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                                    sal_uInt16 nFormatNo, const ScMarkData& rMark );
    void            GetAutoFormatData( SCTAB nTab, SCCOL nStartCol, SCROW nStartRow, SCCOL nEndCol, SCROW nEndRow,
                                        ScAutoFormatData& rData );
    bool            SearchAndReplace( const SvxSearchItem& rSearchItem,
                                      SCCOL& rCol, SCROW& rRow, SCTAB& rTab,
                                      const ScMarkData& rMark, ScRangeList& rMatchedRanges,
                                      OUString& rUndoStr, ScDocument* pUndoDoc = nullptr );

                    // determine Col/Row of subsequent calls
                    // (e.g. not found from the beginning, or subsequent tables)
                    // meaning of explanation in "()" was already unclear in German
    static void     GetSearchAndReplaceStart( const SvxSearchItem& rSearchItem,
                        SCCOL& rCol, SCROW& rRow );

    bool            Solver(SCCOL nFCol, SCROW nFRow, SCTAB nFTab,
                            SCCOL nVCol, SCROW nVRow, SCTAB nVTab,
                            const OUString& sValStr, double& nX);

    SC_DLLPUBLIC void            ApplySelectionPattern( const ScPatternAttr& rAttr, const ScMarkData& rMark,
                                           ScEditDataArray* pDataArray = nullptr );
    void DeleteSelection( InsertDeleteFlags nDelFlag, const ScMarkData& rMark, bool bBroadcast = true );
    void DeleteSelectionTab( SCTAB nTab, InsertDeleteFlags nDelFlag, const ScMarkData& rMark );

    SC_DLLPUBLIC void           SetColWidth( SCCOL nCol, SCTAB nTab, sal_uInt16 nNewWidth );
    SC_DLLPUBLIC void           SetColWidthOnly( SCCOL nCol, SCTAB nTab, sal_uInt16 nNewWidth );
    SC_DLLPUBLIC void           SetRowHeight( SCROW nRow, SCTAB nTab, sal_uInt16 nNewHeight );
    SC_DLLPUBLIC void           SetRowHeightRange( SCROW nStartRow, SCROW nEndRow, SCTAB nTab,
                                            sal_uInt16 nNewHeight );

    SC_DLLPUBLIC void           SetRowHeightOnly( SCROW nStartRow, SCROW nEndRow, SCTAB nTab,
                                                  sal_uInt16 nNewHeight );
    SC_DLLPUBLIC void           SetManualHeight( SCROW nStartRow, SCROW nEndRow, SCTAB nTab, bool bManual );

    SC_DLLPUBLIC sal_uInt16         GetColWidth( SCCOL nCol, SCTAB nTab, bool bHiddenAsZero = true ) const;
    SC_DLLPUBLIC sal_uLong GetColWidth( SCCOL nStartCol, SCCOL nEndCol, SCTAB nTab ) const;
    SC_DLLPUBLIC sal_uInt16         GetRowHeight( SCROW nRow, SCTAB nTab, bool bHiddenAsZero = true ) const;
    SC_DLLPUBLIC sal_uInt16         GetRowHeight( SCROW nRow, SCTAB nTab, SCROW* pStartRow, SCROW* pEndRow ) const;
    SC_DLLPUBLIC sal_uLong          GetRowHeight( SCROW nStartRow, SCROW nEndRow, SCTAB nTab, bool bHiddenAsZero = true ) const;
    SCROW                       GetRowForHeight( SCTAB nTab, sal_uLong nHeight ) const;
    sal_uLong                       GetScaledRowHeight( SCROW nStartRow, SCROW nEndRow, SCTAB nTab, double fScale ) const;
    SC_DLLPUBLIC sal_uLong          GetColOffset( SCCOL nCol, SCTAB nTab, bool bHiddenAsZero = true ) const;
    SC_DLLPUBLIC sal_uLong          GetRowOffset( SCROW nRow, SCTAB nTab, bool bHiddenAsZero = true ) const;

    SC_DLLPUBLIC sal_uInt16         GetOriginalWidth( SCCOL nCol, SCTAB nTab ) const;
    SC_DLLPUBLIC sal_uInt16         GetOriginalHeight( SCROW nRow, SCTAB nTab ) const;

    sal_uInt16          GetCommonWidth( SCCOL nEndCol, SCTAB nTab ) const;

    SCROW           GetHiddenRowCount( SCROW nRow, SCTAB nTab ) const;

    sal_uInt16          GetOptimalColWidth( SCCOL nCol, SCTAB nTab, OutputDevice* pDev,
                                        double nPPTX, double nPPTY,
                                        const Fraction& rZoomX, const Fraction& rZoomY,
                                        bool bFormula,
                                        const ScMarkData* pMarkData = nullptr,
                                        const ScColWidthParam* pParam = nullptr );

    SC_DLLPUBLIC bool SetOptimalHeight(
        sc::RowHeightContext& rCxt, SCROW nStartRow, SCROW nEndRow, SCTAB nTab );

    void UpdateAllRowHeights( sc::RowHeightContext& rCxt, const ScMarkData* pTabMark = nullptr );

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

    SC_DLLPUBLIC void           GetAllRowBreaks(std::set<SCROW>& rBreaks, SCTAB nTab, bool bPage, bool bManual) const;
    SC_DLLPUBLIC void           GetAllColBreaks(std::set<SCCOL>& rBreaks, SCTAB nTab, bool bPage, bool bManual) const;
    SC_DLLPUBLIC ScBreakType    HasRowBreak(SCROW nRow, SCTAB nTab) const;
    SC_DLLPUBLIC ScBreakType    HasColBreak(SCCOL nCol, SCTAB nTab) const;
    SC_DLLPUBLIC void           SetRowBreak(SCROW nRow, SCTAB nTab, bool bPage, bool bManual);
    SC_DLLPUBLIC void           SetColBreak(SCCOL nCol, SCTAB nTab, bool bPage, bool bManual);
    void                        RemoveRowBreak(SCROW nRow, SCTAB nTab, bool bPage, bool bManual);
    void                        RemoveColBreak(SCCOL nCol, SCTAB nTab, bool bPage, bool bManual);
    css::uno::Sequence<
        css::sheet::TablePageBreakData> GetRowBreakData(SCTAB nTab) const;

    SC_DLLPUBLIC bool           RowHidden(SCROW nRow, SCTAB nTab, SCROW* pFirstRow = nullptr, SCROW* pLastRow = nullptr) const;
    SC_DLLPUBLIC bool           HasHiddenRows(SCROW nStartRow, SCROW nEndRow, SCTAB nTab) const;
    SC_DLLPUBLIC bool           ColHidden(SCCOL nCol, SCTAB nTab, SCCOL* pFirstCol = nullptr, SCCOL* pLastCol = nullptr) const;
    SC_DLLPUBLIC void           SetRowHidden(SCROW nStartRow, SCROW nEndRow, SCTAB nTab, bool bHidden);
    SC_DLLPUBLIC void           SetColHidden(SCCOL nStartCol, SCCOL nEndCol, SCTAB nTab, bool bHidden);
    SC_DLLPUBLIC SCROW          FirstVisibleRow(SCROW nStartRow, SCROW nEndRow, SCTAB nTab) const;
    SC_DLLPUBLIC SCROW          LastVisibleRow(SCROW nStartRow, SCROW nEndRow, SCTAB nTab) const;
    SCROW                       CountVisibleRows(SCROW nStartRow, SCROW nEndRow, SCTAB nTab) const;

    bool                        RowFiltered(SCROW nRow, SCTAB nTab, SCROW* pFirstRow = nullptr, SCROW* pLastRow = nullptr) const;
    bool                        HasFilteredRows(SCROW nStartRow, SCROW nEndRow, SCTAB nTab) const;
    bool                        ColFiltered(SCCOL nCol, SCTAB nTab) const;
    SC_DLLPUBLIC void           SetRowFiltered(SCROW nStartRow, SCROW nEndRow, SCTAB nTab, bool bFiltered);
    SCROW                       FirstNonFilteredRow(SCROW nStartRow, SCROW nEndRow, SCTAB nTab) const;
    SCROW                       LastNonFilteredRow(SCROW nStartRow, SCROW nEndRow, SCTAB nTab) const;
    SCROW                       CountNonFilteredRows(SCROW nStartRow, SCROW nEndRow, SCTAB nTab) const;

    SC_DLLPUBLIC bool IsManualRowHeight(SCROW nRow, SCTAB nTab) const;

    bool HasUniformRowHeight( SCTAB nTab, SCROW nRow1, SCROW nRow2 ) const;

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

    SCROW           GetNextDifferentChangedRow( SCTAB nTab, SCROW nStart) const;

    // returns whether to export a Default style for this col or not
    // nDefault is setted to one position in the current row where the Default style is
    void            GetColDefault( SCTAB nTab, SCCOL nCol, SCROW nLastRow, SCROW& nDefault);

    bool            UpdateOutlineCol( SCCOL nStartCol, SCCOL nEndCol, SCTAB nTab, bool bShow );
    bool            UpdateOutlineRow( SCROW nStartRow, SCROW nEndRow, SCTAB nTab, bool bShow );

    void            StripHidden( SCCOL& rX1, SCROW& rY1, SCCOL& rX2, SCROW& rY2, SCTAB nTab );
    void            ExtendHidden( SCCOL& rX1, SCROW& rY1, SCCOL& rX2, SCROW& rY2, SCTAB nTab );

    SC_DLLPUBLIC ScPatternAttr*     GetDefPattern() const;
    SC_DLLPUBLIC ScDocumentPool*    GetPool();
    SC_DLLPUBLIC ScStyleSheetPool*  GetStyleSheetPool() const;

    // PageStyle:
    SC_DLLPUBLIC const OUString  GetPageStyle( SCTAB nTab ) const;
    SC_DLLPUBLIC void           SetPageStyle( SCTAB nTab, const OUString& rName );
    Size            GetPageSize( SCTAB nTab ) const;
    void            SetPageSize( SCTAB nTab, const Size& rSize );
    void            SetRepeatArea( SCTAB nTab, SCCOL nStartCol, SCCOL nEndCol, SCROW nStartRow, SCROW nEndRow );
    void            InvalidatePageBreaks(SCTAB nTab);
    void            UpdatePageBreaks( SCTAB nTab, const ScRange* pUserArea = nullptr );
    void            RemoveManualBreaks( SCTAB nTab );
    bool            HasManualBreaks( SCTAB nTab ) const;

    bool            IsPageStyleInUse( const OUString& rStrPageStyle, SCTAB* pInTab = nullptr );
    bool            RemovePageStyleInUse( const OUString& rStrPageStyle );
    bool            RenamePageStyleInUse( const OUString& rOld, const OUString& rNew );
    void            ModifyStyleSheet( SfxStyleSheetBase& rPageStyle,
                                      const SfxItemSet&  rChanges );

    void            PageStyleModified( SCTAB nTab, const OUString& rNewName );

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
                                           SCCOL nEndCol, SCROW nEndRow, SCTAB nTab, bool bHiddenAsZero = true ) const;
    SC_DLLPUBLIC ScRange            GetRange( SCTAB nTab, const Rectangle& rMMRect, bool bHiddenAsZero = true ) const;

    void            UpdStlShtPtrsFrmNms();
    void            StylesToNames();

    SC_DLLPUBLIC void           CopyStdStylesFrom( ScDocument* pSrcDoc );

    sal_uLong           GetSrcVersion() const   { return nSrcVer; }

    void            SetSrcCharSet( rtl_TextEncoding eNew )   { eSrcSet = eNew; }
    void            UpdateFontCharSet();

    void FillInfo(
        ScTableInfo& rTabInfo, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
        SCTAB nTab, double fColScale, double fRowScale, bool bPageMode, bool bFormulaMode,
        const ScMarkData* pMarkData = nullptr );

    SC_DLLPUBLIC SvNumberFormatter* GetFormatTable() const;
    SC_DLLPUBLIC SvNumberFormatter* CreateFormatTable() const;

    /** Sort a range of data. */
    void Sort(
        SCTAB nTab, const ScSortParam& rSortParam, bool bKeepQuery, bool bUpdateRefs,
        ScProgress* pProgress, sc::ReorderParam* pUndo );

    void Reorder( const sc::ReorderParam& rParam, ScProgress* pProgress );

    SCSIZE          Query( SCTAB nTab, const ScQueryParam& rQueryParam, bool bKeepSub );
    SC_DLLPUBLIC bool           CreateQueryParam( SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2,
                                        SCTAB nTab, ScQueryParam& rQueryParam );
    void            GetUpperCellString(SCCOL nCol, SCROW nRow, SCTAB nTab, OUString& rStr);

    /**
     * Get a list of unique strings to use in filtering criteria.  The string
     * values are sorted, and there are no duplicate values in the list.  The
     * data range to use to populate the filter entries is inferred from the
     * database range that contains the specified cell position.
     */
    void GetFilterEntries(
        SCCOL nCol, SCROW nRow, SCTAB nTab, std::vector<ScTypedStrData>& rStrings, bool& rHasDates);

    SC_DLLPUBLIC void GetFilterEntriesArea(
        SCCOL nCol, SCROW nStartRow, SCROW nEndRow, SCTAB nTab, bool bCaseSens,
        std::vector<ScTypedStrData>& rStrings, bool& rHasDates);

    void GetDataEntries(
        SCCOL nCol, SCROW nRow, SCTAB nTab,
        std::vector<ScTypedStrData>& rStrings, bool bLimit = false );
    void GetFormulaEntries( ScTypedCaseStrSet& rStrings );

    bool HasAutoFilter( SCCOL nCol, SCROW nRow, SCTAB nTab );
    void GetFilterSelCount( SCCOL nCol, SCROW nRow, SCTAB nTab, SCSIZE& nSelected, SCSIZE& nTotal );

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

    void            ReplaceStyle(const SvxSearchItem& rSearchItem,
                                 SCCOL nCol, SCROW nRow, SCTAB nTab,
                                 ScMarkData& rMark);

    void            InvalidateTextWidth( const OUString& rStyleName );
    void            InvalidateTextWidth( SCTAB nTab );
    void            InvalidateTextWidth( const ScAddress* pAdrFrom, const ScAddress* pAdrTo, bool bNumFormatChanged );

    bool            IdleCalcTextWidth();

    void            RepaintRange( const ScRange& rRange );
    void            RepaintRange( const ScRangeList& rRange );

    bool IsIdleEnabled() const { return mbIdleEnabled; }
    void EnableIdle(bool bDo) { mbIdleEnabled = bDo; }

    bool            IsDetectiveDirty() const     { return bDetectiveDirty; }
    void            SetDetectiveDirty(bool bSet) { bDetectiveDirty = bSet; }

    bool            GetHasMacroFunc() const      { return bHasMacroFunc; }
    void            SetHasMacroFunc(bool bSet)   { bHasMacroFunc = bSet; }

    static bool     CheckMacroWarn();

    void            SetRangeOverflowType(sal_uInt32 nType)  { nRangeOverflowType = nType; }
    bool            HasRangeOverflow() const                { return nRangeOverflowType != 0; }
    SC_DLLPUBLIC sal_uInt32      GetRangeOverflowType() const            { return nRangeOverflowType; }

    // for broadcasting/listening
    void            SetInsertingFromOtherDoc( bool bVal ) { bInsertingFromOtherDoc = bVal; }
    bool            IsInsertingFromOtherDoc() const { return bInsertingFromOtherDoc; }
    void            SetLoadingMedium( bool bVal );
    void            SetImportingXML( bool bVal );
    bool            IsImportingXML() const { return bImportingXML; }
    bool            IsCalcingAfterLoad() const { return bCalcingAfterLoad; }
    void            SetNoListening( bool bVal ) { bNoListening = bVal; }
    bool            GetNoListening() const { return bNoListening; }
    ScBroadcastAreaSlotMachine* GetBASM() const { return pBASM; }

    SC_DLLPUBLIC ScChartListenerCollection* GetChartListenerCollection() const { return pChartListenerCollection;}
    void            SetChartListenerCollection( ScChartListenerCollection*,
                        bool bSetChartRangeLists = false );
    void            UpdateChart( const OUString& rName );
    void            RestoreChartListener( const OUString& rName );
    SC_DLLPUBLIC void           UpdateChartListenerCollection();
    bool            IsChartListenerCollectionNeedsUpdate() const
                        { return bChartListenerCollectionNeedsUpdate; }
    void            SetChartListenerCollectionNeedsUpdate( bool bFlg )
                        { bChartListenerCollectionNeedsUpdate = bFlg; }
    void            AddOLEObjectToCollection(const OUString& rName);

    ScChangeViewSettings* GetChangeViewSettings() const     { return pChangeViewSettings; }
    SC_DLLPUBLIC void               SetChangeViewSettings(const ScChangeViewSettings& rNew);

    const rtl::Reference<SvxForbiddenCharactersTable>& GetForbiddenCharacters();
    void            SetForbiddenCharacters(const rtl::Reference<SvxForbiddenCharactersTable>& rNew);

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

    void FillMatrix( ScMatrix& rMat, SCTAB nTab, SCCOL nCol1, SCROW nRow1, SCCOL nCol2, SCROW nRow2, svl::SharedStringPool* pPool = nullptr) const;

    /**
     * Set an array of numerical formula results to a group of contiguous
     * formula cells.
     *
     * @param rTopPos position of the top formula cell of a group.
     * @param pResults array of numeric results.
     * @param nLen length of numeric results.
     */
    void SC_DLLPUBLIC SetFormulaResults( const ScAddress& rTopPos, const double* pResults, size_t nLen );

    void SC_DLLPUBLIC SetFormulaResults( const ScAddress& rTopPos, const formula::FormulaTokenRef* pResults, size_t nLen );

    /**
     * Transfer a series of contiguous cell values from specified position to
     * the passed container. The specified segment will become empty after the
     * transfer.
     */
    void TransferCellValuesTo( const ScAddress& rTopPos, size_t nLen, sc::CellValues& rDest );

    void CopyCellValuesFrom( const ScAddress& rTopPos, const sc::CellValues& rSrc );

    std::set<Color> GetDocColors();

    sc::IconSetBitmapMap& GetIconSetBitmapMap();

private:
    ScDocument(const ScDocument& r) = delete;

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

    DECL_LINK_TYPED(TrackTimeHdl, Idle *, void);

    static ScRecursionHelper*   CreateRecursionHelperInstance();

public:
    void StartListeningArea( const ScRange& rRange, bool bGroupListening, SvtListener* pListener );

    void EndListeningArea( const ScRange& rRange, bool bGroupListening, SvtListener* pListener );
                        /** Broadcast wrapper, calls
                            rHint.GetCell()->Broadcast() and AreaBroadcast()
                            and TrackFormulas()
                            Preferred.
                         */
    void                Broadcast( const ScHint& rHint );

    void BroadcastCells( const ScRange& rRange, sal_uInt32 nHint, bool bBroadcastSingleBroadcasters = true );
    void BroadcastRefMoved( const sc::RefMovedHint& rHint );

                        /// only area, no cell broadcast
    void                AreaBroadcast( const ScHint& rHint );
    void                DelBroadcastAreasInRange( const ScRange& rRange );
    void                UpdateBroadcastAreas( UpdateRefMode eUpdateRefMode,
                                            const ScRange& rRange,
                                            SCsCOL nDx, SCsROW nDy, SCsTAB nDz );

    void                StartListeningCell( const ScAddress& rAddress,
                                            SvtListener* pListener );
    void                EndListeningCell( const ScAddress& rAddress,
                                            SvtListener* pListener );

    void StartListeningCell( sc::StartListeningContext& rCxt, const ScAddress& rPos, SvtListener& rListener );
    void EndListeningCell( sc::EndListeningContext& rCxt, const ScAddress& rPos, SvtListener& rListener );

    void EndListeningFormulaCells( std::vector<ScFormulaCell*>& rCells );
    void CollectAllAreaListeners(
        std::vector<SvtListener*>& rListeners, const ScRange& rRange, sc::AreaOverlapType eType );

    void                PutInFormulaTree( ScFormulaCell* pCell );
    void                RemoveFromFormulaTree( ScFormulaCell* pCell );

/**
     * Calculate formula cells that are on the formula tree either partially,
     * or in full.
     *
     * @param bOnlyForced when true, it only calculates those formula cells
     *                    that are marked "recalc forced".
     * @param bProgressBar whether or not to use progress bar.
     * @param bSetAllDirty when true, it marks all formula cells currently on
     *                     the formula tree dirty, which forces all of them to
     *                     be recalculated.  When false, only those cells
     *                     that are marked dirty prior to this call get
     *                     recalculated.
     */
    SC_DLLPUBLIC void CalcFormulaTree(
       bool bOnlyForced = false, bool bProgressBar = true, bool bSetAllDirty = true );
    void                ClearFormulaTree();
    void                AppendToFormulaTrack( ScFormulaCell* pCell );
    void                RemoveFromFormulaTrack( ScFormulaCell* pCell );
    void                TrackFormulas( sal_uInt32 nHintId = SC_HINT_DATACHANGED );
    bool                IsInFormulaTree( ScFormulaCell* pCell ) const;
    bool                IsInFormulaTrack( ScFormulaCell* pCell ) const;
    HardRecalcState     GetHardRecalcState() { return eHardRecalcState; }
    void                SetHardRecalcState( HardRecalcState eVal ) { eHardRecalcState = eVal; }
    void                StartAllListeners();
    void                StartNeededListeners();
    void                StartNeededListeners( const std::shared_ptr<const sc::ColumnSet>& rpColSet );
    void                StartAllListeners( const ScRange& rRange );

    void                SetForcedFormulas( bool bVal ) { bHasForcedFormulas = bVal; }
    sal_uLong           GetFormulaCodeInTree() const { return nFormulaCodeInTree; }
    bool                IsInInterpreter() const { return nInterpretLevel != 0; }

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
    sal_uInt16          GetMacroInterpretLevel() { return nMacroInterpretLevel; }
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
    void                SetExpandRefs( bool bVal );
    bool                IsExpandRefs() const { return bExpandRefs; }

    sal_uLong           GetXMLImportedFormulaCount() const { return nXMLImportedFormulaCount; }
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
    void            CompileColRowNameFormula();

    /** Maximum string length of a column, e.g. for dBase export.
        @return String length in octets (!) of the destination encoding. In
                case of non-octet encodings (e.g. UCS2) the length in code
                points times sizeof(sal_Unicode) is returned. */
    sal_Int32       GetMaxStringLen( SCTAB nTab, SCCOL nCol,
                                     SCROW nRowStart, SCROW nRowEnd,
                                     rtl_TextEncoding eCharSet ) const;
    /** Maximum string length of numerical cells of a column, e.g. for dBase export.
        @return String length in characters (!) including the decimal
                separator, and the decimal precision needed. */
    sal_Int32       GetMaxNumberStringLen( sal_uInt16& nPrecision,
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
    void            AddUnoListenerCall( const css::uno::Reference<
                                            css::util::XModifyListener >& rListener,
                                        const css::lang::EventObject& rEvent );

    void            SetInLinkUpdate(bool bSet);             // TableLink or AreaLink
    bool            IsInLinkUpdate() const;                 // including DdeLink

    SC_DLLPUBLIC SfxItemPool*       GetEditPool() const;
    SC_DLLPUBLIC SfxItemPool*       GetEnginePool() const;
    SC_DLLPUBLIC ScFieldEditEngine& GetEditEngine();
    SC_DLLPUBLIC ScNoteEditEngine&  GetNoteEngine();

    ScRefreshTimerControl * const & GetRefreshTimerControlAddress() const
        { return pRefreshTimerControl; }

    void            SetPastingDrawFromOtherDoc( bool bVal )
                        { bPastingDrawFromOtherDoc = bVal; }
    bool            PastingDrawFromOtherDoc() const
                        { return bPastingDrawFromOtherDoc; }

                    /// an ID unique to each document instance
    sal_uInt32      GetDocumentID() const;

    void            InvalidateStyleSheetUsage()
                        { bStyleSheetUsageInvalid = true; }
    void SC_DLLPUBLIC GetSortParam( ScSortParam& rParam, SCTAB nTab );
    void SC_DLLPUBLIC SetSortParam( ScSortParam& rParam, SCTAB nTab );

    inline void     SetVbaEventProcessor( const css::uno::Reference< css::script::vba::XVBAEventProcessor >& rxVbaEvents )
                        { mxVbaEvents = rxVbaEvents; }
    const css::uno::Reference< css::script::vba::XVBAEventProcessor >&
                    GetVbaEventProcessor() const { return mxVbaEvents; }

    /** Should only be GRAM_PODF or GRAM_ODFF. */
    void                SetStorageGrammar( formula::FormulaGrammar::Grammar eGrammar );
    formula::FormulaGrammar::Grammar  GetStorageGrammar() const
                            { return eStorageGrammar; }

    SC_DLLPUBLIC SfxUndoManager* GetUndoManager();
    bool IsInVBAMode() const;
    ScRowBreakIterator* GetRowBreakIterator(SCTAB nTab) const;

    void AddSubTotalCell(ScFormulaCell* pCell);
    void RemoveSubTotalCell(ScFormulaCell* pCell);
    void SetSubTotalCellsDirty(const ScRange& rDirtyRange);

    sal_uInt16 GetTextWidth( const ScAddress& rPos ) const;

    SvtScriptType GetScriptType( const ScAddress& rPos ) const;
    void SetScriptType( const ScAddress& rPos, SvtScriptType nType );
    void UpdateScriptTypes( const ScAddress& rPos, SCCOL nColSize, SCROW nRowSize );

    size_t GetFormulaHash( const ScAddress& rPos ) const;

    /**
     * Make specified formula cells non-grouped.
     *
     * @param nTab sheet index
     * @param nCol column index
     * @param rRows list of row indices at which formula cells are to be
     *              unshared. This call sorts the passed row indices and
     *              removes duplicates, which is why the caller must pass it
     *              as reference.
     */
    void UnshareFormulaCells( SCTAB nTab, SCCOL nCol, std::vector<SCROW>& rRows );
    void RegroupFormulaCells( SCTAB nTab, SCCOL nCol );

    ScFormulaVectorState GetFormulaVectorState( const ScAddress& rPos ) const;

    formula::FormulaTokenRef ResolveStaticReference( const ScAddress& rPos );
    formula::FormulaTokenRef ResolveStaticReference( const ScRange& rRange );

    formula::VectorRefArray FetchVectorRefArray( const ScAddress& rPos, SCROW nLength );

    /**
     * Call this before any operations that might trigger one or more formula
     * cells to get calculated.
     */
    void ClearFormulaContext();

    SvtBroadcaster* GetBroadcaster( const ScAddress& rPos );
    const SvtBroadcaster* GetBroadcaster( const ScAddress& rPos ) const;
    void DeleteBroadcasters( sc::ColumnBlockPosition& rBlockPos, const ScAddress& rTopPos, SCROW nLength );

#if DEBUG_COLUMN_STORAGE
    SC_DLLPUBLIC void DumpFormulaGroups( SCTAB nTab, SCCOL nCol ) const;
#endif

#if DEBUG_AREA_BROADCASTER
    SC_DLLPUBLIC void DumpAreaBroadcasters() const;
#endif

    SC_DLLPUBLIC void SetCalcConfig( const ScCalcConfig& rConfig );
    const ScCalcConfig& GetCalcConfig() const { return maCalcConfig; }

    void ConvertFormulaToValue( const ScRange& rRange, sc::TableValues* pUndo );

    void SwapNonEmpty( sc::TableValues& rValues );

    void finalizeOutlineImport();

private:

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

    bool TableExists( SCTAB nTab ) const;
    ScTable* FetchTable( SCTAB nTab );
    const ScTable* FetchTable( SCTAB nTab ) const;

    void    MergeNumberFormatter(ScDocument* pSrcDoc);

    void    ImplCreateOptions(); // Suggestion: switch to on-demand?
    void    ImplDeleteOptions();

    void    DeleteDrawLayer();
    SC_DLLPUBLIC bool   DrawGetPrintArea( ScRange& rRange, bool bSetHor, bool bSetVer ) const;
    void    DrawMovePage( sal_uInt16 nOldPos, sal_uInt16 nNewPos );
    void    DrawCopyPage( sal_uInt16 nOldPos, sal_uInt16 nNewPos );

    void    UpdateDrawPrinter();
    void    UpdateDrawLanguages();
    SC_DLLPUBLIC void   InitClipPtrs( ScDocument* pSourceDoc );

    void    LoadDdeLinks(SvStream& rStream);
    void    SaveDdeLinks(SvStream& rStream) const;

    void    DeleteAreaLinksOnTab( SCTAB nTab );
    void    UpdateRefAreaLinks( UpdateRefMode eUpdateRefMode,
                             const ScRange& r, SCsCOL nDx, SCsROW nDy, SCsTAB nDz );

    void    CopyRangeNamesToClip(ScDocument* pClipDoc, const ScRange& rClipRange, const ScMarkData* pMarks);

    bool    HasPartOfMerged( const ScRange& rRange );

    ScRefCellValue GetRefCellValue( const ScAddress& rPos );

    std::map< SCTAB, ScSortParam > mSheetSortParams;

    SCSIZE GetPatternCount( SCTAB nTab, SCCOL nCol ) const;
    SCSIZE GetPatternCount( SCTAB nTab, SCCOL nCol, SCROW nRow1, SCROW nRow2 ) const;
    void   ReservePatternCount( SCTAB nTab, SCCOL nCol, SCSIZE nReserve );

    void SharePooledResources( ScDocument* pSrcDoc );

    void EndListeningIntersectedGroup(
        sc::EndListeningContext& rCxt, const ScAddress& rPos, std::vector<ScAddress>* pGroupPos = nullptr );

    void EndListeningIntersectedGroups(
        sc::EndListeningContext& rCxt, const ScRange& rRange, std::vector<ScAddress>* pGroupPos = nullptr );

    void EndListeningGroups( const std::vector<ScAddress>& rPosArray );
    void SetNeedsListeningGroups( const std::vector<ScAddress>& rPosArray );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
