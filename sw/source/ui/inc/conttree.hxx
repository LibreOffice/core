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


#ifndef _CONTTREE_HXX
#define _CONTTREE_HXX

#include <svtools/svtreebx.hxx>
#include "swcont.hxx"

class SwWrtShell;
class SwContentType;
class SwNavigationPI;
class SwNavigationConfig;
class Menu;
class ToolBox;
class SwGlblDocContents;
class SwGlblDocContent;
class SfxObjectShell;

//IAccessibility2 Implementation 2009-----
//Solution:  Include  SdrObject  class
class SdrObject;
//-----IAccessibility2 Implementation 2009

#define EDIT_MODE_EDIT          0
#define EDIT_MODE_UPD_IDX       1
#define EDIT_MODE_RMV_IDX       2
#define EDIT_UNPROTECT_TABLE    3
#define EDIT_MODE_DELETE        4
#define EDIT_MODE_RENAME        5

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

class SwContentTree : public SvTreeListBox
{
    ImageList           aEntryImages;
    String              sSpace;
    AutoTimer           aUpdTimer;

    SwContentType*      aActiveContentArr[CONTENT_TYPE_MAX];
    SwContentType*      aHiddenContentArr[CONTENT_TYPE_MAX];
    String              aContextStrings[CONTEXT_COUNT + 1];
    String              sRemoveIdx;
    String              sUpdateIdx;
    String              sUnprotTbl;
    String              sRename;
    String              sReadonlyIdx;
    String              sInvisible;
    String              sPostItShow;
    String              sPostItHide;
    String              sPostItDelete;

    SwWrtShell*         pHiddenShell;   // gedropptes Doc
    SwWrtShell*         pActiveShell;   // die aktive oder eine konst. offene View
    SwNavigationConfig* pConfig;

    sal_Int32           nActiveBlock;
    sal_uInt16              nHiddenBlock;
    sal_uInt16              nRootType;
    sal_uInt16              nLastSelType;
    sal_uInt8               nOutlineLevel;

    sal_Bool                bIsActive           :1;
    sal_Bool                bIsConstant         :1;
    sal_Bool                bIsHidden           :1;
    sal_Bool                bDocChgdInDragging  :1;
    sal_Bool                bIsInternalDrag     :1;
    sal_Bool                bIsRoot             :1;
    sal_Bool                bIsIdleClear        :1;
    sal_Bool                bIsLastReadOnly     :1;
    sal_Bool                bIsOutlineMoveable  :1;
    sal_Bool                bViewHasChanged     :1;
    sal_Bool                bIsImageListInitialized : 1;

    static sal_Bool         bIsInDrag;
    //IAccessibility2 Implementation 2009-----
    sal_Bool                bIsKeySpace;
    Rectangle           oldRectangle;
    //-----IAccessibility2 Implementation 2009
    void                FindActiveTypeAndRemoveUserData();

    using SvLBox::ExecuteDrop;
    using SvTreeListBox::EditEntry;
    using SvListView::Expand;
    using SvListView::Collapse;
    using SvListView::Select;

protected:
//  virtual void    Command( const CommandEvent& rCEvt );
    virtual void    RequestHelp( const HelpEvent& rHEvt );
    virtual void    InitEntry(SvLBoxEntry*,const XubString&,const Image&,const Image&,SvLBoxButtonKind);
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

    SwNavigationPI* GetParentWindow(){return
                        (SwNavigationPI*)Window::GetParent();}

    virtual void    StartDrag( sal_Int8 nAction, const Point& rPosPixel );
    virtual void    DragFinished( sal_Int8 );
    virtual sal_Int8 AcceptDrop( const AcceptDropEvent& rEvt );

    virtual sal_Int8 ExecuteDrop( const ExecuteDropEvent& rEvt );

    sal_Bool        FillTransferData( TransferDataContainer& rTransfer,
                                            sal_Int8& rDragMode );
    sal_Bool            HasContentChanged();

    virtual DragDropMode NotifyStartDrag( TransferDataContainer& rData,
                                        SvLBoxEntry* );
    virtual sal_Bool    NotifyAcceptDrop( SvLBoxEntry* );

