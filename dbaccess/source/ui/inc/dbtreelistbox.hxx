/*************************************************************************
 *
 *  $RCSfile: dbtreelistbox.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:57:37 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef DBAUI_DBTREELISTBOX_HXX
#define DBAUI_DBTREELISTBOX_HXX

#ifndef _SVTREEBOX_HXX
#include <svtools/svtreebx.hxx>
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
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

namespace dbaui
{
    #define TABLE_TYPE  1
    #define VIEW_TYPE   2
    #define FOLDER_TYPE 3

    struct DBTreeEditedEntry
    {
        SvLBoxEntry*    pEntry;
        XubString       aNewText;
    };
    //========================================================================
    class IControlActionListener;
    class IController;
    class DBTreeListBox     :public SvTreeListBox
                            ,public dbaui::OModuleClient
    {
        OScrollHelper               m_aScrollHelper;
        Timer                       m_aTimer; // is needed for table updates
        Point                       m_aMousePos;
        SvLBoxEntry*                m_pSelectedEntry;
        IControlActionListener*     m_pActionListener;
        IController*                m_pContextMenuActionListener;

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
        DECL_LINK( DoubleClickHdl, SvTreeListBox* );
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
        IControlActionListener* getControlActionListener( ) const { return m_pActionListener; }

        void            setContextMenuActionListener( IController* _pConextListener) { m_pContextMenuActionListener = _pConextListener; }
        IController*    getContextMenuActionListener( ) const { return m_pContextMenuActionListener; }

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

        SvLBoxEntry*    GetEntryPosByName(const String& aName,SvLBoxEntry* pStart=NULL) const;
        virtual void    RequestingChilds( SvLBoxEntry* pParent );
        virtual void    SelectHdl();
        virtual void    DeselectHdl();
        // Window
        virtual void    KeyInput( const KeyEvent& rKEvt );

        virtual void    InitEntry( SvLBoxEntry* pEntry, const XubString& aStr, const Image& aCollEntryBmp, const Image& aExpEntryBmp);

        virtual void    SelectEntry(SvLBoxEntry* _pEntry);
        // enable editing for tables/views and queries
        virtual BOOL    EditingEntry( SvLBoxEntry* pEntry, Selection& );
        virtual BOOL    EditedEntry( SvLBoxEntry* pEntry, const XubString& rNewText );

        virtual PopupMenu* CreateContextMenu( void );
        virtual void    ExcecuteContextMenuAction( USHORT nSelectedPopupEntry );

        sal_Int32       lockAutoSelect();
        sal_Int32       unlockAutoSelect();
        sal_Int32       locked() const { return m_nSelectLock; }

        void            SetEnterKeyHdl(const Link& rNewHdl) {m_aEnterKeyHdl = rNewHdl;}

        inline void     clearCurrentSelectionEntry() { m_pSelectedEntry = NULL; }

    protected:
        virtual void        MouseButtonDown( const MouseEvent& rMEvt );
        virtual void        Command( const CommandEvent& rCEvt );

        // DragSourceHelper overridables
        virtual void        StartDrag( sal_Int8 nAction, const Point& rPosPixel );
        // DropTargetHelper overridables
        virtual sal_Int8    AcceptDrop( const AcceptDropEvent& _rEvt );
        virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& _rEvt );

        virtual void        ModelHasRemoved( SvListEntry* pEntry );
        virtual void        ModelHasEntryInvalidated( SvListEntry* pEntry );

        void                implSelected(SvLBoxEntry* _pSelected);
    };
}

#endif // DBAUI_DBTREELISTBOX_HXX
