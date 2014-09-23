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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_CONTTREE_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_CONTTREE_HXX

#include <svtools/treelistbox.hxx>
#include <svtools/svlbitm.hxx>
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
class SdrObject;

#define EDIT_MODE_EDIT          0
#define EDIT_MODE_UPD_IDX       1
#define EDIT_MODE_RMV_IDX       2
#define EDIT_UNPROTECT_TABLE    3
#define EDIT_MODE_DELETE        4
#define EDIT_MODE_RENAME        5

class SwContentTree : public SvTreeListBox
{
    ImageList           aEntryImages;
    OUString            sSpace;
    AutoTimer           aUpdTimer;

    SwContentType*      aActiveContentArr[CONTENT_TYPE_MAX];
    SwContentType*      aHiddenContentArr[CONTENT_TYPE_MAX];
    OUString            aContextStrings[CONTEXT_COUNT + 1];
    OUString            sRemoveIdx;
    OUString            sUpdateIdx;
    OUString            sUnprotTbl;
    OUString            sRename;
    OUString            sReadonlyIdx;
    OUString            sInvisible;
    OUString            sPostItShow;
    OUString            sPostItHide;
    OUString            sPostItDelete;

    SwWrtShell*         pHiddenShell;   // dropped Doc
    SwWrtShell*         pActiveShell;   // the active or a const. open view
    SwNavigationConfig* pConfig;

    std::map< void*, bool > mOutLineNodeMap;

    sal_Int32           nActiveBlock;
    sal_Int32           nHiddenBlock;
    sal_uInt16              nRootType;
    sal_uInt16              nLastSelType;
    sal_uInt8               nOutlineLevel;

    bool                bIsActive           :1;
    bool                bIsConstant         :1;
    bool                bIsHidden           :1;
    bool                bDocChgdInDragging  :1;
    bool                bIsInternalDrag     :1;
    bool                bIsRoot             :1;
    bool                bIsIdleClear        :1;
    bool                bIsLastReadOnly     :1;
    bool                bIsOutlineMoveable  :1;
    bool                bViewHasChanged     :1;
    bool                bIsImageListInitialized : 1;

    static bool         bIsInDrag;

    bool                bIsKeySpace;
    Rectangle           oldRectangle;

    void                FindActiveTypeAndRemoveUserData();

    using SvTreeListBox::ExecuteDrop;
    using SvTreeListBox::EditEntry;

protected:
    virtual void    RequestHelp( const HelpEvent& rHEvt ) SAL_OVERRIDE;
    virtual void    InitEntry(SvTreeListEntry*, const OUString&, const Image&, const Image&, SvLBoxButtonKind) SAL_OVERRIDE;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;

    SwNavigationPI* GetParentWindow(){return
                        (SwNavigationPI*)Window::GetParent();}

    virtual void    StartDrag( sal_Int8 nAction, const Point& rPosPixel ) SAL_OVERRIDE;
    virtual void    DragFinished( sal_Int8 ) SAL_OVERRIDE;
    virtual sal_Int8 AcceptDrop( const AcceptDropEvent& rEvt ) SAL_OVERRIDE;

    virtual sal_Int8 ExecuteDrop( const ExecuteDropEvent& rEvt ) SAL_OVERRIDE;

    bool        FillTransferData( TransferDataContainer& rTransfer,
                                            sal_Int8& rDragMode );
    bool            HasContentChanged();

    virtual DragDropMode NotifyStartDrag( TransferDataContainer& rData,
                                        SvTreeListEntry* ) SAL_OVERRIDE;
    virtual bool    NotifyAcceptDrop( SvTreeListEntry* ) SAL_OVERRIDE;

    virtual TriState NotifyMoving(   SvTreeListEntry*  pTarget,
                                    SvTreeListEntry*  pEntry,
                                    SvTreeListEntry*& rpNewParent,
                                    sal_uLong&        rNewChildPos
                                ) SAL_OVERRIDE;
    virtual TriState NotifyCopying(  SvTreeListEntry*  pTarget,
                                    SvTreeListEntry*  pEntry,
                                    SvTreeListEntry*& rpNewParent,
                                    sal_uLong&        rNewChildPos
                                ) SAL_OVERRIDE;
    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) SAL_OVERRIDE;

    void            EditEntry( SvTreeListEntry* pEntry, sal_uInt8 nMode );

    void            GotoContent(SwContent* pCnt);
    static void     SetInDrag(bool bSet) {bIsInDrag = bSet;}

    virtual PopupMenu* CreateContextMenu( void ) SAL_OVERRIDE;
    virtual void    ExcecuteContextMenuAction( sal_uInt16 nSelectedPopupEntry ) SAL_OVERRIDE;

