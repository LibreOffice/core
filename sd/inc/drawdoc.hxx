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

#ifndef INCLUDED_SD_INC_DRAWDOC_HXX
#define INCLUDED_SD_INC_DRAWDOC_HXX

#include <com/sun/star/text/WritingMode.hpp>
#include <svl/style.hxx>
#include <svx/fmmodel.hxx>
#include <vcl/prntypes.hxx>
#include <xmloff/autolayout.hxx>

#include <vector>
#include <memory>

#include "sddllapi.h"
#include "pres.hxx"

namespace com { namespace sun { namespace star { namespace xml { namespace dom { class XNode; } } } } }
namespace com { namespace sun { namespace star { namespace uno { class XInterface; } } } }
namespace vcl { class Font; }

namespace com
{
    namespace sun
    {
        namespace star
        {
            namespace presentation
            {
                class XPresentation2;
            }
        }
    }
}

class SdOutliner;
class Timer;
class SfxObjectShell;
class SdPage;
class SdAnimationInfo;
class SdIMapInfo;
class IMapObject;
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
class CharClass;
class Idle;
class ImageMap;
class Outliner;
class SdrModel;
class SdrOutliner;
class SdrPage;
class SdrTextObj;
class SfxItemPool;
class Size;

namespace sd
{
class DrawDocShell;
#ifndef SV_DECL_DRAW_DOC_SHELL_DEFINED
#define SV_DECL_DRAW_DOC_SHELL_DEFINED
typedef ::tools::SvRef<DrawDocShell> DrawDocShellRef;
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

enum DocCreationMode
{
    NEW_DOC,
    DOC_LOADED
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

        PresentationSettings();
    };
}

// SdDrawDocument
class SD_DLLPUBLIC SdDrawDocument : public FmFormModel
{
public:
    SAL_DLLPRIVATE void setDocAccTitle( const OUString& rTitle ) { msDocAccTitle = rTitle; }
    SAL_DLLPRIVATE const OUString& getDocAccTitle() const { return msDocAccTitle; }
    SAL_DLLPRIVATE bool getDocReadOnly() const { return bReadOnly; }
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
    bool                mbStartWithPresentation;        ///< is set to true when starting with command line parameter -start
    bool                mbExitAfterPresenting;         ///< true if mbStartWithPresentation AND Presentation was shown fully
    LanguageType        meLanguage;
    LanguageType        meLanguageCJK;
    LanguageType        meLanguageCTL;
    SvxNumType          mePageNumType;
    ::sd::DrawDocShellRef   mxAllocedDocShRef;   // => AllocModel()
    bool                mbAllocDocSh;       // => AllocModel()
    DocumentType const  meDocType;
    std::unique_ptr<CharClass>
                        mpCharClass;

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

protected:

    SAL_DLLPRIVATE virtual css::uno::Reference< css::uno::XInterface > createUnoModel() override;

public:


    SAL_DLLPRIVATE SdDrawDocument(DocumentType eType, SfxObjectShell* pDocSh);
    SAL_DLLPRIVATE virtual ~SdDrawDocument() override;

    // Adapt to given Size and Borders scaling all contained data, maybe
    // including PresObj's in higher derivations
    virtual void adaptSizeAndBorderForAllPages(
        const Size& rNewSize,
        long nLeft = 0,
        long nRight = 0,
        long nUpper = 0,
        long nLower = 0) override;

    // Adapt PageSize for all Pages of PageKind ePageKind. Also
    // set Borders to left/right/upper/lower, ScaleAll, Orientation,
    // PaperBin and BackgroundFullSize. Create Undo-Actions when
    // a SdUndoGroup is given (then used from the View probably)
    void AdaptPageSizeForAllPages(
        const Size& rNewSize,
        PageKind ePageKind,
        SdUndoGroup* pUndoGroup = nullptr,
        long nLeft = 0,
        long nRight = 0,
        long nUpper = 0,
        long nLower = 0,
        bool bScaleAll = false,
        Orientation eOrientation = Orientation::Landscape,
        sal_uInt16 nPaperBin = 0,
        bool bBackgroundFullSize = false);

