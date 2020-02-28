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

#include <svl/lstner.hxx>
#include <vcl/treelistbox.hxx>
#include <vcl/svlbitm.hxx>
#include "swcont.hxx"

#include <map>
#include <memory>

#include <o3tl/enumarray.hxx>
#include <o3tl/typed_flags_set.hxx>

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

enum class EditEntryMode
{
    EDIT          = 0,
    UPD_IDX       = 1,
    RMV_IDX       = 2,
    UNPROTECT_TABLE    = 3,
    DELETE        = 4,
    RENAME        = 5,
};

// Flags for PopupMenu-enable/disable
enum class MenuEnableFlags {
    NONE        = 0x0000,
    InsertIdx   = 0x0001,
    InsertFile  = 0x0002,
    InsertText  = 0x0004,
    Edit        = 0x0008,
    Delete      = 0x0010,
    Update      = 0x0020,
    UpdateSel   = 0x0040,
    EditLink    = 0x0080
};
namespace o3tl {
    template<> struct typed_flags<MenuEnableFlags> : is_typed_flags<MenuEnableFlags, 0x00ff> {};
}

/** TreeListBox for content indicator */
class SwContentTree final : public SfxListener
{
    std::unique_ptr<weld::TreeView> m_xTreeView;
    VclPtr<SwNavigationPI> m_xDialog;
    OUString const      m_sSpace;
    AutoTimer           m_aUpdTimer;

    o3tl::enumarray<ContentTypeId,std::unique_ptr<SwContentType>>  m_aActiveContentArr;
    o3tl::enumarray<ContentTypeId,std::unique_ptr<SwContentType>>  m_aHiddenContentArr;
    OUString            m_aContextStrings[CONTEXT_COUNT + 1];
    OUString const      m_sRemoveIdx;
    OUString const      m_sUpdateIdx;
    OUString const      m_sUnprotTable;
    OUString const      m_sRename;
    OUString const      m_sReadonlyIdx;
    OUString const      m_sInvisible;
    OUString const      m_sPostItShow;
    OUString const      m_sPostItHide;
    OUString const      m_sPostItDelete;
    OUString const      m_sProtected;

    SwWrtShell*         m_pHiddenShell;   // dropped Doc
    SwWrtShell*         m_pActiveShell;   // the active or a const. open view
    SwNavigationConfig* m_pConfig;

    std::map< void*, bool > mOutLineNodeMap;

    sal_Int32           m_nActiveBlock;
    sal_Int32           m_nHiddenBlock;
    ContentTypeId       m_nRootType;
    ContentTypeId       m_nLastSelType;
    sal_uInt8           m_nOutlineLevel;

    sal_uInt32          m_nOutlineTracking = 1;

    enum class State { ACTIVE, CONSTANT, HIDDEN } m_eState;

    bool                m_bDocChgdInDragging  :1;
    bool                m_bIsInternalDrag     :1;
    bool                m_bIsRoot             :1;
    bool                m_bIsIdleClear        :1;
    bool                m_bIsLastReadOnly     :1;
    bool                m_bIsOutlineMoveable  :1;
    bool                m_bViewHasChanged     :1;

    static bool         bIsInDrag;

//TODO    bool                m_bIsKeySpace;
//TODO    tools::Rectangle           m_aOldRectangle;

    // outline root mode drag & drop
    std::vector< SvTreeListEntry* > m_aDndOutlinesSelected;

    bool m_bIsInPromoteDemote = false;

    /**
     * Before any data will be deleted, the last active entry has to be found.
     * After this the UserData will be deleted
     */
    void                FindActiveTypeAndRemoveUserData();

#if 0
    virtual void    RequestHelp( const HelpEvent& rHEvt ) override;
#endif

    SwNavigationPI* GetParentWindow();

#if 0
    virtual void    StartDrag( sal_Int8 nAction, const Point& rPosPixel ) override;
    virtual void    DragFinished( sal_Int8 ) override;
    virtual sal_Int8 AcceptDrop( const AcceptDropEvent& rEvt ) override;

    virtual sal_Int8 ExecuteDrop( const ExecuteDropEvent& rEvt ) override;
#endif

    bool        FillTransferData( TransferDataContainer& rTransfer,
                                            sal_Int8& rDragMode );

    /** Check if the displayed content is valid. */
    bool            HasContentChanged();

#if 0
    virtual DragDropMode NotifyStartDrag( TransferDataContainer& rData,
                                        SvTreeListEntry* ) override;
    virtual bool    NotifyAcceptDrop( SvTreeListEntry* ) override;

