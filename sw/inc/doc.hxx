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
#ifndef INCLUDED_SW_INC_DOC_HXX
#define INCLUDED_SW_INC_DOC_HXX

// SwDoc interfaces
#include <IInterface.hxx>
#include <IDocumentSettingAccess.hxx>
#include <IDocumentMarkAccess.hxx>
#include <IDocumentDeviceAccess.hxx> //from this class's persective this isnt even necessary anymore but I encountered at least
                                     //one different class that relies on this being included here. What to do?
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
#include <IDocumentOutlineNodes.hxx>
#include <IDocumentListItems.hxx>

#include <IDocumentListsAccess.hxx>
class SwList;

#include <IDocumentExternalData.hxx>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <vcl/timer.hxx>
#include <sal/macros.h>
#include "swdllapi.h"
#include <swtypes.hxx>
#include <swatrset.hxx>
#include <toxe.hxx>
#include <flyenum.hxx>
#include <flypos.hxx>
#include <itabenum.hxx>
#include <swdbdata.hxx>
#include <chcmprse.hxx>
#include <com/sun/star/linguistic2/XSpellChecker1.hpp>
#include <com/sun/star/linguistic2/XHyphenatedWord.hpp>
#include <rtl/ref.hxx>
#include <svx/svdtypes.hxx>
#include <sfx2/objsh.hxx>
#include <svl/style.hxx>
#include <editeng/numitem.hxx>
#include <comphelper/implementationreference.hxx>
#include <com/sun/star/chart2/data/XDataProvider.hpp>
#include <com/sun/star/linguistic2/XProofreadingIterator.hpp>
#include <com/sun/star/script/vba/XVBAEventProcessor.hpp>

#include <boost/unordered_map.hpp>

#include <svtools/embedhlp.hxx>
#include <vector>
#include <set>
#include <memory>

#include <boost/scoped_ptr.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

namespace editeng { class SvxBorderLine; }

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
class SvxMacro;
class SvxMacroTableDtor;
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
class SwFmtsBase;
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
class SwDBManager;
class SwNoTxtNode;
class SwNodeIndex;
class SwNodeRange;
class SwNodes;
class SwNumRule;
class SwNumRuleTbl;
class SwPageDesc;
class SwPagePreviewPrtData;
class SwRangeRedline;
class SwRedlineTbl;
class SwExtraRedlineTbl;
class SwRootFrm;
class SwRubyList;
class SwRubyListEntry;
class SwSectionFmt;
class SwSectionFmts;
class SwSectionData;
class SwSelBoxes;
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
class SwViewShell;
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
class _SetGetExpFlds;

namespace sw { namespace mark {
    class MarkManager;
}}
namespace sw {
    class MetaFieldManager;
    class UndoManager;
    class IShellCursorSupplier;
    class DocumentSettingManager;
    class DocumentDeviceManager;
}

namespace com { namespace sun { namespace star {
namespace i18n {
    struct ForbiddenCharacters;    //< comes from the I18N UNO interface
}
namespace uno {
    template < class > class Sequence;
}
namespace container {
    class XNameContainer; //< for getXForms()/isXForms()/initXForms() methods
}
}}}

namespace sfx2 {
    class SvLinkSource;
    class IXmlIdRegistry;
    class LinkManager;
}

// PageDescriptor-interface, Array because of inlines.
class SwPageDescs : public std::vector<SwPageDesc*>
{
public:
    // the destructor will free all objects still in the vector
    ~SwPageDescs();
};

// forward declaration
void SetAllScriptItem( SfxItemSet& rSet, const SfxPoolItem& rItem );

// global function to start grammar checking in the document
void StartGrammarChecking( SwDoc &rDoc );

// Represents the model of a Writer document.
class SW_DLLPUBLIC SwDoc :
    public IInterface,
    public IDocumentSettingAccess,
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
    public IDocumentListItems,
    public IDocumentOutlineNodes,
    public IDocumentListsAccess,
    public IDocumentExternalData
{

    friend void _InitCore();
    friend void _FinitCore();

    // private Member
    ::boost::scoped_ptr<SwNodes> m_pNodes;  //< document content (Nodes Array)
    ::boost::scoped_ptr< ::sw::DocumentDeviceManager > m_DeviceAccess;
    SwAttrPool* mpAttrPool;             //< the attribute pool
    SwPageDescs maPageDescs;             //< PageDescriptors
    Link        maOle2Link;              //< OLE 2.0-notification
    /* @@@MAINTAINABILITY-HORROR@@@
       Timer should not be members of the model
    */
    Timer       maIdleTimer;             //< Own IdleTimer
    Timer       maOLEModifiedTimer;      //< Timer for update modified OLE-Objecs
    Timer       maStatsUpdateTimer;      //< Timer for asynchronous stats calculation
    SwDBData    maDBData;                //< database descriptor
    ::com::sun::star::uno::Sequence <sal_Int8 > maRedlinePasswd;
    OUString    msTOIAutoMarkURL;        //< ::com::sun::star::util::URL of table of index AutoMark file
    boost::ptr_vector< boost::nullable<OUString> > maPatternNms;          // Array for names of document-templates
    com::sun::star::uno::Reference<com::sun::star::container::XNameContainer>
        mxXForms;                        //< container with XForms models
    mutable com::sun::star::uno::Reference< com::sun::star::linguistic2::XProofreadingIterator > m_xGCIterator;

    const ::boost::scoped_ptr< ::sw::mark::MarkManager> mpMarkManager;
    const ::boost::scoped_ptr< ::sw::MetaFieldManager > m_pMetaFieldManager;
    const ::boost::scoped_ptr< ::sw::UndoManager > m_pUndoManager;
    const ::boost::scoped_ptr< ::sw::DocumentSettingManager > m_pDocumentSettingManager;

    // Pointer
    SwFrmFmt        *mpDfltFrmFmt;       //< Default formats.
    SwFrmFmt        *mpEmptyPageFmt;     //< Format for the default empty page
    SwFrmFmt        *mpColumnContFmt;    //< Format for column container
    SwCharFmt       *mpDfltCharFmt;
    SwTxtFmtColl    *mpDfltTxtFmtColl;   //< Defaultformatcollections
    SwGrfFmtColl    *mpDfltGrfFmtColl;

    SwFrmFmts       *mpFrmFmtTbl;        //< Format table
    SwCharFmts      *mpCharFmtTbl;
    SwFrmFmts       *mpSpzFrmFmtTbl;
    SwSectionFmts   *mpSectionFmtTbl;
    SwFrmFmts       *mpTblFrmFmtTbl;     //< For tables
    SwTxtFmtColls   *mpTxtFmtCollTbl;    //< FormatCollections
    SwGrfFmtColls   *mpGrfFmtCollTbl;

    SwTOXTypes      *mpTOXTypes;         //< Tables/indices
    SwDefTOXBase_Impl * mpDefTOXBases;   //< defaults of SwTOXBase's

    SwViewShell       *mpCurrentView;  //< SwDoc should get a new member mpCurrentView
    SdrModel        *mpDrawModel;        //< StarView Drawing

    SwDocUpdtFld    *mpUpdtFlds;         //< Struct for updating fields
    SwFldTypes      *mpFldTypes;
    SwDBManager         *mpDBManager;            /**< Pointer to the DBManager for
                                         evaluation of DB-fields. */

    SwDoc           *mpGlossaryDoc;      //< Pointer to glossary-document.

    SwNumRule       *mpOutlineRule;
    SwFtnInfo       *mpFtnInfo;
    SwEndNoteInfo   *mpEndNoteInfo;
    SwLineNumberInfo*mpLineNumberInfo;
    SwFtnIdxs       *mpFtnIdxs;
    SwDocStat       *mpDocStat;          //< Statistics information.

    SwDocShell      *mpDocShell;         //< Ptr to SfxDocShell of Doc.
    SfxObjectShellLock mxTmpDocShell;    //< A temporary shell that is used to copy OLE-Nodes

    sfx2::LinkManager   *mpLinkMgr;      //< List of linked stuff (graphics/DDE/OLE).

    SwAutoCorrExceptWord *mpACEWord;     /**< For the automated takeover of
                                         auto-corrected words that are "re-corrected". */
    SwURLStateChanged *mpURLStateChgd;   //< SfxClient for changes in INetHistory
    SvNumberFormatter *mpNumberFormatter;//< NumFormatter for tables / fields

    mutable SwNumRuleTbl    *mpNumRuleTbl;   //< List of all named NumRules.

    // Hash map to find numrules by name
    mutable boost::unordered_map<OUString, SwNumRule *, OUStringHash> maNumRuleMap;

    typedef boost::unordered_map<OUString, SwList*, OUStringHash> tHashMapForLists;
    // container to hold the lists of the text document
    tHashMapForLists maLists;
    // relation between list style and its default list
    tHashMapForLists maListStyleLists;

    SwRedlineTbl        *mpRedlineTbl;           //< List of all Ranged Redlines.
    SwExtraRedlineTbl   *mpExtraRedlineTbl;      //< List of all Extra Redlines.
    OUString            *mpAutoFmtRedlnComment;  //< Comment for Redlines inserted via AutoFormat.

    SwUnoCrsrTbl    *mpUnoCrsrTbl;

    SwPagePreviewPrtData *mpPgPViewPrtData;  //< Indenting / spacing for printing of page view.
    SwPaM           *mpExtInputRing;

    SwLayouter      *mpLayouter;     /**< ::com::sun::star::frame::Controller for complex layout formatting
                                     like footnote/endnote in sections */
    IStyleAccess    *mpStyleAccess;  //< handling of automatic styles
    SwLayoutCache   *mpLayoutCache;  /**< Layout cache to read and save with the
                                     document for a faster formatting */

    SwModify *mpUnoCallBack;
    IGrammarContact *mpGrammarContact;   //< for grammar checking in paragraphs during editing

    mutable  comphelper::ImplementationReference< SwChartDataProvider
        , ::com::sun::star::chart2::data::XDataProvider >
                                maChartDataProviderImplRef;
    SwChartLockController_Helper  *mpChartControllerHelper;

    // table of forbidden characters of this document
    rtl::Reference<SvxForbiddenCharactersTable> mxForbiddenCharsTable;
    com::sun::star::uno::Reference< com::sun::star::script::vba::XVBAEventProcessor > mxVbaEvents;
    com::sun::star::uno::Reference<com::sun::star::container::XNameContainer> m_xTemplateToProjectCache;
public:
    struct lessThanNodeNum
    {
        bool operator()( const SwNodeNum* pNodeNumOne,
                         const SwNodeNum* pNodeNumTwo ) const;
    };

    typedef ::std::set< const SwNodeNum*, lessThanNodeNum > tImplSortedNodeNumList;
private:
    tImplSortedNodeNumList* mpListItemsList;

    ::std::auto_ptr< ::sfx2::IXmlIdRegistry > m_pXmlIdRegistry;

    // other

    sal_uInt16  mnUndoCnt;           //< Count of Undo Actions.
    sal_uInt16  mnUndoSttEnd;        //< != 0 -> within parentheses.

    sal_uInt16 mnAutoFmtRedlnCommentNo;  /**< SeqNo for conjoining of AutoFmt-Redlines.
                                         by the UI. Managed by SwAutoFmt! */

    RedlineMode_t meRedlineMode;     //< Current Redline Mode.

    sal_uInt32  mnRsid;              //< current session ID of the document
    sal_uInt32  mnRsidRoot;          //< session ID when the document was created

    sal_Int32   mReferenceCount;
    sal_Int32   mIdleBlockCount;
    sal_Int8    mnLockExpFld;        //< If != 0 UpdateExpFlds() has no effect!

    /** Draw Model Layer IDs
     * LayerIds, Heaven == above document
     *           Hell   == below document
     *         Controls == at the very top
     */
    SdrLayerID  mnHeaven;
    SdrLayerID  mnHell;
    SdrLayerID  mnControls;
    SdrLayerID  mnInvisibleHeaven;
    SdrLayerID  mnInvisibleHell;
    SdrLayerID  mnInvisibleControls;

    bool mbGlossDoc              : 1;    //< TRUE: glossary document.
    bool mbModified              : 1;    //< TRUE: document has changed.
    bool mbDtor                  : 1;    /**< TRUE: is in SwDoc DTOR.
                                               and unfortunately temorarily also in
                                               SwSwgReader::InLayout() when flawed
                                               frames need deletion. */
    bool mbPageNums              : 1;    //< TRUE: There are virtual page numbers.
    bool mbLoaded                : 1;    //< TRUE: Doc loaded.
    bool mbUpdateExpFld          : 1;    //< TRUE: Update expression fields.
    bool mbNewDoc                : 1;    //< TRUE: new Doc.
    bool mbNewFldLst             : 1;    //< TRUE: Rebuild field-list.
    bool mbCopyIsMove            : 1;    //< TRUE: Copy is a hidden Move.
    bool mbVisibleLinks          : 1;    //< TRUE: Links are inserted visibly.
    bool mbInReading             : 1;    //< TRUE: Document is in the process of being read.
    bool mbInXMLImport           : 1;    //< TRUE: During xml import, attribute portion building is not necessary.
    bool mbUpdateTOX             : 1;    //< TRUE: After loading document, update TOX.
    bool mbInLoadAsynchron       : 1;    //< TRUE: Document is in the process of being loaded asynchronously.
    bool mbInCallModified        : 1;    //< TRUE: in Set/Reset-Modified link.
    bool mbIsAutoFmtRedline      : 1;    //< TRUE: Redlines are recorded by Autoformat.
    bool mbOLEPrtNotifyPending   : 1;    /**< TRUE: Printer has changed. At creation of
                                                ::com::sun::star::sdbcx::View
                                                notification of OLE-Objects PrtOLENotify() is required. */
    bool mbAllOLENotify          : 1;    //< True: Notification of all objects is required.
    bool mbIsRedlineMove         : 1;    //< True: Redlines are moved into to / out of the section.
    bool mbInsOnlyTxtGlssry      : 1;    //< True: insert 'only text' glossary into doc
    bool mbContains_MSVBasic     : 1;    //< True: MS-VBasic exist is in our storage
    bool mbReadlineChecked       : 1;    //< sal_True: if the query was already shown
    bool mbLinksUpdated          : 1;    /**< #i38810#
                                          flag indicating, that the links have been updated. */
    bool mbClipBoard             : 1;    //< TRUE: this document represents the clipboard
    bool mbColumnSelection       : 1;    /**< TRUE: this content has bee created by a column selection
                                                (clipboard docs only) */
    bool mbIsPrepareSelAll       : 1;

#ifdef DBG_UTIL
    bool mbXMLExport : 1;                //< sal_True: during XML export
#endif

    // true: Document contains at least one anchored object, which is anchored AT_PAGE with a content position.
    //       Thus, certain adjustment needed during formatting for these kind of anchored objects.
    bool mbContainsAtPageObjWithContentAnchor : 1;

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

    // non-ui-compatibility flags:
    bool mbOldPrinterMetrics                        : 1;

    sal_uInt32  mn32DummyCompatabilityOptions1;
    sal_uInt32  mn32DummyCompatabilityOptions2;
    // COMPATIBILITY FLAGS END

    bool mbStartIdleTimer;                    //< idle timer mode start/stop

    static SwAutoCompleteWord *mpACmpltWords;  //< List of all words for AutoComplete

    // private methods
    void checkRedlining(RedlineMode_t& _rReadlineMode);

    DECL_LINK( AddDrawUndo, SdrUndoAction * );
                                        // DrawModel
    void DrawNotifyUndoHdl();

    /** Only for internal use and therefore private.
     Copy a range within the same or to another document.
     Position may not lie within range! */
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
                            const sal_Int32 nEndContentIndex,
                            const SwNodeIndex& rStartIdx,
                            const bool bCopyFlyAtFly = false ) const;
    sal_Int8 SetFlyFrmAnchor( SwFrmFmt& rFlyFmt, SfxItemSet& rSet, bool bNewFrms );

    typedef SwFmt* (SwDoc:: *FNCopyFmt)( const OUString&, SwFmt*, bool, bool );
    SwFmt* CopyFmt( const SwFmt& rFmt, const SwFmtsBase& rFmtArr,
                        FNCopyFmt fnCopyFmt, const SwFmt& rDfltFmt );
    void CopyFmtArr( const SwFmtsBase& rSourceArr, SwFmtsBase& rDestArr,
                        FNCopyFmt fnCopyFmt, SwFmt& rDfltFmt );
    void CopyPageDescHeaderFooterImpl( bool bCpyHeader,
                                const SwFrmFmt& rSrcFmt, SwFrmFmt& rDestFmt );
    SwFmt* FindFmtByName( const SwFmtsBase& rFmtArr, const OUString& rName ) const;

    // gcc: aFtnInfo::CopyCtor is private, therefore we too have to protect ourselves.
    SwDoc( const SwDoc &);

    // For fields:
    void _InitFieldTypes();     //< Called by CTOR!!
    void _MakeFldList( int eMode );

    // Database fields:
    void UpdateDBNumFlds( SwDBNameInfField& rDBFld, SwCalc& rCalc );
    void AddUsedDBToList( std::vector<OUString>& rDBNameList,
                          const std::vector<OUString>& rUsedDBNames );
    void AddUsedDBToList( std::vector<OUString>& rDBNameList, const OUString& rDBName );
    bool IsNameInArray( const std::vector<OUString>& rOldNames, const OUString& rName );
    void GetAllDBNames( std::vector<OUString>& rAllDBNames );
    OUString ReplaceUsedDBs( const std::vector<OUString>& rUsedDBNames,
                             const OUString& rNewName, const OUString& rFormula );
    std::vector<OUString>& FindUsedDBs( const std::vector<OUString>& rAllDBNames,
                                const OUString& rFormula,
                                std::vector<OUString>& rUsedDBNames );

    void InitDrawModel();
    void ReleaseDrawModel();

    void _CreateNumberFormatter();

    bool _UnProtectTblCells( SwTable& rTbl );

    /** Create sub-documents according to the given collection.
     If no collection is given, take chapter style of the 1st level. */
    bool SplitDoc( sal_uInt16 eDocType, const OUString& rPath, bool bOutline,
                        const SwTxtFmtColl* pSplitColl, int nOutlineLevel = 0 );

    // Update charts of given table.
    void _UpdateCharts( const SwTable& rTbl, SwViewShell& rVSh ) const;

    bool _SelectNextRubyChars( SwPaM& rPam, SwRubyListEntry& rRubyEntry,
                                sal_uInt16 nMode );

    // Our own 'IdleTimer' calls the following method
    DECL_LINK( DoIdleJobs, Timer * );
    // CharTimer calls this method.
    void DoUpdateAllCharts();
    DECL_LINK( DoUpdateModifiedOLE, Timer * );

    SwFmt *_MakeCharFmt(const OUString &, SwFmt *, bool, bool );
    SwFmt *_MakeFrmFmt(const OUString &, SwFmt *, bool, bool );
    SwFmt *_MakeTxtFmtColl(const OUString &, SwFmt *, bool, bool );

