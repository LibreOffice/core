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

#include <boost/scoped_ptr.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

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
class SwTOXBaseSection;
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

typedef boost::ptr_vector<SwPageDesc> SwPageDescs;

void SetAllScriptItem( SfxItemSet& rSet, const SfxPoolItem& rItem );

// global function to start grammar checking in the document
void StartGrammarChecking( SwDoc &rDoc );

// Represents the model of a Writer document.
class SW_DLLPUBLIC SwDoc :
    public IInterface
{
    friend class ::sw::DocumentContentOperationsManager;

    friend void _InitCore();
    friend void _FinitCore();

    // private Member
    ::boost::scoped_ptr<SwNodes> m_pNodes;  //< document content (Nodes Array)
    SwAttrPool* mpAttrPool;             //< the attribute pool
    SwPageDescs maPageDescs;             //< PageDescriptors
    Link<>      maOle2Link;              //< OLE 2.0-notification
    /* @@@MAINTAINABILITY-HORROR@@@
       Timer should not be members of the model
    */
    Idle       maOLEModifiedIdle;      //< Timer for update modified OLE-Objecs
    SwDBData    maDBData;                //< database descriptor
    OUString    msTOIAutoMarkURL;        //< URL of table of index AutoMark file
    boost::ptr_vector< boost::nullable<OUString> > maPatternNms;          // Array for names of document-templates
    com::sun::star::uno::Reference<com::sun::star::container::XNameContainer>
        mxXForms;                        //< container with XForms models
    mutable com::sun::star::uno::Reference< com::sun::star::linguistic2::XProofreadingIterator > m_xGCIterator;

    const ::boost::scoped_ptr< ::sw::mark::MarkManager> mpMarkManager;
    const ::boost::scoped_ptr< ::sw::MetaFieldManager > m_pMetaFieldManager;
    const ::boost::scoped_ptr< ::sw::DocumentDrawModelManager > m_pDocumentDrawModelManager;
    const ::boost::scoped_ptr< ::sw::DocumentRedlineManager > m_pDocumentRedlineManager;
    const ::boost::scoped_ptr< ::sw::DocumentStateManager > m_pDocumentStateManager;
    const ::boost::scoped_ptr< ::sw::UndoManager > m_pUndoManager;
    const ::boost::scoped_ptr< ::sw::DocumentSettingManager > m_pDocumentSettingManager;
    const ::boost::scoped_ptr< ::sw::DocumentChartDataProviderManager > m_pDocumentChartDataProviderManager;
    ::boost::scoped_ptr< ::sw::DocumentDeviceManager > m_pDeviceAccess;
    const ::boost::scoped_ptr< ::sw::DocumentTimerManager > m_pDocumentTimerManager;
    const ::boost::scoped_ptr< ::sw::DocumentLinksAdministrationManager > m_pDocumentLinksAdministrationManager;
    const ::boost::scoped_ptr< ::sw::DocumentListItemsManager > m_pDocumentListItemsManager;
    const ::boost::scoped_ptr< ::sw::DocumentListsManager > m_pDocumentListsManager;
    const ::boost::scoped_ptr< ::sw::DocumentOutlineNodesManager > m_pDocumentOutlineNodesManager;
    const ::boost::scoped_ptr< ::sw::DocumentContentOperationsManager > m_pDocumentContentOperationsManager;
    const ::boost::scoped_ptr< ::sw::DocumentFieldsManager > m_pDocumentFieldsManager;
    const ::boost::scoped_ptr< ::sw::DocumentStatisticsManager > m_pDocumentStatisticsManager;
    const ::boost::scoped_ptr< ::sw::DocumentLayoutManager > m_pDocumentLayoutManager;
    const ::boost::scoped_ptr< ::sw::DocumentStylePoolManager > m_pDocumentStylePoolManager;
    const ::boost::scoped_ptr< ::sw::DocumentExternalDataManager > m_pDocumentExternalDataManager;

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

    SwDBManager         *mpDBManager;            /**< Pointer to the DBManager for
                                         evaluation of DB-fields. */

    SwDoc           *mpGlossaryDoc;      //< Pointer to glossary-document.

    SwNumRule       *mpOutlineRule;
    SwFtnInfo       *mpFtnInfo;
    SwEndNoteInfo   *mpEndNoteInfo;
    SwLineNumberInfo*mpLineNumberInfo;
    SwFtnIdxs       *mpFtnIdxs;

    SwDocShell      *mpDocShell;         //< Ptr to SfxDocShell of Doc.
    SfxObjectShellLock mxTmpDocShell;    //< A temporary shell that is used to copy OLE-Nodes

    SwAutoCorrExceptWord *mpACEWord;     /**< For the automated takeover of
                                         auto-corrected words that are "re-corrected". */
    SwURLStateChanged *mpURLStateChgd;   //< SfxClient for changes in INetHistory
    SvNumberFormatter *mpNumberFormatter;//< NumFormatter for tables / fields

    mutable SwNumRuleTbl    *mpNumRuleTbl;   //< List of all named NumRules.

    // Hash map to find numrules by name
    mutable std::unordered_map<OUString, SwNumRule *, OUStringHash> maNumRuleMap;

    SwUnoCrsrTbl    *mpUnoCrsrTbl;

    SwPagePreviewPrtData *mpPgPViewPrtData;  //< Indenting / spacing for printing of page view.
    SwPaM           *mpExtInputRing;

    IStyleAccess    *mpStyleAccess;  //< handling of automatic styles
    SwLayoutCache   *mpLayoutCache;  /**< Layout cache to read and save with the
                                     document for a faster formatting */

    IGrammarContact *mpGrammarContact;   //< for grammar checking in paragraphs during editing

    // table of forbidden characters of this document
    rtl::Reference<SvxForbiddenCharactersTable> mxForbiddenCharsTable;
    com::sun::star::uno::Reference< com::sun::star::script::vba::XVBAEventProcessor > mxVbaEvents;
    com::sun::star::uno::Reference<com::sun::star::container::XNameContainer> m_xTemplateToProjectCache;

private:
    ::std::unique_ptr< ::sfx2::IXmlIdRegistry > m_pXmlIdRegistry;

    // other

    sal_uInt16  mnUndoCnt;           //< Count of Undo Actions.
    sal_uInt16  mnUndoSttEnd;        //< != 0 -> within parentheses.

    sal_uInt32  mnRsid;              //< current session ID of the document
    sal_uInt32  mnRsidRoot;          //< session ID when the document was created

    sal_Int32   mReferenceCount;

    bool mbGlossDoc              : 1;    //< TRUE: glossary document.
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
    bool mbIsAutoFmtRedline      : 1;    //< TRUE: Redlines are recorded by Autoformat.
    bool mbOLEPrtNotifyPending   : 1;    /**< TRUE: Printer has changed. At creation of
                                                View
                                                notification of OLE-Objects PrtOLENotify() is required. */
    bool mbAllOLENotify          : 1;    //< True: Notification of all objects is required.
    bool mbIsRedlineMove         : 1;    //< True: Redlines are moved into to / out of the section.
    bool mbInsOnlyTxtGlssry      : 1;    //< True: insert 'only text' glossary into doc
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
    SwFlyFrmFmt* _MakeFlySection( const SwPosition& rAnchPos,
                                const SwCntntNode& rNode, RndStdIds eRequestId,
                                const SfxItemSet* pFlyAttrSet,
                                SwFrmFmt* = 0 );
    sal_Int8 SetFlyFrmAnchor( SwFrmFmt& rFlyFmt, SfxItemSet& rSet, bool bNewFrms );

    typedef SwFmt* (SwDoc:: *FNCopyFmt)( const OUString&, SwFmt*, bool, bool );
    SwFmt* CopyFmt( const SwFmt& rFmt, const SwFmtsBase& rFmtArr,
                        FNCopyFmt fnCopyFmt, const SwFmt& rDfltFmt );
    void CopyFmtArr( const SwFmtsBase& rSourceArr, SwFmtsBase& rDestArr,
                        FNCopyFmt fnCopyFmt, SwFmt& rDfltFmt );
    void CopyPageDescHeaderFooterImpl( bool bCpyHeader,
                                const SwFrmFmt& rSrcFmt, SwFrmFmt& rDestFmt );
    static SwFmt* FindFmtByName( const SwFmtsBase& rFmtArr, const OUString& rName );

    SwDoc( const SwDoc &) SAL_DELETED_FUNCTION;

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

    void _CreateNumberFormatter();

    bool _UnProtectTblCells( SwTable& rTbl );

    /** Create sub-documents according to the given collection.
     If no collection is given, take chapter style of the 1st level. */
    bool SplitDoc( sal_uInt16 eDocType, const OUString& rPath, bool bOutline,
                        const SwTxtFmtColl* pSplitColl, int nOutlineLevel = 0 );

    // Update charts of given table.
    void _UpdateCharts( const SwTable& rTbl, SwViewShell const & rVSh ) const;

    static bool _SelectNextRubyChars( SwPaM& rPam, SwRubyListEntry& rRubyEntry,
                                sal_uInt16 nMode );

    // CharTimer calls this method.
    void DoUpdateAllCharts();
    DECL_LINK_TYPED( DoUpdateModifiedOLE, Idle *, void );

public:
    SwFmt *_MakeCharFmt(const OUString &, SwFmt *, bool, bool );
    SwFmt *_MakeFrmFmt(const OUString &, SwFmt *, bool, bool );

private:
    SwFmt *_MakeTxtFmtColl(const OUString &, SwFmt *, bool, bool );

private:
    bool mbReadOnly;
    OUString msDocAccTitle;

    void InitTOXTypes();

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

    bool UpdateParRsid( SwTxtNode *pTxtNode, sal_uInt32 nVal = 0 );
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
    DECL_LINK( AddDrawUndo, SdrUndoAction * );
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

    void setDocReadOnly( bool b) { mbReadOnly = b; }
    bool getDocReadOnly() const { return mbReadOnly; }
    void setDocAccTitle( const OUString& rTitle ) { msDocAccTitle = rTitle; }
    const OUString getDocAccTitle() const { return msDocAccTitle; }

    // INextInterface here
    DECL_LINK(BackgroundDone, SvxBrushItem *);
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
    // Anchor is not already set at valid CntntPos
    // in FlySet/FrmFmt.
    /* new parameter bCalledFromShell

       true: An existing adjust item at pAnchorPos is propagated to
       the content node of the new fly section. That propagation only
       takes place if there is no adjust item in the paragraph style
       for the new fly section.

       false: no propagation
    */
    SwFlyFrmFmt* MakeFlySection( RndStdIds eAnchorType,
                                 const SwPosition* pAnchorPos,
                                 const SfxItemSet* pSet = 0,
                                 SwFrmFmt *pParent = 0,
                                 bool bCalledFromShell = false );
    SwFlyFrmFmt* MakeFlyAndMove( const SwPaM& rPam, const SfxItemSet& rSet,
                                const SwSelBoxes* pSelBoxes = 0,
                                SwFrmFmt *pParent = 0 );

    //UUUU Helper that checks for unique items for DrawingLayer items of type NameOrIndex
    // and evtl. corrects that items to ensure unique names for that type. This call may
    // modify/correct entries inside of the given SfxItemSet, and it will apply a name to
    // the items in question (what is essential to make the named slots associated with
    // these items work for the UNO API and thus e.g. for ODF im/export)
    void CheckForUniqueItemForLineFillNameOrIndex(SfxItemSet& rSet);

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
    static void CountWords( const SwPaM& rPaM, SwDocStat& rStat );

    // Glossary Document
    void SetGlossDoc( bool bGlssDc = true ) { mbGlossDoc = bGlssDc; }
    bool IsInsOnlyTextGlossary() const      { return mbInsOnlyTxtGlssry; }

    void Summary( SwDoc* pExtDoc, sal_uInt8 nLevel, sal_uInt8 nPara, bool bImpress );

    void ChangeAuthorityData(const SwAuthEntry* pNewData);

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

    static void GetGrfNms( const SwFlyFrmFmt& rFmt, OUString* pGrfName, OUString* pFltName );

    // Set a valid name for all Flys that have none (Called by Readers after reading).
    void SetAllUniqueFlyNames();

    /** Reset attributes. All TxtHints and (if completely selected) all hard-
     formatted stuff (auto-formats) are removed.
     Introduce new optional parameter <bSendDataChangedEvents> in order to
     control, if the side effect "send data changed events" is triggered or not. */
    void ResetAttrs( const SwPaM &rRg,
                     bool bTxtAttr = true,
                     const std::set<sal_uInt16> &rAttrs = std::set<sal_uInt16>(),
                     const bool bSendDataChangedEvents = true );
    void RstTxtAttrs(const SwPaM &rRg, bool bInclRefToxMark = false, bool bExactRange = false );

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
          SwCharFmts* GetCharFmts()         { return mpCharFmtTbl;}

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
        {   return static_cast<SwFrmFmt*>(FindFmtByName( static_cast<SwFmtsBase&>(*mpFrmFmtTbl), rName )); }

    SwCharFmt *MakeCharFmt(const OUString &rFmtName, SwCharFmt *pDerivedFrom,
                           bool bBroadcast = false,
                           bool bAuto = true );
    void       DelCharFmt(size_t nFmt, bool bBroadcast = false);
    void       DelCharFmt(SwCharFmt* pFmt, bool bBroadcast = false);
    SwCharFmt* FindCharFmtByName( const OUString& rName ) const
        {   return static_cast<SwCharFmt*>(FindFmtByName( (SwFmtsBase&)*mpCharFmtTbl, rName )); }

    // Formatcollections (styles)
    // TXT
    const SwTxtFmtColl* GetDfltTxtFmtColl() const { return mpDfltTxtFmtColl; }
    SwTxtFmtColl* GetDfltTxtFmtColl() { return mpDfltTxtFmtColl; }
    const SwTxtFmtColls *GetTxtFmtColls() const { return mpTxtFmtCollTbl; }
    SwTxtFmtColls *GetTxtFmtColls() { return mpTxtFmtCollTbl; }
    SwTxtFmtColl *MakeTxtFmtColl( const OUString &rFmtName,
                                  SwTxtFmtColl *pDerivedFrom,
                                  bool bBroadcast = false,
                                  bool bAuto = true );
    SwConditionTxtFmtColl* MakeCondTxtFmtColl( const OUString &rFmtName,
                                               SwTxtFmtColl *pDerivedFrom,
                                               bool bBroadcast = false);
    void DelTxtFmtColl(size_t nFmt, bool bBroadcast = false);
    void DelTxtFmtColl( SwTxtFmtColl* pColl, bool bBroadcast = false );
    /** Add 4th optional parameter <bResetListAttrs>.
     'side effect' of <SetTxtFmtColl> with <bReset = true> is that the hard
     attributes of the affected text nodes are cleared, except the break
     attribute, the page description attribute and the list style attribute.
     The new parameter <bResetListAttrs> indicates, if the list attributes
     (list style, restart at and restart with) are cleared as well in case
     that <bReset = true> and the paragraph style has a list style attribute set. */
    bool SetTxtFmtColl(const SwPaM &rRg, SwTxtFmtColl *pFmt,
                       const bool bReset = true,
                       const bool bResetListAttrs = false);
    SwTxtFmtColl* FindTxtFmtCollByName( const OUString& rName ) const
        {   return static_cast<SwTxtFmtColl*>(FindFmtByName( (SwFmtsBase&)*mpTxtFmtCollTbl, rName )); }

    void ChkCondColls();

    const SwGrfFmtColl* GetDfltGrfFmtColl() const   { return mpDfltGrfFmtColl; }
    SwGrfFmtColl* GetDfltGrfFmtColl()  { return mpDfltGrfFmtColl; }
    const SwGrfFmtColls *GetGrfFmtColls() const     { return mpGrfFmtCollTbl; }
    SwGrfFmtColl *MakeGrfFmtColl(const OUString &rFmtName,
                                    SwGrfFmtColl *pDerivedFrom);
    SwGrfFmtColl* FindGrfFmtCollByName( const OUString& rName ) const
        {   return static_cast<SwGrfFmtColl*>(FindFmtByName( (SwFmtsBase&)*mpGrfFmtCollTbl, rName )); }

    // Table formatting
    const SwFrmFmts* GetTblFrmFmts() const  { return mpTblFrmFmtTbl; }
          SwFrmFmts* GetTblFrmFmts()        { return mpTblFrmFmtTbl; }
    size_t GetTblFrmFmtCount( bool bUsed ) const;
    SwFrmFmt& GetTblFrmFmt(size_t nFmt, bool bUsed ) const;
    SwTableFmt* MakeTblFrmFmt(const OUString &rFmtName, SwFrmFmt *pDerivedFrom);
    void        DelTblFrmFmt( SwTableFmt* pFmt );
    SwTableFmt* FindTblFmtByName( const OUString& rName, bool bAll = false ) const;

    /** Access to frames.
    Iterate over Flys - forr Basic-Collections. */
    size_t GetFlyCount( FlyCntType eType = FLYCNTTYPE_ALL, bool bIgnoreTextBoxes = false ) const;
    SwFrmFmt* GetFlyNum(size_t nIdx, FlyCntType eType = FLYCNTTYPE_ALL, bool bIgnoreTextBoxes = false );

    // Copy formats in own arrays and return them.
    SwFrmFmt  *CopyFrmFmt ( const SwFrmFmt& );
    SwCharFmt *CopyCharFmt( const SwCharFmt& );
    SwTxtFmtColl* CopyTxtColl( const SwTxtFmtColl& rColl );
    SwGrfFmtColl* CopyGrfColl( const SwGrfFmtColl& rColl );

    // Replace all styles with those from rSource.
    void ReplaceStyles( const SwDoc& rSource, bool bIncludePageStyles = true );

    // Replace all property defaults with those from rSource.
    void ReplaceDefaults( const SwDoc& rSource );

    // Replace all compatibility options with those from rSource.
    void ReplaceCompatibilityOptions( const SwDoc& rSource );

    /** Replace all user defined document properties with xSourceDocProps.

        Convenince function used by ReplaceDocumentProperties to skip some UNO calls.
     */
    void ReplaceUserDefinedDocumentProperties( const ::com::sun::star::uno::Reference< ::com::sun::star::document::XDocumentProperties > xSourceDocProps );

    /** Replace document properties with those from rSource.

        This includes the user defined document properties!
     */
    void ReplaceDocumentProperties(const SwDoc& rSource, bool mailMerge = false);

    // Query if style (paragraph- / character- / frame- / page-) is used.
    bool IsUsed( const SwModify& ) const;
    static bool IsUsed( const SwNumRule& );

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

    // Query / connect current document with glossary document.
    void SetGlossaryDoc( SwDoc* pDoc ) { mpGlossaryDoc = pDoc; }

    // travel over PaM Ring
    bool InsertGlossary( SwTextBlocks& rBlock, const OUString& rEntry,
                        SwPaM& rPaM, SwCrsrShell* pShell = 0);

    /** get the set of printable pages for the XRenderable API by
     evaluating the respective settings (see implementation) */
    static void CalculatePagesForPrinting( const SwRootFrm& rLayout, SwRenderData &rData, const SwPrintUIOptions &rOptions, bool bIsPDFExport,
            sal_Int32 nDocPageCount );
    static void UpdatePagesForPrintingWithPostItData( SwRenderData &rData, const SwPrintUIOptions &rOptions, bool bIsPDFExport,
            sal_Int32 nDocPageCount );
    static void CalculatePagePairsForProspectPrinting( const SwRootFrm& rLayout, SwRenderData &rData, const SwPrintUIOptions &rOptions,
            sal_Int32 nDocPageCount );

    // PageDescriptor interface.
    sal_uInt16 GetPageDescCnt() const { return maPageDescs.size(); }
    const SwPageDesc& GetPageDesc( const sal_uInt16 i ) const { return maPageDescs[i]; }
    SwPageDesc& GetPageDesc( sal_uInt16 i ) { return maPageDescs[i]; }
    SwPageDesc* FindPageDesc(const OUString& rName, sal_uInt16* pPos = NULL);
    SwPageDesc* FindPageDesc(const OUString& rName, sal_uInt16* pPos = NULL) const;
    // Just searches the pointer in the maPageDescs vector!
    bool        ContainsPageDesc(const SwPageDesc *pDesc, sal_uInt16* pPos = NULL);

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
    void ChgPageDesc( const OUString & rName, const SwPageDesc& );
    void ChgPageDesc( sal_uInt16 i, const SwPageDesc& );
    void DelPageDesc( const OUString & rName, bool bBroadcast = false);
    void DelPageDesc( sal_uInt16 i, bool bBroadcast = false );
    void PreDelPageDesc(SwPageDesc * pDel);
    SwPageDesc* MakePageDesc(const OUString &rName, const SwPageDesc* pCpy = 0,
                             bool bRegardLanguage = true,
                             bool bBroadcast = false);
    void BroadcastStyleOperation(const OUString& rName, SfxStyleFamily eFamily,
                                 sal_uInt16 nOp);

    /** The html import sometimes overwrites the
     page sizes set in the page descriptions. This function is used to
     correct this. */
    void CheckDefaultPageFmt();

    // Methods for tables/indices
    static sal_uInt16 GetCurTOXMark( const SwPosition& rPos, SwTOXMarks& );
    void DeleteTOXMark( const SwTOXMark* pTOXMark );
    const SwTOXMark& GotoTOXMark( const SwTOXMark& rCurTOXMark,
                                SwTOXSearch eDir, bool bInReadOnly );

    // Insert/Renew table/indes
    SwTOXBaseSection* InsertTableOf( const SwPosition& rPos,
                                            const SwTOXBase& rTOX,
                                            const SfxItemSet* pSet = 0,
                                            bool bExpand = false );
    const SwTOXBaseSection* InsertTableOf( sal_uLong nSttNd, sal_uLong nEndNd,
                                            const SwTOXBase& rTOX,
                                            const SfxItemSet* pSet = 0 );
    static SwTOXBase* GetCurTOX( const SwPosition& rPos );
    static const SwAttrSet& GetTOXBaseAttrSet(const SwTOXBase& rTOX);

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
    // e. g. Bookmarks or tables/indices.
    // If bMoveCrsr is set move Crsr too.

    // Set everything in rOldNode on rNewPos + Offset.
    void CorrAbs(
        const SwNodeIndex& rOldNode,
        const SwPosition& rNewPos,
        const sal_Int32 nOffset = 0,
        bool bMoveCrsr = false );

    // Set everything in the range of [rStartNode, rEndNode] to rNewPos.
    static void CorrAbs(
        const SwNodeIndex& rStartNode,
        const SwNodeIndex& rEndNode,
        const SwPosition& rNewPos,
        bool bMoveCrsr = false );

    // Set everything in this range from rRange to rNewPos.
    static void CorrAbs(
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

    // Deletion only possible if Rule is not used!
    bool RenameNumRule(const OUString & aOldName, const OUString & aNewName,
                           bool bBroadcast = false);
    bool DelNumRule( const OUString& rName, bool bBroadCast = false );
    OUString GetUniqueNumRuleName( const OUString* pChkStr = 0, bool bAutoNum = true ) const;

    void UpdateNumRule();   // Update all invalids.
    void ChgNumRuleFmts( const SwNumRule& rRule );
    bool ReplaceNumRule( const SwPosition& rPos, const OUString& rOldRule,
                        const OUString& rNewRule );

    // Goto next/previous on same level.
    static bool GotoNextNum( SwPosition&, bool bOverUpper = true,
                        sal_uInt8* pUpper = 0, sal_uInt8* pLower = 0 );
    static bool GotoPrevNum( SwPosition&, bool bOverUpper = true,
                        sal_uInt8* pUpper = 0, sal_uInt8* pLower = 0 );

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
    static void GetTabCols( SwTabCols &rFill, const SwCursor* pCrsr,
                    const SwCellFrm* pBoxFrm = 0 );
    void SetTabCols( const SwTabCols &rNew, bool bCurRowOnly,
                    const SwCursor* pCrsr, const SwCellFrm* pBoxFrm = 0 );
    static void GetTabRows( SwTabCols &rFill, const SwCursor* pCrsr,
                    const SwCellFrm* pBoxFrm = 0 );
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

    // Search for an EditShell.
    SwEditShell const * GetEditShell() const;
    SwEditShell* GetEditShell();
    ::sw::IShellCursorSupplier * GetIShellCursorSupplier();

    // OLE 2.0-notification.
    inline       void  SetOle2Link(const Link<>& rLink) {maOle2Link = rLink;}
    inline const Link<>& GetOle2Link() const {return maOle2Link;}

    // insert section (the ODF kind of section, not the nodesarray kind)
    SwSection * InsertSwSection(SwPaM const& rRange, SwSectionData &,
            SwTOXBase const*const pTOXBase = 0,
            SfxItemSet const*const pAttr = 0, bool const bUpdate = true);
    static sal_uInt16 IsInsRegionAvailable( const SwPaM& rRange,
                                const SwNode** ppSttNd = 0 );
    static SwSection* GetCurrSection( const SwPosition& rPos );
    SwSectionFmts& GetSections() { return *mpSectionFmtTbl; }
    const SwSectionFmts& GetSections() const { return *mpSectionFmtTbl; }
    SwSectionFmt *MakeSectionFmt( SwSectionFmt *pDerivedFrom );
    void DelSectionFmt( SwSectionFmt *pFmt, bool bDelNodes = false );
    void UpdateSection(size_t const nSect, SwSectionData &,
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

    // Query if URL was visited.
    // Query via Doc, if only a Bookmark has been given.
    // In this case the document name has to be set in front.
    bool IsVisitedURL( const OUString& rURL );

    // Save current values for automatic registration of exceptions in Autocorrection.
    void SetAutoCorrExceptWord( SwAutoCorrExceptWord* pNew );
    SwAutoCorrExceptWord* GetAutoCorrExceptWord()       { return mpACEWord; }
    void DeleteAutoCorrExceptWord();

    const SwFmtINetFmt* FindINetAttr( const OUString& rName ) const;

    // Call into intransparent Basic; expect possible Return String.
    bool ExecMacro( const SvxMacro& rMacro, OUString* pRet = 0, SbxArray* pArgs = 0 );

    // Call into intransparent Basic / JavaScript.
    sal_uInt16 CallEvent( sal_uInt16 nEvent, const SwCallMouseEvent& rCallEvent,
                        bool bChkPtr = false, SbxArray* pArgs = 0,
                        const Link<>* pCallBack = 0 );

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
                                const SwTxtFmtColl* pSplitColl = 0 );
    bool GenerateGlobalDoc( const OUString& rPath, int nOutlineLevel = 0 );
    bool GenerateHTMLDoc( const OUString& rPath,
                                const SwTxtFmtColl* pSplitColl = 0 );
    bool GenerateHTMLDoc( const OUString& rPath, int nOutlineLevel = 0 );

    //  Compare two documents.
    long CompareDoc( const SwDoc& rDoc );

    // Merge two documents.
    long MergeDoc( const SwDoc& rDoc );

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
    void SetOLEObjModified();

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
    static void GetRowHeight( const SwCursor& rCursor, SwFmtFrmSize *& rpSz );
    void SetRowSplit( const SwCursor& rCursor, const SwFmtRowSplit &rNew );
    static void GetRowSplit( const SwCursor& rCursor, SwFmtRowSplit *& rpSz );
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

    SwChainRet Chainable( const SwFrmFmt &rSource, const SwFrmFmt &rDest );
    SwChainRet Chain( SwFrmFmt &rSource, const SwFrmFmt &rDest );
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
    static sal_uInt16 FillRubyList( const SwPaM& rPam, SwRubyList& rList,
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
    static OUString GetPaMDescr(const SwPaM & rPaM);

    static bool IsFirstOfNumRuleAtPos( const SwPosition & rPos );

    // access methods for XForms model(s)

    // access container for XForms model; will be NULL if !isXForms()
    com::sun::star::uno::Reference<com::sun::star::container::XNameContainer>
        getXForms() const { return mxXForms;}

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

    com::sun::star::uno::Reference< com::sun::star::script::vba::XVBAEventProcessor > GetVbaEventProcessor();
    void SetVBATemplateToProjectCache( com::sun::star::uno::Reference< com::sun::star::container::XNameContainer >& xCache ) { m_xTemplateToProjectCache = xCache; };
        com::sun::star::uno::Reference< com::sun::star::container::XNameContainer > GetVBATemplateToProjectCache() { return m_xTemplateToProjectCache; };
    ::sfx2::IXmlIdRegistry& GetXmlIdRegistry();
    ::sw::MetaFieldManager & GetMetaFieldManager();
    ::sw::UndoManager      & GetUndoManager();
    ::sw::UndoManager const& GetUndoManager() const;

    SfxObjectShell* CreateCopy(bool bCallInitNew) const;
    SwNodeIndex AppendDoc(const SwDoc& rSource, sal_uInt16 nStartPageNumber,
                 SwPageDesc* pTargetPageDesc, bool bDeletePrevious = false, int physicalPageOffset = 0 );

    /**
     * Dumps the entire nodes structure to the given destination (file nodes.xml in the current directory by default)
     * @since 3.5
     */
    void dumpAsXml(struct _xmlTextWriter* = 0) const;

    std::set<Color> GetDocColors();

private:
    // Copies master header to left / first one, if necessary - used by ChgPageDesc().
    void CopyMasterHeader(const SwPageDesc &rChged, const SwFmtHeader &rHead, SwPageDesc &pDesc, bool bLeft, bool bFirst);
    // Copies master footer to left / first one, if necessary - used by ChgPageDesc().
    void CopyMasterFooter(const SwPageDesc &rChged, const SwFmtFooter &rFoot, SwPageDesc &pDesc, bool bLeft, bool bFirst);

};

// This method is called in Dtor of SwDoc and deletes cache of ContourObjects.
void ClrContourCache();

inline const SwTableNode* SwDoc::IsIdxInTbl( const SwNodeIndex& rIdx ) const
{
    return const_cast<SwDoc*>(this)->IsIdxInTbl( rIdx );
}

inline SvNumberFormatter* SwDoc::GetNumberFormatter( bool bCreate )
{
    if( bCreate && !mpNumberFormatter )
        _CreateNumberFormatter();
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

bool sw_GetPostIts( IDocumentFieldsAccess* pIDFA, _SetGetExpFlds * pSrtLst );

#endif  //_DOC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
