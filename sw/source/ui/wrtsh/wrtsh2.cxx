/*************************************************************************
 *
 *  $RCSfile: wrtsh2.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: jp $ $Date: 2000-11-13 10:55:48 $
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
    ASSERT( rURL.Len() && pVSh, "was soll hier geladen werden?" );
    if( !rURL.Len() || !pVSh )
        return ;

    // die Shell kann auch 0 sein !!!!!
    SwWrtShell *pSh = 0;
    if ( pVSh && pVSh->ISA(SwCrsrShell) )
    {
        //Eine CrsrShell ist auch immer eine WrtShell
        pSh = (SwWrtShell*)pVSh;
    }
    else
        return;

    String sTargetFrame;
    if( pTargetFrameName && pTargetFrameName->Len() )
        sTargetFrame = *pTargetFrameName;
    else if( pSh )
        sTargetFrame = pSh->GetDoc()->GetInfo()->GetDefaultTarget();

    String sReferer;
    SwDocShell* pDShell = pSh->GetView().GetDocShell();
    if( pDShell && pDShell->GetMedium() )
        sReferer = pDShell->GetMedium()->GetName();
    SfxViewFrame* pViewFrm = pSh->GetView().GetViewFrame();
    SfxFrameItem aView( SID_DOCFRAME, pViewFrm );
    SfxStringItem aName( SID_FILE_NAME, rURL );
    SfxStringItem aTargetFrameName( SID_TARGETNAME, sTargetFrame );
    SfxStringItem aReferer( SID_REFERER, sReferer );

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

    const SfxPoolItem* aArr[] = {
                &aName, &aFilter,
                &aNewView, /*&aSilent,*/ &aReadOnly,
                &aReferer,
                &aView, &aTargetFrameName,
                0L
    };

    pViewFrm->GetDispatcher()->GetBindings()->Execute( SID_OPENDOC, aArr,
            SFX_CALLMODE_ASYNCHRON|SFX_CALLMODE_RECORD );
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


