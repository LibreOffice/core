/*************************************************************************
 *
 *  $RCSfile: javaedit.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:37 $
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

// include ---------------------------------------------------------------

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#define _JAVAEDIT_CXX

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif

#ifndef _WRTSH_HXX
#include <wrtsh.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif
#ifndef _FLDMGR_HXX
#include <fldmgr.hxx>
#endif
#ifndef _DOCUFLD_HXX
#include <docufld.hxx>
#endif
#ifndef _UITOOL_HXX
#include <uitool.hxx>
#endif
#ifndef _JAVAEDIT_HXX
#include <javaedit.hxx>
#endif

#ifndef _FLDUI_HRC
#include <fldui.hrc>
#endif
#ifndef _JAVAEDIT_HRC
#include <javaedit.hrc>
#endif

// static ----------------------------------------------------------------

// class SwJavaEditDialog ------------------------------------------------



SwJavaEditDialog::SwJavaEditDialog(Window* pParent, SwWrtShell* pWrtSh) :

    SvxStandardDialog(pParent, SW_RES(DLG_JAVAEDIT)),

    aTypeFT         ( this, SW_RES( FT_TYPE ) ),
    aTypeED         ( this, SW_RES( ED_TYPE ) ),
    aUrlRB          ( this, SW_RES( RB_URL ) ),
    aUrlED          ( this, SW_RES( ED_URL ) ),
    aUrlPB          ( this, SW_RES( PB_URL ) ),
    aEditRB         ( this, SW_RES( RB_EDIT ) ),
    aEditED         ( this, SW_RES( ED_EDIT ) ),
    aPostItGB       ( this, SW_RES( GB_POSTIT ) ),
    aOKBtn          ( this, SW_RES( BTN_POST_OK ) ),
    aCancelBtn      ( this, SW_RES( BTN_POST_CANCEL ) ),
    aPrevBtn        ( this, SW_RES( BTN_PREV ) ),
    aNextBtn        ( this, SW_RES( BTN_NEXT ) ),
    aHelpBtn        ( this, SW_RES( BTN_POST_HELP ) ),

    pSh(pWrtSh),
    bNew(TRUE)
{
    // Handler installieren
    aPrevBtn.SetClickHdl( LINK( this, SwJavaEditDialog, PrevHdl ) );
    aNextBtn.SetClickHdl( LINK( this, SwJavaEditDialog, NextHdl ) );
    aOKBtn.SetClickHdl( LINK( this, SwJavaEditDialog, OKHdl ) );

    Link aLk = LINK(this, SwJavaEditDialog, RadioButtonHdl);
    aUrlRB.SetClickHdl(aLk);
    aEditRB.SetClickHdl(aLk);
    aUrlPB.SetClickHdl(LINK(this, SwJavaEditDialog, InsertFileHdl));

    Font aFont( aEditED.GetFont() );
    aFont.SetWeight( WEIGHT_LIGHT );
    aEditED.SetFont( aFont );

    pMgr = new SwFldMgr;
    pFld = (SwScriptField*)pMgr->GetCurFld();

    bNew = !(pFld && pFld->GetTyp()->Which() == RES_SCRIPTFLD);

    CheckTravel();

    if( !bNew )
        SetText( SW_RES( STR_JAVA_EDIT ) );
    else
        // neu anlegen
        SetText( SW_RES( STR_JAVA_INSERT ) );

    FreeResource();

    RadioButtonHdl(NULL);
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/



SwJavaEditDialog::~SwJavaEditDialog()
{
    delete pMgr;
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/



IMPL_LINK_INLINE_START( SwJavaEditDialog, PrevHdl, Button *, EMPTYARG )
{
    SetFld();
    pMgr->GoPrev();
    pFld = (SwScriptField*)pMgr->GetCurFld();
    CheckTravel();
    RadioButtonHdl(NULL);

    return 0;
}
IMPL_LINK_INLINE_END( SwJavaEditDialog, PrevHdl, Button *, EMPTYARG )

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/



IMPL_LINK_INLINE_START( SwJavaEditDialog, NextHdl, Button *, EMPTYARG )
{
    SetFld();
    pMgr->GoNext();
    pFld = (SwScriptField*)pMgr->GetCurFld();
    CheckTravel();
    RadioButtonHdl(NULL);

    return 0;
}
IMPL_LINK_INLINE_END( SwJavaEditDialog, NextHdl, Button *, EMPTYARG )

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/



IMPL_LINK( SwJavaEditDialog, OKHdl, Button *, EMPTYARG )
{
    SetFld();
    EndDialog( RET_OK );
    return 0;
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/



void SwJavaEditDialog::Apply()
{
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/



void SwJavaEditDialog::CheckTravel()
{
    BOOL bTravel = FALSE;
    BOOL bNext, bPrev;

    if(!bNew)
    {
        // Traveling nur bei mehr als einem Feld
        pSh->StartAction();
        pSh->CreateCrsr();

        bNext = pMgr->GoNext();
        if( bNext )
            pMgr->GoPrev();

        if( 0 != ( bPrev = pMgr->GoPrev() ) )
            pMgr->GoNext();
        bTravel |= bNext|bPrev;

        pSh->DestroyCrsr();
        pSh->EndAction();

        if (pFld->IsCodeURL())
        {
            aUrlED.SetText(pFld->GetPar2());
            aEditED.SetText(aEmptyStr);
            aUrlRB.Check();
        }
        else
        {
            aEditED.SetText(pFld->GetPar2());
            aUrlED.SetText(aEmptyStr);
            aEditRB.Check();
        }
        aTypeED.SetText(pFld->GetPar1());
    }

    if ( !bTravel )
    {
        aPrevBtn.Hide();
        aNextBtn.Hide();
    }
    else
    {
        aPrevBtn.Enable(bPrev);
        aNextBtn.Enable(bNext);
    }
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/



void SwJavaEditDialog::SetFld()
{
    if( !aOKBtn.IsEnabled() )
        return ;

    String sText;
    String sType( aTypeED.GetText() );
    BOOL bIsUrl = aUrlRB.IsChecked();

    if( bIsUrl )
    {
        sText = aUrlED.GetText();
        if(sText.Len())
            sText = URIHelper::SmartRelToAbs(sText);
    }
    else
        sText = aEditED.GetText();

    if( !sType.Len() )
        sType = String::CreateFromAscii("JavaScript");

    if( bNew )
        pMgr->InsertFld(TYP_SCRIPTFLD, 0, sType, sText, bIsUrl);
    else if( bIsUrl != pFld->GetFormat() ||
             pFld->GetPar2() != sType || pFld->GetPar1() != sText )
    {
        pMgr->UpdateCurFld( bIsUrl, sType, sText );
        pSh->SetUndoNoResetModified();
    }
}

/*------------------------------------------------------------------------
 Beschreibung:
------------------------------------------------------------------------*/

