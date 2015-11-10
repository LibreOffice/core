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
#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_TABLEWINDOWLISTBOX_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_TABLEWINDOWLISTBOX_HXX

#include <svtools/treelistbox.hxx>
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
        SvTreeListEntry*            pEntry;         // the entry, which was dragged or to which was dropped on

        OJoinExchangeData(OTableWindowListBox* pBox);
        OJoinExchangeData() : pListBox(nullptr), pEntry(nullptr) { }
    };
    struct OJoinDropData
    {
        OJoinExchangeData aSource;
        OJoinExchangeData aDest;
    };

    class OTableWindow;
    class OTableWindowListBox
            :public SvTreeListBox
            ,public IDragTransferableListener
    {
        DECL_LINK_TYPED( OnDoubleClick, SvTreeListBox*, bool );
        DECL_LINK_TYPED( ScrollUpHdl, Timer*, void );
        DECL_LINK_TYPED( ScrollDownHdl, Timer*, void );
        DECL_LINK_TYPED( DropHdl, void*, void );
        DECL_LINK_TYPED( LookForUiHdl, void*, void );

        Timer                       m_aScrollTimer;
        Point                       m_aMousePos;

        VclPtr<OTableWindow>        m_pTabWin;
        ImplSVEvent *               m_nDropEvent;
        ImplSVEvent *               m_nUiEvent;
        OJoinDropData               m_aDropInfo;

        bool                        m_bReallyScrolled : 1;

    protected:
        virtual void LoseFocus() override;
        virtual void GetFocus() override;
        virtual void NotifyScrolled() override;
        virtual void NotifyEndScroll() override;

        virtual bool PreNotify(NotifyEvent& rNEvt) override;

        virtual void dragFinished( ) override;



    public:
        OTableWindowListBox(OTableWindow* pParent);
        virtual ~OTableWindowListBox();
        virtual void dispose() override;

        // DnD stuff
        virtual void        StartDrag( sal_Int8 nAction, const Point& rPosPixel ) override;
        virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt ) override;
        virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt ) override;

        // window
        virtual void Command(const CommandEvent& rEvt) override;

        OTableWindow* GetTabWin(){ return m_pTabWin; }
        SvTreeListEntry* GetEntryFromText( const OUString& rEntryText );

    private:
        using SvTreeListBox::ExecuteDrop;
    };
}
#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_TABLEWINDOWLISTBOX_HXX



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