    SAL_DLLPRIVATE SdDrawDocument*     AllocSdDrawDocument() const;
    SAL_DLLPRIVATE virtual SdrModel*   AllocModel() const override; //forwards to AllocSdDrawDocument

    SAL_DLLPRIVATE SdPage*             AllocSdPage(bool bMasterPage);
    SAL_DLLPRIVATE virtual SdrPage*    AllocPage(bool bMasterPage) override; //forwards to AllocSdPage

    SAL_DLLPRIVATE virtual bool        IsReadOnly() const override;
    SAL_DLLPRIVATE virtual void        SetChanged(bool bFlag = true) override;

    SAL_DLLPRIVATE SfxItemPool&        GetPool() { return( *pItemPool ); }

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
    SAL_DLLPRIVATE SdrPage*            RemovePage(sal_uInt16 nPgNum) override;

    SAL_DLLPRIVATE virtual void     InsertMasterPage(SdrPage* pPage, sal_uInt16 nPos=0xFFFF) override;
    SAL_DLLPRIVATE virtual SdrPage* RemoveMasterPage(sal_uInt16 nPgNum) override;

    SAL_DLLPRIVATE void                RemoveUnnecessaryMasterPages( SdPage* pMaster=nullptr, bool bOnlyDuplicatePages=false, bool bUndo=true );
    void   SetMasterPage(sal_uInt16 nSdPageNum, const OUString& rLayoutName,
                                      SdDrawDocument* pSourceDoc, bool bMaster, bool bCheckMasters);

    SdDrawDocument* OpenBookmarkDoc(const OUString& rBookmarkFile);
    SAL_DLLPRIVATE SdDrawDocument* OpenBookmarkDoc(SfxMedium* pMedium);

    SAL_DLLPRIVATE void InsertBookmark(const std::vector<OUString> &rBookmarkList,
                            std::vector<OUString> &rExchangeList, bool bLink,
                            sal_uInt16 nPgPos,
                            ::sd::DrawDocShell* pBookmarkDocSh,
                            Point const * pObjPos);

    SAL_DLLPRIVATE bool IsStartWithPresentation() const { return mbStartWithPresentation;}
    SAL_DLLPRIVATE void SetStartWithPresentation( bool bStartWithPresentation );

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
    /** Insert pages into this document

        This method inserts whole pages into this document, either
        selected ones (specified via pBookmarkList/pExchangeList), or
        all from the source document.

        @attention Beware! This method in its current state does not
        handle all combinations of their input parameters
        correctly. For example, for pBookmarkList=NULL, bReplace=true
        is ignored (no replace happens).

        @param pBookmarkList
        A list of strings, denoting the names of the pages to be copied

        @param pExchangeList
        A list of strings, denoting the names of the pages to be renamed

        @param bLink
        Whether the inserted pages should be links to the bookmark document

        @param bReplace
        Whether the pages should not be inserted, but replace the pages in
        the destination document

        @param nPgPos
        Insertion point/start of replacement

        @param bNoDialogs
        Whether query dialogs are allowed (e.g. for page scaling)

        @param pBookmarkDocSh
        DocShell of the source document (used e.g. to extract the filename
        for linked pages)

        @param bCopy
        Whether the source document should be treated as immutable (i.e.
        inserted pages are not removed from it, but cloned)

        @param bMergeMasterPages
        Whether the source document's master pages should be copied, too.

        @param bPreservePageNames
        Whether the replace operation should take the name from the new
        page, or preserve the old name
     */

    SAL_DLLPRIVATE bool InsertBookmarkAsPage(const std::vector<OUString> &rBookmarkList,
                                  std::vector<OUString> *pExchangeList,
                              bool bLink, bool bReplace, sal_uInt16 nPgPos,
                              bool bNoDialogs, ::sd::DrawDocShell* pBookmarkDocSh,
                              bool bCopy, bool bMergeMasterPages,
                              bool bPreservePageNames);

