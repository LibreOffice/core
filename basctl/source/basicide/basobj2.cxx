/*************************************************************************
 *
 *  $RCSfile: basobj2.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2000-11-14 14:13:15 $
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


#include <ide_pch.hxx>

#pragma hdrstop

#ifndef _SBXCLASS_HXX //autogen
#include <svtools/sbx.hxx>
#endif
#include <svtools/moduleoptions.hxx>
#include <basobj.hxx>
#include <iderdll.hxx>
#include <iderdll2.hxx>
#include <iderid.hxx>
#include <macrodlg.hxx>
#include <moduldlg.hxx>
#include <basidesh.hxx>

SfxMacro* BasicIDE::CreateMacro()
{
    DBG_ERROR( "BasicIDE::CreateMacro() - war eigentlich nur fuer Macro-Recording!" );
    IDE_DLL()->GetExtraData()->ChoosingMacro() = TRUE;
    SFX_APP()->EnterBasicCall();
    Window* pParent = Application::GetDefDialogParent();
    SfxMacro* pMacro = 0;
    MacroChooser* pChooser = new MacroChooser( pParent, TRUE );
    Window* pOldModalDialogParent = Application::GetDefDialogParent();
    Application::SetDefDialogParent( pChooser );
//  pChooser->SetMode( MACROCHOOSER_RECORDING );
    short nRetValue = pChooser->Execute();

    Application::SetDefDialogParent( pOldModalDialogParent );
    delete pChooser;

    SFX_APP()->LeaveBasicCall();
    IDE_DLL()->GetExtraData()->ChoosingMacro() = FALSE;

    return pMacro;
}

SbMethod* BasicIDE::ChooseMacro( BOOL bExecute, BOOL bChooseOnly )
{
    IDE_DLL()->GetExtraData()->ChoosingMacro() = TRUE;
    SFX_APP()->EnterBasicCall();

    SbMethod* pMethod = NULL;

    Window* pParent = Application::GetDefDialogParent();

    MacroChooser* pChooser = new MacroChooser( pParent, TRUE );
    if ( bChooseOnly || !SvtModuleOptions().IsBasicIDE() )
        pChooser->SetMode( MACROCHOOSER_CHOOSEONLY );

    short nRetValue = pChooser->Execute();

    IDE_DLL()->GetExtraData()->ChoosingMacro() = FALSE;

    switch ( nRetValue )
    {
        case MACRO_OK_RUN:
        {
            pMethod = pChooser->GetMacro();
            if ( pMethod && bExecute )
            {
                pMethod->AddRef();  // festhalten, bis Event abgearbeitet.
                Application::PostUserEvent( LINK( IDE_DLL()->GetExtraData(), BasicIDEData, ExecuteMacroEvent ), pMethod );
            }
        }
        break;
    }

    delete pChooser;

    SFX_APP()->LeaveBasicCall();

    return pMethod;
}

SbMethod* BasicIDE::ChooseMacro( BOOL bExecute, BOOL bChooseOnly, const String& rPreferredMacroDesciption )
{
    if ( rPreferredMacroDesciption.Len() )
        IDE_DLL()->GetExtraData()->GetLastMacro() = rPreferredMacroDesciption;
    return BasicIDE::ChooseMacro( bExecute, bChooseOnly );
}

void BasicIDE::Organize()
{
    Window* pParent = Application::GetDefDialogParent();
    OrganizeDialog* pDlg = new OrganizeDialog( pParent );
    if ( IDE_DLL()->GetShell() )
    {
        IDEBaseWindow* pWin = IDE_DLL()->GetShell()->GetCurWindow();
        if ( pWin )
            pDlg->SetCurrentModule( pWin->CreateSbxDescription() );
    }
    pDlg->Execute();
    delete pDlg;
}



BOOL BasicIDE::IsValidSbxName( const String& rName )
{
    for ( USHORT nChar = 0; nChar < rName.Len(); nChar++ )
    {
        BOOL bValid = ( ( rName.GetChar(nChar) >= 'A' && rName.GetChar(nChar) <= 'Z' ) ||
                        ( rName.GetChar(nChar) >= 'a' && rName.GetChar(nChar) <= 'z' ) ||
                        ( rName.GetChar(nChar) >= '0' && rName.GetChar(nChar) <= '9' && nChar ) ||
                        ( rName.GetChar(nChar) == '_' ) );
        if ( !bValid )
            return FALSE;
    }
    return TRUE;
}



SbMethod* BasicIDE::FindMacro( SbModule* pModule, const String& rMacroName )
{
    return (SbMethod*)pModule->GetMethods()->Find( rMacroName, SbxCLASS_METHOD );
}

SbModule* BasicIDE::FindModule( StarBASIC* pBasic, const String& rModName )
{
    return pBasic->FindModule( rModName );
}

USHORT BasicIDE::GetBasicDialogCount()
{
    return IDE_DLL()->GetExtraData()->GetBasicDialogCount();
}

void BasicIDE::IncBasicDialogCount()
{
    IDE_DLL()->GetExtraData()->IncBasicDialogCount();
}

void BasicIDE::DecBasicDialogCount()
{
    DBG_ASSERT( GetBasicDialogCount(), "DecBasicDialogCount() - Count allready 0!" );
    IDE_DLL()->GetExtraData()->DecBasicDialogCount();
}
