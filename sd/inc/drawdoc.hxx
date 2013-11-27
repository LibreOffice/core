/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _DRAWDOC_HXX
#define _DRAWDOC_HXX

#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/text/WritingMode.hpp>
#include <com/sun/star/frame/XModel.hdl>
#include <vcl/print.hxx>
#include <svx/fmmodel.hxx>
#include "pres.hxx"
#include <svx/pageitem.hxx>
#include <unotools/charclass.hxx>
#include <sot/storage.hxx>
#include <rsc/rscsfx.hxx>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/text/WritingMode.hpp>

// #107844#
#include <svx/svdundo.hxx>

#include <vector>

#ifndef INCLUDED_MEMORY
#include <memory>
#define INCLUDED_MEMORY
#endif
#include "sddllapi.h"
#include "sdpage.hxx"

namespace com
{
    namespace sun
    {
        namespace star
        {
            namespace embed
            {
                class XStorage;
            }
            namespace io
            {
                class XStream;
            }
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
class SdDrawDocShell;
class SdPage;
class SdAnimationInfo;
class SdIMapInfo;
class IMapObject;
class SdStyleSheetPool;
class SfxMedium;
class SvxSearchItem;
class SdrOle2Obj;
class EditStatus;
class Graphic;
class Point;
class Window;
class SdTransferable;
struct SpellCallbackInfo;
struct StyleRequestData;
class SdDrawDocument;

namespace sd
{
#ifndef SV_DECL_DRAW_DOC_SHELL_DEFINED
#define SV_DECL_DRAW_DOC_SHELL_DEFINED
SV_DECL_REF(DrawDocShell)
#endif
class DrawDocShell;
class UndoManager;
class ShapeList;
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
        rtl::OUString maPresPage;
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
//IAccessibility2 Implementation 2009-----
    String msDocAccTitle;
public:
    virtual void setDocAccTitle( const String& rTitle ) { msDocAccTitle = rTitle; }
    virtual const String getDocAccTitle() const { return msDocAccTitle; }
private:
    sal_Bool bReadOnly;
public:
    virtual void setDocReadOnly( sal_Bool b){ bReadOnly = b; }
    virtual sal_Bool getDocReadOnly() const { return bReadOnly; }
private:
//IAccessibility2 Implementation 2009-----
    ::sd::Outliner*     mpOutliner;         // local outliner for outline mode
    ::sd::Outliner*     mpInternalOutliner;  // internal outliner for creation of text objects
    Timer*              mpWorkStartupTimer;
    Timer*              mpOnlineSpellingTimer;
    sd::ShapeList*      mpOnlineSpellingList;
    SvxSearchItem*      mpOnlineSearchItem;
    List*               mpFrameViewList;
    List*               mpCustomShowList;
    ::sd::DrawDocShell* mpDocSh;
    SdTransferable *    mpCreatingTransferable;
    bool                mbHasOnlineSpellErrors;
    bool                mbInitialOnlineSpellingEnabled;
    String              maBookmarkFile;
    ::sd::DrawDocShellRef   mxBookmarkDocShRef;

    sd::PresentationSettings maPresentationSettings;

    ::com::sun::star::uno::Reference< ::com::sun::star::presentation::XPresentation2 > mxPresentation;

    bool                mbNewOrLoadCompleted;

    bool                mbOnlineSpell;
    bool                mbSummationOfParagraphs;
    bool                mbStartWithPresentation;        // is set to true when starting with command line parameter -start
    LanguageType        meLanguage;
    LanguageType        meLanguageCJK;
    LanguageType        meLanguageCTL;
    SvxNumType          mePageNumType;
    ::sd::DrawDocShellRef   mxAllocedDocShRef;   // => AllocModel()
    bool                mbAllocDocSh;       // => AllocModel()
    DocumentType        meDocType;
    CharClass*          mpCharClass;
    ::com::sun::star::lang::Locale* mpLocale;

