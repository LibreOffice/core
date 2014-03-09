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

#include <sfx2/bindings.hxx>
#include "view.hxx"
#include "edtwin.hxx"
#include "wrtsh.hxx"
#include "cmdid.h"
#include "drawbase.hxx"
#include "dselect.hxx"

DrawSelection::DrawSelection(SwWrtShell* pWrtShell, SwEditWin* pEditWin, SwView* pSwView) :
                SwDrawBase(pWrtShell, pEditWin, pSwView)
{
    m_bCreateObj = false;
}

// Process keyboard events

// If a KeyEvent is processed then the return value is sal_True, otherwise
// Sal_False.

sal_Bool DrawSelection::KeyInput(const KeyEvent& rKEvt)
{
    sal_Bool bReturn = sal_False;

    switch (rKEvt.GetKeyCode().GetCode())
    {
        case KEY_ESCAPE:
        {
            if (m_pWin->IsDrawAction())
            {
                m_pSh->BreakMark();
                m_pWin->ReleaseMouse();
            }
            bReturn = sal_True;
        }
        break;
    }

    if (!bReturn)
        bReturn = SwDrawBase::KeyInput(rKEvt);

    return (bReturn);
}

void DrawSelection::Activate(const sal_uInt16 nSlotId)
{
    m_pWin->SetSdrDrawMode(OBJ_NONE);
    m_pWin->SetObjectSelect( sal_True );
    SwDrawBase::Activate(nSlotId);

    m_pSh->GetView().GetViewFrame()->GetBindings().Invalidate(SID_INSERT_DRAW);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
