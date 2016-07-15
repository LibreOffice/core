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

#include "radiobtnbox.hxx"
#include <dialmgr.hxx>

#include "svtools/svlbitm.hxx"

namespace svx {

// class SvxRadioButtonListBox ----------------------------------------------------

SvxRadioButtonListBox::SvxRadioButtonListBox(SvSimpleTableContainer& rParent, WinBits nBits)
    : SvSimpleTable(rParent, nBits)

{
    EnableCheckButton( new SvLBoxButtonData( this, true ) );
}

void SvxRadioButtonListBox::MouseButtonUp( const MouseEvent& _rMEvt )
{
    m_aCurMousePoint = _rMEvt.GetPosPixel();
    SvSimpleTable::MouseButtonUp( _rMEvt );
}

void SvxRadioButtonListBox::KeyInput( const KeyEvent& rKEvt )
{
    if ( !rKEvt.GetKeyCode().GetModifier() && KEY_SPACE == rKEvt.GetKeyCode().GetCode() )
    {
        SvTreeListEntry* pEntry = FirstSelected();
        if ( GetCheckButtonState( pEntry ) == SvButtonState::Unchecked )
        {
            SetCheckButtonState( pEntry, SvButtonState::Checked );
            GetCheckButtonHdl().Call( nullptr );
            return ;
        }
    }

    SvSimpleTable::KeyInput( rKEvt );
}

void SvxRadioButtonListBox::HandleEntryChecked( SvTreeListEntry* _pEntry )
{
    Select( _pEntry );
    SvButtonState eState = GetCheckButtonState( _pEntry );

    if ( SvButtonState::Checked == eState )
    {
        // we have radio button behavior -> so uncheck the other entries
        SvTreeListEntry* pEntry = First();
        while ( pEntry )
        {
            if ( pEntry != _pEntry )
                SetCheckButtonState( pEntry, SvButtonState::Unchecked );
            pEntry = Next( pEntry );
        }
    }
    else
        SetCheckButtonState( _pEntry, SvButtonState::Checked );
}


long SvxRadioButtonListBox::GetControlColumnWidth() const
{
    return pCheckButtonData->Width();
}

} // end of namespace ::svx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
