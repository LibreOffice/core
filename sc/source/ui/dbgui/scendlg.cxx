/*************************************************************************
 *
 *  $RCSfile: scendlg.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: nn $ $Date: 2000-09-29 10:16:25 $
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
#include <svtools/useroptions.hxx>
#include <vcl/msgbox.hxx>

#include "global.hxx"
#include "globstr.hrc"
#include "tabvwsh.hxx"
#include "viewdata.hxx"
#include "document.hxx"
#include "scresid.hxx"
#include "scendlg.hrc"
#include "scendlg.hxx"

//========================================================================

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

    SvtUserOptions aUserOpt;

    String aComment( ScResId( STR_CREATEDBY ) );

    aComment += ' ';
    aComment += aUserOpt.GetFirstName();
    aComment += ' ';
    aComment += aUserOpt.GetLastName();
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

__EXPORT ScNewScenarioDlg::~ScNewScenarioDlg()
{
}

//------------------------------------------------------------------------

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


