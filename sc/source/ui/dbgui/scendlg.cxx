/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_sc.hxx"

#undef SC_DLLIMPLEMENTATION



#include "scitems.hxx"
#include <svx/drawitem.hxx>
#include <svx/xtable.hxx>
#include <sfx2/objsh.hxx>
#include <unotools/useroptions.hxx>
#include <vcl/msgbox.hxx>
#include <unotools/localedatawrapper.hxx>

#include "global.hxx"
#include "globstr.hrc"
#include "tabvwsh.hxx"
#include "viewdata.hxx"
#include "document.hxx"
#include "scresid.hxx"
#include "scendlg.hrc"
#include "scendlg.hxx"

//========================================================================

ScNewScenarioDlg::ScNewScenarioDlg( Window* pParent, const String& rName, sal_Bool bEdit, sal_Bool bSheetProtected)

    :   ModalDialog     ( pParent, ScResId( RID_SCDLG_NEWSCENARIO ) ),
        aFlName         ( this, ScResId( FL_NAME )),
        aEdName         ( this, ScResId( ED_NAME ) ),
        aFlComment      ( this, ScResId( FL_COMMENT ) ),
        aEdComment      ( this, ScResId( ED_COMMENT ) ),
        aFlOptions      ( this, ScResId( FL_OPTIONS ) ),
        aCbShowFrame    ( this, ScResId( CB_SHOWFRAME ) ),
        aLbColor        ( this, ScResId( LB_COLOR ) ),
        //aCbPrintFrame ( this, ScResId( CB_PRINTFRAME ) ),
        aCbTwoWay       ( this, ScResId( CB_TWOWAY ) ),
        //aCbAttrib     ( this, ScResId( CB_ATTRIB ) ),
        //aCbValue      ( this, ScResId( CB_VALUE ) ),
        aCbCopyAll      ( this, ScResId( CB_COPYALL ) ),
        aCbProtect      ( this, ScResId( CB_PROTECT ) ),
        aBtnOk          ( this, ScResId( BTN_OK ) ),
        aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
        aBtnHelp        ( this, ScResId( BTN_HELP ) ),
        aDefScenarioName( rName ),
        bIsEdit         ( bEdit )
{
    if (bIsEdit)
        SetText(String(ScResId(STR_EDIT)));

    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    if ( pDocSh )
    {
        const SfxPoolItem* pItem = pDocSh->GetItem( SID_COLOR_TABLE );
        if ( pItem )
        {
            XColorTable* pColorTable = ((SvxColorTableItem*)pItem)->GetColorTable();
            if (pColorTable)
            {
                aLbColor.SetUpdateMode( false );
                long nCount = pColorTable->Count();
                for ( long n=0; n<nCount; n++ )
                {
                    XColorEntry* pEntry = pColorTable->GetColor(n);
                    aLbColor.InsertEntry( pEntry->GetColor(), pEntry->GetName() );
                }
                aLbColor.SetUpdateMode( sal_True );
            }
        }
    }

    SvtUserOptions aUserOpt;

    String aComment( ScResId( STR_CREATEDBY ) );

    aComment += ' ';
    aComment += (String)aUserOpt.GetFirstName();
    aComment += ' ';
    aComment += (String)aUserOpt.GetLastName();
    aComment.AppendAscii(RTL_CONSTASCII_STRINGPARAM( ", " ));
    aComment += String( ScResId( STR_ON ) );
    aComment += ' ';
    aComment += ScGlobal::GetpLocaleData()->getDate( Date() );
    aComment.AppendAscii(RTL_CONSTASCII_STRINGPARAM( ", " ));
    aComment += ScGlobal::GetpLocaleData()->getTime( Time() );

    aEdComment  .SetText( aComment );
    aEdName     .SetText( rName );
    aBtnOk      .SetClickHdl( LINK( this, ScNewScenarioDlg, OkHdl ) );
    aCbShowFrame.SetClickHdl( LINK( this, ScNewScenarioDlg, EnableHdl ) );

    aLbColor.SetAccessibleName(String(ScResId( STR_COLOR ) ));

    FreeResource();

    aLbColor.SelectEntry( Color( COL_LIGHTGRAY ) );
    aCbShowFrame.Check(sal_True);
    //aCbPrintFrame.Check(sal_True);
    aCbTwoWay.Check(sal_True);
    //aCbAttrib.Check(sal_False);
    //aCbValue.Check(sal_False);
    aCbCopyAll.Check(false);
    aCbProtect.Check(sal_True);

    if (bIsEdit)
        aCbCopyAll.Enable(false);
    // If the Sheet is protected then we disable the Scenario Protect input
    // and default it to true above. Note we are in 'Add' mode here as: if
    // Sheet && scenario protection are true, then we cannot edit this dialog.
    if (bSheetProtected)
        aCbProtect.Enable(false);

    //! die drei funktionieren noch nicht...
    /*
    aCbPrintFrame.Enable(sal_False);
    aCbAttrib.Enable(sal_False);
    aCbValue.Enable(sal_False);
    */

    aEdComment.SetAccessibleRelationMemberOf(&aFlComment);
    aLbColor.SetAccessibleRelationLabeledBy(&aCbShowFrame);
}

//------------------------------------------------------------------------

ScNewScenarioDlg::~ScNewScenarioDlg()
{
}

//------------------------------------------------------------------------

void ScNewScenarioDlg::GetScenarioData( String& rName, String& rComment,
                                        Color& rColor, sal_uInt16& rFlags ) const
{
    rComment = aEdComment.GetText();
    rName    = aEdName.GetText();

    if ( rName.Len() == 0 )
        rName = aDefScenarioName;

    rColor = aLbColor.GetSelectEntryColor();
    sal_uInt16 nBits = 0;
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
    if (aCbProtect.IsChecked())
        nBits |= SC_SCENARIO_PROTECT;
    rFlags = nBits;
}

void ScNewScenarioDlg::SetScenarioData( const String& rName, const String& rComment,
                                        const Color& rColor, sal_uInt16 nFlags )
{
    aEdComment.SetText(rComment);
    aEdName.SetText(rName);
    aLbColor.SelectEntry(rColor);

    aCbShowFrame.Check ( (nFlags & SC_SCENARIO_SHOWFRAME)  != 0 );
    EnableHdl( &aCbShowFrame );
    //aCbPrintFrame.Check( (nFlags & SC_SCENARIO_PRINTFRAME) != 0 );
    aCbTwoWay.Check    ( (nFlags & SC_SCENARIO_TWOWAY)     != 0 );
    //aCbAttrib.Check    ( (nFlags & SC_SCENARIO_ATTRIB)     != 0 );
    //aCbValue.Check     ( (nFlags & SC_SCENARIO_VALUE)      != 0 );
    //  CopyAll nicht
    aCbProtect.Check    ( (nFlags & SC_SCENARIO_PROTECT)     != 0 );
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

//------------------------------------------------------------------------

IMPL_LINK( ScNewScenarioDlg, EnableHdl, CheckBox *, pBox )
{
    if( pBox == &aCbShowFrame )
        aLbColor.Enable( aCbShowFrame.IsChecked() );
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
