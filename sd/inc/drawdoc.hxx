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

#ifndef _DRAWDOC_HXX
#define _DRAWDOC_HXX

#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/text/WritingMode.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <vcl/print.hxx>
#include <svx/fmmodel.hxx>
#include "pres.hxx"
#include <svx/pageitem.hxx>
#include <unotools/charclass.hxx>
#include <sot/storage.hxx>
#include <rsc/rscsfx.hxx>
#include <com/sun/star/xml/dom/XNode.hpp>

#include <svx/svdundo.hxx>

#include <vector>
#include <memory>

#include "sddllapi.h"
#include "sdpage.hxx"

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

namespace sd
{
    class FrameView;
    class Outliner;
}


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
class Window;
class SdTransferable;
struct SpellCallbackInfo;
struct StyleRequestData;
class SdDrawDocument;
class SdCustomShow;
class SdCustomShowList;

namespace sd
{
#ifndef SV_DECL_DRAW_DOC_SHELL_DEFINED
#define SV_DECL_DRAW_DOC_SHELL_DEFINED
SV_DECL_REF(DrawDocShell)
#endif
class DrawDocShell;
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
    String          aName;
    String          aNewName;
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
        sal_Bool mbAll;
        sal_Bool mbEndless;
        sal_Bool mbCustomShow;
        sal_Bool mbManual;
        sal_Bool mbMouseVisible;
        sal_Bool mbMouseAsPen;
        sal_Bool mbLockedPages;
        sal_Bool mbAlwaysOnTop;
        sal_Bool mbFullScreen;
        sal_Bool mbAnimationAllowed;
        sal_Int32 mnPauseTimeout;
        sal_Bool mbShowPauseLogo;
        sal_Bool mbStartWithNavigator;

        PresentationSettings();
        PresentationSettings( const PresentationSettings& r );
    };
}


// ------------------
// - SdDrawDocument -
// ------------------

class SdDrawDocument : public FmFormModel
{
private:
    ::sd::Outliner*     mpOutliner;          ///< local outliner for outline mode
    ::sd::Outliner*     mpInternalOutliner;  ///< internal outliner for creation of text objects
    Timer*              mpWorkStartupTimer;
    Timer*              mpOnlineSpellingTimer;
    sd::ShapeList*      mpOnlineSpellingList;
    SvxSearchItem*      mpOnlineSearchItem;
    std::vector<sd::FrameView*> maFrameViewList;
    SdCustomShowList*   mpCustomShowList;
    ::sd::DrawDocShell* mpDocSh;
    SdTransferable *    mpCreatingTransferable;
    sal_Bool                mbHasOnlineSpellErrors;
    sal_Bool                mbInitialOnlineSpellingEnabled;
    String              maBookmarkFile;
    ::sd::DrawDocShellRef   mxBookmarkDocShRef;

    sd::PresentationSettings maPresentationSettings;

    ::com::sun::star::uno::Reference< ::com::sun::star::presentation::XPresentation2 > mxPresentation;

    sal_Bool                mbNewOrLoadCompleted;

    sal_Bool                mbOnlineSpell;
    sal_Bool                mbSummationOfParagraphs;
    bool                mbStartWithPresentation;        ///< is set to true when starting with command line parameter -start
    bool                mbExitAfterPresenting;         ///< true if mbStartWithPresentation AND Presentation was shown fully
    LanguageType        meLanguage;
    LanguageType        meLanguageCJK;
    LanguageType        meLanguageCTL;
    SvxNumType          mePageNumType;
    ::sd::DrawDocShellRef   mxAllocedDocShRef;   // => AllocModel()
    sal_Bool                mbAllocDocSh;       // => AllocModel()
    DocumentType        meDocType;
    CharClass*          mpCharClass;
    ::com::sun::star::lang::Locale* mpLocale;

    ::std::auto_ptr<ImpDrawPageListWatcher> mpDrawPageListWatcher;
    ::std::auto_ptr<ImpMasterPageListWatcher> mpMasterPageListWatcher;

    void                UpdatePageObjectsInNotes(sal_uInt16 nStartPos);
    void                UpdatePageRelativeURLs(SdPage* pPage, sal_uInt16 nPos, sal_Int32 nIncrement);
    void                FillOnlineSpellingList(SdPage* pPage);
    void                SpellObject(SdrTextObj* pObj);

                        DECL_LINK(WorkStartupHdl, void *);
                        DECL_LINK(OnlineSpellingHdl, void *);
                        DECL_LINK(OnlineSpellEventHdl, EditStatus*);

