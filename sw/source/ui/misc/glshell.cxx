/*************************************************************************
 *
 *  $RCSfile: glshell.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:44 $
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

#ifndef _LIST_HXX //autogen
#include <tools/list.hxx>
#endif
#ifndef _SFXENUMITEM_HXX //autogen
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _SFX_PRINTER_HXX //autogen
#include <sfx2/printer.hxx>
#endif
#ifndef _SFXREQUEST_HXX //autogen
#include <sfx2/request.hxx>
#endif
#ifndef _SFXSIDS_HRC //autogen
#include <sfx2/sfxsids.hrc>
#endif
#ifndef _SVX_SRCHITEM_HXX
#include <svx/srchitem.hxx>
#endif
#ifndef _OFAACCFG_HXX //autogen
#include <offmgr/ofaaccfg.hxx>
#endif
#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif
#ifndef _SFXMACITEM_HXX
#include <svtools/macitem.hxx>
#endif
#ifndef _GLOSHDL_HXX
#include <gloshdl.hxx>
#endif

#ifndef _UITOOL_HXX
#include <uitool.hxx>
#endif
#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _VIEW_HXX
#include <view.hxx>
#endif
#ifndef _GLSHELL_HXX
#include <glshell.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _GLOSDOC_HXX
#include <glosdoc.hxx>
#endif
#ifndef _SHELLIO_HXX
#include <shellio.hxx>
#endif
#ifndef _INITUI_HXX
#include <initui.hxx>                   // fuer ::GetGlossaries()
#endif
#ifndef _CMDID_H
#include <cmdid.h>
#endif
#ifndef _SWERROR_H
#include <swerror.h>
#endif
#ifndef _MISC_HRC
#include <misc.hrc>
#endif


#define SwWebGlosDocShell
#define SwGlosDocShell
#ifndef _ITEMDEF_HXX
#include <itemdef.hxx>
#endif
#ifndef _SWSLOTS_HXX
#include <swslots.hxx>
#endif


SFX_IMPL_INTERFACE( SwGlosDocShell, SwDocShell, SW_RES(0) )
{
}


SFX_IMPL_INTERFACE( SwWebGlosDocShell, SwWebDocShell, SW_RES(0) )
{
}


TYPEINIT1( SwGlosDocShell, SwDocShell );
TYPEINIT1( SwWebGlosDocShell, SwWebDocShell );


void lcl_Execute( SwDocShell& rSh, SfxRequest& rReq )
{
    if ( rReq.GetSlot() == SID_SAVEDOC )
    {
        if( !rSh.HasName() )
        {
            rReq.SetReturnValue( SfxBoolItem( 0, rSh.Save() ) );
        }
        else
        {
            const SfxBoolItem* pRes = ( const SfxBoolItem* )
                                        rSh.ExecuteSlot( rReq,
                                        rSh.SfxObjectShell::GetInterface() );
            if( pRes->GetValue() )
                rSh.GetDoc()->ResetModified();
        }
    }
}


void lcl_GetState( SwDocShell& rSh, SfxItemSet& rSet )
{
    if( SFX_ITEM_AVAILABLE >= rSet.GetItemState( SID_SAVEDOC, FALSE ))
    {
        if( !rSh.GetDoc()->IsModified() )
            rSet.DisableItem( SID_SAVEDOC );
        else
            rSet.Put( SfxStringItem( SID_SAVEDOC, SW_RESSTR(STR_SAVE_GLOSSARY)));
    }
}


BOOL lcl_Save( SwWrtShell& rSh, const String& rGroupName,
                const String& rShortNm, const String& rLongNm )
{
    const OfaAutoCorrCfg* pCfg = OFF_APP()->GetAutoCorrConfig();
    SwTextBlocks * pBlock = ::GetGlossaries()->GetGroupDoc( rGroupName );

    SvxMacro aStart(aEmptyStr, aEmptyStr);
    SvxMacro aEnd(aEmptyStr, aEmptyStr);

    SwGlossaryHdl* pGlosHdl = rSh.GetView().GetGlosHdl();
    pGlosHdl->GetMacros( rShortNm, aStart, aEnd, pBlock );



    USHORT nRet = rSh.SaveGlossaryDoc( *pBlock, rLongNm, rShortNm,
                                pCfg->IsSaveRelFile(), pCfg->IsSaveRelNet(),
                                pBlock->IsOnlyTextBlock( rShortNm ) );

    if(aStart.GetMacName().Len() || aEnd.GetMacName().Len())
    {
        SvxMacro* pStart = aStart.GetMacName().Len() ? &aStart : 0;
        SvxMacro* pEnd = aEnd.GetMacName().Len() ? &aEnd : 0;
        pGlosHdl->SetMacros( rShortNm, pStart, pEnd, pBlock );
    }

    rSh.EnterStdMode();
    if( USHRT_MAX != nRet )
        rSh.ResetModified();
    delete pBlock;
    return nRet != USHRT_MAX;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


SwGlosDocShell::SwGlosDocShell()
    : SwDocShell( SFX_CREATE_MODE_STANDARD )
{
    SetHelpId(SW_GLOSDOCSHELL);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


SwGlosDocShell::~SwGlosDocShell(  )
{
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


void SwGlosDocShell::Execute( SfxRequest& rReq )
{
    ::lcl_Execute( *this, rReq );
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


void SwGlosDocShell::GetState( SfxItemSet& rSet )
{
    ::lcl_GetState( *this, rSet );
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


BOOL SwGlosDocShell::Save()
{
    return ::lcl_Save( *GetWrtShell(), aGroupName, aShortName, aLongName );
}


/**/

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


