/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

#include "TableDesignControl.hxx"
#include "dbu_tbl.hrc"
#include "TableDesignView.hxx"
#include "TableController.hxx"
#include "browserids.hxx"
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include "dbaccess_helpid.hrc"

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
    ,m_bCurrentModified(sal_False)
    ,m_bUpdatable(sal_False)
    ,m_bClipboardFilled(sal_False)
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

    Font aFont( GetDataWindow().GetFont() );
    aFont.SetWeight( WEIGHT_NORMAL );
    GetDataWindow().SetFont( aFont );

    // Font fuer die Ueberschriften auf Light setzen
    aFont = GetFont();
    aFont.SetWeight( WEIGHT_LIGHT );
    SetFont(aFont);

    // HandleColumn, fuer maximal fuenf Ziffern einrichten
    InsertHandleColumn(static_cast<sal_uInt16>(GetTextWidth('0') * 4)/*, sal_True */);

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
            aUrl.Complete =::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(".uno:DSBEditDoc"));
            GetView()->getController().dispatch( aUrl,Sequence< PropertyValue >() );
        }
    }
    EditBrowseBox::KeyInput(rEvt);
}

//------------------------------------------------------------------------
void OTableRowView::SetUpdatable( sal_Bool bUpdate )
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

            sal_uInt16 nColId = GetColumnAtXPosPixel(rEvt.GetMousePosPixel().X());
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
    OSL_FAIL("OTableRowView::Paste : (pseudo-) abstract method called !");
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



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