    std::vector< OUString > maAnnotationAuthors;
    std::vector<com::sun::star::uno::Reference<com::sun::star::xml::dom::XNode>> maLayoutInfo;

    bool                mbUseEmbedFonts;

protected:

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > createUnoModel();

public:

                        TYPEINFO();

                        SdDrawDocument(DocumentType eType, SfxObjectShell* pDocSh);
                        ~SdDrawDocument();

    virtual SdrModel*   AllocModel() const;
    virtual SdrPage*    AllocPage(bool bMasterPage);
    virtual bool    IsReadOnly() const;
    virtual void        SetChanged(sal_Bool bFlag = sal_True);

    SfxItemPool&        GetPool() { return( *pItemPool ); }

    ::sd::Outliner* GetOutliner(sal_Bool bCreateOutliner=sal_True);
    SD_DLLPUBLIC ::sd::Outliner* GetInternalOutliner(sal_Bool bCreateOutliner=sal_True);

    ::sd::DrawDocShell*     GetDocSh() const { return mpDocSh; }

    LanguageType        GetLanguage( const sal_uInt16 nId ) const;
    void                SetLanguage( const LanguageType eLang, const sal_uInt16 nId );

    SvxNumType          GetPageNumType() const;
    void                SetPageNumType(SvxNumType eType) { mePageNumType = eType; }
    SD_DLLPUBLIC String              CreatePageNumValue(sal_uInt16 nNum) const;

    DocumentType        GetDocumentType() const { return meDocType; }

    void                SetAllocDocSh(sal_Bool bAlloc);

    void                CreatingDataObj( SdTransferable* pTransferable ) { mpCreatingTransferable = pTransferable; }

    /** if the document does not contain at least one handout, one slide and one notes page with
        at least one master each this methods creates them.
        If a reference document is given, the sizes and border settings of that document are used
        for newly created slides.
    */
    SD_DLLPUBLIC void   CreateFirstPages( SdDrawDocument* pRefDocument = 0 );
    SD_DLLPUBLIC sal_Bool                CreateMissingNotesAndHandoutPages();

    void                MovePage(sal_uInt16 nPgNum, sal_uInt16 nNewPos);
    void                InsertPage(SdrPage* pPage, sal_uInt16 nPos=0xFFFF);
    void                DeletePage(sal_uInt16 nPgNum);
    SdrPage*            RemovePage(sal_uInt16 nPgNum);

    virtual void     InsertMasterPage(SdrPage* pPage, sal_uInt16 nPos=0xFFFF);
    virtual SdrPage* RemoveMasterPage(sal_uInt16 nPgNum);

    void                RemoveUnnecessaryMasterPages( SdPage* pMaster=NULL, sal_Bool bOnlyDuplicatePages=sal_False, sal_Bool bUndo=sal_True );
    SD_DLLPUBLIC void   SetMasterPage(sal_uInt16 nSdPageNum, const String& rLayoutName,
                                      SdDrawDocument* pSourceDoc, sal_Bool bMaster, sal_Bool bCheckMasters);

    SD_DLLPUBLIC SdDrawDocument* OpenBookmarkDoc(const String& rBookmarkFile);
    SdDrawDocument*     OpenBookmarkDoc(SfxMedium& rMedium);

    sal_Bool InsertBookmark(const std::vector<OUString> &rBookmarkList,
                            std::vector<OUString> &rExchangeList, sal_Bool bLink,
                            sal_Bool bReplace, sal_uInt16 nPgPos, sal_Bool bNoDialogs,
                            ::sd::DrawDocShell* pBookmarkDocSh, sal_Bool bCopy,
                            Point* pObjPos);

    bool IsStartWithPresentation() const;
    void SetStartWithPresentation( bool bStartWithPresentation );

    bool IsExitAfterPresenting() const;
    void SetExitAfterPresenting( bool bExitAfterPresenting );

    void SetLayoutVector();
    std::vector<com::sun::star::uno::Reference<com::sun::star::xml::dom::XNode>> &GetLayoutVector() { return maLayoutInfo; }

