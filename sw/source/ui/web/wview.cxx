/*************************************************************************
 *
 *  $RCSfile: wview.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: tbe $ $Date: 2000-10-23 10:54:54 $
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

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#include "itemdef.hxx"

#ifndef _SVX_SRCHITEM_HXX
#include <svx/srchitem.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFX_TEMPLDLG_HXX //autogen
#include <sfx2/templdlg.hxx>
#endif
#ifndef _SVX_SRCHDLG_HXX //autogen
#include <svx/srchdlg.hxx>
#endif
//#ifndef _IDETEMP_HXX //autogen
//#include <basctl/idetemp.hxx>
//#endif
#ifndef _SBX_SBXOBJECT_HXX //autogen
#include <svtools/sbxobj.hxx>
#endif
#ifndef _UIVWIMP_HXX
#include <uivwimp.hxx>
#endif
#ifndef _SVX_FMSHELL_HXX //autogen
#include <svx/fmshell.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif


#ifndef _SWMODULE_HXX //autogen
#include <swmodule.hxx>
#endif
#ifndef _UNOTXVW_HXX //autogen
#include <unotxvw.hxx>
#endif
#include "swtypes.hxx"
#include "cmdid.h"
#include "globals.hrc"
#include "wrtsh.hxx"
#include "edtwin.hxx"
#include "wgrfsh.hxx"
#include "wfrmsh.hxx"
#include "wolesh.hxx"
#include "wtabsh.hxx"
#include "wlistsh.hxx"
#include "wformsh.hxx"
#include "wtextsh.hxx"
#include "barcfg.hxx"
#include "doc.hxx"

// EIGENTLICH nicht moeglich !!
#include "beziersh.hxx"
#include "drawsh.hxx"
#include "drwtxtsh.hxx"

#include "wview.hxx"
#include "wdocsh.hxx"
#include "web.hrc"
#include "shells.hrc"

#define SwWebView
#define SearchAttributes
#define ReplaceAttributes
#define SearchSettings
#define _ExecSearch ExecSearch
#define _StateSearch StateSearch
#define Frames
#define Graphics
#define OLEObjects
#define Controls
#define Text
#define Frame
#define Graphic
#define Object
#define Draw
#define DrawText
#define TextInTable
#define ListInText
#define ListInTable
#define Page
#include <svx/svxslots.hxx>
#include "swslots.hxx"


SFX_IMPL_VIEWFACTORY(SwWebView, SW_RES(STR_NONAME))
{
    SFX_VIEW_REGISTRATION(SwWebDocShell);
}


SFX_IMPL_INTERFACE( SwWebView, SwView, SW_RES(RID_WEBTOOLS_TOOLBOX) )
{
    SFX_CHILDWINDOW_REGISTRATION(SfxTemplateDialogWrapper::GetChildWindowId());
    SFX_CHILDWINDOW_REGISTRATION(SvxSearchDialogWrapper::GetChildWindowId());
    SFX_OBJECTBAR_REGISTRATION( SFX_OBJECTBAR_TOOLS|
                                SFX_VISIBILITY_STANDARD|SFX_VISIBILITY_SERVER,
                                SW_RES(RID_WEBTOOLS_TOOLBOX) );
}

TYPEINIT1(SwWebView,SwView)

/*-----------------22.01.97 14.27-------------------

--------------------------------------------------*/


SwWebView::SwWebView(SfxViewFrame* pFrame, SfxViewShell* pShell) :
    SwView(pFrame, pShell)
{
}

/*-----------------22.01.97 14.27-------------------

--------------------------------------------------*/


SwWebView::~SwWebView()
{
}

/*-----------------23.01.97 09.01-------------------

--------------------------------------------------*/


