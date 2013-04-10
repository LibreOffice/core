/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
            XColorList* pColorTable = ((SvxColorTableItem*)pItem)->GetColorTable();
            if (pColorTable)
            {
                aLbColor.SetUpdateMode( sal_False );
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
    aComment += ScGlobal::GetpLocaleData()->getDate( Date() );//CHINA001 aComment += ScGlobal::pLocaleData->getDate( Date() );
    aComment.AppendAscii(RTL_CONSTASCII_STRINGPARAM( ", " ));
    aComment += ScGlobal::GetpLocaleData()->getTime( Time() );//CHINA001  aComment += ScGlobal::pLocaleData->getTime( Time() );

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
    aCbCopyAll.Check(sal_False);
    aCbProtect.Check(sal_True);

    if (bIsEdit)
        aCbCopyAll.Enable(sal_False);
    // If the Sheet is protected then we disable the Scenario Protect input
    // and default it to true above. Note we are in 'Add' mode here as: if
    // Sheet && scenario protection are true, then we cannot edit this dialog.
    if (bSheetProtected)
        aCbProtect.Enable(sal_False);

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

__EXPORT ScNewScenarioDlg::~ScNewScenarioDlg()
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

