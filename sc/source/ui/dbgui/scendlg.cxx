/*************************************************************************
 *
 *  $RCSfile: scendlg.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:54 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#include "scitems.hxx"
#include <svx/drawitem.hxx>
#include <svx/xtable.hxx>
#include <sfx2/inimgr.hxx>
#include <sfx2/objsh.hxx>
#include <svtools/iniman.hxx>
#include <vcl/msgbox.hxx>
#include <segmentc.hxx>

#include "global.hxx"
#include "globstr.hrc"
#include "tabvwsh.hxx"
#include "viewdata.hxx"
#include "document.hxx"
#include "scresid.hxx"
#include "scendlg.hrc"
#include "scendlg.hxx"

SEG_EOFGLOBALS()

//========================================================================
#pragma SEG_FUNCDEF(scendlg_01)

ScNewScenarioDlg::ScNewScenarioDlg( Window* pParent, const String& rName, BOOL bEdit )

    :   ModalDialog     ( pParent, ScResId( RID_SCDLG_NEWSCENARIO ) ),
        aBtnOk          ( this, ScResId( BTN_OK ) ),
        aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
        aBtnHelp        ( this, ScResId( BTN_HELP ) ),
        aEdName         ( this, ScResId( ED_NAME ) ),
        aEdComment      ( this, ScResId( ED_COMMENT ) ),
        aCbShowFrame    ( this, ResId( CB_SHOWFRAME ) ),
        aLbColor        ( this, ResId( LB_COLOR ) ),
        //aCbPrintFrame ( this, ResId( CB_PRINTFRAME ) ),
        aCbTwoWay       ( this, ResId( CB_TWOWAY ) ),
        //aCbAttrib     ( this, ResId( CB_ATTRIB ) ),
        //aCbValue      ( this, ResId( CB_VALUE ) ),
        aCbCopyAll      ( this, ResId( CB_COPYALL ) ),
        aGbName         ( this, ResId( GB_NAME )),
        aGbComment      ( this, ResId( GB_COMMENT ) ),
        aGbOptions      ( this, ResId( GB_OPTIONS ) ),
        aDefScenarioName( rName ),
        bIsEdit         ( bEdit )
{
    if (bIsEdit)
        SetText(String(ResId(STR_EDIT)));

    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    if ( pDocSh )
    {
        const SfxPoolItem* pItem = pDocSh->GetItem( ITEMID_COLOR_TABLE );
        if ( pItem )
        {
            XColorTable* pColorTable = ((SvxColorTableItem*)pItem)->GetColorTable();
            if (pColorTable)
            {
                aLbColor.SetUpdateMode( FALSE );
                long nCount = pColorTable->Count();
                for ( long n=0; n<nCount; n++ )
                {
                    XColorEntry* pEntry = pColorTable->Get(n);
                    aLbColor.InsertEntry( pEntry->GetColor(), pEntry->GetName() );
                }
                aLbColor.SetUpdateMode( TRUE );
            }
        }
    }

    String aComment( ScResId( STR_CREATEDBY ) );

    aComment += ' ';
    aComment += SFX_INIMANAGER()->Get( SFX_KEY_USER_NAME );
    aComment.AppendAscii(RTL_CONSTASCII_STRINGPARAM( ", " ));
    aComment += String( ScResId( STR_ON ) );
    aComment += ' ';
    aComment += ScGlobal::pScInternational->GetDate( Date() );
    aComment.AppendAscii(RTL_CONSTASCII_STRINGPARAM( ", " ));
    aComment += ScGlobal::pScInternational->GetTime( Time() );

    aEdComment  .SetText( aComment );
    aEdName     .SetText( rName );
    aBtnOk      .SetClickHdl( LINK( this, ScNewScenarioDlg, OkHdl ) );

    FreeResource();

    aLbColor.SelectEntry( Color( COL_LIGHTGRAY ) );
    aCbShowFrame.Check(TRUE);
    //aCbPrintFrame.Check(TRUE);
    aCbTwoWay.Check(TRUE);
    //aCbAttrib.Check(FALSE);
    //aCbValue.Check(FALSE);
    aCbCopyAll.Check(FALSE);

    if (bIsEdit)
        aCbCopyAll.Enable(FALSE);

    //! die drei funktionieren noch nicht...
    /*
    aCbPrintFrame.Enable(FALSE);
    aCbAttrib.Enable(FALSE);
    aCbValue.Enable(FALSE);
    */
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(scendlg_03)