private:
    bool mbReadOnly;
    OUString msDocAccTitle;

    void InitTOXTypes();
    void Paste( const SwDoc& );
    bool DeleteAndJoinImpl(SwPaM&, const bool);
    bool DeleteAndJoinWithRedlineImpl(SwPaM&, const bool unused = false);
    bool DeleteRangeImpl(SwPaM&, const bool unused = false);
    bool DeleteRangeImplImpl(SwPaM &);
    bool ReplaceRangeImpl(SwPaM&, OUString const&, const bool);

public:
    enum DocumentType {
        DOCTYPE_NATIVE,
        DOCTYPE_MSWORD              //This doc medul is come from Ms Word
        };
    DocumentType    meDocType;
    DocumentType    GetDocumentType(){ return meDocType; }
    void            SetDocumentType( DocumentType eDocType ) { meDocType = eDocType; }

    // Life cycle
    SwDoc();
    virtual ~SwDoc();

    bool IsInDtor() const { return mbDtor; }

    /* @@@MAINTAINABILITY-HORROR@@@
       Implementation details made public.
    */
    SwNodes      & GetNodes()       { return *m_pNodes; }
    SwNodes const& GetNodes() const { return *m_pNodes; }

    // IInterface
    virtual sal_Int32 acquire() SAL_OVERRIDE;
    virtual sal_Int32 release() SAL_OVERRIDE;
    virtual sal_Int32 getReferenceCount() const SAL_OVERRIDE;

    // IDocumentSettingAccess
    virtual bool get(/*[in]*/ DocumentSettingId id) const SAL_OVERRIDE;
    virtual void set(/*[in]*/ DocumentSettingId id, /*[in]*/ bool value) SAL_OVERRIDE;
    virtual const com::sun::star::i18n::ForbiddenCharacters* getForbiddenCharacters(/*[in]*/ sal_uInt16 nLang, /*[in]*/ bool bLocaleData ) const SAL_OVERRIDE;
    virtual void setForbiddenCharacters(/*[in]*/ sal_uInt16 nLang, /*[in]*/ const com::sun::star::i18n::ForbiddenCharacters& rForbiddenCharacters ) SAL_OVERRIDE;
    virtual rtl::Reference<SvxForbiddenCharactersTable>& getForbiddenCharacterTable() SAL_OVERRIDE;
    virtual const rtl::Reference<SvxForbiddenCharactersTable>& getForbiddenCharacterTable() const SAL_OVERRIDE;
    virtual sal_uInt16 getLinkUpdateMode( /*[in]*/bool bGlobalSettings ) const SAL_OVERRIDE;
    virtual void setLinkUpdateMode( /*[in]*/ sal_uInt16 nMode ) SAL_OVERRIDE;
    virtual SwFldUpdateFlags getFieldUpdateFlags( /*[in]*/bool bGlobalSettings ) const SAL_OVERRIDE;
    virtual void setFieldUpdateFlags( /*[in]*/ SwFldUpdateFlags eMode ) SAL_OVERRIDE;
    virtual SwCharCompressType getCharacterCompressionType() const SAL_OVERRIDE;
    virtual void setCharacterCompressionType( /*[in]*/SwCharCompressType nType ) SAL_OVERRIDE;
    virtual sal_uInt32 getRsid() const;
    virtual void setRsid( sal_uInt32 nVal );
    virtual sal_uInt32 getRsidRoot() const;
    virtual void setRsidRoot( sal_uInt32 nVal );

    // IDocumentDeviceAccess
    const IDocumentDeviceAccess* getIDocumentDeviceAccessConst() const;
    IDocumentDeviceAccess* getIDocumentDeviceAccess();
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

    // IDocumentMarkAccess
    IDocumentMarkAccess* getIDocumentMarkAccess();
    const IDocumentMarkAccess* getIDocumentMarkAccess() const;

    // IDocumentRedlineAccess
    virtual RedlineMode_t GetRedlineMode() const SAL_OVERRIDE;
    virtual void SetRedlineMode_intern(/*[in]*/RedlineMode_t eMode) SAL_OVERRIDE;
    virtual void SetRedlineMode(/*[in]*/RedlineMode_t eMode) SAL_OVERRIDE;
    virtual bool IsRedlineOn() const SAL_OVERRIDE;
    virtual bool IsIgnoreRedline() const SAL_OVERRIDE;
    virtual bool IsInRedlines(const SwNode& rNode) const SAL_OVERRIDE;
    virtual const SwRedlineTbl& GetRedlineTbl() const SAL_OVERRIDE;
    virtual const SwExtraRedlineTbl& GetExtraRedlineTbl() const SAL_OVERRIDE;
    virtual SwExtraRedlineTbl& GetExtraRedlineTbl() SAL_OVERRIDE;
    virtual bool HasExtraRedlineTbl() const SAL_OVERRIDE;
    virtual bool AppendRedline(/*[in]*/SwRangeRedline* pPtr, /*[in]*/bool bCallDelete) SAL_OVERRIDE;
    virtual bool AppendTableRowRedline(/*[in]*/SwTableRowRedline* pPtr, /*[in]*/bool bCallDelete) SAL_OVERRIDE;
    virtual bool AppendTableCellRedline(/*[in]*/SwTableCellRedline* pPtr, /*[in]*/bool bCallDelete) SAL_OVERRIDE;
    virtual bool SplitRedline(const SwPaM& rPam) SAL_OVERRIDE;
    virtual bool DeleteRedline(/*[in]*/const SwPaM& rPam, /*[in]*/bool bSaveInUndo, /*[in]*/sal_uInt16 nDelType) SAL_OVERRIDE;
    virtual bool DeleteRedline(/*[in]*/const SwStartNode& rSection, /*[in]*/bool bSaveInUndo, /*[in]*/sal_uInt16 nDelType) SAL_OVERRIDE;
    virtual sal_uInt16 GetRedlinePos(/*[in]*/const SwNode& rNode, /*[in]*/sal_uInt16 nType) const SAL_OVERRIDE;
    virtual void CompressRedlines() SAL_OVERRIDE;
    virtual const SwRangeRedline* GetRedline(/*[in]*/const SwPosition& rPos, /*[in]*/sal_uInt16* pFndPos) const SAL_OVERRIDE;
    virtual bool IsRedlineMove() const SAL_OVERRIDE;
    virtual void SetRedlineMove(/*[in]*/bool bFlag) SAL_OVERRIDE;
    virtual bool AcceptRedline(/*[in]*/sal_uInt16 nPos, /*[in]*/bool bCallDelete) SAL_OVERRIDE;
    virtual bool AcceptRedline(/*[in]*/const SwPaM& rPam, /*[in]*/bool bCallDelete) SAL_OVERRIDE;
    virtual bool RejectRedline(/*[in]*/sal_uInt16 nPos, /*[in]*/bool bCallDelete) SAL_OVERRIDE;
    virtual bool RejectRedline(/*[in]*/const SwPaM& rPam, /*[in]*/bool bCallDelete) SAL_OVERRIDE;
    virtual const SwRangeRedline* SelNextRedline(/*[in]*/SwPaM& rPam) const SAL_OVERRIDE;
    virtual const SwRangeRedline* SelPrevRedline(/*[in]*/SwPaM& rPam) const SAL_OVERRIDE;
    virtual void UpdateRedlineAttr() SAL_OVERRIDE;
    virtual sal_uInt16 GetRedlineAuthor() SAL_OVERRIDE;
    virtual sal_uInt16 InsertRedlineAuthor(const OUString& rAuthor) SAL_OVERRIDE;
    virtual bool SetRedlineComment(/*[in]*/const SwPaM& rPam, /*[in]*/const OUString& rComment) SAL_OVERRIDE;
    virtual const ::com::sun::star::uno::Sequence <sal_Int8>& GetRedlinePassword() const SAL_OVERRIDE;
    virtual void SetRedlinePassword(/*[in]*/const ::com::sun::star::uno::Sequence <sal_Int8>& rNewPassword) SAL_OVERRIDE;

    // IDocumentUndoRedo
    IDocumentUndoRedo      & GetIDocumentUndoRedo();
    IDocumentUndoRedo const& GetIDocumentUndoRedo() const;

    // IDocumentLinksAdministration
    virtual bool IsVisibleLinks() const SAL_OVERRIDE;
    virtual void SetVisibleLinks(bool bFlag) SAL_OVERRIDE;
    virtual sfx2::LinkManager& GetLinkManager() SAL_OVERRIDE;
    virtual const sfx2::LinkManager& GetLinkManager() const SAL_OVERRIDE;
    virtual void UpdateLinks(bool bUI) SAL_OVERRIDE;
    virtual bool GetData(const OUString& rItem, const OUString& rMimeType, ::com::sun::star::uno::Any& rValue) const SAL_OVERRIDE;
    virtual bool SetData(const OUString& rItem, const OUString& rMimeType, const ::com::sun::star::uno::Any& rValue) SAL_OVERRIDE;
    virtual ::sfx2::SvLinkSource* CreateLinkSource(const OUString& rItem) SAL_OVERRIDE;
    virtual bool EmbedAllLinks() SAL_OVERRIDE;
    virtual void SetLinksUpdated(const bool bNewLinksUpdated) SAL_OVERRIDE;
    virtual bool LinksUpdated() const SAL_OVERRIDE;

    // IDocumentFieldsAccess
    virtual const SwFldTypes *GetFldTypes() const SAL_OVERRIDE;
    virtual SwFieldType *InsertFldType(const SwFieldType &) SAL_OVERRIDE;
    virtual SwFieldType *GetSysFldType( const sal_uInt16 eWhich ) const SAL_OVERRIDE;
    virtual SwFieldType* GetFldType(sal_uInt16 nResId, const OUString& rName, bool bDbFieldMatching) const SAL_OVERRIDE;
    virtual void RemoveFldType(sal_uInt16 nFld) SAL_OVERRIDE;
    virtual void UpdateFlds( SfxPoolItem* pNewHt, bool bCloseDB) SAL_OVERRIDE;
    virtual void InsDeletedFldType(SwFieldType &) SAL_OVERRIDE;
    virtual bool PutValueToField(const SwPosition & rPos, const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich) SAL_OVERRIDE;
    virtual bool UpdateFld(SwTxtFld * rDstFmtFld, SwField & rSrcFld, SwMsgPoolItem * pMsgHnt, bool bUpdateTblFlds) SAL_OVERRIDE;
    virtual void UpdateRefFlds(SfxPoolItem* pHt) SAL_OVERRIDE;
    virtual void UpdateTblFlds(SfxPoolItem* pHt) SAL_OVERRIDE;
    virtual void UpdateExpFlds(SwTxtFld* pFld, bool bUpdateRefFlds) SAL_OVERRIDE;
    virtual void UpdateUsrFlds() SAL_OVERRIDE;
    virtual void UpdatePageFlds(SfxPoolItem*) SAL_OVERRIDE;
    virtual void LockExpFlds() SAL_OVERRIDE;
    virtual void UnlockExpFlds() SAL_OVERRIDE;
    virtual bool IsExpFldsLocked() const SAL_OVERRIDE;
    virtual SwDocUpdtFld& GetUpdtFlds() const SAL_OVERRIDE;
    virtual bool SetFieldsDirty(bool b, const SwNode* pChk, sal_uLong nLen) SAL_OVERRIDE;
    virtual void SetFixFields(bool bOnlyTimeDate, const DateTime* pNewDateTime) SAL_OVERRIDE;
    virtual void FldsToCalc(SwCalc& rCalc, sal_uLong nLastNd, sal_uInt16 nLastCnt) SAL_OVERRIDE;
    virtual void FldsToCalc(SwCalc& rCalc, const _SetGetExpFld& rToThisFld) SAL_OVERRIDE;
    virtual void FldsToExpand(SwHash**& ppTbl, sal_uInt16& rTblSize, const _SetGetExpFld& rToThisFld) SAL_OVERRIDE;
    virtual bool IsNewFldLst() const SAL_OVERRIDE;
    virtual void SetNewFldLst( bool bFlag) SAL_OVERRIDE;
    virtual void InsDelFldInFldLst(bool bIns, const SwTxtFld& rFld) SAL_OVERRIDE;

    /** Returns the field at a certain position.
       @param rPos position to search at
       @return pointer to field at the given position or NULL in case no field is found
    */
    static SwField* GetFieldAtPos(const SwPosition& rPos);

    /** Returns the field at a certain position.
       @param rPos position to search at
       @return pointer to field at the given position or NULL in case no field is found
    */
    static SwTxtFld* GetTxtFldAtPos(const SwPosition& rPos);
    bool containsUpdatableFields();

    // IDocumentContentOperations
    virtual bool CopyRange(SwPaM&, SwPosition&, const bool bCopyAll) const SAL_OVERRIDE;
    virtual void DeleteSection(SwNode* pNode) SAL_OVERRIDE;
    virtual bool DeleteRange(SwPaM&) SAL_OVERRIDE;
    virtual bool DelFullPara(SwPaM&) SAL_OVERRIDE;
    // Add optional parameter <bForceJoinNext>, default value <false>
    // Needed for hiding of deletion redlines
    virtual bool DeleteAndJoin( SwPaM&,
                                const bool bForceJoinNext = false ) SAL_OVERRIDE;
    virtual bool MoveRange(SwPaM&, SwPosition&, SwMoveFlags) SAL_OVERRIDE;
    virtual bool MoveNodeRange(SwNodeRange&, SwNodeIndex&, SwMoveFlags) SAL_OVERRIDE;
    virtual bool MoveAndJoin(SwPaM&, SwPosition&, SwMoveFlags) SAL_OVERRIDE;
    virtual bool Overwrite(const SwPaM &rRg, const OUString& rStr) SAL_OVERRIDE;
    virtual bool InsertString(const SwPaM &rRg, const OUString&,
              const enum InsertFlags nInsertMode = INS_EMPTYEXPAND ) SAL_OVERRIDE;
    virtual bool UpdateParRsid( SwTxtNode *pTxtNode, sal_uInt32 nVal = 0 );
    virtual bool UpdateRsid( const SwPaM &rRg, sal_Int32 nLen );
    virtual SwFlyFrmFmt* Insert(const SwPaM &rRg, const OUString& rGrfName, const OUString& rFltName, const Graphic* pGraphic,
                        const SfxItemSet* pFlyAttrSet, const SfxItemSet* pGrfAttrSet, SwFrmFmt*) SAL_OVERRIDE;
    virtual SwFlyFrmFmt* Insert(const SwPaM& rRg, const GraphicObject& rGrfObj, const SfxItemSet* pFlyAttrSet,
                        const SfxItemSet* pGrfAttrSet, SwFrmFmt*) SAL_OVERRIDE;
    virtual SwDrawFrmFmt* InsertDrawObj(
        const SwPaM &rRg,
        SdrObject& rDrawObj,
        const SfxItemSet& rFlyAttrSet ) SAL_OVERRIDE;
    virtual SwFlyFrmFmt* Insert(const SwPaM &rRg, const svt::EmbeddedObjectRef& xObj, const SfxItemSet* pFlyAttrSet,
                        const SfxItemSet* pGrfAttrSet, SwFrmFmt*) SAL_OVERRIDE;

    // Add a para for the char attribute exp...
    virtual bool InsertPoolItem(
        const SwPaM &rRg,
        const SfxPoolItem&,
        const SetAttrMode nFlags,
        const bool bExpandCharToPara=false) SAL_OVERRIDE;

    virtual bool InsertItemSet (const SwPaM &rRg, const SfxItemSet&,
                                const SetAttrMode nFlags) SAL_OVERRIDE;
    virtual void ReRead(SwPaM&, const OUString& rGrfName, const OUString& rFltName, const Graphic* pGraphic, const GraphicObject* pGrfObj) SAL_OVERRIDE;
    virtual void TransliterateText(const SwPaM& rPaM, utl::TransliterationWrapper&) SAL_OVERRIDE;
    virtual SwFlyFrmFmt* InsertOLE(const SwPaM &rRg, const OUString& rObjName, sal_Int64 nAspect, const SfxItemSet* pFlyAttrSet,
                           const SfxItemSet* pGrfAttrSet, SwFrmFmt*) SAL_OVERRIDE;
    virtual bool SplitNode(const SwPosition &rPos, bool bChkTableStart) SAL_OVERRIDE;
    virtual bool AppendTxtNode(SwPosition& rPos) SAL_OVERRIDE;
        virtual void SetModified(SwPaM &rPaM);
    virtual bool ReplaceRange(SwPaM& rPam, const OUString& rNewStr,
                              const bool bRegExReplace) SAL_OVERRIDE;
    virtual void RemoveLeadingWhiteSpace(const SwPosition & rPos ) SAL_OVERRIDE;

    // IDocumentStylePoolAccess
    virtual SwTxtFmtColl* GetTxtCollFromPool( sal_uInt16 nId, bool bRegardLanguage = true ) SAL_OVERRIDE;
    virtual SwFmt* GetFmtFromPool( sal_uInt16 nId ) SAL_OVERRIDE;
    virtual SwFrmFmt* GetFrmFmtFromPool( sal_uInt16 nId ) SAL_OVERRIDE;
    virtual SwCharFmt* GetCharFmtFromPool( sal_uInt16 nId ) SAL_OVERRIDE;
    virtual SwPageDesc* GetPageDescFromPool( sal_uInt16 nId, bool bRegardLanguage = true ) SAL_OVERRIDE;
    virtual SwNumRule* GetNumRuleFromPool( sal_uInt16 nId ) SAL_OVERRIDE;
    virtual bool IsPoolTxtCollUsed( sal_uInt16 nId ) const SAL_OVERRIDE;
    virtual bool IsPoolFmtUsed( sal_uInt16 nId ) const SAL_OVERRIDE;
    virtual bool IsPoolPageDescUsed( sal_uInt16 nId ) const SAL_OVERRIDE;

    // IDocumentLineNumberAccess
    virtual const SwLineNumberInfo& GetLineNumberInfo() const SAL_OVERRIDE;
    virtual void SetLineNumberInfo(const SwLineNumberInfo& rInfo) SAL_OVERRIDE;

    // IDocumentStatistics
    virtual void DocInfoChgd() SAL_OVERRIDE;
    virtual const SwDocStat &GetDocStat() const SAL_OVERRIDE;
    virtual const SwDocStat &GetUpdatedDocStat(bool bCompleteAsync = false, bool bFields = true) SAL_OVERRIDE;
    virtual void SetDocStat(const SwDocStat& rStat) SAL_OVERRIDE;
    virtual void UpdateDocStat(bool bCompleteAsync = false, bool bFields = true) SAL_OVERRIDE;

    // IDocumentState
    virtual void SetModified() SAL_OVERRIDE;
    virtual void ResetModified() SAL_OVERRIDE;
    virtual bool IsModified() const SAL_OVERRIDE;
    virtual bool IsLoaded() const SAL_OVERRIDE;
    virtual bool IsUpdateExpFld() const SAL_OVERRIDE;
    virtual bool IsNewDoc() const SAL_OVERRIDE;
    virtual bool IsPageNums() const SAL_OVERRIDE;
    virtual void SetPageNums(bool b) SAL_OVERRIDE;
    virtual void SetNewDoc(bool b) SAL_OVERRIDE;
    virtual void SetUpdateExpFldStat(bool b) SAL_OVERRIDE;
    virtual void SetLoaded(bool b) SAL_OVERRIDE;

    // IDocumentDrawModelAccess
    virtual const SdrModel* GetDrawModel() const SAL_OVERRIDE;
    virtual SdrModel* GetDrawModel() SAL_OVERRIDE;
    virtual SdrLayerID GetHeavenId() const SAL_OVERRIDE;
    virtual SdrLayerID GetHellId() const SAL_OVERRIDE;
    virtual SdrLayerID GetControlsId() const SAL_OVERRIDE;
    virtual SdrLayerID GetInvisibleHeavenId() const SAL_OVERRIDE;
    virtual SdrLayerID GetInvisibleHellId() const SAL_OVERRIDE;
    virtual SdrLayerID GetInvisibleControlsId() const SAL_OVERRIDE;
    virtual void NotifyInvisibleLayers( SdrPageView& _rSdrPageView ) SAL_OVERRIDE;
    virtual bool IsVisibleLayerId( const SdrLayerID& _nLayerId ) const SAL_OVERRIDE;
    virtual SdrLayerID GetVisibleLayerIdByInvisibleOne( const SdrLayerID& _nInvisibleLayerId ) SAL_OVERRIDE;
    virtual SdrLayerID GetInvisibleLayerIdByVisibleOne( const SdrLayerID& _nVisibleLayerId ) SAL_OVERRIDE;
    virtual SdrModel* _MakeDrawModel() SAL_OVERRIDE;
    virtual SdrModel* GetOrCreateDrawModel() SAL_OVERRIDE;

    // IDocumentLayoutAccess
    virtual void SetCurrentViewShell( SwViewShell* pNew ) SAL_OVERRIDE;
    virtual SwLayouter* GetLayouter() SAL_OVERRIDE;
    virtual const SwLayouter* GetLayouter() const SAL_OVERRIDE;
    virtual void SetLayouter( SwLayouter* pNew ) SAL_OVERRIDE;
    virtual SwFrmFmt* MakeLayoutFmt( RndStdIds eRequest, const SfxItemSet* pSet ) SAL_OVERRIDE;
    virtual void DelLayoutFmt( SwFrmFmt *pFmt ) SAL_OVERRIDE;
    virtual SwFrmFmt* CopyLayoutFmt( const SwFrmFmt& rSrc, const SwFmtAnchor& rNewAnchor, bool bSetTxtFlyAtt, bool bMakeFrms ) SAL_OVERRIDE;
    virtual const SwViewShell *GetCurrentViewShell() const SAL_OVERRIDE;
    virtual SwViewShell *GetCurrentViewShell() SAL_OVERRIDE; //< It must be able to communicate to a SwViewShell.This is going to be removerd later.
    virtual const SwRootFrm *GetCurrentLayout() const SAL_OVERRIDE;
    virtual SwRootFrm *GetCurrentLayout() SAL_OVERRIDE;
    virtual bool HasLayout() const SAL_OVERRIDE;
    void ClearSwLayouterEntries();

    // IDocumentTimerAccess
    virtual void StartIdling() SAL_OVERRIDE;
    virtual void StopIdling() SAL_OVERRIDE;
    virtual void BlockIdling() SAL_OVERRIDE;
    virtual void UnblockIdling() SAL_OVERRIDE;
    virtual void StartBackgroundJobs() SAL_OVERRIDE;

    // IDocumentChartDataProviderAccess
    virtual SwChartDataProvider * GetChartDataProvider( bool bCreate = false ) const SAL_OVERRIDE;
    virtual void CreateChartInternalDataProviders( const SwTable *pTable ) SAL_OVERRIDE;
    virtual SwChartLockController_Helper & GetChartControllerHelper() SAL_OVERRIDE;

    // IDocumentListItems
    virtual void addListItem( const SwNodeNum& rNodeNum ) SAL_OVERRIDE;
    virtual void removeListItem( const SwNodeNum& rNodeNum ) SAL_OVERRIDE;
    virtual OUString getListItemText( const SwNodeNum& rNodeNum,
                                    const bool bWithNumber = true,
                                    const bool bWithSpacesForLevel = false ) const SAL_OVERRIDE;
    virtual void getListItems( IDocumentListItems::tSortedNodeNumList& orNodeNumList ) const SAL_OVERRIDE;
    virtual void getNumItems( IDocumentListItems::tSortedNodeNumList& orNodeNumList ) const SAL_OVERRIDE;

    // IDocumentOutlineNodes
    virtual sal_Int32 getOutlineNodesCount() const SAL_OVERRIDE;
    virtual int getOutlineLevel( const sal_Int32 nIdx ) const SAL_OVERRIDE;
    virtual OUString getOutlineText( const sal_Int32 nIdx,
                                   const bool bWithNumber,
                                   const bool bWithSpacesForLevel,
                                   const bool bWithFtn ) const SAL_OVERRIDE;
    virtual SwTxtNode* getOutlineNode( const sal_Int32 nIdx ) const SAL_OVERRIDE;
    virtual void getOutlineNodes( IDocumentOutlineNodes::tSortedOutlineNodeList& orOutlineNodeList ) const SAL_OVERRIDE;

    // IDocumentListsAccess
    virtual SwList* createList( const OUString& rListId,
                                const OUString& rDefaultListStyleName ) SAL_OVERRIDE;
    virtual void deleteList( const OUString& rListId ) SAL_OVERRIDE;
    virtual SwList* getListByName( const OUString& rListId ) const SAL_OVERRIDE;
    virtual SwList* createListForListStyle( const OUString& rListStyleName ) SAL_OVERRIDE;
    virtual SwList* getListForListStyle( const OUString& rListStyleName ) const SAL_OVERRIDE;
    virtual void deleteListForListStyle( const OUString& rListStyleName ) SAL_OVERRIDE;
    virtual void trackChangeOfListStyleName( const OUString& rListStyleName,
                                             const OUString& rNewListStyleName ) SAL_OVERRIDE;

    // IDocumentExternalData
    virtual void setExternalData(::sw::tExternalDataType eType,
                                 ::sw::tExternalDataPointer pPayload) SAL_OVERRIDE;
    virtual ::sw::tExternalDataPointer getExternalData(::sw::tExternalDataType eType) SAL_OVERRIDE;

    virtual void setDocReadOnly( bool b) { mbReadOnly = b; }
    virtual bool getDocReadOnly() const { return mbReadOnly; }
    virtual void setDocAccTitle( const OUString& rTitle ) { msDocAccTitle = rTitle; }
    virtual const OUString getDocAccTitle() const { return msDocAccTitle; }

    // INextInterface here
    DECL_STATIC_LINK( SwDoc, BackgroundDone, SvxBrushItem *);
    DECL_LINK(CalcFieldValueHdl, EditFieldInfo*);

    // OLE ???
    bool IsOLEPrtNotifyPending() const  { return mbOLEPrtNotifyPending; }
    inline void SetOLEPrtNotifyPending( bool bSet = true );
    void PrtOLENotify( bool bAll ); //All or only marked