    virtual sal_Bool    NotifyMoving(   SvLBoxEntry*  pTarget,
                                    SvLBoxEntry*  pEntry,
                                    SvLBoxEntry*& rpNewParent,
                                    sal_uLong&        rNewChildPos
                                );
    virtual sal_Bool    NotifyCopying(  SvLBoxEntry*  pTarget,
                                    SvLBoxEntry*  pEntry,
                                    SvLBoxEntry*& rpNewParent,
                                    sal_uLong&        rNewChildPos
                                );
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );

    void            EditEntry( SvLBoxEntry* pEntry, sal_uInt8 nMode );

    void            GotoContent(SwContent* pCnt);
    static void     SetInDrag(sal_Bool bSet) {bIsInDrag = bSet;}

    virtual PopupMenu* CreateContextMenu( void );
    virtual void    ExcecuteContextMenuAction( sal_uInt16 nSelectedPopupEntry );

public:
    SwContentTree(Window* pParent, const ResId& rResId);
    ~SwContentTree();
    //IAccessibility2 Implementation 2009-----
    String          GetEntryAltText( SvLBoxEntry* pEntry ) const;
    String          GetEntryLongDescription( SvLBoxEntry* pEntry ) const;
    SdrObject*  GetDrawingObjectsByContent(const SwContent *pCnt);
    //-----IAccessibility2 Implementation 2009

    sal_Bool            ToggleToRoot();
    sal_Bool            IsRoot() const {return bIsRoot;}
    sal_uInt16          GetRootType() const {return nRootType;}
    void            SetRootType(sal_uInt16 nType);
    void            Display( sal_Bool bActiveView );
    void            Clear();
    void            SetHiddenShell(SwWrtShell* pSh);
    void            ShowHiddenShell();
    void            ShowActualView();
    void            SetActiveShell(SwWrtShell* pSh);
    void            SetConstantShell(SwWrtShell* pSh);

    SwWrtShell*     GetWrtShell()
                        {return bIsActive||bIsConstant ?
                                    pActiveShell :
                                        pHiddenShell;}

    static sal_Bool     IsInDrag() {return bIsInDrag;}
    sal_Bool            IsInternalDrag() const {return bIsInternalDrag != 0;}

    sal_Int32       GetActiveBlock() const {return nActiveBlock;}

    sal_uInt8           GetOutlineLevel()const {return nOutlineLevel;}
    void            SetOutlineLevel(sal_uInt8 nSet);

    sal_Bool            Expand( SvLBoxEntry* pParent );

    sal_Bool            Collapse( SvLBoxEntry* pParent );

    void            ExecCommand(sal_uInt16 nCmd, sal_Bool bModifier);

    void            ShowTree();
    void            HideTree();

    sal_Bool            IsConstantView() {return bIsConstant;}
    sal_Bool            IsActiveView()   {return bIsActive;}
    sal_Bool            IsHiddenView()   {return bIsHidden;}

    const SwWrtShell*   GetActiveWrtShell() {return pActiveShell;}
    SwWrtShell*         GetHiddenWrtShell() {return pHiddenShell;}

    DECL_LINK( ContentDoubleClickHdl, SwContentTree * );
//  DECL_LINK( PopupHdl, Menu* );
    DECL_LINK( TimerUpdate, Timer * );

    virtual long    GetTabPos( SvLBoxEntry*, SvLBoxTab* );
    virtual void    RequestingChilds( SvLBoxEntry* pParent );
    virtual void    GetFocus();
    virtual void    KeyInput(const KeyEvent& rKEvt);

    virtual sal_Bool    Select( SvLBoxEntry* pEntry, sal_Bool bSelect=sal_True );
    //IAccessibility2 Implementation 2009-----
    virtual sal_Int32  GetEntryRealChildsNum( SvLBoxEntry* pEntry ) const;
    //-----IAccessibility2 Implementation 2009
};


//----------------------------------------------------------------------------
// TreeListBox fuer Globaldokumente
//----------------------------------------------------------------------------

class SwLBoxString : public SvLBoxString
{
public:

    SwLBoxString( SvLBoxEntry* pEntry, sal_uInt16 nFlags,
        const String& rStr ) : SvLBoxString(pEntry,nFlags,rStr)
    {
    }

    virtual void Paint( const Point& rPos, SvLBox& rDev, sal_uInt16 nFlags,
        SvLBoxEntry* pEntry);
};

namespace sfx2 { class DocumentInserter; }
namespace sfx2 { class FileDialogHelper; }

class SwGlobalTree : public SvTreeListBox
{
private:
    AutoTimer           aUpdateTimer;
    String              aContextStrings[GLOBAL_CONTEXT_COUNT];

