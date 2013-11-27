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



#ifndef _SDTREELB_HXX
#define _SDTREELB_HXX

#include <svtools/transfer.hxx>

#ifndef _SD_SDRESID_HXX
#include "sdresid.hxx"
#endif
#include "pres.hxx"
#include "sddllapi.h"
#include <tools/string.hxx>
#include <svtools/svtreebx.hxx>
#include <svl/urlbmk.hxx>
#include <tools/ref.hxx>
#include "sdxfer.hxx"
//IAccessibility2 Implementation 2009-----
#include <vector>
using namespace std;
//-----IAccessibility2 Implementation 2009
#include <boost/scoped_ptr.hpp>
#include <boost/function.hpp>

class SdDrawDocument;
class SfxMedium;
class SfxViewFrame;
class SdNavigatorWin;
class SdrObject;
class SdrObjList;
class SdPage;
class SvLBoxEntry;

namespace sd {
class ViewShell;

class DrawDocShell;
#ifndef SV_DECL_DRAW_DOC_SHELL_DEFINED
#define SV_DECL_DRAW_DOC_SHELL_DEFINED
SV_DECL_REF(DrawDocShell)
#endif
}

/*************************************************************************
|*
|* Effekte-Tab-Dialog
|*
\************************************************************************/

class SD_DLLPUBLIC SdPageObjsTLB : public SvTreeListBox
{
private:
    static bool  SD_DLLPRIVATE bIsInDrag;      // static, falls der Navigator im ExecuteDrag geloescht wird

//IAccessibility2 Implementation 2009-----
    //Solution: set contenttree in SdNavigatorWin
    sal_Bool                           bisInSdNavigatorWin;
//-----IAccessibility2 Implementation 2009
public:

    // nested class to implement the TransferableHelper
    class SdPageObjsTransferable : public SdTransferable
    {
    public:
        SdPageObjsTransferable(
            SdPageObjsTLB& rParent,
            const INetBookmark& rBookmark,
            ::sd::DrawDocShell& rDocShell,
            NavigatorDragType eDragType,
            const ::com::sun::star::uno::Any& rTreeListBoxData );
        ::sd::DrawDocShell&     GetDocShell() const;
        NavigatorDragType   GetDragType() const;

        static const ::com::sun::star::uno::Sequence< sal_Int8 >& getUnoTunnelId();
        static SdPageObjsTransferable* getImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rxData ) throw();
        /** Return a temporary transferable data flavor that is used
            internally in the navigator for reordering entries.  Its
            lifetime ends with the office application.
        */
        static sal_uInt32 GetListBoxDropFormatId (void);

    private:
        /** Temporary drop flavor id that is used internally in the
            navigator.
        */
        static sal_uInt32 mnListBoxDropFormatId;

        SdPageObjsTLB&      mrParent;
        INetBookmark        maBookmark;
        ::sd::DrawDocShell&     mrDocShell;
        NavigatorDragType   meDragType;
        const ::com::sun::star::uno::Any maTreeListBoxData;
        SD_DLLPRIVATE virtual               ~SdPageObjsTransferable();

        SD_DLLPRIVATE virtual void      AddSupportedFormats();
        SD_DLLPRIVATE virtual sal_Bool  GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
        SD_DLLPRIVATE virtual void      DragFinished( sal_Int8 nDropAction );

        SD_DLLPRIVATE virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rId ) throw( ::com::sun::star::uno::RuntimeException );
    };

    friend class SdPageObjsTLB::SdPageObjsTransferable;

    /** Determine whether the specified page belongs to the current show
        which is either the standard show or a custom show.
        @param pPage
            Pointer to the page for which to check whether it belongs to the
            show.
        @return
            Returns <false/> if there is no custom show or if the current
            show does not contain the specified page at least once.
    */
    bool PageBelongsToCurrentShow (const SdPage* pPage) const;

protected:

    Window*                 mpParent;
    const SdDrawDocument*   mpDoc;
    SdDrawDocument*         mpBookmarkDoc;
    SfxMedium*              mpMedium;
    SfxMedium*              mpOwnMedium;
    Image                   maImgOle;
    Image                   maImgGraphic;
    Image                   maImgOleH;
    Image                   maImgGraphicH;
    bool                    mbLinkableSelected;
    bool                    mbDragEnabled;
    String                  maDocName;
    ::sd::DrawDocShellRef       mxBookmarkDocShRef; // Zum Laden von Bookmarks
    ::sd::DrawDocShell*         mpDropDocSh;
    SdNavigatorWin*         mpDropNavWin;
    SfxViewFrame*           mpFrame;
    //IAccessibility2 Implementation 2009-----
    vector<String>          maTreeItem;
    sal_Bool                    mbSaveTreeItemState;
    String                  maSelectionEntryText;
    //-----IAccessibility2 Implementation 2009

    // DragSourceHelper
    virtual void            StartDrag( sal_Int8 nAction, const Point& rPosPixel );

    // DropTargetHelper
    virtual sal_Int8        AcceptDrop( const AcceptDropEvent& rEvt );
    virtual sal_Int8        ExecuteDrop( const ExecuteDropEvent& rEvt );

    virtual void            RequestingChilds( SvLBoxEntry* pParent );

    void                    DoDrag();
    void                    OnDragFinished( sal_uInt8 nDropAction );

    /** Return the name of the object.  When the object has no user supplied
        name and the bCreate flag is <true/> then a name is created
        automatically.  Additionally the mbShowAllShapes flag is taken into
        account when there is no user supplied name.  When this flag is
        <false/> then no name is created.
        @param pObject
            When this is NULL then an empty string is returned, regardless
            of the value of bCreate.
        @param bCreate
            This flag controls for objects without user supplied name
            whether a name is created.  When a name is created then this
            name is not stored in the object.
    */
    String GetObjectName (
        const SdrObject* pObject,
        const bool bCreate = true) const;
    void                    CloseBookmarkDoc();
                            DECL_STATIC_LINK(SdPageObjsTLB, ExecDragHdl, void*);

    /** Handle the reordering of entries in the navigator.  This method
        reorders both the involved shapes in their page as well as the
        associated list box entries.
    */
    virtual sal_Bool NotifyMoving(
        SvLBoxEntry*  pTarget,
        SvLBoxEntry*  pEntry,
        SvLBoxEntry*& rpNewParent,
        sal_uLong&        rNewChildPos);

    using Window::GetDropTarget;
    virtual SvLBoxEntry* GetDropTarget (const Point& rLocation);
