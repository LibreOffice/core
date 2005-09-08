/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: shtabdlg.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:50:01 $
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

#ifndef SC_SHTABDLG_HXX
#define SC_SHTABDLG_HXX

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef SVTOOLS_TOOLTIPLBOX_HXX
#include <svtools/tooltiplbox.hxx>
#endif
#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif

//------------------------------------------------------------------------

class ScShowTabDlg : public ModalDialog
{
private:
    ::svtools::ToolTipMultiListBox  aLb;
    OKButton                        aBtnOk;
    CancelButton                    aBtnCancel;
    HelpButton                      aBtnHelp;
    FixedText                       aFtLbTitle;

    DECL_LINK( DblClkHdl, void * );

public:
            ScShowTabDlg( Window* pParent );
            ~ScShowTabDlg();

    /** Sets dialog title, fixed text for listbox and help IDs. */
    void    SetDescription(
                const String& rTitle, const String& rFixedText,
                ULONG nDlgHelpId, ULONG nLbHelpId );

    /** Inserts a string into the ListBox. */
    void    Insert( const String& rString, BOOL bSelected );

    USHORT  GetSelectEntryCount() const;
    String  GetSelectEntry(USHORT nPos) const;
    USHORT  GetSelectEntryPos(USHORT nPos) const;
};


#endif


