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

#pragma once

#include <com/sun/star/text/WritingMode.hpp>
#include <svl/style.hxx>
#include <svx/fmmodel.hxx>
#include <unotools/charclass.hxx>
#include <vcl/prntypes.hxx>
#include <xmloff/autolayout.hxx>

#include <vector>
#include <memory>
#include <optional>
#include <string_view>
#include <map>
#include <utility>

#include "sddllapi.h"
#include "pres.hxx"
#include "stlpool.hxx"

namespace com::sun::star::xml::dom { class XNode; }
namespace com::sun::star::uno { class XInterface; }
namespace editeng { class SvxFieldItemUpdater; }
namespace vcl { class Font; }
namespace com::sun::star::presentation { class XPresentation2; }
class SdOutliner;
class Timer;
class SfxObjectShell;
class SdPage;
class SdAnimationInfo;
class SdStyleSheetPool;
class SfxMedium;
class SvxSearchItem;
class EditStatus;
class Point;
class SdTransferable;
struct SpellCallbackInfo;
class SdCustomShowList;
class SdUndoGroup;
class SdrObject;
class Idle;
class ImageMap;
class Outliner;
class SdrModel;
class SdrOutliner;
class SdrPage;
class SdrTextObj;
class SfxItemPool;
class Size;
class SdXImpressDocument;

namespace sd
{
class DrawDocShell;
#ifndef SV_DECL_DRAW_DOC_SHELL_DEFINED
#define SV_DECL_DRAW_DOC_SHELL_DEFINED
typedef rtl::Reference<DrawDocShell> DrawDocShellRef;
#endif
class UndoManager;
class ShapeList;
class FrameView;
}

class ImpDrawPageListWatcher;
class ImpMasterPageListWatcher;

struct StyleReplaceData
{
    SfxStyleFamily  nFamily;
    SfxStyleFamily  nNewFamily;
    OUString        aName;
    OUString        aNewName;
};

enum class DocCreationMode
{
    New,
    Loaded
};

namespace sd
{
    struct PresentationSettings
    {
        OUString maPresPage;
        bool mbAll;
        bool mbEndless;
        bool mbCustomShow;
        bool mbManual;
        bool mbMouseVisible;
        bool mbMouseAsPen;
        bool mbLockedPages;
        bool mbAlwaysOnTop;
        bool mbFullScreen;
        bool mbAnimationAllowed;
        sal_Int32 mnPauseTimeout;
        bool mbShowPauseLogo;
        bool mbStartCustomShow;
        bool mbInteractive;

        PresentationSettings();
    };
}

/**
 * Type aliases for commonly used data structures
 */
typedef std::vector<OUString> PageNameList;            // List of page/bookmark names
typedef std::vector<OUString> SlideLayoutNameList;     // List of slide layout names
typedef std::map<OUString, sal_Int32> SlideLayoutMap;  // Map of slide layout names to indices
typedef std::map<OUString, std::shared_ptr<model::Theme>> ThemeMap; // Map of theme names to theme objects

/**
 * Context for style sheet transfers during page operations
 *
 * This structure contains all data required for transferring styles (graphic, cell, table)
 * between documents or pages. It maintains collections of styles that need to be
 * transferred and manages pointers to source and destination style sheet pools.
 */
struct StyleTransferContext
{
    // Style collections to be transferred
    StyleSheetCopyResultVector aGraphicStyles;  // Graphic styles to be transferred
    StyleSheetCopyResultVector aCellStyles;     // Cell styles to be transferred
    XStyleVector aTableStyles;                  // Table styles to be transferred
    OUString aRenameString;                     // String for style renaming operations

    // Layout information
    SlideLayoutMap aSlideLayouts;               // Layouts to be transferred
    ThemeMap aThemes;                           // Themes to be transferred

    // Style sheet pools (optional, set by the caller)
    SdStyleSheetPool* pSourceStyleSheetPool;    // Source style sheet pool
    SdStyleSheetPool* pDestStyleSheetPool;      // Destination style sheet pool

    StyleTransferContext()
        : pSourceStyleSheetPool(nullptr)
        , pDestStyleSheetPool(nullptr)
    {}

    // Constructor with style sheet pools
    StyleTransferContext(SdStyleSheetPool* pSource, SdStyleSheetPool* pDest)
        : pSourceStyleSheetPool(pSource)
        , pDestStyleSheetPool(pDest)
    {}

    // Set style sheet pools
    void setStyleSheetPools(SdStyleSheetPool* pSource, SdStyleSheetPool* pDest) {
        pSourceStyleSheetPool = pSource;
        pDestStyleSheetPool = pDest;
    }

    // Reset/clear all collections
    void clear() {
        aGraphicStyles.clear();
        aCellStyles.clear();
        aTableStyles.clear();
        aRenameString.clear();
        aSlideLayouts.clear();
        aThemes.clear();
        // Don't clear pointers to style sheet pools
    }
};

/**
 * Options for inserting bookmarks as pages
 *
 * This structure defines all parameters for controlling how pages/bookmarks
 * are inserted into documents. It covers operations like linking, replacing,
 * and various flags to control the behavior during page insertion.
 */
struct InsertBookmarkOptions
{
    bool bLink;               // Insert bookmarks as links
    bool bReplace;            // Replace pages instead of inserting
    bool bNoDialogs;          // No dialogs allowed
    bool bCopy;               // Copy source document
    bool bMergeMasterPages;   // Merge master pages
    bool bPreservePageNames;  // Preserve page names
    bool bIsClipboardOperation; // Operation triggered by clipboard
    bool bIsDragAndDropOperation; // Operation triggered by drag and drop
    bool bIsSameDocumentOperation; // Operation within the same document
    bool bIsFileDocument;     // Operation involves a file document

