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
#include <svtools/treelistbox.hxx>
#include <svtools/svlbitm.hxx>
#include "swcont.hxx"

#include <map>
#include <memory>

#include <o3tl/enumarray.hxx>

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

class SwContentTree
    : public SvTreeListBox
    , public SfxListener
{
    ImageList           m_aEntryImages;
    OUString            m_sSpace;
    AutoTimer           m_aUpdTimer;

    o3tl::enumarray<ContentTypeId,SwContentType*>  m_aActiveContentArr;
    o3tl::enumarray<ContentTypeId,SwContentType*>  m_aHiddenContentArr;
    OUString            m_aContextStrings[CONTEXT_COUNT + 1];
    OUString            m_sRemoveIdx;
    OUString            m_sUpdateIdx;
    OUString            m_sUnprotTable;
    OUString            m_sRename;
    OUString            m_sReadonlyIdx;
    OUString            m_sInvisible;
    OUString            m_sPostItShow;
    OUString            m_sPostItHide;
    OUString            m_sPostItDelete;

    SwWrtShell*         m_pHiddenShell;   // dropped Doc
    SwWrtShell*         m_pActiveShell;   // the active or a const. open view
    SwNavigationConfig* m_pConfig;

    std::map< void*, bool > mOutLineNodeMap;

    sal_Int32           m_nActiveBlock;
    sal_Int32           m_nHiddenBlock;
    ContentTypeId       m_nRootType;
    ContentTypeId       m_nLastSelType;
    sal_uInt8           m_nOutlineLevel;

    bool                m_bIsActive           :1;
    bool                m_bIsConstant         :1;
    bool                m_bIsHidden           :1;
    bool                m_bDocChgdInDragging  :1;
    bool                m_bIsInternalDrag     :1;
    bool                m_bIsRoot             :1;
    bool                m_bIsIdleClear        :1;
    bool                m_bIsLastReadOnly     :1;
    bool                m_bIsOutlineMoveable  :1;
    bool                m_bViewHasChanged     :1;
    bool                m_bIsImageListInitialized : 1;

    static bool         bIsInDrag;

    bool                m_bIsKeySpace;
    Rectangle           m_aOldRectangle;

    void                FindActiveTypeAndRemoveUserData();

    using SvTreeListBox::ExecuteDrop;
    using SvTreeListBox::EditEntry;

protected:
    virtual void    RequestHelp( const HelpEvent& rHEvt ) override;
    virtual void    InitEntry(SvTreeListEntry*, const OUString&, const Image&, const Image&, SvLBoxButtonKind) override;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;

    SwNavigationPI* GetParentWindow();

    virtual void    StartDrag( sal_Int8 nAction, const Point& rPosPixel ) override;
    virtual void    DragFinished( sal_Int8 ) override;
    virtual sal_Int8 AcceptDrop( const AcceptDropEvent& rEvt ) override;

    virtual sal_Int8 ExecuteDrop( const ExecuteDropEvent& rEvt ) override;

    bool        FillTransferData( TransferDataContainer& rTransfer,
                                            sal_Int8& rDragMode );
    bool            HasContentChanged();

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
    virtual void    Paint( vcl::RenderContext& rRenderContext, const Rectangle& rRect ) override;
    virtual void    MouseButtonDown( const MouseEvent& rMEvt ) override;

    void            EditEntry( SvTreeListEntry* pEntry, EditEntryMode nMode );

    void            GotoContent(SwContent* pCnt);
    static void     SetInDrag(bool bSet) {bIsInDrag = bSet;}

    virtual std::unique_ptr<PopupMenu> CreateContextMenu() override;
    virtual void    ExcecuteContextMenuAction( sal_uInt16 nSelectedPopupEntry ) override;

public:
    SwContentTree(vcl::Window* pParent, const ResId& rResId);
    virtual ~SwContentTree();
    virtual void dispose() override;
    OUString        GetEntryAltText( SvTreeListEntry* pEntry ) const override;
    OUString        GetEntryLongDescription( SvTreeListEntry* pEntry ) const override;
    SdrObject*      GetDrawingObjectsByContent(const SwContent *pCnt);

    bool            ToggleToRoot();
    void            SetRootType(ContentTypeId nType);
    void            Display( bool bActiveView );
    void            Clear();
    void            SetHiddenShell(SwWrtShell* pSh);
    void            ShowHiddenShell();
    void            ShowActualView();
    void            SetActiveShell(SwWrtShell* pSh);
    void            SetConstantShell(SwWrtShell* pSh);

    SwWrtShell*     GetWrtShell()
                        {return m_bIsActive||m_bIsConstant ?
                                    m_pActiveShell :
                                        m_pHiddenShell;}

    static bool     IsInDrag() {return bIsInDrag;}

    sal_uInt8       GetOutlineLevel()const {return m_nOutlineLevel;}
    void            SetOutlineLevel(sal_uInt8 nSet);

    virtual bool    Expand( SvTreeListEntry* pParent ) override;

    virtual bool    Collapse( SvTreeListEntry* pParent ) override;

    void            ExecCommand(sal_uInt16 nCmd, bool bModifier);

    void            ShowTree();
    void            HideTree();

    bool            IsConstantView() {return m_bIsConstant;}
    bool            IsActiveView()   {return m_bIsActive;}
    bool            IsHiddenView()   {return m_bIsHidden;}

    const SwWrtShell*   GetActiveWrtShell() {return m_pActiveShell;}
    SwWrtShell*         GetHiddenWrtShell() {return m_pHiddenShell;}

    DECL_LINK_TYPED( ContentDoubleClickHdl, SvTreeListBox*, bool );
    DECL_LINK_TYPED( TimerUpdate, Timer *, void );

    virtual sal_IntPtr GetTabPos( SvTreeListEntry*, SvLBoxTab* ) override;
    virtual void    RequestingChildren( SvTreeListEntry* pParent ) override;
    virtual void    GetFocus() override;
    virtual void    KeyInput(const KeyEvent& rKEvt) override;

    virtual bool    Select( SvTreeListEntry* pEntry, bool bSelect=true ) override;

    using Control::Notify; // FIXME why do we have 2 of these
    virtual void Notify(SfxBroadcaster& rBC, SfxHint const& rHint) override;

};

