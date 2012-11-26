/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

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
        if (rMEvt.IsLeft()
            && rMEvt.GetClicks() == 1
            && !(OBJ_POLY == m_pWin->getSdrObjectCreationInfo().getIdent() && m_pWin->getSdrObjectCreationInfo().getFreehandMode()))
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
    SdrObjectCreationInfo aSdrObjectCreationInfo;
    bool bSet(false);

    switch (nSlotId)
    {
        case SID_DRAW_POLYGON_NOFILL:
            aSdrObjectCreationInfo.setIdent(OBJ_POLY);
            aSdrObjectCreationInfo.setSdrPathObjType(PathType_OpenPolygon);
            break;

        case SID_DRAW_BEZIER_NOFILL:
            aSdrObjectCreationInfo.setIdent(OBJ_POLY);
            aSdrObjectCreationInfo.setSdrPathObjType(PathType_OpenBezier);
            break;

        case SID_DRAW_FREELINE_NOFILL:
            aSdrObjectCreationInfo.setIdent(OBJ_POLY);
            aSdrObjectCreationInfo.setSdrPathObjType(PathType_OpenBezier);
            aSdrObjectCreationInfo.setFreehandMode(true);
            break;

        default:
            break;
    }

    if(bSet)
    {
        m_pWin->setSdrObjectCreationInfo(aSdrObjectCreationInfo);
    }

    SwDrawBase::Activate(nSlotId);
}