    virtual TriState NotifyMoving(   SvTreeListEntry*  pTarget,
                                    SvTreeListEntry*  pEntry,
                                    SvTreeListEntry*& rpNewParent,
                                    sal_uLong&        rNewChildPos
                                ) override;
    virtual TriState NotifyCopying(  SvTreeListEntry*  pTarget,
                                    SvTreeListEntry*  pEntry,
                                    SvTreeListEntry*& rpNewParent,
                                    sal_uLong&        rNewChildPos
                                ) override;
    virtual void    Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect ) override;
    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) override;
#endif

    void            EditEntry(const weld::TreeIter& rEntry, EditEntryMode nMode);

    void            GotoContent(const SwContent* pCnt);
    static void     SetInDrag(bool bSet) {bIsInDrag = bSet;}

#if 0
    virtual VclPtr<PopupMenu> CreateContextMenu() override;
    virtual void    ExecuteContextMenuAction( sal_uInt16 nSelectedPopupEntry ) override;
#endif

    void DeleteOutlineSelections();

public:
    SwContentTree(std::unique_ptr<weld::TreeView> xTreeView, SwNavigationPI* pDialog);
    ~SwContentTree();
    SdrObject*      GetDrawingObjectsByContent(const SwContent *pCnt);

    /** Switch the display to Root */
    void            ToggleToRoot();
    void            SetRootType(ContentTypeId nType);

    /** Show the file */
    void            Display( bool bActiveView );
    /** In the clear the content types have to be deleted, also. */
    void            clear();

    /** After a file is dropped on the Navigator, the new shell will be set */
    void            SetHiddenShell(SwWrtShell* pSh);
    void            ShowHiddenShell();
    void            ShowActualView();

    /** Document change - set new Shell */
    void            SetActiveShell(SwWrtShell* pSh);

    /** Set an open view as active. */
    void            SetConstantShell(SwWrtShell* pSh);

    SwWrtShell*     GetWrtShell()
        { return State::HIDDEN == m_eState ? m_pHiddenShell : m_pActiveShell; }

    static bool     IsInDrag() {return bIsInDrag;}

    sal_uInt8       GetOutlineLevel()const {return m_nOutlineLevel;}
    void            SetOutlineLevel(sal_uInt8 nSet);

#if 0
    /** Expand - Remember the state for content types */
    virtual bool    Expand( SvTreeListEntry* pParent ) override;
    /** Collapse - Remember the state for content types. */
    virtual bool    Collapse( SvTreeListEntry* pParent ) override;
#endif

    /** Execute commands of the Navigator */
    void            ExecCommand(const OString& rCmd, bool bModifier);

    void            ShowTree();
    void            HideTree();

    bool            IsConstantView() const { return State::CONSTANT == m_eState; }
    bool            IsActiveView() const   { return State::ACTIVE == m_eState; }
    bool            IsHiddenView() const   { return State::HIDDEN == m_eState; }

    const SwWrtShell*   GetActiveWrtShell() const {return m_pActiveShell;}
    SwWrtShell*         GetHiddenWrtShell() {return m_pHiddenShell;}

    DECL_LINK(ExpandHdl, const weld::TreeIter&, bool);
    DECL_LINK(CollapseHdl, const weld::TreeIter&, bool);
    DECL_LINK(ContentDoubleClickHdl, weld::TreeView&, bool);
    DECL_LINK(SelectHdl, weld::TreeView&, void);
    DECL_LINK(FocusHdl, weld::Widget&, void);
    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);
    DECL_LINK(TimerUpdate, Timer *, void);

#if 0
    virtual sal_IntPtr GetTabPos( SvTreeListEntry*, SvLBoxTab* ) override;
#endif
    void RequestingChildren(const weld::TreeIter& rParent);

    virtual void Notify(SfxBroadcaster& rBC, SfxHint const& rHint) override;

    void set_accessible_name(const OUString& rName)
    {
        m_xTreeView->set_accessible_name(rName);
    }

    void grab_focus()
    {
        m_xTreeView->grab_focus();
    }

    int count_selected_rows() const
    {
        return m_xTreeView->count_selected_rows();
    }

    void set_selection_mode(SelectionMode eMode)
    {
        m_xTreeView->set_selection_mode(eMode);
    }
};

// TreeListBox for global documents

class SwLBoxString : public SvLBoxString
{
public:

    SwLBoxString( const OUString& rStr ) : SvLBoxString(rStr)
    {
    }

    virtual void Paint(const Point& rPos, SvTreeListBox& rDev, vcl::RenderContext& rRenderContext,
                       const SvViewDataEntry* pView, const SvTreeListEntry& rEntry) override;
};

