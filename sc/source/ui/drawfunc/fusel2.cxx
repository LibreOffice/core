/*************************************************************************
 *
 *  $RCSfile: fusel2.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2004-06-04 11:27:24 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#include <svx/svditer.hxx>
#include <svx/svdocapt.hxx>
#include <svx/svdpagv.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/svdview.hxx>
#include <svx/outliner.hxx>

#include "fusel.hxx"
#include "tabvwsh.hxx"
#include "document.hxx"
#include "detfunc.hxx"
#include "futext.hxx"
#include "sc.hrc"

// -----------------------------------------------------------------------

inline long Diff( const Point& rP1, const Point& rP2 )
{
    long nX = rP1.X() - rP2.X();
    if (nX<0) nX = -nX;
    long nY = rP1.Y() - rP2.Y();
    if (nY<0) nY = -nY;
    return nX+nY;
}

BOOL FuSelection::TestDetective( SdrPageView* pPV, const Point& rPos )
{
    if (!pPV)
        return FALSE;

    BOOL bFound = FALSE;
    SdrObjListIter aIter( *pPV->GetObjList(), IM_FLAT );
    SdrObject* pObject = aIter.Next();
    while (pObject && !bFound)
    {
        if (ScDetectiveFunc::IsNonAlienArrow( pObject ))
        {
            USHORT nHitLog = (USHORT) pWindow->PixelToLogic(
                                Size(pView->GetHitTolerancePixel(),0)).Width();
            if ( pObject->IsHit( rPos, nHitLog ) )
            {
                ScViewData* pViewData = pViewShell->GetViewData();
                ScSplitPos ePos = pViewShell->FindWindow( pWindow );
                Point aLineStart = pObject->GetPoint(0);
                Point aLineEnd   = pObject->GetPoint(1);
                Point aPixel = pWindow->LogicToPixel( aLineStart );
                SCsCOL nStartCol;
                SCsROW nStartRow;
                pViewData->GetPosFromPixel( aPixel.X(), aPixel.Y(), ePos, nStartCol, nStartRow );
                aPixel = pWindow->LogicToPixel( aLineEnd );
                SCsCOL nEndCol;
                SCsROW nEndRow;
                pViewData->GetPosFromPixel( aPixel.X(), aPixel.Y(), ePos, nEndCol, nEndRow );
                SCsCOL nCurX = (SCsCOL) pViewData->GetCurX();
                SCsROW nCurY = (SCsROW) pViewData->GetCurY();
                BOOL bStart = ( Diff( rPos,aLineStart ) > Diff( rPos,aLineEnd ) );
                if ( nCurX == nStartCol && nCurY == nStartRow )
                    bStart = FALSE;
                else if ( nCurX == nEndCol && nCurY == nEndRow )
                    bStart = TRUE;

                SCsCOL nDifX;
                SCsROW nDifY;
                if ( bStart )
                {
                    nDifX = nStartCol - nCurX;
                    nDifY = nStartRow - nCurY;
                }
                else
                {
                    nDifX = nEndCol - nCurX;
                    nDifY = nEndRow - nCurY;
                }
                pViewShell->MoveCursorRel( nDifX, nDifY, SC_FOLLOW_JUMP, FALSE );

                bFound = TRUE;
            }
        }

        pObject = aIter.Next();
    }
    return bFound;
}

BOOL FuSelection::TestComment( SdrPageView* pPV, const Point& rPos )
{
    if (!pPV)
        return FALSE;

    SdrObject* pFoundObj = NULL;

    SdrObjListIter aIter( *pPV->GetObjList(), IM_FLAT );
    SdrObject* pObject = aIter.Next();
    while (pObject)
    {
        if ( pObject->GetLayer()==SC_LAYER_INTERN && pObject->ISA(SdrCaptionObj)
            && pObject->GetLogicRect().IsInside( rPos ) )
        {
            pFoundObj = pObject;
            // keep searching - use the last matching object (on top)
        }
        pObject = aIter.Next();
    }

    if ( pFoundObj )
    {
        pViewShell->GetViewData()->GetDispatcher().
            Execute(SID_DRAW_NOTEEDIT, SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD);
        // now get the created FuText and put in EditMode
        FuPoor* pPoor = pViewShell->GetViewData()->GetView()->GetDrawFuncPtr();
        if ( pPoor && pPoor->GetSlotID() == SID_DRAW_NOTEEDIT )  // no RTTI
        {
            FuText* pText = (FuText*)pPoor;
            Point aPixel = pWindow->LogicToPixel( rPos );
            pText->SetInEditMode( pFoundObj, &aPixel );
        }

        //  repaint outliner view with background now

        OutlinerView* pOlView = pView->GetTextEditOutlinerView();
        if ( pOlView && pOlView->GetWindow() == pWindow )
        {
            Rectangle aEditRect = pOlView->GetOutputArea();
            pWindow->SetFillColor( pOlView->GetBackgroundColor() );
            pWindow->SetLineColor();
            pWindow->DrawRect( aEditRect );
            pOlView->Paint( aEditRect );
        }
    }

    return (pFoundObj != NULL);
}

//==================================================================