    InsertBookmarkOptions()
        : bLink(false), bReplace(false), bNoDialogs(false),
          bCopy(true), bMergeMasterPages(true), bPreservePageNames(false),
          bIsClipboardOperation(false), bIsDragAndDropOperation(false),
          bIsSameDocumentOperation(false), bIsFileDocument(false)
    {}

    // Preset for paste operation
    static InsertBookmarkOptions ForPaste(bool bMergeMasterPages) {
        InsertBookmarkOptions options;
        options.bIsClipboardOperation = true;
        options.bMergeMasterPages = bMergeMasterPages;
        // All defaults are fine for paste
        return options;
    }

    // Preset for file insert operation
    static InsertBookmarkOptions ForFileInsert(bool bLinkPages) {
        InsertBookmarkOptions options;
        options.bLink = bLinkPages;
        options.bIsFileDocument = true;
        return options;
    }

    // Preset for drag and drop operation
    static InsertBookmarkOptions ForDragDrop(bool bMergeMasterPages) {
        InsertBookmarkOptions options;
        options.bIsDragAndDropOperation = true;
        options.bNoDialogs = true;
        options.bMergeMasterPages = bMergeMasterPages;
        return options;
    }

    // Preset for page link resolution
    static InsertBookmarkOptions ForPageLinks(bool bCopySource, bool bNoDialogs) {
        InsertBookmarkOptions options;
        options.bLink = true;
        options.bReplace = true;
        options.bPreservePageNames = true;
        options.bCopy = bCopySource;
        options.bNoDialogs = bNoDialogs;
        return options;
    }

    // Preset for internal document operations
    static InsertBookmarkOptions ForInternalOps(bool bPreserveNames) {
        InsertBookmarkOptions options;
        options.bNoDialogs = true;
        options.bMergeMasterPages = false;
        options.bPreservePageNames = bPreserveNames;
        options.bIsSameDocumentOperation = true;
        return options;
    }

    // Preset for document import operations
    static InsertBookmarkOptions ForDocumentImport() {
        InsertBookmarkOptions options;
        options.bLink = false;            // No linking for document import
        options.bReplace = true;          // Replace pages when importing document
        options.bNoDialogs = true;        // No dialogs for document import
        options.bCopy = true;             // Always copy when importing document
        options.bMergeMasterPages = true; // Always merge master pages
        options.bPreservePageNames = false; // Don't preserve page names
        options.bIsFileDocument = true;   // This is a file document operation
        return options;
    }
};

/**
 * Properties of a document page, used for page insertion and scaling operations
 *
 * This structure stores the geometric properties of a page including size, margins,
 * orientation, and a pointer to the page object itself. Used primarily during
 * page insertion and scaling operations.
 */
struct PageProperties
{
    Size         size;        // Page size dimensions
    sal_Int32    left;        // Left margin
    sal_Int32    right;       // Right margin
    sal_Int32    upper;       // Upper (top) margin
    sal_Int32    lower;       // Lower (bottom) margin
    Orientation  orientation; // Page orientation (portrait/landscape)
    SdPage*      pPage;       // Pointer to the page object
};

/**
 * Document page count information
 *
 * Tracks counts of different page types during document operations,
 * particularly useful during page insertion and master page handling.
 */
struct DocumentPageCounts
{
    sal_uInt16 nDestPageCount;     // Count of standard pages in destination document
    sal_uInt16 nSourcePageCount;   // Count of standard pages in source document
    sal_uInt16 nMasterPageCount;   // Count of master pages in destination document
    sal_uInt16 nNewMPageCount;     // Count of new master pages after processing

    DocumentPageCounts(sal_uInt16 destCount, sal_uInt16 sourceCount, sal_uInt16 masterCount)
        : nDestPageCount(destCount)
        , nSourcePageCount(sourceCount)
        , nMasterPageCount(masterCount)
        , nNewMPageCount(0)
    {}

    // Check if all counts are valid (non-zero)
    bool areValid() const {
        return !(nDestPageCount == 0 || nSourcePageCount == 0 || nMasterPageCount == 0);
    }
};

/**
 * Parameters related to page insertion operations
 *
 * Collects all parameters needed for page insertion operations including
 * insertion position, bookmark name, replacement count, undo status, scaling options,
 * and property information for both main and notes pages.
 */
struct PageInsertionParams
{
    sal_uInt16 nInsertPos;           // Position where to insert pages
    OUString aBookmarkName;          // Name of the bookmark for insertion
    sal_uInt16 nReplacedStandardPages; // Number of replaced standard pages
    bool bUndo;                      // Whether undo is enabled
    bool bScaleObjects;              // Whether to scale objects
    PageNameList* pExchangeList;     // List of pages for exchange operations
    SdDrawDocument* pBookmarkDoc;    // Source document for page insertion
    PageProperties mainProps;        // Properties of main pages (size, borders, etc.)
    PageProperties notesProps;       // Properties of notes pages (size, borders, etc.)

    PageInsertionParams(sal_uInt16 a_nInsertPos, PageNameList* a_pExchangeList = nullptr, SdDrawDocument* a_pBookmarkDoc = nullptr)
        : nInsertPos(a_nInsertPos)
        , aBookmarkName(OUString())
        , nReplacedStandardPages(0)
        , bUndo(true)
        , bScaleObjects(false)
        , pExchangeList(a_pExchangeList)
        , pBookmarkDoc(a_pBookmarkDoc)
        // mainProps and notesProps are default-initialized
    {}
};


