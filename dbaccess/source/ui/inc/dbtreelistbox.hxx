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
#include "moduledbu.hxx"

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <svtools/treelistbox.hxx>
#include <vcl/timer.hxx>

#include <memory>
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
        std::set<SvTreeListEntry*>  m_aSelectedEntries;
        SvTreeListEntry*            m_pDragedEntry;
        IControlActionListener*     m_pActionListener;
        IContextMenuProvider*
                                    m_pContextMenuProvider;

        Link<SvTreeListEntry*,bool> m_aPreExpandHandler;    // handler to be called before a node is expanded
        Link<LinkParamNone*,void>   m_aSelChangeHdl;        // handler to be called (asynchronously) when the selection changes in any way
        Link<LinkParamNone*,void>   m_aCutHandler;          // called when someone press CTRL+X
        Link<LinkParamNone*,void>   m_aCopyHandler;         // called when someone press CTRL+C
        Link<LinkParamNone*,void>   m_aPasteHandler;        // called when someone press CTRL+V
        Link<LinkParamNone*,void>   m_aDeleteHandler;       // called when someone press DELETE Key
        Link<DBTreeListBox*,void>   m_aEnterKeyHdl;

        bool                        m_bHandleEnterKey;

    private:
        void init();
        DECL_LINK_TYPED( OnTimeOut, Timer*, void );
        DECL_LINK_TYPED( OnResetEntry, void*, void );
        DECL_LINK_TYPED( ScrollUpHdl, LinkParamNone*, void );
        DECL_LINK_TYPED( ScrollDownHdl, LinkParamNone*, void );

    public:
        DBTreeListBox( vcl::Window* pParent
            ,WinBits nWinStyle=0
            ,bool _bHandleEnterKey = false);
        virtual ~DBTreeListBox();
        virtual void dispose() SAL_OVERRIDE;

        void                    setControlActionListener( IControlActionListener* _pListener ) { m_pActionListener = _pListener; }
        void                    setContextMenuProvider( IContextMenuProvider* _pContextMenuProvider ) { m_pContextMenuProvider = _pContextMenuProvider; }

        void    SetPreExpandHandler(const Link<SvTreeListEntry*,bool>& _rHdl)  { m_aPreExpandHandler = _rHdl; }
        void    SetSelChangeHdl( const Link<LinkParamNone*,void>& _rHdl )      { m_aSelChangeHdl = _rHdl; }
        void    setCutHandler(const Link<LinkParamNone*,void>& _rHdl)          { m_aCutHandler = _rHdl; }
        void    setCopyHandler(const Link<LinkParamNone*,void>& _rHdl)         { m_aCopyHandler = _rHdl; }
        void    setPasteHandler(const Link<LinkParamNone*,void>& _rHdl)        { m_aPasteHandler = _rHdl; }
        void    setDeleteHandler(const Link<LinkParamNone*,void>& _rHdl)       { m_aDeleteHandler = _rHdl; }

        // modified the given entry so that the expand handler is called whenever the entry is expanded
        // (normally, the expand handler is called only once)
        void            EnableExpandHandler(SvTreeListEntry* _pEntry);

        SvTreeListEntry*    GetEntryPosByName( const OUString& aName, SvTreeListEntry* pStart = NULL, const IEntryFilter* _pFilter = NULL ) const;
        virtual void    RequestingChildren( SvTreeListEntry* pParent ) SAL_OVERRIDE;
        virtual void    SelectHdl() SAL_OVERRIDE;
        virtual void    DeselectHdl() SAL_OVERRIDE;
        // Window
        virtual void    KeyInput( const KeyEvent& rKEvt ) SAL_OVERRIDE;

        virtual void    StateChanged( StateChangedType nStateChange ) SAL_OVERRIDE;
        virtual void    InitEntry(SvTreeListEntry* pEntry, const OUString& aStr, const Image& aCollEntryBmp, const Image& aExpEntryBmp, SvLBoxButtonKind eButtonKind) SAL_OVERRIDE;

        // enable editing for tables/views and queries
        virtual bool    EditingEntry( SvTreeListEntry* pEntry, Selection& ) SAL_OVERRIDE;
        virtual bool    EditedEntry( SvTreeListEntry* pEntry, const OUString& rNewText ) SAL_OVERRIDE;

        virtual bool    DoubleClickHdl() SAL_OVERRIDE;

        virtual std::unique_ptr<PopupMenu> CreateContextMenu() SAL_OVERRIDE;
        virtual void    ExcecuteContextMenuAction( sal_uInt16 nSelectedPopupEntry ) SAL_OVERRIDE;

        void            SetEnterKeyHdl(const Link<DBTreeListBox*,void>& rNewHdl) {m_aEnterKeyHdl = rNewHdl;}

        void            clearCurrentSelection() { m_aSelectedEntries.clear(); }

    protected:
        virtual void        MouseButtonDown( const MouseEvent& rMEvt ) SAL_OVERRIDE;
        virtual void        RequestHelp( const HelpEvent& rHEvt ) SAL_OVERRIDE;

        // DragSourceHelper overridables
        virtual void        StartDrag( sal_Int8 nAction, const Point& rPosPixel ) SAL_OVERRIDE;
        // DropTargetHelper overridables
        virtual sal_Int8    AcceptDrop( const AcceptDropEvent& _rEvt ) SAL_OVERRIDE;
        virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& _rEvt ) SAL_OVERRIDE;

        virtual void        ModelHasRemoved( SvTreeListEntry* pEntry ) SAL_OVERRIDE;
        virtual void        ModelHasEntryInvalidated( SvTreeListEntry* pEntry ) SAL_OVERRIDE;

        void                implStopSelectionTimer();
        void                implStartSelectionTimer();

    protected:
        using SvTreeListBox::ExecuteDrop;
    };
}

#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_DBTREELISTBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