public:
    SwContentTree(vcl::Window* pParent, const ResId& rResId);
    virtual ~SwContentTree();
    OUString        GetEntryAltText( SvTreeListEntry* pEntry ) const SAL_OVERRIDE;
    OUString        GetEntryLongDescription( SvTreeListEntry* pEntry ) const SAL_OVERRIDE;
    SdrObject*      GetDrawingObjectsByContent(const SwContent *pCnt);

    bool            ToggleToRoot();
    bool            IsRoot() const {return bIsRoot;}
    sal_uInt16          GetRootType() const {return nRootType;}
    void            SetRootType(sal_uInt16 nType);
    void            Display( bool bActiveView );
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

    static bool     IsInDrag() {return bIsInDrag;}
    bool            IsInternalDrag() const {return bIsInternalDrag;}

    sal_Int32       GetActiveBlock() const {return nActiveBlock;}

    sal_uInt8           GetOutlineLevel()const {return nOutlineLevel;}
    void            SetOutlineLevel(sal_uInt8 nSet);

    virtual bool    Expand( SvTreeListEntry* pParent ) SAL_OVERRIDE;

    virtual bool    Collapse( SvTreeListEntry* pParent ) SAL_OVERRIDE;

    void            ExecCommand(sal_uInt16 nCmd, bool bModifier);

    void            ShowTree();
    void            HideTree();

    bool            IsConstantView() {return bIsConstant;}
    bool            IsActiveView()   {return bIsActive;}
    bool            IsHiddenView()   {return bIsHidden;}

    const SwWrtShell*   GetActiveWrtShell() {return pActiveShell;}
    SwWrtShell*         GetHiddenWrtShell() {return pHiddenShell;}

    DECL_LINK( ContentDoubleClickHdl, void * );
    DECL_LINK( TimerUpdate, void * );

    virtual sal_IntPtr GetTabPos( SvTreeListEntry*, SvLBoxTab* ) SAL_OVERRIDE;
    virtual void    RequestingChildren( SvTreeListEntry* pParent ) SAL_OVERRIDE;
    virtual void    GetFocus() SAL_OVERRIDE;
    virtual void    KeyInput(const KeyEvent& rKEvt) SAL_OVERRIDE;

    virtual bool    Select( SvTreeListEntry* pEntry, bool bSelect=true ) SAL_OVERRIDE;
    virtual sal_Int32  GetEntryRealChildrenNum( SvTreeListEntry* pEntry ) const;
};

// TreeListBox for global documents

class SwLBoxString : public SvLBoxString
{
public:

    SwLBoxString( SvTreeListEntry* pEntry, sal_uInt16 nFlags,
        const OUString& rStr ) : SvLBoxString(pEntry,nFlags,rStr)
    {
    }

    virtual void Paint(
        const Point& rPos, SvTreeListBox& rDev, const SvViewDataEntry* pView,
        const SvTreeListEntry* pEntry) SAL_OVERRIDE;
};

namespace sfx2 { class DocumentInserter; }
namespace sfx2 { class FileDialogHelper; }

class SwGlobalTree : public SvTreeListBox
{
private:
    AutoTimer           aUpdateTimer;
    OUString            aContextStrings[GLOBAL_CONTEXT_COUNT];

    ImageList           aEntryImages;

    SwWrtShell*             pActiveShell;
    SvTreeListEntry*            pEmphasisEntry; // Drag'n Drop emphasis
    SvTreeListEntry*            pDDSource;      // source for Drag'n Drop
    SwGlblDocContents*      pSwGlblDocContents; // array with sorted content

    vcl::Window*                 pDefParentWin;
    SwGlblDocContent*       pDocContent;
    sfx2::DocumentInserter* pDocInserter;

