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

#include <svx/svdobj.hxx>
#include <svx/svxids.hrc>
#include <vcl/event.hxx>

#include <view.hxx>
#include <edtwin.hxx>
#include <wrtsh.hxx>
#include <drawbase.hxx>
#include <conarc.hxx>

ConstArc::ConstArc(SwWrtShell* pWrtShell, SwEditWin* pEditWin, SwView* pSwView)
    : SwDrawBase(pWrtShell, pEditWin, pSwView)
    , m_nButtonUpCount(0)
{
}

bool ConstArc::MouseButtonDown(const MouseEvent& rMEvt)
{
    bool bReturn = SwDrawBase::MouseButtonDown(rMEvt);
    if (bReturn && !m_nButtonUpCount)
        m_aStartPoint = m_pWin->PixelToLogic(rMEvt.GetPosPixel());
    return bReturn;
}

bool ConstArc::MouseButtonUp(const MouseEvent& rMEvt)
{
    bool bReturn = false;

    if ((m_pSh->IsDrawCreate() || m_pWin->IsDrawAction()) && rMEvt.IsLeft())
    {
        Point aPnt(m_pWin->PixelToLogic(rMEvt.GetPosPixel()));
        if (!m_nButtonUpCount && aPnt == m_aStartPoint)
        {
            SwDrawBase::MouseButtonUp(rMEvt);
            bReturn = true;
        }
        else
        {
            m_nButtonUpCount++;

            if (m_nButtonUpCount == 3) // Generating of circular arc finished
            {
                SwDrawBase::MouseButtonUp(rMEvt);
                m_nButtonUpCount = 0;
                bReturn = true;
            }
            else
                m_pSh->EndCreate(SdrCreateCmd::NextPoint);
        }
    }

    return bReturn;
}

void ConstArc::Activate(const sal_uInt16 nSlotId)
{
    switch (nSlotId)
    {
        case SID_DRAW_ARC:
            m_pWin->SetSdrDrawMode(OBJ_CARC);
            break;
        case SID_DRAW_PIE:
            m_pWin->SetSdrDrawMode(OBJ_SECT);
            break;
        case SID_DRAW_CIRCLECUT:
            m_pWin->SetSdrDrawMode(OBJ_CCUT);
            break;
        default:
            m_pWin->SetSdrDrawMode(OBJ_NONE);
            break;
    }

    SwDrawBase::Activate(nSlotId);
}

void ConstArc::Deactivate()
{
    m_nButtonUpCount = 0;

    SwDrawBase::Deactivate();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
