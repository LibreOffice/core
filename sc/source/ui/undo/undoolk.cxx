/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: undoolk.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 22:40:05 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#include <svx/svdundo.hxx>

#include "document.hxx"
#include "drwlayer.hxx"


// STATIC DATA -----------------------------------------------------------

// -----------------------------------------------------------------------

SdrUndoAction* GetSdrUndoAction( ScDocument* pDoc )
{
    ScDrawLayer* pLayer = pDoc->GetDrawLayer();
    if (pLayer)
        return pLayer->GetCalcUndo();               // muss vorhanden sein
    else
        return NULL;
}

void DoSdrUndoAction( SdrUndoAction* pUndoAction )
{
    pUndoAction->Undo();
}


void RedoSdrUndoAction( SdrUndoAction* pUndoAction )
{
    pUndoAction->Redo();
}

void DeleteSdrUndoAction( SdrUndoAction* pUndoAction )
{
    delete pUndoAction;
}

void EnableDrawAdjust( ScDocument* pDoc, BOOL bEnable )
{
    ScDrawLayer* pLayer = pDoc->GetDrawLayer();
    if (pLayer)
        pLayer->EnableAdjust(bEnable);
}



