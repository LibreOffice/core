/*************************************************************************
 *
 *  $RCSfile: basides3.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: mh $ $Date: 2000-09-29 11:02:36 $
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

#define SI_NOCONTROL
#define SI_NOSBXCONTROLS

#include <basidesh.hrc>
#include <ide_pch.hxx>

#pragma hdrstop

#define __PRIVATE 1

#include <basidesh.hxx>
#include <baside3.hxx>
#include <basobj.hxx>

DialogWindow* BasicIDEShell::CreateDlgWin( StarBASIC* pBasic, String aDlgName, SbxObject* pDlg )
{
    bCreatingWindow = TRUE;

    ULONG nKey = 0;
    DialogWindow* pWin = 0;
    if ( !pDlg )
    {
        pDlg = BasicIDE::FindDialog( pBasic, aDlgName );
        if ( !pDlg )
            pDlg = BasicIDE::CreateDialog( pBasic, aDlgName );
        DBG_ASSERT( pDlg, "Es wurde kein Dialog erzeugt!" );
    }
    else    // Vielleicht gibt es ein suspendiertes?
        pWin = FindDlgWin( pBasic, aDlgName, FALSE, TRUE );


    VCSbxDialogRef xNewDlg = (VCSbxDialog*)pDlg;

    if ( !pWin )
    {
        pWin = new DialogWindow( &GetViewFrame()->GetWindow(), xNewDlg, pBasic );
        nKey = InsertWindowInTable( pWin );
    }
    else
    {
        pWin->SetStatus( pWin->GetStatus() & ~BASWIN_SUSPENDED );
        IDEBaseWindow* pTmp = aIDEWindowTable.First();
        while ( pTmp && !nKey )
        {
            if ( pTmp == pWin )
                nKey = aIDEWindowTable.GetCurKey();
            pTmp = aIDEWindowTable.Next();
        }
        DBG_ASSERT( nKey, "CreateDlgWin: Kein Key - Fenster nicht gefunden!" );
    }
    pWin->GrabScrollBars( &aHScrollBar, &aVScrollBar );
    pTabBar->InsertPage( (USHORT)nKey, xNewDlg->GetName() );
    if ( !pCurWin )
        SetCurWindow( pWin, FALSE, FALSE );

    bCreatingWindow = FALSE;
    return pWin;
}

DialogWindow* BasicIDEShell::FindDlgWin( StarBASIC* pBasic, const String& rDlgName, BOOL bCreateIfNotExist, BOOL bFindSuspended )
{
    DialogWindow* pDlgWin = 0;
    IDEBaseWindow* pWin = aIDEWindowTable.First();
    while ( pWin && !pDlgWin )
    {
        if ( ( !pWin->IsSuspended() || bFindSuspended ) && pWin->IsA( TYPE( DialogWindow ) ) )
        {
            String aDlgStr( ((DialogWindow*)pWin)->GetDialogName() );
            if ( !pBasic )  // nur irgendeins finden...
                pDlgWin = (DialogWindow*)pWin;
            else if ( ( pWin->GetBasic() == pBasic ) &&
                ( ( rDlgName.Len() == 0 ) || ( aDlgStr == rDlgName ) ) )
                pDlgWin = (DialogWindow*)pWin;
        }
        pWin = aIDEWindowTable.Next();
    }
    if ( !pDlgWin && bCreateIfNotExist )
        pDlgWin = CreateDlgWin( pBasic, rDlgName );

    return pDlgWin;
}

SdrView* BasicIDEShell::GetCurDlgView()
{
    if ( !pCurWin || !pCurWin->IsA( TYPE( DialogWindow ) ) )
        return NULL;

    DialogWindow* pWin = (DialogWindow*)pCurWin;
    return pWin->GetView();
}

// Nur wenn Dialogfenster oben:
void __EXPORT BasicIDEShell::ExecuteDialog( SfxRequest& rReq )
{
    if ( !pCurWin || !pCurWin->IsA( TYPE( DialogWindow ) ) )
        return;

    pCurWin->ExecuteCommand( rReq );
}

