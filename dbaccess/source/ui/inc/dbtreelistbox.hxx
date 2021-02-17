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
#pragma once

#include <vcl/InterimItemWindow.hxx>
#include <vcl/transfer.hxx>
#include <vcl/timer.hxx>
#include <vcl/weld.hxx>

#include <memory>

namespace dbaui
{
    class IEntryFilter
    {
    public:
        virtual bool    includeEntry(const void* pUserData) const = 0;

    protected:
        ~IEntryFilter() {}
    };

    class IControlActionListener;
    class IContextMenuProvider;

    class TreeListBox;

    class TreeListBoxDropTarget : public DropTargetHelper
    {
    private:
        TreeListBox& m_rTreeView;

        virtual sal_Int8 AcceptDrop( const AcceptDropEvent& rEvt ) override;
        virtual sal_Int8 ExecuteDrop( const ExecuteDropEvent& rEvt ) override;

    public:
        TreeListBoxDropTarget(TreeListBox& rTreeView);
    };

    class TreeListBox
    {
    protected:
        std::unique_ptr<weld::TreeView> m_xTreeView;
        TreeListBoxDropTarget m_aDropTargetHelper;

        std::unique_ptr<weld::TreeIter> m_xDragedEntry;
        IControlActionListener*     m_pActionListener;
        IContextMenuProvider*       m_pContextMenuProvider;

        DECL_LINK(KeyInputHdl, const KeyEvent&, bool);
        DECL_LINK(SelectHdl, weld::TreeView&, void);
        DECL_LINK(QueryTooltipHdl, const weld::TreeIter&, OUString);
        DECL_LINK(CommandHdl, const CommandEvent&, bool);
        DECL_LINK(DragBeginHdl, bool&, bool);

    private:
        Timer                       m_aTimer; // is needed for table updates
        rtl::Reference<TransferDataContainer> m_xHelper;

        Link<LinkParamNone*,void>   m_aSelChangeHdl;        // handler to be called (asynchronously) when the selection changes in any way
        Link<LinkParamNone*,void>   m_aCopyHandler;         // called when someone press CTRL+C
        Link<LinkParamNone*,void>   m_aPasteHandler;        // called when someone press CTRL+V
        Link<LinkParamNone*,void>   m_aDeleteHandler;       // called when someone press DELETE Key

        DECL_LINK(OnTimeOut, Timer*, void);

    protected:
        void implStopSelectionTimer();
        void implStartSelectionTimer();

        virtual bool DoChildKeyInput(const KeyEvent& rKEvt);

    public:
        TreeListBox(std::unique_ptr<weld::TreeView> xTreeView, bool bSQLType);
        virtual ~TreeListBox();

        std::unique_ptr<weld::TreeIter> GetEntryPosByName(std::u16string_view rName,
                                                          const weld::TreeIter* pStart = nullptr,
                                                          const IEntryFilter* pFilter = nullptr) const;

        std::unique_ptr<weld::TreeIter> GetRootLevelParent(const weld::TreeIter* pEntry) const;

        void setControlActionListener(IControlActionListener* pListener) { m_pActionListener = pListener; }
        void setContextMenuProvider(IContextMenuProvider* pContextMenuProvider) { m_pContextMenuProvider = pContextMenuProvider; }

        weld::TreeView& GetWidget() { return *m_xTreeView; }
        const weld::TreeView& GetWidget() const { return *m_xTreeView; }

        TransferDataContainer& GetDataTransfer() { return *m_xHelper; }

        sal_Int8 AcceptDrop(const AcceptDropEvent& rEvt);
        sal_Int8 ExecuteDrop(const ExecuteDropEvent& rEvt);

        void    SetSelChangeHdl( const Link<LinkParamNone*,void>& _rHdl )      { m_aSelChangeHdl = _rHdl; }
        void    setCopyHandler(const Link<LinkParamNone*,void>& _rHdl)         { m_aCopyHandler = _rHdl; }
        void    setPasteHandler(const Link<LinkParamNone*,void>& _rHdl)        { m_aPasteHandler = _rHdl; }
        void    setDeleteHandler(const Link<LinkParamNone*,void>& _rHdl)       { m_aDeleteHandler = _rHdl; }
    };

    class InterimDBTreeListBox : public InterimItemWindow
                               , public TreeListBox
    {
    private:
        std::unique_ptr<weld::Label> m_xStatusBar;
    public:
        InterimDBTreeListBox(vcl::Window* pParent, bool bSQLType);
        virtual void dispose() override;
        weld::Label& GetStatusBar() { return *m_xStatusBar; }
        virtual ~InterimDBTreeListBox() override;
        void show_container() { m_xContainer->show(); }
    protected:
        virtual bool DoChildKeyInput(const KeyEvent& rKEvt) override;
    };

    class DBTreeViewBase
    {
    protected:
        std::unique_ptr<weld::Builder> m_xBuilder;
        std::unique_ptr<weld::Container> m_xContainer;
        std::unique_ptr<TreeListBox> m_xTreeListBox;
    public:
        DBTreeViewBase(weld::Container* pContainer);
        virtual ~DBTreeViewBase();

        weld::TreeView& GetWidget() { return m_xTreeListBox->GetWidget(); }
        const weld::TreeView& GetWidget() const { return m_xTreeListBox->GetWidget(); }

        TreeListBox& getListBox() const { return *m_xTreeListBox; }

        void hide() { m_xContainer->hide(); }
        void show() { m_xContainer->show(); }
        bool get_visible() const { return m_xContainer->get_visible(); }
    };

    class DBTreeView final : public DBTreeViewBase
    {
    public:
        DBTreeView(weld::Container* pContainer, bool bSQLType);
    };

    class DBTableTreeView final : public DBTreeViewBase
    {
    public:
        DBTableTreeView(weld::Container* pContainer);
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
