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
#include <vcl/builderfactory.hxx>
#include "actctrl.hxx"

void NumEditAction::Action()
{
    aActionLink.Call( *this );
}

bool NumEditAction::Notify( NotifyEvent& rNEvt )
{
    bool nHandled = false;

    if ( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
    {
        const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
        const vcl::KeyCode aKeyCode = pKEvt->GetKeyCode();
        const sal_uInt16 nModifier = aKeyCode.GetModifier();
        if( aKeyCode.GetCode() == KEY_RETURN &&
                !nModifier)
        {
            Action();
            nHandled = true;
        }

    }
    if(!nHandled)
        NumericField::Notify( rNEvt );
    return nHandled;
}

void ReturnActionEdit::KeyInput( const KeyEvent& rEvt)
{
    const vcl::KeyCode aKeyCode = rEvt.GetKeyCode();
    const sal_uInt16 nModifier = aKeyCode.GetModifier();
    if( aKeyCode.GetCode() == KEY_RETURN &&
            !nModifier)
    {
        if(aReturnActionLink.IsSet())
            aReturnActionLink.Call(*this);
    }
    else
        Edit::KeyInput(rEvt);
}

VCL_BUILDER_DECL_FACTORY(ReturnActionEdit)
{
    VclBuilder::ensureDefaultWidthChars(rMap);
    rRet = VclPtr<ReturnActionEdit>::Create(pParent, WB_LEFT|WB_VCENTER|WB_BORDER|WB_3DLOOK);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
