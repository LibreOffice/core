/*************************************************************************
 *
 *  $RCSfile: confirmdelete.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: fs $ $Date: 2001-07-25 13:37:28 $
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
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef SVX_FORM_CONFIRMDELETE_HXX
#include "confirmdelete.hxx"
#endif

#ifndef _SVX_DIALMGR_HXX
#include <svx/dialmgr.hxx>
#endif
#ifndef _SVX_FMHELP_HRC
#include "fmhelp.hrc"
#endif
#ifndef _SVX_FMRESIDS_HRC
#include "fmresids.hrc"
#endif
#ifndef _UTL_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif

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
        ::utl::ConfigManager* pMgr = ::utl::ConfigManager::GetConfigManager();
        Any aProductName = pMgr->GetDirectConfigProperty(::utl::ConfigManager::PRODUCTNAME);
        ::rtl::OUString sProductName;
        aProductName >>= sProductName;

        String aTitle = sProductName;
        aTitle.AppendAscii(" Base");
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
        sal_uInt16 nDefId = BUTTONID_NO;

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


/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *
 *  Revision 1.0 25.07.01 09:44:47  fs
 ************************************************************************/

