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
#ifndef SW_DOC_HXX
#define SW_DOC_HXX

/** SwDoc interfaces */

#include <IInterface.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentDeviceAccess.hxx>
#include <IDocumentMarkAccess.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentLinksAdministration.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentContentOperations.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <IDocumentLineNumberAccess.hxx>
#include <IDocumentStatistics.hxx>
#include <IDocumentState.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentTimerAccess.hxx>
#include <IDocumentChartDataProviderAccess.hxx>
// --> OD 2007-10-26 #i83479#
#include <IDocumentOutlineNodes.hxx>
#include <IDocumentListItems.hxx>
#include <set>
// <--
// --> OD 2008-03-12 #refactorlists#
#include <IDocumentListsAccess.hxx>
class SwList;
// <--
#include <IDocumentExternalData.hxx>
#define _SVSTDARR_STRINGSDTOR
#include <svl/svstdarr.hxx>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <vcl/timer.hxx>
#include "swdllapi.h"
#include <swtypes.hxx>
#include <swatrset.hxx>
#include <toxe.hxx>             // enums
#include <flyenum.hxx>
#include <itabenum.hxx>
#include <swdbdata.hxx>
#include <chcmprse.hxx>
#include <com/sun/star/linguistic2/XSpellChecker1.hpp>
#include <com/sun/star/linguistic2/XHyphenatedWord.hpp>
#include <vos/ref.hxx>
#include <svx/svdtypes.hxx>
#include <sfx2/objsh.hxx>
#include <svl/style.hxx>
#include <editeng/numitem.hxx>
#include "comphelper/implementationreference.hxx"
#include <com/sun/star/chart2/data/XDataProvider.hpp>
#include <com/sun/star/linguistic2/XProofreadingIterator.hpp>
#ifdef FUTURE_VBA
#include <com/sun/star/script/vba/XVBAEventProcessor.hpp>
#endif

#include <hash_map>
#include <stringhash.hxx>

#include <svtools/embedhlp.hxx>
#include <vector>
#include <set>
#include <map>
#include <memory>

#include <boost/scoped_ptr.hpp>

class SvxForbiddenCharactersTable;
class SwExtTextInput;
class DateTime;
class EditFieldInfo;
class JobSetup;
class Color;
class KeyCode;
class Outliner;
class OutputDevice;
class Point;
class SbxArray;
class SdrModel;
class SdrObject;
class SdrUndoAction;
class VirtualDevice;
class SfxPrinter;
class SvNumberFormatter;
class SvStringsSort;
class SvUShorts;
class SvUShortsSort;
class SvxMacro;
class SvxMacroTableDtor;
class SvxBorderLine;
class SwAutoCompleteWord;
class SwAutoCorrExceptWord;
class SwCalc;
class SwCellFrm;
class SwCharFmt;
class SwCharFmts;
class SwConditionTxtFmtColl;
class SwCrsrShell;
class SwCursor;
class SwDBNameInfField;
class SwDocShell;
class SwDocUpdtFld;
class SwDrawFrmFmt;
class SwDrawView;
class SwEditShell;
class SwFieldType;
class SwField;
class SwTxtFld;
class SwFldTypes;
class SwFlyFrmFmt;
class SwFmt;
class SwFmtINetFmt;
class SwFmtRefMark;
class SwFrmFmt;
class SwFrmFmts;
class SwFtnIdxs;
class SwFtnInfo;
class SwEndNoteInfo;
class GraphicObject;
class SwGrfFmtColl;
class SwGrfFmtColls;
class SwLineNumberInfo;
class SwNewDBMgr;
class SwNoTxtNode;
class SwNodeIndex;
class SwNodeRange;
class SwNodes;
class SwNumRule;
class SwNumRuleTbl;
class SwPageDesc;
class SwPosFlyFrms;
class SwPagePreViewPrtData;
class SwRedline;
class SwRedlineTbl;
class SwRootFrm;
class SwRubyList;
class SwRubyListEntry;
class SwSectionFmt;
class SwSectionFmts;
class SwSectionData;
class SwSelBoxes;
class SwSpzFrmFmts;
class SwTOXBase;
class SwTOXBaseSection;
class SwTOXMark;
class SwTOXMarks;
class SwTOXType;
class SwTOXTypes;
class SwTabCols;
class SwTable;
class SwTableAutoFmt;
class SwTableBox;
class SwTableBoxFmt;
class SwTableFmt;
class SwTableLineFmt;
class SwTableNode;
class SwTextBlocks;
class SwTxtFmtColl;
class SwTxtFmtColls;
class SwURLStateChanged;
class SwUnoCrsr;
class SwUnoCrsrTbl;
class ViewShell;
class _SetGetExpFld;
class SwDrawContact;
class SwLayouter;
class SdrView;
class SdrMarkList;
class SwAuthEntry;
class SwLayoutCache;
class IStyleAccess;
struct SwCallMouseEvent;
struct SwDocStat;
struct SwHash;
struct SwSortOptions;
struct SwDefTOXBase_Impl;
class SwPrintData;
class SwPrintUIOptions;
class SdrPageView;
struct SwConversionArgs;
class SwRewriter;
class SwMsgPoolItem;
class SwChartDataProvider;
class SwChartLockController_Helper;
class IGrammarContact;
class SwPrintData;
class SwRenderData;
class SwPageFrm;
class SwViewOption;
class IDocumentUndoRedo;

namespace sw { namespace mark {
    class MarkManager;
}}
namespace sw {
    class MetaFieldManager;
    class UndoManager;
    class IShellCursorSupplier;
}

namespace com { namespace sun { namespace star {
namespace i18n {
    struct ForbiddenCharacters;    // comes from the I18N UNO interface
}
namespace uno {
    template < class > class Sequence;
}
namespace container {
    class XNameContainer; // for getXForms()/isXForms()/initXForms() methods
}
}}}

namespace sfx2 {
    class SvLinkSource;
    class IXmlIdRegistry;
    class LinkManager;
}

//PageDescriptor-Schnittstelle, Array hier wegen inlines.
typedef SwPageDesc* SwPageDescPtr;
SV_DECL_PTRARR_DEL( SwPageDescs, SwPageDescPtr, 4, 4 )

// forward declartion
void SetAllScriptItem( SfxItemSet& rSet, const SfxPoolItem& rItem );

// global function to start grammar checking in the document
void StartGrammarChecking( SwDoc &rDoc );

