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
#include <IDocumentMarkAccess.hxx>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <vcl/timer.hxx>
#include <vcl/idle.hxx>
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
#include <sfx2/objsh.hxx>
#include <svl/style.hxx>
#include <editeng/numitem.hxx>
#include <com/sun/star/linguistic2/XProofreadingIterator.hpp>
#include <com/sun/star/script/vba/XVBAEventProcessor.hpp>
#include <tox.hxx>
#include <frmfmt.hxx>
#include <charfmt.hxx>
#include <docary.hxx>

#include <svtools/embedhlp.hxx>

#include <memory>
#include <set>
#include <unordered_map>
#include <vector>

namespace editeng { class SvxBorderLine; }

class SvxForbiddenCharactersTable;
class SwExtTextInput;
class DateTime;
class EditFieldInfo;
class JobSetup;
class Color;
class Outliner;
class OutputDevice;
class Point;
class SbxArray;
class SwDrawModel;
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
class SwCellFrame;
class SwCellStyleTable;
class SwCharFormat;
class SwCharFormats;
class SwConditionTextFormatColl;
class SwCursorShell;
class SwCursor;
class SwDBNameInfField;
class SwDocShell;
class SwDocUpdateField;
class SwDrawFrameFormat;
class SwDrawView;
class SwEditShell;
class SwFieldType;
class SwField;
class SwTextField;
class SwFieldTypes;
class SwFlyFrameFormat;
class SwFormatsBase;
class SwFormat;
class SwFormatINetFormat;
class SwFormatRefMark;
class SwFrameFormat;
class SwFrameFormats;
class SwFootnoteIdxs;
class SwFootnoteInfo;
class SwEndNoteInfo;
class GraphicObject;
class SwGrfFormatColl;
class SwGrfFormatColls;
class SwLineNumberInfo;
class SwDBManager;
class SwNoTextNode;
class SwNodeIndex;
class SwNodeRange;
class SwNodes;
class SwNumRule;
class SwNumRuleTable;
class SwPageDesc;
class SwPagePreviewPrtData;
class SwRangeRedline;
class SwRedlineTable;
class SwExtraRedlineTable;
class SwRootFrame;
class SwRubyList;
class SwRubyListEntry;
class SwSectionFormat;
class SwSectionFormats;
class SwSectionData;
class SwSelBoxes;
class SwTableAutoFormatTable;
class SwTOXBaseSection;
class SwTOXTypes;
class SwTabCols;
class SwTable;
class SwTableAutoFormat;
class SwTableBox;
class SwTableBoxFormat;
class SwTableFormat;
class SwTableLineFormat;
class SwTableNode;
class SwTextBlocks;
class SwTextFormatColl;
class SwTextFormatColls;
class SwURLStateChanged;
class SwUnoCursor;
class SwViewShell;
class SetGetExpField;
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
class SwPageFrame;
class SwViewOption;
class SwList;
class IDocumentUndoRedo;
class IDocumentSettingAccess;
class IDocumentDeviceAccess;
class IDocumentDrawModelAccess;
class IDocumentChartDataProviderAccess;
class IDocumentTimerAccess;
class IDocumentLinksAdministration;
class IDocumentListItems;
class IDocumentListsAccess;
class IDocumentOutlineNodes;
class IDocumentContentOperations;
class IDocumentRedlineAccess;
class IDocumentStatistics;
class IDocumentState;
class IDocumentLayoutAccess;
class IDocumentStylePoolAccess;
class IDocumentExternalData;
class SetGetExpFields;

