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

#include <TableDesignControl.hxx>
#include <TableDesignView.hxx>
#include <TableController.hxx>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <vcl/commandevent.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weldutils.hxx>
#include <helpids.h>

using namespace ::dbaui;
using namespace ::svt;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
// Defines
#define HANDLE_ID 0

OTableRowView::OTableRowView(vcl::Window* pParent)
    : EditBrowseBox(pParent, EditBrowseBoxFlags::NONE, WB_TABSTOP|WB_HIDE|WB_3DLOOK,
                    BrowserMode::COLUMNSELECTION | BrowserMode::MULTISELECTION |
                    BrowserMode::AUTOSIZE_LASTCOL | BrowserMode::KEEPHIGHLIGHT |
                    BrowserMode::HLINES | BrowserMode::VLINES)
    , m_nDataPos(-1)
    , m_nCurrentPos(-1)
    , m_nCurUndoActId(0)
{
    SetHelpId(HID_TABDESIGN_BACKGROUND);
    SetSizePixel(LogicToPixel(Size(40, 12), MapMode(MapUnit::MapAppFont)));
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

    BrowserMode const nMode = BrowserMode::COLUMNSELECTION | BrowserMode::MULTISELECTION | BrowserMode::KEEPHIGHLIGHT |
                              BrowserMode::HLINES | BrowserMode::VLINES | BrowserMode::AUTOSIZE_LASTCOL;

    SetMode(nMode);
}

void OTableRowView::KeyInput( const KeyEvent& rEvt )
{
    if (IsDeleteAllowed())
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

            sal_uInt16 nColId = GetColumnId(GetColumnAtXPosPixel(rEvt.GetMousePosPixel().X()));
            sal_Int32  nRow = GetRowAtYPosPixel(rEvt.GetMousePosPixel().Y());

            if ( nColId == HANDLE_ID )
            {
                ::tools::Rectangle aRect(rEvt.GetMousePosPixel(), Size(1, 1));
                weld::Window* pPopupParent = weld::GetPopupParent(*this, aRect);
                std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(pPopupParent, u"dbaccess/ui/tabledesignrowmenu.ui"_ustr));
                std::unique_ptr<weld::Menu> xContextMenu(xBuilder->weld_menu(u"menu"_ustr));
                sal_Int32 nSelectRowCount = GetSelectRowCount();
                xContextMenu->set_sensitive(u"cut"_ustr, nSelectRowCount != 0);
                xContextMenu->set_sensitive(u"copy"_ustr, nSelectRowCount  != 0);
                OUString sIdent = xContextMenu->popup_at_rect(pPopupParent, aRect);
                if (sIdent == "cut")
                    cut();
                else if (sIdent == "copy")
                    copy();
                else if (sIdent == "insert")
                {
                    InsertNewRows( nRow );
                    SetNoSelection();
                    GoToRow( nRow );
                    SeekRow( nRow );
                }

                return;
            }

            [[fallthrough]];
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

void OTableRowView::Paste( sal_Int32 nRow )
{
    InsertRows( nRow );
}

EditBrowseBox::RowStatus OTableRowView::GetRowStatus(sal_Int32 nRow) const
{
    if (nRow >= 0 && m_nDataPos == nRow)
        return CURRENT;
    else
        return CLEAN;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
