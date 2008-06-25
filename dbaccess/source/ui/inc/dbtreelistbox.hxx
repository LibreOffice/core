/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dbtreelistbox.hxx,v $
 * $Revision: 1.13 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef DBAUI_DBTREELISTBOX_HXX
#define DBAUI_DBTREELISTBOX_HXX

#ifndef _SVTREEBOX_HXX
#include <svtools/svtreebx.hxx>
#endif
#ifndef _SV_TIMER_HXX
#include <vcl/timer.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef DBAUI_SCROLLHELPER_HXX
#include "ScrollHelper.hxx"
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif


namespace dbaui
{
    struct DBTreeEditedEntry
    {
        SvLBoxEntry*    pEntry;
        XubString       aNewText;
    };

    class IEntryFilter
    {
    public:
        virtual bool    includeEntry( SvLBoxEntry* _pEntry ) const = 0;
    };

    //========================================================================
    class IControlActionListener;
    class IContextMenuProvider;
    class DBTreeListBox     :public SvTreeListBox
    {
        OModuleClient               m_aModuleClient;
        OScrollHelper               m_aScrollHelper;
        Timer                       m_aTimer; // is needed for table updates
        Point                       m_aMousePos;
        SvLBoxEntry*                m_pSelectedEntry;
        SvLBoxEntry*                m_pDragedEntry;
        IControlActionListener*     m_pActionListener;
        IContextMenuProvider*
                                    m_pContextMenuProvider;

        Link                        m_aPreExpandHandler;    // handler to be called before a node is expanded
        Link                        m_aCutHandler;          // called when someone press CTRL+X
        Link                        m_aCopyHandler;         // called when someone press CTRL+C
        Link                        m_aPasteHandler;        // called when someone press CTRL+V
        Link                        m_aDeleteHandler;       // called when someone press DELETE Key
        Link                        m_aEditingHandler;      // called before someone will edit an entry
        Link                        m_aEditedHandler;       // called after someone edited an entry
        Link                        m_aEnterKeyHdl;


        sal_Int32                   m_nSelectLock;
        sal_Bool                    m_bHandleEnterKey;

    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xORB;

    private:
        void init();
        DECL_LINK( OnTimeOut, void* );
        DECL_LINK( OnResetEntry, SvLBoxEntry* );
        DECL_LINK( ScrollUpHdl, SvTreeListBox* );
        DECL_LINK( ScrollDownHdl, SvTreeListBox* );

    public:
        DBTreeListBox( Window* pParent
            ,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB
            ,WinBits nWinStyle=0
            ,sal_Bool _bHandleEnterKey = sal_False);
        DBTreeListBox( Window* pParent
            ,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB
            ,const ResId& rResId
            ,sal_Bool _bHandleEnterKey = sal_False);
        ~DBTreeListBox();

        void                    setControlActionListener( IControlActionListener* _pListener ) { m_pActionListener = _pListener; }
        void                    setContextMenuProvider( IContextMenuProvider* _pContextMenuProvider ) { m_pContextMenuProvider = _pContextMenuProvider; }

        inline void setORB(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _xORB) { m_xORB = _xORB; }


        void    SetPreExpandHandler(const Link& _rHdl) { m_aPreExpandHandler = _rHdl; }
        Link    GetPreExpandHandler() const         { return m_aPreExpandHandler; }

        void    setCutHandler(const Link& _rHdl)    { m_aCutHandler = _rHdl; }
        Link    getCutHandler() const               { return m_aCutHandler; }

        void    setCopyHandler(const Link& _rHdl)   { m_aCopyHandler = _rHdl; }
        Link    getCopyHandler() const              { return m_aCopyHandler; }

        void    setPasteHandler(const Link& _rHdl)  { m_aPasteHandler = _rHdl; }
        Link    getPasteHandler() const             { return m_aPasteHandler; }

        void    setDeleteHandler(const Link& _rHdl) { m_aDeleteHandler = _rHdl; }
        Link    getDeleteHandler() const            { return m_aDeleteHandler; }

        void    setEditingHandler(const Link& _rHdl){ m_aEditingHandler = _rHdl; }
        Link    getEditingHandler() const           { return m_aEditingHandler; }

        void    setEditedHandler(const Link& _rHdl) { m_aEditedHandler = _rHdl; }
        Link    getEditedHandler() const            { return m_aEditedHandler; }

        inline SvLBoxEntry* GetSelectedEntry() const { return m_pSelectedEntry; }
        // modified the given entry so that the expand handler is called whenever the entry is expanded
        // (normally, the expand handler is called only once)
        void            EnableExpandHandler(SvLBoxEntry* _pEntry);

        SvLBoxEntry*    GetEntryPosByName( const String& aName, SvLBoxEntry* pStart = NULL, const IEntryFilter* _pFilter = NULL ) const;
        virtual void    RequestingChilds( SvLBoxEntry* pParent );
        virtual void    SelectHdl();
        virtual void    DeselectHdl();
        // Window
        virtual void    KeyInput( const KeyEvent& rKEvt );

        virtual void    StateChanged( StateChangedType nStateChange );
        virtual void    InitEntry( SvLBoxEntry* pEntry, const XubString& aStr, const Image& aCollEntryBmp, const Image& aExpEntryBmp, SvLBoxButtonKind eButtonKind);

        virtual void    SelectEntry(SvLBoxEntry* _pEntry);
        // enable editing for tables/views and queries
        virtual BOOL    EditingEntry( SvLBoxEntry* pEntry, Selection& );
        virtual BOOL    EditedEntry( SvLBoxEntry* pEntry, const XubString& rNewText );

        virtual BOOL    DoubleClickHdl();

        virtual PopupMenu* CreateContextMenu( void );
        virtual void    ExcecuteContextMenuAction( USHORT nSelectedPopupEntry );

        sal_Int32       lockAutoSelect();
        sal_Int32       unlockAutoSelect();
        sal_Int32       locked() const { return m_nSelectLock; }

        void            SetEnterKeyHdl(const Link& rNewHdl) {m_aEnterKeyHdl = rNewHdl;}

        inline void     clearCurrentSelectionEntry() { m_pSelectedEntry = NULL; }

    protected:
        virtual void        MouseButtonDown( const MouseEvent& rMEvt );
        virtual void        RequestHelp( const HelpEvent& rHEvt );

        // DragSourceHelper overridables
        virtual void        StartDrag( sal_Int8 nAction, const Point& rPosPixel );
        // DropTargetHelper overridables
        virtual sal_Int8    AcceptDrop( const AcceptDropEvent& _rEvt );
        virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& _rEvt );

        virtual void        ModelHasRemoved( SvListEntry* pEntry );
        virtual void        ModelHasEntryInvalidated( SvListEntry* pEntry );

        void                implSelected(SvLBoxEntry* _pSelected);

    protected:
        using SvTreeListBox::ExecuteDrop;
    };
}

#endif // DBAUI_DBTREELISTBOX_HXX
