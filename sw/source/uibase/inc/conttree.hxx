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
#include <vcl/timer.hxx>
#include <vcl/transfer.hxx>
#include <vcl/weld.hxx>
#include <ndarr.hxx>
#include "swcont.hxx"

#include <map>
#include <memory>

#include <o3tl/enumarray.hxx>
#include <o3tl/typed_flags_set.hxx>

#include <svx/sdr/overlay/overlayobject.hxx>
#include <editsh.hxx>
#include <edglbldc.hxx>

class SwWrtShell;
class SwContentType;
class SwNavigationPI;
class SwNavigationConfig;
class Menu;
class ToolBox;
class SwGlblDocContents;
class SfxObjectShell;
class SdrObject;

enum class EditEntryMode
{
    EDIT          = 0,
    UPD_IDX       = 1,
    UNPROTECT_TABLE    = 2,
    DELETE        = 3,
    RENAME        = 4,
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

class SwContentTree;

class SwContentTreeDropTarget final : public DropTargetHelper
{
private:
    SwContentTree& m_rTreeView;

    virtual sal_Int8 AcceptDrop( const AcceptDropEvent& rEvt ) override;
    virtual sal_Int8 ExecuteDrop( const ExecuteDropEvent& rEvt ) override;

public:
    SwContentTreeDropTarget(SwContentTree& rTreeView);
};

/** TreeListBox for content indicator */
class SwContentTree final : public SfxListener
{
    std::unique_ptr<weld::TreeView> m_xTreeView;
    SwContentTreeDropTarget m_aDropTargetHelper;
    SwNavigationPI*     m_pDialog;
    OUString            m_sSpace;
    AutoTimer           m_aUpdTimer;
    AutoTimer m_aOverlayObjectDelayTimer;

    o3tl::enumarray<ContentTypeId,std::unique_ptr<SwContentType>>  m_aActiveContentArr;
    o3tl::enumarray<ContentTypeId,std::unique_ptr<SwContentType>>  m_aHiddenContentArr;
    OUString            m_aContextStrings[CONTEXT_COUNT + 1];
    OUString            m_sInvisible;
    OUString            m_sSelectedItem;  // last selected item (only bookmarks yet)

    SwWrtShell*         m_pHiddenShell;   // dropped Doc
    SwWrtShell*         m_pActiveShell;   // the active or a const. open view
    SwNavigationConfig* m_pConfig;

    // these maps store the expand state of nodes with children
    std::map< void*, bool > mOutLineNodeMap;
    std::map<const void*, bool> m_aRegionNodeExpandMap;
    std::map<const void*, bool> m_aPostItNodeExpandMap;

    sal_Int32           m_nActiveBlock;  // used to restore content types expand state
    sal_Int32           m_nHiddenBlock;
    size_t              m_nEntryCount;
    ContentTypeId       m_nRootType;    // content type that is currently displayed in the tree
    ContentTypeId       m_nLastSelType;
    sal_uInt8           m_nOutlineLevel;

    sal_uInt8           m_nOutlineTracking = 1; // 1 default, 2 focus, 3 off
    o3tl::enumarray<ContentTypeId, bool> mTrackContentType;

    SwOutlineNodes::size_type m_nLastGotoContentWasOutlinePos = SwOutlineNodes::npos;

    enum class State { ACTIVE, CONSTANT, HIDDEN } m_eState;

    bool                m_bIsRoot             :1;
    bool                m_bIsIdleClear        :1;
    bool                m_bIsLastReadOnly     :1;
    bool                m_bIsOutlineMoveable  :1;
    bool                m_bViewHasChanged     :1;

    // outline root mode drag & drop
    std::vector<std::unique_ptr<weld::TreeIter>> m_aDndOutlinesSelected;

    bool m_bDocHasChanged = true;
    bool m_bIgnoreDocChange = false; // used to prevent tracking update

    ImplSVEvent* m_nRowActivateEventId = nullptr;
    bool m_bSelectTo = false;

    std::unique_ptr<weld::TreeIter> m_xOverlayCompareEntry;
    std::unique_ptr<sdr::overlay::OverlayObject> m_xOverlayObject;

    void OverlayObject(std::vector<basegfx::B2DRange>&& aRanges = {});

