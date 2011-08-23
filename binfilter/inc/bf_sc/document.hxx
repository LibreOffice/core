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



#ifndef _SV_TIMER_HXX //autogen
#include <vcl/timer.hxx>
#endif


#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif

#ifndef SC_TABLE_HXX
#include "table.hxx"		// FastGetRowHeight (inline)
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
namespace com { namespace sun { namespace star {
    namespace lang {
        class XMultiServiceFactory;
    }
    namespace i18n {
        class XBreakIterator;
    }
} } }
class KeyEvent;
class OutputDevice;
class SvMemoryStream;
class Window;
class List;
namespace binfilter {
class SvNumberFormatter;

class SfxBroadcaster;
class SfxHint;
class SfxItemSet;
class SfxPoolItem; 
class SfxItemPool;
class SfxStyleSheetBase;

class SdrObject;
class SfxObjectShell;
class SfxBindings;
class SfxPrinter;
class SfxStatusBarManager;
class SvxBorderLine;
class SvxBoxInfoItem;
class SvxBoxItem;
class SvxBrushItem;
class SvxForbiddenCharactersTable;
class SvxLinkManager;
class SvxShadowItem;
class XColorTable;

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
class StrCollection;
class TypedStrCollection;
class ScChangeTrack;
class ScFieldEditEngine;
struct ScConsolidateParam;
class ScDPObject;
class ScDPCollection;
class ScMatrix;
class ScScriptTypeData;
class ScPoolHelper;
class ScImpExpLogMsg;
struct ScSortParam;
class ScRefreshTimerControl;

//STRIP008 namespace com { namespace sun { namespace star {
//STRIP008     namespace lang {
//STRIP008         class XMultiServiceFactory;
//STRIP008     }
//STRIP008     namespace i18n {
//STRIP008         class XBreakIterator;
//STRIP008     }
//STRIP008 } } }

} //namespace binfilter

#include <bf_svtools/zforlist.hxx>		// IsNumberFormat
/*#ifdef _ZFORLIST_DECLARE_TABLE
class SvNumberFormatterIndexTable;
#else
class Table;
typedef Table SvNumberFormatterIndexTable;
#endif*/
namespace binfilter {//STRIP009
#define SC_TAB_APPEND		0xFFFF
#define SC_DOC_NEW			0xFFFF
#define REPEAT_NONE			0xFFFF

#define SC_MACROCALL_ALLOWED		0
#define SC_MACROCALL_NOTALLOWED		1
#define SC_MACROCALL_ASK			2

#define SC_ASIANCOMPRESSION_INVALID		0xff
#define SC_ASIANKERNING_INVALID			0xff


enum ScDocumentMode
    {
        SCDOCMODE_DOCUMENT,
        SCDOCMODE_CLIP,
        SCDOCMODE_UNDO
    };



// -----------------------------------------------------------------------
//
//			structs fuer FillInfo
//

enum ScShadowPart
{
    SC_SHADOW_HSTART,
    SC_SHADOW_VSTART,
    SC_SHADOW_HORIZ,
    SC_SHADOW_VERT,
    SC_SHADOW_CORNER
};

#define SC_ROTDIR_NONE			0
#define SC_ROTDIR_STANDARD		1
#define SC_ROTDIR_LEFT			2
#define SC_ROTDIR_RIGHT			3
#define SC_ROTDIR_CENTER		4

struct CellInfo
    {
        ScBaseCell*					pCell;

        const ScPatternAttr*		pPatternAttr;
        const SfxItemSet*			pConditionSet;

        const SvxBrushItem*			pBackground;

        const SvxBoxItem*			pLinesAttr;				// Original-Item (intern)

        const SvxBorderLine* 		pThisBottom;			// einzelne inkl. zusammengefasst
        const SvxBorderLine* 		pNextTop;				// (intern)
        const SvxBorderLine* 		pThisRight;
        const SvxBorderLine* 		pNextLeft;

        const SvxBorderLine*		pRightLine;				// dickere zum Zeichnen
        const SvxBorderLine*		pBottomLine;

        const SvxShadowItem*		pShadowAttr;			// Original-Item (intern)

        ScShadowPart				eHShadowPart;			// Schatten effektiv zum Zeichnen
        ScShadowPart				eVShadowPart;
        const SvxShadowItem*		pHShadowOrigin;
        const SvxShadowItem*		pVShadowOrigin;

        USHORT						nWidth;

        BOOL						bMarked;
        BOOL						bStandard;
        BOOL						bEmptyCellText;

        BOOL						bMerged;
        BOOL						bHOverlapped;
        BOOL						bVOverlapped;
        BOOL						bAutoFilter;
        BOOL						bPushButton;
        BYTE						nRotateDir;

        BOOL						bPrinted;				// bei Bedarf (Pagebreak-Modus)

        BOOL						bHideGrid;				// output-intern
        BOOL						bEditEngine;			// output-intern
    };

#define SC_ROTMAX_NONE	USHRT_MAX

struct RowInfo
    {
        CellInfo*		pCellInfo;

        USHORT			nHeight;
        USHORT			nRowNo;
        USHORT			nRotMaxCol;			// SC_ROTMAX_NONE, wenn nichts

        BOOL			bEmptyBack;
        BOOL			bEmptyText;
        BOOL			bAutoFilter;
        BOOL			bPushButton;
        BOOL			bChanged;			// TRUE, wenn nicht getestet
    };

struct ScDocStat
{
    String	aDocName;
    USHORT	nTableCount;
    ULONG	nCellCount;
    USHORT	nPageCount;
};

// nicht 11 Parameter bei CopyBlockFromClip, konstante Werte der Schleife hier
struct ScCopyBlockFromClipParams
{
    ScDocument*	pRefUndoDoc;
    ScDocument*	pClipDoc;
    USHORT		nInsFlag;
    USHORT		nTabStart;
    USHORT		nTabEnd;
    BOOL		bAsLink;
    BOOL		bSkipAttrForEmpty;
};

#define ROWINFO_MAX	1024


// for loading of binary file format symbol string cells which need font conversion
struct ScSymbolStringCellEntry
{
    ScStringCell*   pCell;
    USHORT          nRow;
};


// Spezialwert fuer Recalc-Alwyas-Zellen

#define	BCA_BRDCST_ALWAYS ScAddress( 0, 32767, 0 )
#define	BCA_LISTEN_ALWAYS ScRange( BCA_BRDCST_ALWAYS, BCA_BRDCST_ALWAYS )

// -----------------------------------------------------------------------

// DDE Link Modes

#define SC_DDE_DEFAULT		0
#define SC_DDE_ENGLISH		1
#define SC_DDE_TEXT			2
#define SC_DDE_IGNOREMODE   255     // for usage in FindDdeLink() only!

// -----------------------------------------------------------------------

class ScDocument
{
friend class ScValueIterator;
friend class ScQueryValueIterator;
friend class ScCellIterator;
friend class ScQueryCellIterator;
friend class ScHorizontalCellIterator;
friend class ScDocAttrIterator;
friend class ScAttrRectIterator;
friend class ScPivot;

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceManager;

    vos::ORef<ScPoolHelper> xPoolHelper;