// SdDrawDocument
class SD_DLLPUBLIC SdDrawDocument final : public FmFormModel
{
public:
    SAL_DLLPRIVATE void setDocAccTitle( const OUString& rTitle ) { msDocAccTitle = rTitle; }
    SAL_DLLPRIVATE const OUString& getDocAccTitle() const { return msDocAccTitle; }
    SAL_DLLPRIVATE bool getDocReadOnly() const { return m_bReadOnly; }
private:
    OUString            msDocAccTitle;
    std::unique_ptr<SdOutliner>
                        mpOutliner;          ///< local outliner for outline mode
    std::unique_ptr<SdOutliner>
                        mpInternalOutliner;  ///< internal outliner for creation of text objects
    std::unique_ptr<Timer> mpWorkStartupTimer;
    std::unique_ptr<Idle>
                        mpOnlineSpellingIdle;
    std::unique_ptr<sd::ShapeList>
                        mpOnlineSpellingList;
    std::unique_ptr<SvxSearchItem>
                        mpOnlineSearchItem;
    std::vector<std::unique_ptr<sd::FrameView>>
                        maFrameViewList;
    std::unique_ptr<SdCustomShowList>   mpCustomShowList;
    ::sd::DrawDocShell* mpDocSh;
    SdTransferable *    mpCreatingTransferable;
    bool                mbHasOnlineSpellErrors;
    bool                mbInitialOnlineSpellingEnabled;
    OUString            maBookmarkFile;
    ::sd::DrawDocShellRef   mxBookmarkDocShRef;

    sd::PresentationSettings maPresentationSettings;

    css::uno::Reference< css::presentation::XPresentation2 > mxPresentation;

    bool                mbNewOrLoadCompleted;

    bool                mbOnlineSpell;
    bool                mbSummationOfParagraphs;
    sal_uInt16          mnStartWithPresentation; ///< 1-based starting slide# when presenting via command line parameter --show
    bool                mbExitAfterPresenting; ///< true if GetStartWithPresentation AND Presentation was shown fully
    LanguageType        meLanguage;
    LanguageType        meLanguageCJK;
    LanguageType        meLanguageCTL;
    SvxNumType          mePageNumType;
    ::sd::DrawDocShellRef   mxAllocedDocShRef;   // => AllocModel()
    bool                mbAllocDocSh;       // => AllocModel()
    DocumentType        meDocType;
    std::optional<CharClass>
                        moCharClass;

    ::std::unique_ptr<ImpDrawPageListWatcher> mpDrawPageListWatcher;
    ::std::unique_ptr<ImpMasterPageListWatcher> mpMasterPageListWatcher;

    SAL_DLLPRIVATE void                UpdatePageObjectsInNotes(sal_uInt16 nStartPos);
    SAL_DLLPRIVATE void                UpdatePageRelativeURLs(SdPage const * pPage, sal_uInt16 nPos, sal_Int32 nIncrement);
    SAL_DLLPRIVATE void                FillOnlineSpellingList(SdPage const * pPage);
    SAL_DLLPRIVATE void                SpellObject(SdrTextObj* pObj);

                        DECL_DLLPRIVATE_LINK(WorkStartupHdl, Timer *, void);
                        DECL_DLLPRIVATE_LINK(OnlineSpellingHdl, Timer *, void);
                        DECL_DLLPRIVATE_LINK(OnlineSpellEventHdl, EditStatus&, void);

    std::vector< OUString > maAnnotationAuthors;
    std::vector<css::uno::Reference< css::xml::dom::XNode> > maLayoutInfo;

    std::vector<css::uno::Reference< css::xml::dom::XNode> > maPresObjectInfo;

    bool mbEmbedFonts : 1;
    bool mbEmbedUsedFontsOnly : 1;
    bool mbEmbedFontScriptLatin : 1;
    bool mbEmbedFontScriptAsian : 1;
    bool mbEmbedFontScriptComplex : 1;

    sal_Int32 mnImagePreferredDPI;

    SAL_DLLPRIVATE virtual css::uno::Reference< css::frame::XModel > createUnoModel() override;

public:

    /**
     * Initialize the bookmark document for page/object operations
     */
    bool initBookmarkDoc(::sd::DrawDocShell* pBookmarkDocSh, SdDrawDocument*& pBookmarkDoc, OUString& aBookmarkName);

    /**
     * Get page properties from the first standard and notes pages
     */
    void getPageProperties(PageProperties& mainProps, PageProperties& notesProps, sal_uInt16 nSdPageCount);

    // --- Page insertion and document handling operations ---

    /**
     * Insert specific pages selected from the bookmark list
     */
    void insertSelectedPages(const PageNameList& rBookmarkList,
        PageInsertionParams& rParams,
        InsertBookmarkOptions rOptions);

    /**
     * Insert all pages from the source document
     */
    void insertAllPages(PageInsertionParams& rParams,
        const InsertBookmarkOptions& rOptions,
        const sal_uInt16& nBMSdPageCount);

    /**
     * Determine whether objects should be scaled during insertion
     */
    bool determineScaleObjects(bool bNoDialogs,
        const PageNameList& rBookmarkList,
        PageInsertionParams& rParams);

    /**
     * Collect layouts that need to be transferred from source document
     */
    void collectLayoutsToTransfer(const PageNameList& rBookmarkList,
        SdDrawDocument* pBookmarkDoc,
        SlideLayoutNameList& rLayoutsToTransfer,
        const sal_uInt16& nBMSdPageCount);

    /**
     * Transfer layout styles from source document to destination
     */
    void transferLayoutStyles(const SlideLayoutNameList& layoutsToTransfer,
        SdDrawDocument* pBookmarkDoc,
        SfxUndoManager* pUndoMgr,
        StyleTransferContext& rStyleContext);