IMPL_LINK( SwJavaEditDialog, RadioButtonHdl, RadioButton *, pBtn )
{
    BOOL bEnable = aUrlRB.IsChecked();
    aUrlPB.Enable(bEnable);
    aUrlED.Enable(bEnable);
    aEditED.Enable(!bEnable);

    if( !bNew )
    {
        bEnable = !pSh->IsReadOnlyAvailable() || !pSh->HasReadonlySel();
        aOKBtn.Enable( bEnable );
        aUrlED.SetReadOnly( !bEnable );
        aEditED.SetReadOnly( !bEnable);
        aTypeED.SetReadOnly( !bEnable);
        if( aUrlPB.IsEnabled() && !bEnable )
            aUrlPB.Enable( FALSE );
    }
    return 0;
}

/***************************************************************************
    Beschreibung:
***************************************************************************/

IMPL_LINK( SwJavaEditDialog, InsertFileHdl, PushButton *, pBtn )
{
    String sFileName;
    if( GetFileFilterNameDlg( *pBtn, sFileName ))
        aUrlED.SetText( sFileName );
    return 0;
}

/*************************************************************************

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.13  2000/09/18 16:05:31  willem.vandorp
      OpenOffice header added.

      Revision 1.12  2000/06/26 13:20:15  os
      INetURLObject::SmartRelToAbs removed

      Revision 1.11  2000/04/18 15:17:32  os
      UNICODE

      Revision 1.10  2000/02/11 14:46:57  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.9  1999/01/20 15:56:58  JP
      Task #58677#: Crsr in Readonly Bereichen zulassen


      Rev 1.8   20 Jan 1999 16:56:58   JP
   Task #58677#: Crsr in Readonly Bereichen zulassen

      Rev 1.7   19 Aug 1998 17:22:14   JP
   Bug #55247#: fuers MoveNext-/-PrevFldType einen eigenen Pam verwenden

      Rev 1.6   13 Feb 1998 14:16:02   JP
   statt selbst den InserDocDialog zu rufen, die neue Funktion GetFileFilterNameDlg rufen

      Rev 1.5   24 Nov 1997 18:23:28   MA
   includes

      Rev 1.4   03 Nov 1997 13:18:10   MA
   precomp entfernt

      Rev 1.3   23 Jul 1997 20:25:54   HJS
   includes

      Rev 1.2   29 May 1997 16:16:02   OM
   URL in ScriptDialog eingeben

      Rev 1.1   13 Feb 1997 17:36:02   OM
   Segmentierung

      Rev 1.0   13 Feb 1997 17:35:22   OM
   Initial revision.

*************************************************************************/