    // #109538#
    ::std::auto_ptr<ImpDrawPageListWatcher> mpDrawPageListWatcher;
    ::std::auto_ptr<ImpMasterPageListWatcher> mpMasterPageListWatcher;

    void                UpdatePageObjectsInNotes(sal_uInt32 nStartPos);
    void                UpdatePageRelativeURLs(SdPage* pPage, sal_uInt32 nPos, sal_Int32 nIncrement);
    void                FillOnlineSpellingList(SdPage* pPage);
    void                SpellObject(const SdrTextObj* pObj);

                        DECL_LINK(WorkStartupHdl, Timer*);
                        DECL_LINK(OnlineSpellingHdl, Timer*);
                        DECL_LINK(OnlineSpellEventHdl, EditStatus*);

    std::vector< rtl::OUString > maAnnotationAuthors;

protected:

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > createUnoModel();

public:

    class InsertBookmarkAsPage_PageFunctorBase;

                        SdDrawDocument(DocumentType eType, SfxObjectShell* pDocSh);
                        ~SdDrawDocument();

    virtual SdrModel*   AllocModel() const;
    virtual SdrPage*    AllocPage(bool bMasterPage);
    virtual const SdrModel* LoadModel(const String& rFileName);
    virtual void        DisposeLoadedModels();
    virtual bool IsReadOnly() const;
    virtual void        SetChanged(bool bFlag = true);
    virtual SvStream*   GetDocumentStream(SdrDocumentStreamInfo& rStreamInfo) const;

//  SfxItemPool&        GetPool() { return( *pItemPool ); }

    ::sd::Outliner* GetOutliner(bool bCreateOutliner = true);
    SD_DLLPUBLIC ::sd::Outliner* GetInternalOutliner(bool bCreateOutliner = true);

    ::sd::DrawDocShell*     GetDocSh() const { return mpDocSh; }

    LanguageType        GetLanguage( const sal_uInt16 nId ) const;
    void                SetLanguage( const LanguageType eLang, const sal_uInt16 nId );

    SvxNumType          GetPageNumType() const;
    void                SetPageNumType(SvxNumType eType) { mePageNumType = eType; }
    SD_DLLPUBLIC String              CreatePageNumValue(sal_uInt32 nNum) const;

    DocumentType        GetDocumentType() const { return meDocType; }

    void                SetAllocDocSh(bool bAlloc);

    void                CreatingDataObj( SdTransferable* pTransferable ) { mpCreatingTransferable = pTransferable; }

    /** if the document does not contain at least one handout, one slide and one notes page with
        at least one master each this methods creates them.
        If a reference document is given, the sizes and border settings of that document are used
        for newly created slides.
    */
    SD_DLLPUBLIC void   CreateFirstPages( SdDrawDocument* pRefDocument = 0 );
    SD_DLLPUBLIC bool                CreateMissingNotesAndHandoutPages();

    void                MovePage(sal_uInt32 nPgNum, sal_uInt32 nNewPos);
    void                InsertPage(SdrPage* pPage, sal_uInt32 nPos = SDRPAGE_NOTFOUND);
    void                DeletePage(sal_uInt32 nPgNum);
    SdrPage*            RemovePage(sal_uInt32 nPgNum);

    virtual void     InsertMasterPage(SdrPage* pPage, sal_uInt32 nPos = SDRPAGE_NOTFOUND);
    virtual SdrPage* RemoveMasterPage(sal_uInt32 nPgNum);

    void                RemoveUnnecessaryMasterPages( SdPage* pMaster=NULL, bool bOnlyDuplicatePages = false, bool bUndo = true);
    SD_DLLPUBLIC void   SetMasterPage(sal_uInt32 nSdPageNum, const String& rLayoutName, SdDrawDocument* pSourceDoc, bool bMaster, bool bCheckMasters);

    SD_DLLPUBLIC SdDrawDocument* OpenBookmarkDoc(const String& rBookmarkFile);
    SdDrawDocument*     OpenBookmarkDoc(SfxMedium& rMedium);
    bool InsertBookmark(List* pBookmarkList, List* pExchangeList, bool bLink, bool bReplace, sal_uInt32 nPgPos,
        bool bNoDialogs, ::sd::DrawDocShell* pBookmarkDocSh, bool bCopy, basegfx::B2DPoint* pObjPos);