    /**
     * Copy styles between documents with options for replacement and dialog suppression
     */
    static void copyStyles(bool bReplace, bool bNoDialogs,
        StyleTransferContext& rStyleContext);

    /**
     * Remove duplicate master pages after insertion
     */
    void removeDuplicateMasterPages(PageInsertionParams& rParams,
        DocumentPageCounts& rPageCounts);

    /**
     * Update pages after insertion with proper styles and properties
     */
    void updateInsertedPages(PageInsertionParams& rParams,
        const InsertBookmarkOptions& rOptions,
        const DocumentPageCounts& rPageCounts,
        StyleTransferContext& rStyleContext);

    /**
     * Rename object styles if needed after page insertion
     */
    void renameObjectStylesIfNeeded(sal_uInt32 nInsertPos,
        StyleTransferContext& rStyleContext,
        sal_uInt32 nBMSdPageCount);

    /**
     * Clean up unused styles after page insertion
     */
    void cleanupStyles(SfxUndoManager* pUndoMgr,
        StyleTransferContext& rStyleContext);

    /**
     * Begin an undoable action for page operations
     */
    SfxUndoManager* beginUndoAction();

    /**
     * End an undoable action for page operations
     */
    void endUndoAction(bool bUndo, SfxUndoManager* pUndoMgr);

    SAL_DLLPRIVATE SdDrawDocument(DocumentType eType, SfxObjectShell* pDocSh);
    SAL_DLLPRIVATE virtual ~SdDrawDocument() override;

    // Adapt to given Size and Borders scaling all contained data, maybe
    // including PresObj's in higher derivations
    virtual void adaptSizeAndBorderForAllPages(
        const Size& rNewSize,
        tools::Long nLeft = 0,
        tools::Long nRight = 0,
        tools::Long nUpper = 0,
        tools::Long nLower = 0) override;

    // Adapt PageSize for all Pages of PageKind ePageKind. Also
    // set Borders to left/right/upper/lower, ScaleAll, Orientation,
    // PaperBin and BackgroundFullSize. Create Undo-Actions when
    // a SdUndoGroup is given (then used from the View probably)
    void AdaptPageSizeForAllPages(
        const Size& rNewSize,
        PageKind ePageKind,
        SdUndoGroup* pUndoGroup = nullptr,
        tools::Long nLeft = 0,
        tools::Long nRight = 0,
        tools::Long nUpper = 0,
        tools::Long nLower = 0,
        bool bScaleAll = false,
        Orientation eOrientation = Orientation::Landscape,
        sal_uInt16 nPaperBin = 0,
        bool bBackgroundFullSize = false);

    SAL_DLLPRIVATE SdDrawDocument*     AllocSdDrawDocument() const;
    SAL_DLLPRIVATE virtual SdrModel*   AllocModel() const override; //forwards to AllocSdDrawDocument

    SAL_DLLPRIVATE rtl::Reference<SdPage>          AllocSdPage(bool bMasterPage);
    SAL_DLLPRIVATE virtual rtl::Reference<SdrPage> AllocPage(bool bMasterPage) override; //forwards to AllocSdPage

    // Override SfxBaseModel::getUnoModel and return a more concrete type
    SdXImpressDocument* getUnoModel();

    SAL_DLLPRIVATE virtual bool        IsReadOnly() const override;
    SAL_DLLPRIVATE virtual void        SetChanged(bool bFlag = true) override;

    SAL_DLLPRIVATE SfxItemPool&        GetPool() { return( *m_pItemPool ); }

    SAL_DLLPRIVATE SdOutliner* GetOutliner(bool bCreateOutliner=true);
    SdOutliner* GetInternalOutliner(bool bCreateOutliner=true);

    SAL_DLLPRIVATE ::sd::DrawDocShell*     GetDocSh() const { return mpDocSh; }

    SAL_DLLPRIVATE LanguageType        GetLanguage( const sal_uInt16 nId ) const;
    SAL_DLLPRIVATE void                SetLanguage( const LanguageType eLang, const sal_uInt16 nId );

    SAL_DLLPRIVATE SvxNumType          GetPageNumType() const override;
    SAL_DLLPRIVATE void                SetPageNumType(SvxNumType eType) { mePageNumType = eType; }
    OUString CreatePageNumValue(sal_uInt16 nNum) const;

    SAL_DLLPRIVATE DocumentType        GetDocumentType() const { return meDocType; }

    SAL_DLLPRIVATE void                SetAllocDocSh(bool bAlloc);

    SAL_DLLPRIVATE void                CreatingDataObj( SdTransferable* pTransferable ) { mpCreatingTransferable = pTransferable; }
    SAL_DLLPRIVATE virtual bool        IsCreatingDataObj() const override { return mpCreatingTransferable != nullptr; }

    /** if the document does not contain at least one handout, one slide and one notes page with
        at least one master each this methods creates them.
        If a reference document is given, the sizes and border settings of that document are used
        for newly created slides.
    */
    void   CreateFirstPages( SdDrawDocument const * pRefDocument = nullptr );
    bool                CreateMissingNotesAndHandoutPages();

    SAL_DLLPRIVATE void                MovePage(sal_uInt16 nPgNum, sal_uInt16 nNewPos) override;
    SAL_DLLPRIVATE void                InsertPage(SdrPage* pPage, sal_uInt16 nPos=0xFFFF) override;
    SAL_DLLPRIVATE void                DeletePage(sal_uInt16 nPgNum) override;
    SAL_DLLPRIVATE rtl::Reference<SdrPage> RemovePage(sal_uInt16 nPgNum) override;