    /** Insert pages into this document

        This method inserts whole pages into this document, either
        selected ones (specified via pBookmarkList/pExchangeList), or
        all from the source document.

        @attention Beware! This method in it's current state does not
        handle all combinations of their input parameters
        correctly. For example, for pBookmarkList=NULL, bReplace=sal_True
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
    sal_Bool InsertBookmarkAsPage(const std::vector<OUString> &rBookmarkList,
                                  std::vector<OUString> *pExchangeList,
                              sal_Bool bLink, sal_Bool bReplace, sal_uInt16 nPgPos,
                              sal_Bool bNoDialogs, ::sd::DrawDocShell* pBookmarkDocSh,
                              sal_Bool bCopy, sal_Bool bMergeMasterPages,
                              sal_Bool bPreservePageNames);

    sal_Bool InsertBookmarkAsObject(const std::vector<OUString> &rBookmarkList,
                                    const std::vector<OUString> &rExchangeList,
                                    sal_Bool bLink, ::sd::DrawDocShell* pBookmarkDocSh,
                                    Point* pObjPos, bool bCalcObjCount = false);

    SD_DLLPUBLIC void   CloseBookmarkDoc();

    SdrObject*          GetObj(const String& rObjName) const;

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
    sal_uInt16 GetPageByName(const String& rPgName, sal_Bool& rbIsMasterPage ) const;
    SD_DLLPUBLIC SdPage*GetSdPage(sal_uInt16 nPgNum, PageKind ePgKind) const;
    SD_DLLPUBLIC sal_uInt16 GetSdPageCount(PageKind ePgKind) const;

    void                SetSelected(SdPage* pPage, sal_Bool bSelect);
    sal_Bool                MovePages(sal_uInt16 nTargetPage);

    SD_DLLPUBLIC SdPage*GetMasterSdPage(sal_uInt16 nPgNum, PageKind ePgKind);
    SD_DLLPUBLIC sal_uInt16 GetMasterSdPageCount(PageKind ePgKind) const;

    SD_DLLPUBLIC sal_uInt16 GetActiveSdPageCount() const;

    sal_uInt16              GetMasterPageUserCount(SdrPage* pMaster) const;

    const sd::PresentationSettings& getPresentationSettings() const { return maPresentationSettings; }
    sd::PresentationSettings& getPresentationSettings() { return maPresentationSettings; }

    const ::com::sun::star::uno::Reference< ::com::sun::star::presentation::XPresentation2 >& getPresentation() const;

       void                SetSummationOfParagraphs( sal_Bool bOn = sal_True ) { mbSummationOfParagraphs = bOn; }
    sal_Bool            IsSummationOfParagraphs() const { return mbSummationOfParagraphs; }

    /** Set the mode that controls whether (and later how) the formatting of the document
        depends on the current printer metrics.
        @param nMode
            Use <const
            scope="com::sun::star::document::PrinterIndependentLayout">ENABLED</const>
            to make formatting printer-independent and <const
            scope="com::sun::star::document::PrinterIndependentLayout">DISABLED</const>
            to make formatting depend on the current printer metrics.
    */
    void SetPrinterIndependentLayout (sal_Int32 nMode);

    /** Get the flag that controls whether the formatting of the document
        depends on the current printer metrics.
        @return
            Use <const
            scope="com::sun::star::document::PrinterIndependentLayout">ENABLED</const>
            when formatting is printer-independent and <const
            scope="com::sun::star::document::PrinterIndependentLayout">DISABLED</const>
            when formatting depends on the current printer metrics.
    */
    sal_Int32 GetPrinterIndependentLayout (void);

    void                SetOnlineSpell( sal_Bool bIn );
    sal_Bool                GetOnlineSpell() const { return mbOnlineSpell; }
    void                StopOnlineSpelling();
    void                StartOnlineSpelling(sal_Bool bForceSpelling=sal_True);

    void                ImpOnlineSpellCallback(SpellCallbackInfo* pInfo, SdrObject* pObj, SdrOutliner* pOutl);

    void                InsertObject(SdrObject* pObj, SdPage* pPage);
    void                RemoveObject(SdrObject* pObj, SdPage* pPage);

    sal_uLong               GetLinkCount();

    std::vector<sd::FrameView*>& GetFrameViewList() { return maFrameViewList; }
    SD_DLLPUBLIC SdCustomShowList* GetCustomShowList(sal_Bool bCreate = sal_False);

    void                NbcSetChanged(sal_Bool bFlag = sal_True);

    void                SetTextDefaults() const;

    void                CreateLayoutTemplates();
    void                RenameLayoutTemplate(const String& rOldLayoutName, const String& rNewName);

    void                CreateDefaultCellStyles();

    SD_DLLPUBLIC void   StopWorkStartupDelay();

    void                NewOrLoadCompleted(DocCreationMode eMode);
    void                NewOrLoadCompleted( SdPage* pPage, SdStyleSheetPool* pSPool );
    sal_Bool                IsNewOrLoadCompleted() const {return mbNewOrLoadCompleted; }

