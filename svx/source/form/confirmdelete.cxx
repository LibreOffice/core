/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
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
#include "precompiled_svx.hxx"
#include "confirmdelete.hxx"
#include <svx/dialmgr.hxx>
#ifndef _SVX_FMHELP_HRC
#include "fmhelp.hrc"
#endif
#ifndef _SVX_FMRESIDS_HRC
#include "fmresids.hrc"
#endif
#include <unotools/configmgr.hxx>
#include <vcl/msgbox.hxx>

//........................................................................
namespace svxform
{
//........................................................................

#define BORDER_HEIGHT   6 // default distance control-dialog
#define BORDER_WIDTH    6 // default distance control-dialog

    using namespace ::com::sun::star::uno;

    //====================================================================
    //= class ConfirmDeleteDialog
    //====================================================================
    //------------------------------------------------------------------------------
    ConfirmDeleteDialog::ConfirmDeleteDialog(Window* pParent, const String& _rTitle)
              :ButtonDialog(pParent, WB_HORZ | WB_STDDIALOG)
              ,m_aInfoImage (this)
              ,m_aTitle     (this, WB_WORDBREAK | WB_LEFT)
              ,m_aMessage   (this, WB_WORDBREAK | WB_LEFT)
    {
        String sMessage(SVX_RES(RID_STR_DELETECONFIRM));

        // Changed as per BugID 79541 Branding/Configuration
        Any aProductName = ::utl::ConfigManager::GetDirectConfigProperty(::utl::ConfigManager::PRODUCTNAME);
        ::rtl::OUString sProductName;
        aProductName >>= sProductName;

        String aTitle = sProductName;
        aProductName = ::utl::ConfigManager::GetDirectConfigProperty(::utl::ConfigManager::PRODUCTVERSION);
        aProductName >>= sProductName;
        aTitle.AppendAscii(" ");
        aTitle += String(sProductName);
        SetText(aTitle);

        SetHelpId(HID_DLG_DBMSG);
        SetSizePixel(LogicToPixel(Size(220, 30),MAP_APPFONT));

        m_aInfoImage.SetPosSizePixel(LogicToPixel(Point(6, 6),MAP_APPFONT),
                                   LogicToPixel(Size(20, 20),MAP_APPFONT));
        m_aInfoImage.Show();

        m_aTitle.SetPosSizePixel(LogicToPixel(Point(45, 6),MAP_APPFONT),
                                 LogicToPixel(Size(169, 20),MAP_APPFONT));

        Font  aFont = m_aTitle.GetFont();
        aFont.SetWeight(WEIGHT_SEMIBOLD);
        m_aTitle.SetFont(aFont);
        m_aTitle.Show();

        m_aMessage.SetPosSizePixel(LogicToPixel(Point(45, 29),MAP_APPFONT),
                                   LogicToPixel(Size(169, 1),MAP_APPFONT));
        m_aMessage.Show();

        // Image festlegen
        m_aInfoImage.SetImage(WarningBox::GetStandardImage());

        // Title setzen
        m_aTitle.SetText(_rTitle);

        // Ermitteln der Hoehe des Textfeldes und des Dialogs
        Size aBorderSize = LogicToPixel(Size(BORDER_WIDTH, BORDER_HEIGHT),MAP_APPFONT);
        Rectangle aDlgRect(GetPosPixel(),GetSizePixel());
        Rectangle aMessageRect(m_aMessage.GetPosPixel(),m_aMessage.GetSizePixel());
        Rectangle aTextRect =
            GetTextRect(aMessageRect, sMessage, TEXT_DRAW_WORDBREAK | TEXT_DRAW_MULTILINE | TEXT_DRAW_LEFT);

        long nHText = aTextRect.Bottom() > aMessageRect.Bottom() ? aTextRect.Bottom() - aMessageRect.Bottom() : 0;

        aDlgRect.Bottom() += nHText + 2 * aBorderSize.Height();
        aMessageRect.Bottom() += nHText;

        // Dialog anpassen
        SetSizePixel(aDlgRect.GetSize());
        SetPageSizePixel(aDlgRect.GetSize());

        // Message Text anpassen und setzen
        m_aMessage.SetSizePixel(aMessageRect.GetSize());
        m_aMessage.SetText(sMessage);

        // Buttons anlegen
        AddButton(BUTTON_YES, BUTTONID_YES, 0);
        AddButton(BUTTON_NO, BUTTONID_NO, BUTTONDIALOG_DEFBUTTON | BUTTONDIALOG_FOCUSBUTTON);
    }

    //------------------------------------------------------------------------------
    ConfirmDeleteDialog::~ConfirmDeleteDialog()
    {
    }

//........................................................................
}   // namespace svxform
//........................................................................



