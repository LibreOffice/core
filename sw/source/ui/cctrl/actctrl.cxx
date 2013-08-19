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


#include <comphelper/string.hxx>
#include <vcl/builder.hxx>
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
        const sal_uInt16 nModifier = aKeyCode.GetModifier();
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

NoSpaceEdit::NoSpaceEdit(Window* pParent)
    : Edit(pParent, WB_BORDER|WB_TABSTOP)
    , sForbiddenChars(OUString(" "))
{
}

NoSpaceEdit::~NoSpaceEdit()
{
}

void NoSpaceEdit::KeyInput(const KeyEvent& rEvt)
{
    bool bCallParent = true;
    if(rEvt.GetCharCode())
    {
        OUString sKey(rEvt.GetCharCode());
        if( STRING_NOTFOUND != sForbiddenChars.Search(sKey))
            bCallParent = false;
    }
    if(bCallParent)
        Edit::KeyInput(rEvt);
}

void NoSpaceEdit::Modify()
{
    Selection aSel = GetSelection();
    OUString sTemp = GetText();
    for(sal_uInt16 i = 0; i < sForbiddenChars.Len(); i++)
    {
        sTemp = comphelper::string::remove(sTemp, sForbiddenChars.GetChar(i));
    }
    sal_Int32 nDiff = GetText().getLength() - sTemp.getLength();
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

ReturnActionEdit::ReturnActionEdit(Window* pParent, const ResId& rResId)
    : Edit(pParent, rResId)
{
}

ReturnActionEdit::~ReturnActionEdit()
{
}

void ReturnActionEdit::KeyInput( const KeyEvent& rEvt)
{
    const KeyCode aKeyCode = rEvt.GetKeyCode();
    const sal_uInt16 nModifier = aKeyCode.GetModifier();
    if( aKeyCode.GetCode() == KEY_RETURN &&
            !nModifier)
    {
        if(aReturnActionLink.IsSet())
            aReturnActionLink.Call(this);
    }
    else
        Edit::KeyInput(rEvt);
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeTableNameEdit(Window *pParent, VclBuilder::stringmap &rMap)
{
    VclBuilder::ensureDefaultWidthChars(rMap);
    return new TableNameEdit(pParent);
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeNoSpaceEdit(Window *pParent, VclBuilder::stringmap &rMap)
{
    VclBuilder::ensureDefaultWidthChars(rMap);
    return new NoSpaceEdit(pParent);
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeReturnActionEdit(Window *pParent, VclBuilder::stringmap &rMap)
{
    VclBuilder::ensureDefaultWidthChars(rMap);
    return new ReturnActionEdit(pParent, WB_LEFT|WB_VCENTER|WB_BORDER|WB_3DLOOK);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
