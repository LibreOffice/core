/*************************************************************************
 *
 *  $RCSfile: wrtsh2.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:53 $
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

#ifndef _HINTIDS_HXX
#include <hintids.hxx>      // define ITEMIDs
#endif

#ifndef _SFXMACITEM_HXX //autogen
#include <svtools/macitem.hxx>
#endif
#ifndef _SFXFRAME_HXX //autogen
#include <sfx2/frame.hxx>
#endif
#ifndef _SFXDOCINF_HXX //autogen
#include <sfx2/docinf.hxx>
#endif
#ifndef _SFXINIMGR_HXX //autogen
#include <svtools/iniman.hxx>
#endif
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX
#include <svtools/stritem.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif
#ifndef _SFX_FCONTNR_HXX //autogen
#include <sfx2/fcontnr.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _LINKMGR_HXX //autogen
#include <so3/linkmgr.hxx>
#endif

#ifndef _FMTINFMT_HXX //autogen
#include <fmtinfmt.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>      // SET_CURR_SHELL
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>       // Felder
#endif
#ifndef _EXPFLD_HXX
#include <expfld.hxx>
#endif
#ifndef _DDEFLD_HXX
#include <ddefld.hxx>
#endif
#ifndef _DOCUFLD_HXX
#include <docufld.hxx>
#endif
#ifndef _REFFLD_HXX
#include <reffld.hxx>
#endif
#ifndef _SWUNDO_HXX
#include <swundo.hxx>
#endif
#ifndef _NUMRULE_HXX
#include <numrule.hxx>      // Numerierung
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _INPDLG_HXX
#include <inpdlg.hxx>       // Eingabefelder
#endif
#ifndef _VIEWOPT_HXX
#include <viewopt.hxx>      // SwViewOptions
#endif
#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>       // fuer UpdateTable
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>      // fuer UpdateTable
#endif
#ifndef _HINTS_HXX
#include <hints.hxx>        // fuer UpdateTable
#endif
#ifndef _MDIEXP_HXX
#include <mdiexp.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _SWEVENT_HXX
#include <swevent.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _SECTION_HXX
#include <section.hxx>
#endif
#ifndef _NAVICONT_HXX
#include <navicont.hxx>
#endif
#ifndef _NAVIPI_HXX
#include <navipi.hxx>
#endif

#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _WRTSH_HRC
#include <wrtsh.hrc>
#endif


void lcl_LoadUrl( const String& rURL, const String& rTargetFrameName,
                const String& rReferer, USHORT nFilter, SfxFrameItem& );


/*------------------------------------------------------------------------
        Beschreibung:
------------------------------------------------------------------------*/

void SwWrtShell::Insert(SwField &rFld)
{
    ResetCursorStack();
    if(!_CanInsert())
        return;
    StartAllAction();
    StartUndo(UNDO_INSERT);

//JP 09.06.97: Wozu die Abpruefung des FeldTypen??
//          Wobei das SETREFFLD kein Feld ist und hier garnicht durchkommt!
//  USHORT nType = rFld.GetTyp()->Which();
//  if(HasSelection() && nType != RES_HIDDENTXTFLD && nType != RES_SETREFFLD)
    if( HasSelection() )
        DelRight();

    SwEditShell::Insert(rFld);
    EndUndo(UNDO_INSERT);
    EndAllAction();
}

/*--------------------------------------------------------------------
    Beschreibung: Felder Update anschmeissen
 --------------------------------------------------------------------*/



void SwWrtShell::UpdateInputFlds( SwInputFieldList* pLst, BOOL bOnlyInSel )
{
    // ueber die Liste der Eingabefelder gehen und Updaten
    SwInputFieldList* pTmp = pLst;
    if( !pTmp )
        pTmp = new SwInputFieldList( this );

    if (bOnlyInSel)
        pTmp->RemoveUnselectedFlds();

    pTmp->PushCrsr();

    const USHORT nCnt = pTmp->Count();
    BOOL bCancel = FALSE;
    for( USHORT i = 0; i < nCnt && !bCancel; ++i )
    {
        pTmp->GotoFieldPos( i );
        bCancel = StartInputFldDlg( pTmp->GetField( i ), TRUE );

        // Sonst Updatefehler bei Multiselektion:
        pTmp->GetField( i )->GetTyp()->UpdateFlds();
    }
    pTmp->PopCrsr();

    if( !pLst )
        delete pTmp;
}