SwWebGlosDocShell::SwWebGlosDocShell()
    : SwWebDocShell( SFX_CREATE_MODE_STANDARD )
{
    SetHelpId(SW_WEBGLOSDOCSHELL);
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


SwWebGlosDocShell::~SwWebGlosDocShell(  )
{
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


void SwWebGlosDocShell::Execute( SfxRequest& rReq )
{
    ::lcl_Execute( *this, rReq );
}
/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


void SwWebGlosDocShell::GetState( SfxItemSet& rSet )
{
    ::lcl_GetState( *this, rSet );
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


BOOL SwWebGlosDocShell::Save()
{
    return ::lcl_Save( *GetWrtShell(), aGroupName, aShortName, aLongName );
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/


SV_DECL_REF(SwDocShell)
SV_IMPL_REF(SwDocShell)

void SwGlossaries::EditGroupDoc( const String& rGroup, const String& rShortName )
{
    SwTextBlocks* pGroup = GetGroupDoc( rGroup );
    if(pGroup->IsOld())
    {
        pGroup->ConvertToNew();
        if( pGroup->GetError() )
            ErrorHandler::HandleError( pGroup->GetError() );
    }

    if( pGroup && pGroup->GetCount() )
    {
        SwDocShellRef xDocSh;

        // erfrage welche View registriert ist. Im WebWriter gibts es keine
        // normale View
        USHORT nViewId = 0 != &SwView::Factory() ? 2 : 6;
        String sLongName = pGroup->GetLongName(pGroup->GetIndex( rShortName ));

        if( 6 == nViewId )
        {
            xDocSh = new SwWebGlosDocShell();
            SwWebGlosDocShell* pDocSh = (SwWebGlosDocShell*)&xDocSh;
            pDocSh->DoInitNew( 0 );

            pDocSh->SetLongName( sLongName );
            pDocSh->SetShortName( rShortName);
            pDocSh->SetGroupName( rGroup );
        }
        else
        {
            xDocSh = new SwGlosDocShell();
            SwGlosDocShell* pDocSh = (SwGlosDocShell*)&xDocSh;
            pDocSh->DoInitNew( 0 );

            pDocSh->SetLongName( sLongName );
            pDocSh->SetShortName( rShortName );
            pDocSh->SetGroupName( rGroup );
        }

        // Dokumenttitel setzen
        SfxViewFrame* pFrame = SFX_APP()->CreateViewFrame( *xDocSh, nViewId );
        String aDocTitle(SW_RES( STR_GLOSSARY ));
        aDocTitle += sLongName;

        BOOL bDoesUndo = xDocSh->GetDoc()->DoesUndo();
        xDocSh->GetDoc()->DoUndo( FALSE );

        xDocSh->GetWrtShell()->InsertGlossary( *pGroup, rShortName );
        if( !xDocSh->GetDoc()->GetPrt() )
        {
            // wir erzeugen einen default SfxPrinter.
            // Das ItemSet wird vom Sfx geloescht!
            SfxItemSet *pSet = new SfxItemSet( xDocSh->GetDoc()->GetAttrPool(),
                        FN_PARAM_ADDPRINTER, FN_PARAM_ADDPRINTER,
                        SID_PRINTER_NOTFOUND_WARN, SID_PRINTER_NOTFOUND_WARN,
                        SID_PRINTER_CHANGESTODOC, SID_PRINTER_CHANGESTODOC,
                        0 );
            SfxPrinter* pPrinter = new SfxPrinter( pSet );

            // und haengen ihn ans Dokument.
            xDocSh->GetDoc()->SetPrt( pPrinter );
        }

        xDocSh->SetTitle( aDocTitle );
        xDocSh->GetDoc()->DoUndo( bDoesUndo );
        xDocSh->GetDoc()->ResetModified();
        pFrame->GetFrame()->Appear();
        delete pGroup;
    }
}


/*------------------------------------------------------------------------
    $Log: not supported by cvs2svn $
    Revision 1.59  2000/09/18 16:05:57  willem.vandorp
    OpenOffice header added.

    Revision 1.58  2000/05/10 11:53:43  os
    Basic API removed

    Revision 1.57  1999/12/22 09:21:04  os
    #71203# editing of AutoText: keep macros

    Revision 1.56  1999/07/23 08:23:24  OS
    #67828# glossary - methods on doc level


      Rev 1.55   23 Jul 1999 10:23:24   OS
   #67828# glossary - methods on doc level

      Rev 1.54   09 Feb 1999 10:47:04   OS
   #61205# AutoText-Gruppen koennen beliebige Namen erhalten

      Rev 1.53   22 Jan 1999 13:06:14   JP
   Task #61076#: UndoFlag zuruecksetzen

      Rev 1.52   27 Jun 1998 15:59:06   JP
   neu: Textbausteine ohne Formatierung

      Rev 1.51   19 Jan 1998 11:57:42   MBA
   CreateViewFrame verwenden

      Rev 1.50   25 Nov 1997 11:58:26   TJ
   include fuer SfxPrinter

      Rev 1.49   24 Nov 1997 16:47:44   MA
   includes

      Rev 1.48   13 Nov 1997 13:21:16   MH
   chg: header

      Rev 1.47   03 Sep 1997 19:09:14   MBA
   Kein wswslots.hxx mehr includen

      Rev 1.46   01 Sep 1997 13:16:34   OS
   DLL-Umstellung

      Rev 1.45   26 Aug 1997 16:05:10   HJS
   internal comp. err. - header vor-includet

      Rev 1.44   05 Aug 1997 16:14:20   TJ
   include svx/srchitem.hxx

      Rev 1.43   31 Jul 1997 15:22:04   MH
   chg: header

      Rev 1.42   11 Jul 1997 16:35:46   JP
   Bug #41481#: SaveGlossaryDoc - beachtet auch Tabellen am Anfang

      Rev 1.41   07 Apr 1997 16:08:24   MH
   chg: header

      Rev 1.40   12 Feb 1997 12:01:16   JP
   neu: WebGlossryDocShell

      Rev 1.39   11 Nov 1996 11:05:46   MA
   ResMgr

      Rev 1.38   15 Oct 1996 14:13:26   JP
   AutoCorrCfg in die OfficeApplication verschoben

      Rev 1.37   23 Sep 1996 08:07:54   OS
   Statusstring fuer SID_SAVE

      Rev 1.36   28 Aug 1996 14:12:12   OS
   includes

      Rev 1.35   09 Aug 1996 11:08:04   OS
   Appear am Frame aufrufen, damit das Fenster richtige erkannt wird

      Rev 1.34   08 Aug 1996 21:37:50   JP
   Save: das gesamte Doc selektieren, nicht den akt. Bereich

      Rev 1.33   09 Jul 1996 10:11:16   OS
   Autotexte relativ sichern

      Rev 1.32   25 Jun 1996 18:39:36   HJS
   includes

      Rev 1.31   19 Jun 1996 12:29:24   OM
   Umstellung auf 323

      Rev 1.30   27 Mar 1996 09:39:20   OS
   GlosDocShell ueber Ref anlegen

      Rev 1.29   22 Mar 1996 14:06:50   HJS
   umstellung 311

      Rev 1.28   14 Dec 1995 23:00:40   JP
   Filter Umstellung: ErrorHandling und Read/Write-Parameter

      Rev 1.27   03 Dec 1995 11:24:32   OS
   +include itemdef.hxx

      Rev 1.26   29 Nov 1995 13:47:00   OS
   -slotadd.hxx

------------------------------------------------------------------------*/


