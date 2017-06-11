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
#include "core_resource.hxx"

#include <com/sun/star/frame/XPopupMenuController.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <svtools/treelistbox.hxx>
#include <vcl/timer.hxx>

#include <memory>
#include <set>

namespace dbaui
{
    struct DBTreeEditedEntry
    {
        OUString       aNewText;
    };

    class IEntryFilter
    {
    public:
        virtual bool    includeEntry( SvTreeListEntry* _pEntry ) const = 0;

    protected:
        ~IEntryFilter() {}
    };

    class IControlActionListener;
    class IContextMenuProvider;
    class DBTreeListBox     :public SvTreeListBox
    {
        dbaccess::OModuleClient     m_aModuleClient;
        OScrollHelper               m_aScrollHelper;
        Timer                       m_aTimer; // is needed for table updates
        Point                       m_aMousePos;
        std::set<SvTreeListEntry*>  m_aSelectedEntries;
        SvTreeListEntry*            m_pDragedEntry;
        IControlActionListener*     m_pActionListener;
        IContextMenuProvider*       m_pContextMenuProvider;
        css::uno::Reference<css::frame::XPopupMenuController> m_xMenuController;

        Link<SvTreeListEntry*,bool> m_aPreExpandHandler;    // handler to be called before a node is expanded
        Link<LinkParamNone*,void>   m_aSelChangeHdl;        // handler to be called (asynchronously) when the selection changes in any way
        Link<LinkParamNone*,void>   m_aCopyHandler;         // called when someone press CTRL+C
        Link<LinkParamNone*,void>   m_aPasteHandler;        // called when someone press CTRL+V
        Link<LinkParamNone*,void>   m_aDeleteHandler;       // called when someone press DELETE Key
        Link<DBTreeListBox*,void>   m_aEnterKeyHdl;

    private:
        void init();
        DECL_LINK( OnTimeOut, Timer*, void );
        DECL_LINK( OnResetEntry, void*, void );
        DECL_LINK( ScrollUpHdl, LinkParamNone*, void );
        DECL_LINK( ScrollDownHdl, LinkParamNone*, void );
        DECL_LINK( MenuEventListener, VclMenuEvent&, void );

    public:
        DBTreeListBox( vcl::Window* pParent, WinBits nWinStyle=0);
        virtual ~DBTreeListBox() override;
        virtual void dispose() override;

        void                    setControlActionListener( IControlActionListener* _pListener ) { m_pActionListener = _pListener; }
        void                    setContextMenuProvider( IContextMenuProvider* _pContextMenuProvider ) { m_pContextMenuProvider = _pContextMenuProvider; }

        void    SetPreExpandHandler(const Link<SvTreeListEntry*,bool>& _rHdl)  { m_aPreExpandHandler = _rHdl; }
        void    SetSelChangeHdl( const Link<LinkParamNone*,void>& _rHdl )      { m_aSelChangeHdl = _rHdl; }
        void    setCopyHandler(const Link<LinkParamNone*,void>& _rHdl)         { m_aCopyHandler = _rHdl; }
        void    setPasteHandler(const Link<LinkParamNone*,void>& _rHdl)        { m_aPasteHandler = _rHdl; }
        void    setDeleteHandler(const Link<LinkParamNone*,void>& _rHdl)       { m_aDeleteHandler = _rHdl; }

        // modified the given entry so that the expand handler is called whenever the entry is expanded
        // (normally, the expand handler is called only once)
        void            EnableExpandHandler(SvTreeListEntry* _pEntry);

        SvTreeListEntry*    GetEntryPosByName( const OUString& aName, SvTreeListEntry* pStart = nullptr, const IEntryFilter* _pFilter = nullptr ) const;
        virtual void    RequestingChildren( SvTreeListEntry* pParent ) override;
        virtual void    SelectHdl() override;
        virtual void    DeselectHdl() override;
        // Window
        virtual void    KeyInput( const KeyEvent& rKEvt ) override;

        virtual void    StateChanged( StateChangedType nStateChange ) override;
        virtual void    InitEntry(SvTreeListEntry* pEntry, const OUString& aStr, const Image& aCollEntryBmp, const Image& aExpEntryBmp, SvLBoxButtonKind eButtonKind) override;

        // enable editing for tables/views and queries
        virtual bool    EditingEntry( SvTreeListEntry* pEntry, Selection& ) override;
        virtual bool    EditedEntry( SvTreeListEntry* pEntry, const OUString& rNewText ) override;

        virtual bool    DoubleClickHdl() override;

        virtual VclPtr<PopupMenu> CreateContextMenu() override;
        virtual void    ExecuteContextMenuAction( sal_uInt16 nSelectedPopupEntry ) override;

        void            SetEnterKeyHdl(const Link<DBTreeListBox*,void>& rNewHdl) {m_aEnterKeyHdl = rNewHdl;}

        void            clearCurrentSelection() { m_aSelectedEntries.clear(); }

    protected:
        virtual void        MouseButtonDown( const MouseEvent& rMEvt ) override;
        virtual void        RequestHelp( const HelpEvent& rHEvt ) override;

        // DragSourceHelper overridables
        virtual void        StartDrag( sal_Int8 nAction, const Point& rPosPixel ) override;
        // DropTargetHelper overridables
        virtual sal_Int8    AcceptDrop( const AcceptDropEvent& _rEvt ) override;
        virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& _rEvt ) override;

        virtual void        ModelHasRemoved( SvTreeListEntry* pEntry ) override;
        virtual void        ModelHasEntryInvalidated( SvTreeListEntry* pEntry ) override;

        void                implStopSelectionTimer();
        void                implStartSelectionTimer();

    protected:
        using SvTreeListBox::ExecuteDrop;
    };
}

#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_DBTREELISTBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