    ScFieldEditEngine*	pEditEngine;					// uses pEditPool from xPoolHelper
    SfxObjectShell*		pShell;
    SfxPrinter*			pPrinter;
    ScDrawLayer*		pDrawLayer;						// SdrModel
    XColorTable*		pColorTable;
    ScConditionalFormatList* pCondFormList;				// bedingte Formate
    ScValidationDataList* pValidationList;				// Gueltigkeit
    SvNumberFormatterIndexTable*		pFormatExchangeList;			// zum Umsetzen von Zahlenformaten
    ScTable*			pTab[MAXTAB+1];
    ScRangeName*		pRangeName;
    ScDBCollection*		pDBCollection;
    ScPivotCollection*	pPivotCollection;
    ScDPCollection*		pDPCollection;
    ScChartCollection*	pChartCollection;
    ScPatternAttr*		pSelectionAttr;					// Attribute eines Blocks
    SvxLinkManager*		pLinkManager;
    ScFormulaCell*		pFormulaTree;					// Berechnungsbaum Start
    ScFormulaCell*		pEOFormulaTree;					// Berechnungsbaum Ende, letzte Zelle
    ScFormulaCell*		pFormulaTrack;					// BroadcastTrack Start
    ScFormulaCell*		pEOFormulaTrack;				// BrodcastTrack Ende, letzte Zelle
    ScBroadcastAreaSlotMachine*	pBASM;					// BroadcastAreas
    ScChartListenerCollection* pChartListenerCollection;
    StrCollection*		pOtherObjects;					// non-chart OLE objects
    SvMemoryStream*		pClipData;
    ScDetOpList*		pDetOpList;
    ScChangeTrack*		pChangeTrack;
    SfxBroadcaster*		pUnoBroadcaster;
    ScChangeViewSettings* pChangeViewSettings;
    ScScriptTypeData*	pScriptTypeData;
    ScRefreshTimerControl* pRefreshTimerControl;
    vos::ORef<SvxForbiddenCharactersTable> xForbiddenCharacters;

    ScFieldEditEngine*	pCacheFieldEditEngine;

    ::com::sun::star::uno::Sequence<sal_Int8>	aProtectPass;
    String              aDocName;                       // opt: Dokumentname
    ScRangePairListRef	xColNameRanges;
    ScRangePairListRef	xRowNameRanges;

    ScViewOptions*		pViewOptions;					// View-Optionen
    ScDocOptions*		pDocOptions;					// Dokument-Optionen
    ScExtDocOptions*	pExtDocOptions;					// fuer Import etc.
    ScConsolidateParam*	pConsolidateDlgData;

    List*               pLoadedSymbolStringCellList;    // binary file format import of symbol font string cells

    ScRange				aClipRange;
    ScRange				aEmbedRange;
    ScAddress			aCurTextWidthCalcPos;
    ScAddress			aOnlineSpellPos;				// within whole document
    ScRange				aVisSpellRange;
    ScAddress			aVisSpellPos;					// within aVisSpellRange (see nVisSpellState)

    Timer				aTrackTimer;

public:
    ScTabOpList         aTableOpList;		            // list of ScInterpreterTableOpParams currently in use
    ScInterpreterTableOpParams  aLastTableOpParams;     // remember last params
private:

    LanguageType		eLanguage;						// default language
    LanguageType		eCjkLanguage;					// default language for asian text
    LanguageType		eCtlLanguage;					// default language for complex text
    CharSet				eSrcSet; 						// Einlesen: Quell-Zeichensatz

    ULONG				nFormulaCodeInTree;				// FormelRPN im Formelbaum
    ULONG               nXMLImportedFormulaCount;        // progress count during XML import
    USHORT				nInterpretLevel;				// >0 wenn im Interpreter
    USHORT				nMacroInterpretLevel; 			// >0 wenn Macro im Interpreter
    USHORT				nInterpreterTableOpLevel;		// >0 if in Interpreter TableOp
    USHORT				nMaxTableNumber;
    USHORT				nSrcVer;						// Dateiversion (Laden/Speichern)
    USHORT				nSrcMaxRow;						// Zeilenzahl zum Laden/Speichern
    USHORT				nFormulaTrackCount;
    USHORT				nHardRecalcState;				// 0: soft, 1: hard-warn, 2: hard
    USHORT				nVisibleTab;					// fuer OLE etc.

    ScLkUpdMode			eLinkMode;

    BOOL				bProtected;
    BOOL				bAutoCalc;						// Automatisch Berechnen
    BOOL				bAutoCalcShellDisabled;			// in/von/fuer ScDocShell disabled
    // ob noch ForcedFormulas berechnet werden muessen,
    // im Zusammenspiel mit ScDocShell SetDocumentModified,
    // AutoCalcShellDisabled und TrackFormulas
    BOOL				bForcedFormulaPending;
    BOOL				bCalculatingFormulaTree;
    BOOL				bIsClip;
    BOOL				bCutMode;
    BOOL				bIsUndo;

    BOOL				bIsEmbedded;					// Embedded-Bereich anzeigen/anpassen ?

    // kein SetDirty bei ScFormulaCell::CompileTokenArray sondern am Ende
    // von ScDocument::CompileAll[WithFormats], CopyScenario, CopyBlockFromClip
    BOOL				bNoSetDirty;
    // kein Broadcast, keine Listener aufbauen waehrend aus einem anderen
    // Doc (per Filter o.ae.) inserted wird, erst bei CompileAll / CalcAfterLoad
    BOOL				bInsertingFromOtherDoc;
    BOOL				bImportingXML;		// special handling of formula text
    BOOL				bCalcingAfterLoad;				// in CalcAfterLoad TRUE
    // wenn temporaer keine Listener auf/abgebaut werden sollen
    BOOL				bNoListening;
    BOOL				bLoadingDone;
    BOOL				bIdleDisabled;
    BOOL				bInLinkUpdate;					// TableLink or AreaLink
    BOOL				bChartListenerCollectionNeedsUpdate;
    // ob RC_FORCED Formelzellen im Dokument sind/waren (einmal an immer an)
    BOOL				bHasForcedFormulas;
    // ist beim Laden/Speichern etwas weggelassen worden?
    BOOL				bLostData;
    // ob das Doc gerade zerstoert wird (kein Notify-Tracking etc. mehr)
    BOOL				bInDtorClear;
    // ob bei Spalte/Zeile einfuegen am Rand einer Referenz die Referenz
    // erweitert wird, wird in jedem UpdateReference aus InputOptions geholt,
    // gesetzt und am Ende von UpdateReference zurueckgesetzt
    BOOL				bExpandRefs;
    // fuer Detektiv-Update, wird bei jeder Aenderung an Formeln gesetzt
    BOOL				bDetectiveDirty;

    BYTE				nMacroCallMode;		// Makros per Warnung-Dialog disabled?
    BOOL				bHasMacroFunc;		// valid only after loading

    BYTE				nVisSpellState;

    BYTE				nAsianCompression;
    BYTE				nAsianKerning;

    BOOL                bPastingDrawFromOtherDoc;

    BYTE                nInDdeLinkUpdate;   // originating DDE links (stacked bool)

    BOOL				bInUnoBroadcast;

    mutable BOOL        bStyleSheetUsageInvalid;


    inline BOOL 		RowHidden( USHORT nRow, USHORT nTab );		// FillInfo

public:
    long			GetCellCount() const;		// alle Zellen
    long			GetWeightedCount() const;	// Formeln und Edit staerker gewichtet
    DECL_LINK( GetUserDefinedColor, USHORT * );
                                                                // Numberformatter

public:
                    ScDocument( ScDocumentMode eMode = SCDOCMODE_DOCUMENT,
                                SfxObjectShell* pDocShell = NULL );
                    ~ScDocument();

    inline ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                    GetServiceManager() const { return xServiceManager; }

    const String& 	GetName() const { return aDocName; }
    void			SetName( const String& r ) { aDocName = r; }


    void			InitDrawLayer( SfxObjectShell* pDocShell = NULL );
    XColorTable*	GetColorTable();

    SvxLinkManager*		GetLinkManager() { return pLinkManager; }