class SW_DLLPUBLIC SwDoc :
    public IInterface,
    public IDocumentSettingAccess,
    public IDocumentDeviceAccess,
    public IDocumentRedlineAccess,
    public IDocumentLinksAdministration,
    public IDocumentFieldsAccess,
    public IDocumentContentOperations,
    public IDocumentStylePoolAccess,
    public IDocumentLineNumberAccess,
    public IDocumentStatistics,
    public IDocumentState,
    public IDocumentDrawModelAccess,
    public IDocumentLayoutAccess,
    public IDocumentTimerAccess,
    public IDocumentChartDataProviderAccess,
    // --> OD 2007-10-26 #i83479#
    public IDocumentListItems,
    public IDocumentOutlineNodes,
    // <--
    // --> OD 2008-03-12 #refactorlists#
    public IDocumentListsAccess,
    // <--
    public IDocumentExternalData
{

    friend void _InitCore();
    friend void _FinitCore();

    //---------------- private Member --------------------------------

    // -------------------------------------------------------------------
    ::boost::scoped_ptr<SwNodes> m_pNodes;  /// document content (Nodes Array)
    SwAttrPool* mpAttrPool;             // der Attribut Pool
    SwPageDescs aPageDescs;             // PageDescriptoren
    Link        aOle2Link;              // OLE 2.0-Benachrichtigung
    /* @@@MAINTAINABILITY-HORROR@@@
       Timer should not be members of the model
    */
    Timer       aIdleTimer;             // der eigene IdleTimer
    Timer       aOLEModifiedTimer;      // Timer for update modified OLE-Objecs
    SwDBData    aDBData;                // database descriptor
    ::com::sun::star::uno::Sequence <sal_Int8 > aRedlinePasswd;
    String      sTOIAutoMarkURL;        // ::com::sun::star::util::URL of table of index AutoMark file
    SvStringsDtor aPatternNms;          // Array fuer die Namen der Dokument-Vorlagen
    com::sun::star::uno::Reference<com::sun::star::container::XNameContainer>
        xXForms;                        // container with XForms models
    mutable com::sun::star::uno::Reference< com::sun::star::linguistic2::XProofreadingIterator > m_xGCIterator;

    const ::boost::scoped_ptr< ::sw::mark::MarkManager> pMarkManager;
    const ::boost::scoped_ptr< ::sw::MetaFieldManager > m_pMetaFieldManager;
    const ::boost::scoped_ptr< ::sw::UndoManager > m_pUndoManager;

    // -------------------------------------------------------------------
    // die Pointer
                                //Defaultformate
    SwFrmFmt        *pDfltFrmFmt;
     SwFrmFmt       *pEmptyPageFmt;     // Format fuer die Default-Leerseite.
    SwFrmFmt        *pColumnContFmt;    // Format fuer Spaltencontainer
    SwCharFmt       *pDfltCharFmt;
    SwTxtFmtColl    *pDfltTxtFmtColl;   // Defaultformatcollections
    SwGrfFmtColl    *pDfltGrfFmtColl;

    SwFrmFmts       *pFrmFmtTbl;        // Formattabellen
    SwCharFmts      *pCharFmtTbl;
    SwSpzFrmFmts    *pSpzFrmFmtTbl;
    SwSectionFmts   *pSectionFmtTbl;
    SwFrmFmts       *pTblFrmFmtTbl;     // spz. fuer Tabellen
    SwTxtFmtColls   *pTxtFmtCollTbl;    // FormatCollections
    SwGrfFmtColls   *pGrfFmtCollTbl;

    SwTOXTypes      *pTOXTypes;         // Verzeichnisse
    SwDefTOXBase_Impl * pDefTOXBases;   // defaults of SwTOXBase's

    ViewShell       *pCurrentView;  // SwDoc should get a new member pCurrentView//swmod 071225
    SdrModel        *pDrawModel;        // StarView Drawing

    SwDocUpdtFld    *pUpdtFlds;         // Struktur zum Field-Update
    SwFldTypes      *pFldTypes;         // Feldtypen
    SwNewDBMgr      *pNewDBMgr;         // Pointer auf den neuen DBMgr fuer
                                        // Evaluierung der DB-Fields

    VirtualDevice   *pVirDev;           // can be used for formatting
    SfxPrinter      *pPrt;              // can be used for formatting
    SwPrintData     *pPrtData;          // Print configuration

    SwDoc           *pGlossaryDoc;      // Pointer auf das Glossary-Dokument. Dieses

    SwNumRule       *pOutlineRule;
    SwFtnInfo       *pFtnInfo;
    SwEndNoteInfo   *pEndNoteInfo;
    SwLineNumberInfo*pLineNumberInfo;
    SwFtnIdxs       *pFtnIdxs;
    SwDocStat       *pDocStat;          // Statistic Informationen
    SvxMacroTableDtor *pMacroTable;     // Tabelle der dokumentglobalen Macros

    SwDocShell      *pDocShell;         // Ptr auf die SfxDocShell vom Doc
    SfxObjectShellLock xTmpDocShell;    // A temporary shell that is used to copy OLE-Nodes

    sfx2::LinkManager   *pLinkMgr;          // Liste von Verknuepften (Grafiken/DDE/OLE)

    SwAutoCorrExceptWord *pACEWord;     // fuer die automatische Uebernahme von
                                        // autokorrigierten Woertern, die "zurueck"
                                        // verbessert werden
    SwURLStateChanged *pURLStateChgd;   // SfxClient fuer Aenderungen in der
                                        // INetHistory
    SvNumberFormatter *pNumberFormatter;    // NumFormatter fuer die Tabellen/Felder

    mutable SwNumRuleTbl    *pNumRuleTbl;           // Liste aller benannten NumRules

    // Hash map to find numrules by name
    mutable std::hash_map<String, SwNumRule *, StringHash> maNumRuleMap;

    // --> OD 2008-03-12 #refactorlists#
    typedef std::hash_map< String, SwList*, StringHash > tHashMapForLists;
    // container to hold the lists of the text document
    tHashMapForLists maLists;
    // relation between list style and its default list
    tHashMapForLists maListStyleLists;
    // <--

    SwRedlineTbl    *pRedlineTbl;           // Liste aller Redlines
    String          *pAutoFmtRedlnComment;  // Kommentar fuer Redlines, die
                                        // uebers Autoformat eingefuegt werden
    SwUnoCrsrTbl    *pUnoCrsrTbl;

    SwPagePreViewPrtData *pPgPViewPrtData;      // Einzuege/Abstaende fuers
                                        // Drucken der Seitenansicht
    SwPaM           *pExtInputRing;

    SwLayouter      *pLayouter;     // ::com::sun::star::frame::Controller for complex layout formatting
                                    // like footnote/endnote in sections
    IStyleAccess    *pStyleAccess;  // handling of automatic styles
    SwLayoutCache   *pLayoutCache;  // Layout cache to read and save with the
                                    // document for a faster formatting

    SwModify *pUnoCallBack;
    IGrammarContact *mpGrammarContact;   // for grammar checking in paragraphs during editing

    mutable  comphelper::ImplementationReference< SwChartDataProvider
        , ::com::sun::star::chart2::data::XDataProvider >
                                aChartDataProviderImplRef;
    SwChartLockController_Helper  *pChartControllerHelper;

    // table of forbidden characters of this document
    vos::ORef<SvxForbiddenCharactersTable>  xForbiddenCharsTable;
#ifdef FUTURE_VBA
    com::sun::star::uno::Reference< com::sun::star::script::vba::XVBAEventProcessor > mxVbaEvents;
#endif
    // --> OD 2007-10-26 #i83479#
public:
    struct lessThanNodeNum
    {
        bool operator()( const SwNodeNum* pNodeNumOne,
                         const SwNodeNum* pNodeNumTwo ) const;
    };

    typedef ::std::set< const SwNodeNum*, lessThanNodeNum > tImplSortedNodeNumList;
private:
    tImplSortedNodeNumList* mpListItemsList;
    // <--

    ::std::auto_ptr< ::sfx2::IXmlIdRegistry > m_pXmlIdRegistry;

    // -------------------------------------------------------------------
    // sonstige

    sal_uInt16 nAutoFmtRedlnCommentNo;  // SeqNo fuers UI-seitige zusammenfassen
                                    // von AutoFmt-Redlines. Wird vom SwAutoFmt
                                    // verwaltet!
    sal_uInt16  nLinkUpdMode;       // UpdateMode fuer Links
    SwFldUpdateFlags    eFldUpdMode;        // Mode fuer Felder/Charts automatisch aktualisieren
    RedlineMode_t eRedlineMode;     // aktueller Redline Modus
    SwCharCompressType eChrCmprType;    // for ASIAN: compress punctuation/kana

    sal_Int32   mReferenceCount;
    sal_Int32   mIdleBlockCount;
    sal_Int8    nLockExpFld;        // Wenn != 0 hat UpdateExpFlds() keine Wirkung

    /* Draw Model Layer IDs
     * LayerIds, Heaven == ueber dem Dokument
     * Hell   == unter dem Dokument
     * Controls == ganz oben
     */
    SdrLayerID  nHeaven;
    SdrLayerID  nHell;
    SdrLayerID  nControls;
    SdrLayerID  nInvisibleHeaven;
    SdrLayerID  nInvisibleHell;
    SdrLayerID  nInvisibleControls;

    bool mbGlossDoc              : 1;    //sal_True: ist ein Textbaustein Dokument
    bool mbModified              : 1;    //sal_True: Dokument ist veraendert
    bool mbDtor                  : 1;    //sal_True: ist im SwDoc DTOR
                                         //      leider auch temporaer von
                                         //      SwSwgReader::InLayout(), wenn fehlerhafte
                                         //      Frames geloescht werden muessen
    bool mbPageNums              : 1;    // TRUE: es gibt virtuelle Seitennummern
    bool mbLoaded                : 1;    // TRUE: ein geladenes Doc
    bool mbUpdateExpFld          : 1;    // TRUE: Expression-Felder updaten
    bool mbNewDoc                : 1;    // TRUE: neues Doc
    bool mbNewFldLst             : 1;    // TRUE: Felder-Liste neu aufbauen
    bool mbCopyIsMove            : 1;    // TRUE: Copy ist ein verstecktes Move
    bool mbVisibleLinks          : 1;    // TRUE: Links werden sichtbar eingefuegt
    bool mbInReading             : 1;    // TRUE: Dokument wird gerade gelesen
    bool mbInXMLImport           : 1;    // TRUE: During xml import, attribute portion building is not necessary
    bool mbUpdateTOX             : 1;    // TRUE: nach Dokument laden die TOX Updaten
    bool mbInLoadAsynchron       : 1;    // TRUE: Dokument wird gerade asynchron geladen
    bool mbHTMLMode              : 1;    // TRUE: Dokument ist im HTMLMode
    bool mbInCallModified        : 1;    // TRUE: im Set/Reset-Modified Link
    bool mbIsGlobalDoc           : 1;    // TRUE: es ist ein GlobalDokument
    bool mbGlblDocSaveLinks      : 1;    // TRUE: im GlobalDoc. gelinkte Sect. mit speichern
    bool mbIsLabelDoc            : 1;    // TRUE: es ist ein Etiketten-Dokument
    bool mbIsAutoFmtRedline      : 1;    // TRUE: die Redlines werden vom Autoformat aufgezeichnet
    bool mbOLEPrtNotifyPending   : 1;    // TRUE: Printer  ist geaendert und beim
                                         //       Erzeugen der ::com::sun::star::sdbcx::View ist eine Benachrichtigung
                                         //       der OLE-Objekte PrtOLENotify() notwendig.
    bool mbAllOLENotify          : 1;    // True: Benachrichtigung aller Objekte ist notwendig
    bool mbIsRedlineMove         : 1;    // True: die Redlines werden in/aus der Section verschoben
    bool mbInsOnlyTxtGlssry      : 1;    // True: insert 'only text' glossary into doc
    bool mbContains_MSVBasic     : 1;    // True: MS-VBasic exist is in our storage
    bool mbPurgeOLE              : 1;    // sal_True: Purge OLE-Objects
    bool mbKernAsianPunctuation  : 1;    // sal_True: kerning also for ASIAN punctuation
    bool mbReadlineChecked       : 1;    // sal_True: if the query was already shown
    bool mbLinksUpdated          : 1;    // OD 2005-02-11 #i38810#
                                         // flag indicating, that the links have been updated.
    bool mbClipBoard             : 1;    // true: this document represents the clipboard
    bool mbColumnSelection       : 1;    // true: this content has bee created by a column selection
                                         //       (clipboard docs only)

#ifdef DBG_UTIL
    bool mbXMLExport : 1;                // sal_True: during XML export
#endif

    // --> OD 2006-03-21 #b6375613#
    // Document flag to trigger conversion, which applys the workaround for documents,
    // which uses a certain layout defect in OOo 1.x to layout the documents.
    // This conversion is performed, when the frames for the layout are created.
    // Thus, this document flag has to be set after load a document and before
    // creating the document view.
    bool mbApplyWorkaroundForB6375613 : 1;
    // <--

    //
    // COMPATIBILITY FLAGS START
    //
    //
    // HISTORY OF THE COMPATIBILITY FLAGS:
    //
    // SO5:
    // mbParaSpaceMax                        def = sal_False, sal_True since SO8
    // mbParaSpaceMaxAtPages                 def = sal_False, sal_True since SO8
    //
    // SO6:
    // mbTabCompat                           def = sal_False, sal_True since SO8
    //
    // SO7:
    // mbUseVirtualDevice                    def = sal_True
    // mbAddFlyOffsets                       def = sal_False, hidden
    //
    // SO7pp1:
    // bOldNumbering                        def = sal_False, hidden
    //
    // SO8:
    // mbAddExternalLeading                  def = sal_True
    // mbUseHiResolutionVirtualDevice        def = sal_True, hidden
    // mbOldLineSpacing                      def = sal_False
    // mbAddParaSpacingToTableCells          def = sal_True
    // mbUseFormerObjectPos                  def = sal_False
    // mbUseFormerTextWrapping               def = sal_False
    // mbConsiderWrapOnObjPos                def = sal_False
    //
    // SO8pp1:
    // mbIgnoreFirstLineIndentInNumbering    def = sal_False, hidden
    // mbDoNotJustifyLinesWithManualBreak    def = sal_False, hidden
    // mbDoNotResetParaAttrsForNumFont       def = sal_False, hidden
    //
    // SO8pp3
    // mbDoNotCaptureDrawObjsOnPage         def = sal_False, hidden
    // - Relevant for drawing objects, which don't follow the text flow, but
    //   whose position is outside the page area:
    //   sal_False: Such drawing objects are captured on the page area of its anchor.
    //   sal_True: Such drawing objects can leave the page area, they aren't captured.
    // mbTableRowKeep                            def = sal_False, hidden
    // mbIgnoreTabsAndBlanksForLineCalculation   def = sal_False, hidden
    // mbClipAsCharacterAnchoredWriterFlyFrame   def = sal_False, hidden
    // - Introduced in order to re-activate clipping of as-character anchored
    //   Writer fly frames in method <SwFlyInCntFrm::MakeAll()> for documents,
    //   which are created with version prior SO8/OOo 2.0
    //
    // SO8pp4
    // mbUnixForceZeroExtLeading                def = sal_False, hidden
    //
    // SO8pu8
    // mbOldPrinterMetrics                      def = sal_False, hidden
    //
    // SO9
    // #i24363# tab stops relative to indent
    // mbTabRelativeToIndent                    def = sal_True, hidden
    // #i89181# suppress tab stop at left indent for paragraphs in lists, whose
    // list level position and space mode equals LABEL_ALIGNMENT and whose list
    // label is followed by a tab character.
    // mbTabAtLeftIndentForParagraphsInList     def = sal_False, hidden

    bool mbParaSpaceMax                     : 1;
    bool mbParaSpaceMaxAtPages              : 1;
    bool mbTabCompat                        : 1;
    bool mbUseVirtualDevice                 : 1;
    bool mbAddFlyOffsets                    : 1;
    bool mbAddExternalLeading               : 1;
    bool mbUseHiResolutionVirtualDevice     : 1;
    bool mbOldLineSpacing                   : 1;    // OD  2004-01-06 #i11859#
    bool mbAddParaSpacingToTableCells       : 1;    // OD  2004-02-16 #106629#
    bool mbUseFormerObjectPos               : 1;    // OD  2004-03-12 #i11860#
    bool mbUseFormerTextWrapping            : 1;    // FME 2005-05-11 #108724#
    bool mbConsiderWrapOnObjPos             : 1;    // OD  2004-05-05 #i28701#
                                                    // sal_True: object positioning algorithm has consider the wrapping style of                                                    //       the floating screen objects as given by its attribute 'WrapInfluenceOnObjPos'
    bool mbMathBaselineAlignment            : 1;    // TL  2010-10-29 #i972#

    // non-ui-compatibility flags:
    bool mbOldNumbering                             : 1;   // HBRINKM #111955#
    bool mbIgnoreFirstLineIndentInNumbering         : 1;   // FME 2005-05-30# i47448#
    bool mbDoNotJustifyLinesWithManualBreak         : 1;   // FME 2005-06-08 #i49277#
    bool mbDoNotResetParaAttrsForNumFont            : 1;   // FME 2005-08-11 #i53199#
    bool mbTableRowKeep                             : 1;   // FME 2006-02-10 #131283#
    bool mbIgnoreTabsAndBlanksForLineCalculation    : 1;   // FME 2006-03-01 #i3952#
    bool mbDoNotCaptureDrawObjsOnPage               : 1;   // OD 2006-03-14 #i62875#
    bool mbOutlineLevelYieldsOutlineRule            : 1;
    bool mbClipAsCharacterAnchoredWriterFlyFrames   : 1;   // OD 2006-04-13 #b6402800#
    bool mbUnixForceZeroExtLeading                  : 1;   // FME 2006-10-09 #i60945#
    bool mbOldPrinterMetrics                        : 1;   // FME 2007-05-14 #147385#
    bool mbTabRelativeToIndent                      : 1;   // #i24363# tab stops relative to indent
    bool mbProtectForm                              : 1;
    bool mbTabAtLeftIndentForParagraphsInList;             // OD 2008-06-05 #i89181# - see above

    bool mbLastBrowseMode                           : 1;

    // #i78591#
    sal_uInt32  n32DummyCompatabilityOptions1;
    sal_uInt32  n32DummyCompatabilityOptions2;
    //
    // COMPATIBILITY FLAGS END
    //

    sal_Bool    mbStartIdleTimer                 ;    // idle timer mode start/stop

    static SwAutoCompleteWord *pACmpltWords;    // Liste aller Worte fuers AutoComplete

    //---------------- private Methoden ------------------------------
    void checkRedlining(RedlineMode_t& _rReadlineMode);

    DECL_LINK( AddDrawUndo, SdrUndoAction * );
                                        // DrawModel
    void DrawNotifyUndoHdl();   // wegen CLOOKs

        // nur fuer den internen Gebrauch deshalb privat.
        // Kopieren eines Bereiches im oder in ein anderes Dokument !
        // Die Position darf nicht im Bereich liegen !!
    bool CopyImpl( SwPaM&, SwPosition&, const bool MakeNewFrms /*= true */,
            const bool bCopyAll, SwPaM *const pCpyRng /*= 0*/ ) const;

    SwFlyFrmFmt* _MakeFlySection( const SwPosition& rAnchPos,
                                const SwCntntNode& rNode, RndStdIds eRequestId,
                                const SfxItemSet* pFlyAttrSet,
                                SwFrmFmt* = 0 );

    SwFlyFrmFmt* _InsNoTxtNode( const SwPosition&rPos, SwNoTxtNode*,
                                const SfxItemSet* pFlyAttrSet,
                                const SfxItemSet* pGrfAttrSet,
                                SwFrmFmt* = 0 );

    void CopyFlyInFlyImpl(  const SwNodeRange& rRg,
                            const xub_StrLen nEndContentIndex,
                            const SwNodeIndex& rStartIdx,
                            const bool bCopyFlyAtFly = false ) const;
    sal_Int8 SetFlyFrmAnchor( SwFrmFmt& rFlyFmt, SfxItemSet& rSet, sal_Bool bNewFrms );

    // --> OD 2005-01-13 #i40550#
    typedef SwFmt* (SwDoc:: *FNCopyFmt)( const String&, SwFmt*, sal_Bool, sal_Bool );
    // <--
    SwFmt* CopyFmt( const SwFmt& rFmt, const SvPtrarr& rFmtArr,
                        FNCopyFmt fnCopyFmt, const SwFmt& rDfltFmt );
    void CopyFmtArr( const SvPtrarr& rSourceArr, SvPtrarr& rDestArr,
                        FNCopyFmt fnCopyFmt, SwFmt& rDfltFmt );
    void CopyPageDescHeaderFooterImpl( bool bCpyHeader,
                                const SwFrmFmt& rSrcFmt, SwFrmFmt& rDestFmt );
    SwFmt* FindFmtByName( const SvPtrarr& rFmtArr,
                                    const String& rName ) const;

    VirtualDevice& CreateVirtualDevice_() const;
    SfxPrinter& CreatePrinter_() const;
    void        PrtDataChanged();   //Printer oder JobSetup geandert, es muss
                                    //fuer entsprechende Invalidierungen und
                                    //Benachrichtigungen gesorgt werden.

    // gcc: aFtnInfo::CopyCtor ist private, also muessen wir uns auch schuetzen
    SwDoc( const SwDoc &);

    // fuer Felder:
    void _InitFieldTypes();     // wird vom CTOR gerufen!!
    void _MakeFldList( int eMode );

    // Datenbankfelder:
    void UpdateDBNumFlds( SwDBNameInfField& rDBFld, SwCalc& rCalc );
    void AddUsedDBToList( SvStringsDtor& rDBNameList,
                          const SvStringsDtor& rUsedDBNames );
    void AddUsedDBToList( SvStringsDtor& rDBNameList, const String& rDBName );
    sal_Bool IsNameInArray( const SvStringsDtor& rOldNames, const String& rName );
    void GetAllDBNames( SvStringsDtor& rAllDBNames );
    void ReplaceUsedDBs( const SvStringsDtor& rUsedDBNames,
                        const String& rNewName, String& rFormel );
    SvStringsDtor& FindUsedDBs( const SvStringsDtor& rAllDBNames,
                                const String& rFormel,
                                SvStringsDtor& rUsedDBNames );

    void InitDrawModel();
    void ReleaseDrawModel();

    void _CreateNumberFormatter();

    sal_Bool _UnProtectTblCells( SwTable& rTbl );

    // erzeuge Anhand der vorgebenen Collection Teildokumente
    // falls keine angegeben ist, nehme die Kapitelvorlage der 1. Ebene
    sal_Bool SplitDoc( sal_uInt16 eDocType, const String& rPath,
                        const SwTxtFmtColl* pSplitColl );
    sal_Bool SplitDoc( sal_uInt16 eDocType, const String& rPath, int nOutlineLevel = 0 ); //#outline level,add by zhaijianwei.


    // Charts der angegebenen Tabelle updaten
    void _UpdateCharts( const SwTable& rTbl, ViewShell& rVSh ) const;

    sal_Bool _SelectNextRubyChars( SwPaM& rPam, SwRubyListEntry& rRubyEntry,
                                sal_uInt16 nMode );

    // unser eigener 'IdlTimer' ruft folgende Methode
    DECL_LINK( DoIdleJobs, Timer * );
    // der CharTimer ruft diese Methode
    DECL_LINK( DoUpdateAllCharts, Timer * );
    DECL_LINK( DoUpdateModifiedOLE, Timer * );

     SwFmt *_MakeCharFmt(const String &, SwFmt *, sal_Bool, sal_Bool );
     SwFmt *_MakeFrmFmt(const String &, SwFmt *, sal_Bool, sal_Bool );
     SwFmt *_MakeTxtFmtColl(const String &, SwFmt *, sal_Bool, sal_Bool );

     void InitTOXTypes();
     void   Paste( const SwDoc& );
     bool DeleteAndJoinImpl(SwPaM&, const bool);
     bool DeleteAndJoinWithRedlineImpl(SwPaM&, const bool unused = false);
     bool DeleteRangeImpl(SwPaM&, const bool unused = false);
     bool DeleteRangeImplImpl(SwPaM &);
     bool ReplaceRangeImpl(SwPaM&, String const&, const bool);

public:

    /** Life cycle
    */
    SwDoc();
    ~SwDoc();

    inline bool IsInDtor() const { return mbDtor; }

    /* @@@MAINTAINABILITY-HORROR@@@
       Implementation details made public.
    */
    SwNodes      & GetNodes()       { return *m_pNodes; }
    SwNodes const& GetNodes() const { return *m_pNodes; }

    /** IInterface
    */
    virtual sal_Int32 acquire();
    virtual sal_Int32 release();
    virtual sal_Int32 getReferenceCount() const;

    /** IDocumentSettingAccess
    */
    virtual bool get(/*[in]*/ DocumentSettingId id) const;
    virtual void set(/*[in]*/ DocumentSettingId id, /*[in]*/ bool value);
    virtual const com::sun::star::i18n::ForbiddenCharacters* getForbiddenCharacters(/*[in]*/ sal_uInt16 nLang, /*[in]*/ bool bLocaleData ) const;
    virtual void setForbiddenCharacters(/*[in]*/ sal_uInt16 nLang, /*[in]*/ const com::sun::star::i18n::ForbiddenCharacters& rForbiddenCharacters );
    virtual vos::ORef<SvxForbiddenCharactersTable>& getForbiddenCharacterTable();
    virtual const vos::ORef<SvxForbiddenCharactersTable>& getForbiddenCharacterTable() const;
    virtual sal_uInt16 getLinkUpdateMode( /*[in]*/bool bGlobalSettings ) const;
    virtual void setLinkUpdateMode( /*[in]*/ sal_uInt16 nMode );
    virtual SwFldUpdateFlags getFieldUpdateFlags( /*[in]*/bool bGlobalSettings ) const;
    virtual void setFieldUpdateFlags( /*[in]*/ SwFldUpdateFlags eMode );
    virtual SwCharCompressType getCharacterCompressionType() const;
    virtual void setCharacterCompressionType( /*[in]*/SwCharCompressType nType );

    /** IDocumentDeviceAccess
    */
    virtual SfxPrinter* getPrinter(/*[in]*/ bool bCreate ) const;
    virtual void setPrinter(/*[in]*/ SfxPrinter* pP,/*[in]*/ bool bDeleteOld,/*[in]*/ bool bCallPrtDataChanged );
    virtual VirtualDevice* getVirtualDevice(/*[in]*/ bool bCreate ) const;
    virtual void setVirtualDevice(/*[in]*/ VirtualDevice* pVd,/*[in]*/ bool bDeleteOld, /*[in]*/ bool bCallVirDevDataChanged );
    virtual OutputDevice* getReferenceDevice(/*[in]*/ bool bCreate ) const;
    virtual void setReferenceDeviceType(/*[in]*/ bool bNewVirtual,/*[in]*/ bool bNewHiRes );
    virtual const JobSetup* getJobsetup() const;
    virtual void setJobsetup(/*[in]*/ const JobSetup& rJobSetup );
    virtual const SwPrintData & getPrintData() const;
    virtual void setPrintData(/*[in]*/ const SwPrintData& rPrtData);

    /** IDocumentMarkAccess
    */
    IDocumentMarkAccess* getIDocumentMarkAccess();
    const IDocumentMarkAccess* getIDocumentMarkAccess() const;

    /** IDocumentRedlineAccess
    */
    virtual RedlineMode_t GetRedlineMode() const;
    virtual void SetRedlineMode_intern(/*[in]*/RedlineMode_t eMode);
    virtual void SetRedlineMode(/*[in]*/RedlineMode_t eMode);
    virtual bool IsRedlineOn() const;
    virtual bool IsIgnoreRedline() const;
    virtual bool IsInRedlines(const SwNode& rNode) const;
    virtual const SwRedlineTbl& GetRedlineTbl() const;
    virtual bool AppendRedline(/*[in]*/SwRedline* pPtr, /*[in]*/bool bCallDelete);
    virtual bool SplitRedline(const SwPaM& rPam);
    virtual bool DeleteRedline(/*[in]*/const SwPaM& rPam, /*[in]*/bool bSaveInUndo, /*[in]*/sal_uInt16 nDelType);
    virtual bool DeleteRedline(/*[in]*/const SwStartNode& rSection, /*[in]*/bool bSaveInUndo, /*[in]*/sal_uInt16 nDelType);
    virtual sal_uInt16 GetRedlinePos(/*[in]*/const SwNode& rNode, /*[in]*/sal_uInt16 nType) const;
    virtual void CompressRedlines();
    virtual const SwRedline* GetRedline(/*[in]*/const SwPosition& rPos, /*[in]*/sal_uInt16* pFndPos) const;
    virtual bool IsRedlineMove() const;
    virtual void SetRedlineMove(/*[in]*/bool bFlag);
    virtual bool AcceptRedline(/*[in]*/sal_uInt16 nPos, /*[in]*/bool bCallDelete);
    virtual bool AcceptRedline(/*[in]*/const SwPaM& rPam, /*[in]*/bool bCallDelete);
    virtual bool RejectRedline(/*[in]*/sal_uInt16 nPos, /*[in]*/bool bCallDelete);
    virtual bool RejectRedline(/*[in]*/const SwPaM& rPam, /*[in]*/bool bCallDelete);
    virtual const SwRedline* SelNextRedline(/*[in]*/SwPaM& rPam) const;
    virtual const SwRedline* SelPrevRedline(/*[in]*/SwPaM& rPam) const;
    virtual void UpdateRedlineAttr();
    virtual sal_uInt16 GetRedlineAuthor();
    virtual sal_uInt16 InsertRedlineAuthor(const String& rAuthor);
    virtual bool SetRedlineComment(/*[in]*/const SwPaM& rPam, /*[in]*/const String& rComment);
    virtual const ::com::sun::star::uno::Sequence <sal_Int8>& GetRedlinePassword() const;
    virtual void SetRedlinePassword(/*[in]*/const ::com::sun::star::uno::Sequence <sal_Int8>& rNewPassword);

    /** IDocumentUndoRedo
    */
    IDocumentUndoRedo      & GetIDocumentUndoRedo();
    IDocumentUndoRedo const& GetIDocumentUndoRedo() const;

    /** IDocumentLinksAdministration
    */
    virtual bool IsVisibleLinks() const;
    virtual void SetVisibleLinks(bool bFlag);
    virtual sfx2::LinkManager& GetLinkManager();
    virtual const sfx2::LinkManager& GetLinkManager() const;
    virtual void UpdateLinks(sal_Bool bUI);
    virtual bool GetData(const String& rItem, const String& rMimeType, ::com::sun::star::uno::Any& rValue) const;
    virtual bool SetData(const String& rItem, const String& rMimeType, const ::com::sun::star::uno::Any& rValue);
    virtual ::sfx2::SvLinkSource* CreateLinkSource(const String& rItem);
    virtual bool EmbedAllLinks();
    virtual void SetLinksUpdated(const bool bNewLinksUpdated);
    virtual bool LinksUpdated() const;

    /** IDocumentFieldsAccess
    */
    virtual const SwFldTypes *GetFldTypes() const;
    virtual SwFieldType *InsertFldType(const SwFieldType &);
    virtual SwFieldType *GetSysFldType( const sal_uInt16 eWhich ) const;
    virtual SwFieldType* GetFldType(sal_uInt16 nResId, const String& rName, bool bDbFieldMatching) const;
    virtual void RemoveFldType(sal_uInt16 nFld);
    virtual void UpdateFlds( SfxPoolItem* pNewHt, bool bCloseDB);
    virtual void InsDeletedFldType(SwFieldType &);
    virtual bool PutValueToField(const SwPosition & rPos, const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich);
    virtual bool UpdateFld(SwTxtFld * rDstFmtFld, SwField & rSrcFld, SwMsgPoolItem * pMsgHnt, bool bUpdateTblFlds);
    virtual void UpdateRefFlds(SfxPoolItem* pHt);
    virtual void UpdateTblFlds(SfxPoolItem* pHt);
    virtual void UpdateExpFlds(SwTxtFld* pFld, bool bUpdateRefFlds);
    virtual void UpdateUsrFlds();
    virtual void UpdatePageFlds(SfxPoolItem*);
    virtual void LockExpFlds();
    virtual void UnlockExpFlds();
    virtual bool IsExpFldsLocked() const;
    virtual SwDocUpdtFld& GetUpdtFlds() const;
    virtual bool SetFieldsDirty(bool b, const SwNode* pChk, sal_uLong nLen);
    virtual void SetFixFields(bool bOnlyTimeDate, const DateTime* pNewDateTime);
    virtual void FldsToCalc(SwCalc& rCalc, sal_uLong nLastNd, sal_uInt16 nLastCnt);
    virtual void FldsToCalc(SwCalc& rCalc, const _SetGetExpFld& rToThisFld);
    virtual void FldsToExpand(SwHash**& ppTbl, sal_uInt16& rTblSize, const _SetGetExpFld& rToThisFld);
    virtual bool IsNewFldLst() const;
    virtual void SetNewFldLst( bool bFlag);
    virtual void InsDelFldInFldLst(bool bIns, const SwTxtFld& rFld);

    /** Returns the field at a certain position.
       @param rPos position to search at
       @return pointer to field at the given position or NULL in case no field is found
    */
    static SwField* GetField(const SwPosition& rPos);

    /** Returns the field at a certain position.
       @param rPos position to search at
       @return pointer to field at the given position or NULL in case no field is found
    */
    static SwTxtFld* GetTxtFld(const SwPosition& rPos);

    /** IDocumentContentOperations
    */
    virtual bool CopyRange(SwPaM&, SwPosition&, const bool bCopyAll) const;
    virtual void DeleteSection(SwNode* pNode);
    virtual bool DeleteRange(SwPaM&);
    virtual bool DelFullPara(SwPaM&);
    // --> OD 2009-08-20 #i100466#
    // Add optional parameter <bForceJoinNext>, default value <false>
    // Needed for hiding of deletion redlines
    virtual bool DeleteAndJoin( SwPaM&,
                                const bool bForceJoinNext = false );
    // <--
    virtual bool MoveRange(SwPaM&, SwPosition&, SwMoveFlags);
    virtual bool MoveNodeRange(SwNodeRange&, SwNodeIndex&, SwMoveFlags);
    virtual bool MoveAndJoin(SwPaM&, SwPosition&, SwMoveFlags);
    virtual bool Overwrite(const SwPaM &rRg, const String& rStr);
    virtual bool InsertString(const SwPaM &rRg, const String&,
              const enum InsertFlags nInsertMode = INS_EMPTYEXPAND );
    virtual SwFlyFrmFmt* Insert(const SwPaM &rRg, const String& rGrfName, const String& rFltName, const Graphic* pGraphic,
                        const SfxItemSet* pFlyAttrSet, const SfxItemSet* pGrfAttrSet, SwFrmFmt*);
    virtual SwFlyFrmFmt* Insert(const SwPaM& rRg, const GraphicObject& rGrfObj, const SfxItemSet* pFlyAttrSet,
                        const SfxItemSet* pGrfAttrSet, SwFrmFmt*);
    virtual SwDrawFrmFmt* Insert(const SwPaM &rRg, SdrObject& rDrawObj, const SfxItemSet* pFlyAttrSet, SwFrmFmt*);
    virtual SwFlyFrmFmt* Insert(const SwPaM &rRg, const svt::EmbeddedObjectRef& xObj, const SfxItemSet* pFlyAttrSet,
                        const SfxItemSet* pGrfAttrSet, SwFrmFmt*);
    virtual bool InsertPoolItem(const SwPaM &rRg, const SfxPoolItem&,
                                const SetAttrMode nFlags);
    virtual bool InsertItemSet (const SwPaM &rRg, const SfxItemSet&,
                                const SetAttrMode nFlags);
    virtual void ReRead(SwPaM&, const String& rGrfName, const String& rFltName, const Graphic* pGraphic, const GraphicObject* pGrfObj);
    virtual void TransliterateText(const SwPaM& rPaM, utl::TransliterationWrapper&);
    virtual SwFlyFrmFmt* InsertOLE(const SwPaM &rRg, const String& rObjName, sal_Int64 nAspect, const SfxItemSet* pFlyAttrSet,
                           const SfxItemSet* pGrfAttrSet, SwFrmFmt*);
    virtual bool SplitNode(const SwPosition &rPos, bool bChkTableStart);
    virtual bool AppendTxtNode(SwPosition& rPos);
        virtual void SetModified(SwPaM &rPaM);
    virtual bool ReplaceRange(SwPaM& rPam, const String& rNewStr,
                              const bool bRegExReplace);
    virtual void RemoveLeadingWhiteSpace(const SwPosition & rPos );

    /** IDocumentStylePoolAccess
    */
    virtual SwTxtFmtColl* GetTxtCollFromPool( sal_uInt16 nId, bool bRegardLanguage = true );
    virtual SwFmt* GetFmtFromPool( sal_uInt16 nId );
    virtual SwFrmFmt* GetFrmFmtFromPool( sal_uInt16 nId );
    virtual SwCharFmt* GetCharFmtFromPool( sal_uInt16 nId );
    virtual SwPageDesc* GetPageDescFromPool( sal_uInt16 nId, bool bRegardLanguage = true );
    virtual SwNumRule* GetNumRuleFromPool( sal_uInt16 nId );
    virtual bool IsPoolTxtCollUsed( sal_uInt16 nId ) const;
    virtual bool IsPoolFmtUsed( sal_uInt16 nId ) const;
    virtual bool IsPoolPageDescUsed( sal_uInt16 nId ) const;

    /** IDocumentLineNumberAccess
    */
    virtual const SwLineNumberInfo& GetLineNumberInfo() const;
    virtual void SetLineNumberInfo(const SwLineNumberInfo& rInfo);

    /** IDocumentStatistics
    */
    virtual void DocInfoChgd();
    virtual const SwDocStat &GetDocStat() const;
    virtual void SetDocStat(const SwDocStat& rStat);
    virtual void UpdateDocStat(SwDocStat& rStat);

    /** IDocumentState
    */
    virtual void SetModified();
    virtual void ResetModified();
    virtual bool IsModified() const;
    virtual bool IsLoaded() const;
    virtual bool IsUpdateExpFld() const;
    virtual bool IsNewDoc() const;
    virtual bool IsPageNums() const;
    virtual void SetPageNums(bool b);
    virtual void SetNewDoc(bool b);
    virtual void SetUpdateExpFldStat(bool b);
    virtual void SetLoaded(bool b);

    /** IDocumentDrawModelAccess
    */
    virtual const SdrModel* GetDrawModel() const;
    virtual SdrModel* GetDrawModel();
    virtual SdrLayerID GetHeavenId() const;
    virtual SdrLayerID GetHellId() const;
    virtual SdrLayerID GetControlsId() const;
    virtual SdrLayerID GetInvisibleHeavenId() const;
    virtual SdrLayerID GetInvisibleHellId() const;
    virtual SdrLayerID GetInvisibleControlsId() const;
    virtual void NotifyInvisibleLayers( SdrPageView& _rSdrPageView );
    virtual bool IsVisibleLayerId( const SdrLayerID& _nLayerId ) const;
    virtual SdrLayerID GetVisibleLayerIdByInvisibleOne( const SdrLayerID& _nInvisibleLayerId );
    virtual SdrLayerID GetInvisibleLayerIdByVisibleOne( const SdrLayerID& _nVisibleLayerId );
    virtual SdrModel* _MakeDrawModel();
    virtual SdrModel* GetOrCreateDrawModel();

    /** IDocumentLayoutAccess
    */
    virtual void SetCurrentViewShell( ViewShell* pNew );//swmod 071225
    virtual SwLayouter* GetLayouter();
    virtual const SwLayouter* GetLayouter() const;
    virtual void SetLayouter( SwLayouter* pNew );
    virtual SwFrmFmt* MakeLayoutFmt( RndStdIds eRequest, const SfxItemSet* pSet );
    virtual void DelLayoutFmt( SwFrmFmt *pFmt );
    virtual SwFrmFmt* CopyLayoutFmt( const SwFrmFmt& rSrc, const SwFmtAnchor& rNewAnchor, bool bSetTxtFlyAtt, bool bMakeFrms );
    virtual const ViewShell *GetCurrentViewShell() const;   //swmod 080219
    virtual ViewShell *GetCurrentViewShell();//swmod 080219 It must be able to communicate to a ViewShell.This is going to be removerd later.
    virtual const SwRootFrm *GetCurrentLayout() const;
    virtual SwRootFrm *GetCurrentLayout();//swmod 080219
    virtual bool HasLayout() const;

    /** IDocumentTimerAccess
    */
    virtual void StartIdling();
    virtual void StopIdling();
    virtual void BlockIdling();
    virtual void UnblockIdling();

    /** IDocumentChartDataProviderAccess
    */
    virtual SwChartDataProvider * GetChartDataProvider( bool bCreate = false ) const;
    virtual void CreateChartInternalDataProviders( const SwTable *pTable );
    virtual SwChartLockController_Helper & GetChartControllerHelper();

    /** IDocumentListItems

        OD 2007-10-26 #i83479#
    */
    virtual void addListItem( const SwNodeNum& rNodeNum );
    virtual void removeListItem( const SwNodeNum& rNodeNum );
    virtual String getListItemText( const SwNodeNum& rNodeNum,
                                    const bool bWithNumber = true,
                                    const bool bWithSpacesForLevel = false ) const;
    virtual void getListItems( IDocumentListItems::tSortedNodeNumList& orNodeNumList ) const;
    virtual void getNumItems( IDocumentListItems::tSortedNodeNumList& orNodeNumList ) const;

    /** IDocumentOutlineNodes

        OD 2007-11-15 #i83479#
    */
    virtual sal_Int32 getOutlineNodesCount() const;
    virtual int getOutlineLevel( const sal_Int32 nIdx ) const;
    virtual String getOutlineText( const sal_Int32 nIdx,
                                   const bool bWithNumber,
                                   const bool bWithSpacesForLevel ) const;
    virtual SwTxtNode* getOutlineNode( const sal_Int32 nIdx ) const;
    virtual void getOutlineNodes( IDocumentOutlineNodes::tSortedOutlineNodeList& orOutlineNodeList ) const;

    /** IDocumentListsAccess

        OD 2008-03-26 #refactorlists#
    */
    virtual SwList* createList( String sListId,
                                const String sDefaultListStyleName );
    virtual void deleteList( const String sListId );
    virtual SwList* getListByName( const String sListId ) const;
    virtual SwList* createListForListStyle( const String sListStyleName );
    virtual SwList* getListForListStyle( const String sListStyleName ) const;
    virtual void deleteListForListStyle( const String sListStyleName );
    // --> OD 2008-07-08 #i91400#
    virtual void trackChangeOfListStyleName( const String sListStyleName,
                                             const String sNewListStyleName );
    // <--

    /** IDocumentExternalData */
    virtual void setExternalData(::sw::tExternalDataType eType,
                                 ::sw::tExternalDataPointer pPayload);
    virtual ::sw::tExternalDataPointer getExternalData(::sw::tExternalDataType eType);


    /** INextInterface here
    */

    DECL_STATIC_LINK( SwDoc, BackgroundDone, SvxBrushItem *);
    DECL_LINK(CalcFieldValueHdl, EditFieldInfo*);

    /** OLE ???
    */
    bool IsOLEPrtNotifyPending() const  { return mbOLEPrtNotifyPending; }
    inline void SetOLEPrtNotifyPending( bool bSet = true );
    void PrtOLENotify( sal_Bool bAll ); //Alle oder nur Markierte

#ifdef DBG_UTIL
    bool InXMLExport() const            { return mbXMLExport; }
    void SetXMLExport( bool bFlag )     { mbXMLExport = bFlag; }
#endif

    // liefert zu allen fliegenden Rahmen die Position im Dokument.
    // Wird ein Pam-Pointer uebergeben, muessen die absatzgebundenen
    // FlyFrames von der ::com::sun::star::awt::Selection vollstaendig umschlossen sein
    // ( Start < Pos < End ) !!!
    // (wird fuer die Writer benoetigt)
    void GetAllFlyFmts( SwPosFlyFrms& rPosFlyFmts, const SwPaM* = 0,
                        sal_Bool bDrawAlso = sal_False ) const;

    // wegen swrtf.cxx und define private public, jetzt hier
    SwFlyFrmFmt  *MakeFlyFrmFmt (const String &rFmtName, SwFrmFmt *pDerivedFrom);
    SwDrawFrmFmt *MakeDrawFrmFmt(const String &rFmtName, SwFrmFmt *pDerivedFrom);

    // JP 08.05.98: fuer Flys muss jetzt diese Schnittstelle benutzt
    //              werden. pAnchorPos muss gesetzt sein, wenn keine
    //              Seitenbindung vorliegt UND der ::com::sun::star::chaos::Anchor nicht schon
    //              im FlySet/FrmFmt mit gueltiger CntntPos gesetzt ist
    /* #109161# new parameter bCalledFromShell

       sal_True: An existing adjust item at pAnchorPos is propagated to
       the content node of the new fly section. That propagation only
       takes place if there is no adjust item in the paragraph style
       for the new fly section.

       sal_False: no propagation
    */
    SwFlyFrmFmt* MakeFlySection( RndStdIds eAnchorType,
                                 const SwPosition* pAnchorPos,
                                 const SfxItemSet* pSet = 0,
                                 SwFrmFmt *pParent = 0,
                                 sal_Bool bCalledFromShell = sal_False );
    SwFlyFrmFmt* MakeFlyAndMove( const SwPaM& rPam, const SfxItemSet& rSet,
                                const SwSelBoxes* pSelBoxes = 0,
                                SwFrmFmt *pParent = 0 );

    void CopyWithFlyInFly( const SwNodeRange& rRg,
                            const xub_StrLen nEndContentIndex,
                            const SwNodeIndex& rInsPos,
                            sal_Bool bMakeNewFrms = sal_True,
                            sal_Bool bDelRedlines = sal_True,
                            sal_Bool bCopyFlyAtFly = sal_False ) const;

    sal_Bool SetFlyFrmAttr( SwFrmFmt& rFlyFmt, SfxItemSet& rSet );

    sal_Bool SetFrmFmtToFly( SwFrmFmt& rFlyFmt, SwFrmFmt& rNewFmt,
                        SfxItemSet* pSet = 0, sal_Bool bKeepOrient = sal_False );
    // --> OD 2009-07-20 #i73249#
    void SetFlyFrmTitle( SwFlyFrmFmt& rFlyFrmFmt,
                         const String& sNewTitle );
    void SetFlyFrmDescription( SwFlyFrmFmt& rFlyFrmFmt,
                               const String& sNewDescription );
    // <--

    /** Footnotes
    */
    // Fussnoten Informationen
    const SwFtnInfo& GetFtnInfo() const         { return *pFtnInfo; }
    void SetFtnInfo(const SwFtnInfo& rInfo);
    const SwEndNoteInfo& GetEndNoteInfo() const { return *pEndNoteInfo; }
    void SetEndNoteInfo(const SwEndNoteInfo& rInfo);
          SwFtnIdxs& GetFtnIdxs()       { return *pFtnIdxs; }
    const SwFtnIdxs& GetFtnIdxs() const { return *pFtnIdxs; }
    // change footnotes in area
    bool SetCurFtn( const SwPaM& rPam, const String& rNumStr,
                    sal_uInt16 nNumber, bool bIsEndNote );

    /** Operations on the content of the document e.g.
        spell-checking/hyphenating/word-counting
    */
    ::com::sun::star::uno::Any
            Spell( SwPaM&, ::com::sun::star::uno::Reference<
                            ::com::sun::star::linguistic2::XSpellChecker1 > &,
                   sal_uInt16* pPageCnt, sal_uInt16* pPageSt, bool bGrammarCheck,
                   SwConversionArgs *pConvArgs = 0 ) const;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XHyphenatedWord >
            Hyphenate( SwPaM *pPam, const Point &rCrsrPos,
                         sal_uInt16* pPageCnt, sal_uInt16* pPageSt );

    // count words in pam
    void CountWords( const SwPaM& rPaM, SwDocStat& rStat ) const;


    /** ???
    */
    // Textbaustein Dokument?
    void SetGlossDoc( bool bGlssDc = true ) { mbGlossDoc = bGlssDc; }
    bool IsInsOnlyTextGlossary() const      { return mbInsOnlyTxtGlssry; }

    // Abstakt fuellen
    void Summary( SwDoc* pExtDoc, sal_uInt8 nLevel, sal_uInt8 nPara, sal_Bool bImpress );

    void ChangeAuthorityData(const SwAuthEntry* pNewData);

    bool IsInCallModified() const      { return mbInCallModified; }
    sal_Bool IsInHeaderFooter( const SwNodeIndex& rIdx ) const;
    short GetTextDirection( const SwPosition& rPos,
                            const Point* pPt = 0 ) const;
    sal_Bool IsInVerticalText( const SwPosition& rPos,
                               const Point* pPt = 0 ) const;
    /** Database ???
    */

    /*  Datenbank &&  DB-Manager */
    void SetNewDBMgr( SwNewDBMgr* pNewMgr )     { pNewDBMgr = pNewMgr; }
    SwNewDBMgr* GetNewDBMgr() const             { return pNewDBMgr; }
    void ChangeDBFields( const SvStringsDtor& rOldNames,
                        const String& rNewName );
    void SetInitDBFields(sal_Bool b);
    // Von Feldern verwendete Datenbanken herausfinden
    void GetAllUsedDB( SvStringsDtor& rDBNameList,
                       const SvStringsDtor* pAllDBNames = 0 );

    void ChgDBData( const SwDBData& rNewData );
    SwDBData GetDBData();
    const SwDBData& GetDBDesc();
    const SwDBData& _GetDBDesc() const { return aDBData; }

    /** Some helper functions
    */
    String GetUniqueGrfName() const;
    String GetUniqueOLEName() const;
    String GetUniqueFrameName() const;

    std::set<SwRootFrm*> GetAllLayouts();//swmod 080225

    void SetFlyName( SwFlyFrmFmt& rFmt, const String& rName );
    const SwFlyFrmFmt* FindFlyByName( const String& rName, sal_Int8 nNdTyp = 0 ) const;

    void GetGrfNms( const SwFlyFrmFmt& rFmt, String* pGrfName,
                    String* pFltName ) const;

    // setze bei allen Flys ohne Namen einen gueltigen (Wird von den Readern
    // nach dem Einlesen gerufen )
    void SetAllUniqueFlyNames();

        //Zuruecksetzen der Attribute; es werden alle TxtHints und bei
        //vollstaendiger Selektion harte Formatierung (AUTO-Formate) entfernt
    // --> OD 2008-11-28 #i96644#
    // introduce new optional parameter <bSendDataChangedEvents> in order to
    // control, if the side effect "send data changed events" is triggered or not.
    void ResetAttrs( const SwPaM &rRg,
                     sal_Bool bTxtAttr = sal_True,
                     const SvUShortsSort* = 0,
                     const bool bSendDataChangedEvents = true );
    // <--
    void RstTxtAttrs(const SwPaM &rRg, sal_Bool bInclRefToxMark = sal_False );

        // Setze das Attribut im angegebenen Format. Ist Undo aktiv, wird
        // das alte in die Undo-History aufgenommen
    void SetAttr( const SfxPoolItem&, SwFmt& );
    void SetAttr( const SfxItemSet&, SwFmt& );

    // --> OD 2008-02-12 #newlistlevelattrs#
    // method to reset a certain attribute at the given format
    void ResetAttrAtFormat( const sal_uInt16 nWhichId,
                            SwFmt& rChangedFormat );
    // <--

        // Setze das Attribut als neues default Attribut in diesem Dokument.
        // Ist Undo aktiv, wird das alte in die Undo-History aufgenommen
    void SetDefault( const SfxPoolItem& );
    void SetDefault( const SfxItemSet& );

    // Erfrage das Default Attribut in diesem Dokument.
    const SfxPoolItem& GetDefault( sal_uInt16 nFmtHint ) const;
    // TextAttribute nicht mehr aufspannen lassen
    sal_Bool DontExpandFmt( const SwPosition& rPos, sal_Bool bFlag = sal_True );

    /* Formate */
    const SwFrmFmts* GetFrmFmts() const     { return pFrmFmtTbl; }
          SwFrmFmts* GetFrmFmts()           { return pFrmFmtTbl; }
    const SwCharFmts* GetCharFmts() const   { return pCharFmtTbl;}

    /* LayoutFormate (Rahmen, DrawObjecte), mal const mal nicht */
    const SwSpzFrmFmts* GetSpzFrmFmts() const   { return pSpzFrmFmtTbl; }
          SwSpzFrmFmts* GetSpzFrmFmts()         { return pSpzFrmFmtTbl; }

    const SwFrmFmt *GetDfltFrmFmt() const   { return pDfltFrmFmt; }
          SwFrmFmt *GetDfltFrmFmt()         { return pDfltFrmFmt; }
    const SwFrmFmt *GetEmptyPageFmt() const { return pEmptyPageFmt; }
          SwFrmFmt *GetEmptyPageFmt()       { return pEmptyPageFmt; }
    const SwFrmFmt *GetColumnContFmt() const{ return pColumnContFmt; }
          SwFrmFmt *GetColumnContFmt()      { return pColumnContFmt; }
    const SwCharFmt *GetDfltCharFmt() const { return pDfltCharFmt;}
          SwCharFmt *GetDfltCharFmt()       { return pDfltCharFmt;}

    // Returns the interface of the management of (auto)styles
    IStyleAccess& GetIStyleAccess() { return *pStyleAccess; }

    // Remove all language dependencies from all existing formats
    void RemoveAllFmtLanguageDependencies();

    SwFrmFmt  *MakeFrmFmt(const String &rFmtName, SwFrmFmt *pDerivedFrom,
                          sal_Bool bBroadcast = sal_False, sal_Bool bAuto = sal_True);
    void       DelFrmFmt( SwFrmFmt *pFmt, sal_Bool bBroadcast = sal_False );
    SwFrmFmt* FindFrmFmtByName( const String& rName ) const
        {   return (SwFrmFmt*)FindFmtByName( (SvPtrarr&)*pFrmFmtTbl, rName ); }

    // --> OD 2005-01-13 #i40550#
    SwCharFmt *MakeCharFmt(const String &rFmtName, SwCharFmt *pDerivedFrom,
                           sal_Bool bBroadcast = sal_False,
                           sal_Bool bAuto = sal_True );
    // <--
    void       DelCharFmt(sal_uInt16 nFmt, sal_Bool bBroadcast = sal_False);
    void       DelCharFmt(SwCharFmt* pFmt, sal_Bool bBroadcast = sal_False);
    SwCharFmt* FindCharFmtByName( const String& rName ) const
        {   return (SwCharFmt*)FindFmtByName( (SvPtrarr&)*pCharFmtTbl, rName ); }

    /* Formatcollections (Vorlagen) */
    // TXT
    const SwTxtFmtColl* GetDfltTxtFmtColl() const { return pDfltTxtFmtColl; }
    const SwTxtFmtColls *GetTxtFmtColls() const { return pTxtFmtCollTbl; }
    // --> OD 2005-01-13 #i40550#
    SwTxtFmtColl *MakeTxtFmtColl( const String &rFmtName,
                                  SwTxtFmtColl *pDerivedFrom,
                                  sal_Bool bBroadcast = sal_False,
                                  sal_Bool bAuto = sal_True );
    // <--
    SwConditionTxtFmtColl* MakeCondTxtFmtColl( const String &rFmtName,
                                               SwTxtFmtColl *pDerivedFrom,
                                               sal_Bool bBroadcast = sal_False);
    void DelTxtFmtColl(sal_uInt16 nFmt, sal_Bool bBroadcast = sal_False);
    void DelTxtFmtColl( SwTxtFmtColl* pColl, sal_Bool bBroadcast = sal_False );
    // --> OD 2007-11-06 #i62675#
    // Add 4th optional parameter <bResetListAttrs>.
    // 'side effect' of <SetTxtFmtColl> with <bReset = true> is that the hard
    // attributes of the affected text nodes are cleared, except the break
    // attribute, the page description attribute and the list style attribute.
    // The new parameter <bResetListAttrs> indicates, if the list attributes
    // (list style, restart at and restart with) are cleared as well in case
    // that <bReset = true> and the paragraph style has a list style attribute set.
    sal_Bool SetTxtFmtColl( const SwPaM &rRg, SwTxtFmtColl *pFmt,
                            bool bReset = true,
                            bool bResetListAttrs = false );
    // <--
    SwTxtFmtColl* FindTxtFmtCollByName( const String& rName ) const
        {   return (SwTxtFmtColl*)FindFmtByName( (SvPtrarr&)*pTxtFmtCollTbl, rName ); }

    void ChkCondColls();

        // GRF
    const SwGrfFmtColl* GetDfltGrfFmtColl() const   { return pDfltGrfFmtColl; }
    const SwGrfFmtColls *GetGrfFmtColls() const     { return pGrfFmtCollTbl; }
    SwGrfFmtColl *MakeGrfFmtColl(const String &rFmtName,
                                    SwGrfFmtColl *pDerivedFrom);
    SwGrfFmtColl* FindGrfFmtCollByName( const String& rName ) const
        {   return (SwGrfFmtColl*)FindFmtByName( (SvPtrarr&)*pGrfFmtCollTbl, rName ); }

        // Tabellen-Formate
    const SwFrmFmts* GetTblFrmFmts() const  { return pTblFrmFmtTbl; }
          SwFrmFmts* GetTblFrmFmts()        { return pTblFrmFmtTbl; }
    sal_uInt16 GetTblFrmFmtCount( sal_Bool bUsed ) const;
    SwFrmFmt& GetTblFrmFmt(sal_uInt16 nFmt, sal_Bool bUsed ) const;
    SwTableFmt* MakeTblFrmFmt(const String &rFmtName, SwFrmFmt *pDerivedFrom);
    void        DelTblFrmFmt( SwTableFmt* pFmt );
    SwTableFmt* FindTblFmtByName( const String& rName, sal_Bool bAll = sal_False ) const;

    //Rahmenzugriff
    //iterieren ueber Flys - fuer Basic-Collections
    sal_uInt16 GetFlyCount( FlyCntType eType = FLYCNTTYPE_ALL) const;
    SwFrmFmt* GetFlyNum(sal_uInt16 nIdx, FlyCntType eType = FLYCNTTYPE_ALL);


    // kopiere die Formate in die eigenen Arrays und returne diese
    SwFrmFmt  *CopyFrmFmt ( const SwFrmFmt& );
    SwCharFmt *CopyCharFmt( const SwCharFmt& );
    SwTxtFmtColl* CopyTxtColl( const SwTxtFmtColl& rColl );
    SwGrfFmtColl* CopyGrfColl( const SwGrfFmtColl& rColl );

        // ersetze alle Formate mit denen aus rSource
    void ReplaceStyles( SwDoc& rSource );

    // erfrage ob die Absatz-/Zeichen-/Rahmen-/Seiten - Vorlage benutzt wird
    sal_Bool IsUsed( const SwModify& ) const;
    sal_Bool IsUsed( const SwNumRule& ) const;

        // setze den Namen der neu geladenen Dokument-Vorlage
    sal_uInt16 SetDocPattern( const String& rPatternName );
        // gebe den Dok-VorlagenNamen zurueck. !!! Kann auch 0 sein !!!
    String* GetDocPattern( sal_uInt16 nPos ) const { return aPatternNms[nPos]; }

        // Loesche alle nicht referenzierten FeldTypen
    void GCFieldTypes();                // impl. in docfld.cxx

        // akt. Dokument mit Textbausteindokument verbinden/erfragen
    void SetGlossaryDoc( SwDoc* pDoc ) { pGlossaryDoc = pDoc; }

    // travel over PaM Ring
    sal_Bool InsertGlossary( SwTextBlocks& rBlock, const String& rEntry,
                        SwPaM& rPaM, SwCrsrShell* pShell = 0);

    // get the set of printable pages for the XRenderable API by
    // evaluating the respective settings (see implementation)
    void CalculatePagesForPrinting( const SwRootFrm& rLayout, SwRenderData &rData, const SwPrintUIOptions &rOptions, bool bIsPDFExport,
            sal_Int32 nDocPageCount );
    void UpdatePagesForPrintingWithPostItData( SwRenderData &rData, const SwPrintUIOptions &rOptions, bool bIsPDFExport,
            sal_Int32 nDocPageCount );
    void CalculatePagePairsForProspectPrinting( const SwRootFrm& rLayout, SwRenderData &rData, const SwPrintUIOptions &rOptions,
            sal_Int32 nDocPageCount );

        //PageDescriptor-Schnittstelle
    sal_uInt16 GetPageDescCnt() const { return aPageDescs.Count(); }
    const SwPageDesc& GetPageDesc( const sal_uInt16 i ) const { return *aPageDescs[i]; }
    SwPageDesc* FindPageDescByName( const String& rName,
                                    sal_uInt16* pPos = 0 ) const;

        // kopiere den gesamten PageDesc - ueber Dokumentgrenzen und "tief"!
        // optional kann das kopieren der PoolFmtId, -HlpId verhindert werden
    void CopyPageDesc( const SwPageDesc& rSrcDesc, SwPageDesc& rDstDesc,
                        sal_Bool bCopyPoolIds = sal_True );

        // kopiere die Kopzeile (mit dem Inhalt!) aus dem SrcFmt
        // ins DestFmt ( auch ueber Doc grenzen hinaus!)
    void CopyHeader( const SwFrmFmt& rSrcFmt, SwFrmFmt& rDestFmt )
        { CopyPageDescHeaderFooterImpl( true, rSrcFmt, rDestFmt ); }
        // kopiere die Fusszeile (mit dem Inhalt!) aus dem SrcFmt
        // ins DestFmt ( auch ueber Doc grenzen hinaus!)
    void CopyFooter( const SwFrmFmt& rSrcFmt, SwFrmFmt& rDestFmt )
        { CopyPageDescHeaderFooterImpl( false, rSrcFmt, rDestFmt ); }

        //fuer Reader

    SwPageDesc * GetPageDesc( const String & rName );
    SwPageDesc& _GetPageDesc( sal_uInt16 i ) const { return *aPageDescs[i]; }
    void ChgPageDesc( const String & rName, const SwPageDesc& );
    void ChgPageDesc( sal_uInt16 i, const SwPageDesc& );
    sal_Bool FindPageDesc( const String & rName, sal_uInt16 * pFound );
    // -> #116530#
    void DelPageDesc( const String & rName, sal_Bool bBroadcast = sal_False);
    void DelPageDesc( sal_uInt16 i, sal_Bool bBroadcast = sal_False );
    // <- #116530#
    void PreDelPageDesc(SwPageDesc * pDel); // #i7983#
    // -> #116530#
    sal_uInt16 MakePageDesc( const String &rName, const SwPageDesc* pCpy = 0,
                             sal_Bool bRegardLanguage = sal_True,
                             sal_Bool bBroadcast = sal_False);
    void BroadcastStyleOperation(String rName, SfxStyleFamily eFamily,
                                 sal_uInt16 nOp);
    // <- #116530#


    // --> FME 2005-03-16 #i44963# The html import sometimes overwrites the
    // page sizes set in the page descriptions. This function is used to
    // correct this.
    void CheckDefaultPageFmt();
    // <--

        // Methoden fuer die Verzeichnisse:
        // - Verzeichnismarke einfuegen loeschen travel
    sal_uInt16 GetCurTOXMark( const SwPosition& rPos, SwTOXMarks& ) const;
    void DeleteTOXMark( const SwTOXMark* pTOXMark );
    const SwTOXMark& GotoTOXMark( const SwTOXMark& rCurTOXMark,
                                SwTOXSearch eDir, sal_Bool bInReadOnly );

        // - Verzeichnis einfuegen, und bei Bedarf erneuern
    const SwTOXBaseSection* InsertTableOf( const SwPosition& rPos,
                                            const SwTOXBase& rTOX,
                                            const SfxItemSet* pSet = 0,
                                            sal_Bool bExpand = sal_False );
    const SwTOXBaseSection* InsertTableOf( sal_uLong nSttNd, sal_uLong nEndNd,
                                            const SwTOXBase& rTOX,
                                            const SfxItemSet* pSet = 0                                          );
    const SwTOXBase* GetCurTOX( const SwPosition& rPos ) const;
    const SwAttrSet& GetTOXBaseAttrSet(const SwTOXBase& rTOX) const;

    sal_Bool DeleteTOX( const SwTOXBase& rTOXBase, sal_Bool bDelNodes = sal_False );
    String GetUniqueTOXBaseName( const SwTOXType& rType,
                                const String* pChkStr = 0 ) const;

    sal_Bool SetTOXBaseName(const SwTOXBase& rTOXBase, const String& rName);
    void SetTOXBaseProtection(const SwTOXBase& rTOXBase, sal_Bool bProtect);

    // nach einlesen einer Datei alle Verzeichnisse updaten
    void SetUpdateTOX( bool bFlag = true )     { mbUpdateTOX = bFlag; }
    bool IsUpdateTOX() const                   { return mbUpdateTOX; }

    const String&   GetTOIAutoMarkURL() const {return sTOIAutoMarkURL;}
    void            SetTOIAutoMarkURL(const String& rSet)  {sTOIAutoMarkURL = rSet;}
    void            ApplyAutoMark();

    bool IsInReading() const                    { return mbInReading; }
    void SetInReading( bool bNew )              { mbInReading = bNew; }

    bool IsClipBoard() const                    { return mbClipBoard; }
    // N.B.: must be called right after constructor! (@see GetXmlIdRegistry)
    void SetClipBoard( bool bNew )              { mbClipBoard = bNew; }

    bool IsColumnSelection() const              { return mbColumnSelection; }
    void SetColumnSelection( bool bNew )        { mbColumnSelection = bNew; }

    bool IsInXMLImport() const { return mbInXMLImport; }
    void SetInXMLImport( bool bNew ) { mbInXMLImport = bNew; }

    // - Verzeichnis-Typen verwalten
    sal_uInt16 GetTOXTypeCount( TOXTypes eTyp ) const;
    const SwTOXType* GetTOXType( TOXTypes eTyp, sal_uInt16 nId ) const;
    sal_Bool DeleteTOXType( TOXTypes eTyp, sal_uInt16 nId );
    const SwTOXType* InsertTOXType( const SwTOXType& rTyp );
    const SwTOXTypes& GetTOXTypes() const { return *pTOXTypes; }

    const SwTOXBase*    GetDefaultTOXBase( TOXTypes eTyp, sal_Bool bCreate = sal_False );
    void                SetDefaultTOXBase(const SwTOXBase& rBase);

    // - Schluessel fuer die Indexverwaltung
    sal_uInt16 GetTOIKeys( SwTOIKeyType eTyp, SvStringsSort& rArr ) const;

    // Sortieren Tabellen Text
    sal_Bool SortTbl(const SwSelBoxes& rBoxes, const SwSortOptions&);
    sal_Bool SortText(const SwPaM&, const SwSortOptions&);

        // korrigiere die im Dokument angemeldeten SwPosition-Objecte,
        // wie z.B. die ::com::sun::star::text::Bookmarks oder die Verzeichnisse.
        // JP 22.06.95: ist bMoveCrsr gesetzt, verschiebe auch die Crsr

        // Setzt alles in rOldNode auf rNewPos + Offset
    void CorrAbs( const SwNodeIndex& rOldNode, const SwPosition& rNewPos,
                    const xub_StrLen nOffset = 0, sal_Bool bMoveCrsr = sal_False );
        // Setzt alles im Bereich von [rStartNode, rEndNode] nach rNewPos
    void CorrAbs( const SwNodeIndex& rStartNode, const SwNodeIndex& rEndNode,
                    const SwPosition& rNewPos, sal_Bool bMoveCrsr = sal_False );
        // Setzt alles im Bereich von rRange nach rNewPos
    void CorrAbs( const SwPaM& rRange, const SwPosition& rNewPos,
                    sal_Bool bMoveCrsr = sal_False );
        // Setzt alles in rOldNode auf relative Pos
    void CorrRel( const SwNodeIndex& rOldNode, const SwPosition& rNewPos,
                    const xub_StrLen nOffset = 0, sal_Bool bMoveCrsr = sal_False );

        // GliederungsRegeln erfragen / setzen
    // --> OD 2005-11-02 #i51089 - TUNING#
    inline SwNumRule* GetOutlineNumRule() const
    {
        return pOutlineRule;
    }
    // <--
    void SetOutlineNumRule( const SwNumRule& rRule );
    void PropagateOutlineRule();

    // Gliederung - hoch-/runterstufen
    sal_Bool OutlineUpDown( const SwPaM& rPam, short nOffset = 1 );
    // Gliederung - hoch-/runtermoven
    sal_Bool MoveOutlinePara( const SwPaM& rPam, short nOffset = 1);
        // zu diesem Gliederungspunkt
    sal_Bool GotoOutline( SwPosition& rPos, const String& rName ) const;
    // die Aenderungen an den Gliederungsvorlagen in die OutlineRule uebernehmen

        // setzt, wenn noch keine Numerierung, sonst wird geaendert
        // arbeitet mit alten und neuen Regeln, nur Differenzen aktualisieren
    // --> OD 2005-02-18 #i42921# - re-use unused 3rd parameter
    // --> OD 2008-02-08 #newlistlevelattrs#
    // Add optional parameter <bResetIndentAttrs> - default value sal_False.
    // If <bResetIndentAttrs> equals true, the indent attributes "before text"
    // and "first line indent" are additionally reset at the provided PaM, if
    // the list style makes use of the new list level attributes.
    // --> OD 2008-03-17 #refactorlists#
    // introduce parameters <bCreateNewList> and <sContinuedListId>
    // <bCreateNewList> indicates, if a new list is created by applying the
    // given list style.
    void SetNumRule( const SwPaM&,
                     const SwNumRule&,
                     const bool bCreateNewList,
                     const String sContinuedListId = String(),
                     sal_Bool bSetItem = sal_True,
                     const bool bResetIndentAttrs = false );
    // <--
    void SetCounted( const SwPaM&, bool bCounted);

    // --> OD 2009-08-25 #i86492#
    // no longer needed.
    // SwDoc::SetNumRule( rPaM, rNumRule, false, <ListId>, sal_True, true ) have to be used instead.
//    /**
//       Replace numbering rules in a PaM by another numbering rule.

//       \param rPaM         PaM to replace the numbering rules in
//       \param rNumRule     numbering rule to replace the present numbering rules
//     */
//    void ReplaceNumRule(const SwPaM & rPaM, const SwNumRule & rNumRule);

    void MakeUniqueNumRules(const SwPaM & rPaM);

    void SetNumRuleStart( const SwPosition& rPos, sal_Bool bFlag = sal_True );
    void SetNodeNumStart( const SwPosition& rPos, sal_uInt16 nStt );

    SwNumRule* GetCurrNumRule( const SwPosition& rPos ) const;

    const SwNumRuleTbl& GetNumRuleTbl() const { return *pNumRuleTbl; }

    // #i36749#
    /**
       Add numbering rule to document.

       @param pRule    rule to add
    */
    void AddNumRule(SwNumRule * pRule);

    // --> OD 2008-02-11 #newlistlevelattrs#
    // add optional parameter <eDefaultNumberFormatPositionAndSpaceMode>
    sal_uInt16 MakeNumRule( const String &rName,
        const SwNumRule* pCpy = 0,
        sal_Bool bBroadcast = sal_False,
        const SvxNumberFormat::SvxNumPositionAndSpaceMode eDefaultNumberFormatPositionAndSpaceMode =
            SvxNumberFormat::LABEL_WIDTH_AND_POSITION );
    // <--
    sal_uInt16 FindNumRule( const String& rName ) const;
    SwNumRule* FindNumRulePtr( const String& rName ) const;

    // loeschen geht nur, wenn die ::com::sun::star::chaos::Rule niemand benutzt!
    // #106897#
    sal_Bool RenameNumRule(const String & aOldName, const String & aNewName,
                           sal_Bool bBroadcast = sal_False);
    sal_Bool DelNumRule( const String& rName, sal_Bool bBroadCast = sal_False );
    String GetUniqueNumRuleName( const String* pChkStr = 0, sal_Bool bAutoNum = sal_True ) const;

    void UpdateNumRule();   // alle invaliden Updaten
    // #106897#
    void ChgNumRuleFmts( const SwNumRule& rRule, const String * pOldName = 0 );
    sal_Bool ReplaceNumRule( const SwPosition& rPos, const String& rOldRule,
                        const String& rNewRule );

        // zum naechsten/vorhergehenden Punkt auf gleicher Ebene
    sal_Bool GotoNextNum( SwPosition&, sal_Bool bOverUpper = sal_True,
                        sal_uInt8* pUpper = 0, sal_uInt8* pLower = 0 );
    sal_Bool GotoPrevNum( SwPosition&, sal_Bool bOverUpper = sal_True,
                        sal_uInt8* pUpper = 0, sal_uInt8* pLower = 0 );

    // #i23731#
    /** Searches for a text node with a numbering rule.

       OD 2005-10-24 #i55391# - add optional parameter <bInvestigateStartNode>
       OD 2008-03-18 #refactorlists# - add output parameter <sListId>

       \param rPos         position to start search
       \param bForward     - sal_True:  search forward
                           - sal_False: search backward
       \param bNum         - sal_True:  search for enumeration
                           - sal_False: search for itemize
       \param bOutline     - sal_True:  search for outline numbering rule
                           - sal_False: search for non-outline numbering rule
       \param nNonEmptyAllowed   number of non-empty paragraphs allowed between
                                 rPos and found paragraph

        @param sListId
        output parameter - in case a list style is found, <sListId> holds the
        list id, to which the text node belongs, which applies the found list style.

        @param bInvestigateStartNode
        input parameter - boolean, indicating, if start node, determined by given
        start position has to be investigated or not.
     */
    const SwNumRule * SearchNumRule(const SwPosition & rPos,
                                    const bool bForward,
                                    const bool bNum,
                                    const bool bOutline,
                                    int nNonEmptyAllowed,
                                    String& sListId,
                                    const bool bInvestigateStartNode = false );

        // Absaetze ohne Numerierung, aber mit Einzuegen
    sal_Bool NoNum( const SwPaM& );
        // Loeschen, Splitten der Aufzaehlungsliste
    void DelNumRules( const SwPaM& );

    // Invalidates all numrules
    void InvalidateNumRules();

        // Hoch-/Runterstufen
    sal_Bool NumUpDown( const SwPaM&, sal_Bool bDown = sal_True );
        // Bewegt selektierte Absaetze (nicht nur Numerierungen)
        // entsprechend des Offsets. (negativ: zum Doc-Anf.)
    sal_Bool MoveParagraph( const SwPaM&, long nOffset = 1, sal_Bool bIsOutlMv = sal_False );
        // No-/Numerierung ueber Delete/Backspace ein/abschalten #115901#
    sal_Bool NumOrNoNum( const SwNodeIndex& rIdx, sal_Bool bDel = sal_False);
        // Animation der Grafiken stoppen
    void StopNumRuleAnimations( OutputDevice* );

        // fuege eine neue Tabelle auf der Position rPos ein. (es
        // wird vor dem Node eingefuegt !!)
        //JP 28.10.96:
        //  fuer AutoFormat bei der Eingabe: dann muessen die Spalten
        //  auf die vordefinierten Breite gesetzt werden. Im Array stehen die
        //  Positionen der Spalten!! (nicht deren Breite!)
    /* #109161# new parameter bCalledFromShell:

       sal_True: called from shell -> propagate existing adjust item at
       rPos to every new cell. A existing adjust item in the table
       heading or table contents paragraph style prevent that
       propagation.

       sal_False: do not propagate
    */
    const SwTable* InsertTable( const SwInsertTableOptions& rInsTblOpts,  // HEADLINE_NO_BORDER
                                const SwPosition& rPos, sal_uInt16 nRows,
                                sal_uInt16 nCols, short eAdjust,
                                const SwTableAutoFmt* pTAFmt = 0,
                                const SvUShorts* pColArr = 0,
                                sal_Bool bCalledFromShell = sal_False,
                                sal_Bool bNewModel = sal_True );

    // steht der Index in einer Tabelle, dann returne den TableNode sonst 0
                 SwTableNode* IsIdxInTbl( const SwNodeIndex& rIdx );
    inline const SwTableNode* IsIdxInTbl( const SwNodeIndex& rIdx ) const;

        // erzeuge aus dem makierten Bereich eine ausgeglichene Tabelle
    const SwTable* TextToTable( const SwInsertTableOptions& rInsTblOpts, // HEADLINE_NO_BORDER,
                                const SwPaM& rRange, sal_Unicode cCh,
                                short eAdjust,
                                const SwTableAutoFmt* = 0 );
    // text to table conversion - API support
    const SwTable* TextToTable( const std::vector< std::vector<SwNodeRange> >& rTableNodes );
        // erzeuge aus der Tabelle wieder normalen Text
    sal_Bool TableToText( const SwTableNode* pTblNd, sal_Unicode cCh );
        // einfuegen von Spalten/Zeilen in der Tabelle
    sal_Bool InsertCol( const SwCursor& rCursor,
                    sal_uInt16 nCnt = 1, sal_Bool bBehind = sal_True );
    sal_Bool InsertCol( const SwSelBoxes& rBoxes,
                    sal_uInt16 nCnt = 1, sal_Bool bBehind = sal_True );
    sal_Bool InsertRow( const SwCursor& rCursor,
                    sal_uInt16 nCnt = 1, sal_Bool bBehind = sal_True );
    sal_Bool InsertRow( const SwSelBoxes& rBoxes,
                    sal_uInt16 nCnt = 1, sal_Bool bBehind = sal_True );
        // loeschen von Spalten/Zeilen in der Tabelle
    sal_Bool DeleteRowCol( const SwSelBoxes& rBoxes, bool bColumn = false );
    sal_Bool DeleteRow( const SwCursor& rCursor );
    sal_Bool DeleteCol( const SwCursor& rCursor );
        // teilen / zusammenfassen von Boxen in der Tabelle
    sal_Bool SplitTbl( const SwSelBoxes& rBoxes, sal_Bool bVert = sal_True,
                       sal_uInt16 nCnt = 1, sal_Bool bSameHeight = sal_False );
        // returnt den enum TableMergeErr
    sal_uInt16 MergeTbl( SwPaM& rPam );
    String GetUniqueTblName() const;
    sal_Bool IsInsTblFormatNum() const;
    sal_Bool IsInsTblChangeNumFormat() const;
    sal_Bool IsInsTblAlignNum() const;

        // aus der FEShell wg.. Undo und bModified
    void GetTabCols( SwTabCols &rFill, const SwCursor* pCrsr,
                    const SwCellFrm* pBoxFrm = 0 ) const;
    void SetTabCols( const SwTabCols &rNew, sal_Bool bCurRowOnly,
                    const SwCursor* pCrsr, const SwCellFrm* pBoxFrm = 0 );
    void GetTabRows( SwTabCols &rFill, const SwCursor* pCrsr,
                    const SwCellFrm* pBoxFrm = 0 ) const;
    void SetTabRows( const SwTabCols &rNew, sal_Bool bCurColOnly, const SwCursor* pCrsr,
                     const SwCellFrm* pBoxFrm = 0 );


    // Direktzugriff fuer Uno
    void SetTabCols(SwTable& rTab, const SwTabCols &rNew, const SwTabCols &rOld,
                                    const SwTableBox *pStart, sal_Bool bCurRowOnly);

    void SetRowsToRepeat( SwTable &rTable, sal_uInt16 nSet );

        // AutoFormat fuer die Tabelle/TabellenSelection
    sal_Bool SetTableAutoFmt( const SwSelBoxes& rBoxes, const SwTableAutoFmt& rNew );
        // Erfrage wie attributiert ist
    sal_Bool GetTableAutoFmt( const SwSelBoxes& rBoxes, SwTableAutoFmt& rGet );
        // setze das InsertDB als Tabelle Undo auf:
    void AppendUndoForInsertFromDB( const SwPaM& rPam, sal_Bool bIsTable );
        // setze die Spalten/Zeilen/ZTellen Breite/Hoehe
    sal_Bool SetColRowWidthHeight( SwTableBox& rAktBox, sal_uInt16 eType,
                                SwTwips nAbsDiff, SwTwips nRelDiff );
    SwTableBoxFmt* MakeTableBoxFmt();
    SwTableLineFmt* MakeTableLineFmt();
    // teste ob die Box ein numerischen Wert darstellt und aender dann ggfs.
    // das Format der Box
    void ChkBoxNumFmt( SwTableBox& rAktBox, sal_Bool bCallUpdate );
    void SetTblBoxFormulaAttrs( SwTableBox& rBox, const SfxItemSet& rSet );
    void ClearBoxNumAttrs( const SwNodeIndex& rNode );

    sal_Bool InsCopyOfTbl( SwPosition& rInsPos, const SwSelBoxes& rBoxes,
                        const SwTable* pCpyTbl = 0, sal_Bool bCpyName = sal_False,
                        sal_Bool bCorrPos = sal_False );

    sal_Bool UnProtectCells( const String& rTblName );
    sal_Bool UnProtectCells( const SwSelBoxes& rBoxes );
    sal_Bool UnProtectTbls( const SwPaM& rPam );
    sal_Bool HasTblAnyProtection( const SwPosition* pPos,
                              const String* pTblName = 0,
                              sal_Bool* pFullTblProtection = 0 );

    // Tabelle an der Position in der GrundLine aufsplitten, sprich eine
    // neue Tabelle erzeugen.
    sal_Bool SplitTable( const SwPosition& rPos, sal_uInt16 eMode = 0,
                        sal_Bool bCalcNewSize = sal_False );
    // und die Umkehrung davon. rPos muss in der Tabelle stehen, die bestehen
    // bleibt. Das Flag besagt ob die aktuelle mit der davor oder dahinter
    // stehenden vereint wird.
    sal_Bool MergeTable( const SwPosition& rPos, sal_Bool bWithPrev = sal_True,
                        sal_uInt16 nMode = 0 );

    // Charts der angegebenen Tabelle zum Update bewegen
    void UpdateCharts( const String &rName ) const;
    // update all charts, for that exists any table
    void UpdateAllCharts()          { DoUpdateAllCharts( 0 ); }
    // Tabelle wird umbenannt und aktualisiert die Charts
    void SetTableName( SwFrmFmt& rTblFmt, const String &rNewName );

    // returne zum Namen die im Doc gesetzte Referenz
    const SwFmtRefMark* GetRefMark( const String& rName ) const;
    // returne die RefMark per Index - fuer Uno
    const SwFmtRefMark* GetRefMark( sal_uInt16 nIndex ) const;
    // returne die Namen aller im Doc gesetzten Referenzen.
    //  Ist der ArrayPointer 0 dann returne nur, ob im Doc. eine RefMark
    //  gesetzt ist
    sal_uInt16 GetRefMarks( SvStringsDtor* = 0 ) const;

    //Einfuegen einer Beschriftung - falls ein FlyFormat erzeugt wird, so
    // returne dieses.
    SwFlyFrmFmt* InsertLabel( const SwLabelType eType, const String &rTxt, const String& rSeparator,
                    const String& rNumberingSeparator,
                    const sal_Bool bBefore, const sal_uInt16 nId, const sal_uLong nIdx,
                    const String& rCharacterStyle,
                    const sal_Bool bCpyBrd = sal_True );
    SwFlyFrmFmt* InsertDrawLabel(
        const String &rTxt, const String& rSeparator, const String& rNumberSeparator,
        const sal_uInt16 nId, const String& rCharacterStyle, SdrObject& rObj );

    // erfrage den Attribut Pool
    const SwAttrPool& GetAttrPool() const   { return *mpAttrPool; }
          SwAttrPool& GetAttrPool()         { return *mpAttrPool; }

    // suche ueber das Layout eine EditShell und ggfs. eine ViewShell
    SwEditShell* GetEditShell( ViewShell** ppSh = 0 ) const;
    ::sw::IShellCursorSupplier * GetIShellCursorSupplier();

    // OLE 2.0-Benachrichtung
    inline       void  SetOle2Link(const Link& rLink) {aOle2Link = rLink;}
    inline const Link& GetOle2Link() const {return aOle2Link;}

    // insert section (the ODF kind of section, not the nodesarray kind)
    SwSection * InsertSwSection(SwPaM const& rRange, SwSectionData &,
            SwTOXBase const*const pTOXBase = 0,
            SfxItemSet const*const pAttr = 0, bool const bUpdate = true);
    sal_uInt16 IsInsRegionAvailable( const SwPaM& rRange,
                                const SwNode** ppSttNd = 0 ) const;
    SwSection* GetCurrSection( const SwPosition& rPos ) const;
    SwSectionFmts& GetSections() { return *pSectionFmtTbl; }
    const SwSectionFmts& GetSections() const { return *pSectionFmtTbl; }
    SwSectionFmt *MakeSectionFmt( SwSectionFmt *pDerivedFrom );
    void DelSectionFmt( SwSectionFmt *pFmt, sal_Bool bDelNodes = sal_False );
    void UpdateSection(sal_uInt16 const nSect, SwSectionData &,
            SfxItemSet const*const = 0, bool const bPreventLinkUpdate = false);
    String GetUniqueSectionName( const String* pChkStr = 0 ) const;

    /* @@@MAINTAINABILITY-HORROR@@@
       The model should not have anything to do with a shell.
       Unnecessary compile/link time dependency.
    */
    // Pointer auf die SfxDocShell vom Doc, kann 0 sein !!!
          SwDocShell* GetDocShell()         { return pDocShell; }
    const SwDocShell* GetDocShell() const   { return pDocShell; }
    void SetDocShell( SwDocShell* pDSh );

    // in case during copying of embedded object a new shell is created,
    // it should be set here and cleaned later
    void SetTmpDocShell( SfxObjectShellLock rLock )    { xTmpDocShell = rLock; }
    SfxObjectShellLock GetTmpDocShell()    { return xTmpDocShell; }

    // fuer die TextBausteine - diese habe nur ein SvPersist zur
    // Verfuegung
    SfxObjectShell* GetPersist() const;

    // Pointer auf den Storage des SfxDocShells, kann 0 sein !!!
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > GetDocStorage();

        // abfrage/setze Flag, ob das Dokument im asynchronen Laden ist
    bool IsInLoadAsynchron() const             { return mbInLoadAsynchron; }
    void SetInLoadAsynchron( bool bFlag )       { mbInLoadAsynchron = bFlag; }

    // erzeuge um das zu Servende Object eine Selektion
    sal_Bool SelectServerObj( const String& rStr, SwPaM*& rpPam,
                            SwNodeRange*& rpRange ) const;

    // fuer Drag&Move: ( z.B. RefMarks "verschieben" erlauben )
    bool IsCopyIsMove() const              { return mbCopyIsMove; }
    void SetCopyIsMove( bool bFlag )        { mbCopyIsMove = bFlag; }

    SwDrawContact* GroupSelection( SdrView& );
    void UnGroupSelection( SdrView& );
    sal_Bool DeleteSelection( SwDrawView& );

    // Invalidiert OnlineSpell-WrongListen
    void SpellItAgainSam( sal_Bool bInvalid, sal_Bool bOnlyWrong, sal_Bool bSmartTags );
    void InvalidateAutoCompleteFlag();

    // <--
    void SetCalcFieldValueHdl(Outliner* pOutliner);

    // erfrage ob die ::com::sun::star::util::URL besucht war. Uebers Doc, falls nur ein ::com::sun::star::text::Bookmark
    // angegeben ist. Dann muss der Doc. Name davor gesetzt werden!
    sal_Bool IsVisitedURL( const String& rURL ) const;

    // speicher die akt. Werte fuer die automatische Aufnahme von Ausnahmen
    // in die Autokorrektur
    void SetAutoCorrExceptWord( SwAutoCorrExceptWord* pNew );
    SwAutoCorrExceptWord* GetAutoCorrExceptWord()       { return pACEWord; }

    const SwFmtINetFmt* FindINetAttr( const String& rName ) const;

    // rufe ins dunkle Basic, mit evt. Return String
    sal_Bool ExecMacro( const SvxMacro& rMacro, String* pRet = 0, SbxArray* pArgs = 0 );
    // rufe ins dunkle Basic/JavaScript
    sal_uInt16 CallEvent( sal_uInt16 nEvent, const SwCallMouseEvent& rCallEvent,
                        sal_Bool bChkPtr = sal_False, SbxArray* pArgs = 0,
                        const Link* pCallBack = 0 );

    // linken Rand ueber Objectleiste einstellen (aenhlich dem Stufen von
    // Numerierungen), optional kann man "um" den Offset stufen oder "auf"
    // die Position gestuft werden (bModulus = sal_True)
    void MoveLeftMargin( const SwPaM& rPam, sal_Bool bRight = sal_True,
                        sal_Bool bModulus = sal_True );

    // Numberformatter erfragen
    inline       SvNumberFormatter* GetNumberFormatter( sal_Bool bCreate = sal_True );
    inline const SvNumberFormatter* GetNumberFormatter( sal_Bool bCreate = sal_True ) const;

    bool HasInvisibleContent() const;
    /// delete invisible content, like hidden sections and paragraphs
    bool RemoveInvisibleContent();
    /// restore the invisible content if it's available on the undo stack
    bool RestoreInvisibleContent();
    // replace fields by text - mailmerge support
    sal_Bool ConvertFieldsToText();

    // erzeuge Anhand der vorgebenen Collection Teildokumente
    // falls keine angegeben ist, nehme die Kapitelvorlage der 1. Ebene
    sal_Bool GenerateGlobalDoc( const String& rPath,
                                const SwTxtFmtColl* pSplitColl = 0 );
    sal_Bool GenerateGlobalDoc( const String& rPath, int nOutlineLevel = 0 );   //#outline level,add by zhaojianwei
    sal_Bool GenerateHTMLDoc( const String& rPath,
                                const SwTxtFmtColl* pSplitColl = 0 );
    sal_Bool GenerateHTMLDoc( const String& rPath, int nOutlineLevel = 0 ); //#outline level,add by zhaojianwei

    //  vergleiche zwei Dokument miteinander
    long CompareDoc( const SwDoc& rDoc );
    // merge zweier Dokumente
    long MergeDoc( const SwDoc& rDoc );
    // setze Kommentar-Text fuers Redline, das dann per AppendRedline
    // hereinkommt. Wird vom Autoformat benutzt. 0-Pointer setzt den Modus
    // wieder zurueck. Die SequenceNummer ist fuers UI-seitige zusammen-
    // fassen von Redlines.
    void SetAutoFmtRedlineComment( const String* pTxt, sal_uInt16 nSeqNo = 0 );

    bool IsAutoFmtRedline() const           { return mbIsAutoFmtRedline; }
    void SetAutoFmtRedline( bool bFlag )    { mbIsAutoFmtRedline = bFlag; }

    // fuer AutoFormat: mit Undo/Redlining - Behandlung
    void SetTxtFmtCollByAutoFmt( const SwPosition& rPos, sal_uInt16 nPoolId,
                                const SfxItemSet* pSet = 0 );
    void SetFmtItemByAutoFmt( const SwPaM& rPam, const SfxItemSet& );

    // !!!NUR fuer die SW-Textblocks!! beachtet kein LAYOUT!!!
    void ClearDoc();        // loescht den gesamten Inhalt.

    // erfrage / setze die Daten fuer die PagePreView
    const SwPagePreViewPrtData* GetPreViewPrtData() const { return pPgPViewPrtData; }
    // wenn der Pointer == 0 ist, dann wird im Doc der Pointer zerstoert,
    // ansonsten wird das Object kopiert.
    // Der Pointer geht NICHT in den Besitz des Doc's!!
    void SetPreViewPrtData( const SwPagePreViewPrtData* pData );

    // update all modified OLE-Objects. The modification is called over the
    // StarOne - Interface              --> Bug 67026
    void SetOLEObjModified()
    {   if( GetCurrentViewShell() ) aOLEModifiedTimer.Start(); }    //swmod 071107//swmod 071225

    // -------------------- Uno - Schnittstellen ---------------------------
    const SwUnoCrsrTbl& GetUnoCrsrTbl() const       { return *pUnoCrsrTbl; }
    SwUnoCrsr* CreateUnoCrsr( const SwPosition& rPos, sal_Bool bTblCrsr = sal_False );
    // -------------------- Uno - Schnittstellen ---------------------------

    // -------------------- FeShell - Schnittstellen -----------------------
    // !!!!! diese gehen immer davon aus, das ein Layout existiert  !!!!
    sal_Bool ChgAnchor( const SdrMarkList& _rMrkList,
                        RndStdIds _eAnchorType,
                        const sal_Bool _bSameOnly,
                        const sal_Bool _bPosCorr );

    void SetRowHeight( const SwCursor& rCursor, const SwFmtFrmSize &rNew );
    void GetRowHeight( const SwCursor& rCursor, SwFmtFrmSize *& rpSz ) const;
    void SetRowSplit( const SwCursor& rCursor, const SwFmtRowSplit &rNew );
    void GetRowSplit( const SwCursor& rCursor, SwFmtRowSplit *& rpSz ) const;
    sal_Bool BalanceRowHeight( const SwCursor& rCursor, sal_Bool bTstOnly = sal_True );
    void SetRowBackground( const SwCursor& rCursor, const SvxBrushItem &rNew );
    sal_Bool GetRowBackground( const SwCursor& rCursor, SvxBrushItem &rToFill ) const;
    void SetTabBorders( const SwCursor& rCursor, const SfxItemSet& rSet );
    void SetTabLineStyle( const SwCursor& rCursor,
                          const Color* pColor, sal_Bool bSetLine,
                          const SvxBorderLine* pBorderLine );
    void GetTabBorders( const SwCursor& rCursor, SfxItemSet& rSet ) const;
    void SetBoxAttr( const SwCursor& rCursor, const SfxPoolItem &rNew );
    sal_Bool GetBoxAttr( const SwCursor& rCursor, SfxPoolItem &rToFill ) const;
    void SetBoxAlign( const SwCursor& rCursor, sal_uInt16 nAlign );
    sal_uInt16 GetBoxAlign( const SwCursor& rCursor ) const;
    void AdjustCellWidth( const SwCursor& rCursor, sal_Bool bBalance = sal_False );

    int Chainable( const SwFrmFmt &rSource, const SwFrmFmt &rDest );
    int Chain( SwFrmFmt &rSource, const SwFrmFmt &rDest );
    void Unchain( SwFrmFmt &rFmt );

    // fuers Copy/Move aus der FrmShell
    SdrObject* CloneSdrObj( const SdrObject&, sal_Bool bMoveWithinDoc = sal_False,
                            sal_Bool bInsInPage = sal_True );

    //
    // -------------------- FeShell - Schnittstellen Ende ------------------


    // Schnittstelle fuer die TextInputDaten - ( fuer die Texteingabe
    // von japanischen/chinesischen Zeichen)
    SwExtTextInput* CreateExtTextInput( const SwPaM& rPam );
    void DeleteExtTextInput( SwExtTextInput* pDel );
    SwExtTextInput* GetExtTextInput( const SwNode& rNd,
                                xub_StrLen nCntntPos = STRING_NOTFOUND) const;
    SwExtTextInput* GetExtTextInput() const;

    // Schnistelle fuer den Zugriff auf die AutoComplete-Liste
    static SwAutoCompleteWord& GetAutoCompleteWords() { return *pACmpltWords; }

    bool ContainsMSVBasic() const          { return mbContains_MSVBasic; }
    void SetContainsMSVBasic( bool bFlag )  { mbContains_MSVBasic = bFlag; }

    // Interface for the list of Ruby - texts/attributes
    sal_uInt16 FillRubyList( const SwPaM& rPam, SwRubyList& rList,
                        sal_uInt16 nMode );
    sal_uInt16 SetRubyList( const SwPaM& rPam, const SwRubyList& rList,
                        sal_uInt16 nMode );

    void ReadLayoutCache( SvStream& rStream );
    void WriteLayoutCache( SvStream& rStream );
    SwLayoutCache* GetLayoutCache() const { return pLayoutCache; }

    /** Checks if any of the text node contains hidden characters.
        Used for optimization. Changing the view option 'view hidden text'
        has to trigger a reformatting only if some of the text is hidden.
    */
    bool ContainsHiddenChars() const;

    // call back for API wrapper
    SwModify*   GetUnoCallBack() const;

    IGrammarContact* getGrammarContact() const { return mpGrammarContact; }

    // -> #i27615#
    /** Marks/Unmarks a list level of a certain list

        OD 2008-04-02 #refactorlists#
        levels of a certain lists are marked now

        @param sListId    list Id of the list whose level has to be marked/unmarked
        @param nListLevel level to mark
        @param bValue     - sal_True  mark the level
                          - sal_False unmark the level
    */
    void MarkListLevel( const String& sListId,
                        const int nListLevel,
                        const sal_Bool bValue );

    /** Marks/Unmarks a list level of a certain list

        OD 2008-04-02 #refactorlists#
        levels of a certain lists are marked now

        @param rList      list whose level has to be marked/unmarked
        @param nListLevel level to mark
        @param bValue     - sal_True  mark the level
                          - sal_False unmark the level
     */
    void MarkListLevel( SwList& rList,
                        const int nListLevel,
                        const sal_Bool bValue );
    // <- #i27615#

    // Change a format undoable.
    void ChgFmt(SwFmt & rFmt, const SfxItemSet & rSet);

    void RenameFmt(SwFmt & rFmt, const String & sNewName,
                   sal_Bool bBroadcast = sal_False);

    // Change a TOX undoable.
    void ChgTOX(SwTOXBase & rTOX, const SwTOXBase & rNew);

    // #111827#
    /**
       Returns a textual description of a PaM.

       @param rPaM     the PaM to describe

       If rPaM only spans one paragraph the result is:

            '<text in the PaM>'

       <text in the PaM> is shortened to nUndoStringLength characters.

       If rPaM spans more than one paragraph the result is:

            paragraphs                               (STR_PARAGRAPHS)

       @return the textual description of rPaM
     */
    String GetPaMDescr(const SwPaM & rPaM) const;

    // -> #i23726#
    sal_Bool IsFirstOfNumRule(SwPosition & rPos);
    // <- #i23726#

    // --> #i31958# access methods for XForms model(s)

    /// access container for XForms model; will be NULL if !isXForms()
    com::sun::star::uno::Reference<com::sun::star::container::XNameContainer>
        getXForms() const;

    com::sun::star::uno::Reference< com::sun::star::linguistic2::XProofreadingIterator > GetGCIterator() const;

    /// is this an XForms document?
    bool isXForms() const;

    /// initialize XForms models; turn this into an XForms document
    void initXForms( bool bCreateDefaultModel );
    // <-- #i31958# access methods for XForms model(s)

    // --> OD 2006-03-21 #b6375613#
    inline bool ApplyWorkaroundForB6375613() const
    {
        return mbApplyWorkaroundForB6375613;
    }
    void SetApplyWorkaroundForB6375613( bool p_bApplyWorkaroundForB6375613 );
    // <--

    //Update all the page masters
    void SetDefaultPageMode(bool bSquaredPageMode);
    sal_Bool IsSquaredPageMode() const;

    // i#78591#
    void Setn32DummyCompatabilityOptions1( sal_uInt32 CompatabilityOptions1 )
    {
        n32DummyCompatabilityOptions1 = CompatabilityOptions1;
    }
    sal_uInt32 Getn32DummyCompatabilityOptions1( )
    {
        return n32DummyCompatabilityOptions1;
    }
    void Setn32DummyCompatabilityOptions2( sal_uInt32 CompatabilityOptions2 )
    {
        n32DummyCompatabilityOptions2 = CompatabilityOptions2;
    }
    sal_uInt32 Getn32DummyCompatabilityOptions2( )
    {
        return n32DummyCompatabilityOptions2;
    }
#ifdef FUTURE_VBA
    com::sun::star::uno::Reference< com::sun::star::script::vba::XVBAEventProcessor > GetVbaEventProcessor();
#endif
    ::sfx2::IXmlIdRegistry& GetXmlIdRegistry();
    ::sw::MetaFieldManager & GetMetaFieldManager();
    ::sw::UndoManager      & GetUndoManager();
    ::sw::UndoManager const& GetUndoManager() const;
    SfxObjectShell* CreateCopy(bool bCallInitNew) const;
};