    void BringEntryToAttention(const weld::TreeIter& rEntry);
    void BringFramesToAttention(const std::vector<const SwFrameFormat*>& rFrameFormats);
    void BringBookmarksToAttention(const std::vector<OUString>& rNames);
    void BringURLFieldsToAttention(const SwGetINetAttrs& rINetAttrsArr);
    void BringReferencesToAttention(std::vector<const SwTextAttr*>& rTextAttrsArr);
    void BringPostItFieldsToAttention(std::vector<const SwTextAttr*>& rTextAttrsArr);
    void BringDrawingObjectsToAttention(std::vector<const SdrObject*>& rDrawingObjectsArr);
    void BringTextFieldsToAttention(std::vector<const SwTextAttr*>& rTextAttrsArr);
    void BringFootnotesToAttention(std::vector<const SwTextAttr*>& rTextAttrsArr);
    void BringTypesWithFlowFramesToAttention(const std::vector<const SwNode*>& rNodes,
                                             const bool bIncludeTopMargin = true);

    /**
     * Before any data will be deleted, the last active entry has to be found.
     * After this the UserData will be deleted
     */
    void                FindActiveTypeAndRemoveUserData();

    void InsertContent(const weld::TreeIter& rParent);

    void insert(const weld::TreeIter* pParent, const OUString& rStr, const OUString& rId,
                bool bChildrenOnDemand, weld::TreeIter* pRet);

    void remove(const weld::TreeIter& rIter);

    SwNavigationPI* GetParentWindow();

    bool FillTransferData(TransferDataContainer& rTransfer);

    /** Check if the displayed content is valid. */
    bool            HasContentChanged();

    size_t          GetAbsPos(const weld::TreeIter& rIter);

    void            EditEntry(const weld::TreeIter& rEntry, EditEntryMode nMode);

    void            GotoContent(const SwContent* pCnt);

    void            ExecuteContextMenuAction(const OUString& rSelectedPopupEntry);

    void DeleteOutlineSelections();
    void CopyOutlineSelections();

    size_t GetEntryCount() const;

    size_t GetChildCount(const weld::TreeIter& rParent) const;

    std::unique_ptr<weld::TreeIter> GetEntryAtAbsPos(size_t nAbsPos) const;

    void Expand(const weld::TreeIter& rParent, std::vector<std::unique_ptr<weld::TreeIter>>* pNodesToExpand);

    void MoveOutline(SwOutlineNodes::size_type nTargetPos);

    void UpdateLastSelType();

    /** Expand - Remember the state for content types */
    DECL_LINK(ExpandHdl, const weld::TreeIter&, bool);
    /** Collapse - Remember the state for content types. */
    DECL_LINK(CollapseHdl, const weld::TreeIter&, bool);
    DECL_LINK(ContentDoubleClickHdl, weld::TreeView&, bool);
    DECL_LINK(AsyncContentDoubleClickHdl, void*, void);
    DECL_LINK(SelectHdl, weld::TreeView&, void);
    DECL_LINK(FocusInHdl, weld::Widget&, void);
    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);
    DECL_LINK(CommandHdl, const CommandEvent&, bool);
    DECL_LINK(QueryTooltipHdl, const weld::TreeIter&, OUString);
    DECL_LINK(DragBeginHdl, bool&, bool);
    DECL_LINK(TimerUpdate, Timer *, void);
    DECL_LINK(OverlayObjectDelayTimerHdl, Timer *, void);
    DECL_LINK(MouseMoveHdl, const MouseEvent&, bool);
    DECL_LINK(MousePressHdl, const MouseEvent&, bool);

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

    bool            IsInDrag() const;
    bool            HasHeadings() const;

    sal_uInt8       GetOutlineLevel()const {return m_nOutlineLevel;}
    void            SetOutlineLevel(sal_uInt8 nSet);

    void            SetOutlineTracking(sal_uInt8 nSet);
    void            SetContentTypeTracking(ContentTypeId eCntTypeId, bool bSet);

    /** Execute commands of the Navigator */
    void            ExecCommand(std::u16string_view rCmd, bool bModifier);

    void            ShowTree();
    void            HideTree();

    bool            IsConstantView() const { return State::CONSTANT == m_eState; }
    bool            IsActiveView() const   { return State::ACTIVE == m_eState; }
    bool            IsHiddenView() const   { return State::HIDDEN == m_eState; }

    const SwWrtShell*   GetActiveWrtShell() const {return m_pActiveShell;}
    SwWrtShell*         GetHiddenWrtShell() {return m_pHiddenShell;}

    void Select();

    void UpdateTracking();
    void SelectOutlinesWithSelection();
    void SelectContentType(std::u16string_view rContentTypeName);

    // return true if it has any children
    bool RequestingChildren(const weld::TreeIter& rParent);

    void ExpandAllHeadings();

    virtual void Notify(SfxBroadcaster& rBC, SfxHint const& rHint) override;