    const ScDocOptions&		GetDocOptions() const;
    void					SetDocOptions( const ScDocOptions& rOpt );
    const ScViewOptions&	GetViewOptions() const;
    void 	 				SetViewOptions( const ScViewOptions& rOpt );
    void					SetPrintOptions();

    ScExtDocOptions*		GetExtDocOptions()	{ return pExtDocOptions; }

    void					GetLanguage( LanguageType& rLatin, LanguageType& rCjk, LanguageType& rCtl ) const;
    void					SetLanguage( LanguageType eLatin, LanguageType eCjk, LanguageType eCtl );

    void						SetConsolidateDlgData( const ScConsolidateParam* pData );
    const ScConsolidateParam*	GetConsolidateDlgData() const { return pConsolidateDlgData; }

    void			Clear();

    ScFieldEditEngine*	CreateFieldEditEngine();
    void				DisposeFieldEditEngine(ScFieldEditEngine*& rpEditEngine);

    ScRangeName*	GetRangeName();
    void			SetRangeName( ScRangeName* pNewRangeName );
    USHORT			GetMaxTableNumber() { return nMaxTableNumber; }
    void			SetMaxTableNumber(USHORT nNumber) { nMaxTableNumber = nNumber; }

    ScRangePairList*	GetColNameRanges() { return &xColNameRanges; }
    ScRangePairList*	GetRowNameRanges() { return &xRowNameRanges; }
    ScRangePairListRef&	GetColNameRangesRef() { return xColNameRanges; }
    ScRangePairListRef&	GetRowNameRangesRef() { return xRowNameRanges; }

    ScDBCollection*	GetDBCollection() const;
    ScDBData*		GetDBAtCursor(USHORT nCol, USHORT nRow, USHORT nTab,
                                        BOOL bStartOnly = FALSE) const;
    ScDBData*		GetDBAtArea(USHORT nTab, USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2) const;


    ScDPCollection*		GetDPCollection();




    void			UpdateChartArea( const String& rChartName,
                                    const ScRangeListRef& rNewList,
                                    BOOL bColHeaders, BOOL bRowHeaders, BOOL bAdd,
                                    Window* pWindow );
    SchMemChart*	FindChartData(const String& rName, BOOL bForModify = FALSE);

    void			MakeTable( USHORT nTab );

    USHORT			GetVisibleTab() const		{ return nVisibleTab; }
    void			SetVisibleTab(USHORT nTab)	{ nVisibleTab = nTab; }

    BOOL			HasTable( USHORT nTab ) const;
    BOOL			GetName( USHORT nTab, String& rName ) const;
    BOOL			GetTable( const String& rName, USHORT& rTab ) const;
    inline USHORT	GetTableCount() const { return nMaxTableNumber; }
    SvNumberFormatterIndexTable*	GetFormatExchangeList() const { return pFormatExchangeList; }

    void			SetDocProtection( BOOL bProtect, const ::com::sun::star::uno::Sequence <sal_Int8>& aPass );
    void			SetTabProtection( USHORT nTab, BOOL bProtect, const ::com::sun::star::uno::Sequence <sal_Int8>& aPass );
    BOOL			IsDocProtected() const;
    BOOL			IsDocEditable() const;
    BOOL			IsTabProtected( USHORT nTab ) const;
    const ::com::sun::star::uno::Sequence <sal_Int8>&	GetDocPassword() const;
    const ::com::sun::star::uno::Sequence <sal_Int8>&	GetTabPassword( USHORT nTab ) const;

    void			LockTable(USHORT nTab);
    void			UnlockTable(USHORT nTab);

    BOOL			IsBlockEditable( USHORT nTab, USHORT nStartCol, USHORT nStartRow,
                                        USHORT nEndCol, USHORT nEndRow,
                                        BOOL* pOnlyNotBecauseOfMatrix = NULL ) const;
    BOOL			IsSelectionEditable( const ScMarkData& rMark,
                                        BOOL* pOnlyNotBecauseOfMatrix = NULL ) const;


    BOOL			GetMatrixFormulaRange( const ScAddress& rCellPos, ScRange& rMatrix );

    BOOL			IsEmbedded() const;
    void			SnapVisArea( Rectangle& rRect ) const;			// 1/100 mm

    BOOL			ValidTabName( const String& rName ) const;
    BOOL			ValidNewTabName( const String& rName ) const;
    void			CreateValidTabName(String& rName) const;
    BOOL			InsertTab( USHORT nPos, const String& rName,
                                BOOL bExternalDocument = FALSE );
/*N*/ 	BOOL            DeleteTab( USHORT nTab, ScDocument* pRefUndoDoc = NULL );
    BOOL			RenameTab( USHORT nTab, const String& rName,
                                BOOL bUpdateRef = TRUE,
                                BOOL bExternalDocument = FALSE );
    ULONG			TransferTab(ScDocument* pSrcDoc, USHORT nSrcPos, USHORT nDestPos,
                                    BOOL bInsertNew = TRUE,
                                    BOOL bResultsOnly = FALSE );
    void			TransferDrawPage(ScDocument* pSrcDoc, USHORT nSrcPos, USHORT nDestPos);
    void			SetVisible( USHORT nTab, BOOL bVisible );
    BOOL			IsVisible( USHORT nTab ) const;
    void			SetScenario( USHORT nTab, BOOL bFlag );
    BOOL			IsScenario( USHORT nTab ) const;
    void			GetScenarioData( USHORT nTab, String& rComment,
                                        Color& rColor, USHORT& rFlags ) const;
    void			SetScenarioData( USHORT nTab, const String& rComment,
                                        const Color& rColor, USHORT nFlags );
    BOOL			IsActiveScenario( USHORT nTab ) const;
    void			SetActiveScenario( USHORT nTab, BOOL bActive );		// nur fuer Undo etc.
    BYTE			GetLinkMode( USHORT nTab ) const;
    BOOL			IsLinked( USHORT nTab ) const;
    const String&	GetLinkDoc( USHORT nTab ) const;
    const String&	GetLinkFlt( USHORT nTab ) const;
    const String&	GetLinkOpt( USHORT nTab ) const;
    const String&	GetLinkTab( USHORT nTab ) const;
    ULONG			GetLinkRefreshDelay( USHORT nTab ) const;
    void			SetLink( USHORT nTab, BYTE nMode, const String& rDoc,
                            const String& rFilter, const String& rOptions,
                            const String& rTabName, ULONG nRefreshDelay );
    BOOL			HasLink( const String& rDoc,
                             const String& rFilter, const String& rOptions ) const;
    BOOL			LinkExternalTab( USHORT& nTab, const String& aDocTab,
                                    const String& aFileName,
                                    const String& aTabName );

                    // originating DDE links
    void            IncInDdeLinkUpdate() { if ( nInDdeLinkUpdate < 255 ) ++nInDdeLinkUpdate; }
    void            DecInDdeLinkUpdate() { if ( nInDdeLinkUpdate ) --nInDdeLinkUpdate; }
    BOOL            IsInDdeLinkUpdate() const   { return nInDdeLinkUpdate != 0; }


                    // Fuer StarOne Api:
    USHORT			GetDdeLinkCount() const;
    BOOL			GetDdeLinkData( USHORT nPos, String& rAppl, String& rTopic, String& rItem ) const;
    BOOL			UpdateDdeLink( const String& rAppl, const String& rTopic, const String& rItem );

                    // For XCL/XML Export (nPos is index of DDE links only):
    BOOL			GetDdeLinkMode(USHORT nPos, USHORT& nMode);
    BOOL			GetDdeLinkResultDimension( USHORT nPos , USHORT& nCol, USHORT& nRow, ScMatrix*& pMatrix);
    BOOL			GetDdeLinkResult(const ScMatrix* pMatrix, USHORT nCol, USHORT nRow, String& rStrValue, double& rDoubValue, BOOL& bIsString);