// TreeListBox for global documents

class SwLBoxString : public SvLBoxString
{
public:

    SwLBoxString( SvTreeListEntry* pEntry, sal_uInt16 nFlags,
        const OUString& rStr ) : SvLBoxString(pEntry,nFlags,rStr)
    {
    }

    virtual void Paint(const Point& rPos, SvTreeListBox& rDev, vcl::RenderContext& rRenderContext,
                       const SvViewDataEntry* pView, const SvTreeListEntry& rEntry) override;
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
    SvTreeListEntry*        pEmphasisEntry; // Drag'n Drop emphasis
    SvTreeListEntry*        pDDSource;      // source for Drag'n Drop
    SwGlblDocContents*      pSwGlblDocContents; // array with sorted content

    VclPtr<vcl::Window>     pDefParentWin;
    SwGlblDocContent*       pDocContent;
    sfx2::DocumentInserter* pDocInserter;

    bool                bIsInternalDrag     :1;
    bool                bLastEntryEmphasis  :1; // Drag'n Drop
    bool                bIsImageListInitialized : 1;

    static const SfxObjectShell* pShowShell;

    void        InsertRegion( const SwGlblDocContent* _pContent,
                              const css::uno::Sequence< OUString >& _rFiles );

    DECL_LINK_TYPED(  DialogClosedHdl, sfx2::FileDialogHelper*, void );

    using SvTreeListBox::DoubleClickHdl;
    using SvTreeListBox::ExecuteDrop;
    using Window::Update;

protected:

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
    virtual void    InitEntry(SvTreeListEntry*, const OUString&, const Image&, const Image&, SvLBoxButtonKind) override;

    void            Clear();

    DECL_LINK_TYPED( PopupHdl, Menu*, bool );
    DECL_LINK_TYPED( Timeout, Timer*, void );
    DECL_LINK_TYPED( DoubleClickHdl, SvTreeListBox*, bool );

    SwNavigationPI* GetParentWindow();

    void            OpenDoc(const SwGlblDocContent*);
    void            GotoContent(const SwGlblDocContent*);
    sal_uInt16          GetEnableFlags() const;

    static const SfxObjectShell*    GetShowShell() {return pShowShell;}
    static void     SetShowShell(const SfxObjectShell*pSet) {pShowShell = pSet;}
    DECL_STATIC_LINK_TYPED(SwGlobalTree, ShowFrameHdl, void*, void);

    virtual std::unique_ptr<PopupMenu> CreateContextMenu() override;
    virtual void    ExcecuteContextMenuAction( sal_uInt16 nSelectedPopupEntry ) override;

public:
    SwGlobalTree(vcl::Window* pParent, const ResId& rResId);
    virtual ~SwGlobalTree();
    virtual void        dispose() override;

    void                TbxMenuHdl(sal_uInt16 nTbxId, ToolBox* pBox);
    void                InsertRegion( const SwGlblDocContent* pCont,
                                        const OUString* pFileName = nullptr );
    void                EditContent(const SwGlblDocContent* pCont );

    void                ShowTree();
    void                HideTree();

    void                ExecCommand(sal_uInt16 nCmd);

    void                Display(bool bOnlyUpdateUserData = false);

    bool                Update(bool bHard);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