    SAL_DLLPRIVATE virtual void     InsertMasterPage(SdrPage* pPage, sal_uInt16 nPos=0xFFFF) override;
    SAL_DLLPRIVATE virtual rtl::Reference<SdrPage> RemoveMasterPage(sal_uInt16 nPgNum) override;

    SAL_DLLPRIVATE void                RemoveUnnecessaryMasterPages( SdPage* pMaster=nullptr, bool bOnlyDuplicatePages=false, bool bUndo=true );
    void   SetMasterPage(sal_uInt16 nSdPageNum, std::u16string_view rLayoutName,
                                      SdDrawDocument* pSourceDoc, bool bMaster, bool bCheckMasters);

    SdDrawDocument* OpenBookmarkDoc(const OUString& rBookmarkFile);
    SAL_DLLPRIVATE SdDrawDocument* OpenBookmarkDoc(SfxMedium* pMedium);

    SAL_DLLPRIVATE void InsertBookmark(const std::vector<OUString> &rBookmarkList,
                            std::vector<OUString> &rExchangeList, bool bLink,
                            sal_uInt16 nPgPos,
                            ::sd::DrawDocShell* pBookmarkDocSh,
                            Point const * pObjPos);

    SAL_DLLPRIVATE sal_uInt16 GetStartWithPresentation() const { return mnStartWithPresentation; }
    SAL_DLLPRIVATE void SetStartWithPresentation(sal_uInt16 nStartingSlide);

    SAL_DLLPRIVATE bool IsExitAfterPresenting() const { return mbExitAfterPresenting;}
    SAL_DLLPRIVATE void SetExitAfterPresenting( bool bExitAfterPresenting );

    /// load xml-based impress layout definitions into document
    SAL_DLLPRIVATE void InitLayoutVector();
    /// return reference to vector of Impress layout definitions
    SAL_DLLPRIVATE const std::vector<css::uno::Reference< css::xml::dom::XNode> >& GetLayoutVector() const
    { return maLayoutInfo; }

   /// load xml-based impress master presentation object definitions into document
    SAL_DLLPRIVATE void InitObjectVector();
    /// return reference to vector of master presentation object definitions
    SAL_DLLPRIVATE const std::vector<css::uno::Reference<css::xml::dom::XNode> >& GetObjectVector() const { return maPresObjectInfo; }


    /**
     * Paste pages from clipboard - handles regular paste operations
     *
     * This method is called when the user performs a paste operation from the clipboard.
     * It handles the insertion of pages that were previously copied or cut to the clipboard.
     *
     * @param rBookmarkList List of page names to be pasted
     * @param pExchangeList Optional list of names to use for the pasted pages
     * @param nInsertPos Position where pages should be inserted
     * @param pBookmarkDocSh Source document shell
     * @param bMergeMasterPages Whether to merge master pages from source
     * @return true if operation was successful
     */
    bool PasteBookmarkAsPage(
        const PageNameList &rBookmarkList,
        PageNameList *pExchangeList,
        sal_uInt16 nInsertPos,
        ::sd::DrawDocShell* pBookmarkDocSh,
        bool bMergeMasterPages);

    /**
     * Insert pages from external files
     *
     * This method is called when inserting pages from external files, either through
     * the Insert > Page from File menu command or when handling file links.
     * It manages the transfer of pages from an external document to the current one.
     *
     * @param rBookmarkList List of page names to be inserted
     * @param pExchangeList Optional list of names to use for the inserted pages
     * @param bLink Whether to link to the source pages instead of copying
     * @param nInsertPos Position where pages should be inserted
     * @param pBookmarkDocSh Source document shell
     * @return true if operation was successful
     */
    bool InsertFileAsPage(
        const PageNameList &rBookmarkList,
        PageNameList *pExchangeList,
        bool bLink,
        sal_uInt16 nInsertPos,
        ::sd::DrawDocShell* pBookmarkDocSh);

    /**
     * Handle drag and drop operations
     *
     * This method is called when pages are dragged and dropped, either within
     * the same document or from another document. It processes the dropped pages
     * and inserts them at the specified position.
     *
     * @param rBookmarkList List of page names to be dropped
     * @param nInsertPos Position where pages should be inserted
     * @param pBookmarkDocSh Source document shell
     * @param bMergeMasterPages Whether to merge master pages from source
     * @return true if operation was successful
     */
    bool DropBookmarkAsPage(
        const PageNameList &rBookmarkList,
        sal_uInt16 nInsertPos,
        ::sd::DrawDocShell* pBookmarkDocSh,
        bool bMergeMasterPages);

    /**
     * Resolve page links
     *
     * This method is called when linked pages need to be resolved, typically
     * when a document with linked pages is opened or when the user chooses
     * to update or break links to external pages.
     *
     * @param rBookmarkList List of page names to resolve links for
     * @param nInsertPos Position where resolved pages should be inserted
     * @param bNoDialogs Whether to suppress dialogs during operation
     * @param bCopy Whether to copy the linked pages
     * @return true if operation was successful
     */
    bool ResolvePageLinks(
        const PageNameList &rBookmarkList,
        sal_uInt16 nInsertPos,
        bool bNoDialogs,
        bool bCopy);

    /**
     * Copy or move pages within the same document
     *
     * This method is called for internal page operations such as duplicating pages
     * or moving pages within the same document. It handles the copying or moving
     * of pages while maintaining proper references and styles.
     *
     * @param rBookmarkList List of page names to be copied or moved
     * @param pExchangeList Optional list of names to use for the destination pages
     * @param nInsertPos Position where pages should be inserted
     * @param bPreservePageNames Whether to preserve original page names
     * @return true if operation was successful
     */
    bool CopyOrMovePagesWithinDocument(
        const PageNameList &rBookmarkList,
        PageNameList *pExchangeList,
        sal_uInt16 nInsertPos,
        bool bPreservePageNames);