                    // For XCL/XML Import (nPos is index of DDE links only):
    void            CreateDdeLink(const String& rAppl, const String& rTopic, const String& rItem, const BYTE nMode = SC_DDE_DEFAULT );
    BOOL			FindDdeLink(const String& rAppl, const String& rTopic, const String& rItem, const BYTE nMode, USHORT& nPos );
    BOOL			CreateDdeLinkResultDimension(USHORT nPos, USHORT nCols, USHORT nRows, ScMatrix*& pMatrix);
    void			SetDdeLinkResult(ScMatrix* pMatrix, const USHORT nCol, const USHORT nRow, const String& rStrValue, const double& rDoubValue, BOOL bString, BOOL bEmpty);


    SfxBindings*	GetViewBindings();
    SfxObjectShell* GetDocumentShell() const	{ return pShell; }
    ScDrawLayer*	GetDrawLayer()				{ return pDrawLayer; }

    BOOL			IsChart( SdrObject* pObject );
    void			UpdateAllCharts( BOOL bDoUpdate = TRUE );
    void			UpdateChartRef( UpdateRefMode eUpdateRefMode,
                                    USHORT nCol1, USHORT nRow1, USHORT nTab1,
                                    USHORT nCol2, USHORT nRow2, USHORT nTab2,
                                    short nDx, short nDy, short nDz );
                    //! setzt nur die neue RangeList, keine ChartListener o.ae.

    ScOutlineTable*	GetOutlineTable( USHORT nTab, BOOL bCreate = FALSE );



    void			PutCell( const ScAddress&, ScBaseCell* pCell, BOOL bForceTab = FALSE );
    void			PutCell( USHORT nCol, USHORT nRow, USHORT nTab, ScBaseCell* pCell,
                            BOOL bForceTab = FALSE );
    void			PutCell(USHORT nCol, USHORT nRow, USHORT nTab, ScBaseCell* pCell,
                            ULONG nFormatIndex, BOOL bForceTab = FALSE);
                    //	return TRUE = Zahlformat gesetzt
    BOOL			SetString( USHORT nCol, USHORT nRow, USHORT nTab, const String& rString );
    void			SetValue( USHORT nCol, USHORT nRow, USHORT nTab, const double& rVal );
    void			SetNote( USHORT nCol, USHORT nRow, USHORT nTab, const ScPostIt& rNote );
    void 			SetError( USHORT nCol, USHORT nRow, USHORT nTab, const USHORT nError);

    void 			InsertMatrixFormula(USHORT nCol1, USHORT nRow1,
                                        USHORT nCol2, USHORT nRow2,
                                        const ScMarkData& rMark,
                                        const String& rFormula,
                                        const ScTokenArray* p = NULL );

    void			GetString( USHORT nCol, USHORT nRow, USHORT nTab, String& rString );
    void			GetInputString( USHORT nCol, USHORT nRow, USHORT nTab, String& rString );
    double			GetValue( const ScAddress& );
    void			GetValue( USHORT nCol, USHORT nRow, USHORT nTab, double& rValue );
    double			RoundValueAsShown( double fVal, ULONG nFormat );
    void			GetNumberFormat( USHORT nCol, USHORT nRow, USHORT nTab,
                                     sal_uInt32& rFormat );
    ULONG			GetNumberFormat( const ScAddress& ) const;
                    /// if no number format attribute is set the calculated
                    /// number format of the formula cell is returned
    void			GetNumberFormatInfo( short& nType, ULONG& nIndex,
                        const ScAddress& rPos, const ScFormulaCell& rFCell ) const;
    BOOL			GetNote( USHORT nCol, USHORT nRow, USHORT nTab, ScPostIt& rNote);
    void			GetCellType( USHORT nCol, USHORT nRow, USHORT nTab, CellType& rCellType ) const;
    CellType		GetCellType( const ScAddress& rPos ) const;
    void			GetCell( USHORT nCol, USHORT nRow, USHORT nTab, ScBaseCell*& rpCell ) const;
    ScBaseCell*		GetCell( const ScAddress& rPos ) const;

    void			RefreshNoteFlags();
    BOOL			HasNoteObject( USHORT nCol, USHORT nRow, USHORT nTab ) const;

    BOOL			HasData( USHORT nCol, USHORT nRow, USHORT nTab );
    BOOL			HasStringData( USHORT nCol, USHORT nRow, USHORT nTab ) const;
    BOOL			HasValueData( USHORT nCol, USHORT nRow, USHORT nTab ) const;

    BOOL			ExtendMerge( USHORT nStartCol, USHORT nStartRow,
                                USHORT& rEndCol, USHORT& rEndRow, USHORT nTab,
                                BOOL bRefresh = FALSE, BOOL bAttrs = FALSE );
    BOOL			ExtendMerge( ScRange& rRange, BOOL bRefresh = FALSE, BOOL bAttrs = FALSE );
    BOOL			ExtendOverlapped( USHORT& rStartCol, USHORT& rStartRow,
                                USHORT nEndCol, USHORT nEndRow, USHORT nTab );
    BOOL			ExtendOverlapped( ScRange& rRange );


                    //	ohne Ueberpruefung:
    void			DoMerge( USHORT nTab, USHORT nStartCol, USHORT nStartRow,
                                    USHORT nEndCol, USHORT nEndRow );

    BOOL			IsBlockEmpty( USHORT nTab, USHORT nStartCol, USHORT nStartRow,
                                                USHORT nEndCol, USHORT nEndRow ) const;
    BOOL			IsPrintEmpty( USHORT nTab, USHORT nStartCol, USHORT nStartRow,
                                                USHORT nEndCol, USHORT nEndRow,
                                                BOOL bLeftIsEmpty = FALSE,
                                                ScRange* pLastRange = NULL,
                                                Rectangle* pLastMM = NULL ) const;


    BOOL			HasAttrib( USHORT nCol1, USHORT nRow1, USHORT nTab1,
                            USHORT nCol2, USHORT nRow2, USHORT nTab2, USHORT nMask );
    BOOL			HasAttrib( const ScRange& rRange, USHORT nMask );



    void			ResetChanged( const ScRange& rRange );

    void			SetDirty();
     void			SetDirty( const ScRange& );
     void			SetTableOpDirty( const ScRange& );	// for Interpreter TableOp
    void			CalcAll();
    void			CalcAfterLoad();
    void			CompileXML();

                    // Automatisch Berechnen
    void			SetAutoCalc( BOOL bNewAutoCalc );
    BOOL			GetAutoCalc() const { return bAutoCalc; }
                    // Automatisch Berechnen in/von/fuer ScDocShell disabled
    void			SetAutoCalcShellDisabled( BOOL bNew ) { bAutoCalcShellDisabled = bNew; }
    BOOL			IsAutoCalcShellDisabled() const { return bAutoCalcShellDisabled; }
                    // ForcedFormulas zu berechnen
    void			SetForcedFormulaPending( BOOL bNew ) { bForcedFormulaPending = bNew; }
    BOOL			IsForcedFormulaPending() const { return bForcedFormulaPending; }
                    // if CalcFormulaTree() is currently running
    BOOL			IsCalculatingFormulaTree() { return bCalculatingFormulaTree; }

     USHORT			GetErrCode( const ScAddress& ) const;

    void			GetDataArea( USHORT nTab, USHORT& rStartCol, USHORT& rStartRow,
                                    USHORT& rEndCol, USHORT& rEndRow, BOOL bIncludeOld );
    BOOL			GetCellArea( USHORT nTab, USHORT& rEndCol, USHORT& rEndRow ) const;
    BOOL			GetTableArea( USHORT nTab, USHORT& rEndCol, USHORT& rEndRow ) const;
    BOOL			GetPrintArea( USHORT nTab, USHORT& rEndCol, USHORT& rEndRow,
                                    BOOL bNotes = TRUE ) const;
    void			InvalidateTableArea();

