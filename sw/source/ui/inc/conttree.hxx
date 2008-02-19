/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: conttree.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: rt $ $Date: 2008-02-19 13:56:13 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _CONTTREE_HXX
#define _CONTTREE_HXX

#ifndef _SVTREEBOX_HXX
#include <svtools/svtreebx.hxx>
#endif

#ifndef _SWCONT_HXX
#include "swcont.hxx"
#endif

class SwWrtShell;
class SwContentType;
class SwNavigationPI;
class SwNavigationConfig;
class Menu;
class ToolBox;
class SwGlblDocContents;
class SwGlblDocContent;
class SfxObjectShell;


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
    USHORT              nHiddenBlock;
    USHORT              nRootType;
    USHORT              nLastSelType;
    BYTE                nOutlineLevel;

    BOOL                bIsActive           :1;
    BOOL                bIsConstant         :1;
    BOOL                bIsHidden           :1;
    BOOL                bDocChgdInDragging  :1;
    BOOL                bIsInternalDrag     :1;
    BOOL                bIsRoot             :1;
    BOOL                bIsIdleClear        :1;
    BOOL                bIsLastReadOnly     :1;
    BOOL                bIsOutlineMoveable  :1;
    BOOL                bViewHasChanged     :1;
    BOOL                bIsImageListInitialized : 1;

    static BOOL         bIsInDrag;

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
    BOOL            HasContentChanged();

    virtual DragDropMode NotifyStartDrag( TransferDataContainer& rData,
                                        SvLBoxEntry* );
    virtual BOOL    NotifyAcceptDrop( SvLBoxEntry* );

    virtual BOOL    NotifyMoving(   SvLBoxEntry*  pTarget,
                                    SvLBoxEntry*  pEntry,
                                    SvLBoxEntry*& rpNewParent,
                                    ULONG&        rNewChildPos
                                );
    virtual BOOL    NotifyCopying(  SvLBoxEntry*  pTarget,
                                    SvLBoxEntry*  pEntry,
                                    SvLBoxEntry*& rpNewParent,
                                    ULONG&        rNewChildPos
                                );
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );

    void            EditEntry( SvLBoxEntry* pEntry, BYTE nMode );

    void            GotoContent(SwContent* pCnt);
    static void     SetInDrag(BOOL bSet) {bIsInDrag = bSet;}

    virtual PopupMenu* CreateContextMenu( void );
    virtual void    ExcecuteContextMenuAction( USHORT nSelectedPopupEntry );

public:
    SwContentTree(Window* pParent, const ResId& rResId);
    ~SwContentTree();

    BOOL            ToggleToRoot();
    BOOL            IsRoot() const {return bIsRoot;}
    USHORT          GetRootType() const {return nRootType;}
    void            SetRootType(USHORT nType);
    void            Display( BOOL bActiveView );
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

    static BOOL     IsInDrag() {return bIsInDrag;}
    BOOL            IsInternalDrag() const {return bIsInternalDrag != 0;}

    sal_Int32       GetActiveBlock() const {return nActiveBlock;}

    BYTE            GetOutlineLevel()const {return nOutlineLevel;}
    void            SetOutlineLevel(BYTE nSet);

    BOOL            Expand( SvLBoxEntry* pParent );

    BOOL            Collapse( SvLBoxEntry* pParent );

    void            ExecCommand(USHORT nCmd, BOOL bModifier);

    void            ShowTree();
    void            HideTree();

    BOOL            IsConstantView() {return bIsConstant;}
    BOOL            IsActiveView()   {return bIsActive;}
    BOOL            IsHiddenView()   {return bIsHidden;}

    const SwWrtShell*   GetActiveWrtShell() {return pActiveShell;}
    SwWrtShell*         GetHiddenWrtShell() {return pHiddenShell;}

    DECL_LINK( ContentDoubleClickHdl, SwContentTree * );
//  DECL_LINK( PopupHdl, Menu* );
    DECL_LINK( TimerUpdate, Timer * );

    virtual long    GetTabPos( SvLBoxEntry*, SvLBoxTab* );
    virtual void    RequestingChilds( SvLBoxEntry* pParent );
    virtual void    GetFocus();
    virtual void    KeyInput(const KeyEvent& rKEvt);

    virtual BOOL    Select( SvLBoxEntry* pEntry, BOOL bSelect=TRUE );
};


//----------------------------------------------------------------------------
// TreeListBox fuer Globaldokumente
//----------------------------------------------------------------------------

class SwLBoxString : public SvLBoxString
{
public:

    SwLBoxString( SvLBoxEntry* pEntry, USHORT nFlags,
        const String& rStr ) : SvLBoxString(pEntry,nFlags,rStr)
    {
    }

    virtual void Paint( const Point& rPos, SvLBox& rDev, USHORT nFlags,
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

    BOOL                bIsInternalDrag     :1;
    BOOL                bLastEntryEmphasis  :1; // Drag'n Drop
    BOOL                bIsImageListInitialized : 1;

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
    virtual BOOL    NotifyMoving(   SvLBoxEntry*  pTarget,
                                    SvLBoxEntry*  pEntry,
                                    SvLBoxEntry*& rpNewParent,
                                    ULONG&        rNewChildPos
                                );
    virtual BOOL    NotifyCopying(  SvLBoxEntry*  pTarget,
                                    SvLBoxEntry*  pEntry,
                                    SvLBoxEntry*& rpNewParent,
                                    ULONG&        rNewChildPos
                                );

    virtual void    StartDrag( sal_Int8 nAction, const Point& rPosPixel );
    virtual void    DragFinished( sal_Int8 );
    virtual DragDropMode NotifyStartDrag( TransferDataContainer& rData,
                                        SvLBoxEntry* );
    virtual BOOL    NotifyAcceptDrop( SvLBoxEntry* );

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

    BOOL            IsInternalDrag() const {return bIsInternalDrag != 0;}
    SwNavigationPI* GetParentWindow()
                        { return (SwNavigationPI*)Window::GetParent(); }

    void            OpenDoc(const SwGlblDocContent*);
    void            GotoContent(const SwGlblDocContent*);
    USHORT          GetEnableFlags() const;

    static const SfxObjectShell*    GetShowShell() {return pShowShell;}
    static void     SetShowShell(const SfxObjectShell*pSet) {pShowShell = pSet;}
    DECL_STATIC_LINK(SwGlobalTree, ShowFrameHdl, SwGlobalTree*);

    virtual PopupMenu* CreateContextMenu( void );
    virtual void    ExcecuteContextMenuAction( USHORT nSelectedPopupEntry );

public:
    SwGlobalTree(Window* pParent, const ResId& rResId);
    virtual ~SwGlobalTree();

    void                TbxMenuHdl(USHORT nTbxId, ToolBox* pBox);
    void                InsertRegion( const SwGlblDocContent* pCont,
                                        const String* pFileName = 0 );
    void                EditContent(const SwGlblDocContent* pCont );

    void                ShowTree();
    void                HideTree();

    void                ExecCommand(USHORT nCmd);

    void                Display(BOOL bOnlyUpdateUserData = FALSE);

    BOOL                Update(BOOL bHard);
};

#endif

