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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_DBTREELISTBOX_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_DBTREELISTBOX_HXX

#include "ScrollHelper.hxx"

#include <com/sun/star/frame/XPopupMenuController.hpp>

#include <vcl/InterimItemWindow.hxx>
#include <vcl/transfer.hxx>
#include <vcl/timer.hxx>
#include <vcl/weld.hxx>

#include <memory>
#include <set>

#include "dbexchange.hxx"

namespace dbaui
{
    struct DBTreeEditedEntry
    {
        OUString       aNewText;
    };

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
        rtl::Reference<ODataClipboard> m_xHelper;

        Link<LinkParamNone*,void>   m_aSelChangeHdl;        // handler to be called (asynchronously) when the selection changes in any way
        Link<LinkParamNone*,void>   m_aCopyHandler;         // called when someone press CTRL+C
        Link<LinkParamNone*,void>   m_aPasteHandler;        // called when someone press CTRL+V
        Link<LinkParamNone*,void>   m_aDeleteHandler;       // called when someone press DELETE Key

        DECL_LINK(OnTimeOut, Timer*, void);

    protected:
        void implStopSelectionTimer();
        void implStartSelectionTimer();

        virtual bool DoChildKeyInput(const KeyEvent& rKEvt);
        virtual bool DoContextMenu(const CommandEvent& rCEvt);

    public:
        TreeListBox(std::unique_ptr<weld::TreeView> xTreeView);
        virtual ~TreeListBox();

        std::unique_ptr<weld::TreeIter> GetEntryPosByName(const OUString& rName,
                                                          const weld::TreeIter* pStart = nullptr,
                                                          const IEntryFilter* pFilter = nullptr) const;

        std::unique_ptr<weld::TreeIter> GetRootLevelParent(const weld::TreeIter* pEntry) const;

        void setControlActionListener(IControlActionListener* pListener) { m_pActionListener = pListener; }
        void setContextMenuProvider(IContextMenuProvider* pContextMenuProvider) { m_pContextMenuProvider = pContextMenuProvider; }

        weld::TreeView& GetWidget() { return *m_xTreeView; }
        const weld::TreeView& GetWidget() const { return *m_xTreeView; }

        ODataClipboard& GetDataTransfer() { return *m_xHelper; }

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
    public:
        InterimDBTreeListBox(vcl::Window* pParent);
        virtual void dispose() override;
        virtual ~InterimDBTreeListBox() override;
    protected:
        virtual bool DoChildKeyInput(const KeyEvent& rKEvt) override;
        virtual bool DoContextMenu(const CommandEvent& rCEvt) override;
    };
}

#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_DBTREELISTBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