void SwWebView::SelectShell()
{
    // Entscheidung, ob UpdateTable gerufen werden muss
    BOOL bUpdateTable = FALSE;
    const SwFrmFmt* pCurTableFmt = GetWrtShell().GetTableFmt();
    if(pCurTableFmt && pCurTableFmt != GetLastTblFrmFmt())
    {
        bUpdateTable = TRUE; // kann erst spaeter ausgefuehrt werden
    }
    SetLastTblFrmFmt(pCurTableFmt);
    //SEL_TBL und SEL_TBL_CELLS koennen verodert sein!
    int nNewSelectionType = (GetWrtShell().GetSelectionType()
                                & ~SwWrtShell::SEL_TBL_CELLS);

    int nSelectionType = GetSelectionType();
    if ( nNewSelectionType == nSelectionType )
    {
        GetViewFrame()->GetBindings().InvalidateAll( FALSE );
        if ( nSelectionType & SwWrtShell::SEL_OLE ||
             nSelectionType & SwWrtShell::SEL_GRF )
            //Fuer Grafiken und OLE kann sich natuerlich das Verb aendern!
            ImpSetVerb( nNewSelectionType );
    }
    else
    {

    //  DELETEZ(pxSelectionObj); //Selektionsobjekt loeschen
        SfxDispatcher &rDispatcher = *GetViewFrame()->GetDispatcher();
        SwToolbarConfigItem *pBarCfg = SW_MOD()->GetWebToolbarConfig();

        if( GetCurShell() )
        {
            rDispatcher.Flush();        // alle gecachten Shells wirklich loeschen

            //Zur alten Selektion merken welche Toolbar sichtbar war
            USHORT nId = rDispatcher.GetObjectBarId( SFX_OBJECTBAR_OBJECT );
            if ( nId )
                pBarCfg->SetTopToolbar( nSelectionType, nId );

            SfxShell *pSfxShell;
            USHORT i;
            for ( i = 0; TRUE; ++i )
            {
                pSfxShell = rDispatcher.GetShell( i );
                if ( !(pSfxShell->ISA( SwBaseShell ) ||
                    pSfxShell->ISA( SwDrawTextShell )) )
                    break;
            }
            pSfxShell = rDispatcher.GetShell( --i );
            ASSERT( pSfxShell, "My Shell ist lost in space" );
            rDispatcher.Pop( *pSfxShell, SFX_SHELL_POP_UNTIL | SFX_SHELL_POP_DELETE);
        }

        FASTBOOL bInitFormShell = FALSE;
        if( !GetFormShell() )
        {
            bInitFormShell = TRUE;
            SetFormShell( new FmFormShell( this ) );
            rDispatcher.Push( *GetFormShell() );
        }

        FASTBOOL bSetExtInpCntxt = FALSE;
        nSelectionType = nNewSelectionType;
        SetSelectionType( nSelectionType );
        ShellModes eShellMode;

        if ( nSelectionType & SwWrtShell::SEL_OLE )
        {
            eShellMode = SEL_OBJECT;
            SetShell( new SwWebOleShell( *this ));
            rDispatcher.Push( *GetCurShell() );
        }
        else if ( nSelectionType & SwWrtShell::SEL_FRM
            || nSelectionType & SwWrtShell::SEL_GRF)
        {
            eShellMode = SEL_FRAME;
            SetShell( new SwWebFrameShell( *this ));
            rDispatcher.Push( *GetCurShell() );
            if(nSelectionType & SwWrtShell::SEL_GRF )
            {
                eShellMode = SEL_GRAPHIC;
                SetShell( new SwWebGrfShell( *this ));
                rDispatcher.Push( *GetCurShell() );
            }
        }
        else if ( nSelectionType & SwWrtShell::SEL_FRM )
        {
            eShellMode = SEL_FRAME;
            SetShell( new SwWebFrameShell( *this ));
            rDispatcher.Push( *GetCurShell() );
        }
        else if ( nSelectionType & SwWrtShell::SEL_DRW )
        {
            eShellMode = SEL_DRAW;
            SetShell( new SwDrawShell( *this ));
            rDispatcher.Push( *GetCurShell() );
            if ( nSelectionType & SwWrtShell::SEL_BEZ )
            {
                eShellMode = SEL_BEZIER;
                SetShell( new SwBezierShell( *this ));
                rDispatcher.Push( *GetCurShell() );
            }

        }
        else if ( nSelectionType & SwWrtShell::SEL_DRW_FORM )
        {
            eShellMode = SEL_DRAW_FORM;
            SetShell( new SwWebDrawFormShell( *this ));

            rDispatcher.Push( *GetCurShell() );
        }
        else if ( nSelectionType & SwWrtShell::SEL_DRW_TXT )
        {
            eShellMode = SEL_DRAWTEXT;
            rDispatcher.Push( *(new SwBaseShell( *this )) );
            SetShell( new SwDrawTextShell( *this ));
            rDispatcher.Push( *GetCurShell() );
        }
        else
        {
            bSetExtInpCntxt = TRUE;
            eShellMode = SEL_TEXT;
            if ( nSelectionType & SwWrtShell::SEL_NUM )
            {
                eShellMode = SEL_LIST_TEXT;
                SetShell( new SwWebListShell( *this ));
                rDispatcher.Push( *GetCurShell() );
            }
            SetShell( new SwWebTextShell(*this));
            rDispatcher.Push( *GetCurShell() );
            if ( nSelectionType & SwWrtShell::SEL_TBL )
            {
                eShellMode = eShellMode == SEL_LIST_TEXT ? SEL_TABLE_LIST_TEXT
                                                        : SEL_TABLE_TEXT;
                SetShell( new SwWebTableShell( *this ));
                rDispatcher.Push( *GetCurShell() );
            }
        }
        ImpSetVerb( nSelectionType );
        GetViewImpl()->SetShellMode(eShellMode);

        if( !GetDocShell()->IsReadOnly() )
        {
            if( bSetExtInpCntxt && GetWrtShell().HasReadonlySel() )
                bSetExtInpCntxt = FALSE;

            InputContext aCntxt( GetEditWin().GetInputContext() );
            aCntxt.SetOptions( bSetExtInpCntxt
                                ? (aCntxt.GetOptions() |
                                        ( INPUTCONTEXT_TEXT |
                                            INPUTCONTEXT_EXTTEXTINPUT ))
                                : (aCntxt.GetOptions() & ~
                                        ( INPUTCONTEXT_TEXT |
                                            INPUTCONTEXT_EXTTEXTINPUT )) );
            GetEditWin().SetInputContext( aCntxt );
        }

        //Zur neuen Selektion die Toolbar aktivieren, die auch beim letzten Mal
        //aktiviert war
        //Vorher muss ein Flush() sein, betrifft aber lt. MBA nicht das UI und ist
        //kein Performance-Problem
        rDispatcher.Flush();
        USHORT nId = pBarCfg->GetTopToolbar( nSelectionType );
        if ( USHRT_MAX != nId )
            rDispatcher.ShowObjectBar( nId );

        Point aPnt = GetEditWin().GetPointerPosPixel();
        aPnt = GetEditWin().PixelToLogic(aPnt);
        GetEditWin().UpdatePointer(aPnt);

        if ( bInitFormShell && GetWrtShell().GetDrawView() )
            GetFormShell()->SetView( PTR_CAST( FmFormView,
                                                GetWrtShell().GetDrawView()));


    }
    GetViewImpl()->GetUNOObject_Impl()->NotifySelChanged();

    //Guenstiger Zeitpunkt fuer die Kommunikation mit OLE-Objekten?
    if ( GetDocShell()->GetDoc()->IsOLEPrtNotifyPending() )
        GetDocShell()->GetDoc()->PrtOLENotify( FALSE );

    //jetzt das Tabellen-Update
    if(bUpdateTable)
        GetWrtShell().UpdateTable();
}