    bool                bIsInternalDrag     :1;
    bool                bLastEntryEmphasis  :1; // Drag'n Drop
    bool                bIsImageListInitialized : 1;

    static const SfxObjectShell* pShowShell;

    void        InsertRegion( const SwGlblDocContent* _pContent,
                              const com::sun::star::uno::Sequence< OUString >& _rFiles );

    DECL_LINK(  DialogClosedHdl, sfx2::FileDialogHelper* );

    using SvTreeListBox::DoubleClickHdl;
    using SvTreeListBox::ExecuteDrop;
    using Window::Update;

protected:

    virtual sal_Int8 AcceptDrop( const AcceptDropEvent& rEvt ) SAL_OVERRIDE;

    virtual sal_Int8 ExecuteDrop( const ExecuteDropEvent& rEvt ) SAL_OVERRIDE;

    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;

    virtual void    RequestHelp( const HelpEvent& rHEvt ) SAL_OVERRIDE;

    virtual sal_IntPtr GetTabPos( SvTreeListEntry*, SvLBoxTab* ) SAL_OVERRIDE;
    virtual TriState NotifyMoving(   SvTreeListEntry*  pTarget,
                                    SvTreeListEntry*  pEntry,
                                    SvTreeListEntry*& rpNewParent,
                                    sal_uLong&        rNewChildPos
                                ) SAL_OVERRIDE;
    virtual TriState NotifyCopying(  SvTreeListEntry*  pTarget,
                                    SvTreeListEntry*  pEntry,
                                    SvTreeListEntry*& rpNewParent,
                                    sal_uLong&        rNewChildPos
                                ) SAL_OVERRIDE;

    virtual void    StartDrag( sal_Int8 nAction, const Point& rPosPixel ) SAL_OVERRIDE;
    virtual void    DragFinished( sal_Int8 ) SAL_OVERRIDE;
    virtual DragDropMode NotifyStartDrag( TransferDataContainer& rData,
                                        SvTreeListEntry* ) SAL_OVERRIDE;
    virtual bool    NotifyAcceptDrop( SvTreeListEntry* ) SAL_OVERRIDE;

    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) SAL_OVERRIDE;
    virtual void    KeyInput(const KeyEvent& rKEvt) SAL_OVERRIDE;
    virtual void    GetFocus() SAL_OVERRIDE;
    virtual void    SelectHdl() SAL_OVERRIDE;
    virtual void    DeselectHdl() SAL_OVERRIDE;
    virtual void    InitEntry(SvTreeListEntry*, const OUString&, const Image&, const Image&, SvLBoxButtonKind) SAL_OVERRIDE;

    void            Clear();

    DECL_LINK(      PopupHdl, Menu* );
    DECL_LINK(      Timeout, void* );
    DECL_LINK(      DoubleClickHdl, void* );

    bool            IsInternalDrag() const {return bIsInternalDrag;}
    SwNavigationPI* GetParentWindow()
                        { return (SwNavigationPI*)Window::GetParent(); }

    void            OpenDoc(const SwGlblDocContent*);
    void            GotoContent(const SwGlblDocContent*);
    sal_uInt16          GetEnableFlags() const;

    static const SfxObjectShell*    GetShowShell() {return pShowShell;}
    static void     SetShowShell(const SfxObjectShell*pSet) {pShowShell = pSet;}
    DECL_STATIC_LINK(SwGlobalTree, ShowFrameHdl, SwGlobalTree*);

    virtual PopupMenu* CreateContextMenu( void ) SAL_OVERRIDE;
    virtual void    ExcecuteContextMenuAction( sal_uInt16 nSelectedPopupEntry ) SAL_OVERRIDE;

public:
    SwGlobalTree(vcl::Window* pParent, const ResId& rResId);
    virtual ~SwGlobalTree();

    void                TbxMenuHdl(sal_uInt16 nTbxId, ToolBox* pBox);
    void                InsertRegion( const SwGlblDocContent* pCont,
                                        const OUString* pFileName = 0 );
    void                EditContent(const SwGlblDocContent* pCont );

    void                ShowTree();
    void                HideTree();

    void                ExecCommand(sal_uInt16 nCmd);

    void                Display(bool bOnlyUpdateUserData = false);

    bool                Update(bool bHard);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
