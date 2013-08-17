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
#ifndef DBAUI_DBTREELISTBOX_HXX
#define DBAUI_DBTREELISTBOX_HXX

#include "ScrollHelper.hxx"
#include "moduledbu.hxx"

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <svtools/treelistbox.hxx>
#include <vcl/timer.hxx>

#include <set>

namespace dbaui
{
    struct DBTreeEditedEntry
    {
        SvTreeListEntry*    pEntry;
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
        OModuleClient               m_aModuleClient;
        OScrollHelper               m_aScrollHelper;
        Timer                       m_aTimer; // is needed for table updates
        Point                       m_aMousePos;
        std::set<SvTreeListEntry*>      m_aSelectedEntries;
        SvTreeListEntry*                m_pDragedEntry;
        IControlActionListener*     m_pActionListener;
        IContextMenuProvider*
                                    m_pContextMenuProvider;

        Link                        m_aPreExpandHandler;    // handler to be called before a node is expanded
        Link                        m_aSelChangeHdl;        // handlet to be called (asynchronously) when the selection changes in any way
        Link                        m_aCutHandler;          // called when someone press CTRL+X
        Link                        m_aCopyHandler;         // called when someone press CTRL+C
        Link                        m_aPasteHandler;        // called when someone press CTRL+V
        Link                        m_aDeleteHandler;       // called when someone press DELETE Key
        Link                        m_aEditingHandler;      // called before someone will edit an entry
        Link                        m_aEditedHandler;       // called after someone edited an entry
        Link                        m_aEnterKeyHdl;

        sal_Bool                    m_bHandleEnterKey;

    private:
        void init();
        DECL_LINK( OnTimeOut, void* );
        DECL_LINK( OnResetEntry, SvTreeListEntry* );
        DECL_LINK( ScrollUpHdl, SvTreeListBox* );
        DECL_LINK( ScrollDownHdl, SvTreeListBox* );

    public:
        DBTreeListBox( Window* pParent
            ,WinBits nWinStyle=0
            ,sal_Bool _bHandleEnterKey = sal_False);
        DBTreeListBox( Window* pParent
            ,const ResId& rResId
            ,sal_Bool _bHandleEnterKey = sal_False);
        ~DBTreeListBox();

        void                    setControlActionListener( IControlActionListener* _pListener ) { m_pActionListener = _pListener; }
        void                    setContextMenuProvider( IContextMenuProvider* _pContextMenuProvider ) { m_pContextMenuProvider = _pContextMenuProvider; }

        void    SetPreExpandHandler(const Link& _rHdl)  { m_aPreExpandHandler = _rHdl; }
        void    SetSelChangeHdl( const Link& _rHdl )    { m_aSelChangeHdl = _rHdl; }
        void    setCutHandler(const Link& _rHdl)        { m_aCutHandler = _rHdl; }
        void    setCopyHandler(const Link& _rHdl)       { m_aCopyHandler = _rHdl; }
        void    setPasteHandler(const Link& _rHdl)      { m_aPasteHandler = _rHdl; }
        void    setDeleteHandler(const Link& _rHdl)     { m_aDeleteHandler = _rHdl; }
        void    setEditingHandler(const Link& _rHdl)    { m_aEditingHandler = _rHdl; }
        void    setEditedHandler(const Link& _rHdl)     { m_aEditedHandler = _rHdl; }

        // modified the given entry so that the expand handler is called whenever the entry is expanded
        // (normally, the expand handler is called only once)
        void            EnableExpandHandler(SvTreeListEntry* _pEntry);

        SvTreeListEntry*    GetEntryPosByName( const String& aName, SvTreeListEntry* pStart = NULL, const IEntryFilter* _pFilter = NULL ) const;
        virtual void    RequestingChildren( SvTreeListEntry* pParent );
        virtual void    SelectHdl();
        virtual void    DeselectHdl();
        // Window
        virtual void    KeyInput( const KeyEvent& rKEvt );

        virtual void    StateChanged( StateChangedType nStateChange );
        virtual void    InitEntry(SvTreeListEntry* pEntry, const OUString& aStr, const Image& aCollEntryBmp, const Image& aExpEntryBmp, SvLBoxButtonKind eButtonKind);

        // enable editing for tables/views and queries
        virtual sal_Bool    EditingEntry( SvTreeListEntry* pEntry, Selection& );
        virtual sal_Bool    EditedEntry( SvTreeListEntry* pEntry, const OUString& rNewText );

        virtual sal_Bool    DoubleClickHdl();

        virtual PopupMenu* CreateContextMenu( void );
        virtual void    ExcecuteContextMenuAction( sal_uInt16 nSelectedPopupEntry );

        void            SetEnterKeyHdl(const Link& rNewHdl) {m_aEnterKeyHdl = rNewHdl;}

        void            clearCurrentSelection() { m_aSelectedEntries.clear(); }

    protected:
        virtual void        MouseButtonDown( const MouseEvent& rMEvt );
        virtual void        RequestHelp( const HelpEvent& rHEvt );

        // DragSourceHelper overridables
        virtual void        StartDrag( sal_Int8 nAction, const Point& rPosPixel );
        // DropTargetHelper overridables
        virtual sal_Int8    AcceptDrop( const AcceptDropEvent& _rEvt );
        virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& _rEvt );

        virtual void        ModelHasRemoved( SvTreeListEntry* pEntry );
        virtual void        ModelHasEntryInvalidated( SvTreeListEntry* pEntry );

        void                implStopSelectionTimer();
        void                implStartSelectionTimer();

    protected:
        using SvTreeListBox::ExecuteDrop;
    };
}

#endif // DBAUI_DBTREELISTBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
