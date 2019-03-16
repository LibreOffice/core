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

#include <vcl/builderfactory.hxx>
#include <actctrl.hxx>
#include <vcl/event.hxx>
#include <vcl/toolbox.hxx>

bool NumEditAction::EventNotify( NotifyEvent& rNEvt )
{
    bool bHandled = false;

    if ( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
    {
        const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
        const vcl::KeyCode aKeyCode = pKEvt->GetKeyCode();
        const sal_uInt16 aCode = aKeyCode.GetCode();
        const sal_uInt16 nModifier = aKeyCode.GetModifier();
        if( aCode == KEY_RETURN &&
                !nModifier)
        {
            aActionLink.Call( *this );
            bHandled = true;
        }
        else
        {
            vcl::Window* pParent = GetParent();
            if ( pParent != nullptr && aCode == KEY_TAB &&
                 pParent->GetType() == WindowType::TOOLBOX )
            {
                static_cast<ToolBox*>(pParent)->ChangeHighlightUpDn( aKeyCode.IsShift() );
                bHandled = true;
            }
        }
    }
    if(!bHandled)
        bHandled = NumericField::EventNotify(rNEvt);
    return bHandled;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
