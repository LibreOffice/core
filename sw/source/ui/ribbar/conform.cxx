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


#include <svx/fmglob.hxx>
#include <svx/svdview.hxx>
#include <svx/fmshell.hxx>

#include "view.hxx"
#include "edtwin.hxx"
#include "wrtsh.hxx"
#include "drawbase.hxx"
#include "conform.hxx"

extern sal_Bool bNoInterrupt;       // in mainwn.cxx

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/


ConstFormControl::ConstFormControl(SwWrtShell* pWrtShell, SwEditWin* pEditWin, SwView* pSwView) :
    SwDrawBase(pWrtShell, pEditWin, pSwView)
{
    m_bInsForm = sal_True;
}

/*************************************************************************
|*
|* MouseButtonDown-event
|*
\************************************************************************/


sal_Bool ConstFormControl::MouseButtonDown(const MouseEvent& rMEvt)
{
    sal_Bool bReturn = sal_False;

    SdrView *pSdrView = m_pSh->GetDrawView();

    pSdrView->SetOrthogonal(rMEvt.IsShift());
    pSdrView->SetAngleSnapEnabled(rMEvt.IsShift());

    if (rMEvt.IsMod2())
    {
        pSdrView->SetCreate1stPointAsCenter(sal_True);
        pSdrView->SetResizeAtCenter(sal_True);
    }
    else
    {
        pSdrView->SetCreate1stPointAsCenter(sal_False);
        pSdrView->SetResizeAtCenter(sal_False);
    }

    SdrViewEvent aVEvt;
    SdrHitKind eHit = pSdrView->PickAnything(rMEvt, SDRMOUSEBUTTONDOWN, aVEvt);

    // Nur neues Objekt, wenn nicht im Basismode (bzw reinem Selektionsmode)
    if (rMEvt.IsLeft() && !m_pWin->IsDrawAction() &&
        (eHit == SDRHIT_UNMARKEDOBJECT || eHit == SDRHIT_NONE || m_pSh->IsDrawCreate()))
    {
        bNoInterrupt = sal_True;
        m_pWin->CaptureMouse();

        m_pWin->SetPointer(Pointer(POINTER_DRAW_RECT));

        const basegfx::B2DPoint aPixelPos(rMEvt.GetPosPixel().X(), rMEvt.GetPosPixel().Y());
        m_aStartPos = m_pWin->GetInverseViewTransformation() * aPixelPos;

        bReturn = m_pSh->BeginCreate(m_pWin->getSdrObjectCreationInfo(), m_aStartPos);

        if (bReturn)
            m_pWin->SetDrawAction(sal_True);
    }
    else
        bReturn = SwDrawBase::MouseButtonDown(rMEvt);

    return (bReturn);
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/


void ConstFormControl::Activate(const sal_uInt16 nSlotId)
{
    const SdrObjectCreationInfo aSdrObjectCreationInfo(nSlotId);

    m_pWin->setSdrObjectCreationInfo(aSdrObjectCreationInfo);
    SwDrawBase::Activate(nSlotId);
    m_pSh->GetDrawView()->setSdrObjectCreationInfo(aSdrObjectCreationInfo);

    m_pWin->SetPointer(Pointer(POINTER_DRAW_RECT));
}
/* -----------------------------19.04.2002 12:42------------------------------

 ---------------------------------------------------------------------------*/
void ConstFormControl::CreateDefaultObject()
{
    const basegfx::B2DPoint aCenterPos(GetDefaultCenterPos().X(), GetDefaultCenterPos().Y());
    const basegfx::B2DPoint aStartPos(aCenterPos - basegfx::B2DPoint(2.0 * MM50, MM50));
    const basegfx::B2DPoint aEndPos(aCenterPos + basegfx::B2DPoint(2.0 * MM50, MM50));

    if(!m_pSh->HasDrawView())
        m_pSh->MakeDrawView();

    SdrView *pSdrView = m_pSh->GetDrawView();
    pSdrView->SetDesignMode(true);

    m_pSh->BeginCreate(m_pWin->getSdrObjectCreationInfo(), aStartPos);
    m_pSh->MoveCreate(aEndPos);
    m_pSh->EndCreate(SDRCREATE_FORCEEND);
}