    /**
     * Import a whole document
     *
     * This method is called when importing an entire document, such as when
     * using Insert > Document or when merging presentations. It handles the
     * transfer of all pages and associated resources from the source document.
     *
     * @param rBookmarkList List of page names to be imported (empty for all pages)
     * @param nInsertPos Position where imported pages should be inserted
     * @param pBookmarkDocSh Source document shell
     * @return true if operation was successful
     */
    bool ImportDocumentPages(
        const PageNameList &rBookmarkList,
        sal_uInt16 nInsertPos,
        ::sd::DrawDocShell* pBookmarkDocSh);

    SAL_DLLPRIVATE bool InsertBookmarkAsObject(const PageNameList &rBookmarkList,
                                    const PageNameList &rExchangeList,
                                    ::sd::DrawDocShell* pBookmarkDocSh,
                                    Point const * pObjPos, bool bCalcObjCount);

    void   CloseBookmarkDoc();

    SAL_DLLPRIVATE SdrObject*          GetObj(std::u16string_view rObjName) const;

    /** Return the first page that has the given name.  Regular pages and
        notes pages are searched first.  When not found then the master
        pages are searched.
        @param rPgName
            Name of the page to return.
        @param rbIsMasterPage
            Is set by the method to indicate whether the returned index
            belongs to a master page (<TRUE/>) or a regular or notes page
            (<FALSE/>). The given value is ignored.
        @return
            Returns the index of the page with the given name or
            SDRPAGE_NOTFOUND (=0xffff) when such a page does not exist.
    */
    SAL_DLLPRIVATE sal_uInt16 GetPageByName(std::u16string_view rPgName, bool& rbIsMasterPage ) const;
    /** checks, if the given name is a *unique* name for an *existing* slide

        @param rPageName the name of an existing slide

        @return true, if the name is unique and the slide exists
    */
    bool IsPageNameUnique( std::u16string_view rPageName ) const;
    SdPage*GetSdPage(sal_uInt16 nPgNum, PageKind ePgKind) const;
    sal_uInt16 GetSdPageCount(PageKind ePgKind) const;

    SAL_DLLPRIVATE void                SetSelected(SdPage* pPage, bool bSelect);
    SAL_DLLPRIVATE void                UnselectAllPages();
    SAL_DLLPRIVATE bool                MovePages(sal_uInt16 nTargetPage);

    SdPage*GetMasterSdPage(sal_uInt16 nPgNum, PageKind ePgKind);
    sal_uInt16 GetMasterSdPageCount(PageKind ePgKind) const;

    sal_uInt16 GetActiveSdPageCount() const;

    SAL_DLLPRIVATE sal_uInt16              GetMasterPageUserCount(SdrPage const * pMaster) const;

    SAL_DLLPRIVATE const sd::PresentationSettings& getPresentationSettings() const { return maPresentationSettings; }
    SAL_DLLPRIVATE sd::PresentationSettings& getPresentationSettings() { return maPresentationSettings; }

    SAL_DLLPRIVATE const css::uno::Reference< css::presentation::XPresentation2 >& getPresentation() const;

    SAL_DLLPRIVATE void                SetSummationOfParagraphs( bool bOn = true ) { mbSummationOfParagraphs = bOn; }
    SAL_DLLPRIVATE bool            IsSummationOfParagraphs() const { return mbSummationOfParagraphs; }

    /** Set the mode that controls whether (and later how) the formatting of the document
        depends on the current printer metrics.
        @param nMode
            Use <const
            scope="css::document::PrinterIndependentLayout">ENABLED</const>
            to make formatting printer-independent and <const
            scope="css::document::PrinterIndependentLayout">DISABLED</const>
            to make formatting depend on the current printer metrics.
    */
    SAL_DLLPRIVATE void SetPrinterIndependentLayout (sal_Int32 nMode);

    /** Get the flag that controls whether the formatting of the document
        depends on the current printer metrics.
        @return
            Use <const
            scope="css::document::PrinterIndependentLayout">ENABLED</const>
            when formatting is printer-independent and <const
            scope="css::document::PrinterIndependentLayout">DISABLED</const>
            when formatting depends on the current printer metrics.
    */
    SAL_DLLPRIVATE sal_Int32 GetPrinterIndependentLayout() const { return mnPrinterIndependentLayout;}

    SAL_DLLPRIVATE void                SetOnlineSpell( bool bIn );
    SAL_DLLPRIVATE bool                GetOnlineSpell() const { return mbOnlineSpell; }
    SAL_DLLPRIVATE void                StopOnlineSpelling();
    SAL_DLLPRIVATE void                StartOnlineSpelling(bool bForceSpelling=true);

    SAL_DLLPRIVATE void                ImpOnlineSpellCallback(SpellCallbackInfo const * pInfo, SdrObject* pObj, SdrOutliner const * pOutl);

    SAL_DLLPRIVATE void                InsertObject(SdrObject* pObj);
    SAL_DLLPRIVATE void                RemoveObject(SdrObject* pObj);

    SAL_DLLPRIVATE sal_uLong           GetLinkCount() const;

    SAL_DLLPRIVATE std::vector<std::unique_ptr<sd::FrameView>>& GetFrameViewList() { return maFrameViewList; }
    SdCustomShowList* GetCustomShowList(bool bCreate = false);

    SAL_DLLPRIVATE void                NbcSetChanged(bool bFlag);

    SAL_DLLPRIVATE void                SetTextDefaults() const;

    SAL_DLLPRIVATE void                CreateLayoutTemplates();
    SAL_DLLPRIVATE void                RenameLayoutTemplate(const OUString& rOldLayoutName, const OUString& rNewName);

