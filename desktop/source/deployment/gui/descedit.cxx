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


#include <vcl/scrbar.hxx>
#include <vcl/txtattr.hxx>
#include <vcl/xtextedt.hxx>

#include "descedit.hxx"

#include "dp_gui.hrc"

using dp_gui::DescriptionEdit;

// DescriptionEdit -------------------------------------------------------

DescriptionEdit::DescriptionEdit( Window* pParent, const ResId& rResId ) :

    ExtMultiLineEdit( pParent, rResId ),

    m_bIsVerticalScrollBarHidden( true )

{
    Init();
}

// -----------------------------------------------------------------------

void DescriptionEdit::Init()
{
    Clear();
    // no tabstop
    SetStyle( ( GetStyle() & ~WB_TABSTOP ) | WB_NOTABSTOP );
    // read-only
    SetReadOnly();
    // no cursor
    EnableCursor( sal_False );
}

// -----------------------------------------------------------------------

void DescriptionEdit::Clear()
{
    SetText( OUString() );

    m_bIsVerticalScrollBarHidden = true;
    ScrollBar*  pVScrBar = GetVScrollBar();
    if ( pVScrBar )
        pVScrBar->Hide();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
