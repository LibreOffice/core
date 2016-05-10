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
// Defines
#define HANDLE_ID 0

OTableRowView::OTableRowView(vcl::Window* pParent)
    :EditBrowseBox(pParent, ModuleRes(RID_DB_TAB_EDITOR),EditBrowseBoxFlags::NONE,
                    BrowserMode::COLUMNSELECTION | BrowserMode::MULTISELECTION | BrowserMode::AUTOSIZE_LASTCOL |
                    BrowserMode::KEEPHIGHLIGHT | BrowserMode::HLINES | BrowserMode::VLINES)
    ,m_nDataPos(-1)
    ,m_nCurrentPos(-1)
    ,m_nCurUndoActId(0)
    ,m_bClipboardFilled(false)
{

}

void OTableRowView::Init()
{
    EditBrowseBox::Init();

    vcl::Font aFont( GetDataWindow().GetFont() );
    aFont.SetWeight( WEIGHT_NORMAL );
    GetDataWindow().SetFont( aFont );

    // set font for the headings to light
    aFont = GetFont();
    aFont.SetWeight( WEIGHT_LIGHT );
    SetFont(aFont);

    // set up HandleColumn for at maximum 5 digits
    InsertHandleColumn(static_cast<sal_uInt16>(GetTextWidth(OUString('0')) * 4)/*, sal_True */);

    BrowserMode nMode = BrowserMode::COLUMNSELECTION | BrowserMode::MULTISELECTION | BrowserMode::KEEPHIGHLIGHT |
                        BrowserMode::HLINES | BrowserMode::VLINES | BrowserMode::AUTOSIZE_LASTCOL;

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
            css::util::URL aUrl;
            aUrl.Complete = ".uno:DSBEditDoc";
            GetView()->getController().dispatch( aUrl,Sequence< PropertyValue >() );
        }
    }
    EditBrowseBox::KeyInput(rEvt);
}

void OTableRowView::Command(const CommandEvent& rEvt)
{

    switch (rEvt.GetCommand())
    {
        case CommandEventId::ContextMenu:
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
                aContextMenu.EnableItem( SID_DELETE, false );
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

            SAL_FALLTHROUGH;
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
