/*************************************************************************
 *
 *  $RCSfile: brkdlg.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:31 $
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

#ifndef _SFX_PROGRESS_HXX
#include <sfx2/progress.hxx>
#endif

#include <svx/svdedtv.hxx>
#include <svx/svdetc.hxx>
#include <sfx2/app.hxx>
#include <vcl/msgbox.hxx>

#include "sdattr.hxx"
#include "brkdlg.hxx"
#include "brkdlg.hrc"
#include "sdresid.hxx"
#include "sdview.hxx"
#include "drawview.hxx"
#include "strings.hrc"
#include "docshell.hxx"

/*************************************************************************
|*
|* Dialog zum aufbrechen von Metafiles
|*
\************************************************************************/

SdBreakDlg::SdBreakDlg( Window* pWindow, SdDrawView* _pDrView, SdDrawDocShell* pShell,
                        ULONG nSumActionCount, ULONG nObjCount ) :
                SfxModalDialog     ( pWindow, SdResId( DLG_BREAK ) ),
                aFtObjInfo          ( this, SdResId( FT_OBJ_INFO ) ),
                aFtActInfo          ( this, SdResId( FT_ACT_INFO ) ),
                aFtInsInfo          ( this, SdResId( FT_INS_INFO ) ),
                aFiObjInfo          ( this, SdResId( FI_OBJ_INFO ) ),
                aFiActInfo          ( this, SdResId( FI_ACT_INFO ) ),
                aFiInsInfo          ( this, SdResId( FI_INS_INFO ) ),
                aBtnCancel          ( this, SdResId( BTN_CANCEL ) ),
                aLink               ( LINK( this, SdBreakDlg, UpDate)),
                mpProgress          ( NULL )
{
    aBtnCancel.SetClickHdl( LINK( this, SdBreakDlg, CancelButtonHdl));

    mpProgress = new SfxProgress( pShell, String(SdResId(STR_BREAK_METAFILE)), nSumActionCount*3 );

    pProgrInfo = new SvdProgressInfo( &aLink );
    // jede Action wird in DoImport() 3mal bearbeitet
    pProgrInfo->Init( nSumActionCount*3, nObjCount );

    pDrView = _pDrView;
    bCancel = FALSE;

    FreeResource();
}

SdBreakDlg::~SdBreakDlg()
{
    if( mpProgress )
        delete mpProgress;

    if( pProgrInfo )
        delete pProgrInfo;
}

// Control-Handler fuer den Abbruch Button
IMPL_LINK( SdBreakDlg, CancelButtonHdl, void *, EMPTYARG )
{
  bCancel = TRUE;
  aBtnCancel.Disable();
  return( 0L );
}

// Die UpDate Methode muss regelmaessig von der Arbeitsfunktion
// ausgeuehrt werden.
// Beim ersten aufruf wird die gesamtanzahl der actions uebergeben.
// Jeder weitere sollte die bearbeiteten actions seit dem letzten aufruf von
// UpDate erhalten.

IMPL_LINK( SdBreakDlg, UpDate, void*, nInit )
{
    String aEmptyStr;

    if(pProgrInfo == NULL)
      return 1L;

    // Statuszeile updaten oder Fehlermeldung?
    if(nInit == (void*)1L)
    {
        ErrorBox aErrBox( this, WB_OK, String( SdResId( STR_BREAK_FAIL ) ) );
        aErrBox.Execute();
    }
    else
    {
        if(mpProgress)
            mpProgress->SetState( pProgrInfo->GetSumCurAction() );
    }

    // Welches Oject wird gerade angezeigt?
    String info = UniString::CreateFromInt32( pProgrInfo->GetCurObj() );
    info.Append( sal_Unicode('/') );
    info.Append( UniString::CreateFromInt32( pProgrInfo->GetObjCount() ) );
    aFiObjInfo.SetText(info);

    // Wieviele Actions sind schon aufgebrochen?
    if(pProgrInfo->GetActionCount() == 0)
    {
        aFiActInfo.SetText( aEmptyStr );
    }
    else
    {
        info = UniString::CreateFromInt32( pProgrInfo->GetCurAction() );
        info.Append( sal_Unicode('/') );
        info.Append( UniString::CreateFromInt32( pProgrInfo->GetActionCount() ) );
        aFiActInfo.SetText(info);
    }

    // Und erst eingefuegt????
    if(pProgrInfo->GetInsertCount() == 0)
    {
        aFiInsInfo.SetText( aEmptyStr );
    }
    else
    {
        info = UniString::CreateFromInt32( pProgrInfo->GetCurInsert() );
        info.Append( sal_Unicode('/') );
        info.Append( UniString::CreateFromInt32( pProgrInfo->GetInsertCount() ) );
        aFiInsInfo.SetText(info);
    }

    Application::Reschedule();
    return( bCancel?0L:1L );
}

// Oeffnet den Modalen Dialog und startet einen Timer der die Arbeitsfunktion
// nach oeffnen des Dialogs ausfuehrt
short SdBreakDlg::Execute()
{
  aTimer.SetTimeout( 10 );
  aTimer.SetTimeoutHdl( LINK( this, SdBreakDlg, InitialUpdate ) );
  aTimer.Start();

  return SfxModalDialog::Execute();
}

// Linkmethode welche die Arbeitsfunktion startet
IMPL_LINK( SdBreakDlg, InitialUpdate, Timer*, pTimer )
{
    pDrView->DoImportMarkedMtf(pProgrInfo);
    EndDialog(TRUE);
    return 0L;
}