#ifdef DBG_UTIL
    bool InXMLExport() const            { return mbXMLExport; }
    void SetXMLExport( bool bFlag )     { mbXMLExport = bFlag; }
#endif
    void SetSelAll( bool bSel )
    {
        mbIsPrepareSelAll = bSel;
    }
    bool IsPrepareSelAll() const { return mbIsPrepareSelAll; }
    void SetPrepareSelAll() { mbIsPrepareSelAll = true; }

    void SetContainsAtPageObjWithContentAnchor( const bool bFlag )
    {
        mbContainsAtPageObjWithContentAnchor = bFlag;
    }
    bool DoesContainAtPageObjWithContentAnchor()
    {
        return mbContainsAtPageObjWithContentAnchor;
    }

    /** Returns positions of all FlyFrames in the document.
     If a Pam-Pointer is passed the FlyFrames attached to paragraphes
     have to be surrounded completely by ::com::sun::star::awt::Selection.
     ( Start < Pos < End ) !!!
     (Required for Writers.) */
    SwPosFlyFrms GetAllFlyFmts( const SwPaM* = 0,
                        bool bDrawAlso = false,
                        bool bAsCharAlso = false ) const;

    // Because swrtf.cxx and define private public here now.
    SwFlyFrmFmt  *MakeFlyFrmFmt (const OUString &rFmtName, SwFrmFmt *pDerivedFrom);
    SwDrawFrmFmt *MakeDrawFrmFmt(const OUString &rFmtName, SwFrmFmt *pDerivedFrom);

    // From now on this interface has to be used for Flys.
    // pAnchorPos must be set, if they are not attached to pages AND
    // ::com::sun::star::chaos::Anchor is not already set at valid CntntPos
    // in FlySet/FrmFmt.
    /* new parameter bCalledFromShell

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
                                 bool bCalledFromShell = false );
    SwFlyFrmFmt* MakeFlyAndMove( const SwPaM& rPam, const SfxItemSet& rSet,
                                const SwSelBoxes* pSelBoxes = 0,
                                SwFrmFmt *pParent = 0 );

    void CopyWithFlyInFly( const SwNodeRange& rRg,
                            const sal_Int32 nEndContentIndex,
                            const SwNodeIndex& rInsPos,
                            const SwPaM* pCopiedPaM = NULL,
                            bool bMakeNewFrms = true,
                            bool bDelRedlines = true,
                            bool bCopyFlyAtFly = false ) const;

    bool SetFlyFrmAttr( SwFrmFmt& rFlyFmt, SfxItemSet& rSet );

    bool SetFrmFmtToFly( SwFrmFmt& rFlyFmt, SwFrmFmt& rNewFmt,
                        SfxItemSet* pSet = 0, bool bKeepOrient = false );
    void SetFlyFrmTitle( SwFlyFrmFmt& rFlyFrmFmt,
                         const OUString& sNewTitle );
    void SetFlyFrmDescription( SwFlyFrmFmt& rFlyFrmFmt,
                               const OUString& sNewDescription );

    // Footnotes
    // Footnote information
    const SwFtnInfo& GetFtnInfo() const         { return *mpFtnInfo; }
    void SetFtnInfo(const SwFtnInfo& rInfo);
    const SwEndNoteInfo& GetEndNoteInfo() const { return *mpEndNoteInfo; }
    void SetEndNoteInfo(const SwEndNoteInfo& rInfo);
          SwFtnIdxs& GetFtnIdxs()       { return *mpFtnIdxs; }
    const SwFtnIdxs& GetFtnIdxs() const { return *mpFtnIdxs; }
    // change footnotes in area
    bool SetCurFtn( const SwPaM& rPam, const OUString& rNumStr,
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

    // Glossary Document
    void SetGlossDoc( bool bGlssDc = true ) { mbGlossDoc = bGlssDc; }
    bool IsInsOnlyTextGlossary() const      { return mbInsOnlyTxtGlssry; }

    void Summary( SwDoc* pExtDoc, sal_uInt8 nLevel, sal_uInt8 nPara, bool bImpress );

    void ChangeAuthorityData(const SwAuthEntry* pNewData);

    bool IsInCallModified() const      { return mbInCallModified; }
    bool IsInHeaderFooter( const SwNodeIndex& rIdx ) const;
    short GetTextDirection( const SwPosition& rPos,
                            const Point* pPt = 0 ) const;
    bool IsInVerticalText( const SwPosition& rPos,
                               const Point* pPt = 0 ) const;

    // Database  and DB-Manager
    void SetDBManager( SwDBManager* pNewMgr )     { mpDBManager = pNewMgr; }
    SwDBManager* GetDBManager() const             { return mpDBManager; }
    void ChangeDBFields( const std::vector<OUString>& rOldNames,
                        const OUString& rNewName );
    void SetInitDBFields(bool b);

    // Find out which databases are used by fields.
    void GetAllUsedDB( std::vector<OUString>& rDBNameList,
                       const std::vector<OUString>* pAllDBNames = 0 );

    void ChgDBData( const SwDBData& rNewData );
    SwDBData GetDBData();
    const SwDBData& GetDBDesc();
    const SwDBData& _GetDBDesc() const { return maDBData; }

    // Some helper functions
    OUString GetUniqueGrfName() const;
    OUString GetUniqueOLEName() const;
    OUString GetUniqueFrameName() const;

    std::set<SwRootFrm*> GetAllLayouts();

    void SetFlyName( SwFlyFrmFmt& rFmt, const OUString& rName );
    const SwFlyFrmFmt* FindFlyByName( const OUString& rName, sal_Int8 nNdTyp = 0 ) const;

    void GetGrfNms( const SwFlyFrmFmt& rFmt, OUString* pGrfName, OUString* pFltName ) const;

    // Set a valid name for all Flys that have none (Called by Readers after reading).
    void SetAllUniqueFlyNames();

    /** Reset attributes. All TxtHints and (if completely selected) all hard-
     formated stuff (auto-formats) are removed.
     Introduce new optional parameter <bSendDataChangedEvents> in order to
     control, if the side effect "send data changed events" is triggered or not. */
    void ResetAttrs( const SwPaM &rRg,
                     bool bTxtAttr = true,
                     const std::set<sal_uInt16> &rAttrs = std::set<sal_uInt16>(),
                     const bool bSendDataChangedEvents = true );
    void RstTxtAttrs(const SwPaM &rRg, bool bInclRefToxMark = false );

    /** Set attribute in given format.1y
     *  If Undo is enabled, the old values is added to the Undo history. */
    void SetAttr( const SfxPoolItem&, SwFmt& );
    /** Set attribute in given format.1y
     *  If Undo is enabled, the old values is added to the Undo history. */
    void SetAttr( const SfxItemSet&, SwFmt& );

    // method to reset a certain attribute at the given format
    void ResetAttrAtFormat( const sal_uInt16 nWhichId,
                            SwFmt& rChangedFormat );

    /** Set attribute as new default attribute in current document.
     If Undo is activated, the old one is listed in Undo-History. */
    void SetDefault( const SfxPoolItem& );
    void SetDefault( const SfxItemSet& );

    // Query default attribute in this document.
    const SfxPoolItem& GetDefault( sal_uInt16 nFmtHint ) const;

    // Do not expand text attributes.
    bool DontExpandFmt( const SwPosition& rPos, bool bFlag = true );

    // Formats
    const SwFrmFmts* GetFrmFmts() const     { return mpFrmFmtTbl; }
          SwFrmFmts* GetFrmFmts()           { return mpFrmFmtTbl; }
    const SwCharFmts* GetCharFmts() const   { return mpCharFmtTbl;}

    // LayoutFormats (frames, DrawObjects), sometimes const sometimes not
    const SwFrmFmts* GetSpzFrmFmts() const   { return mpSpzFrmFmtTbl; }
          SwFrmFmts* GetSpzFrmFmts()         { return mpSpzFrmFmtTbl; }

    const SwFrmFmt *GetDfltFrmFmt() const   { return mpDfltFrmFmt; }
          SwFrmFmt *GetDfltFrmFmt()         { return mpDfltFrmFmt; }
    const SwFrmFmt *GetEmptyPageFmt() const { return mpEmptyPageFmt; }
          SwFrmFmt *GetEmptyPageFmt()       { return mpEmptyPageFmt; }
    const SwFrmFmt *GetColumnContFmt() const{ return mpColumnContFmt; }
          SwFrmFmt *GetColumnContFmt()      { return mpColumnContFmt; }
    const SwCharFmt *GetDfltCharFmt() const { return mpDfltCharFmt;}
          SwCharFmt *GetDfltCharFmt()       { return mpDfltCharFmt;}

    // @return the interface of the management of (auto)styles
    IStyleAccess& GetIStyleAccess() { return *mpStyleAccess; }

    // Remove all language dependencies from all existing formats
    void RemoveAllFmtLanguageDependencies();

    SwFrmFmt  *MakeFrmFmt(const OUString &rFmtName, SwFrmFmt *pDerivedFrom,
                          bool bBroadcast = false, bool bAuto = true);
    void       DelFrmFmt( SwFrmFmt *pFmt, bool bBroadcast = false );
    SwFrmFmt* FindFrmFmtByName( const OUString& rName ) const
        {   return (SwFrmFmt*)FindFmtByName( (SwFmtsBase&)*mpFrmFmtTbl, rName ); }

    SwCharFmt *MakeCharFmt(const OUString &rFmtName, SwCharFmt *pDerivedFrom,
                           bool bBroadcast = false,
                           bool bAuto = true );
    void       DelCharFmt(sal_uInt16 nFmt, bool bBroadcast = false);
    void       DelCharFmt(SwCharFmt* pFmt, bool bBroadcast = false);
    SwCharFmt* FindCharFmtByName( const OUString& rName ) const
        {   return (SwCharFmt*)FindFmtByName( (SwFmtsBase&)*mpCharFmtTbl, rName ); }

    // Formatcollections (styles)
    // TXT
    const SwTxtFmtColl* GetDfltTxtFmtColl() const { return mpDfltTxtFmtColl; }
    const SwTxtFmtColls *GetTxtFmtColls() const { return mpTxtFmtCollTbl; }
    SwTxtFmtColl *MakeTxtFmtColl( const OUString &rFmtName,
                                  SwTxtFmtColl *pDerivedFrom,
                                  bool bBroadcast = false,
                                  bool bAuto = true );
    SwConditionTxtFmtColl* MakeCondTxtFmtColl( const OUString &rFmtName,
                                               SwTxtFmtColl *pDerivedFrom,
                                               bool bBroadcast = false);
    void DelTxtFmtColl(sal_uInt16 nFmt, bool bBroadcast = false);
    void DelTxtFmtColl( SwTxtFmtColl* pColl, bool bBroadcast = false );
    /** Add 4th optional parameter <bResetListAttrs>.
     'side effect' of <SetTxtFmtColl> with <bReset = true> is that the hard
     attributes of the affected text nodes are cleared, except the break
     attribute, the page description attribute and the list style attribute.
     The new parameter <bResetListAttrs> indicates, if the list attributes
     (list style, restart at and restart with) are cleared as well in case
     that <bReset = true> and the paragraph style has a list style attribute set. */
    bool SetTxtFmtColl( const SwPaM &rRg, SwTxtFmtColl *pFmt,
                            bool bReset = true,
                            bool bResetListAttrs = false );
    SwTxtFmtColl* FindTxtFmtCollByName( const OUString& rName ) const
        {   return (SwTxtFmtColl*)FindFmtByName( (SwFmtsBase&)*mpTxtFmtCollTbl, rName ); }

    void ChkCondColls();

    const SwGrfFmtColl* GetDfltGrfFmtColl() const   { return mpDfltGrfFmtColl; }
    const SwGrfFmtColls *GetGrfFmtColls() const     { return mpGrfFmtCollTbl; }
    SwGrfFmtColl *MakeGrfFmtColl(const OUString &rFmtName,
                                    SwGrfFmtColl *pDerivedFrom);
    SwGrfFmtColl* FindGrfFmtCollByName( const OUString& rName ) const
        {   return (SwGrfFmtColl*)FindFmtByName( (SwFmtsBase&)*mpGrfFmtCollTbl, rName ); }

    // Table formatting
    const SwFrmFmts* GetTblFrmFmts() const  { return mpTblFrmFmtTbl; }
          SwFrmFmts* GetTblFrmFmts()        { return mpTblFrmFmtTbl; }
    sal_uInt16 GetTblFrmFmtCount( bool bUsed ) const;
    SwFrmFmt& GetTblFrmFmt(sal_uInt16 nFmt, bool bUsed ) const;
    SwTableFmt* MakeTblFrmFmt(const OUString &rFmtName, SwFrmFmt *pDerivedFrom);
    void        DelTblFrmFmt( SwTableFmt* pFmt );
    SwTableFmt* FindTblFmtByName( const OUString& rName, bool bAll = false ) const;

    /** Access to frames.
    Iterate over Flys - forr Basic-Collections. */
    sal_uInt16 GetFlyCount( FlyCntType eType = FLYCNTTYPE_ALL) const;
    SwFrmFmt* GetFlyNum(sal_uInt16 nIdx, FlyCntType eType = FLYCNTTYPE_ALL);

    // Copy formats in own arrays and return them.
    SwFrmFmt  *CopyFrmFmt ( const SwFrmFmt& );
    SwCharFmt *CopyCharFmt( const SwCharFmt& );
    SwTxtFmtColl* CopyTxtColl( const SwTxtFmtColl& rColl );
    SwGrfFmtColl* CopyGrfColl( const SwGrfFmtColl& rColl );

    // Replace all styles with those from rSource.
    void ReplaceStyles( const SwDoc& rSource, bool bIncludePageStyles = true );

    // Replace all property defaults with those from rSource.
    void ReplaceDefaults( const SwDoc& rSource );

    // Replace all compatability options with those from rSource.
    void ReplaceCompatabilityOptions(const SwDoc& rSource);

    // Query if style (paragraph- / character- / frame- / page-) is used.
    bool IsUsed( const SwModify& ) const;
    bool IsUsed( const SwNumRule& ) const;

    // Set name of newly loaded document template.
    sal_uInt16 SetDocPattern( const OUString& rPatternName );

    // @return name of document template. Can be 0!
    const OUString* GetDocPattern( sal_uInt16 nPos ) const
    {
        if(nPos >= maPatternNms.size())
            return NULL;
        if(boost::is_null(maPatternNms.begin() + nPos))
            return NULL;
        return &(maPatternNms[nPos]);
    }

    // Delete all unreferenced field types.
    void GCFieldTypes();    //< impl. in docfld.cxx

    // Query / connect current document with glossary document.
    void SetGlossaryDoc( SwDoc* pDoc ) { mpGlossaryDoc = pDoc; }

    // travel over PaM Ring
    bool InsertGlossary( SwTextBlocks& rBlock, const OUString& rEntry,
                        SwPaM& rPaM, SwCrsrShell* pShell = 0);

    /** get the set of printable pages for the XRenderable API by
     evaluating the respective settings (see implementation) */
    void CalculatePagesForPrinting( const SwRootFrm& rLayout, SwRenderData &rData, const SwPrintUIOptions &rOptions, bool bIsPDFExport,
            sal_Int32 nDocPageCount );
    void UpdatePagesForPrintingWithPostItData( SwRenderData &rData, const SwPrintUIOptions &rOptions, bool bIsPDFExport,
            sal_Int32 nDocPageCount );
    void CalculatePagePairsForProspectPrinting( const SwRootFrm& rLayout, SwRenderData &rData, const SwPrintUIOptions &rOptions,
            sal_Int32 nDocPageCount );

    // PageDescriptor interface.
    sal_uInt16 GetPageDescCnt() const { return maPageDescs.size(); }
    const SwPageDesc& GetPageDesc( const sal_uInt16 i ) const { return *maPageDescs[i]; }
    SwPageDesc& GetPageDesc( sal_uInt16 i ) { return *maPageDescs[i]; }
    SwPageDesc* FindPageDescByName( const OUString& rName,
                                    sal_uInt16* pPos = 0 ) const;

    /** Copy the complete PageDesc - beyond document and "deep"!
     Optionally copying of PoolFmtId, -HlpId can be prevented. */
    void CopyPageDesc( const SwPageDesc& rSrcDesc, SwPageDesc& rDstDesc,
                        bool bCopyPoolIds = true );

    /** Copy header (with contents) from SrcFmt to DestFmt
     (can also be copied into other document). */
    void CopyHeader( const SwFrmFmt& rSrcFmt, SwFrmFmt& rDestFmt )
        { CopyPageDescHeaderFooterImpl( true, rSrcFmt, rDestFmt ); }

    /** Copy footer (with contents) from SrcFmt to DestFmt.
     (can also be copied into other document). */
    void CopyFooter( const SwFrmFmt& rSrcFmt, SwFrmFmt& rDestFmt )
        { CopyPageDescHeaderFooterImpl( false, rSrcFmt, rDestFmt ); }

    // For Reader
    SwPageDesc * GetPageDesc( const OUString & rName );
    void ChgPageDesc( const OUString & rName, const SwPageDesc& );
    void ChgPageDesc( sal_uInt16 i, const SwPageDesc& );
    bool FindPageDesc( const OUString & rName, sal_uInt16 * pFound );
    void DelPageDesc( const OUString & rName, bool bBroadcast = false);
    void DelPageDesc( sal_uInt16 i, bool bBroadcast = false );
    void PreDelPageDesc(SwPageDesc * pDel);
    sal_uInt16 MakePageDesc( const OUString &rName, const SwPageDesc* pCpy = 0,
                             bool bRegardLanguage = true,
                             bool bBroadcast = false);
    void BroadcastStyleOperation(const OUString& rName, SfxStyleFamily eFamily,
                                 sal_uInt16 nOp);

    /** The html import sometimes overwrites the
     page sizes set in the page descriptions. This function is used to
     correct this. */
    void CheckDefaultPageFmt();

    // Methods for tables/indices
    sal_uInt16 GetCurTOXMark( const SwPosition& rPos, SwTOXMarks& ) const;
    void DeleteTOXMark( const SwTOXMark* pTOXMark );
    const SwTOXMark& GotoTOXMark( const SwTOXMark& rCurTOXMark,
                                SwTOXSearch eDir, bool bInReadOnly );

    // Insert/Renew table/indes
    const SwTOXBaseSection* InsertTableOf( const SwPosition& rPos,
                                            const SwTOXBase& rTOX,
                                            const SfxItemSet* pSet = 0,
                                            bool bExpand = false );
    const SwTOXBaseSection* InsertTableOf( sal_uLong nSttNd, sal_uLong nEndNd,
                                            const SwTOXBase& rTOX,
                                            const SfxItemSet* pSet = 0 );
    const SwTOXBase* GetCurTOX( const SwPosition& rPos ) const;
    const SwAttrSet& GetTOXBaseAttrSet(const SwTOXBase& rTOX) const;

    bool DeleteTOX( const SwTOXBase& rTOXBase, bool bDelNodes = false );
    OUString GetUniqueTOXBaseName( const SwTOXType& rType,
                                   const OUString& sChkStr ) const;

    bool SetTOXBaseName(const SwTOXBase& rTOXBase, const OUString& rName);
    void SetTOXBaseProtection(const SwTOXBase& rTOXBase, bool bProtect);

    // After reading file update all tables/indices
    void SetUpdateTOX( bool bFlag = true )     { mbUpdateTOX = bFlag; }
    bool IsUpdateTOX() const                   { return mbUpdateTOX; }

    OUString        GetTOIAutoMarkURL() const {return msTOIAutoMarkURL;}
    void            SetTOIAutoMarkURL(const OUString& rSet) {msTOIAutoMarkURL = rSet;}
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

    // Manage types of tables/indices
    sal_uInt16 GetTOXTypeCount( TOXTypes eTyp ) const;
    const SwTOXType* GetTOXType( TOXTypes eTyp, sal_uInt16 nId ) const;
    bool DeleteTOXType( TOXTypes eTyp, sal_uInt16 nId );
    const SwTOXType* InsertTOXType( const SwTOXType& rTyp );
    const SwTOXTypes& GetTOXTypes() const { return *mpTOXTypes; }

    const SwTOXBase*    GetDefaultTOXBase( TOXTypes eTyp, bool bCreate = false );
    void                SetDefaultTOXBase(const SwTOXBase& rBase);

    // Key for management of index.
    sal_uInt16 GetTOIKeys( SwTOIKeyType eTyp, std::vector<OUString>& rArr ) const;

    // Sort table text.
    bool SortTbl(const SwSelBoxes& rBoxes, const SwSortOptions&);
    bool SortText(const SwPaM&, const SwSortOptions&);

    // Correct the SwPosition-Objects that are registered with the document
    // e. g. ::com::sun::star::text::Bookmarks or tables/indices.
    // If bMoveCrsr is set move Crsr too.

    // Set everything in rOldNode on rNewPos + Offset.
    void CorrAbs(
        const SwNodeIndex& rOldNode,
        const SwPosition& rNewPos,
        const sal_Int32 nOffset = 0,
        bool bMoveCrsr = false );

    // Set everything in the range of [rStartNode, rEndNode] to rNewPos.
    void CorrAbs(
        const SwNodeIndex& rStartNode,
        const SwNodeIndex& rEndNode,
        const SwPosition& rNewPos,
        bool bMoveCrsr = false );

    // Set everything in this range from rRange to rNewPos.
    void CorrAbs(
        const SwPaM& rRange,
        const SwPosition& rNewPos,
        bool bMoveCrsr = false );

    // Set everything in rOldNode to relative Pos.
    void CorrRel(
        const SwNodeIndex& rOldNode,
        const SwPosition& rNewPos,
        const sal_Int32 nOffset = 0,
        bool bMoveCrsr = false );

    // Query / set rules for Outline.
    inline SwNumRule* GetOutlineNumRule() const
    {
        return mpOutlineRule;
    }
    void SetOutlineNumRule( const SwNumRule& rRule );
    void PropagateOutlineRule();

    // Outline - promote / demote.
    bool OutlineUpDown( const SwPaM& rPam, short nOffset = 1 );

    // Ountline - move up / move down.
    bool MoveOutlinePara( const SwPaM& rPam, short nOffset = 1);

    bool GotoOutline( SwPosition& rPos, const OUString& rName ) const;

    /** Accept changes of outline styles for OUtlineRule.
     re-use unused 3rd parameter
     Optional parameter <bResetIndentAttrs> - default value false:
      If <bResetIndentAttrs> equals true, the indent attributes "before text"
      and "first line indent" are additionally reset at the provided PaM, if
      the list style makes use of the new list level attributes.
     Parameters <bCreateNewList> and <sContinuedListId>:
      <bCreateNewList> indicates, if a new list is created by applying the given list style.
      If <bCreateNewList> equals false, <sContinuedListId> may contain the
      list Id of a list, which has to be continued by applying the given list style */
    void SetNumRule( const SwPaM&,
                     const SwNumRule&,
                     const bool bCreateNewList,
                     const OUString& sContinuedListId = OUString(),
                     bool bSetItem = true,
                     const bool bResetIndentAttrs = false );
    void SetCounted( const SwPaM&, bool bCounted);

    void MakeUniqueNumRules(const SwPaM & rPaM);

    void SetNumRuleStart( const SwPosition& rPos, bool bFlag = true );
    void SetNodeNumStart( const SwPosition& rPos, sal_uInt16 nStt );

    SwNumRule* GetNumRuleAtPos( const SwPosition& rPos ) const;

    const SwNumRuleTbl& GetNumRuleTbl() const { return *mpNumRuleTbl; }

    /**
       Add numbering rule to document.

       @param pRule    rule to add
    */
    void AddNumRule(SwNumRule * pRule);

    // add optional parameter <eDefaultNumberFormatPositionAndSpaceMode>
    sal_uInt16 MakeNumRule( const OUString &rName,
        const SwNumRule* pCpy = 0,
        bool bBroadcast = false,
        const SvxNumberFormat::SvxNumPositionAndSpaceMode eDefaultNumberFormatPositionAndSpaceMode =
            SvxNumberFormat::LABEL_WIDTH_AND_POSITION );
    sal_uInt16 FindNumRule( const OUString& rName ) const;
    SwNumRule* FindNumRulePtr( const OUString& rName ) const;

    // Deletion only possible if ::com::sun::star::chaos::Rule is not used!
    bool RenameNumRule(const OUString & aOldName, const OUString & aNewName,
                           bool bBroadcast = false);
    bool DelNumRule( const OUString& rName, bool bBroadCast = false );
    OUString GetUniqueNumRuleName( const OUString* pChkStr = 0, bool bAutoNum = true ) const;

    void UpdateNumRule();   // Update all invalids.
    void ChgNumRuleFmts( const SwNumRule& rRule );
    bool ReplaceNumRule( const SwPosition& rPos, const OUString& rOldRule,
                        const OUString& rNewRule );

    // Goto next/previous on same level.
    bool GotoNextNum( SwPosition&, bool bOverUpper = true,
                        sal_uInt8* pUpper = 0, sal_uInt8* pLower = 0 );
    bool GotoPrevNum( SwPosition&, bool bOverUpper = true,
                        sal_uInt8* pUpper = 0, sal_uInt8* pLower = 0 );

    /** Searches for a text node with a numbering rule.

       add optional parameter <bInvestigateStartNode>
       add output parameter <sListId>

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
                                    OUString& sListId,
                                    const bool bInvestigateStartNode = false );

    // Paragraphs without numbering but with indents.
    bool NoNum( const SwPaM& );

    // Delete, splitting of numbering list.
    void DelNumRules( const SwPaM& );

    // Invalidates all numrules
    void InvalidateNumRules();

    bool NumUpDown( const SwPaM&, bool bDown = true );

    /** Move selected paragraphes (not only numberings)
     according to offsets. (if negative: go to doc start). */
    bool MoveParagraph( const SwPaM&, long nOffset = 1, bool bIsOutlMv = false );

    bool NumOrNoNum( const SwNodeIndex& rIdx, bool bDel = false);

    void StopNumRuleAnimations( OutputDevice* );

    /** Insert new table at position @param rPos (will be inserted before Node!).
     For AutoFormat at input: columns have to be set at predefined width.
     The array holds the positions of the columns (not their widths).
     new @param bCalledFromShell:
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
                                const std::vector<sal_uInt16> *pColArr = 0,
                                bool bCalledFromShell = false,
                                bool bNewModel = true );

    // If index is in a table, return TableNode, else 0.
                 SwTableNode* IsIdxInTbl( const SwNodeIndex& rIdx );
    inline const SwTableNode* IsIdxInTbl( const SwNodeIndex& rIdx ) const;

    // Create a balanced table out of the selected range.
    const SwTable* TextToTable( const SwInsertTableOptions& rInsTblOpts, // HEADLINE_NO_BORDER,
                                const SwPaM& rRange, sal_Unicode cCh,
                                short eAdjust,
                                const SwTableAutoFmt* = 0 );

    // text to table conversion - API support
    const SwTable* TextToTable( const std::vector< std::vector<SwNodeRange> >& rTableNodes );

    bool TableToText( const SwTableNode* pTblNd, sal_Unicode cCh );

    // Create columns / rows in table.
    bool InsertCol( const SwCursor& rCursor,
                    sal_uInt16 nCnt = 1, bool bBehind = true );
    bool InsertCol( const SwSelBoxes& rBoxes,
                    sal_uInt16 nCnt = 1, bool bBehind = true );
    bool InsertRow( const SwCursor& rCursor,
                    sal_uInt16 nCnt = 1, bool bBehind = true );
    bool InsertRow( const SwSelBoxes& rBoxes,
                    sal_uInt16 nCnt = 1, bool bBehind = true );

    // Delete Columns/Rows in table.
    bool DeleteRowCol( const SwSelBoxes& rBoxes, bool bColumn = false );
    bool DeleteRow( const SwCursor& rCursor );
    bool DeleteCol( const SwCursor& rCursor );

    // Split / concatenate boxes in table.
    bool SplitTbl( const SwSelBoxes& rBoxes, bool bVert = true,
                       sal_uInt16 nCnt = 1, bool bSameHeight = false );

    // @return enum TableMergeErr.
    sal_uInt16 MergeTbl( SwPaM& rPam );
    OUString GetUniqueTblName() const;
    bool IsInsTblFormatNum() const;
    bool IsInsTblChangeNumFormat() const;
    bool IsInsTblAlignNum() const;

    // From FEShell (for Undo and BModified).
    void GetTabCols( SwTabCols &rFill, const SwCursor* pCrsr,
                    const SwCellFrm* pBoxFrm = 0 ) const;
    void SetTabCols( const SwTabCols &rNew, bool bCurRowOnly,
                    const SwCursor* pCrsr, const SwCellFrm* pBoxFrm = 0 );
    void GetTabRows( SwTabCols &rFill, const SwCursor* pCrsr,
                    const SwCellFrm* pBoxFrm = 0 ) const;
    void SetTabRows( const SwTabCols &rNew, bool bCurColOnly, const SwCursor* pCrsr,
                     const SwCellFrm* pBoxFrm = 0 );

    // Direct access for UNO.
    void SetTabCols(SwTable& rTab, const SwTabCols &rNew, const SwTabCols &rOld,
                                    const SwTableBox *pStart, bool bCurRowOnly);

    void SetRowsToRepeat( SwTable &rTable, sal_uInt16 nSet );

    // AutoFormat for table/table selection.
    bool SetTableAutoFmt( const SwSelBoxes& rBoxes, const SwTableAutoFmt& rNew );

    // Query attributes.
    bool GetTableAutoFmt( const SwSelBoxes& rBoxes, SwTableAutoFmt& rGet );

    void AppendUndoForInsertFromDB( const SwPaM& rPam, bool bIsTable );

    bool SetColRowWidthHeight( SwTableBox& rAktBox, sal_uInt16 eType,
                                SwTwips nAbsDiff, SwTwips nRelDiff );
    SwTableBoxFmt* MakeTableBoxFmt();
    SwTableLineFmt* MakeTableLineFmt();

    // Check if box has numerical valule. Change format of box if required.
    void ChkBoxNumFmt( SwTableBox& rAktBox, bool bCallUpdate );
    void SetTblBoxFormulaAttrs( SwTableBox& rBox, const SfxItemSet& rSet );
    void ClearBoxNumAttrs( const SwNodeIndex& rNode );
    void ClearLineNumAttrs( SwPosition & rPos );

    bool InsCopyOfTbl( SwPosition& rInsPos, const SwSelBoxes& rBoxes,
                        const SwTable* pCpyTbl = 0, bool bCpyName = false,
                        bool bCorrPos = false );

    bool UnProtectCells( const OUString& rTblName );
    bool UnProtectCells( const SwSelBoxes& rBoxes );
    bool UnProtectTbls( const SwPaM& rPam );
    bool HasTblAnyProtection( const SwPosition* pPos,
                              const OUString* pTblName = 0,
                              bool* pFullTblProtection = 0 );

    // Split table at baseline position, i.e. create a new table.
    bool SplitTable( const SwPosition& rPos, sal_uInt16 eMode = 0,
                        bool bCalcNewSize = false );

    /** And vice versa: rPos must be in the table that remains. The flag indicates
     whether the current table is merged with the one before or behind it. */
    bool MergeTable( const SwPosition& rPos, bool bWithPrev = true,
                        sal_uInt16 nMode = 0 );

    // Make charts of given table update.
    void UpdateCharts( const OUString &rName ) const;

    // Update all charts, for that exists any table.
    void UpdateAllCharts()          { DoUpdateAllCharts(); }

    // Table is renamed and refreshes charts.
    void SetTableName( SwFrmFmt& rTblFmt, const OUString &rNewName );

    // @return the reference in document that is set for name.
    const SwFmtRefMark* GetRefMark( const OUString& rName ) const;

    // @return RefMark via index - for UNO.
    const SwFmtRefMark* GetRefMark( sal_uInt16 nIndex ) const;

    /** @return names of all references that are set in document.
     If array pointer is 0 return only whether a RefMark is set in document. */
    sal_uInt16 GetRefMarks( std::vector<OUString>* = 0 ) const;

    // Insert label. If a FlyFormat is created, return it.
    SwFlyFrmFmt* InsertLabel( const SwLabelType eType, const OUString &rTxt, const OUString& rSeparator,
                    const OUString& rNumberingSeparator,
                    const bool bBefore, const sal_uInt16 nId, const sal_uLong nIdx,
                    const OUString& rCharacterStyle,
                    const bool bCpyBrd = true );
    SwFlyFrmFmt* InsertDrawLabel(
        const OUString &rTxt, const OUString& rSeparator, const OUString& rNumberSeparator,
        const sal_uInt16 nId, const OUString& rCharacterStyle, SdrObject& rObj );

    // Query attribute pool.
    const SwAttrPool& GetAttrPool() const   { return *mpAttrPool; }
          SwAttrPool& GetAttrPool()         { return *mpAttrPool; }

    // Search an EditShell or, if appropriate, a SwViewShell via layout.
    SwEditShell* GetEditShell( SwViewShell** ppSh = 0 ) const;
    ::sw::IShellCursorSupplier * GetIShellCursorSupplier();

    // OLE 2.0-notification.
    inline       void  SetOle2Link(const Link& rLink) {maOle2Link = rLink;}
    inline const Link& GetOle2Link() const {return maOle2Link;}

    // insert section (the ODF kind of section, not the nodesarray kind)
    SwSection * InsertSwSection(SwPaM const& rRange, SwSectionData &,
            SwTOXBase const*const pTOXBase = 0,
            SfxItemSet const*const pAttr = 0, bool const bUpdate = true);
    sal_uInt16 IsInsRegionAvailable( const SwPaM& rRange,
                                const SwNode** ppSttNd = 0 ) const;
    SwSection* GetCurrSection( const SwPosition& rPos ) const;
    SwSectionFmts& GetSections() { return *mpSectionFmtTbl; }
    const SwSectionFmts& GetSections() const { return *mpSectionFmtTbl; }
    SwSectionFmt *MakeSectionFmt( SwSectionFmt *pDerivedFrom );
    void DelSectionFmt( SwSectionFmt *pFmt, bool bDelNodes = false );
    void UpdateSection(sal_uInt16 const nSect, SwSectionData &,
            SfxItemSet const*const = 0, bool const bPreventLinkUpdate = false);
    OUString GetUniqueSectionName( const OUString* pChkStr = 0 ) const;

    /* @@@MAINTAINABILITY-HORROR@@@
       The model should not have anything to do with a shell.
       Unnecessary compile/link time dependency.
    */

    // Pointer to SfxDocShell from Doc. Can be 0!!
          SwDocShell* GetDocShell()         { return mpDocShell; }
    const SwDocShell* GetDocShell() const   { return mpDocShell; }
    void SetDocShell( SwDocShell* pDSh );

    /** in case during copying of embedded object a new shell is created,
     it should be set here and cleaned later */
    void SetTmpDocShell( SfxObjectShellLock rLock )    { mxTmpDocShell = rLock; }
    SfxObjectShellLock GetTmpDocShell()    { return mxTmpDocShell; }

    // For Autotexts? (text modules) They have only one SVPersist at their disposal.
    SfxObjectShell* GetPersist() const;

    // Pointer to storage of SfxDocShells. Can be 0!!!
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > GetDocStorage();

    // Query / set flag indicating if document is loaded asynchronously at this moment.
    bool IsInLoadAsynchron() const             { return mbInLoadAsynchron; }
    void SetInLoadAsynchron( bool bFlag )       { mbInLoadAsynchron = bFlag; }

    bool SelectServerObj( const OUString& rStr, SwPaM*& rpPam,
                            SwNodeRange*& rpRange ) const;

    // For Drag&Move: (e.g. allow "moving" of RefMarks)
    bool IsCopyIsMove() const              { return mbCopyIsMove; }
    void SetCopyIsMove( bool bFlag )        { mbCopyIsMove = bFlag; }

    SwDrawContact* GroupSelection( SdrView& );
    void UnGroupSelection( SdrView& );
    bool DeleteSelection( SwDrawView& );

    // Invalidates OnlineSpell-WrongLists.
    void SpellItAgainSam( bool bInvalid, bool bOnlyWrong, bool bSmartTags );
    void InvalidateAutoCompleteFlag();

    void SetCalcFieldValueHdl(Outliner* pOutliner);

    // Query if ::com::sun::star::util::URL was visited.
    // Query via Doc, if only a ::com::sun::star::text::Bookmark has been given.
    // In this case the document name has to be set in front.
    bool IsVisitedURL( const OUString& rURL ) const;

    // Save current values for automatic registration of exceptions in Autocorrection.
    void SetAutoCorrExceptWord( SwAutoCorrExceptWord* pNew );
    SwAutoCorrExceptWord* GetAutoCorrExceptWord()       { return mpACEWord; }

    const SwFmtINetFmt* FindINetAttr( const OUString& rName ) const;

    // Call into intransparent Basic; expect possible Return String.
    bool ExecMacro( const SvxMacro& rMacro, OUString* pRet = 0, SbxArray* pArgs = 0 );

    // Call into intransparent Basic / JavaScript.
    sal_uInt16 CallEvent( sal_uInt16 nEvent, const SwCallMouseEvent& rCallEvent,
                        bool bChkPtr = false, SbxArray* pArgs = 0,
                        const Link* pCallBack = 0 );

    /** Adjust left margin via object bar (similar to adjustment of numerations).
     One can either change the margin "by" adding or substracting a given
     offset or set it "to" this position (bModulus = true). */
    void MoveLeftMargin( const SwPaM& rPam, bool bRight = true,
                        bool bModulus = true );

    // Query NumberFormatter.
    inline       SvNumberFormatter* GetNumberFormatter( bool bCreate = true );
    inline const SvNumberFormatter* GetNumberFormatter( bool bCreate = true ) const;

    bool HasInvisibleContent() const;
    // delete invisible content, like hidden sections and paragraphs
    bool RemoveInvisibleContent();
    // restore the invisible content if it's available on the undo stack
    bool RestoreInvisibleContent();

    bool ConvertFieldsToText();

    // Create sub-documents according to given collection.
    // If no collection is given, use chapter styles for 1st level.
    bool GenerateGlobalDoc( const OUString& rPath,
                                const SwTxtFmtColl* pSplitColl = 0 );
    bool GenerateGlobalDoc( const OUString& rPath, int nOutlineLevel = 0 );
    bool GenerateHTMLDoc( const OUString& rPath,
                                const SwTxtFmtColl* pSplitColl = 0 );
    bool GenerateHTMLDoc( const OUString& rPath, int nOutlineLevel = 0 );

    //  Compare two documents.
    long CompareDoc( const SwDoc& rDoc );

    // Merge two documents.
    long MergeDoc( const SwDoc& rDoc );

    /** Set comment-text for Redline. It then comes in via AppendRedLine.
     Used by AutoFormat. 0-pointer resets mode.
     Sequence number is for conjoining of Redlines by the UI. */
    void SetAutoFmtRedlineComment( const OUString* pTxt, sal_uInt16 nSeqNo = 0 );

    bool IsAutoFmtRedline() const           { return mbIsAutoFmtRedline; }
    void SetAutoFmtRedline( bool bFlag )    { mbIsAutoFmtRedline = bFlag; }

    // For AutoFormat: with Undo/Redlining.
    void SetTxtFmtCollByAutoFmt( const SwPosition& rPos, sal_uInt16 nPoolId,
                                const SfxItemSet* pSet = 0 );
    void SetFmtItemByAutoFmt( const SwPaM& rPam, const SfxItemSet& );

    // Only for SW-textbloxks! Does not pay any attention to layout!
    void ClearDoc();        // Deletes all content!

    // Query /set data for PagePreview.
    const SwPagePreviewPrtData* GetPreviewPrtData() const { return mpPgPViewPrtData; }

    // If pointer == 0 destroy pointer in document.
    // Else copy object.
    // Pointer is not transferred to ownership by document!
    void SetPreviewPrtData( const SwPagePreviewPrtData* pData );

    /** update all modified OLE-Objects. The modification is called over the
     StarOne - Interface */
    void SetOLEObjModified()
    {   if( GetCurrentViewShell() ) maOLEModifiedTimer.Start(); }

    // Uno - Interfaces
    const SwUnoCrsrTbl& GetUnoCrsrTbl() const       { return *mpUnoCrsrTbl; }
    SwUnoCrsr* CreateUnoCrsr( const SwPosition& rPos, bool bTblCrsr = false );

    // FeShell - Interfaces
    // !!! These assume always an existing layout !!!
    bool ChgAnchor( const SdrMarkList& _rMrkList,
                        RndStdIds _eAnchorType,
                        const bool _bSameOnly,
                        const bool _bPosCorr );

    void SetRowHeight( const SwCursor& rCursor, const SwFmtFrmSize &rNew );
    void GetRowHeight( const SwCursor& rCursor, SwFmtFrmSize *& rpSz ) const;
    void SetRowSplit( const SwCursor& rCursor, const SwFmtRowSplit &rNew );
    void GetRowSplit( const SwCursor& rCursor, SwFmtRowSplit *& rpSz ) const;
    bool BalanceRowHeight( const SwCursor& rCursor, bool bTstOnly = true );
    void SetRowBackground( const SwCursor& rCursor, const SvxBrushItem &rNew );
    bool GetRowBackground( const SwCursor& rCursor, SvxBrushItem &rToFill ) const;
    void SetTabBorders( const SwCursor& rCursor, const SfxItemSet& rSet );
    void SetTabLineStyle( const SwCursor& rCursor,
                          const Color* pColor, bool bSetLine,
                          const editeng::SvxBorderLine* pBorderLine );
    void GetTabBorders( const SwCursor& rCursor, SfxItemSet& rSet ) const;
    void SetBoxAttr( const SwCursor& rCursor, const SfxPoolItem &rNew );
    /**
    Retrieves a box attribute from the given cursor.

    @return Whether the property is set over the current box selection.

    @remarks A property is 'set' if it's set to the same value over all boxes in the current selection.
    The property value is retrieved from the first box in the current selection. It is then compared to
    the values of the same property over any other boxes in the selection; if any value is different from
    that of the first box, the property is unset (and sal_False is returned).
    */
    bool GetBoxAttr( const SwCursor& rCursor, SfxPoolItem &rToFill ) const;
    void SetBoxAlign( const SwCursor& rCursor, sal_uInt16 nAlign );
    sal_uInt16 GetBoxAlign( const SwCursor& rCursor ) const;
    void AdjustCellWidth( const SwCursor& rCursor, bool bBalance = false );

    int Chainable( const SwFrmFmt &rSource, const SwFrmFmt &rDest );
    int Chain( SwFrmFmt &rSource, const SwFrmFmt &rDest );
    void Unchain( SwFrmFmt &rFmt );

    // For Copy/Move from FrmShell.
    SdrObject* CloneSdrObj( const SdrObject&, bool bMoveWithinDoc = false,
                            bool bInsInPage = true );

    // FeShell - Interface end

    // Interface for TextInputData - for text input of Chinese and Japanese.
    SwExtTextInput* CreateExtTextInput( const SwPaM& rPam );
    void DeleteExtTextInput( SwExtTextInput* pDel );
    SwExtTextInput* GetExtTextInput( const SwNode& rNd,
                                sal_Int32 nCntntPos = -1) const;
    SwExtTextInput* GetExtTextInput() const;

    // Interface for access to AutoComplete-List.
    static SwAutoCompleteWord& GetAutoCompleteWords() { return *mpACmpltWords; }

    bool ContainsMSVBasic() const          { return mbContains_MSVBasic; }
    void SetContainsMSVBasic( bool bFlag )  { mbContains_MSVBasic = bFlag; }

    // Interface for the list of Ruby - texts/attributes
    sal_uInt16 FillRubyList( const SwPaM& rPam, SwRubyList& rList,
                        sal_uInt16 nMode );
    sal_uInt16 SetRubyList( const SwPaM& rPam, const SwRubyList& rList,
                        sal_uInt16 nMode );

    void ReadLayoutCache( SvStream& rStream );
    void WriteLayoutCache( SvStream& rStream );
    SwLayoutCache* GetLayoutCache() const { return mpLayoutCache; }

    /** Checks if any of the text node contains hidden characters.
        Used for optimization. Changing the view option 'view hidden text'
        has to trigger a reformatting only if some of the text is hidden.
    */
    bool ContainsHiddenChars() const;

    // call back for API wrapper
    SwModify*   GetUnoCallBack() const;

    IGrammarContact* getGrammarContact() const { return mpGrammarContact; }

    /** Marks/Unmarks a list level of a certain list

        levels of a certain lists are marked now

        @param sListId    list Id of the list whose level has to be marked/unmarked
        @param nListLevel level to mark
        @param bValue     - true  mark the level
                          - false unmark the level
    */
    void MarkListLevel( const OUString& sListId,
                        const int nListLevel,
                        const bool bValue );

    /** Marks/Unmarks a list level of a certain list

        levels of a certain lists are marked now

        @param rList      list whose level has to be marked/unmarked
        @param nListLevel level to mark
        @param bValue     - true  mark the level
                          - false unmark the level
     */
    void MarkListLevel( SwList& rList,
                        const int nListLevel,
                        const bool bValue );

    // Change a format undoable.
    void ChgFmt(SwFmt & rFmt, const SfxItemSet & rSet);

    void RenameFmt(SwFmt & rFmt, const OUString & sNewName,
                   bool bBroadcast = false);

    // Change a TOX undoable.
    void ChgTOX(SwTOXBase & rTOX, const SwTOXBase & rNew);

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
    OUString GetPaMDescr(const SwPaM & rPaM) const;

    bool IsFirstOfNumRuleAtPos( const SwPosition & rPos );

    // access methods for XForms model(s)

    // access container for XForms model; will be NULL if !isXForms()
    com::sun::star::uno::Reference<com::sun::star::container::XNameContainer>
        getXForms() const;

    com::sun::star::uno::Reference< com::sun::star::linguistic2::XProofreadingIterator > GetGCIterator() const;

    // #i31958# is this an XForms document?
    bool isXForms() const;

    // #i31958# initialize XForms models; turn this into an XForms document
    void initXForms( bool bCreateDefaultModel );

    // #i113606# for disposing XForms
    void disposeXForms( );

    //Update all the page masters
    void SetDefaultPageMode(bool bSquaredPageMode);
    bool IsSquaredPageMode() const;

    void Setn32DummyCompatabilityOptions1( const sal_uInt32 CompatabilityOptions1 )
    {
        mn32DummyCompatabilityOptions1 = CompatabilityOptions1;
    }
    sal_uInt32 Getn32DummyCompatabilityOptions1()
    {
        return mn32DummyCompatabilityOptions1;
    }
    void Setn32DummyCompatabilityOptions2( const sal_uInt32 CompatabilityOptions2 )
    {
        mn32DummyCompatabilityOptions2 = CompatabilityOptions2;
    }
    sal_uInt32 Getn32DummyCompatabilityOptions2()
    {
        return mn32DummyCompatabilityOptions2;
    }
    com::sun::star::uno::Reference< com::sun::star::script::vba::XVBAEventProcessor > GetVbaEventProcessor();
    void SetVBATemplateToProjectCache( com::sun::star::uno::Reference< com::sun::star::container::XNameContainer >& xCache ) { m_xTemplateToProjectCache = xCache; };
        com::sun::star::uno::Reference< com::sun::star::container::XNameContainer > GetVBATemplateToProjectCache() { return m_xTemplateToProjectCache; };
    ::sfx2::IXmlIdRegistry& GetXmlIdRegistry();
    ::sw::MetaFieldManager & GetMetaFieldManager();
    ::sw::UndoManager      & GetUndoManager();
    ::sw::UndoManager const& GetUndoManager() const;
    ::sw::DocumentSettingManager      & GetDocumentSettingManager();
    ::sw::DocumentSettingManager const& GetDocumentSettingManager() const;
    SfxObjectShell* CreateCopy(bool bCallInitNew) const;

    /**
     * Dumps the entire nodes structure to the given destination (file nodes.xml in the current directory by default)
     * @since 3.5
     */
    void dumpAsXml( xmlTextWriterPtr writer = NULL );