namespace sfx2 { class DocumentInserter; }
namespace sfx2 { class FileDialogHelper; }

class SwGlobalTree final : public SvTreeListBox
{
private:
    VclPtr<SwNavigationPI>  m_xDialog;
    AutoTimer               m_aUpdateTimer;
    OUString                m_aContextStrings[GLOBAL_CONTEXT_COUNT];

    SwWrtShell*             m_pActiveShell;
    SvTreeListEntry*        m_pEmphasisEntry; // Drag'n Drop emphasis
    SvTreeListEntry*        m_pDDSource;      // source for Drag'n Drop
    std::unique_ptr<SwGlblDocContents> m_pSwGlblDocContents; // array with sorted content

    std::unique_ptr<SwGlblDocContent>       m_pDocContent;
    std::unique_ptr<sfx2::DocumentInserter> m_pDocInserter;

    bool                m_bIsInternalDrag     :1;
    bool                m_bLastEntryEmphasis  :1; // Drag'n Drop

    static const SfxObjectShell* pShowShell;

    void        InsertRegion( const SwGlblDocContent* _pContent,
                              const css::uno::Sequence< OUString >& _rFiles );

    DECL_LINK(  DialogClosedHdl, sfx2::FileDialogHelper*, void );

    using SvTreeListBox::DoubleClickHdl;
    using SvTreeListBox::ExecuteDrop;
    using Window::Update;

    virtual sal_Int8 AcceptDrop( const AcceptDropEvent& rEvt ) override;

    virtual sal_Int8 ExecuteDrop( const ExecuteDropEvent& rEvt ) override;

    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;

    virtual void    RequestHelp( const HelpEvent& rHEvt ) override;

    virtual sal_IntPtr GetTabPos( SvTreeListEntry*, SvLBoxTab* ) override;
    virtual TriState NotifyMoving(   SvTreeListEntry*  pTarget,
                                    SvTreeListEntry*  pEntry,
                                    SvTreeListEntry*& rpNewParent,
                                    sal_uLong&        rNewChildPos
                                ) override;
    virtual TriState NotifyCopying(  SvTreeListEntry*  pTarget,
                                    SvTreeListEntry*  pEntry,
                                    SvTreeListEntry*& rpNewParent,
                                    sal_uLong&        rNewChildPos
                                ) override;

    virtual void    StartDrag( sal_Int8 nAction, const Point& rPosPixel ) override;
    virtual void    DragFinished( sal_Int8 ) override;
    virtual DragDropMode NotifyStartDrag( TransferDataContainer& rData,
                                        SvTreeListEntry* ) override;
    virtual bool    NotifyAcceptDrop( SvTreeListEntry* ) override;

    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void    KeyInput(const KeyEvent& rKEvt) override;
    virtual void    GetFocus() override;
    virtual void    SelectHdl() override;
    virtual void    DeselectHdl() override;
    virtual void    InitEntry(SvTreeListEntry*, const OUString&, const Image&, const Image&) override;

    void            Clear();

    DECL_LINK( PopupHdl, Menu*, bool );
    DECL_LINK( Timeout, Timer*, void );
    DECL_LINK( DoubleClickHdl, SvTreeListBox*, bool );

    SwNavigationPI* GetParentWindow();

    void            OpenDoc(const SwGlblDocContent*);
    void            GotoContent(const SwGlblDocContent*);
    MenuEnableFlags GetEnableFlags() const;

    static void     SetShowShell(const SfxObjectShell*pSet) {pShowShell = pSet;}
    DECL_STATIC_LINK(SwGlobalTree, ShowFrameHdl, void*, void);

    virtual VclPtr<PopupMenu> CreateContextMenu() override;
    virtual void    ExecuteContextMenuAction( sal_uInt16 nSelectedPopupEntry ) override;

public:
    SwGlobalTree(vcl::Window* pParent, SwNavigationPI* pDialog);
    virtual ~SwGlobalTree() override;
    virtual void        dispose() override;
    virtual Size        GetOptimalSize() const override;

    void                TbxMenuHdl(sal_uInt16 nTbxId, ToolBox* pBox);
    void                InsertRegion( const SwGlblDocContent* pCont,
                                        const OUString* pFileName = nullptr );
    void                EditContent(const SwGlblDocContent* pCont );

    void                ShowTree();
    void                HideTree();

    void                ExecCommand(const OUString& rCmd);

    void                Display(bool bOnlyUpdateUserData = false);

    bool                Update(bool bHard);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
