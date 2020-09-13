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

#include <vcl/transfer.hxx>
#include <vcl/InterimItemWindow.hxx>
#include "callbacks.hxx"

struct AcceptDropEvent;
struct ExecuteDropEvent;
namespace dbaui
{
    class OTableWindowListBox;
    struct OJoinExchangeData
    {
    public:
        VclPtr<OTableWindowListBox>    pListBox;       // the ListBox inside the same (you can get the TabWin and the WinName out of it)
        int                            nEntry;         // the entry, which was dragged or to which was dropped on

        OJoinExchangeData(OTableWindowListBox* pBox);
        OJoinExchangeData() : pListBox(nullptr), nEntry(-1) { }
    };

    struct OJoinDropData
    {
        OJoinExchangeData aSource;
        OJoinExchangeData aDest;
    };

    class OJoinExchObj;
    class OTableWindow;
    class TableWindowListBoxHelper;

    class OTableWindowListBox
            : public InterimItemWindow
            , public IDragTransferableListener
    {
        std::unique_ptr<weld::TreeView> m_xTreeView;
        std::unique_ptr<TableWindowListBoxHelper> m_xDragDropTargetHelper;

        DECL_LINK( OnDoubleClick, weld::TreeView&, bool );
        DECL_LINK( DropHdl, void*, void );
        DECL_LINK( LookForUiHdl, void*, void );
        DECL_LINK( DragBeginHdl, bool&, bool );
        DECL_LINK( ScrollHdl, weld::TreeView&, void );

        rtl::Reference<OJoinExchObj> m_xHelper;

        VclPtr<OTableWindow>        m_pTabWin;
        ImplSVEvent *               m_nDropEvent;
        ImplSVEvent *               m_nUiEvent;
        OJoinDropData               m_aDropInfo;

    protected:
        virtual void LoseFocus() override;
        virtual void GetFocus() override;

        virtual void dragFinished( ) override;

    public:
        OTableWindowListBox(OTableWindow* pParent);
        virtual ~OTableWindowListBox() override;
        virtual void dispose() override;

        const weld::TreeView& get_widget() const { return *m_xTreeView; }
        weld::TreeView& get_widget() { return *m_xTreeView; }

        // DnD stuff
        sal_Int8 AcceptDrop(const AcceptDropEvent& rEvt);
        sal_Int8 ExecuteDrop(const ExecuteDropEvent& rEvt);

        // window
        virtual void Command(const CommandEvent& rEvt) override;

        OTableWindow* GetTabWin(){ return m_pTabWin; }
        int GetEntryFromText( const OUString& rEntryText );
    };

    class TableWindowListBoxHelper final : public DropTargetHelper
    {
    private:
        OTableWindowListBox& m_rParent;

        virtual sal_Int8 AcceptDrop(const AcceptDropEvent& rEvt) override
        {
            return m_rParent.AcceptDrop(rEvt);
        }

        virtual sal_Int8 ExecuteDrop(const ExecuteDropEvent& rEvt) override
        {
            return m_rParent.ExecuteDrop(rEvt);
        }

    public:
        TableWindowListBoxHelper(OTableWindowListBox& rParent, const css::uno::Reference<css::datatransfer::dnd::XDropTarget>& rDropTarget)
            : DropTargetHelper(rDropTarget)
            , m_rParent(rParent)
        {
        }
    };
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