    BOOL			GetDataStart( USHORT nTab, USHORT& rStartCol, USHORT& rStartRow ) const;

    void			ExtendPrintArea( OutputDevice* pDev, USHORT nTab,
                                    USHORT nStartCol, USHORT nStartRow,
                                    USHORT& rEndCol, USHORT nEndRow );


    void			GetNextPos( USHORT& rCol, USHORT& rRow, USHORT nTab, short nMovX, short nMovY,
                                BOOL bMarked, BOOL bUnprotected, const ScMarkData& rMark );

    BOOL			GetNextMarkedCell( USHORT& rCol, USHORT& rRow, USHORT nTab,
                                        const ScMarkData& rMark );


/*N*/ 	BOOL			InsertRow( USHORT nStartCol, USHORT nStartTab,
/*N*/ 							   USHORT nEndCol,   USHORT nEndTab,
/*N*/ 							   USHORT nStartRow, USHORT nSize, ScDocument* pRefUndoDoc = NULL );
/*N*/ 	BOOL			InsertRow( const ScRange& rRange, ScDocument* pRefUndoDoc = NULL );
/*N*/ 	void			DeleteRow( USHORT nStartCol, USHORT nStartTab,
/*N*/ 							   USHORT nEndCol,   USHORT nEndTab,
/*N*/ 							   USHORT nStartRow, USHORT nSize,
/*N*/ 							   ScDocument* pRefUndoDoc = NULL, BOOL* pUndoOutline = NULL );
/*N*/ 	void			DeleteRow( const ScRange& rRange,
/*N*/ 							   ScDocument* pRefUndoDoc = NULL, BOOL* pUndoOutline = NULL );
/*N*/ 	BOOL			InsertCol( USHORT nStartRow, USHORT nStartTab,
/*N*/ 							   USHORT nEndRow,   USHORT nEndTab,
/*N*/ 							   USHORT nStartCol, USHORT nSize, ScDocument* pRefUndoDoc = NULL );
/*N*/ 	BOOL			InsertCol( const ScRange& rRange, ScDocument* pRefUndoDoc = NULL );
/*N*/ 	void			DeleteCol( USHORT nStartRow, USHORT nStartTab,
/*N*/ 							   USHORT nEndRow, USHORT nEndTab,
/*N*/ 							   USHORT nStartCol, USHORT nSize,
/*N*/ 							   ScDocument* pRefUndoDoc = NULL, BOOL* pUndoOutline = NULL );
/*N*/ 	void			DeleteCol( const ScRange& rRange,
/*N*/ 							   ScDocument* pRefUndoDoc = NULL, BOOL* pUndoOutline = NULL );

/*N*/ 	BOOL			CanInsertRow( const ScRange& rRange ) const;
/*N*/ 	BOOL			CanInsertCol( const ScRange& rRange ) const;

    void			FitBlock( const ScRange& rOld, const ScRange& rNew, BOOL bClear = TRUE );
    BOOL			CanFitBlock( const ScRange& rOld, const ScRange& rNew );

    BOOL			IsClipOrUndo() const 						{ return bIsClip || bIsUndo; }
    BOOL			IsUndo() const								{ return bIsUndo; }
    BOOL			IsClipboard() const 						{ return bIsClip; }
    BOOL			IsUndoEnabled() const						{ return !bImportingXML; }
    void			ResetClip( ScDocument* pSourceDoc, const ScMarkData* pMarks );



    void			DeleteArea(USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2,
                            const ScMarkData& rMark, USHORT nDelFlag);
    void			DeleteAreaTab(USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2,
                                USHORT nTab, USHORT nDelFlag);
    void			DeleteAreaTab(const ScRange& rRange, USHORT nDelFlag);
    void			CopyToClip(USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2,
                                BOOL bCut, ScDocument* pClipDoc, BOOL bAllTabs,
                                const ScMarkData* pMarks = NULL,
                                BOOL bKeepScenarioFlags = FALSE, BOOL bIncludeObjects = FALSE);
    void 			CopyBlockFromClip( USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2,
                                    const ScMarkData& rMark, short nDx, short nDy,
                                    const ScCopyBlockFromClipParams* pCBFCP );
    void 			CopyNonFilteredFromClip( USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2,
                                    const ScMarkData& rMark, short nDx, short nDy,
                                    const ScCopyBlockFromClipParams* pCBFCP );
    void 			StartListeningFromClip( USHORT nCol1, USHORT nRow1,
                                        USHORT nCol2, USHORT nRow2,
                                        const ScMarkData& rMark, USHORT nInsFlag );
    void 			BroadcastFromClip( USHORT nCol1, USHORT nRow1,
                                    USHORT nCol2, USHORT nRow2,
                                    const ScMarkData& rMark, USHORT nInsFlag );
    void			CopyFromClip( const ScRange& rDestRange, const ScMarkData& rMark,
                                    USHORT nInsFlag,
                                    ScDocument* pRefUndoDoc = NULL,
                                    ScDocument* pClipDoc = NULL,
                                    BOOL bResetCut = TRUE,
                                    BOOL bAsLink = FALSE,
                                    BOOL bIncludeFiltered = TRUE,
                                    BOOL bSkipAttrForEmpty = FALSE );

    void			GetClipArea(USHORT& nClipX, USHORT& nClipY, BOOL bIncludeFiltered);


    void			InitUndo( ScDocument* pSrcDoc, USHORT nTab1, USHORT nTab2,
                                BOOL bColInfo = FALSE, BOOL bRowInfo = FALSE );

                    //	nicht mehr benutzen:
    void			CopyToDocument(USHORT nCol1, USHORT nRow1, USHORT nTab1,
                                USHORT nCol2, USHORT nRow2, USHORT nTab2,
                                USHORT nFlags, BOOL bMarked, ScDocument* pDestDoc,
                                const ScMarkData* pMarks = NULL, BOOL bColRowFlags = TRUE);

    void			CopyToDocument(const ScRange& rRange,
                                USHORT nFlags, BOOL bMarked, ScDocument* pDestDoc,
                                const ScMarkData* pMarks = NULL, BOOL bColRowFlags = TRUE);

    const ScRangeList* GetScenarioRanges( USHORT nTab ) const;


/*N*/ 	void			UpdateReference( UpdateRefMode eUpdateRefMode, USHORT nCol1, USHORT nRow1, USHORT nTab1,
/*N*/ 									 USHORT nCol2, USHORT nRow2, USHORT nTab2,
/*N*/ 									 short nDx, short nDy, short nDz,
/*N*/ 									 ScDocument* pUndoDoc = NULL, BOOL bIncludeDraw = TRUE );



    void			Fill(	USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2,
                            const ScMarkData& rMark,
                            USHORT nFillCount, FillDir eFillDir = FILL_TO_BOTTOM,
                            FillCmd eFillCmd = FILL_LINEAR, FillDateCmd eFillDateCmd = FILL_DAY,
                            double nStepValue = 1.0, double nMaxValue = 1E307);

    BOOL			GetSelectionFunction( ScSubTotalFunc eFunc,
                                            const ScAddress& rCursor, const ScMarkData& rMark,
                                            double& rResult );

    const SfxPoolItem* 		GetAttr( USHORT nCol, USHORT nRow, USHORT nTab, USHORT nWhich ) const;
    const ScPatternAttr*	GetPattern( USHORT nCol, USHORT nRow, USHORT nTab ) const;
    ScPatternAttr*			CreateSelectionPattern( const ScMarkData& rMark, BOOL bDeep = TRUE );

