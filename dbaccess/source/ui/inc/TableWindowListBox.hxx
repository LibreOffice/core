/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TableWindowListBox.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-01 15:19:44 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef DBAUI_TABLEWINDOWLISTBOX_HXX
#define DBAUI_TABLEWINDOWLISTBOX_HXX

#ifndef _SVTREEBOX_HXX
#include <svtools/svtreebx.hxx>
#endif

#ifndef _DBACCESS_UI_CALLBACKS_HXX_
#include "callbacks.hxx"
#endif

struct AcceptDropEvent;
struct ExecuteDropEvent;
namespace dbaui
{
    class OTableWindowListBox;
    struct OJoinExchangeData
    {
    public:
        OTableWindowListBox*    pListBox;       // die ListBox innerhalb desselben (daraus kann man sich das TabWin und daraus den WinName besorgen)
        SvLBoxEntry*            pEntry;         // der Eintrag, der gedraggt oder auf den gedroppt wurde

        OJoinExchangeData(OTableWindowListBox* pBox);
        OJoinExchangeData() : pListBox(NULL), pEntry(NULL) { }
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
        DECL_LINK( OnDoubleClick, SvTreeListBox* );
        DECL_LINK( ScrollUpHdl, SvTreeListBox* );
        DECL_LINK( ScrollDownHdl, SvTreeListBox* );
        DECL_LINK( DropHdl, void* );
        DECL_LINK( LookForUiHdl, void* );

        Timer                       m_aScrollTimer;
        Point                       m_aMousePos;

        OTableWindow*               m_pTabWin;
        sal_Int32                   m_nDropEvent;
        sal_Int32                   m_nUiEvent;
        OJoinDropData               m_aDropInfo;

        BOOL                        m_bReallyScrolled : 1;
        BOOL                        m_bDragSource : 1;

    protected:
        virtual void LoseFocus();
        virtual void GetFocus();
        virtual void NotifyScrolled();
        virtual void NotifyEndScroll();

        virtual long PreNotify(NotifyEvent& rNEvt);

        virtual void dragFinished( );



    public:
        OTableWindowListBox(OTableWindow* pParent);
        virtual ~OTableWindowListBox();

        // DnD stuff
        virtual void        StartDrag( sal_Int8 nAction, const Point& rPosPixel );
        virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt );
        virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt );

        // window
        virtual void Command(const CommandEvent& rEvt);

        OTableWindow* GetTabWin(){ return m_pTabWin; }
        SvLBoxEntry* GetEntryFromText( const String& rEntryText );

    private:
        using SvTreeListBox::ExecuteDrop;
    };
}
#endif // DBAUI_TABLEWINDOWLISTBOX_HXX



