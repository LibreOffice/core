/*************************************************************************
 *
 *  $RCSfile: drawdoc.hxx,v $
 *
 *  $Revision: 1.32 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 13:38:17 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _DRAWDOC_HXX
#define _DRAWDOC_HXX

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HDL_
#include <com/sun/star/frame/XModel.hdl>
#endif
#ifndef _SV_PRINT_HXX
#include <vcl/print.hxx>
#endif
#ifndef _FM_FMMODEL_HXX
#include <svx/fmmodel.hxx>
#endif
#ifndef _PRESENTATION_HXX
#include "pres.hxx"
#endif
#ifndef _SVX_PAGEITEM_HXX //autogen
#include <svx/pageitem.hxx>
#endif
#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif
#ifndef _TOOLS_INTN_HXX
#include <tools/intn.hxx>
#endif
#ifndef _SOT_STORAGE_HXX
#include <sot/storage.hxx>
#endif
#ifndef _RSCSFX_HXX
#include <rsc/rscsfx.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_WRITINGMODE_HPP_
#include <com/sun/star/text/WritingMode.hpp>
#endif

// #107844#
#ifndef _SVDUNDO_HXX
#include <svx/svdundo.hxx>
#endif
#include <memory>

#ifndef _SDPAGE_HXX
#include "sdpage.hxx"
#endif

namespace sd {
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

namespace sd {
#ifndef SV_DECL_DRAW_DOC_SHELL_DEFINED
#define SV_DECL_DRAW_DOC_SHELL_DEFINED
SV_DECL_REF(DrawDocShell)
#endif
class DrawDocShell;
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

//////////////////////////////////////////////////////////////////////////////
// #107844#
// An undo class which is able to set/unset user calls is needed to handle
// the undo/redo of PresObjs correctly. It can also add/remove the object
// from the PresObjList of that page.

class SdrUndoUserCallObj : public SdrUndoObj
{
protected:
    SdPage*                         mpOld;
    SdPage*                         mpNew;
    PresObjKind                     meKind;

public:
    SdrUndoUserCallObj(SdrObject& rNewObj, SdPage* pNew);
    SdrUndoUserCallObj(SdrObject& rNewObj, SdPage* pOld, SdPage* pNew);

    virtual void Undo();
    virtual void Redo();
};


// ------------------
// - SdDrawDocument -
// ------------------

class SdDrawDocument
    : public FmFormModel
{
private:

    ::sd::Outliner* pOutliner;          // local outliner for outline mode
    ::sd::Outliner* pInternalOutliner;  // internal outliner for creation of text objects
    Timer*              pWorkStartupTimer;
    Timer*              pOnlineSpellingTimer;
    List*               pOnlineSpellingList;
    List*               pDeletedPresObjList;
    SvxSearchItem*      pOnlineSearchItem;
    List*               pFrameViewList;
    List*               pCustomShowList;
    ::sd::DrawDocShell*     pDocSh;
    SdTransferable *    pCreatingTransferable;
    BOOL                bHasOnlineSpellErrors;
    BOOL                bInitialOnlineSpellingEnabled;
    String              aBookmarkFile;
    ::sd::DrawDocShellRef   xBookmarkDocShRef;
    String              aPresPage;
    BOOL                bNewOrLoadCompleted;
    BOOL                bPresAll;
    BOOL                bPresEndless;
    BOOL                bPresManual;
    BOOL                bPresMouseVisible;
    BOOL                bPresMouseAsPen;
    BOOL                bStartPresWithNavigator;
    BOOL                bAnimationAllowed;
    BOOL                bPresLockedPages;
    BOOL                bPresAlwaysOnTop;
    BOOL                bPresFullScreen;
    ULONG               nPresPause;
    BOOL                bPresShowLogo;
    BOOL                bOnlineSpell;
    BOOL                bHideSpell;
    BOOL                bCustomShow;
    BOOL                bSummationOfParagraphs;
    bool                mbStartWithPresentation;        // is set to true when starting with command line parameter -start

    ULONG               nPresFirstPage;
    LanguageType        eLanguage;
    LanguageType        eLanguageCJK;
    LanguageType        eLanguageCTL;
    SvxNumType          ePageNumType;
    Link                aOldNotifyUndoActionHdl;
    ::sd::DrawDocShellRef   xAllocedDocShRef;   // => AllocModel()
    BOOL                bAllocDocSh;        // => AllocModel()
    DocumentType        eDocType;
    UINT16              nFileFormatVersion;
    SotStorage*         pDocStor;
    SotStorageRef       xPictureStorage;
    SotStorageStreamRef xDocStream;
    International*      mpInternational;
    CharClass*          mpCharClass;
    ::com::sun::star::lang::Locale* mpLocale;

    // #109538#
    ::std::auto_ptr<ImpDrawPageListWatcher> mpDrawPageListWatcher;
    ::std::auto_ptr<ImpMasterPageListWatcher> mpMasterPageListWatcher;

    void                UpdatePageObjectsInNotes(USHORT nStartPos);
    void                FillOnlineSpellingList(SdPage* pPage);
    void                SpellObject(SdrTextObj* pObj);

                        DECL_LINK(NotifyUndoActionHdl, SfxUndoAction*);
                        DECL_LINK(WorkStartupHdl, Timer*);
                        DECL_LINK(OnlineSpellingHdl, Timer*);
                        DECL_LINK(OnlineSpellEventHdl, EditStatus*);

protected:

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > createUnoModel();

public:

    class InsertBookmarkAsPage_PageFunctorBase;

                        TYPEINFO();

                        SdDrawDocument(DocumentType eType, SfxObjectShell* pDocSh);
                        ~SdDrawDocument();

    virtual SdrModel*   AllocModel() const;
    virtual SdrPage*    AllocPage(FASTBOOL bMasterPage);
    virtual const SdrModel* LoadModel(const String& rFileName);
    virtual void        DisposeLoadedModels();
    virtual FASTBOOL    IsReadOnly() const;
    virtual void        SetChanged(sal_Bool bFlag = sal_True);
    virtual SvStream*   GetDocumentStream(SdrDocumentStreamInfo& rStreamInfo) const;
    virtual void        HandsOff();

    SfxItemPool&        GetPool() { return( *pItemPool ); }

    ::sd::Outliner* GetOutliner(BOOL bCreateOutliner=TRUE);
    ::sd::Outliner* GetInternalOutliner(BOOL bCreateOutliner=TRUE);

    ::sd::DrawDocShell*     GetDocSh() const { return(pDocSh) ; }

    LanguageType        GetLanguage( const USHORT nId ) const;
    void                SetLanguage( const LanguageType eLang, const USHORT nId );

    SvxNumType          GetPageNumType() const;
    void                SetPageNumType(SvxNumType eType) { ePageNumType = eType; }
    String              CreatePageNumValue(USHORT nNum) const;

    DocumentType        GetDocumentType() const { return eDocType; }

    void                SetAllocDocSh(BOOL bAlloc);

    void                CreatingDataObj( SdTransferable* pTransferable ) { pCreatingTransferable = pTransferable; }

    void                CreateFirstPages();
    BOOL                CreateMissingNotesAndHandoutPages();

    void                MovePage(USHORT nPgNum, USHORT nNewPos);
    void                InsertPage(SdrPage* pPage, USHORT nPos=0xFFFF);
    void                DeletePage(USHORT nPgNum);
    SdrPage*            RemovePage(USHORT nPgNum);
    void                RemoveUnnessesaryMasterPages( SdPage* pMaster=NULL, BOOL bOnlyDuplicatePages=FALSE, BOOL bUndo=TRUE );
    void                SetMasterPage(USHORT nSdPageNum, const String& rLayoutName,
                                      SdDrawDocument* pSourceDoc, BOOL bMaster, BOOL bCheckMasters);

    SdDrawDocument*     OpenBookmarkDoc(const String& rBookmarkFile);
    SdDrawDocument*     OpenBookmarkDoc(SfxMedium& rMedium);
    BOOL                InsertBookmark(List* pBookmarkList, List* pExchangeList, BOOL bLink,
                                        BOOL bReplace, USHORT nPgPos, BOOL bNoDialogs,
                                        ::sd::DrawDocShell* pBookmarkDocSh, BOOL bCopy,
                                        Point* pObjPos);

    bool IsStartWithPresentation() const;
    void SetStartWithPresentation( bool bStartWithPresentation );

    /** Insert pages into this document

        This method inserts whole pages into this document, either
        selected ones (specified via pBookmarkList/pExchangeList), or
        all from the source document.

        @attention Beware! This method in it's current state does not
        handle all combinations of their input parameters
        correctly. For example, for pBookmarkList=NULL, bReplace=TRUE
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
    BOOL                InsertBookmarkAsPage(List* pBookmarkList, List* pExchangeList,
                                              BOOL bLink, BOOL bReplace, USHORT nPgPos,
                                              BOOL bNoDialogs, ::sd::DrawDocShell* pBookmarkDocSh,
                                              BOOL bCopy, BOOL bMergeMasterPages,
                                              BOOL bPreservePageNames);
    BOOL                InsertBookmarkAsObject(List* pBookmarkList, List* pExchangeListL,
                                                BOOL bLink, ::sd::DrawDocShell* pBookmarkDocSh,
                                                Point* pObjPos);
    void                IterateBookmarkPages( SdDrawDocument* pBookmarkDoc, List* pBookmarkList,
                                              USHORT nBMSdPageCount,
                                              InsertBookmarkAsPage_PageFunctorBase& rPageIterator );
    void                CloseBookmarkDoc();

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
    USHORT GetPageByName(const String& rPgName, BOOL& rbIsMasterPage ) const;
    SdPage*             GetSdPage(USHORT nPgNum, PageKind ePgKind) const;
    USHORT              GetSdPageCount(PageKind ePgKind) const;
    void                SetSelected(SdPage* pPage, BOOL bSelect);
    BOOL                MovePages(USHORT nTargetPage);

    SdPage*             GetMasterSdPage(USHORT nPgNum, PageKind ePgKind);
    USHORT              GetMasterSdPageCount(PageKind ePgKind) const;

    USHORT              GetMasterPageUserCount(SdrPage* pMaster) const;

    void                SetPresPage( const String& rPresPage ) { aPresPage = rPresPage; }
    const String&       GetPresPage() const { return aPresPage; }

    void                SetPresAll(BOOL bNewPresAll);
    BOOL                GetPresAll() const       { return bPresAll; }

    void                SetPresEndless(BOOL bNewPresEndless);
    BOOL                GetPresEndless() const   { return bPresEndless; }

    void                SetPresManual(BOOL bNewPresManual);
    BOOL                GetPresManual() const        { return bPresManual; }

    void                SetPresMouseVisible(BOOL bNewPresMouseVisible);
    BOOL                GetPresMouseVisible() const { return bPresMouseVisible; }

    void                SetPresMouseAsPen(BOOL bNewPresMouseAsPen);
    BOOL                GetPresMouseAsPen() const    { return bPresMouseAsPen; }

    void                SetPresFirstPage (ULONG nNewFirstPage);
    ULONG               GetPresFirstPage() const { return nPresFirstPage; }

    void                SetStartPresWithNavigator (BOOL bStart);
    BOOL                GetStartPresWithNavigator() const { return bStartPresWithNavigator; }

    void                SetAnimationAllowed (BOOL bAllowed) { bAnimationAllowed = bAllowed; }
    BOOL                IsAnimationAllowed() const { return bAnimationAllowed; }

    void                SetPresPause( ULONG nSecondsToWait ) { nPresPause = nSecondsToWait; }
    ULONG               GetPresPause() const { return nPresPause; }

    void                SetPresShowLogo( BOOL bShowLogo ) { bPresShowLogo = bShowLogo; }
    BOOL                IsPresShowLogo() const { return bPresShowLogo; }

    void                SetPresLockedPages (BOOL bLock);
    BOOL                GetPresLockedPages() const { return bPresLockedPages; }

    void                SetPresAlwaysOnTop (BOOL bOnTop);
    BOOL                GetPresAlwaysOnTop() const { return bPresAlwaysOnTop; }

    void                SetPresFullScreen (BOOL bNewFullScreen);
    BOOL                GetPresFullScreen() const { return bPresFullScreen; }

       void                SetSummationOfParagraphs( BOOL bOn = TRUE ) { bSummationOfParagraphs = bOn; }
    const BOOL          IsSummationOfParagraphs() const { return bSummationOfParagraphs; }

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

    void                SetOnlineSpell( BOOL bIn );
    BOOL                GetOnlineSpell() const { return bOnlineSpell; }
    void                StopOnlineSpelling();
    void                StartOnlineSpelling(BOOL bForceSpelling=TRUE);

    void                ImpOnlineSpellCallback(SpellCallbackInfo* pInfo, SdrObject* pObj, SdrOutliner* pOutl);

    void                InsertObject(SdrObject* pObj, SdPage* pPage);
    void                RemoveObject(SdrObject* pObj, SdPage* pPage);

    void                SetHideSpell( BOOL bIn );
    BOOL                GetHideSpell() const { return bHideSpell; }

    ULONG               GetLinkCount();

    List*               GetFrameViewList() const { return pFrameViewList; }
    List*               GetCustomShowList(BOOL bCreate = FALSE);

    void                SetCustomShow(BOOL bCustShow) { bCustomShow = bCustShow; }
    BOOL                IsCustomShow() const { return bCustomShow; }

    void                NbcSetChanged(sal_Bool bFlag = sal_True);

    void                SetTextDefaults() const;

    void                CreateLayoutTemplates();
    void                RenameLayoutTemplate(const String& rOldLayoutName, const String& rNewName);

    void                StopWorkStartupDelay();

    void                NewOrLoadCompleted(DocCreationMode eMode);
    void                NewOrLoadCompleted( SdPage* pPage, SdStyleSheetPool* pSPool );
    BOOL                IsNewOrLoadCompleted() const {return bNewOrLoadCompleted; }

    ::sd::FrameView* GetFrameView(ULONG nPos) {
        return static_cast< ::sd::FrameView*>(
            pFrameViewList->GetObject(nPos));}

    SdAnimationInfo*    GetAnimationInfo(SdrObject* pObject) const;

    SdIMapInfo*         GetIMapInfo( SdrObject* pObject ) const;
    IMapObject*         GetHitIMapObject( SdrObject* pObject, const Point& rWinPoint, const ::Window& rCmpWnd );

    Graphic             GetGraphicFromOle2Obj( const SdrOle2Obj* pOle2Obj );

    List*               GetDeletedPresObjList();
    /** Clear the list of deleted presentation objects.  Call this
        method when the undo stack is also cleared.
    */
    void ClearDeletedPresObjList (void);

    CharClass*          GetCharClass() const { return mpCharClass; }
    International*      GetInternational() const { return mpInternational; }

    void                RestoreLayerNames();
    void                MakeUniqueLayerNames();

    void                UpdateAllLinks();

    void                CheckMasterPages();

    void                Merge(SdrModel& rSourceModel,
                                USHORT nFirstPageNum=0, USHORT nLastPageNum=0xFFFF,
                                USHORT nDestPos=0xFFFF,
                                FASTBOOL bMergeMasterPages=FALSE, FASTBOOL bAllMasterPages=FALSE,
                                FASTBOOL bUndo=TRUE, FASTBOOL bTreadSourceAsConst=FALSE);

    ::com::sun::star::text::WritingMode GetDefaultWritingMode() const;
    void SetDefaultWritingMode( ::com::sun::star::text::WritingMode eMode );