    const ScConditionalFormat* GetCondFormat( USHORT nCol, USHORT nRow, USHORT nTab ) const;
    const SfxItemSet*	GetCondResult( USHORT nCol, USHORT nRow, USHORT nTab ) const;

    const ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XBreakIterator >& GetBreakIterator();
    BYTE			GetStringScriptType( const String& rString );
    BYTE			GetCellScriptType( ScBaseCell* pCell, ULONG nNumberFormat );
    BYTE			GetScriptType( USHORT nCol, USHORT nRow, USHORT nTab, ScBaseCell* pCell = NULL );

    void			AddDetectiveOperation( const ScDetOpData& rData );
    void			ClearDetectiveOperations();
    ScDetOpList*	GetDetOpList() const				{ return pDetOpList; }

    BOOL			HasDetectiveObjects(USHORT nTab) const;

    void			GetSelectionFrame( const ScMarkData& rMark,
                                       SvxBoxItem&		rLineOuter,
                                       SvxBoxInfoItem&	rLineInner );
    void			ApplySelectionFrame( const ScMarkData& rMark,
                                         const SvxBoxItem* pLineOuter,
                                         const SvxBoxInfoItem* pLineInner );

    void			ClearSelectionItems( const USHORT* pWhich, const ScMarkData& rMark );
    void			ChangeSelectionIndent( BOOL bIncrement, const ScMarkData& rMark );

    ULONG			AddCondFormat( const ScConditionalFormat& rNew );
    void			FindConditionalFormat( ULONG nKey, ScRangeList& rRanges );
    void			FindConditionalFormat( ULONG nKey, ScRangeList& rRanges, USHORT nTab );
    void			ConditionalChanged( ULONG nKey );
    void			SetConditionalUsed( ULONG nKey );		// beim Speichern

    ULONG			AddValidationEntry( const ScValidationData& rNew );
    void			SetValidationUsed( ULONG nKey );		// beim Speichern

    const ScValidationData*	GetValidationEntry( ULONG nIndex ) const;

    ScConditionalFormatList* GetCondFormList() const		// Ref-Undo
                    { return pCondFormList; }

    ScValidationDataList* GetValidationList() const
                    { return pValidationList; }

    void			ApplyAttr( USHORT nCol, USHORT nRow, USHORT nTab,
                                const SfxPoolItem& rAttr );
    void			ApplyPatternArea( USHORT nStartCol, USHORT nStartRow,
                                        USHORT nEndCol, USHORT nEndRow,
                                        const ScMarkData& rMark, const ScPatternAttr& rAttr );
    void			ApplyPatternAreaTab( USHORT nStartCol, USHORT nStartRow,
                                            USHORT nEndCol, USHORT nEndRow, USHORT nTab,
                                            const ScPatternAttr& rAttr );

    void			ApplyStyleArea( USHORT nStartCol, USHORT nStartRow,
                                    USHORT nEndCol, USHORT nEndRow,
                                    const ScMarkData& rMark, const ScStyleSheet& rStyle);
     void			ApplyStyleAreaTab( USHORT nStartCol, USHORT nStartRow,
                                        USHORT nEndCol, USHORT nEndRow, USHORT nTab,
                                        const ScStyleSheet& rStyle);

    void			ApplySelectionStyle( const ScStyleSheet& rStyle, const ScMarkData& rMark );

    const ScStyleSheet*	GetSelectionStyle( const ScMarkData& rMark ) const;

    void			StyleSheetChanged( const SfxStyleSheetBase* pStyleSheet, BOOL bRemoved,
                                        OutputDevice* pDev,
                                        double nPPTX, double nPPTY,
                                        const Fraction& rZoomX, const Fraction& rZoomY );

    BOOL			IsStyleSheetUsed( const ScStyleSheet& rStyle, BOOL bGatherAllStyles ) const;

                    // Rueckgabe TRUE bei ApplyFlags: Wert geaendert
    BOOL			ApplyFlagsTab( USHORT nStartCol, USHORT nStartRow,
                                            USHORT nEndCol, USHORT nEndRow,
                                            USHORT nTab, INT16 nFlags );
    BOOL			RemoveFlagsTab( USHORT nStartCol, USHORT nStartRow,
                                            USHORT nEndCol, USHORT nEndRow,
                                            USHORT nTab, INT16 nFlags );


    void			AutoFormat( USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow,
                                    USHORT nFormatNo, const ScMarkData& rMark );


    BOOL			Solver(USHORT nFCol, USHORT nFRow, USHORT nFTab,
                            USHORT nVCol, USHORT nVRow, USHORT nVTab,
                            const String& sValStr, double& nX);

    void			ApplySelectionPattern( const ScPatternAttr& rAttr, const ScMarkData& rMark );

                    //

    void			SetColWidth( USHORT nCol, USHORT nTab, USHORT nNewWidth );
    void			SetRowHeightRange( USHORT nStartRow, USHORT nEndRow, USHORT nTab,
                                            USHORT nNewHeight );
    void			SetManualHeight( USHORT nStartRow, USHORT nEndRow, USHORT nTab, BOOL bManual );

    USHORT			GetColWidth( USHORT nCol, USHORT nTab ) const;
    USHORT			GetRowHeight( USHORT nRow, USHORT nTab ) const;

    USHORT			GetOriginalWidth( USHORT nCol, USHORT nTab ) const;
    USHORT			GetOriginalHeight( USHORT nRow, USHORT nTab ) const;


    inline USHORT	FastGetRowHeight( USHORT nRow, USHORT nTab ) const;		// ohne Ueberpruefungen!

    USHORT			GetHiddenRowCount( USHORT nRow, USHORT nTab ) const;

    USHORT			GetOptimalColWidth( USHORT nCol, USHORT nTab, OutputDevice* pDev,
                                        double nPPTX, double nPPTY,
                                        const Fraction& rZoomX, const Fraction& rZoomY,
                                        BOOL bFormula,
                                        const ScMarkData* pMarkData = NULL,
                                        BOOL bSimpleTextImport = FALSE );
    BOOL			SetOptimalHeight( USHORT nStartRow, USHORT nEndRow, USHORT nTab, USHORT nExtra,
                                        OutputDevice* pDev,
                                        double nPPTX, double nPPTY,
                                        const Fraction& rZoomX, const Fraction& rZoomY,
                                        BOOL bShrink );

    void			ShowCol(USHORT nCol, USHORT nTab, BOOL bShow);
    void			ShowRows(USHORT nRow1, USHORT nRow2, USHORT nTab, BOOL bShow);
    void			SetRowFlags( USHORT nRow, USHORT nTab, BYTE nNewFlags );

    BYTE			GetColFlags( USHORT nCol, USHORT nTab ) const;
    BYTE			GetRowFlags( USHORT nRow, USHORT nTab ) const;

                    /// @return  the index of the last column with any set flags (auto-pagebreak is ignored).
                    /// @return  the index of the last row with any set flags (auto-pagebreak is ignored).

                    /// @return  the index of the last changed column (flags and column width, auto pagebreak is ignored).
    USHORT          GetLastChangedCol( USHORT nTab ) const;
                    /// @return  the index of the last changed row (flags and row height, auto pagebreak is ignored).
    USHORT          GetLastChangedRow( USHORT nTab ) const;

    USHORT          GetNextDifferentChangedCol( USHORT nTab, USHORT nStart) const;

                    // #108550#; if bCareManualSize is set then the row
                    // heights are compared only if the manual size flag for
                    // the row is set. If the bCareManualSize is not set then
                    // the row heights are always compared.
    USHORT          GetNextDifferentChangedRow( USHORT nTab, USHORT nStart, bool bCareManualSize = true) const;