__EXPORT ScNewScenarioDlg::~ScNewScenarioDlg()
{
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(scendlg_02)

void ScNewScenarioDlg::GetScenarioData( String& rName, String& rComment,
                                        Color& rColor, USHORT& rFlags ) const
{
    rComment = aEdComment.GetText();
    rName    = aEdName.GetText();

    if ( rName.Len() == 0 )
        rName = aDefScenarioName;

    rColor = aLbColor.GetSelectEntryColor();
    USHORT nBits = 0;
    if (aCbShowFrame.IsChecked())
        nBits |= SC_SCENARIO_SHOWFRAME;
    /*
    if (aCbPrintFrame.IsChecked())
        nBits |= SC_SCENARIO_PRINTFRAME;
    */
    if (aCbTwoWay.IsChecked())
        nBits |= SC_SCENARIO_TWOWAY;
    /*
    if (aCbAttrib.IsChecked())
        nBits |= SC_SCENARIO_ATTRIB;
    if (aCbValue.IsChecked())
        nBits |= SC_SCENARIO_VALUE;
    */
    if (aCbCopyAll.IsChecked())
        nBits |= SC_SCENARIO_COPYALL;
    rFlags = nBits;
}

void ScNewScenarioDlg::SetScenarioData( const String& rName, const String& rComment,
                                        const Color& rColor, USHORT nFlags )
{
    aEdComment.SetText(rComment);
    aEdName.SetText(rName);
    aLbColor.SelectEntry(rColor);

    aCbShowFrame.Check ( (nFlags & SC_SCENARIO_SHOWFRAME)  != 0 );
    //aCbPrintFrame.Check( (nFlags & SC_SCENARIO_PRINTFRAME) != 0 );
    aCbTwoWay.Check    ( (nFlags & SC_SCENARIO_TWOWAY)     != 0 );
    //aCbAttrib.Check    ( (nFlags & SC_SCENARIO_ATTRIB)     != 0 );
    //aCbValue.Check     ( (nFlags & SC_SCENARIO_VALUE)      != 0 );
    //  CopyAll nicht
}

//------------------------------------------------------------------------
#pragma SEG_FUNCDEF(scendlg_04)

IMPL_LINK( ScNewScenarioDlg, OkHdl, OKButton *, EMPTYARG )
{
    String      aName   ( aEdName.GetText() );
    ScDocument* pDoc    = ((ScTabViewShell*)SfxViewShell::Current())->
                                GetViewData()->GetDocument();

    aName.EraseLeadingChars( ' ' );
    aName.EraseTrailingChars( ' ' );
    aEdName.SetText( aName );

    if ( !pDoc->ValidTabName( aName ) )
    {
        InfoBox( this, ScGlobal::GetRscString( STR_INVALIDTABNAME ) ).
            Execute();
        aEdName.GrabFocus();
    }
    else if ( !bIsEdit && !pDoc->ValidNewTabName( aName ) )
    {
        InfoBox( this, ScGlobal::GetRscString( STR_NEWTABNAMENOTUNIQUE ) ).
            Execute();
        aEdName.GrabFocus();
    }
    else
        EndDialog( RET_OK );
    return 0;

    //! beim Editieren testen, ob eine andere Tabelle den Namen hat!
}

/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.24  2000/09/17 14:08:57  willem.vandorp
    OpenOffice header added.

    Revision 1.23  2000/08/31 16:38:20  willem.vandorp
    Header and footer replaced

    Revision 1.22  2000/04/14 17:38:03  nn
    unicode changes

    Revision 1.21  2000/02/11 12:24:00  hr
    #70473# changes for unicode ( patched by automated patchtool )

    Revision 1.20  1998/03/16 18:23:02  ANK
    Neues Outfit


      Rev 1.19   16 Mar 1998 19:23:02   ANK
   Neues Outfit

      Rev 1.18   10 Mar 1998 22:05:40   NN
   Controls fuer neue Einstellungen

      Rev 1.17   08 Mar 1998 21:16:20   NN
   Szenario-Einstellungen

      Rev 1.16   05 Dec 1997 19:56:10   ANK
   Includes geaendert

      Rev 1.15   29 Oct 1996 14:03:56   NN
   ueberall ScResId statt ResId

      Rev 1.14   22 Mar 1996 12:06:16   ER
   GetUserName --> Get SFX_KEY_USER_NAME

      Rev 1.13   29 Jan 1996 15:12:30   MO
   neuer Link

      Rev 1.12   08 Nov 1995 13:06:24   MO
   301-Aenderungen

      Rev 1.11   26 Jul 1995 08:11:52   STE
   Compilefehler behoben

      Rev 1.10   25 Jul 1995 19:15:58   HJS
   GetAppInternational => pInternational

      Rev 1.9   24 Jul 1995 14:11:52   MO
   EXPORT

      Rev 1.8   04 May 1995 08:16:28   TRI
   pApp -> Application::

      Rev 1.7   27 Apr 1995 15:35:48   MO
   Fehlermeldung, wenn TabName ungueltig

      Rev 1.6   27 Apr 1995 10:01:16   MO
   Leerzeichn im Namen durch '_' ersetzen

      Rev 1.5   31 Mar 1995 10:16:24   MO
   Ueberpruefung auf doppelte Tabellennamen

      Rev 1.4   29 Mar 1995 16:39:40   MO
   Defautlname im Ctor

      Rev 1.3   28 Mar 1995 17:29:18   MO
   Benutzername ueber Sfx-Inimanager

      Rev 1.2   24 Mar 1995 13:27:14   TRI
   Segmentierung

      Rev 1.1   21 Mar 1995 14:09:44   TRI
   sfx.hxx included

      Rev 1.0   20 Mar 1995 16:46:08   MO
   Dialog zum Anlegen eines Szenarios

------------------------------------------------------------------------ */

#pragma SEG_EOFMODULE


