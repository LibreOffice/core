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

#ifndef SC_SHTABDLG_HXX
#define SC_SHTABDLG_HXX

#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/lstbox.hxx>

//------------------------------------------------------------------------

class ScShowTabDlg : public ModalDialog
{
private:
    FixedText                       aFtLbTitle;
    MultiListBox                    aLb;
    OKButton                        aBtnOk;
    CancelButton                    aBtnCancel;
    HelpButton                      aBtnHelp;

    DECL_LINK( DblClkHdl, void * );

public:
            ScShowTabDlg( Window* pParent );
            ~ScShowTabDlg();

    /** Sets dialog title, fixed text for listbox and help IDs. */
    void    SetDescription(
                const String& rTitle, const String& rFixedText,
                const OString& nDlgHelpId, const OString& nLbHelpId );

    /** Inserts a string into the ListBox. */
    void    Insert( const String& rString, sal_Bool bSelected );

    sal_uInt16  GetSelectEntryCount() const;
    String  GetSelectEntry(sal_uInt16 nPos) const;
    sal_uInt16  GetSelectEntryPos(sal_uInt16 nPos) const;
};


#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