    // returns whether to export a Default style for this col/row or not
    // nDefault is setted to one possition in the current row/col where the Default style is
    BOOL			GetColDefault( USHORT nTab, USHORT nCol, USHORT nLastRow, USHORT& nDefault);
    BOOL			GetRowDefault( USHORT nTab, USHORT nRow, USHORT nLastCol, USHORT& nDefault);


    BOOL			UpdateOutlineCol( USHORT nStartCol, USHORT nEndCol, USHORT nTab, BOOL bShow );
    BOOL			UpdateOutlineRow( USHORT nStartRow, USHORT nEndRow, USHORT nTab, BOOL bShow );

    ScPatternAttr*		GetDefPattern() const;
    ScDocumentPool*		GetPool();
    ScStyleSheetPool*	GetStyleSheetPool() const;

    // PageStyle:
    const String&	GetPageStyle( USHORT nTab ) const;
    void			SetPageStyle( USHORT nTab, const String& rName );
    Size			GetPageSize( USHORT nTab ) const;
    void			SetPageSize( USHORT nTab, const Size& rSize );
    void			SetRepeatArea( USHORT nTab, USHORT nStartCol, USHORT nEndCol, USHORT nStartRow, USHORT nEndRow );
    void			UpdatePageBreaks( USHORT nTab, const ScRange* pUserArea = NULL );
    void			RemoveManualBreaks( USHORT nTab );

    BOOL			RemovePageStyleInUse( const String& rStrPageStyle );



    // war vorher im PageStyle untergracht. Jetzt an jeder Tabelle:
    BOOL			HasPrintRange();
    USHORT			GetPrintRangeCount( USHORT nTab );
    const ScRange*	GetPrintRange( USHORT nTab, USHORT nPos );
    const ScRange*	GetRepeatColRange( USHORT nTab );
    const ScRange*	GetRepeatRowRange( USHORT nTab );
    void			SetPrintRangeCount( USHORT nTab, USHORT nNew );
    void			SetPrintRange( USHORT nTab, USHORT nPos, const ScRange& rNew );
    void			SetRepeatColRange( USHORT nTab, const ScRange* pNew );
    void			SetRepeatRowRange( USHORT nTab, const ScRange* pNew );
    ScPrintRangeSaver* CreatePrintRangeSaver() const;

    Rectangle		GetMMRect( USHORT nStartCol, USHORT nStartRow,
                                USHORT nEndCol, USHORT nEndRow, USHORT nTab );
    ScRange			GetRange( USHORT nTab, const Rectangle& rMMRect );

    BOOL			LoadPool( SvStream& rStream, BOOL bLoadRefCounts );
    BOOL			SavePool( SvStream& rStream ) const;

    BOOL			Load( SvStream& rStream, ScProgress* pProgress );
    BOOL			Save( SvStream& rStream, ScProgress* pProgress ) const;

    void			UpdStlShtPtrsFrmNms();


    CharSet			GetSrcCharSet() const	{ return eSrcSet; }
    ULONG			GetSrcVersion() const	{ return nSrcVer; }
    USHORT			GetSrcMaxRow() const	{ return nSrcMaxRow; }

    void			SetLostData();
    BOOL			HasLostData() const		{ return bLostData; }

    void			SetSrcCharSet( CharSet eNew )	{ eSrcSet = eNew; }
    void			UpdateFontCharSet();

    friend SvStream& operator>>( SvStream& rStream, ScDocument& rDocument );
    friend SvStream& operator<<( SvStream& rStream, const ScDocument& rDocument );

    SvNumberFormatter*	GetFormatTable() const;

    void			Sort( USHORT nTab, const ScSortParam& rSortParam, BOOL bKeepQuery );
    USHORT			Query( USHORT nTab, const ScQueryParam& rQueryParam, BOOL bKeepSub );
    BOOL			CreateQueryParam( USHORT nCol1, USHORT nRow1, USHORT nCol2, USHORT nRow2,
                                        USHORT nTab, ScQueryParam& rQueryParam );


    BOOL			HasColHeader( USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow,
                                    USHORT nTab );

    SfxPrinter*		GetPrinter();
    void			SetPrinter( SfxPrinter* pNewPrinter );


    void			DoColResize( USHORT nTab, USHORT nCol1, USHORT nCol2, USHORT nAdd );

    // Idleberechnung der OutputDevice-Zelltextbreite
    BOOL			IsLoadingDone() const { return bLoadingDone; }
    void			InvalidateTextWidth( const ScAddress* pAdrFrom = NULL,
                                         const ScAddress* pAdrTo   = NULL,
                                         BOOL bBroadcast = FALSE );

    BOOL			IsIdleDisabled() const		{ return bIdleDisabled; }
    void			DisableIdle(BOOL bDo)		{ bIdleDisabled = bDo; }

    BOOL			IsDetectiveDirty() const	 { return bDetectiveDirty; }
    void			SetDetectiveDirty(BOOL bSet) { bDetectiveDirty = bSet; }

    BYTE			GetMacroCallMode() const	 { return nMacroCallMode; }
    void			SetMacroCallMode(BYTE nNew)	 { nMacroCallMode = nNew; }

    BOOL			GetHasMacroFunc() const		 { return bHasMacroFunc; }
    void			SetHasMacroFunc(BOOL bSet)	 { bHasMacroFunc = bSet; }

    BOOL			CheckMacroWarn();

    // fuer Broadcasting/Listening
    void			SetNoSetDirty( BOOL bVal ) { bNoSetDirty = bVal; }
    BOOL			GetNoSetDirty() const { return bNoSetDirty; }
    void			SetInsertingFromOtherDoc( BOOL bVal ) { bInsertingFromOtherDoc = bVal; }
    BOOL			IsInsertingFromOtherDoc() const { return bInsertingFromOtherDoc; }
    void			SetImportingXML( BOOL bVal );
    BOOL			IsImportingXML() const { return bImportingXML; }
    void			SetCalcingAfterLoad( BOOL bVal ) { bCalcingAfterLoad = bVal; }
    BOOL			IsCalcingAfterLoad() const { return bCalcingAfterLoad; }
    void			SetNoListening( BOOL bVal ) { bNoListening = bVal; }
    BOOL			GetNoListening() const { return bNoListening; }

    ScChartListenerCollection* GetChartListenerCollection() const
                        { return pChartListenerCollection; }
    void			UpdateChart( const String& rName, Window* pWin );
    void			UpdateChartListenerCollection();
    BOOL			IsChartListenerCollectionNeedsUpdate() const
                        { return bChartListenerCollectionNeedsUpdate; }
    void			SetChartListenerCollectionNeedsUpdate( BOOL bFlg )
                        { bChartListenerCollectionNeedsUpdate = bFlg; }
    void			AddOLEObjectToCollection(const String& rName);

    ScChangeViewSettings* GetChangeViewSettings() const		{ return pChangeViewSettings; }
    void				SetChangeViewSettings(const ScChangeViewSettings& rNew);

    vos::ORef<SvxForbiddenCharactersTable> GetForbiddenCharacters();
    void			SetForbiddenCharacters( const vos::ORef<SvxForbiddenCharactersTable> xNew );

    BYTE			GetAsianCompression() const;		// CharacterCompressionType values
    BOOL			IsValidAsianCompression() const;
    void			SetAsianCompression(BYTE nNew);

    BOOL			GetAsianKerning() const;
    BOOL			IsValidAsianKerning() const;
    void			SetAsianKerning(BOOL bNew);

    BYTE			GetEditTextDirection(USHORT nTab) const;	// EEHorizontalTextDirection values