/*--------------------------------------------------------------------
    Beschreibung: EingabeDialog fuer ein bestimmtes Feld starten
 --------------------------------------------------------------------*/



BOOL SwWrtShell::StartInputFldDlg( SwField* pFld, BOOL bNextButton )
{
//JP 14.08.96: Bug 30332 - nach Umbau der modularietaet im SFX, muss jetzt
//              das TopWindow der Application benutzt werden.
//  SwFldInputDlg* pDlg = new SwFldInputDlg( GetWin(), *this, pFld );

    SwFldInputDlg* pDlg = new SwFldInputDlg( NULL, *this, pFld, bNextButton );
    BOOL bRet = RET_CANCEL == pDlg->Execute();

    delete pDlg;
    GetWin()->Update();
    return bRet;
}



/*--------------------------------------------------------------------
    Beschreibung: Verzeichnis einfuegen Selektion loeschen
 --------------------------------------------------------------------*/



void SwWrtShell::InsertTableOf(const SwTOXBase& rTOX, const SfxItemSet* pSet)
{
    if(!_CanInsert())
        return;

    if(HasSelection())
        DelRight();

    SwEditShell::InsertTableOf(rTOX, pSet);
}


/*--------------------------------------------------------------------
    Beschreibung: Verzeichnis Updaten Selektion loeschen
 --------------------------------------------------------------------*/

BOOL SwWrtShell::UpdateTableOf(const SwTOXBase& rTOX, const SfxItemSet* pSet)
{
    if(!_CanInsert())
        return FALSE;

    return SwEditShell::UpdateTableOf(rTOX, pSet);
}

    // ein Klick aus das angegebene Feld. Der Cursor steht auf diesem.
    // Fuehre die vor definierten Aktionen aus.


void SwWrtShell::ClickToField( const SwField& rFld, USHORT nFilter )
{
    bIsInClickToEdit = TRUE;
    switch( rFld.GetTyp()->Which() )
    {
    case RES_JUMPEDITFLD:
        {
            USHORT nSlotId = 0;
            switch( rFld.GetFormat() )
            {
            case JE_FMT_TABLE:
                nSlotId = FN_INSERT_TABLE;
                break;

            case JE_FMT_FRAME:
                nSlotId = FN_INSERT_FRAME;
                break;

            case JE_FMT_GRAPHIC:    nSlotId = SID_INSERT_GRAPHIC;       break;
            case JE_FMT_OLE:        nSlotId = SID_INSERT_OBJECT;        break;

//          case JE_FMT_TEXT:
            }

            Right( TRUE );      // Feld selektieren

            if( nSlotId )
            {
                StartUndo( UNDO_START );
                GetView().GetViewFrame()->GetDispatcher()->Execute( nSlotId,
                            SFX_CALLMODE_SYNCHRON|SFX_CALLMODE_RECORD );
                EndUndo( UNDO_END );
            }
        }
        break;

    case RES_MACROFLD:
        {
            const SwMacroField *pFld = (const SwMacroField*)&rFld;
            String sLibName(pFld->GetLibName());
            String sMacroName(pFld->GetMacroName());

            if( sLibName.Len() && sMacroName.Len() )
            {
                String sText( rFld.GetPar2() );
                String sRet( sText );
                SvxMacro aMacro( sMacroName, sLibName, STARBASIC );
                ExecMacro( aMacro, &sRet );

                // return Wert veraendert?
                if( sRet != sText )
                {
                    StartAllAction();
                    ((SwField&)rFld).SetPar2( sRet );
                    ((SwField&)rFld).GetTyp()->UpdateFlds();
                    EndAllAction();
                }
            }
        }
        break;

    case RES_GETREFFLD:
        StartAllAction();
        SwCrsrShell::GotoRefMark( ((SwGetRefField&)rFld).GetSetRefName(),
                                    ((SwGetRefField&)rFld).GetSubType(),
                                    ((SwGetRefField&)rFld).GetSeqNo() );
        EndAllAction();
        break;

    case RES_INPUTFLD:
        StartInputFldDlg( (SwField*)&rFld, FALSE );
        break;

    case RES_SETEXPFLD:
        if( ((SwSetExpField&)rFld).GetInputFlag() )
            StartInputFldDlg( (SwField*)&rFld, FALSE );
        break;
    }

    bIsInClickToEdit = FALSE;
}