public:

    static SdDrawDocument* pDocLockedInsertingLinks;  // static to prevent recursions while resolving links

    friend SvStream&    operator<<(SvStream& rOut, SdDrawDocument& rDoc);
    friend SvStream&    operator>>(SvStream& rIn, SdDrawDocument& rDoc);


    /** This method acts as a simplified front end for the more complex
        <member>CreatePage()</member> method.
        @param nPageNum
            The page number as passed to the <member>GetSdPage()</member>
            method from which to use certain properties for the new pages.
            These include the auto layout.
        @return
            Returns an index of the inserted pages that can be used with the
            <member>GetSdPage()</member> method.
    */
    USHORT CreatePage (USHORT nPageNum);

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

        @return
            Returns an index of the inserted pages that can be used with the
            <member>GetSdPage()</member> method.
    */
    USHORT CreatePage (
        SdPage* pCurrentPage,
        PageKind ePageKind,
        const String& sStandardPageName,
        const String& sNotesPageName,
        AutoLayout eStandardLayout,
        AutoLayout eNotesLayout,
        BOOL bIsPageBack,
        BOOL bIsPageObj);

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
    USHORT DuplicatePage (USHORT nPageNum);

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

        @return
            Returns an index of the inserted pages that can be used with the
            <member>GetSdPage()</member> method.
    */
    USHORT DuplicatePage (
        SdPage* pCurrentPage,
        PageKind ePageKind,
        const String& sStandardPageName,
        const String& sNotesPageName,
        AutoLayout eStandardLayout,
        AutoLayout eNotesLayout,
        BOOL bIsPageBack,
        BOOL bIsPageObj);

    /** return the document fonts for latin, cjk and ctl according to the current
        languages set at this document */
    void getDefaultFonts( Font& rLatinFont, Font& rCJKFont, Font& rCTLFont );

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

        @return
            Returns an index of the inserted pages that can be used with the
            <member>GetSdPage()</member> method.
    */
    USHORT InsertPageSet (
        SdPage* pCurrentPage,
        PageKind ePageKind,
        const String& sStandardPageName,
        const String& sNotesPageName,
        AutoLayout eStandardLayout,
        AutoLayout eNotesLayout,
        BOOL bIsPageBack,
        BOOL bIsPageObj,

        SdPage* pStandardPage,
        SdPage* pNotesPage);

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
        USHORT nInsertionPoint,
        BOOL bIsPageBack,
        BOOL bIsPageObj);

    // #109538#
    virtual void PageListChanged();
    virtual void MasterPageListChanged();
};

#endif // _DRAWDOC_HXX