    ::sd::FrameView* GetFrameView(sal_uLong nPos) {
        return nPos < maFrameViewList.size() ? maFrameViewList[nPos] : NULL; }

    /** deprecated*/
    SdAnimationInfo*    GetAnimationInfo(SdrObject* pObject) const;

    SD_DLLPUBLIC static     SdAnimationInfo* GetShapeUserData(SdrObject& rObject, bool bCreate = false );

    SdIMapInfo*         GetIMapInfo( SdrObject* pObject ) const;
    IMapObject*         GetHitIMapObject( SdrObject* pObject, const Point& rWinPoint, const ::Window& rCmpWnd );

    CharClass*          GetCharClass() const { return mpCharClass; }

    void                RestoreLayerNames();

    void                UpdateAllLinks();

    void                CheckMasterPages();

    void                Merge(SdrModel& rSourceModel,
                                sal_uInt16 nFirstPageNum=0, sal_uInt16 nLastPageNum=0xFFFF,
                                sal_uInt16 nDestPos=0xFFFF,
                                bool bMergeMasterPages = false, bool bAllMasterPages = false,
                                bool bUndo = true, bool bTreadSourceAsConst = false);

    SD_DLLPUBLIC ::com::sun::star::text::WritingMode GetDefaultWritingMode() const;
    void SetDefaultWritingMode( ::com::sun::star::text::WritingMode eMode );

    /** replacespOldPage from all custom shows with pNewPage or removes pOldPage from
        all custom shows if pNewPage is 0.
    */
    void ReplacePageInCustomShows( const SdPage* pOldPage, const SdPage* pNewPage );

public:

    static SdDrawDocument* pDocLockedInsertingLinks;  // static to prevent recursions while resolving links

    /** Create and insert a set of two new pages: a standard (draw) page and
        the associated notes page.  The new pages are inserted direclty
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
    sal_uInt16 CreatePage (
        SdPage* pCurrentPage,
        PageKind ePageKind,
        const String& sStandardPageName,
        const String& sNotesPageName,
        AutoLayout eStandardLayout,
        AutoLayout eNotesLayout,
        sal_Bool bIsPageBack,
        sal_Bool bIsPageObj,
        const sal_Int32 nInsertPosition = -1);

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
    sal_uInt16 DuplicatePage (sal_uInt16 nPageNum);

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
    sal_uInt16 DuplicatePage (
        SdPage* pCurrentPage,
        PageKind ePageKind,
        const String& sStandardPageName,
        const String& sNotesPageName,
        sal_Bool bIsPageBack,
        sal_Bool bIsPageObj,
        const sal_Int32 nInsertPosition = -1);

    /** return the document fonts for latin, cjk and ctl according to the current
        languages set at this document */
    void getDefaultFonts( Font& rLatinFont, Font& rCJKFont, Font& rCTLFont );

    sd::UndoManager* GetUndoManager() const;

    /** converts the given western font height to a corresponding ctl font height, deppending on the system language */
    static sal_uInt32 convertFontHeightToCTL( sal_uInt32 nWesternFontHeight );

    /** Get the style sheet pool if it was a SdStyleSheetPool.
     */
    SD_DLLPUBLIC SdStyleSheetPool* GetSdStyleSheetPool() const;

       void UpdatePageRelativeURLs(const String& rOldName, const String& rNewName);

    void SetCalcFieldValueHdl( ::Outliner* pOutliner);

    sal_uInt16 GetAnnotationAuthorIndex( const OUString& rAuthor );

    bool IsUsingEmbededFonts() { return mbUseEmbedFonts; }
    void SetIsUsingEmbededFonts( bool bUse ) { mbUseEmbedFonts = bUse; }

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
    sal_uInt16 InsertPageSet (
        SdPage* pCurrentPage,
        PageKind ePageKind,
        const String& sStandardPageName,
        const String& sNotesPageName,
        sal_Bool bIsPageBack,
        sal_Bool bIsPageObj,
        SdPage* pStandardPage,
        SdPage* pNotesPage,
        sal_Int32 nInsertPosition = -1);

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
    void SetupNewPage (
        SdPage* pPreviousPage,
        SdPage* pPage,
        const String& sPageName,
        sal_uInt16 nInsertionPoint,
        sal_Bool bIsPageBack,
        sal_Bool bIsPageObj);

    virtual void PageListChanged();
    virtual void MasterPageListChanged();
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
    sal_Bool mbIsEnableSetModified;
    sal_Bool mbIsDocumentChanged;
};

}

#endif // _DRAWDOC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