void SwWrtShell::ClickToINetAttr( const SwFmtINetFmt& rItem, USHORT nFilter )
{
    if( !rItem.GetValue().Len() )
        return ;

    bIsInClickToEdit = TRUE;

    // erstmal das evt. gesetzte ObjectSelect Macro ausfuehren
    const SvxMacro* pMac = rItem.GetMacro( SFX_EVENT_MOUSECLICK_OBJECT );
    if( pMac )
    {
        SwCallMouseEvent aCallEvent;
        aCallEvent.Set( &rItem );
        GetDoc()->CallEvent( SFX_EVENT_MOUSECLICK_OBJECT, aCallEvent, FALSE );
    }

    // damit die Vorlagenumsetzung sofort angezeigt wird
    ::LoadURL( rItem.GetValue(), this, nFilter, &rItem.GetTargetFrame() );
    bIsInClickToEdit = FALSE;
}



BOOL SwWrtShell::ClickToINetGrf( const Point& rDocPt, USHORT nFilter )
{
    BOOL bRet = FALSE;
    String sURL;
    String sTargetFrameName;
    const SwFrmFmt* pFnd = IsURLGrfAtPos( rDocPt, &sURL, &sTargetFrameName );
    if( pFnd && sURL.Len() )
    {
        bRet = TRUE;
        // erstmal das evt. gesetzte ObjectSelect Macro ausfuehren
        const SvxMacro* pMac = &pFnd->GetMacro().GetMacro( SFX_EVENT_MOUSECLICK_OBJECT );
        if( pMac )
        {
            SwCallMouseEvent aCallEvent;
            aCallEvent.Set( EVENT_OBJECT_URLITEM, pFnd );
            GetDoc()->CallEvent( SFX_EVENT_MOUSECLICK_OBJECT, aCallEvent, FALSE );
        }

        ::LoadURL( sURL, this, nFilter, &sTargetFrameName);
    }
    return bRet;
}


void LoadURL( const String& rURL, ViewShell* pVSh, USHORT nFilter,
              const String *pTargetFrameName )
{
    ASSERT( rURL.Len(), "was soll hier geladen werden?" );
    if( !rURL.Len() )
        return ;

    // die Shell kann auch 0 sein !!!!!
    SwWrtShell *pSh = 0;
    if ( pVSh && pVSh->ISA(SwCrsrShell) )
    {
        //Eine CrsrShell ist auch immer eine WrtShell
        pSh = (SwWrtShell*)pVSh;
    }

    String sTargetFrame;
    if( pTargetFrameName && pTargetFrameName->Len() )
        sTargetFrame = *pTargetFrameName;
    else if( pSh )
        sTargetFrame = pSh->GetDoc()->GetInfo()->GetDefaultTarget();

    String sReferer;
    SfxFrame* pViewFrm = 0;
    if( pSh )
    {
        SwDocShell* pDShell = pSh->GetView().GetDocShell();
        if( pDShell && pDShell->GetMedium() )
            sReferer = pDShell->GetMedium()->GetName();
        pViewFrm = pSh->GetView().GetViewFrame()->GetFrame();
    }

    SfxFrameItem aView( SID_DOCFRAME, pViewFrm );
    ::lcl_LoadUrl( rURL, sTargetFrame, sReferer, nFilter,
                    aView );
}

