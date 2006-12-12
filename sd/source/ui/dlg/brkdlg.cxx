/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: brkdlg.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 16:59:04 $
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
#include "precompiled_sd.hxx"

#ifdef SD_DLLIMPLEMENTATION
#undef SD_DLLIMPLEMENTATION
#endif

#include "BreakDlg.hxx"

#ifndef _SFX_PROGRESS_HXX
#include <sfx2/progress.hxx>
#endif

#include <svx/svdedtv.hxx>
#include <svx/svdetc.hxx>
#include <sfx2/app.hxx>
#include <vcl/msgbox.hxx>

#include "sdattr.hxx"
#include "brkdlg.hrc"
#include "sdresid.hxx"
#ifndef SD_VIEW_HXX
#include "View.hxx"
#endif
#ifndef SD_DRAW_VIEW_HXX
#include "drawview.hxx"
#endif
#include "strings.hrc"
#ifndef SD_DRAW_DOC_SHELL_HXX
#include "DrawDocShell.hxx"
#endif

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
    ULONG nSumActionCount,
    ULONG nObjCount )
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
    bCancel = FALSE;

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
  bCancel = TRUE;
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
    EndDialog(TRUE);
    return 0L;
}

} // end of namespace sd