    sal_Int8 AcceptDrop(const AcceptDropEvent& rEvt);
    sal_Int8 ExecuteDrop(const ExecuteDropEvent& rEvt);

    bool IsDropFormatSupported(SotClipboardFormatId nFormat)
    {
        return m_aDropTargetHelper.IsDropFormatSupported(nFormat);
    }

    void set_accessible_name(const OUString& rName)
    {
        m_xTreeView->set_accessible_name(rName);
    }

    void grab_focus()
    {
        m_xTreeView->grab_focus();
    }

    void set_selection_mode(SelectionMode eMode)
    {
        m_xTreeView->set_selection_mode(eMode);
    }

    weld::TreeView& get_widget()
    {
        return *m_xTreeView;
    }
};

namespace sfx2 { class DocumentInserter; }
namespace sfx2 { class FileDialogHelper; }

class SwGlobalTree;

class SwGlobalTreeDropTarget final : public DropTargetHelper
{
private:
    SwGlobalTree& m_rTreeView;

    virtual sal_Int8 AcceptDrop( const AcceptDropEvent& rEvt ) override;
    virtual sal_Int8 ExecuteDrop( const ExecuteDropEvent& rEvt ) override;

public:
    SwGlobalTreeDropTarget(SwGlobalTree& rTreeView);
};

class SwGlobalTree final : public SfxListener
{
private:
    std::unique_ptr<weld::TreeView> m_xTreeView;
    SwGlobalTreeDropTarget  m_aDropTargetHelper;
    SwNavigationPI*         m_pDialog;
    AutoTimer               m_aUpdateTimer;
    OUString                m_aContextStrings[GLOBAL_CONTEXT_COUNT];

    SwWrtShell*             m_pActiveShell;
    std::unique_ptr<SwGlblDocContents> m_pSwGlblDocContents; // array with sorted content

    std::optional<SwGlblDocContent>       m_oDocContent;
    std::unique_ptr<sfx2::DocumentInserter> m_pDocInserter;

    static const SfxObjectShell* s_pShowShell;

    void        InsertRegion( const SwGlblDocContent* _pContent,
                              const css::uno::Sequence< OUString >& _rFiles );

    DECL_LINK(  DialogClosedHdl, sfx2::FileDialogHelper*, void );

    void Select();

    DECL_LINK(Timeout, Timer*, void);
    DECL_LINK(DoubleClickHdl, weld::TreeView&, bool);
    DECL_LINK(SelectHdl, weld::TreeView&, void);
    DECL_LINK(FocusInHdl, weld::Widget&, void);
    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);
    DECL_LINK(CommandHdl, const CommandEvent&, bool);
    DECL_LINK(QueryTooltipHdl, const weld::TreeIter&, OUString);

    SwNavigationPI* GetParentWindow();

    void            OpenDoc(const SwGlblDocContent*);
    void            GotoContent(const SwGlblDocContent*);
    MenuEnableFlags GetEnableFlags() const;

    static void     SetShowShell(const SfxObjectShell*pSet) {s_pShowShell = pSet;}
    DECL_STATIC_LINK(SwGlobalTree, ShowFrameHdl, void*, void);

public:
    SwGlobalTree(std::unique_ptr<weld::TreeView> xTreeView, SwNavigationPI* pDialog);
    ~SwGlobalTree();

    bool get_visible() const { return m_xTreeView->get_visible(); }

    void set_accessible_name(const OUString& rName)
    {
        m_xTreeView->set_accessible_name(rName);
    }

    void grab_focus()
    {
        m_xTreeView->grab_focus();
    }

    void set_selection_mode(SelectionMode eMode)
    {
        m_xTreeView->set_selection_mode(eMode);
    }

    weld::TreeView& get_widget()
    {
        return *m_xTreeView;
    }

    void MoveSelectionTo(const weld::TreeIter* pDropTarget);

    void                TbxMenuHdl(std::u16string_view rCommand, weld::Menu& rMenu);
    void                InsertRegion( const SwGlblDocContent* pCont,
                                        const OUString* pFileName = nullptr );
    void                EditContent(const SwGlblDocContent* pCont );

    void                ShowTree();
    void                HideTree();

    void                ExecCommand(std::u16string_view rCmd);

    void                Display(bool bOnlyUpdateUserData = false);

    bool                Update(bool bHard);

    void                ExecuteContextMenuAction(std::u16string_view rSelectedPopupEntry);

    const SwWrtShell*   GetActiveWrtShell() const {return m_pActiveShell;}

    virtual void Notify(SfxBroadcaster& rBC, SfxHint const& rHint) override;
    void UpdateTracking();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
