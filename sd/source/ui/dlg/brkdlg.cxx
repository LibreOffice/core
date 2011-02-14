/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#ifdef SD_DLLIMPLEMENTATION
#undef SD_DLLIMPLEMENTATION
#endif

#include "BreakDlg.hxx"
#include <sfx2/progress.hxx>

#include <svx/svdedtv.hxx>
#include <svx/svdetc.hxx>
#include <sfx2/app.hxx>
#include <vcl/msgbox.hxx>

#include "sdattr.hxx"
#include "brkdlg.hrc"
#include "sdresid.hxx"
#include "View.hxx"
#include "drawview.hxx"
#include "strings.hrc"
#include "DrawDocShell.hxx"

namespace sd {

/*************************************************************************
|*
|* Dialog zum aufbrechen von Metafiles
|*
\************************************************************************/

BreakDlg::BreakDlg(
    ::Window* pWindow,
    DrawView* _pDrView,
    DrawDocShell* pShell,
    sal_uLong nSumActionCount,
    sal_uLong nObjCount )
    : SfxModalDialog     ( pWindow, SdResId( DLG_BREAK ) ),
      aFtObjInfo            ( this, SdResId( FT_OBJ_INFO ) ),
      aFtActInfo            ( this, SdResId( FT_ACT_INFO ) ),
      aFtInsInfo            ( this, SdResId( FT_INS_INFO ) ),
      aFiObjInfo            ( this, SdResId( FI_OBJ_INFO ) ),
      aFiActInfo            ( this, SdResId( FI_ACT_INFO ) ),
      aFiInsInfo            ( this, SdResId( FI_INS_INFO ) ),
      aBtnCancel            ( this, SdResId( BTN_CANCEL ) ),
      aLink             ( LINK( this, BreakDlg, UpDate)),
      mpProgress            ( NULL )
{
    aBtnCancel.SetClickHdl( LINK( this, BreakDlg, CancelButtonHdl));

    mpProgress = new SfxProgress( pShell, String(SdResId(STR_BREAK_METAFILE)), nSumActionCount*3 );

    pProgrInfo = new SvdProgressInfo( &aLink );
    // jede Action wird in DoImport() 3mal bearbeitet
    pProgrInfo->Init( nSumActionCount*3, nObjCount );

    pDrView = _pDrView;
    bCancel = sal_False;

    FreeResource();
}

BreakDlg::~BreakDlg()
{
    if( mpProgress )
        delete mpProgress;

    if( pProgrInfo )
        delete pProgrInfo;
}

// Control-Handler fuer den Abbruch Button
IMPL_LINK( BreakDlg, CancelButtonHdl, void *, EMPTYARG )
{
  bCancel = sal_True;
  aBtnCancel.Disable();
  return( 0L );
}

// Die UpDate Methode muss regelmaessig von der Arbeitsfunktion
// ausgeuehrt werden.
// Beim ersten aufruf wird die gesamtanzahl der actions uebergeben.
// Jeder weitere sollte die bearbeiteten actions seit dem letzten aufruf von
// UpDate erhalten.

IMPL_LINK( BreakDlg, UpDate, void*, nInit )
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
short BreakDlg::Execute()
{
  aTimer.SetTimeout( 10 );
  aTimer.SetTimeoutHdl( LINK( this, BreakDlg, InitialUpdate ) );
  aTimer.Start();

  return SfxModalDialog::Execute();
}

// Linkmethode welche die Arbeitsfunktion startet
IMPL_LINK( BreakDlg, InitialUpdate, Timer*, EMPTYARG )
{
    pDrView->DoImportMarkedMtf(pProgrInfo);
    EndDialog(sal_True);
    return 0L;
}

} // end of namespace sd