    ImageList           aEntryImages;

    SwWrtShell*             pActiveShell;   //
    SvLBoxEntry*            pEmphasisEntry; // Drag'n Drop-Emphasis
    SvLBoxEntry*            pDDSource;      // Quelle beim DnD
    SwGlblDocContents*      pSwGlblDocContents; // Array mit sortierten Inhalten

    Window*                 pDefParentWin;
    SwGlblDocContent*       pDocContent;
    sfx2::DocumentInserter* pDocInserter;

    sal_Bool                bIsInternalDrag     :1;
    sal_Bool                bLastEntryEmphasis  :1; // Drag'n Drop
    sal_Bool                bIsImageListInitialized : 1;

    static const SfxObjectShell* pShowShell;

    void        InsertRegion( const SwGlblDocContent* _pContent,
                              const com::sun::star::uno::Sequence< ::rtl::OUString >& _rFiles );

    DECL_LINK(  DialogClosedHdl, sfx2::FileDialogHelper* );

    using SvLBox::DoubleClickHdl;
    using SvLBox::ExecuteDrop;
    using Window::Update;

protected:

    virtual sal_Int8 AcceptDrop( const AcceptDropEvent& rEvt );

    virtual sal_Int8 ExecuteDrop( const ExecuteDropEvent& rEvt );

    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

    virtual void    RequestHelp( const HelpEvent& rHEvt );

    virtual long    GetTabPos( SvLBoxEntry*, SvLBoxTab* );
    virtual sal_Bool    NotifyMoving(   SvLBoxEntry*  pTarget,
                                    SvLBoxEntry*  pEntry,
                                    SvLBoxEntry*& rpNewParent,
                                    sal_uLong&        rNewChildPos
                                );
    virtual sal_Bool    NotifyCopying(  SvLBoxEntry*  pTarget,
                                    SvLBoxEntry*  pEntry,
                                    SvLBoxEntry*& rpNewParent,
                                    sal_uLong&        rNewChildPos
                                );

    virtual void    StartDrag( sal_Int8 nAction, const Point& rPosPixel );
    virtual void    DragFinished( sal_Int8 );
    virtual DragDropMode NotifyStartDrag( TransferDataContainer& rData,
                                        SvLBoxEntry* );
    virtual sal_Bool    NotifyAcceptDrop( SvLBoxEntry* );

    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    KeyInput(const KeyEvent& rKEvt);
    virtual void    GetFocus();
    virtual void    SelectHdl();
    virtual void    DeselectHdl();
    virtual void InitEntry(SvLBoxEntry*,const XubString&,const Image&,const Image&,SvLBoxButtonKind);

    void            Clear();

    DECL_LINK(      PopupHdl, Menu* );
    DECL_LINK(      Timeout, Timer* );
    DECL_LINK(      DoubleClickHdl, SwGlobalTree * );

    sal_Bool            IsInternalDrag() const {return bIsInternalDrag != 0;}
    SwNavigationPI* GetParentWindow()
                        { return (SwNavigationPI*)Window::GetParent(); }

    void            OpenDoc(const SwGlblDocContent*);
    void            GotoContent(const SwGlblDocContent*);
    sal_uInt16          GetEnableFlags() const;

    static const SfxObjectShell*    GetShowShell() {return pShowShell;}
    static void     SetShowShell(const SfxObjectShell*pSet) {pShowShell = pSet;}
    DECL_STATIC_LINK(SwGlobalTree, ShowFrameHdl, SwGlobalTree*);

    virtual PopupMenu* CreateContextMenu( void );
    virtual void    ExcecuteContextMenuAction( sal_uInt16 nSelectedPopupEntry );

public:
    SwGlobalTree(Window* pParent, const ResId& rResId);
    virtual ~SwGlobalTree();

    void                TbxMenuHdl(sal_uInt16 nTbxId, ToolBox* pBox);
    void                InsertRegion( const SwGlblDocContent* pCont,
                                        const String* pFileName = 0 );
    void                EditContent(const SwGlblDocContent* pCont );

    void                ShowTree();
    void                HideTree();

    void                ExecCommand(sal_uInt16 nCmd);

    void                Display(sal_Bool bOnlyUpdateUserData = sal_False);

    sal_Bool                Update(sal_Bool bHard);
};

#endif