/*------------------------------------------------------------------------
    $Log: not supported by cvs2svn $
    Revision 1.1.1.1  2000/09/18 17:14:51  hr
    initial import

    Revision 1.45  2000/09/18 16:06:20  willem.vandorp
    OpenOffice header added.

    Revision 1.44  2000/09/07 15:59:35  os
    change: SFX_DISPATCHER/SFX_BINDINGS removed

    Revision 1.43  2000/09/04 11:46:58  tbe
    basicide, isetbrw, si, vcdlged moved from svx to basctl

    Revision 1.42  2000/08/08 13:47:37  os
    #77423# separate graphic shell

    Revision 1.41  2000/05/10 11:54:44  os
    Basic API removed

    Revision 1.40  1999/09/10 13:19:16  os
    Chg: resource types removed

    Revision 1.39  1999/08/05 17:25:16  JP
    have to change: FmFormShell


      Rev 1.38   05 Aug 1999 19:25:16   JP
   have to change: FmFormShell

      Rev 1.37   05 May 1999 11:23:30   OS
   #63790# Tabellen-Update nur bei Bedarf rufen

      Rev 1.36   08 Mar 1999 13:39:02   MA
   #62792# OLE-Printer-Benachrichtigung vervollstaendigt

      Rev 1.35   08 Feb 1999 12:51:58   OS
   #61377# falsche DBG_ERRORs entsorgt

      Rev 1.34   02 Feb 1999 13:16:28   JP
   Bug #61335#: Schnistelle zu SwView geaendert

      Rev 1.33   21 Oct 1998 11:40:00   OS
   #58243# Objectbar aus der richtigen Konfiguration lesen

      Rev 1.32   30 Jun 1998 13:02:08   MA
   das war wohl grad etwas zu gruendlich

      Rev 1.30   09 Jun 1998 15:33:32   OM
   VC-Controls entfernt

      Rev 1.29   08 Jun 1998 18:09:42   OM
   Forms im HTML-Mode

      Rev 1.28   12 May 1998 19:00:54   MA
   schon mal Praeventiv die FormShell pushen

      Rev 1.27   02 Apr 1998 10:07:18   MA
   new: ToolbarCfg

      Rev 1.26   11 Feb 1998 10:34:04   MA
   #47035# und CopyCtor entfernt

      Rev 1.25   16 Dec 1997 12:11:26   OS
   Impl-Pointer fuer UNO

      Rev 1.24   28 Nov 1997 14:46:38   MA
   includes

      Rev 1.23   21 Nov 1997 11:33:20   MA
   includes

      Rev 1.22   03 Nov 1997 14:00:14   MA
   precomp entfernt

      Rev 1.21   18 Sep 1997 14:02:04   OS
   Pointer::GetPosPixel fuer VCL ersetzt

      Rev 1.20   05 Sep 1997 12:36:00   MH
   chg: header

      Rev 1.19   03 Sep 1997 14:52:36   MBA
   Neue WebShells

      Rev 1.18   03 Sep 1997 10:51:30   MBA
   swslots.hxx statt wswslots.hxx

      Rev 1.17   05 Aug 1997 16:41:48   TJ
   include svx/srchitem.hxx

      Rev 1.16   08 Jul 1997 12:08:06   OM
   Draw-Selektionsmodi aufgeraeumt

      Rev 1.15   25 Jun 1997 13:35:52   OM
   #40966# DrawBaseShell-Ptr wieder eingefuehrt

      Rev 1.14   19 Jun 1997 16:39:24   MA
   fix: reste vom DrawText-Umbau

      Rev 1.13   18 Jun 1997 17:27:36   HJS
   alte segmentierung raus

      Rev 1.12   17 Jun 1997 15:43:12   MA
   DrawTxtShell nicht von BaseShell ableiten + Opts

      Rev 1.11   14 Apr 1997 16:18:00   OS
   Search/ReplaceAttributes

      Rev 1.10   10 Apr 1997 10:18:58   TRI
   includes

      Rev 1.9   09 Apr 1997 08:15:38   MA
   includes

      Rev 1.8   23 Feb 1997 15:06:26   OS
   GetIcon ueberladen

      Rev 1.7   03 Feb 1997 15:03:36   MA
   chg: ListShell nach unten

      Rev 1.6   31 Jan 1997 13:35:16   OS
   neu: SwWebFrameShell

      Rev 1.5   30 Jan 1997 11:07:24   OS
   erweiterter View-Ctor

      Rev 1.4   29 Jan 1997 12:52:18   OS
   SelectShell: DrawCtrlShell nachgetragen

      Rev 1.3   24 Jan 1997 14:54:28   OS
   RID_WEBTOOLS_TOOLBOX

      Rev 1.2   23 Jan 1997 17:37:24   OS
   GetDispatcher nicht an der SwView aufrufen

      Rev 1.1   23 Jan 1997 17:25:20   OS
   Cast fuer SetDrawBaseShell

      Rev 1.0   23 Jan 1997 16:17:26   OS
   Initial revision.

------------------------------------------------------------------------*/



