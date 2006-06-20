/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TableDesignControl.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 03:32:45 $
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

#ifndef DBAUI_TABLEDESIGNCONTROL_HXX
#include "TableDesignControl.hxx"
#endif
#ifndef _DBU_TBL_HRC_
#include "dbu_tbl.hrc"
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

using namespace ::dbaui;
using namespace ::svt;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
//--- Defines
#define HANDLE_ID 0

DBG_NAME(OTableRowView)
//------------------------------------------------------------------------
OTableRowView::OTableRowView(Window* pParent)
    :EditBrowseBox(pParent, ModuleRes(RID_DB_TAB_EDITOR),EBBF_NONE,
                    BROWSER_COLUMNSELECTION | BROWSER_MULTISELECTION | BROWSER_AUTOSIZE_LASTCOL |
                    BROWSER_KEEPSELECTION | BROWSER_HLINESFULL | BROWSER_VLINESFULL)
    ,m_nDataPos(-1)
    ,m_nCurrentPos(-1)
    ,m_nCurUndoActId(0)
    ,m_bCurrentModified(FALSE)
    ,m_bUpdatable(FALSE)
    ,m_bClipboardFilled(FALSE)
{
    DBG_CTOR(OTableRowView,NULL);

}

//------------------------------------------------------------------------
OTableRowView::~OTableRowView()
{

    DBG_DTOR(OTableRowView,NULL);
}

//------------------------------------------------------------------------
void OTableRowView::Init()
{
    EditBrowseBox::Init();

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
    InsertHandleColumn(static_cast<USHORT>(GetTextWidth('0') * 4)/*, TRUE */);

    BrowserMode nMode = BROWSER_COLUMNSELECTION | BROWSER_MULTISELECTION | BROWSER_KEEPSELECTION |
                        BROWSER_HLINESFULL | BROWSER_VLINESFULL | BROWSER_AUTOSIZE_LASTCOL;
    if (IsUpdatable())
        nMode |= BROWSER_HIDECURSOR;

    SetMode(nMode);
}

//------------------------------------------------------------------------
void OTableRowView::KeyInput( const KeyEvent& rEvt )
{
    if (IsDeleteAllowed(0))
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
            aUrl.Complete =::rtl::OUString::createFromAscii(".uno:DSBEditDoc");
            GetView()->getController()->dispatch( aUrl,Sequence< PropertyValue >() );
        }
    }
    EditBrowseBox::KeyInput(rEvt);
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
                EditBrowseBox::Command(rEvt);
                return;
            }

            USHORT nColId = GetColumnAtXPosPixel(rEvt.GetMousePosPixel().X());
            long   nRow = GetRowAtYPosPixel(rEvt.GetMousePosPixel().Y());

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
                        cut();
                        break;
                    case SID_COPY:
                        copy();
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
            EditBrowseBox::Command(rEvt);
    }

}

//------------------------------------------------------------------------------
void OTableRowView::cut()
{
    CopyRows();
    DeleteRows();
}

//------------------------------------------------------------------------------
void OTableRowView::copy()
{
    CopyRows();
}

//------------------------------------------------------------------------------
void OTableRowView::paste()
{
    OSL_ENSURE(0,"OTableRowView::Paste : (pseudo-) abstract method called !");
}

//------------------------------------------------------------------------------
void OTableRowView::Paste( long nRow )
{
    InsertRows( nRow );
}

//------------------------------------------------------------------------------
EditBrowseBox::RowStatus OTableRowView::GetRowStatus(long nRow) const
{
    if (nRow >= 0 && m_nDataPos == nRow)
        return CURRENT;
    else
        return CLEAN;
}



