/*************************************************************************
 *
 *  $RCSfile: TableWindowListBox.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 17:52:37 $
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
        DECL_LINK( DoubleClickHdl, SvTreeListBox* );
        DECL_LINK( ScrollUpHdl, SvTreeListBox* );
        DECL_LINK( ScrollDownHdl, SvTreeListBox* );
        DECL_LINK( DropHdl, void* );

        Timer                       m_aScrollTimer;
        Point                       m_aMousePos;

        OTableWindow*               m_pTabWin;
        sal_Int32                   m_nDropEvent;
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
        OTableWindowListBox(OTableWindow* pParent, const String& rDatabaseName, const String& rTableName);
        virtual ~OTableWindowListBox();

        // DnD stuff
        virtual void        StartDrag( sal_Int8 nAction, const Point& rPosPixel );
        virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt );
        virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt );

        // window
        virtual void Command(const CommandEvent& rEvt);

        OTableWindow* GetTabWin(){ return m_pTabWin; }
        SvLBoxEntry* GetEntryFromText( const String& rEntryText );


    };
}
#endif // DBAUI_TABLEWINDOWLISTBOX_HXX



