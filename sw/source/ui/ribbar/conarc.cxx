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


#include <svx/svdobj.hxx>


#include "view.hxx"
#include "edtwin.hxx"
#include "wrtsh.hxx"
#include "drawbase.hxx"
#include "conarc.hxx"



/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/



ConstArc::ConstArc(SwWrtShell* pWrtShell, SwEditWin* pEditWin, SwView* pSwView)
    : SwDrawBase(pWrtShell, pEditWin, pSwView), nAnzButUp(0)
{
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/



sal_Bool ConstArc::MouseButtonDown( const MouseEvent& rMEvt )
{
    sal_Bool bReturn;

    if ((bReturn = SwDrawBase::MouseButtonDown(rMEvt)) == sal_True)
    {
        if (!nAnzButUp)
            aStartPnt = m_pWin->PixelToLogic(rMEvt.GetPosPixel());
    }
    return (bReturn);
}

/*************************************************************************
|*
|* MouseButtonUp-event
|*
\************************************************************************/



sal_Bool ConstArc::MouseButtonUp( const MouseEvent& rMEvt )
{
    sal_Bool bReturn = sal_False;

    if ((m_pSh->IsDrawCreate() || m_pWin->IsDrawAction()) && rMEvt.IsLeft())
    {
        Point aPnt(m_pWin->PixelToLogic(rMEvt.GetPosPixel()));
        if (!nAnzButUp && aPnt == aStartPnt)
        {
            SwDrawBase::MouseButtonUp(rMEvt);
            bReturn = sal_True;
        }
        else
        {   nAnzButUp++;

            if (nAnzButUp == 3)     // Kreisbogenerzeugung beendet
            {
                SwDrawBase::MouseButtonUp(rMEvt);
                nAnzButUp = 0;
                bReturn = sal_True;
            }
            else
                m_pSh->EndCreate(SDRCREATE_NEXTPOINT);
        }
    }

    return (bReturn);
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/



void ConstArc::Activate(const sal_uInt16 nSlotId)
{
    SdrObjectCreationInfo aSdrObjectCreationInfo;

    switch (nSlotId)
    {
        case SID_DRAW_ARC:
            aSdrObjectCreationInfo.setIdent(OBJ_CIRC);
            aSdrObjectCreationInfo.setSdrCircleObjType(CircleType_Arc);
            break;
        case SID_DRAW_PIE:
            aSdrObjectCreationInfo.setIdent(OBJ_CIRC);
            aSdrObjectCreationInfo.setSdrCircleObjType(CircleType_Sector);
            break;
        case SID_DRAW_CIRCLECUT:
            aSdrObjectCreationInfo.setIdent(OBJ_CIRC);
            aSdrObjectCreationInfo.setSdrCircleObjType(CircleType_Segment);
            break;
        default:
            break;
    }

    m_pWin->setSdrObjectCreationInfo(aSdrObjectCreationInfo);

    SwDrawBase::Activate(nSlotId);
}

/*************************************************************************
|*
|* Funktion deaktivieren
|*
\************************************************************************/

void ConstArc::Deactivate()
{
    nAnzButUp = 0;

    SwDrawBase::Deactivate();
}



