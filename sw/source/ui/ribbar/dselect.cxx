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


#include <sfx2/bindings.hxx>
#include "view.hxx"
#include "edtwin.hxx"
#include "wrtsh.hxx"
#include "cmdid.h"
#include "drawbase.hxx"
#include "dselect.hxx"

extern sal_Bool bNoInterrupt;       // in mainwn.cxx

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

DrawSelection::DrawSelection(SwWrtShell* pWrtShell, SwEditWin* pEditWin, SwView* pSwView) :
                SwDrawBase(pWrtShell, pEditWin, pSwView)
{
    m_bCreateObj = sal_False;
}

/*************************************************************************
|*
|* Tastaturereignisse bearbeiten
|*
|* Wird ein KeyEvent bearbeitet, so ist der Return-Wert sal_True, andernfalls
|* sal_False.
|*
\************************************************************************/

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

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/

void DrawSelection::Activate(const sal_uInt16 nSlotId)
{
    m_pWin->setSdrObjectCreationInfo(SdrObjectCreationInfo());
    m_pWin->SetObjectSelect( sal_True );
    SwDrawBase::Activate(nSlotId);

    m_pSh->GetView().GetViewFrame()->GetBindings().Invalidate(SID_INSERT_DRAW);
}


