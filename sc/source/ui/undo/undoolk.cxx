/*************************************************************************
 *
 *  $RCSfile: undoolk.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:07 $
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

//------------------------------------------------------------------

//sfx
//#define _BASEDLGS_HXX //*
#define _DLGCFG_HXX
#define _SFXBASIC_HXX
#define _MODALDLG_HXX
#define _SFX_TEMPLDLG_HXX
#define _SFXSTBMGR_HXX
#define _SFXTBXMGR_HXX
#define _SFXIMGMGR_HXX
#define _SFXMNUITEM_HXX
#define _SFXMNUMGR_HXX
#define _SFXSTBITEM_HXX
#define _SFXTBXCTRL_HXX
#define _PASSWD_HXX
#define _SFXFILEDLG_HXX
#define _SFXREQUEST_HXX
#define _SFXOBJFACE_HXX
#define _SFXMSGPOOL_HXX
#define _SFXMSGDESCR_HXX
#define _SFXMSG_HXX
#define _INTRO_HXX
#define _SFX_PRNMON_HXX

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