namespace sw { namespace mark {
    class MarkManager;
}}
namespace sw {
    class MetaFieldManager;
    class UndoManager;
    class IShellCursorSupplier;
    class DocumentSettingManager;
    class DocumentDeviceManager;
    class DocumentDrawModelManager;
    class DocumentChartDataProviderManager;
    class DocumentTimerManager;
    class DocumentLinksAdministrationManager;
    class DocumentListItemsManager;
    class DocumentListsManager;
    class DocumentOutlineNodesManager;
    class DocumentContentOperationsManager;
    class DocumentRedlineManager;
    class DocumentFieldsManager;
    class DocumentStatisticsManager;
    class DocumentStateManager;
    class DocumentLayoutManager;
    class DocumentStylePoolManager;
    class DocumentExternalDataManager;
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

typedef std::vector<std::unique_ptr<SwPageDesc>> SwPageDescs;

void SetAllScriptItem( SfxItemSet& rSet, const SfxPoolItem& rItem );

// global function to start grammar checking in the document
void StartGrammarChecking( SwDoc &rDoc );

// Represents the model of a Writer document.
class SW_DLLPUBLIC SwDoc :
    public IInterface
{
    friend class ::sw::DocumentContentOperationsManager;

    friend void InitCore();
    friend void FinitCore();

    // private Member
    std::unique_ptr<SwNodes> m_pNodes;    //< document content (Nodes Array)
    SwAttrPool*              mpAttrPool;  //< the attribute pool
    SwPageDescs              m_PageDescs; //< PageDescriptors
    Link<bool,void>          maOle2Link;  //< OLE 2.0-notification
    /* @@@MAINTAINABILITY-HORROR@@@
       Timer should not be members of the model
    */
    Idle       maOLEModifiedIdle;      //< Timer for update modified OLE-Objects
    SwDBData    maDBData;                //< database descriptor
    OUString    msTOIAutoMarkURL;        //< URL of table of index AutoMark file
    std::vector<OUString> m_PatternNames; //< Array for names of document-templates
    css::uno::Reference<css::container::XNameContainer>
        mxXForms;                        //< container with XForms models
    mutable css::uno::Reference< css::linguistic2::XProofreadingIterator > m_xGCIterator;

    const std::unique_ptr< ::sw::mark::MarkManager> mpMarkManager;
    const std::unique_ptr< ::sw::MetaFieldManager > m_pMetaFieldManager;
    const std::unique_ptr< ::sw::DocumentDrawModelManager > m_pDocumentDrawModelManager;
    const std::unique_ptr< ::sw::DocumentRedlineManager > m_pDocumentRedlineManager;
    const std::unique_ptr< ::sw::DocumentStateManager > m_pDocumentStateManager;
    const std::unique_ptr< ::sw::UndoManager > m_pUndoManager;
    const std::unique_ptr< ::sw::DocumentSettingManager > m_pDocumentSettingManager;
    const std::unique_ptr< ::sw::DocumentChartDataProviderManager > m_pDocumentChartDataProviderManager;
    std::unique_ptr< ::sw::DocumentDeviceManager > m_pDeviceAccess;
    const std::unique_ptr< ::sw::DocumentTimerManager > m_pDocumentTimerManager;
    const std::unique_ptr< ::sw::DocumentLinksAdministrationManager > m_pDocumentLinksAdministrationManager;
    const std::unique_ptr< ::sw::DocumentListItemsManager > m_pDocumentListItemsManager;
    const std::unique_ptr< ::sw::DocumentListsManager > m_pDocumentListsManager;
    const std::unique_ptr< ::sw::DocumentOutlineNodesManager > m_pDocumentOutlineNodesManager;
    const std::unique_ptr< ::sw::DocumentContentOperationsManager > m_pDocumentContentOperationsManager;
    const std::unique_ptr< ::sw::DocumentFieldsManager > m_pDocumentFieldsManager;
    const std::unique_ptr< ::sw::DocumentStatisticsManager > m_pDocumentStatisticsManager;
    const std::unique_ptr< ::sw::DocumentLayoutManager > m_pDocumentLayoutManager;
    const std::unique_ptr< ::sw::DocumentStylePoolManager > m_pDocumentStylePoolManager;
    const std::unique_ptr< ::sw::DocumentExternalDataManager > m_pDocumentExternalDataManager;

    // Pointer
    SwFrameFormat        *mpDfltFrameFormat;       //< Default formats.
    SwFrameFormat        *mpEmptyPageFormat;     //< Format for the default empty page
    SwFrameFormat        *mpColumnContFormat;    //< Format for column container
    SwCharFormat       *mpDfltCharFormat;
    SwTextFormatColl    *mpDfltTextFormatColl;   //< Defaultformatcollections
    SwGrfFormatColl    *mpDfltGrfFormatColl;

    SwFrameFormats       *mpFrameFormatTable;        //< Format table
    SwCharFormats      *mpCharFormatTable;
    SwFrameFormats       *mpSpzFrameFormatTable;
    SwSectionFormats   *mpSectionFormatTable;
    SwFrameFormats       *mpTableFrameFormatTable;     //< For tables
    SwTextFormatColls   *mpTextFormatCollTable;    //< FormatCollections
    SwGrfFormatColls   *mpGrfFormatCollTable;

    SwTOXTypes      *mpTOXTypes;         //< Tables/indices
    SwDefTOXBase_Impl * mpDefTOXBases;   //< defaults of SwTOXBase's

    SwDBManager         *mpDBManager;            /**< Pointer to the DBManager for
                                         evaluation of DB-fields. */

    SwNumRule       *mpOutlineRule;
    SwFootnoteInfo       *mpFootnoteInfo;
    SwEndNoteInfo   *mpEndNoteInfo;
    SwLineNumberInfo*mpLineNumberInfo;
    SwFootnoteIdxs       *mpFootnoteIdxs;

    SwDocShell      *mpDocShell;         //< Ptr to SfxDocShell of Doc.
    SfxObjectShellLock mxTmpDocShell;    //< A temporary shell that is used to copy OLE-Nodes

    SwAutoCorrExceptWord *mpACEWord;     /**< For the automated takeover of
                                         auto-corrected words that are "re-corrected". */
    SwURLStateChanged *mpURLStateChgd;   //< SfxClient for changes in INetHistory
    SvNumberFormatter *mpNumberFormatter;//< NumFormatter for tables / fields

    mutable SwNumRuleTable    *mpNumRuleTable;   //< List of all named NumRules.

    // Hash map to find numrules by name
    mutable std::unordered_map<OUString, SwNumRule *, OUStringHash> maNumRuleMap;

    SwPagePreviewPrtData *mpPgPViewPrtData;  //< Indenting / spacing for printing of page view.
    SwPaM           *mpExtInputRing;

    IStyleAccess    *mpStyleAccess;  //< handling of automatic styles
    SwLayoutCache   *mpLayoutCache;  /**< Layout cache to read and save with the
                                     document for a faster formatting */

    IGrammarContact *mpGrammarContact;   //< for grammar checking in paragraphs during editing

    // table of forbidden characters of this document
    rtl::Reference<SvxForbiddenCharactersTable> mxForbiddenCharsTable;
    css::uno::Reference< css::script::vba::XVBAEventProcessor > mxVbaEvents;
    css::uno::Reference<css::container::XNameContainer> m_xTemplateToProjectCache;

    /// Table styles (autoformats that are applied with table changes).
    std::unique_ptr<SwTableAutoFormatTable> mpTableStyles;
    /// Cell Styles not assigned to a Table Style
    std::unique_ptr<SwCellStyleTable> mpCellStyles;
private:
    std::unique_ptr< ::sfx2::IXmlIdRegistry > m_pXmlIdRegistry;

    // other

    sal_uInt32  mnRsid;              //< current session ID of the document
    sal_uInt32  mnRsidRoot;          //< session ID when the document was created

    sal_Int32   mReferenceCount;

    bool mbDtor                  : 1;    /**< TRUE: is in SwDoc DTOR.
                                               and unfortunately temorarily also in
                                               SwSwgReader::InLayout() when flawed
                                               frames need deletion. */
    bool mbCopyIsMove            : 1;    //< TRUE: Copy is a hidden Move.
    bool mbInReading             : 1;    //< TRUE: Document is in the process of being read.
    bool mbInMailMerge           : 1;    //< TRUE: Document is in the process of being written by mail merge.
    bool mbInXMLImport           : 1;    //< TRUE: During xml import, attribute portion building is not necessary.
    bool mbUpdateTOX             : 1;    //< TRUE: After loading document, update TOX.
    bool mbInLoadAsynchron       : 1;    //< TRUE: Document is in the process of being loaded asynchronously.
    bool mbIsAutoFormatRedline      : 1;    //< TRUE: Redlines are recorded by Autoformat.
    bool mbOLEPrtNotifyPending   : 1;    /**< TRUE: Printer has changed. At creation of
                                                View
                                                notification of OLE-Objects PrtOLENotify() is required. */
    bool mbAllOLENotify          : 1;    //< True: Notification of all objects is required.
    bool mbInsOnlyTextGlssry      : 1;    //< True: insert 'only text' glossary into doc
    bool mbContains_MSVBasic     : 1;    //< True: MS-VBasic exist is in our storage
    bool mbClipBoard             : 1;    //< TRUE: this document represents the clipboard
    bool mbColumnSelection       : 1;    /**< TRUE: this content has bee created by a column selection
                                                (clipboard docs only) */
    bool mbIsPrepareSelAll       : 1;

#ifdef DBG_UTIL
    bool mbXMLExport : 1;                //< true: during XML export
#endif

    // true: Document contains at least one anchored object, which is anchored AT_PAGE with a content position.
    //       Thus, certain adjustment needed during formatting for these kind of anchored objects.
    bool mbContainsAtPageObjWithContentAnchor : 1;

    static SwAutoCompleteWord *mpACmpltWords;  //< List of all words for AutoComplete

    // private methods
    SwFlyFrameFormat* MakeFlySection_( const SwPosition& rAnchPos,
                                const SwContentNode& rNode, RndStdIds eRequestId,
                                const SfxItemSet* pFlyAttrSet,
                                SwFrameFormat* = nullptr );
    sal_Int8 SetFlyFrameAnchor( SwFrameFormat& rFlyFormat, SfxItemSet& rSet, bool bNewFrames );

    typedef SwFormat* (SwDoc:: *FNCopyFormat)( const OUString&, SwFormat*, bool, bool );
    SwFormat* CopyFormat( const SwFormat& rFormat, const SwFormatsBase& rFormatArr,
                        FNCopyFormat fnCopyFormat, const SwFormat& rDfltFormat );
    void CopyFormatArr( const SwFormatsBase& rSourceArr, SwFormatsBase& rDestArr,
                        FNCopyFormat fnCopyFormat, SwFormat& rDfltFormat );
    void CopyPageDescHeaderFooterImpl( bool bCpyHeader,
                                const SwFrameFormat& rSrcFormat, SwFrameFormat& rDestFormat );
    static SwFormat* FindFormatByName( const SwFormatsBase& rFormatArr, const OUString& rName );

    SwDoc( const SwDoc &) = delete;

    // Database fields:
    void AddUsedDBToList( std::vector<OUString>& rDBNameList,
                          const std::vector<OUString>& rUsedDBNames );
    void AddUsedDBToList( std::vector<OUString>& rDBNameList, const OUString& rDBName );
    static bool IsNameInArray( const std::vector<OUString>& rOldNames, const OUString& rName );
    void GetAllDBNames( std::vector<OUString>& rAllDBNames );
    static OUString ReplaceUsedDBs( const std::vector<OUString>& rUsedDBNames,
                             const OUString& rNewName, const OUString& rFormula );
    static std::vector<OUString>& FindUsedDBs( const std::vector<OUString>& rAllDBNames,
                                const OUString& rFormula,
                                std::vector<OUString>& rUsedDBNames );

    void CreateNumberFormatter();

    bool UnProtectTableCells( SwTable& rTable );

    /** Create sub-documents according to the given collection.
     If no collection is given, take chapter style of the 1st level. */
    bool SplitDoc( sal_uInt16 eDocType, const OUString& rPath, bool bOutline,
                        const SwTextFormatColl* pSplitColl, int nOutlineLevel = 0 );

    // Update charts of given table.
    void UpdateCharts_( const SwTable& rTable, SwViewShell const & rVSh ) const;

    static bool SelectNextRubyChars( SwPaM& rPam, SwRubyListEntry& rRubyEntry,
                                sal_uInt16 nMode );

    // CharTimer calls this method.
    void DoUpdateAllCharts();
    DECL_LINK_TYPED( DoUpdateModifiedOLE, Idle *, void );

public:
    SwFormat *MakeCharFormat_(const OUString &, SwFormat *, bool, bool );
    SwFormat *MakeFrameFormat_(const OUString &, SwFormat *, bool, bool );

private:
    SwFormat *MakeTextFormatColl_(const OUString &, SwFormat *, bool, bool );

private:
    OUString msDocAccTitle;

    void InitTOXTypes();

public:
    enum DocumentType {
        DOCTYPE_NATIVE,
        DOCTYPE_MSWORD              //This doc model comes from MS Word
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
    virtual sal_Int32 acquire() override;
    virtual sal_Int32 release() override;
    virtual sal_Int32 getReferenceCount() const override;

    // IDocumentSettingAccess
    IDocumentSettingAccess const & getIDocumentSettingAccess() const; //The IDocumentSettingAccess interface
    IDocumentSettingAccess & getIDocumentSettingAccess();
    ::sw::DocumentSettingManager      & GetDocumentSettingManager(); //The implementation of the interface with some additional methods
    ::sw::DocumentSettingManager const& GetDocumentSettingManager() const;
    sal_uInt32 getRsid() const;
    void setRsid( sal_uInt32 nVal );
    sal_uInt32 getRsidRoot() const;
    void setRsidRoot( sal_uInt32 nVal );

    // IDocumentDeviceAccess
    IDocumentDeviceAccess const & getIDocumentDeviceAccess() const;
    IDocumentDeviceAccess & getIDocumentDeviceAccess();

    // IDocumentMarkAccess
    IDocumentMarkAccess* getIDocumentMarkAccess();
    const IDocumentMarkAccess* getIDocumentMarkAccess() const;

    // IDocumentRedlineAccess
    IDocumentRedlineAccess const& getIDocumentRedlineAccess() const;
    IDocumentRedlineAccess& getIDocumentRedlineAccess();

    ::sw::DocumentRedlineManager const& GetDocumentRedlineManager() const;
    ::sw::DocumentRedlineManager& GetDocumentRedlineManager();

    // IDocumentUndoRedo
    IDocumentUndoRedo      & GetIDocumentUndoRedo();
    IDocumentUndoRedo const& GetIDocumentUndoRedo() const;

    // IDocumentLinksAdministration
    IDocumentLinksAdministration const & getIDocumentLinksAdministration() const;
    IDocumentLinksAdministration & getIDocumentLinksAdministration();

    ::sw::DocumentLinksAdministrationManager const & GetDocumentLinksAdministrationManager() const;
    ::sw::DocumentLinksAdministrationManager & GetDocumentLinksAdministrationManager();

    // IDocumentFieldsAccess
    IDocumentFieldsAccess const & getIDocumentFieldsAccess() const;
    IDocumentFieldsAccess & getIDocumentFieldsAccess();

    ::sw::DocumentFieldsManager & GetDocumentFieldsManager();

    // IDocumentContentOperations
    IDocumentContentOperations const & getIDocumentContentOperations() const;
    IDocumentContentOperations & getIDocumentContentOperations();
    ::sw::DocumentContentOperationsManager const & GetDocumentContentOperationsManager() const;
    ::sw::DocumentContentOperationsManager & GetDocumentContentOperationsManager();

    bool UpdateParRsid( SwTextNode *pTextNode, sal_uInt32 nVal = 0 );
    bool UpdateRsid( const SwPaM &rRg, sal_Int32 nLen );

    // IDocumentStylePoolAccess
    IDocumentStylePoolAccess const & getIDocumentStylePoolAccess() const;
    IDocumentStylePoolAccess & getIDocumentStylePoolAccess();

    // SwLineNumberInfo
    const SwLineNumberInfo& GetLineNumberInfo() const;
    void SetLineNumberInfo(const SwLineNumberInfo& rInfo);

    // IDocumentStatistics
    IDocumentStatistics const & getIDocumentStatistics() const;
    IDocumentStatistics & getIDocumentStatistics();

    ::sw::DocumentStatisticsManager const & GetDocumentStatisticsManager() const;
    ::sw::DocumentStatisticsManager & GetDocumentStatisticsManager();

    // IDocumentState
    IDocumentState const & getIDocumentState() const;
    IDocumentState & getIDocumentState();

    // IDocumentDrawModelAccess
    DECL_LINK_TYPED( AddDrawUndo, SdrUndoAction *, void );
    IDocumentDrawModelAccess const & getIDocumentDrawModelAccess() const;
    IDocumentDrawModelAccess & getIDocumentDrawModelAccess();

    ::sw::DocumentDrawModelManager const & GetDocumentDrawModelManager() const;
    ::sw::DocumentDrawModelManager & GetDocumentDrawModelManager();

    // IDocumentLayoutAccess
    IDocumentLayoutAccess const & getIDocumentLayoutAccess() const;
    IDocumentLayoutAccess & getIDocumentLayoutAccess();

    ::sw::DocumentLayoutManager const & GetDocumentLayoutManager() const;
    ::sw::DocumentLayoutManager & GetDocumentLayoutManager();

    // IDocumentTimerAccess
    // Our own 'IdleTimer' calls the following method
    IDocumentTimerAccess const & getIDocumentTimerAccess() const;
    IDocumentTimerAccess & getIDocumentTimerAccess();

    // IDocumentChartDataProviderAccess
    IDocumentChartDataProviderAccess const & getIDocumentChartDataProviderAccess() const;
    IDocumentChartDataProviderAccess & getIDocumentChartDataProviderAccess();

    // IDocumentListItems
    IDocumentListItems const & getIDocumentListItems() const;
    IDocumentListItems & getIDocumentListItems();

    // IDocumentOutlineNodes
    IDocumentOutlineNodes const & getIDocumentOutlineNodes() const;
    IDocumentOutlineNodes & getIDocumentOutlineNodes();

    // IDocumentListsAccess
    IDocumentListsAccess const & getIDocumentListsAccess() const;
    IDocumentListsAccess & getIDocumentListsAccess();

    //IDocumentExternalData
    IDocumentExternalData const & getIDocumentExternalData() const;
    IDocumentExternalData & getIDocumentExternalData();

    //End of Interfaces

    void setDocAccTitle( const OUString& rTitle ) { msDocAccTitle = rTitle; }
    const OUString& getDocAccTitle() const { return msDocAccTitle; }

    // INextInterface here
    DECL_LINK_TYPED(CalcFieldValueHdl, EditFieldInfo*, void);

    // OLE ???
    bool IsOLEPrtNotifyPending() const  { return mbOLEPrtNotifyPending; }
    inline void SetOLEPrtNotifyPending( bool bSet = true );
    void PrtOLENotify( bool bAll ); //All or only marked

#ifdef DBG_UTIL
    bool InXMLExport() const            { return mbXMLExport; }
#endif
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
     have to be surrounded completely by css::awt::Selection.
     ( Start < Pos < End ) !!!
     (Required for Writers.) */
    SwPosFlyFrames GetAllFlyFormats( const SwPaM* = nullptr,
                        bool bDrawAlso = false,
                        bool bAsCharAlso = false ) const;

    // Because swrtf.cxx and define private public here now.
    SwFlyFrameFormat  *MakeFlyFrameFormat (const OUString &rFormatName, SwFrameFormat *pDerivedFrom);
    SwDrawFrameFormat *MakeDrawFrameFormat(const OUString &rFormatName, SwFrameFormat *pDerivedFrom);

    // From now on this interface has to be used for Flys.
    // pAnchorPos must be set, if they are not attached to pages AND
    // Anchor is not already set at valid ContentPos
    // in FlySet/FrameFormat.
    /* new parameter bCalledFromShell

       true: An existing adjust item at pAnchorPos is propagated to
       the content node of the new fly section. That propagation only
       takes place if there is no adjust item in the paragraph style
       for the new fly section.

       false: no propagation
    */
    SwFlyFrameFormat* MakeFlySection( RndStdIds eAnchorType,
                                 const SwPosition* pAnchorPos,
                                 const SfxItemSet* pSet = nullptr,
                                 SwFrameFormat *pParent = nullptr,
                                 bool bCalledFromShell = false );
    SwFlyFrameFormat* MakeFlyAndMove( const SwPaM& rPam, const SfxItemSet& rSet,
                                const SwSelBoxes* pSelBoxes = nullptr,
                                SwFrameFormat *pParent = nullptr );

    //UUUU Helper that checks for unique items for DrawingLayer items of type NameOrIndex
    // and evtl. corrects that items to ensure unique names for that type. This call may
    // modify/correct entries inside of the given SfxItemSet, and it will apply a name to
    // the items in question (what is essential to make the named slots associated with
    // these items work for the UNO API and thus e.g. for ODF im/export)
    void CheckForUniqueItemForLineFillNameOrIndex(SfxItemSet& rSet);

    bool SetFlyFrameAttr( SwFrameFormat& rFlyFormat, SfxItemSet& rSet );

    bool SetFrameFormatToFly( SwFrameFormat& rFlyFormat, SwFrameFormat& rNewFormat,
                        SfxItemSet* pSet = nullptr, bool bKeepOrient = false );
    void SetFlyFrameTitle( SwFlyFrameFormat& rFlyFrameFormat,
                         const OUString& sNewTitle );
    void SetFlyFrameDescription( SwFlyFrameFormat& rFlyFrameFormat,
                               const OUString& sNewDescription );

    // Footnotes
    // Footnote information
    const SwFootnoteInfo& GetFootnoteInfo() const         { return *mpFootnoteInfo; }
    void SetFootnoteInfo(const SwFootnoteInfo& rInfo);
    const SwEndNoteInfo& GetEndNoteInfo() const { return *mpEndNoteInfo; }
    void SetEndNoteInfo(const SwEndNoteInfo& rInfo);
          SwFootnoteIdxs& GetFootnoteIdxs()       { return *mpFootnoteIdxs; }
    const SwFootnoteIdxs& GetFootnoteIdxs() const { return *mpFootnoteIdxs; }
    // change footnotes in area
    bool SetCurFootnote( const SwPaM& rPam, const OUString& rNumStr,
                    sal_uInt16 nNumber, bool bIsEndNote );

    /** Operations on the content of the document e.g.
        spell-checking/hyphenating/word-counting
    */
    css::uno::Any
            Spell( SwPaM&, css::uno::Reference< css::linguistic2::XSpellChecker1 > &,
                   sal_uInt16* pPageCnt, sal_uInt16* pPageSt, bool bGrammarCheck,
                   SwConversionArgs *pConvArgs = nullptr ) const;

    css::uno::Reference< css::linguistic2::XHyphenatedWord >
            Hyphenate( SwPaM *pPam, const Point &rCursorPos,
                         sal_uInt16* pPageCnt, sal_uInt16* pPageSt );

    // count words in pam
    static void CountWords( const SwPaM& rPaM, SwDocStat& rStat );

    // Glossary Document
    bool IsInsOnlyTextGlossary() const      { return mbInsOnlyTextGlssry; }

    void Summary( SwDoc* pExtDoc, sal_uInt8 nLevel, sal_uInt8 nPara, bool bImpress );

    void ChangeAuthorityData(const SwAuthEntry* pNewData);

    bool IsInHeaderFooter( const SwNodeIndex& rIdx ) const;
    short GetTextDirection( const SwPosition& rPos,
                            const Point* pPt = nullptr ) const;
    bool IsInVerticalText( const SwPosition& rPos ) const;

    // Database  and DB-Manager
    void SetDBManager( SwDBManager* pNewMgr )     { mpDBManager = pNewMgr; }
    SwDBManager* GetDBManager() const             { return mpDBManager; }
    void ChangeDBFields( const std::vector<OUString>& rOldNames,
                        const OUString& rNewName );
    void SetInitDBFields(bool b);

    // Find out which databases are used by fields.
    void GetAllUsedDB( std::vector<OUString>& rDBNameList,
                       const std::vector<OUString>* pAllDBNames = nullptr );

    void ChgDBData( const SwDBData& rNewData );
    SwDBData GetDBData();
    const SwDBData& GetDBDesc();

    // Some helper functions
    OUString GetUniqueGrfName() const;
    OUString GetUniqueOLEName() const;
    OUString GetUniqueFrameName() const;
    OUString GetUniqueShapeName() const;

    std::set<SwRootFrame*> GetAllLayouts();

    void SetFlyName( SwFlyFrameFormat& rFormat, const OUString& rName );
    const SwFlyFrameFormat* FindFlyByName( const OUString& rName, sal_Int8 nNdTyp = 0 ) const;

    static void GetGrfNms( const SwFlyFrameFormat& rFormat, OUString* pGrfName, OUString* pFltName );

    // Set a valid name for all Flys that have none (Called by Readers after reading).
    void SetAllUniqueFlyNames();

    /** Reset attributes. All TextHints and (if completely selected) all hard-
     formatted stuff (auto-formats) are removed.
     Introduce new optional parameter <bSendDataChangedEvents> in order to
     control, if the side effect "send data changed events" is triggered or not. */
    void ResetAttrs( const SwPaM &rRg,
                     bool bTextAttr = true,
                     const std::set<sal_uInt16> &rAttrs = std::set<sal_uInt16>(),
                     const bool bSendDataChangedEvents = true );
    void RstTextAttrs(const SwPaM &rRg, bool bInclRefToxMark = false, bool bExactRange = false );

    /** Set attribute in given format.1y
     *  If Undo is enabled, the old values is added to the Undo history. */
    void SetAttr( const SfxPoolItem&, SwFormat& );
    /** Set attribute in given format.1y
     *  If Undo is enabled, the old values is added to the Undo history. */
    void SetAttr( const SfxItemSet&, SwFormat& );

    // method to reset a certain attribute at the given format
    void ResetAttrAtFormat( const sal_uInt16 nWhichId,
                            SwFormat& rChangedFormat );

    /** Set attribute as new default attribute in current document.
     If Undo is activated, the old one is listed in Undo-History. */
    void SetDefault( const SfxPoolItem& );
    void SetDefault( const SfxItemSet& );

    // Query default attribute in this document.
    const SfxPoolItem& GetDefault( sal_uInt16 nFormatHint ) const;

    // Do not expand text attributes.
    bool DontExpandFormat( const SwPosition& rPos, bool bFlag = true );

    // Formats
    const SwFrameFormats* GetFrameFormats() const     { return mpFrameFormatTable; }
          SwFrameFormats* GetFrameFormats()           { return mpFrameFormatTable; }
    const SwCharFormats* GetCharFormats() const   { return mpCharFormatTable;}
          SwCharFormats* GetCharFormats()         { return mpCharFormatTable;}

    // LayoutFormats (frames, DrawObjects), sometimes const sometimes not
    const SwFrameFormats* GetSpzFrameFormats() const   { return mpSpzFrameFormatTable; }
          SwFrameFormats* GetSpzFrameFormats()         { return mpSpzFrameFormatTable; }

    const SwFrameFormat *GetDfltFrameFormat() const   { return mpDfltFrameFormat; }
          SwFrameFormat *GetDfltFrameFormat()         { return mpDfltFrameFormat; }
    const SwFrameFormat *GetEmptyPageFormat() const { return mpEmptyPageFormat; }
          SwFrameFormat *GetEmptyPageFormat()       { return mpEmptyPageFormat; }
    const SwFrameFormat *GetColumnContFormat() const{ return mpColumnContFormat; }
          SwFrameFormat *GetColumnContFormat()      { return mpColumnContFormat; }
    const SwCharFormat *GetDfltCharFormat() const { return mpDfltCharFormat;}
          SwCharFormat *GetDfltCharFormat()       { return mpDfltCharFormat;}

    // @return the interface of the management of (auto)styles
    IStyleAccess& GetIStyleAccess() { return *mpStyleAccess; }

    // Remove all language dependencies from all existing formats
    void RemoveAllFormatLanguageDependencies();

    SwFrameFormat  *MakeFrameFormat(const OUString &rFormatName, SwFrameFormat *pDerivedFrom,
                          bool bBroadcast = false, bool bAuto = true);
    void       DelFrameFormat( SwFrameFormat *pFormat, bool bBroadcast = false );
    SwFrameFormat* FindFrameFormatByName( const OUString& rName ) const
        {   return static_cast<SwFrameFormat*>(FindFormatByName( static_cast<SwFormatsBase&>(*mpFrameFormatTable), rName )); }

    SwCharFormat *MakeCharFormat(const OUString &rFormatName, SwCharFormat *pDerivedFrom,
                           bool bBroadcast = false,
                           bool bAuto = true );
    void       DelCharFormat(size_t nFormat, bool bBroadcast = false);
    void       DelCharFormat(SwCharFormat* pFormat, bool bBroadcast = false);
    SwCharFormat* FindCharFormatByName( const OUString& rName ) const
        {   return static_cast<SwCharFormat*>(FindFormatByName( (SwFormatsBase&)*mpCharFormatTable, rName )); }

    // Formatcollections (styles)
    // TXT
    const SwTextFormatColl* GetDfltTextFormatColl() const { return mpDfltTextFormatColl; }
    SwTextFormatColl* GetDfltTextFormatColl() { return mpDfltTextFormatColl; }
    const SwTextFormatColls *GetTextFormatColls() const { return mpTextFormatCollTable; }
    SwTextFormatColls *GetTextFormatColls() { return mpTextFormatCollTable; }
    SwTextFormatColl *MakeTextFormatColl( const OUString &rFormatName,
                                  SwTextFormatColl *pDerivedFrom,
                                  bool bBroadcast = false,
                                  bool bAuto = true );
    SwConditionTextFormatColl* MakeCondTextFormatColl( const OUString &rFormatName,
                                               SwTextFormatColl *pDerivedFrom,
                                               bool bBroadcast = false);
    void DelTextFormatColl(size_t nFormat, bool bBroadcast = false);
    void DelTextFormatColl( SwTextFormatColl* pColl, bool bBroadcast = false );
    /** Add 4th optional parameter <bResetListAttrs>.
     'side effect' of <SetTextFormatColl> with <bReset = true> is that the hard
     attributes of the affected text nodes are cleared, except the break
     attribute, the page description attribute and the list style attribute.
     The new parameter <bResetListAttrs> indicates, if the list attributes
     (list style, restart at and restart with) are cleared as well in case
     that <bReset = true> and the paragraph style has a list style attribute set. */
    bool SetTextFormatColl(const SwPaM &rRg, SwTextFormatColl *pFormat,
                       const bool bReset = true,
                       const bool bResetListAttrs = false);
    SwTextFormatColl* FindTextFormatCollByName( const OUString& rName ) const
        {   return static_cast<SwTextFormatColl*>(FindFormatByName( (SwFormatsBase&)*mpTextFormatCollTable, rName )); }

    void ChkCondColls();

    const SwGrfFormatColl* GetDfltGrfFormatColl() const   { return mpDfltGrfFormatColl; }
    SwGrfFormatColl* GetDfltGrfFormatColl()  { return mpDfltGrfFormatColl; }
    const SwGrfFormatColls *GetGrfFormatColls() const     { return mpGrfFormatCollTable; }
    SwGrfFormatColl *MakeGrfFormatColl(const OUString &rFormatName,
                                    SwGrfFormatColl *pDerivedFrom);
    SwGrfFormatColl* FindGrfFormatCollByName( const OUString& rName ) const
        {   return static_cast<SwGrfFormatColl*>(FindFormatByName( (SwFormatsBase&)*mpGrfFormatCollTable, rName )); }

    // Table formatting
    const SwFrameFormats* GetTableFrameFormats() const  { return mpTableFrameFormatTable; }
          SwFrameFormats* GetTableFrameFormats()        { return mpTableFrameFormatTable; }
    size_t GetTableFrameFormatCount( bool bUsed ) const;
    SwFrameFormat& GetTableFrameFormat(size_t nFormat, bool bUsed ) const;
    SwTableFormat* MakeTableFrameFormat(const OUString &rFormatName, SwFrameFormat *pDerivedFrom);
    void        DelTableFrameFormat( SwTableFormat* pFormat );
    SwTableFormat* FindTableFormatByName( const OUString& rName, bool bAll = false ) const;

    /** Access to frames.
    Iterate over Flys - for Basic-Collections. */
    size_t GetFlyCount( FlyCntType eType = FLYCNTTYPE_ALL, bool bIgnoreTextBoxes = false ) const;
    SwFrameFormat* GetFlyNum(size_t nIdx, FlyCntType eType = FLYCNTTYPE_ALL, bool bIgnoreTextBoxes = false );
    std::vector<SwFrameFormat const*> GetFlyFrameFormats(
            FlyCntType eType = FLYCNTTYPE_ALL,
            bool bIgnoreTextBoxes = false);

    // Copy formats in own arrays and return them.
    SwFrameFormat  *CopyFrameFormat ( const SwFrameFormat& );
    SwCharFormat *CopyCharFormat( const SwCharFormat& );
    SwTextFormatColl* CopyTextColl( const SwTextFormatColl& rColl );
    SwGrfFormatColl* CopyGrfColl( const SwGrfFormatColl& rColl );

    // Replace all styles with those from rSource.
    void ReplaceStyles( const SwDoc& rSource, bool bIncludePageStyles = true );

    // Replace all property defaults with those from rSource.
    void ReplaceDefaults( const SwDoc& rSource );

    // Replace all compatibility options with those from rSource.
    void ReplaceCompatibilityOptions( const SwDoc& rSource );

    /** Replace all user defined document properties with xSourceDocProps.

        Convenince function used by ReplaceDocumentProperties to skip some UNO calls.
     */
    void ReplaceUserDefinedDocumentProperties( const css::uno::Reference< css::document::XDocumentProperties >& xSourceDocProps );

    /** Replace document properties with those from rSource.

        This includes the user defined document properties!
     */
    void ReplaceDocumentProperties(const SwDoc& rSource, bool mailMerge = false);

    // Query if style (paragraph- / character- / frame- / page-) is used.
    bool IsUsed( const SwModify& ) const;
    static bool IsUsed( const SwNumRule& );

    // Set name of newly loaded document template.
    size_t SetDocPattern(const OUString& rPatternName);

    // @return name of document template. Can be 0!
    const OUString* GetDocPattern(size_t nPos) const;

    // travel over PaM Ring
    bool InsertGlossary( SwTextBlocks& rBlock, const OUString& rEntry,
                        SwPaM& rPaM, SwCursorShell* pShell = nullptr);

    /** get the set of printable pages for the XRenderable API by
     evaluating the respective settings (see implementation) */
    static void CalculatePagesForPrinting( const SwRootFrame& rLayout, SwRenderData &rData, const SwPrintUIOptions &rOptions, bool bIsPDFExport,
            sal_Int32 nDocPageCount );
    static void UpdatePagesForPrintingWithPostItData( SwRenderData &rData, const SwPrintUIOptions &rOptions, bool bIsPDFExport,
            sal_Int32 nDocPageCount );
    static void CalculatePagePairsForProspectPrinting( const SwRootFrame& rLayout, SwRenderData &rData, const SwPrintUIOptions &rOptions,
            sal_Int32 nDocPageCount );

    // PageDescriptor interface.
    size_t GetPageDescCnt() const { return m_PageDescs.size(); }
    const SwPageDesc& GetPageDesc(const size_t i) const { return *m_PageDescs[i]; }
    SwPageDesc& GetPageDesc(size_t const i) { return *m_PageDescs[i]; }
    SwPageDesc* FindPageDesc(const OUString& rName, size_t* pPos = nullptr) const;
    // Just searches the pointer in the m_PageDescs vector!
    bool        ContainsPageDesc(const SwPageDesc *pDesc, size_t* pPos = nullptr);

    /** Copy the complete PageDesc - beyond document and "deep"!
     Optionally copying of PoolFormatId, -HlpId can be prevented. */
    void CopyPageDesc( const SwPageDesc& rSrcDesc, SwPageDesc& rDstDesc,
                        bool bCopyPoolIds = true );

    /** Copy header (with contents) from SrcFormat to DestFormat
     (can also be copied into other document). */
    void CopyHeader( const SwFrameFormat& rSrcFormat, SwFrameFormat& rDestFormat )
        { CopyPageDescHeaderFooterImpl( true, rSrcFormat, rDestFormat ); }

    /** Copy footer (with contents) from SrcFormat to DestFormat.
     (can also be copied into other document). */
    void CopyFooter( const SwFrameFormat& rSrcFormat, SwFrameFormat& rDestFormat )
        { CopyPageDescHeaderFooterImpl( false, rSrcFormat, rDestFormat ); }

    // For Reader
    void ChgPageDesc( const OUString & rName, const SwPageDesc& );
    void ChgPageDesc( size_t i, const SwPageDesc& );
    void DelPageDesc( const OUString & rName, bool bBroadcast = false);
    void DelPageDesc( size_t i, bool bBroadcast = false );
    void PreDelPageDesc(SwPageDesc * pDel);
    SwPageDesc* MakePageDesc(const OUString &rName, const SwPageDesc* pCpy = nullptr,
                             bool bRegardLanguage = true,
                             bool bBroadcast = false);
    void BroadcastStyleOperation(const OUString& rName, SfxStyleFamily eFamily,
                                 sal_uInt16 nOp);

    /** The html import sometimes overwrites the
     page sizes set in the page descriptions. This function is used to
     correct this. */
    void CheckDefaultPageFormat();

    // Methods for tables/indices
    static sal_uInt16 GetCurTOXMark( const SwPosition& rPos, SwTOXMarks& );
    void DeleteTOXMark( const SwTOXMark* pTOXMark );
    const SwTOXMark& GotoTOXMark( const SwTOXMark& rCurTOXMark,
                                SwTOXSearch eDir, bool bInReadOnly );

    // Insert/Renew table/indes
    SwTOXBaseSection* InsertTableOf( const SwPosition& rPos,
                                            const SwTOXBase& rTOX,
                                            const SfxItemSet* pSet = nullptr,
                                            bool bExpand = false );
    void              InsertTableOf( sal_uLong nSttNd, sal_uLong nEndNd,
                                            const SwTOXBase& rTOX,
                                            const SfxItemSet* pSet = nullptr );
    static SwTOXBase* GetCurTOX( const SwPosition& rPos );
    static const SwAttrSet& GetTOXBaseAttrSet(const SwTOXBase& rTOX);

    bool DeleteTOX( const SwTOXBase& rTOXBase, bool bDelNodes = false );
    OUString GetUniqueTOXBaseName( const SwTOXType& rType,
                                   const OUString& sChkStr ) const;

    bool SetTOXBaseName(const SwTOXBase& rTOXBase, const OUString& rName);

    // After reading file update all tables/indices
    void SetUpdateTOX( bool bFlag = true )     { mbUpdateTOX = bFlag; }
    bool IsUpdateTOX() const                   { return mbUpdateTOX; }

    const OUString& GetTOIAutoMarkURL() const {return msTOIAutoMarkURL;}
    void            SetTOIAutoMarkURL(const OUString& rSet) {msTOIAutoMarkURL = rSet;}

    bool IsInReading() const                    { return mbInReading; }
    void SetInReading( bool bNew )              { mbInReading = bNew; }

    bool IsInMailMerge() const                  { return mbInMailMerge; }
    void SetInMailMerge( bool bNew )            { mbInMailMerge = bNew; }

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
    const SwTOXType* InsertTOXType( const SwTOXType& rTyp );
    const SwTOXTypes& GetTOXTypes() const { return *mpTOXTypes; }

    const SwTOXBase*    GetDefaultTOXBase( TOXTypes eTyp, bool bCreate = false );
    void                SetDefaultTOXBase(const SwTOXBase& rBase);

    // Key for management of index.
    sal_uInt16 GetTOIKeys( SwTOIKeyType eTyp, std::vector<OUString>& rArr ) const;

    // Sort table text.
    bool SortTable(const SwSelBoxes& rBoxes, const SwSortOptions&);
    bool SortText(const SwPaM&, const SwSortOptions&);

    // Correct the SwPosition-Objects that are registered with the document
    // e. g. Bookmarks or tables/indices.
    // If bMoveCursor is set move Cursor too.

    // Set everything in rOldNode on rNewPos + Offset.
    void CorrAbs(
        const SwNodeIndex& rOldNode,
        const SwPosition& rNewPos,
        const sal_Int32 nOffset = 0,
        bool bMoveCursor = false );

    // Set everything in the range of [rStartNode, rEndNode] to rNewPos.
    static void CorrAbs(
        const SwNodeIndex& rStartNode,
        const SwNodeIndex& rEndNode,
        const SwPosition& rNewPos,
        bool bMoveCursor = false );

    // Set everything in this range from rRange to rNewPos.
    static void CorrAbs(
        const SwPaM& rRange,
        const SwPosition& rNewPos,
        bool bMoveCursor = false );

    // Set everything in rOldNode to relative Pos.
    void CorrRel(
        const SwNodeIndex& rOldNode,
        const SwPosition& rNewPos,
        const sal_Int32 nOffset = 0,
        bool bMoveCursor = false );

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
      list Id of a list, which has to be continued by applying the given list style

     Returns the set ListId if bSetItem is true */
    OUString SetNumRule( const SwPaM&,
                     const SwNumRule&,
                     bool bCreateNewList,
                     const OUString& sContinuedListId = OUString(),
                     bool bSetItem = true,
                     const bool bResetIndentAttrs = false );
    void SetCounted( const SwPaM&, bool bCounted);

    void MakeUniqueNumRules(const SwPaM & rPaM);

    void SetNumRuleStart( const SwPosition& rPos, bool bFlag = true );
    void SetNodeNumStart( const SwPosition& rPos, sal_uInt16 nStt );

    static SwNumRule* GetNumRuleAtPos( const SwPosition& rPos );

    const SwNumRuleTable& GetNumRuleTable() const { return *mpNumRuleTable; }

    /**
       Add numbering rule to document.

       @param pRule    rule to add
    */
    void AddNumRule(SwNumRule * pRule);

    // add optional parameter <eDefaultNumberFormatPositionAndSpaceMode>
    sal_uInt16 MakeNumRule( const OUString &rName,
        const SwNumRule* pCpy = nullptr,
        bool bBroadcast = false,
        const SvxNumberFormat::SvxNumPositionAndSpaceMode eDefaultNumberFormatPositionAndSpaceMode =
            SvxNumberFormat::LABEL_WIDTH_AND_POSITION );
    sal_uInt16 FindNumRule( const OUString& rName ) const;
    SwNumRule* FindNumRulePtr( const OUString& rName ) const;

    // Deletion only possible if Rule is not used!
    bool RenameNumRule(const OUString & aOldName, const OUString & aNewName,
                           bool bBroadcast = false);
    bool DelNumRule( const OUString& rName, bool bBroadCast = false );
    OUString GetUniqueNumRuleName( const OUString* pChkStr = nullptr, bool bAutoNum = true ) const;

    void UpdateNumRule();   // Update all invalids.
    void ChgNumRuleFormats( const SwNumRule& rRule );
    bool ReplaceNumRule( const SwPosition& rPos, const OUString& rOldRule,
                        const OUString& rNewRule );

    // Goto next/previous on same level.
    static bool GotoNextNum( SwPosition&, bool bOverUpper = true,
                        sal_uInt8* pUpper = nullptr, sal_uInt8* pLower = nullptr );
    static bool GotoPrevNum( SwPosition&, bool bOverUpper = true,
                        sal_uInt8* pUpper = nullptr, sal_uInt8* pLower = nullptr );

    /** Searches for a text node with a numbering rule.

       add optional parameter <bInvestigateStartNode>
       add output parameter <sListId>

       \param rPos         position to start search
       \param bForward     - true:  search forward
                           - false: search backward
       \param bNum         - true:  search for enumeration
                           - false: search for itemize
       \param bOutline     - true:  search for outline numbering rule
                           - false: search for non-outline numbering rule
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
       true: called from shell -> propagate existing adjust item at
       rPos to every new cell. A existing adjust item in the table
       heading or table contents paragraph style prevent that
       propagation.
       false: do not propagate
    */
    const SwTable* InsertTable( const SwInsertTableOptions& rInsTableOpts,  // HEADLINE_NO_BORDER
                                const SwPosition& rPos, sal_uInt16 nRows,
                                sal_uInt16 nCols, short eAdjust,
                                const SwTableAutoFormat* pTAFormat = nullptr,
                                const std::vector<sal_uInt16> *pColArr = nullptr,
                                bool bCalledFromShell = false,
                                bool bNewModel = true );

    // If index is in a table, return TableNode, else 0.
                 SwTableNode* IsIdxInTable( const SwNodeIndex& rIdx );
    inline const SwTableNode* IsIdxInTable( const SwNodeIndex& rIdx ) const;

    // Create a balanced table out of the selected range.
    const SwTable* TextToTable( const SwInsertTableOptions& rInsTableOpts, // HEADLINE_NO_BORDER,
                                const SwPaM& rRange, sal_Unicode cCh,
                                short eAdjust,
                                const SwTableAutoFormat* = nullptr );

    // text to table conversion - API support
    const SwTable* TextToTable( const std::vector< std::vector<SwNodeRange> >& rTableNodes );

    bool TableToText( const SwTableNode* pTableNd, sal_Unicode cCh );

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
    bool SplitTable( const SwSelBoxes& rBoxes, bool bVert = true,
                       sal_uInt16 nCnt = 1, bool bSameHeight = false );

    // @return enum TableMergeErr.
    sal_uInt16 MergeTable( SwPaM& rPam );
    OUString GetUniqueTableName() const;
    bool IsInsTableFormatNum() const;
    bool IsInsTableChangeNumFormat() const;
    bool IsInsTableAlignNum() const;

    // From FEShell (for Undo and BModified).
    static void GetTabCols( SwTabCols &rFill, const SwCursor* pCursor,
                    const SwCellFrame* pBoxFrame = nullptr );
    void SetTabCols( const SwTabCols &rNew, bool bCurRowOnly,
                    const SwCursor* pCursor, const SwCellFrame* pBoxFrame = nullptr );
    static void GetTabRows( SwTabCols &rFill, const SwCursor* pCursor,
                    const SwCellFrame* pBoxFrame = nullptr );
    void SetTabRows( const SwTabCols &rNew, bool bCurColOnly, const SwCursor* pCursor,
                     const SwCellFrame* pBoxFrame = nullptr );

    // Direct access for UNO.
    void SetTabCols(SwTable& rTab, const SwTabCols &rNew, const SwTabCols &rOld,
                                    const SwTableBox *pStart, bool bCurRowOnly);

    void SetRowsToRepeat( SwTable &rTable, sal_uInt16 nSet );

    /// AutoFormat for table/table selection.
    /// @param bResetDirect Reset direct formatting that might be applied to the cells.
    bool SetTableAutoFormat(const SwSelBoxes& rBoxes, const SwTableAutoFormat& rNew, bool bResetDirect = false, bool isSetStyleName = false);

    // Query attributes.
    bool GetTableAutoFormat( const SwSelBoxes& rBoxes, SwTableAutoFormat& rGet );

    /// Return the available table styles.
    SwTableAutoFormatTable& GetTableStyles() { return *mpTableStyles.get(); }
    const SwTableAutoFormatTable& GetTableStyles() const { return *mpTableStyles.get(); }

    const SwCellStyleTable& GetCellStyles() const  { return *mpCellStyles.get(); }
          SwCellStyleTable& GetCellStyles()        { return *mpCellStyles.get(); }

    void AppendUndoForInsertFromDB( const SwPaM& rPam, bool bIsTable );

    bool SetColRowWidthHeight( SwTableBox& rAktBox, sal_uInt16 eType,
                                SwTwips nAbsDiff, SwTwips nRelDiff );
    SwTableBoxFormat* MakeTableBoxFormat();
    SwTableLineFormat* MakeTableLineFormat();

    // Check if box has numerical value. Change format of box if required.
    void ChkBoxNumFormat( SwTableBox& rAktBox, bool bCallUpdate );
    void SetTableBoxFormulaAttrs( SwTableBox& rBox, const SfxItemSet& rSet );
    void ClearBoxNumAttrs( const SwNodeIndex& rNode );
    void ClearLineNumAttrs( SwPosition & rPos );

    bool InsCopyOfTable( SwPosition& rInsPos, const SwSelBoxes& rBoxes,
                        const SwTable* pCpyTable = nullptr, bool bCpyName = false,
                        bool bCorrPos = false );

    bool UnProtectCells( const OUString& rTableName );
    bool UnProtectCells( const SwSelBoxes& rBoxes );
    bool UnProtectTables( const SwPaM& rPam );
    bool HasTableAnyProtection( const SwPosition* pPos,
                              const OUString* pTableName = nullptr,
                              bool* pFullTableProtection = nullptr );

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
    void SetTableName( SwFrameFormat& rTableFormat, const OUString &rNewName );

    // @return the reference in document that is set for name.
    const SwFormatRefMark* GetRefMark( const OUString& rName ) const;

    // @return RefMark via index - for UNO.
    const SwFormatRefMark* GetRefMark( sal_uInt16 nIndex ) const;

    /** @return names of all references that are set in document.
     If array pointer is 0 return only whether a RefMark is set in document. */
    sal_uInt16 GetRefMarks( std::vector<OUString>* = nullptr ) const;

    // Insert label. If a FlyFormat is created, return it.
    SwFlyFrameFormat* InsertLabel( const SwLabelType eType, const OUString &rText, const OUString& rSeparator,
                    const OUString& rNumberingSeparator,
                    const bool bBefore, const sal_uInt16 nId, const sal_uLong nIdx,
                    const OUString& rCharacterStyle,
                    const bool bCpyBrd = true );
    SwFlyFrameFormat* InsertDrawLabel(
        const OUString &rText, const OUString& rSeparator, const OUString& rNumberSeparator,
        const sal_uInt16 nId, const OUString& rCharacterStyle, SdrObject& rObj );

    // Query attribute pool.
    const SwAttrPool& GetAttrPool() const   { return *mpAttrPool; }
          SwAttrPool& GetAttrPool()         { return *mpAttrPool; }

    // Search for an EditShell.
    SwEditShell const * GetEditShell() const;
    SwEditShell* GetEditShell();
    ::sw::IShellCursorSupplier * GetIShellCursorSupplier();

    // OLE 2.0-notification.
    inline       void  SetOle2Link(const Link<bool,void>& rLink) {maOle2Link = rLink;}
    inline const Link<bool,void>& GetOle2Link() const {return maOle2Link;}

    // insert section (the ODF kind of section, not the nodesarray kind)
    SwSection * InsertSwSection(SwPaM const& rRange, SwSectionData &,
            SwTOXBase const*const pTOXBase = nullptr,
            SfxItemSet const*const pAttr = nullptr, bool const bUpdate = true);
    static sal_uInt16 IsInsRegionAvailable( const SwPaM& rRange,
                                const SwNode** ppSttNd = nullptr );
    static SwSection* GetCurrSection( const SwPosition& rPos );
    SwSectionFormats& GetSections() { return *mpSectionFormatTable; }
    const SwSectionFormats& GetSections() const { return *mpSectionFormatTable; }
    SwSectionFormat *MakeSectionFormat();
    void DelSectionFormat( SwSectionFormat *pFormat, bool bDelNodes = false );
    void UpdateSection(size_t const nSect, SwSectionData &,
            SfxItemSet const*const = nullptr, bool const bPreventLinkUpdate = false);
    OUString GetUniqueSectionName( const OUString* pChkStr = nullptr ) const;

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
    const SfxObjectShellLock& GetTmpDocShell()    { return mxTmpDocShell; }

    // For Autotexts? (text modules) They have only one SVPersist at their disposal.
    SfxObjectShell* GetPersist() const;

    // Pointer to storage of SfxDocShells. Can be 0!!!
    css::uno::Reference< css::embed::XStorage > GetDocStorage();

    // Query / set flag indicating if document is loaded asynchronously at this moment.
    bool IsInLoadAsynchron() const             { return mbInLoadAsynchron; }
    void SetInLoadAsynchron( bool bFlag )       { mbInLoadAsynchron = bFlag; }

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

    // Query if URL was visited.
    // Query via Doc, if only a Bookmark has been given.
    // In this case the document name has to be set in front.
    bool IsVisitedURL( const OUString& rURL );

    // Save current values for automatic registration of exceptions in Autocorrection.
    void SetAutoCorrExceptWord( SwAutoCorrExceptWord* pNew );
    SwAutoCorrExceptWord* GetAutoCorrExceptWord()       { return mpACEWord; }
    void DeleteAutoCorrExceptWord();

    const SwFormatINetFormat* FindINetAttr( const OUString& rName ) const;

    // Call into intransparent Basic; expect possible Return String.
    bool ExecMacro( const SvxMacro& rMacro, OUString* pRet = nullptr, SbxArray* pArgs = nullptr );

    // Call into intransparent Basic / JavaScript.
    sal_uInt16 CallEvent( sal_uInt16 nEvent, const SwCallMouseEvent& rCallEvent,
                        bool bChkPtr = false, SbxArray* pArgs = nullptr );

    /** Adjust left margin via object bar (similar to adjustment of numerations).
     One can either change the margin "by" adding or subtracting a given
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
                                const SwTextFormatColl* pSplitColl = nullptr );
    bool GenerateGlobalDoc( const OUString& rPath, int nOutlineLevel = 0 );
    bool GenerateHTMLDoc( const OUString& rPath,
                                const SwTextFormatColl* pSplitColl = nullptr );
    bool GenerateHTMLDoc( const OUString& rPath, int nOutlineLevel = 0 );

    //  Compare two documents.
    long CompareDoc( const SwDoc& rDoc );

    // Merge two documents.
    long MergeDoc( const SwDoc& rDoc );

    bool IsAutoFormatRedline() const           { return mbIsAutoFormatRedline; }
    void SetAutoFormatRedline( bool bFlag )    { mbIsAutoFormatRedline = bFlag; }

    // For AutoFormat: with Undo/Redlining.
    void SetTextFormatCollByAutoFormat( const SwPosition& rPos, sal_uInt16 nPoolId,
                                const SfxItemSet* pSet = nullptr );
    void SetFormatItemByAutoFormat( const SwPaM& rPam, const SfxItemSet& );

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
    void SetOLEObjModified();

    // Uno - Interfaces
    std::shared_ptr<SwUnoCursor> CreateUnoCursor( const SwPosition& rPos, bool bTableCursor = false );

    // FeShell - Interfaces
    // !!! These assume always an existing layout !!!
    bool ChgAnchor( const SdrMarkList& _rMrkList,
                        RndStdIds _eAnchorType,
                        const bool _bSameOnly,
                        const bool _bPosCorr );

    void SetRowHeight( const SwCursor& rCursor, const SwFormatFrameSize &rNew );
    static void GetRowHeight( const SwCursor& rCursor, SwFormatFrameSize *& rpSz );
    void SetRowSplit( const SwCursor& rCursor, const SwFormatRowSplit &rNew );
    static void GetRowSplit( const SwCursor& rCursor, SwFormatRowSplit *& rpSz );
    bool BalanceRowHeight( const SwCursor& rCursor, bool bTstOnly = true );
    void SetRowBackground( const SwCursor& rCursor, const SvxBrushItem &rNew );
    static bool GetRowBackground( const SwCursor& rCursor, SvxBrushItem &rToFill );
    void SetTabBorders( const SwCursor& rCursor, const SfxItemSet& rSet );
    void SetTabLineStyle( const SwCursor& rCursor,
                          const Color* pColor, bool bSetLine,
                          const editeng::SvxBorderLine* pBorderLine );
    static void GetTabBorders( const SwCursor& rCursor, SfxItemSet& rSet );
    void SetBoxAttr( const SwCursor& rCursor, const SfxPoolItem &rNew );
    /**
    Retrieves a box attribute from the given cursor.

    @return Whether the property is set over the current box selection.

    @remarks A property is 'set' if it's set to the same value over all boxes in the current selection.
    The property value is retrieved from the first box in the current selection. It is then compared to
    the values of the same property over any other boxes in the selection; if any value is different from
    that of the first box, the property is unset (and false is returned).
    */
    static bool GetBoxAttr( const SwCursor& rCursor, SfxPoolItem &rToFill );
    void SetBoxAlign( const SwCursor& rCursor, sal_uInt16 nAlign );
    static sal_uInt16 GetBoxAlign( const SwCursor& rCursor );
    void AdjustCellWidth( const SwCursor& rCursor, bool bBalance = false );

    SwChainRet Chainable( const SwFrameFormat &rSource, const SwFrameFormat &rDest );
    SwChainRet Chain( SwFrameFormat &rSource, const SwFrameFormat &rDest );
    void Unchain( SwFrameFormat &rFormat );

    // For Copy/Move from FrameShell.
    SdrObject* CloneSdrObj( const SdrObject&, bool bMoveWithinDoc = false,
                            bool bInsInPage = true );

    // FeShell - Interface end

    // Interface for TextInputData - for text input of Chinese and Japanese.
    SwExtTextInput* CreateExtTextInput( const SwPaM& rPam );
    void DeleteExtTextInput( SwExtTextInput* pDel );
    SwExtTextInput* GetExtTextInput( const SwNode& rNd,
                                sal_Int32 nContentPos = -1) const;
    SwExtTextInput* GetExtTextInput() const;

    // Interface for access to AutoComplete-List.
    static SwAutoCompleteWord& GetAutoCompleteWords() { return *mpACmpltWords; }

    bool ContainsMSVBasic() const          { return mbContains_MSVBasic; }
    void SetContainsMSVBasic( bool bFlag )  { mbContains_MSVBasic = bFlag; }

    // Interface for the list of Ruby - texts/attributes
    static sal_uInt16 FillRubyList( const SwPaM& rPam, SwRubyList& rList,
                        sal_uInt16 nMode );
    void SetRubyList( const SwPaM& rPam, const SwRubyList& rList,
                        sal_uInt16 nMode );

    void ReadLayoutCache( SvStream& rStream );
    void WriteLayoutCache( SvStream& rStream );
    SwLayoutCache* GetLayoutCache() const { return mpLayoutCache; }

    /** Checks if any of the text node contains hidden characters.
        Used for optimization. Changing the view option 'view hidden text'
        has to trigger a reformatting only if some of the text is hidden.
    */
    bool ContainsHiddenChars() const;

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
    static void MarkListLevel( SwList& rList,
                        const int nListLevel,
                        const bool bValue );

    // Change a format undoable.
    void ChgFormat(SwFormat & rFormat, const SfxItemSet & rSet);

    void RenameFormat(SwFormat & rFormat, const OUString & sNewName,
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
    static OUString GetPaMDescr(const SwPaM & rPaM);

    static bool IsFirstOfNumRuleAtPos( const SwPosition & rPos );

    // access methods for XForms model(s)

    // access container for XForms model; will be NULL if !isXForms()
    const css::uno::Reference<css::container::XNameContainer>&
        getXForms() const { return mxXForms;}

    css::uno::Reference< css::linguistic2::XProofreadingIterator > const & GetGCIterator() const;

    // #i31958# is this an XForms document?
    bool isXForms() const;

    // #i31958# initialize XForms models; turn this into an XForms document
    void initXForms( bool bCreateDefaultModel );

    // #i113606# for disposing XForms
    void disposeXForms( );

    //Update all the page masters
    void SetDefaultPageMode(bool bSquaredPageMode);
    bool IsSquaredPageMode() const;

    css::uno::Reference< css::script::vba::XVBAEventProcessor > const & GetVbaEventProcessor();
    void SetVBATemplateToProjectCache( css::uno::Reference< css::container::XNameContainer >& xCache ) { m_xTemplateToProjectCache = xCache; };
    const css::uno::Reference< css::container::XNameContainer >& GetVBATemplateToProjectCache() { return m_xTemplateToProjectCache; };
    ::sfx2::IXmlIdRegistry& GetXmlIdRegistry();
    ::sw::MetaFieldManager & GetMetaFieldManager();
    ::sw::UndoManager      & GetUndoManager();
    ::sw::UndoManager const& GetUndoManager() const;

    SfxObjectShell* CreateCopy(bool bCallInitNew, bool bEmpty) const;
    SwNodeIndex AppendDoc(const SwDoc& rSource, sal_uInt16 nStartPageNumber,
                 bool bDeletePrevious = false, int physicalPageOffset = 0,
                 const sal_uLong nDocNo = 1);

    /**
     * Dumps the entire nodes structure to the given destination (file nodes.xml in the current directory by default)
     * @since 3.5
     */
    void dumpAsXml(struct _xmlTextWriter* = nullptr) const;

    std::set<Color> GetDocColors();
    std::vector< std::weak_ptr<SwUnoCursor> > mvUnoCursorTable;

    // Remove expired UnoCursor weak pointers the document keeps to notify about document death.
    void cleanupUnoCursorTable() const
    {
        auto & rTable = const_cast<SwDoc*>(this)->mvUnoCursorTable;
        // In most cases we'll remove most of the elements.
        rTable.erase( std::remove_if(rTable.begin(),
                                     rTable.end(),
                                     [] (std::weak_ptr<SwUnoCursor> const & x) { return x.expired(); }),
                      rTable.end());
    }

private:
    // Copies master header to left / first one, if necessary - used by ChgPageDesc().
    void CopyMasterHeader(const SwPageDesc &rChged, const SwFormatHeader &rHead, SwPageDesc &pDesc, bool bLeft, bool bFirst);
    // Copies master footer to left / first one, if necessary - used by ChgPageDesc().
    void CopyMasterFooter(const SwPageDesc &rChged, const SwFormatFooter &rFoot, SwPageDesc &pDesc, bool bLeft, bool bFirst);

};

// This method is called in Dtor of SwDoc and deletes cache of ContourObjects.
void ClrContourCache();

inline const SwTableNode* SwDoc::IsIdxInTable( const SwNodeIndex& rIdx ) const
{
    return const_cast<SwDoc*>(this)->IsIdxInTable( rIdx );
}

inline SvNumberFormatter* SwDoc::GetNumberFormatter( bool bCreate )
{
    if( bCreate && !mpNumberFormatter )
        CreateNumberFormatter();
    return mpNumberFormatter;
}

inline const SvNumberFormatter* SwDoc::GetNumberFormatter( bool bCreate ) const
{
    return const_cast<SwDoc*>(this)->GetNumberFormatter( bCreate );
}

inline void SwDoc::SetOLEPrtNotifyPending( bool bSet )
{
    mbOLEPrtNotifyPending = bSet;
    if( !bSet )
        mbAllOLENotify = false;
}

bool sw_GetPostIts( IDocumentFieldsAccess* pIDFA, SetGetExpFields * pSrtLst );

#endif  //_DOC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
