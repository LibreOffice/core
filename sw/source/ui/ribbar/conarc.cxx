/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: conarc.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 12:23:56 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"



#ifndef _SVDOBJ_HXX //autogen
#include <svx/svdobj.hxx>
#endif


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



BOOL ConstArc::MouseButtonDown( const MouseEvent& rMEvt )
{
    BOOL bReturn;

    if ((bReturn = SwDrawBase::MouseButtonDown(rMEvt)) == TRUE)
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



BOOL ConstArc::MouseButtonUp( const MouseEvent& rMEvt )
{
    BOOL bReturn = FALSE;

    if ((m_pSh->IsDrawCreate() || m_pWin->IsDrawAction()) && rMEvt.IsLeft())
    {
        Point aPnt(m_pWin->PixelToLogic(rMEvt.GetPosPixel()));
        if (!nAnzButUp && aPnt == aStartPnt)
        {
            SwDrawBase::MouseButtonUp(rMEvt);
            bReturn = TRUE;
        }
        else
        {   nAnzButUp++;

            if (nAnzButUp == 3)     // Kreisbogenerzeugung beendet
            {
                SwDrawBase::MouseButtonUp(rMEvt);
                nAnzButUp = 0;
                bReturn = TRUE;
            }
            else
                m_pSh->EndCreate(SDRCREATE_NEXTPOINT);
        }
    }
/*  else if ( pView->IsCreateObj() && rMEvt.IsRight() )
    {
        pView->EndCreateObj( SDRCREATE_FORCEEND );
        bReturn = TRUE;
    }*/

    return (bReturn);
}

/*************************************************************************
|*
|* Function aktivieren
|*
\************************************************************************/



void ConstArc::Activate(const USHORT nSlotId)
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



