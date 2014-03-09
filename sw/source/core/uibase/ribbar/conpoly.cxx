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

#include <svx/svdmark.hxx>
#include <svx/svdview.hxx>
#include <svx/svdopath.hxx>

#include "view.hxx"
#include "edtwin.hxx"
#include "wrtsh.hxx"
#include "drawbase.hxx"
#include "conpoly.hxx"
#include <basegfx/polygon/b2dpolygon.hxx>

ConstPolygon::ConstPolygon(SwWrtShell* pWrtShell, SwEditWin* pEditWin, SwView* pSwView) :
                SwDrawBase(pWrtShell, pEditWin, pSwView)
{
}

sal_Bool ConstPolygon::MouseButtonDown(const MouseEvent& rMEvt)
{
    sal_Bool bReturn;

    if ((bReturn = SwDrawBase::MouseButtonDown(rMEvt)) == sal_True)
        aLastPos = rMEvt.GetPosPixel();

    return (bReturn);
}

sal_Bool ConstPolygon::MouseMove(const MouseEvent& rMEvt)
{
    sal_Bool bReturn = SwDrawBase::MouseMove(rMEvt);

    return bReturn;
}

sal_Bool ConstPolygon::MouseButtonUp(const MouseEvent& rMEvt)
{
    sal_Bool bReturn = sal_False;

    if (m_pSh->IsDrawCreate())
    {
        if (rMEvt.IsLeft() && rMEvt.GetClicks() == 1 &&
                                        m_pWin->GetSdrDrawMode() != OBJ_FREELINE)
        {
            if (!m_pSh->EndCreate(SDRCREATE_NEXTPOINT))
            {
                m_pSh->BreakCreate();
                EnterSelectMode(rMEvt);
                return sal_True;
            }
        }
        else
        {
            bReturn = SwDrawBase::MouseButtonUp(rMEvt);

            // #i85045# removed double mechanism to check for AutoClose polygon
            // after construction; the method here did not check for already closed and
            // also worked only for a single polygon. Removing.
        }
    }
    else
        bReturn = SwDrawBase::MouseButtonUp(rMEvt);

    return (bReturn);
}

void ConstPolygon::Activate(const sal_uInt16 nSlotId)
{
    switch (nSlotId)
    {
        case SID_DRAW_POLYGON_NOFILL:
            m_pWin->SetSdrDrawMode(OBJ_PLIN);
            break;

        case SID_DRAW_BEZIER_NOFILL:
            m_pWin->SetSdrDrawMode(OBJ_PATHLINE);
            break;

        case SID_DRAW_FREELINE_NOFILL:
            m_pWin->SetSdrDrawMode(OBJ_FREELINE);
            break;

        default:
            break;
    }

    SwDrawBase::Activate(nSlotId);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
