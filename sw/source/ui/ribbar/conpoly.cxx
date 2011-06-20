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


#include <svx/svdmark.hxx>
#include <svx/svdview.hxx>
#include <svx/svdopath.hxx>

#include "view.hxx"
#include "edtwin.hxx"
#include "wrtsh.hxx"
#include "drawbase.hxx"
#include "conpoly.hxx"
#include <basegfx/polygon/b2dpolygon.hxx>

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/



ConstPolygon::ConstPolygon(SwWrtShell* pWrtShell, SwEditWin* pEditWin, SwView* pSwView) :
                SwDrawBase(pWrtShell, pEditWin, pSwView)
{
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/



sal_Bool ConstPolygon::MouseButtonDown(const MouseEvent& rMEvt)
{
    sal_Bool bReturn;

    if ((bReturn = SwDrawBase::MouseButtonDown(rMEvt)) == sal_True)
        aLastPos = rMEvt.GetPosPixel();

    return (bReturn);
}

/*************************************************************************
|*
|* MouseMove-event
|*
\************************************************************************/



sal_Bool ConstPolygon::MouseMove(const MouseEvent& rMEvt)
{
    sal_Bool bReturn = sal_False;

    bReturn = SwDrawBase::MouseMove(rMEvt);

    return bReturn;
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/



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
            Point aPnt(m_pWin->PixelToLogic(rMEvt.GetPosPixel()));
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

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/



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
