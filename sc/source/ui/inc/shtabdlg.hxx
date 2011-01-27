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

#ifndef SC_SHTABDLG_HXX
#define SC_SHTABDLG_HXX

#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <svtools/tooltiplbox.hxx>
#include <vcl/dialog.hxx>

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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
