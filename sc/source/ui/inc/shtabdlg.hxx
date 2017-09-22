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

#ifndef INCLUDED_SC_SOURCE_UI_INC_SHTABDLG_HXX
#define INCLUDED_SC_SOURCE_UI_INC_SHTABDLG_HXX

#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/layout.hxx>
#include <vcl/lstbox.hxx>

class ScShowTabDlg : public ModalDialog
{
private:
    VclPtr<VclFrame> m_pFrame;
    VclPtr<ListBox> m_pLb;

    DECL_LINK( DblClkHdl, ListBox&, void );

public:
    ScShowTabDlg( vcl::Window* pParent );
    virtual ~ScShowTabDlg() override;
    virtual void dispose() override;

    /** Sets dialog title, fixed text for listbox and help IDs. */
    void    SetDescription(
                const OUString& rTitle, const OUString& rFixedText,
                const OString& nDlgHelpId, const OString& nLbHelpId );

    /** Inserts a string into the ListBox. */
    void    Insert( const OUString& rString, bool bSelected );

    sal_Int32 GetSelectedEntryCount() const;
    OUString    GetSelectedEntry(sal_Int32 nPos) const;
    sal_Int32 GetSelectedEntryPos(sal_Int32 nPos) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