    SAL_DLLPRIVATE bool InsertBookmarkAsObject(const std::vector<OUString> &rBookmarkList,
                                    const std::vector<OUString> &rExchangeList,
                                    ::sd::DrawDocShell* pBookmarkDocSh,
                                    Point const * pObjPos, bool bCalcObjCount);

    void   CloseBookmarkDoc();

    SAL_DLLPRIVATE SdrObject*          GetObj(const OUString& rObjName) const;

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
    SAL_DLLPRIVATE sal_uInt16 GetPageByName(const OUString& rPgName, bool& rbIsMasterPage ) const;
    /** checks, if the given name is a *unique* name for an *existing* slide

        @param rPageName the name of an existing slide

        @return true, if the name is unique and the slide exists
    */
    bool IsPageNameUnique( const OUString& rPageName ) const;
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
    SAL_DLLPRIVATE sal_Int32 GetPrinterIndependentLayout() { return mnPrinterIndependentLayout;}

    SAL_DLLPRIVATE void                SetOnlineSpell( bool bIn );
    SAL_DLLPRIVATE bool                GetOnlineSpell() const { return mbOnlineSpell; }
    SAL_DLLPRIVATE void                StopOnlineSpelling();
    SAL_DLLPRIVATE void                StartOnlineSpelling(bool bForceSpelling=true);

    SAL_DLLPRIVATE void                ImpOnlineSpellCallback(SpellCallbackInfo const * pInfo, SdrObject* pObj, SdrOutliner const * pOutl);

    SAL_DLLPRIVATE void                InsertObject(SdrObject* pObj);
    SAL_DLLPRIVATE void                RemoveObject(SdrObject* pObj);

    SAL_DLLPRIVATE sal_uLong           GetLinkCount();

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

    SAL_DLLPRIVATE static SdIMapInfo*  GetIMapInfo( SdrObject const * pObject );
    SAL_DLLPRIVATE static IMapObject*  GetHitIMapObject( SdrObject const * pObject, const Point& rWinPoint );

    SAL_DLLPRIVATE CharClass*          GetCharClass() const { return mpCharClass.get(); }

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

    SAL_DLLPRIVATE void UpdatePageRelativeURLs(const OUString& rOldName, const OUString& rNewName);

    SAL_DLLPRIVATE static void SetCalcFieldValueHdl( ::Outliner* pOutliner);

    SAL_DLLPRIVATE sal_uInt16 GetAnnotationAuthorIndex( const OUString& rAuthor );

    SAL_DLLPRIVATE bool IsEmbedFonts() { return mbEmbedFonts; }
    SAL_DLLPRIVATE bool IsEmbedUsedFontsOnly() { return mbEmbedUsedFontsOnly; }
    SAL_DLLPRIVATE bool IsEmbedFontScriptLatin() { return mbEmbedFontScriptLatin; }
    SAL_DLLPRIVATE bool IsEmbedFontScriptAsian() { return mbEmbedFontScriptAsian; }
    SAL_DLLPRIVATE bool IsEmbedFontScriptComplex() { return mbEmbedFontScriptComplex; }

    SAL_DLLPRIVATE void SetEmbedFonts(bool bUse) { mbEmbedFonts = bUse; }
    SAL_DLLPRIVATE void SetEmbedUsedFontsOnly(bool bUse) { mbEmbedUsedFontsOnly = bUse; }
    SAL_DLLPRIVATE void SetEmbedFontScriptLatin(bool bUse) { mbEmbedFontScriptLatin = bUse; }
    SAL_DLLPRIVATE void SetEmbedFontScriptAsian(bool bUse) { mbEmbedFontScriptAsian = bUse; }
    SAL_DLLPRIVATE void SetEmbedFontScriptComplex(bool bUse) { mbEmbedFontScriptComplex = bUse; }

    void dumpAsXml(xmlTextWriterPtr pWriter) const override;

private:
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
    SAL_DLLPRIVATE virtual ImageMap* GetImageMapForObject(SdrObject* pObj) override;
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

#endif // INCLUDED_SD_INC_DRAWDOC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
