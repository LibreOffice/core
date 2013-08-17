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

OTableRowView::~OTableRowView()
{

    DBG_DTOR(OTableRowView,NULL);
}

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
    InsertHandleColumn(static_cast<sal_uInt16>(GetTextWidth(OUString('0')) * 4)/*, sal_True */);

    BrowserMode nMode = BROWSER_COLUMNSELECTION | BROWSER_MULTISELECTION | BROWSER_KEEPSELECTION |
                        BROWSER_HLINESFULL | BROWSER_VLINESFULL | BROWSER_AUTOSIZE_LASTCOL;
    if (IsUpdatable())
        nMode |= BROWSER_HIDECURSOR;

    SetMode(nMode);
}

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
            aUrl.Complete =OUString(".uno:DSBEditDoc");
            GetView()->getController().dispatch( aUrl,Sequence< PropertyValue >() );
        }
    }
    EditBrowseBox::KeyInput(rEvt);
}

void OTableRowView::SetUpdatable( sal_Bool bUpdate )
{
    m_bUpdatable = bUpdate;

}

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

void OTableRowView::cut()
{
    CopyRows();
    DeleteRows();
}

void OTableRowView::copy()
{
    CopyRows();
}

void OTableRowView::paste()
{
    OSL_FAIL("OTableRowView::Paste : (pseudo-) abstract method called !");
}

void OTableRowView::Paste( long nRow )
{
    InsertRows( nRow );
}

EditBrowseBox::RowStatus OTableRowView::GetRowStatus(long nRow) const
{
    if (nRow >= 0 && m_nDataPos == nRow)
        return CURRENT;
    else
        return CLEAN;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