void lcl_LoadUrl( const String& rURL, const String& rTargetFrameName,
                    const String& rReferer, USHORT nFilter,
                    SfxFrameItem& rView )
{
    SfxStringItem aName( SID_FILE_NAME, rURL );
    SfxStringItem aTargetFrameName( SID_TARGETNAME, rTargetFrameName );
    SfxStringItem aReferer( SID_REFERER, rReferer );

    SfxBoolItem aNewView( SID_OPEN_NEW_VIEW, FALSE );
    //#39076# Silent kann lt. SFX entfernt werden.
//  SfxBoolItem aSilent( SID_SILENT, TRUE );
    SfxBoolItem aReadOnly( SID_BROWSING, TRUE );

    SfxStringItem aFilter( SID_FILTER_NAME, aEmptyStr );
    switch( nFilter & ~URLLOAD_NEWVIEW )
    {
    case URLLOAD_CHOOSEFILTER:
        aFilter.SetValue( SfxExecutableFilterContainer::GetChooserFilter()
                            ->GetName() );
        break;
    case URLLOAD_DOWNLOADFILTER:
        aFilter.SetValue( SfxExecutableFilterContainer::GetDownloadFilter()
                            ->GetName() );
        break;
    }

    if( nFilter & URLLOAD_NEWVIEW )
        aTargetFrameName.SetValue( String::CreateFromAscii("_blank") );

    ((SfxViewFrame*)rView.GetFrame())->GetDispatcher()->Execute( SID_OPENDOC,
            SFX_CALLMODE_ASYNCHRON|SFX_CALLMODE_RECORD,
                            &aName,
                            &aFilter,
                            &aNewView, /*&aSilent,*/ &aReadOnly,
                            &aReferer,
                            &rView, &aTargetFrameName,
                            0L );
}

void SwWrtShell::NavigatorPaste( const NaviContentBookmark& rBkmk, const DropEvent* pEvt )
{
    USHORT nRegionMode;
    if(pEvt->IsDefaultAction())
        nRegionMode = rBkmk.GetDefaultDragType();
    else
        switch(pEvt->GetAction())
        {
            case DROP_COPY: nRegionMode = REGION_MODE_NONE; break;
            case DROP_MOVE: nRegionMode = REGION_MODE_EMBEDDED; break;
            default: nRegionMode = REGION_MODE_LINK;
        }
    if(nRegionMode == REGION_MODE_NONE)
    {
        // Einfuegen
        String sURL = rBkmk.GetURL();
        //handelt es sich um ein Sprung innerhalb des akt. Docs?
        const SwDocShell* pDocShell = GetView().GetDocShell();
        if(pDocShell->HasName())
        {
            USHORT nPos = 0;
            const String rName = URIHelper::SmartRelToAbs(
                                    pDocShell->GetMedium()->GetURLObject().GetURLNoMark());

            if(COMPARE_EQUAL == sURL.CompareTo(rName, rName.Len()))
                sURL.Erase(0, rName.Len());
        }
        SwFmtINetFmt aFmt( sURL, aEmptyStr );
        InsertURL( aFmt, rBkmk.GetDescription() );
    }
    else
    {
        SwSection aSection( FILE_LINK_SECTION, GetUniqueSectionName( 0 ) );
        String aLinkFile( rBkmk.GetURL().GetToken(0, '#') );
        aLinkFile += cTokenSeperator;
        aLinkFile += cTokenSeperator;
        aLinkFile += rBkmk.GetURL().GetToken(1, '#');
        aSection.SetLinkFileName( aLinkFile );
        aSection.SetProtect( TRUE );
        const SwSection* pIns = InsertSection( aSection );
        if( REGION_MODE_EMBEDDED == nRegionMode && pIns )
        {
            aSection = *pIns;
            aSection.SetLinkFileName( aEmptyStr );
            aSection.SetType( CONTENT_SECTION );
            aSection.SetProtect( FALSE );

            // the update of content from linked section at time delete
            // the undostack. Then the change of the section dont create
            // any undoobject. -  BUG 69145
            BOOL bDoesUndo = DoesUndo();
            if( UNDO_INSSECTION != GetUndoIds() )
                DoUndo( FALSE );
            ChgSection( GetSectionFmtPos( *pIns->GetFmt() ), aSection );
            DoUndo( bDoesUndo );
        }
    }
}