    SAL_DLLPRIVATE void                CreateDefaultCellStyles();

    void   StopWorkStartupDelay();

    SAL_DLLPRIVATE void                NewOrLoadCompleted(DocCreationMode eMode);
    SAL_DLLPRIVATE void                NewOrLoadCompleted( SdPage* pPage, SdStyleSheetPool* pSPool );
    SAL_DLLPRIVATE bool                IsNewOrLoadCompleted() const {return mbNewOrLoadCompleted; }

    SAL_DLLPRIVATE ::sd::FrameView* GetFrameView(sal_uLong nPos) {
        return nPos < maFrameViewList.size() ? maFrameViewList[nPos].get() : nullptr; }

    /** deprecated*/
    SAL_DLLPRIVATE static SdAnimationInfo* GetAnimationInfo(SdrObject* pObject);

    static     SdAnimationInfo* GetShapeUserData(SdrObject& rObject, bool bCreate = false );

    SAL_DLLPRIVATE const std::optional<CharClass>& GetCharClass() const { return moCharClass; }

    SAL_DLLPRIVATE void                UpdateAllLinks();

    SAL_DLLPRIVATE void                CheckMasterPages();

    SAL_DLLPRIVATE void                Merge(SdrModel& rSourceModel,
                                sal_uInt16 nFirstPageNum, sal_uInt16 nLastPageNum,
                                sal_uInt16 nDestPos,
                                bool bMergeMasterPages, bool bAllMasterPages,
                                bool bUndo = true, bool bTreadSourceAsConst = false) override;

    css::text::WritingMode GetDefaultWritingMode() const;
    SAL_DLLPRIVATE void SetDefaultWritingMode( css::text::WritingMode eMode );

    /** replacespOldPage from all custom shows with pNewPage or removes pOldPage from
        all custom shows if pNewPage is 0.
    */
    SAL_DLLPRIVATE void ReplacePageInCustomShows( const SdPage* pOldPage, const SdPage* pNewPage );

public:

    /// static to prevent recursions while resolving links
    SAL_DLLPRIVATE static SdDrawDocument* s_pDocLockedInsertingLinks;

    /** Create and insert a set of two new pages: a standard (draw) page and
        the associated notes page.  The new pages are inserted directly
        after the specified page set.
        @param pCurrentPage
            This page is used to retrieve the layout for the page to
            create.
        @param ePageKind
            This specifies whether <argument>pCurrentPage</argument> is a
            standard (draw) page or a notes page.
        @param sStandardPageName
            Name of the standard page.  An empty string leads to using an
            automatically created name.
        @param sNotesPageName
            Name of the standard page.  An empty string leads to using an
            automatically created name.
        @param eStandardLayout
            Layout to use for the new standard page.  Note that this layout
            is not used when the given <argument>pCurrentPage</argument> is
            not a standard page.  In this case the layout is taken from the
            standard page associated with <argument>pCurrentPage</argument>.
        @param eNotesLayout
            Layout to use for the new notes page.  Note that this layout
            is not used when the given <argument>pCurrentPage</argument> is
            not a notes page.  In this case the layout is taken from the
            notes page associated with <argument>pCurrentPage</argument>.
        @param bIsPageBack
            This flag indicates whether to show the background shape.
        @param bIsPageObj
            This flag indicates whether to show the shapes on the master page.
        @param nInsertPosition
            Position where to insert the standard page.  When -1 then the
            new page set is inserted after the current page.

        @return
            Returns an index of the inserted pages that can be used with the
            <member>GetSdPage()</member> method.
    */
    SAL_DLLPRIVATE sal_uInt16 CreatePage (
        SdPage* pCurrentPage,
        PageKind ePageKind,
        const OUString& sStandardPageName,
        const OUString& sNotesPageName,
        AutoLayout eStandardLayout,
        AutoLayout eNotesLayout,
        bool bIsPageBack,
        bool bIsPageObj,
        const sal_Int32 nInsertPosition);

    /** This method acts as a simplified front end for the more complex
        <member>DuplicatePage()</member> method.
        @param nPageNum
            The page number as passed to the <member>GetSdPage()</member>
            method for which the standard page and the notes page are to be
            copied.
        @return
            Returns an index of the inserted pages that can be used with the
            <member>GetSdPage()</member> method.
    */
    SAL_DLLPRIVATE sal_uInt16 DuplicatePage (sal_uInt16 nPageNum);

    /** Create and insert a set of two new pages that are copies of the
        given <argument>pCurrentPage</argument> and its associated notes
        resp. standard page.  The copies are inserted directly after the
        specified page set.
        @param pCurrentPage
            This page and its associated notes/standard page is copied.
        @param ePageKind
            This specifies whether <argument>pCurrentPage</argument> is a
            standard (draw) page or a notes page.
        @param sStandardPageName
            Name of the standard page.  An empty string leads to using an
            automatically created name.
        @param sNotesPageName
            Name of the standard page.  An empty string leads to using an
            automatically created name.
        @param bIsPageBack
            This flag indicates whether to show the background shape.
        @param bIsPageObj
            This flag indicates whether to show the shapes on the master page.
        @param nInsertPosition
            Position where to insert the standard page.  When -1 then the
            new page set is inserted after the current page.

        @return
            Returns an index of the inserted pages that can be used with the
            <member>GetSdPage()</member> method.
    */
    SAL_DLLPRIVATE sal_uInt16 DuplicatePage (
        SdPage* pCurrentPage,
        PageKind ePageKind,
        const OUString& sStandardPageName,
        const OUString& sNotesPageName,
        bool bIsPageBack,
        bool bIsPageObj,
        const sal_Int32 nInsertPosition);