    bool IsStartWithPresentation() const;
    void SetStartWithPresentation( bool bStartWithPresentation );

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
    bool InsertBookmarkAsPage(List* pBookmarkList, List* pExchangeList, bool bLink, bool bReplace, sal_uInt32 nPgPos,
        bool bNoDialogs, ::sd::DrawDocShell* pBookmarkDocSh, bool bCopy, bool bMergeMasterPages, bool bPreservePageNames);

    bool InsertBookmarkAsObject(List* pBookmarkList, List* pExchangeListL, bool bLink, ::sd::DrawDocShell* pBookmarkDocSh,
        basegfx::B2DPoint* pObjPos);
    void                IterateBookmarkPages( SdDrawDocument* pBookmarkDoc, List* pBookmarkList,
                                              sal_uInt32 nBMSdPageCount,
                                              InsertBookmarkAsPage_PageFunctorBase& rPageIterator );
    SD_DLLPUBLIC void   CloseBookmarkDoc();

    SdrObject*          GetObj(const String& rObjName) const;

    /** Return the first page that has the given name.  Regular pages and
        notes pages are searched first.  When not found then the master
        pages are searched.
        @param rPgName
            Name of the page to return.
        @param rbIsMasterPage
            Is set by the method to indicate whether the returned index
            belongs to a master page (<true/>) or a regular or notes page
            (<false/>). The given value is ignored.
        @return
            Returns the index of the page with the given name or
            SDRPAGE_NOTFOUND (=0xffffffff) when such a page does not exist.
    */
    sal_uInt32 GetPageByName(const String& rPgName, bool& rbIsMasterPage ) const;
    SD_DLLPUBLIC SdPage*GetSdPage(sal_uInt32 nPgNum, PageKind ePgKind) const;
    SD_DLLPUBLIC sal_uInt32 GetSdPageCount(PageKind ePgKind) const;

    void                SetSelected(SdPage* pPage, bool bSelect);
    bool                MovePages(sal_uInt32 nTargetPage);

    SD_DLLPUBLIC SdPage*GetMasterSdPage(sal_uInt32 nPgNum, PageKind ePgKind);
    SD_DLLPUBLIC sal_uInt32 GetMasterSdPageCount(PageKind ePgKind) const;

    sal_uInt32 GetMasterPageUserCount(SdrPage* pMaster) const;

    const sd::PresentationSettings& getPresentationSettings() const { return maPresentationSettings; }
    sd::PresentationSettings& getPresentationSettings() { return maPresentationSettings; }

    const ::com::sun::star::uno::Reference< ::com::sun::star::presentation::XPresentation2 >& getPresentation() const;

       void                SetSummationOfParagraphs( bool bOn = true ) { mbSummationOfParagraphs = bOn; }
    bool            IsSummationOfParagraphs() const { return mbSummationOfParagraphs; }

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

    void                SetOnlineSpell( bool bIn );
    bool                GetOnlineSpell() const { return mbOnlineSpell; }
    void                StopOnlineSpelling();
    void                StartOnlineSpelling(bool bForceSpelling = true);

    void                ImpOnlineSpellCallback(SpellCallbackInfo* pInfo, SdrObject* pObj, SdrOutliner* pOutl);

    void                InsertObject(const SdrObject* pObj);
    void                RemoveObject(const SdrObject* pObj);

    sal_uInt32          GetLinkCount();

    List*               GetFrameViewList() const { return mpFrameViewList; }
    SD_DLLPUBLIC List*  GetCustomShowList(bool bCreate = false);

    void                NbcSetChanged(bool bFlag = true);

    void                SetSDTextDefaults();

    void                CreateLayoutTemplates();
    void                RenameLayoutTemplate(const String& rOldLayoutName, const String& rNewName);

