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

#include "svtools/svmedit2.hxx"
#include "svl/lstner.hxx"
#include "vcl/xtextedt.hxx"
#include "vcl/scrbar.hxx"

#include "dp_gui_autoscrolledit.hxx"


namespace dp_gui {


AutoScrollEdit::AutoScrollEdit( Window* pParent, const ResId& rResId )
    : ExtMultiLineEdit( pParent, rResId )
{
    ScrollBar*  pScroll = GetVScrollBar();
    if (pScroll)
        pScroll->Hide();
    StartListening( *GetTextEngine() );
}

AutoScrollEdit::~AutoScrollEdit()
{
    EndListeningAll();
}

void AutoScrollEdit::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( rHint.IsA( TYPE(TextHint) ) )
    {
        sal_uLong   nId = ((const TextHint&)rHint).GetId();
        if ( nId == TEXT_HINT_VIEWSCROLLED )
        {
            ScrollBar*  pScroll = GetVScrollBar();
            if ( pScroll )
                pScroll->Show();
        }
    }
}


} // namespace dp_gui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