// Diese Methode wird im Dtor vom SwDoc gerufen und loescht den Cache
// der Konturobjekte
void ClrContourCache();


//------------------ inline impl. ---------------------------------

inline const SwTableNode* SwDoc::IsIdxInTbl( const SwNodeIndex& rIdx ) const
{
    return ((SwDoc*)this)->IsIdxInTbl( rIdx );
}

inline SvNumberFormatter* SwDoc::GetNumberFormatter( sal_Bool bCreate )
{
    if( bCreate && !pNumberFormatter )
        _CreateNumberFormatter();
    return pNumberFormatter;
}

inline const SvNumberFormatter* SwDoc::GetNumberFormatter( sal_Bool bCreate ) const
{
    return ((SwDoc*)this)->GetNumberFormatter( bCreate );
}

inline void SwDoc::SetOLEPrtNotifyPending( bool bSet )
{
    mbOLEPrtNotifyPending = bSet;
    if( !bSet )
        mbAllOLENotify = sal_False;
}

// --> OD 2006-03-14 #i62875#
// namespace <docfunc> for functions and procedures working on a Writer document.
namespace docfunc
{
    /** method to check, if given Writer document contains at least one drawing object

        OD 2006-03-17 #i62875#

        @author OD

        @param p_rDoc
        input parameter - reference to the Writer document, which is investigated.
    */
    bool ExistsDrawObjs( SwDoc& p_rDoc );

    /** method to check, if given Writer document contains only drawing objects,
        which are completely on its page.

        OD 2006-03-17 #i62875#

        @author OD

        @param p_rDoc
        input parameter - reference to the Writer document, which is investigated.
    */
    bool AllDrawObjsOnPage( SwDoc& p_rDoc );

    /** method to check, if the outline style has to written as a normal list style

        OD 2006-09-27 #i69627#
        The outline style has to written as a normal list style, if a parent
        paragraph style of one of the paragraph styles, which are assigned to
        the list levels of the outline style, has a list style set or inherits
        a list style from its parent paragraphs style.
        This information is needed for the OpenDocument file format export.

        @author OD

        @param rDoc
        input parameter - reference to the text document, which is investigated.

        @return boolean
        indicating, if the outline style has to written as a normal list style
    */
    bool HasOutlineStyleToBeWrittenAsNormalListStyle( SwDoc& rDoc );
}
// <--
#endif  //_DOC_HXX