//IAccessibility2 Implementation 2009-----
    virtual void    InitEntry(SvLBoxEntry*,const XubString&,const Image&,const Image&,SvLBoxButtonKind );
//-----IAccessibility2 Implementation 2009

public:

                            SdPageObjsTLB( Window* pParent, const SdResId& rSdResId );
                            ~SdPageObjsTLB();
//IAccessibility2 Implementation 2009-----
   // helper function for   GetEntryAltText and GetEntryLongDescription
    String          getAltLongDescText( SvLBoxEntry* pEntry , sal_Bool isAltText) const;
    String          GetEntryAltText( SvLBoxEntry* pEntry ) const;
    String          GetEntryLongDescription( SvLBoxEntry* pEntry ) const;
//-----IAccessibility2 Implementation 2009
    virtual void            SelectHdl();
    virtual void            KeyInput( const KeyEvent& rKEvt );

    void                    SetViewFrame( SfxViewFrame* pViewFrame );
    SfxViewFrame*           GetViewFrame() const { return mpFrame; }

    void                    Fill( const SdDrawDocument*, bool bAllPages, const String& rDocName );
    void                    Fill( const SdDrawDocument*, SfxMedium* pSfxMedium, const String& rDocName );
    void                    SetShowAllShapes (const bool bShowAllShapes, const bool bFill);
    bool                    GetShowAllShapes (void) const;
    bool                    IsEqualToDoc( const SdDrawDocument* pInDoc = NULL );
    bool                    HasSelectedChilds( const String& rName );
    bool                    SelectEntry( const String& rName );
    String                  GetSelectEntry();
//IAccessibility2 Implementation 2009-----
    //Solution: Mark Current Entry
    void                    MarkCurEntry( const String& rName );
    void                    SetSdNavigatorWinFlag(sal_Bool isInSdNavigatorWin){bisInSdNavigatorWin =isInSdNavigatorWin;};
    void                    FreshCurEntry();

    void                    Clear();
    void                    SetSaveTreeItemStateFlag(sal_Bool bState){mbSaveTreeItemState = bState;}
    void                    SaveExpandedTreeItemState(SvLBoxEntry* pEntry, vector<String>& vectTreeItem);
//-----IAccessibility2 Implementation 2009
    List*                   GetSelectEntryList( sal_uInt16 nDepth );
    SdDrawDocument*         GetBookmarkDoc(SfxMedium* pMedium = NULL);
    ::sd::DrawDocShell*         GetDropDocSh() { return(mpDropDocSh); }

    bool                    IsLinkableSelected() const;

    static bool             IsInDrag();
    using SvLBox::ExecuteDrop;

    using SvTreeListBox::SelectEntry;

    /** Return the view shell that is linked to the given doc shell.
        Call this method when the there is a chance that the doc shell
        has been disconnected from the view shell (but not the other
        way round.)
        @return
            May return <NULL/> when the link between view shell and
            doc shell has been severed.
    */
    static ::sd::ViewShell* GetViewShellForDocShell (::sd::DrawDocShell &rDocShell);

private:
    /** This flag controls whether all shapes are shown as children of pages
        and group shapes or only the named shapes.
    */
    bool mbShowAllShapes;
    /** This flag controls whether to show all pages.
    */
    bool mbShowAllPages;

    /** Return <true/> when the current transferable may be dropped at the
        given list box entry.
    */
    bool IsDropAllowed (SvLBoxEntry* pEntry);

    /** This inner class is defined in sdtreelb.cxx and is basically a
        container for the icons used in the list box for the entries.
    */
    class IconProvider;

    /** Add one list box entry for the parent of the given shapes and one child entry for
        each of the given shapes.
        @param rList
            The container of shapes that are to be inserted.
        @param pShape
            The parent shape or NULL when the parent is a page.
        @param rsName
            The name to be displayed for the new parent node.
        @param bIsExcluded
            Some pages can be excluded (from the show?).
        @param pParentEntry
            The parent entry of the new parent entry.
        @param rIconProvider
            Icons used to visualize the different shape and page types.
    */
    void AddShapeList (
        const SdrObjList& rList,
        SdrObject* pShape,
        const ::rtl::OUString& rsName,
        const bool bIsExcluded,
        SvLBoxEntry* pParentEntry,
        const IconProvider& rIconProvider);

    /** Add the given object to a transferable object so that the object can
        be dragged and dropped without having a name.
    */
    void AddShapeToTransferable (
        SdTransferable& rTransferable,
        SdrObject& rObject) const;
};

#endif      // _SDTREELB_HXX