/*************************************************************************

      $Log: not supported by cvs2svn $
      Revision 1.137  2000/09/18 16:06:27  willem.vandorp
      OpenOffice header added.

      Revision 1.136  2000/09/07 15:59:36  os
      change: SFX_DISPATCHER/SFX_BINDINGS removed

      Revision 1.135  2000/08/31 11:31:37  jp
      add missing include

      Revision 1.134  2000/06/26 13:04:50  os
      INetURLObject::SmartRelToAbs removed

      Revision 1.133  2000/04/19 11:18:25  os
      UNICODE

      Revision 1.132  2000/03/03 12:29:37  mib
      Removed JavaScript

      Revision 1.131  2000/02/11 15:03:35  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.130  1999/10/15 09:58:25  jp
      Bug #69145#: NavigatorPaste: copy embedded Section without undo

      Revision 1.129  1999/08/17 11:59:18  OS
      extended indexes: get/set section attributes


      Rev 1.128   17 Aug 1999 13:59:18   OS
   extended indexes: get/set section attributes

      Rev 1.127   27 Apr 1999 16:50:36   JP
   Bug #65314#: neu: IsJavaScriptEnabled

      Rev 1.126   19 Apr 1999 13:00:42   OS
   #64960# GetURLObject()->GetURLNoMark statt GetPhysicalName

      Rev 1.125   15 Apr 1999 16:48:32   JP
   Bug #64841#: LoadURL: URL-Attrs ohne URL ignorieren

      Rev 1.124   12 Oct 1998 10:01:38   OM
   #57790# Dialog nicht mehr AppModal

      Rev 1.123   01 Jul 1998 16:37:48   JP
   Bug #51378#: Return String von JavaScript auswerten

      Rev 1.122   19 Jun 1998 18:57:48   JP
   Bug #51378#: LoadURL CallbackHdl - JavaReturnWert auswerten

      Rev 1.121   19 May 1998 12:45:06   OM
   SvxMacro-Umstellung

      Rev 1.120   13 Feb 1998 17:46:50   HJS
   falsche ifdef SOLAR_JAVE aufgeloest

      Rev 1.119   02 Feb 1998 15:04:18   OM
   #46781# Macrofelder auch ausfuehren, wenn Dateiname mehr als 1 Punkt enthaelt

      Rev 1.118   02 Dec 1997 14:21:40   MA
   includes

      Rev 1.117   24 Nov 1997 14:35:08   MA
   includes

      Rev 1.116   03 Nov 1997 14:02:56   MA
   precomp entfernt

      Rev 1.115   24 Oct 1997 18:30:08   JP
   ClickToINetGrf: BOOL returnen - ob wirklich eine Grafik getroffen wurde

      Rev 1.114   01 Sep 1997 13:22:00   OS
   DLL-Umstellung

      Rev 1.113   15 Aug 1997 12:15:42   OS
   chartar/frmatr/txtatr aufgeteilt

      Rev 1.112   11 Aug 1997 10:54:18   OS
   paraitem/frmitems/textitem aufgeteilt

      Rev 1.111   08 Aug 1997 17:28:40   OM
   Headerfile-Umstellung

      Rev 1.110   07 Aug 1997 14:59:36   OM
   Headerfile-Umstellung

      Rev 1.109   05 Aug 1997 13:40:10   MH
   chg: header

      Rev 1.108   23 Jul 1997 21:46:28   HJS
   includes

      Rev 1.107   23 Jul 1997 10:19:04   OM
   Inputfields in Selektion updaten

      Rev 1.106   27 Jun 1997 17:27:06   JP
   Bug #38115#: nicht nur Top sondern auch den akt. Frame merken

      Rev 1.105   26 Jun 1997 15:59:24   OS
   BulletOn jetzt in wrtsh1.cxx

      Rev 1.104   09 Jun 1997 16:21:08   JP
   ClickINetURL/ClickToINetGrf: falls JavaScript abgeschaltet ist LoadURL trotzdem rufen

      Rev 1.103   09 Jun 1997 15:28:04   NF
   Define raus, da SfxModalDialog unbekannt

      Rev 1.102   09 Jun 1997 11:47:46   JP
   InsertFld: Abpruefung auf den FeldTypen entfernt - ueberflussig

      Rev 1.101   23 Apr 1997 14:44:36   OS
   FrameItem anlegen fuer VA3.0

      Rev 1.100   21 Apr 1997 18:22:12   MA
   #39076# kein Silent

      Rev 1.99   19 Apr 1997 09:39:32   OS
   NavigatorPaste: GetPhysicalName am Medium!

      Rev 1.98   15 Apr 1997 15:12:38   JP
   Bug #37980#: LoadURL - bei Events mit JavaScript immer asynchron callen

      Rev 1.97   11 Apr 1997 08:48:52   MA
   includes

      Rev 1.96   09 Apr 1997 15:34:40   JP
   ClickToINetGrf/ClickToINetAttr - handelt das asynchron JavaScript und Laden einer URL

      Rev 1.95   07 Apr 1997 13:47:12   MH
   chg: header

      Rev 1.94   20 Mar 1997 08:02:40   OS
   Bookmark nicht mit GetToken(#), sondern mit Compare behandeln

      Rev 1.93   18 Feb 1997 23:29:30   NF
   SID_FILE_CONVERT wech...

      Rev 1.92   18 Feb 1997 23:26:56   NF
   SID_FILE_CONVERT wech...

      Rev 1.91   12 Feb 1997 18:22:32   MA
   chg: LoadURL, Historie auch bei newview erhalten (kopieren)

      Rev 1.90   07 Feb 1997 12:04:18   OS
   Navigator benutzt eigenes Drag-Format

      Rev 1.89   28 Jan 1997 14:11:10   JP
   ClickToINet: CallEvent statt ExecMac rufen

      Rev 1.88   16 Jan 1997 09:59:46   MA
   Umstellung Frame

      Rev 1.87   15 Jan 1997 16:09:00   OM
   Neue OLE-Dialoge

      Rev 1.86   04 Dec 1996 15:11:22   JP
   SW_EVENT -> SVX_EVENT/SFX_EVENT

      Rev 1.85   28 Nov 1996 17:15:40   OS
   ChildWindow testen

      Rev 1.84   19 Nov 1996 16:15:26   OS
   Navigatorumstellung

      Rev 1.83   06 Nov 1996 10:54:20   MA
   chg: URL per Dokument; Draw+Calc eingebunden (ifdef)

      Rev 1.82   05 Nov 1996 15:34:12   JP
   GotoRefMark: Parameter erweitert fuer erweiterte RefMarks

      Rev 1.81   17 Oct 1996 14:45:42   MH
   Syntax

      Rev 1.80   17 Oct 1996 14:37:08   MH
   add: include

      Rev 1.79   11 Oct 1996 14:52:58   NF
   clooks

      Rev 1.78   25 Sep 1996 10:36:46   PL
   IRIX

      Rev 1.77   05 Sep 1996 16:53:20   OS
   Tasten fuer NavigatorPaste auswerten

      Rev 1.76   02 Sep 1996 18:43:22   JP
   INetFeld entfernt

      Rev 1.75   30 Aug 1996 12:40:20   OS
   InputFldDlg mit Next-Button

      Rev 1.74   29 Aug 1996 09:26:02   OS
   includes

      Rev 1.73   19 Aug 1996 22:22:24   JP
   LoadURL: neue View darf auch keinen TargetNamen haben

      Rev 1.72   14 Aug 1996 16:54:54   JP
   Bug #30332#: am FieldInputWin das TopWindow der Applikation setzen

      Rev 1.71   14 Aug 1996 09:35:02   JP
   neu: NavigatorPaste - Paste aus dem Navigator ins Doc (Code aus dataex hierher verschoben)

      Rev 1.70   12 Aug 1996 18:10:46   JP
   LoadURL: das ViewItem darf nicht uebergeben werden

      Rev 1.69   12 Aug 1996 16:56:36   JP
   neues FilterFlag: neue Ansicht beim LoadURL oeffnen

      Rev 1.68   08 Aug 1996 10:04:02   JP
   neu: ClickToINetAttr - entspricht dem ClickToFld

      Rev 1.67   29 Jul 1996 11:06:10   JP
   ueberfluessige Methode entfernt

      Rev 1.66   23 Jul 1996 19:45:24   MIB
   Default-Target-Frame aus Dokinfo holen

      Rev 1.65   19 Jul 1996 15:38:12   JP
   Umstellung Numerierung

      Rev 1.64   26 Jun 1996 15:22:58   OS
   Aufruf von Dispatcher.Execute an 324 angepasst

*************************************************************************/


