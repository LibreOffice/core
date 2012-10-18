/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _CONTTREE_HXX
#define _CONTTREE_HXX

#include "svtools/treelistbox.hxx"
#include "svtools/svlbitm.hxx"
#include "swcont.hxx"

#include <map>

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

    SwWrtShell*         pHiddenShell;   // dropped Doc
    SwWrtShell*         pActiveShell;   // the active or a const. open view
    SwNavigationConfig* pConfig;

    std::map< void*, sal_Bool > mOutLineNodeMap;

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

    void                FindActiveTypeAndRemoveUserData();

    using SvTreeListBox::ExecuteDrop;
    using SvTreeListBox::EditEntry;
    using SvListView::Expand;
    using SvListView::Collapse;
    using SvListView::Select;

protected:
    virtual void    RequestHelp( const HelpEvent& rHEvt );
    virtual void    InitEntry(SvTreeListEntry*,const XubString&,const Image&,const Image&,SvLBoxButtonKind);
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
                                        SvTreeListEntry* );
    virtual sal_Bool    NotifyAcceptDrop( SvTreeListEntry* );

    virtual sal_Bool    NotifyMoving(   SvTreeListEntry*  pTarget,
                                    SvTreeListEntry*  pEntry,
                                    SvTreeListEntry*& rpNewParent,
                                    sal_uLong&        rNewChildPos
                                );
    virtual sal_Bool    NotifyCopying(  SvTreeListEntry*  pTarget,
                                    SvTreeListEntry*  pEntry,
                                    SvTreeListEntry*& rpNewParent,
                                    sal_uLong&        rNewChildPos
                                );
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );

    void            EditEntry( SvTreeListEntry* pEntry, sal_uInt8 nMode );

    void            GotoContent(SwContent* pCnt);
    static void     SetInDrag(sal_Bool bSet) {bIsInDrag = bSet;}

    virtual PopupMenu* CreateContextMenu( void );
    virtual void    ExcecuteContextMenuAction( sal_uInt16 nSelectedPopupEntry );

public:
    SwContentTree(Window* pParent, const ResId& rResId);
    ~SwContentTree();

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

    sal_Bool            Expand( SvTreeListEntry* pParent );

    sal_Bool            Collapse( SvTreeListEntry* pParent );

    void            ExecCommand(sal_uInt16 nCmd, sal_Bool bModifier);

    void            ShowTree();
    void            HideTree();

    sal_Bool            IsConstantView() {return bIsConstant;}
    sal_Bool            IsActiveView()   {return bIsActive;}
    sal_Bool            IsHiddenView()   {return bIsHidden;}

    const SwWrtShell*   GetActiveWrtShell() {return pActiveShell;}
    SwWrtShell*         GetHiddenWrtShell() {return pHiddenShell;}

    DECL_LINK( ContentDoubleClickHdl, void * );
    DECL_LINK( TimerUpdate, void * );

    virtual long    GetTabPos( SvTreeListEntry*, SvLBoxTab* );
    virtual void    RequestingChildren( SvTreeListEntry* pParent );
    virtual void    GetFocus();
    virtual void    KeyInput(const KeyEvent& rKEvt);

    virtual sal_Bool    Select( SvTreeListEntry* pEntry, sal_Bool bSelect=sal_True );
};


//----------------------------------------------------------------------------
// TreeListBox for global documents
//----------------------------------------------------------------------------

class SwLBoxString : public SvLBoxString
{
public:

    SwLBoxString( SvTreeListEntry* pEntry, sal_uInt16 nFlags,
        const String& rStr ) : SvLBoxString(pEntry,nFlags,rStr)
    {
    }

    virtual void Paint( const Point& rPos, SvTreeListBox& rDev, sal_uInt16 nFlags,
        SvTreeListEntry* pEntry);
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
    SvTreeListEntry*            pEmphasisEntry; // Drag'n Drop emphasis
    SvTreeListEntry*            pDDSource;      // source for Drag'n Drop
    SwGlblDocContents*      pSwGlblDocContents; // array with sorted content

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

    using SvTreeListBox::DoubleClickHdl;
    using SvTreeListBox::ExecuteDrop;
    using Window::Update;

protected:

    virtual sal_Int8 AcceptDrop( const AcceptDropEvent& rEvt );

    virtual sal_Int8 ExecuteDrop( const ExecuteDropEvent& rEvt );

    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

    virtual void    RequestHelp( const HelpEvent& rHEvt );

    virtual long    GetTabPos( SvTreeListEntry*, SvLBoxTab* );
    virtual sal_Bool    NotifyMoving(   SvTreeListEntry*  pTarget,
                                    SvTreeListEntry*  pEntry,
                                    SvTreeListEntry*& rpNewParent,
                                    sal_uLong&        rNewChildPos
                                );
    virtual sal_Bool    NotifyCopying(  SvTreeListEntry*  pTarget,
                                    SvTreeListEntry*  pEntry,
                                    SvTreeListEntry*& rpNewParent,
                                    sal_uLong&        rNewChildPos
                                );

    virtual void    StartDrag( sal_Int8 nAction, const Point& rPosPixel );
    virtual void    DragFinished( sal_Int8 );
    virtual DragDropMode NotifyStartDrag( TransferDataContainer& rData,
                                        SvTreeListEntry* );
    virtual sal_Bool    NotifyAcceptDrop( SvTreeListEntry* );

    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    KeyInput(const KeyEvent& rKEvt);
    virtual void    GetFocus();
    virtual void    SelectHdl();
    virtual void    DeselectHdl();
    virtual void InitEntry(SvTreeListEntry*,const XubString&,const Image&,const Image&,SvLBoxButtonKind);

    void            Clear();

    DECL_LINK(      PopupHdl, Menu* );
    DECL_LINK(      Timeout, void* );
    DECL_LINK(      DoubleClickHdl, void* );

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