private:
    // Copies master header to left / first one, if necessary - used by ChgPageDesc().
    void CopyMasterHeader(const SwPageDesc &rChged, const SwFmtHeader &rHead, SwPageDesc *pDesc, bool bLeft, bool bFirst);
    // Copies master footer to left / first one, if necessary - used by ChgPageDesc().
    void CopyMasterFooter(const SwPageDesc &rChged, const SwFmtFooter &rFoot, SwPageDesc *pDesc, bool bLeft, bool bFirst);

    /** continue computing a chunk of document statistics
      * \param nChars  number of characters to count before exiting
      * \param bFields if stat. fields should be updated
      *
      * returns false when there is no more to calculate
      */
    bool IncrementalDocStatCalculate(long nChars, bool bFields = true);

    // Our own 'StatsUpdateTimer' calls the following method
    DECL_LINK( DoIdleStatsUpdate, Timer * );
};

// This method is called in Dtor of SwDoc and deletes cache of ContourObjects.
void ClrContourCache();

inline const SwTableNode* SwDoc::IsIdxInTbl( const SwNodeIndex& rIdx ) const
{
    return ((SwDoc*)this)->IsIdxInTbl( rIdx );
}

inline SvNumberFormatter* SwDoc::GetNumberFormatter( bool bCreate )
{
    if( bCreate && !mpNumberFormatter )
        _CreateNumberFormatter();
    return mpNumberFormatter;
}

inline const SvNumberFormatter* SwDoc::GetNumberFormatter( bool bCreate ) const
{
    return ((SwDoc*)this)->GetNumberFormatter( bCreate );
}

inline void SwDoc::SetOLEPrtNotifyPending( bool bSet )
{
    mbOLEPrtNotifyPending = bSet;
    if( !bSet )
        mbAllOLENotify = false;
}

// namespace <docfunc> for functions and procedures working on a Writer document.
namespace docfunc
{
    /** method to check, if given Writer document contains at least one drawing object

        @author OD

        @param p_rDoc
        input parameter - reference to the Writer document, which is investigated.
    */
    bool ExistsDrawObjs( SwDoc& p_rDoc );

    /** method to check, if given Writer document contains only drawing objects,
        which are completely on its page.

        @author OD

        @param p_rDoc
        input parameter - reference to the Writer document, which is investigated.
    */
    bool AllDrawObjsOnPage( SwDoc& p_rDoc );

    /** method to check, if the outline style has to written as a normal list style

        #i69627#
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

bool sw_GetPostIts( IDocumentFieldsAccess* pIDFA, _SetGetExpFlds * pSrtLst );

#endif  //_DOC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