    ScLkUpdMode		GetLinkMode() const				{ return eLinkMode ;}
    void			SetLinkMode( ScLkUpdMode nSet )	{ 	eLinkMode  = nSet;}


private:
    void				SetAutoFilterFlags();

    DECL_LINK( TrackTimeHdl, Timer* );

public:
    void				StartListeningArea( const ScRange& rRange,
                                            SfxListener* pListener );
    void				EndListeningArea( const ScRange& rRange,
                                            SfxListener* pListener );
                        /** Broadcast wrapper, calls
                            rHint.GetCell()->Broadcast() and AreaBroadcast()
                            and TrackFormulas() and conditional format list
                            SourceChanged().
                            Preferred.
                         */
    void                Broadcast( const ScHint& rHint );
                        /// deprecated
    void				Broadcast( ULONG nHint, const ScAddress& rAddr,
                                    ScBaseCell* pCell );
                        /// only area, no cell broadcast
    void                AreaBroadcast( const ScHint& rHint );
                        /// only areas in range, no cell broadcasts
    void                AreaBroadcastInRange( const ScRange& rRange,
                                              const ScHint& rHint );
    void				DelBroadcastAreasInRange( const ScRange& rRange );
/*N*/ 	void				UpdateBroadcastAreas( UpdateRefMode eUpdateRefMode,
/*N*/ 											const ScRange& rRange,
/*N*/ 											short nDx, short nDy, short nDz );


    void				StartListeningCell( const ScAddress& rAddress,
                                            SfxListener* pListener );
    void				EndListeningCell( const ScAddress& rAddress,
                                            SfxListener* pListener );
    void				PutInFormulaTree( ScFormulaCell* pCell );
    void				RemoveFromFormulaTree( ScFormulaCell* pCell );
    void				ClearFormulaTree();
    void				AppendToFormulaTrack( ScFormulaCell* pCell );
    void				RemoveFromFormulaTrack( ScFormulaCell* pCell );
    void				TrackFormulas( ULONG nHintId = SC_HINT_DATACHANGED );
    USHORT				GetFormulaTrackCount() const { return nFormulaTrackCount; }
    BOOL				IsInFormulaTree( ScFormulaCell* pCell ) const;
    BOOL				IsInFormulaTrack( ScFormulaCell* pCell ) const;
    USHORT				GetHardRecalcState() { return nHardRecalcState; }
    void				SetHardRecalcState( USHORT nVal ) { nHardRecalcState = nVal; }
    void				StartAllListeners();
    const ScFormulaCell*	GetFormulaTree() const { return pFormulaTree; }
    BOOL				HasForcedFormulas() const { return bHasForcedFormulas; }
    void				SetForcedFormulas( BOOL bVal ) { bHasForcedFormulas = bVal; }
    ULONG				GetFormulaCodeInTree() const { return nFormulaCodeInTree; }
    BOOL				IsInInterpreter() const { return nInterpretLevel != 0; }
    USHORT				GetInterpretLevel() { return nInterpretLevel; }
    void				IncInterpretLevel()
                            {
                                if ( nInterpretLevel < USHRT_MAX )
                                    nInterpretLevel++;
                            }
    void				DecInterpretLevel()
                            {
                                if ( nInterpretLevel )
                                    nInterpretLevel--;
                            }
    BOOL				IsInMacroInterpreter() const { return nMacroInterpretLevel != 0; }
    USHORT				GetMacroInterpretLevel() { return nMacroInterpretLevel; }
    void				IncMacroInterpretLevel()
                            {
                                if ( nMacroInterpretLevel < USHRT_MAX )
                                    nMacroInterpretLevel++;
                            }
    void				DecMacroInterpretLevel()
                            {
                                if ( nMacroInterpretLevel )
                                    nMacroInterpretLevel--;
                            }
    BOOL				IsInInterpreterTableOp() const { return nInterpreterTableOpLevel != 0; }
    USHORT				GetInterpreterTableOpLevel() { return nInterpreterTableOpLevel; }
    void				IncInterpreterTableOpLevel()
                            {
                                if ( nInterpreterTableOpLevel < USHRT_MAX )
                                    nInterpreterTableOpLevel++;
                            }
    void				DecInterpreterTableOpLevel()
                            {
                                if ( nInterpreterTableOpLevel )
                                    nInterpreterTableOpLevel--;
                            }
                        // add a formula to be remembered for TableOp broadcasts
    void                AddTableOpFormulaCell( ScFormulaCell* );
    void                InvalidateLastTableOpParams() { aLastTableOpParams.bValid = FALSE; }
    BOOL				IsInDtorClear() const { return bInDtorClear; }
    void				SetExpandRefs( BOOL bVal ) { bExpandRefs = bVal; }
    BOOL				IsExpandRefs() { return bExpandRefs; }

    void				IncSizeRecalcLevel( USHORT nTab );
    void				DecSizeRecalcLevel( USHORT nTab );

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


    void 			CompileDBFormula();
    void 			CompileDBFormula( BOOL bCreateFormulaString );
    void 			CompileNameFormula( BOOL bCreateFormulaString );
    void 			CompileColRowNameFormula();

    // maximale Stringlaengen einer Column, fuer z.B. dBase Export


    ScChangeTrack*		GetChangeTrack() const { return pChangeTrack; }

    //! only for import filter, deletes any existing ChangeTrack via
    //! EndChangeTracking() and takes ownership of new ChangeTrack pTrack
    void			SetChangeTrack( ScChangeTrack* pTrack );

/*N*/ 	void			StartChangeTracking();
/*N*/ 	void			EndChangeTracking();


    void			AddUnoObject( SfxListener& rObject );
    void			RemoveUnoObject( SfxListener& rObject );
    void			BroadcastUno( const SfxHint &rHint );

    void			SetInLinkUpdate(BOOL bSet);				// TableLink or AreaLink

    SfxItemPool*		GetEditPool() const;
    SfxItemPool*		GetEnginePool() const;
    ScFieldEditEngine&	GetEditEngine();


    ScRefreshTimerControl*	GetRefreshTimerControl() const
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

    void            InvalidateStyleSheetUsage()
                        { bStyleSheetUsageInvalid = TRUE; }

private: // CLOOK-Impl-Methoden
    void	ImplLoadDocOptions( SvStream& rStream );
    void	ImplLoadViewOptions( SvStream& rStream );
    void	ImplSaveDocOptions( SvStream& rStream ) const;
    void	ImplSaveViewOptions( SvStream& rStream ) const;
    void	ImplCreateOptions(); // bei Gelegenheit auf on-demand umstellen?
    void	ImplDeleteOptions();

    void	DeleteDrawLayer();
    void	DeleteColorTable();
    void	LoadDrawLayer(SvStream& rStream);
    void	StoreDrawLayer(SvStream& rStream) const;
    BOOL	DrawGetPrintArea( ScRange& rRange, BOOL bSetHor, BOOL bSetVer ) const;

    void	UpdateDrawPrinter();
    void	UpdateDrawLanguages();

    void	LoadDdeLinks(SvStream& rStream);
    void	SaveDdeLinks(SvStream& rStream) const;
    void	LoadAreaLinks(SvStream& rStream);
    void	SaveAreaLinks(SvStream& rStream) const;

    void	UpdateRefAreaLinks( UpdateRefMode eUpdateRefMode,
                             const ScRange& r, short nDx, short nDy, short nDz );


};


inline USHORT ScDocument::FastGetRowHeight( USHORT nRow, USHORT nTab ) const
{
    return ( pTab[nTab]->pRowFlags[nRow] & CR_HIDDEN ) ? 0 : pTab[nTab]->pRowHeight[nRow];
}



} //namespace binfilter
#endif