    void                CreateDefaultCellStyles();

    SD_DLLPUBLIC void   StopWorkStartupDelay();

    void                NewOrLoadCompleted(DocCreationMode eMode);
    void                NewOrLoadCompleted( SdPage* pPage, SdStyleSheetPool* pSPool );
    bool                IsNewOrLoadCompleted() const {return mbNewOrLoadCompleted; }

    ::sd::FrameView* GetFrameView(sal_uInt32 nPos) {
        return static_cast< ::sd::FrameView*>(
            mpFrameViewList->GetObject(nPos));}

    /** deprecated*/
    SdAnimationInfo*    GetAnimationInfo(SdrObject* pObject) const;

    SD_DLLPUBLIC static     SdAnimationInfo* GetShapeUserData(const SdrObject& rObject, bool bCreate = false );

    SdIMapInfo*         GetIMapInfo( SdrObject* pObject ) const;
    IMapObject*         GetHitIMapObject( SdrObject* pObject, const basegfx::B2DPoint& rWinPoint, const ::Window& rCmpWnd );

    CharClass*          GetCharClass() const { return mpCharClass; }

    void                RestoreLayerNames();

    void                UpdateAllLinks();

    void                CheckMasterPages();

    void                Merge(SdrModel& rSourceModel,
                                sal_uInt32 nFirstPageNum=0, sal_uInt32 nLastPageNum = SDRPAGE_NOTFOUND,
                                sal_uInt32 nDestPos = SDRPAGE_NOTFOUND,
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
    sal_uInt32 CreatePage (
        SdPage* pCurrentPage,
        PageKind ePageKind,
        const String& sStandardPageName,
        const String& sNotesPageName,
        AutoLayout eStandardLayout,
        AutoLayout eNotesLayout,
        bool bIsPageBack,
        bool bIsPageObj,
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
    sal_uInt32 DuplicatePage (sal_uInt32 nPageNum);

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
    sal_uInt32 DuplicatePage (
        SdPage* pCurrentPage,
        PageKind ePageKind,
        const String& sStandardPageName,
        const String& sNotesPageName,
        AutoLayout eStandardLayout,
        AutoLayout eNotesLayout,
        bool bIsPageBack,
        bool bIsPageObj,
        const sal_Int32 nInsertPosition = -1);

    /** return the document fonts for latin, cjk and ctl according to the current
        languages set at this document */
    void getDefaultFonts( Font& rLatinFont, Font& rCJKFont, Font& rCTLFont );

    sd::UndoManager* GetUndoManager() const;

    /* converts the given western font height to a corresponding ctl font height, deppending on the system language */
    static sal_uInt32 convertFontHeightToCTL( sal_uInt32 nWesternFontHeight );

       void UpdatePageRelativeURLs(const String& rOldName, const String& rNewName);

    void SetCalcFieldValueHdl( ::Outliner* pOutliner);

    sal_uInt16 GetAnnotationAuthorIndex( const rtl::OUString& rAuthor );

    /** Get the style sheet pool if it was a SdStyleSheetPool.
     */
    SD_DLLPUBLIC SdStyleSheetPool* GetSdStyleSheetPool() const;

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
    sal_uInt32 InsertPageSet (
        SdPage* pCurrentPage,
        PageKind ePageKind,
        const String& sStandardPageName,
        const String& sNotesPageName,
        AutoLayout eStandardLayout,
        AutoLayout eNotesLayout,
        bool bIsPageBack,
        bool bIsPageObj,
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
        sal_uInt32 nInsertionPoint,
        bool bIsPageBack,
        bool bIsPageObj);

    // #109538#
//  virtual void PageListChanged();
//  virtual void MasterPageListChanged();
//IAccessibility2 Implementation 2009-----
    virtual ImageMap* GetImageMapForObject(SdrObject* pObj);
//-----IAccessibility2 Implementation 2009
};

namespace sd
{

// an instance of this guard disables modification of a document
// during its lifetime
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

#endif // _DRAWDOC_HXX
