/*************************************************************************
 *
 *  $RCSfile: TableDesignControl.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: oj $ $Date: 2001-02-14 14:26:29 $
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

#ifndef DBAUI_TABLEDESIGNCONTROL_HXX
#include "TableDesignControl.hxx"
#endif
#ifndef _DBU_RESOURCE_HRC_
#include "dbu_resource.hrc"
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef DBAUI_TABLEDESIGNVIEW_HXX
#include "TableDesignView.hxx"
#endif
#ifndef DBUI_TABLECONTROLLER_HXX
#include "TableController.hxx"
#endif
#ifndef DBACCESS_UI_BROWSER_ID_HXX
#include "browserids.hxx"
#endif
#ifndef _COM_SUN_STAR_UTIL_URL_HPP_
#include <com/sun/star/util/URL.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _DBA_DBACCESS_HELPID_HRC_
#include "dbaccess_helpid.hrc"
#endif

using namespace dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
//--- Defines
#define HANDLE_ID 0

//------------------------------------------------------------------------
OTableRowView::OTableRowView(Window* pParent)
                   :DbBrowseBox(pParent, ModuleRes(RID_DB_TAB_EDITOR),DBBF_NONE,
                                  BROWSER_COLUMNSELECTION | BROWSER_MULTISELECTION | BROWSER_AUTOSIZE_LASTCOL |
                                  BROWSER_KEEPSELECTION | BROWSER_HLINESFULL | BROWSER_VLINESFULL)
                   ,m_nDataPos(-1)
                   ,m_nCurrentPos(-1)
                   ,m_bUpdatable(FALSE)
                   ,m_bCurrentModified(FALSE)
                   ,m_bClipboardFilled(FALSE)
                   ,m_nCurUndoActId(0)
{
}

//------------------------------------------------------------------------
OTableRowView::~OTableRowView()
{
}

//------------------------------------------------------------------------
void OTableRowView::Init()
{
    DbBrowseBox::Init();

//  SetMapMode( MapMode(MAP_TWIP) );
//  GetDataWindow().SetMapMode( GetMapMode() );

    Font aFont( GetDataWindow().GetFont() );
    aFont.SetWeight( WEIGHT_NORMAL );
    GetDataWindow().SetFont( aFont );

    // Font fuer die Ueberschriften auf Light setzen
    aFont = GetFont();
    aFont.SetWeight( WEIGHT_LIGHT );
    SetFont(aFont);

    // HandleColumn, fuer maximal fuenf Ziffern einrichten
    InsertHandleColumn(GetTextWidth('0') * 4/*, TRUE */);

    BrowserMode nMode = BROWSER_COLUMNSELECTION | BROWSER_MULTISELECTION | BROWSER_KEEPSELECTION |
                        BROWSER_HLINESFULL | BROWSER_VLINESFULL | BROWSER_AUTOSIZE_LASTCOL;
    if (IsUpdatable())
        nMode |= BROWSER_HIDECURSOR;

    SetMode(nMode);
}

//------------------------------------------------------------------------
void OTableRowView::KeyInput( const KeyEvent& rEvt )
{
    if (IsUpdatable() && GetSelectRowCount())
    {
        if (rEvt.GetKeyCode().GetCode() == KEY_DELETE &&    // Delete rows
            !rEvt.GetKeyCode().IsShift() &&
            !rEvt.GetKeyCode().IsMod1())
        {
            DeleteRows();
            return;
        }
        if( rEvt.GetKeyCode().GetCode() == KEY_F2 )
        {
            ::com::sun::star::util::URL aUrl;
            aUrl.Complete =::rtl::OUString::createFromAscii(".uno:DBSlots/EditDoc");
            GetView()->getController()->dispatch( aUrl,Sequence< PropertyValue >() );
        }
    }
    DbBrowseBox::KeyInput(rEvt);
}

//------------------------------------------------------------------------
void OTableRowView::SetUpdatable( BOOL bUpdate )
{
    m_bUpdatable = bUpdate;

}

//------------------------------------------------------------------------
void OTableRowView::Command(const CommandEvent& rEvt)
{

    switch (rEvt.GetCommand())
    {
        case COMMAND_CONTEXTMENU:
        {
            if (!rEvt.IsMouseEvent())
            {
                DbBrowseBox::Command(rEvt);
                return;
            }

            USHORT nColId = GetColumnAtXPosPixel(rEvt.GetMousePosPixel().X());
            long   nRow = GetRowAtYPosPixel(rEvt.GetMousePosPixel().Y());
            long   nOldRow = GetCurRow();

            if ( nColId == HANDLE_ID )
            {
                PopupMenu aContextMenu(ModuleRes(RID_TABLEDESIGNROWPOPUPMENU));
                long nSelectRowCount = GetSelectRowCount();
                aContextMenu.EnableItem( SID_CUT, nSelectRowCount != 0);
                aContextMenu.EnableItem( SID_COPY, nSelectRowCount  != 0);
                aContextMenu.EnableItem( SID_PASTE, m_bClipboardFilled );
                aContextMenu.EnableItem( SID_DELETE, IsUpdatable() && nSelectRowCount != 0 );
                switch (aContextMenu.Execute(this, rEvt.GetMousePosPixel()))
                {
                    case SID_CUT:
                        Cut();
                        break;
                    case SID_COPY:
                        Copy();
                        break;
                    case SID_PASTE:
                        Paste( nRow );
                        SetNoSelection();
                        GoToRow( nRow );
                        SeekRow( nRow );
                        break;

                    case SID_DELETE:
                        DeleteRows();
                        break;
                    case SID_TABLEDESIGN_INSERTROWS:
                        InsertNewRows( nRow );
                        SetNoSelection();
                        GoToRow( nRow );
                        SeekRow( nRow );
                        break;
                    default:
                        break;
                }
            }

        }
        default:
            DbBrowseBox::Command(rEvt);
    }

}

//------------------------------------------------------------------------------
void OTableRowView::Cut()
{
    CopyRows();
    DeleteRows();
}

//------------------------------------------------------------------------------
void OTableRowView::Copy()
{
    CopyRows();
}

//------------------------------------------------------------------------------
void OTableRowView::Paste()
{
    OSL_ENSURE(0,"OTableRowView::Paste : (pseudo-) abstract method called !");
}

//------------------------------------------------------------------------------
void OTableRowView::Paste( long nRow )
{
    InsertRows( nRow );
}

//------------------------------------------------------------------------------
DbBrowseBox::RowStatus OTableRowView::GetRowStatus(long nRow) const
{
    if (m_nCurrentPos >= 0 && m_nDataPos == m_nCurrentPos)
        return CURRENT;
    else
        return CLEAN;
}



