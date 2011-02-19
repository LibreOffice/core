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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include "actctrl.hxx"

void NumEditAction::Action()
{
    aActionLink.Call( this );
}


long NumEditAction::Notify( NotifyEvent& rNEvt )
{
    long nHandled = 0;

    if ( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
        const KeyCode aKeyCode = pKEvt->GetKeyCode();
        const USHORT nModifier = aKeyCode.GetModifier();
        if( aKeyCode.GetCode() == KEY_RETURN &&
                !nModifier)
        {
            Action();
            nHandled = 1;
        }

    }
    if(!nHandled)
        NumericField::Notify( rNEvt );
    return nHandled;
}

NoSpaceEdit::NoSpaceEdit( Window* pParent, const ResId& rResId)
    : Edit(pParent, rResId),
    sForbiddenChars(String::CreateFromAscii(" "))
{
}

NoSpaceEdit::~NoSpaceEdit()
{
}

void NoSpaceEdit::KeyInput(const KeyEvent& rEvt)
{
    BOOL bCallParent = TRUE;
    if(rEvt.GetCharCode())
    {
        String sKey = rEvt.GetCharCode();
        if( STRING_NOTFOUND != sForbiddenChars.Search(sKey))
            bCallParent = FALSE;
    }
    if(bCallParent)
        Edit::KeyInput(rEvt);
}

void NoSpaceEdit::Modify()
{
    Selection aSel = GetSelection();
    String sTemp = GetText();
    for(USHORT i = 0; i < sForbiddenChars.Len(); i++)
    {
        sTemp.EraseAllChars( sForbiddenChars.GetChar(i) );
    }
    USHORT nDiff = GetText().Len() - sTemp.Len();
    if(nDiff)
    {
        aSel.setMin(aSel.getMin() - nDiff);
        aSel.setMax(aSel.getMin());
        SetText(sTemp);
        SetSelection(aSel);
    }
    if(GetModifyHdl().IsSet())
        GetModifyHdl().Call(this);
}

ReturnActionEdit::~ReturnActionEdit()
{
}

void ReturnActionEdit::KeyInput( const KeyEvent& rEvt)
{
    const KeyCode aKeyCode = rEvt.GetKeyCode();
    const USHORT nModifier = aKeyCode.GetModifier();
    if( aKeyCode.GetCode() == KEY_RETURN &&
            !nModifier)
    {
        if(aReturnActionLink.IsSet())
            aReturnActionLink.Call(this);
    }
    else
        Edit::KeyInput(rEvt);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