    /** return the document fonts for latin, cjk and ctl according to the current
        languages set at this document */
    SAL_DLLPRIVATE void getDefaultFonts( vcl::Font& rLatinFont, vcl::Font& rCJKFont, vcl::Font& rCTLFont );

    sd::UndoManager* GetUndoManager() const;

    /** converts the given western font height to a corresponding ctl font height, depending on the system language */
    SAL_DLLPRIVATE static sal_uInt32 convertFontHeightToCTL( sal_uInt32 nWesternFontHeight );

    /** Get the style sheet pool if it was a SdStyleSheetPool.
     */
    SdStyleSheetPool* GetSdStyleSheetPool() const;

    SAL_DLLPRIVATE void UpdatePageRelativeURLs(
        std::u16string_view aOldName, std::u16string_view aNewName);

    SAL_DLLPRIVATE static void SetCalcFieldValueHdl( ::Outliner* pOutliner);

    SAL_DLLPRIVATE sal_uInt16 GetAnnotationAuthorIndex( const OUString& rAuthor );

    SAL_DLLPRIVATE bool IsEmbedFonts() const { return mbEmbedFonts; }
    SAL_DLLPRIVATE bool IsEmbedUsedFontsOnly() const { return mbEmbedUsedFontsOnly; }
    SAL_DLLPRIVATE bool IsEmbedFontScriptLatin() const { return mbEmbedFontScriptLatin; }
    SAL_DLLPRIVATE bool IsEmbedFontScriptAsian() const { return mbEmbedFontScriptAsian; }
    SAL_DLLPRIVATE bool IsEmbedFontScriptComplex() const { return mbEmbedFontScriptComplex; }

    SAL_DLLPRIVATE void SetEmbedFonts(bool bUse) { mbEmbedFonts = bUse; }
    SAL_DLLPRIVATE void SetEmbedUsedFontsOnly(bool bUse) { mbEmbedUsedFontsOnly = bUse; }
    SAL_DLLPRIVATE void SetEmbedFontScriptLatin(bool bUse) { mbEmbedFontScriptLatin = bUse; }
    SAL_DLLPRIVATE void SetEmbedFontScriptAsian(bool bUse) { mbEmbedFontScriptAsian = bUse; }
    SAL_DLLPRIVATE void SetEmbedFontScriptComplex(bool bUse) { mbEmbedFontScriptComplex = bUse; }

    sal_Int32 getImagePreferredDPI() const override { return mnImagePreferredDPI; }
    void setImagePreferredDPI(sal_Int32 nValue) { mnImagePreferredDPI = nValue; }

    void dumpAsXml(xmlTextWriterPtr pWriter) const override;

private:

    void UpdatePageRelativeURLsImpl(const std::function<void(const SvxFieldItem & rFieldItem, editeng::SvxFieldItemUpdater& rFieldItemUpdater)>& rItemCallback);

    /** This member stores the printer independent layout mode.  Please
        refer to <member>SetPrinterIndependentLayout()</member> for its
        values.
    */
    sal_Int32 mnPrinterIndependentLayout;

    /** Insert a given set of standard and notes page after the given <argument>pCurrentPage</argument>.
        @param pCurrentPage
            This page and its associated notes/standard page is copied.
        @param ePageKind
            This specifies whether <argument>pCurrentPage</argument> is a
            standard (draw) page or a notes page.
        @param sStandardPageName
            Name of the standard page.  An empty string leads to using an
            automatically created name.
        @param sNotesPageName
            Name of the standard page.  An empty string leads to using an
            automatically created name.
        @param bIsPageBack
            This flag indicates whether to show the background shape.
        @param bIsPageObj
            This flag indicates whether to show the shapes on the master page.
        @param pStandardPage
            The standard page to insert.
        @param pNotesPage
            The notes page to insert.
        @param nInsertPosition
            Position where to insert the standard page.  When -1 then the
            new page set is inserted after the current page.

        @return
            Returns an index of the inserted pages that can be used with the
            <member>GetSdPage()</member> method.
    */
    SAL_DLLPRIVATE sal_uInt16 InsertPageSet (
        SdPage* pCurrentPage,
        PageKind ePageKind,
        const OUString& sStandardPageName,
        const OUString& sNotesPageName,
        bool bIsPageBack,
        bool bIsPageObj,
        SdPage* pStandardPage,
        SdPage* pNotesPage,
        sal_Int32 nInsertPosition);

    /** Set up a newly created page and insert it into the list of pages.
        @param pPreviousPage
            A page to take the size and border geometry from.
        @param pPage
            This is the page to set up and insert.
        @param sPageName
            The name of the new page.
        @param nInsertionPoint
            Index of the page before which the new page will be inserted.
        @param bIsPageBack
            This flag indicates whether to show the background shape.
        @param bIsPageObj
            This flag indicates whether to show the shapes on the master
            page.
    */
    SAL_DLLPRIVATE void SetupNewPage (
        SdPage const * pPreviousPage,
        SdPage* pPage,
        const OUString& sPageName,
        sal_uInt16 nInsertionPoint,
        bool bIsPageBack,
        bool bIsPageObj);

    SAL_DLLPRIVATE virtual void PageListChanged() override;
    SAL_DLLPRIVATE virtual void MasterPageListChanged() override;
};

namespace sd
{

/**an instance of this guard disables modification of a document
 during its lifetime*/
class ModifyGuard
{
public:
    ModifyGuard( SdDrawDocument* pDoc );
    ~ModifyGuard();

private:
    void init();

    DrawDocShell* mpDocShell;
    SdDrawDocument* mpDoc;
    bool mbIsEnableSetModified;
    bool mbIsDocumentChanged;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
