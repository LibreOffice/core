/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <vcl/msgbox.hxx>
#include <sfx2/app.hxx>

#include "reffact.hxx"
#include "document.hxx"
#include "scresid.hxx"
#include "globstr.hrc"
#include "simpref.hrc"
#include "rangenam.hxx"     // IsNameValid
#include "simpref.hxx"
#include "scmod.hxx"

//============================================================================

#define ABS_SREF          SCA_VALID \
                        | SCA_COL_ABSOLUTE | SCA_ROW_ABSOLUTE | SCA_TAB_ABSOLUTE
#define ABS_DREF          ABS_SREF \
                        | SCA_COL2_ABSOLUTE | SCA_ROW2_ABSOLUTE | SCA_TAB2_ABSOLUTE
#define ABS_DREF3D      ABS_DREF | SCA_TAB_3D

//============================================================================
//  class ScSimpleRefDlg

//----------------------------------------------------------------------------
ScSimpleRefDlg::ScSimpleRefDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
                          ScViewData*   ptrViewData )

    :   ScAnyRefDlg ( pB, pCW, pParent, RID_SCDLG_SIMPLEREF ),
        //
        aFtAssign       ( this, ScResId( FT_ASSIGN ) ),
        aEdAssign       ( this, this, &aFtAssign, ScResId( ED_ASSIGN ) ),
        aRbAssign       ( this, ScResId( RB_ASSIGN ), &aEdAssign, this ),

        aBtnOk          ( this, ScResId( BTN_OK ) ),
        aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
        aBtnHelp        ( this, ScResId( BTN_HELP ) ),

        //
        pDoc            ( ptrViewData->GetDocument() ),
        bAutoReOpen     ( sal_True ),
        bCloseOnButtonUp( false ),
        bSingleCell     ( false ),
        bMultiSelection ( false )
{
    //  damit die Strings in der Resource bei den FixedTexten bleiben koennen:
    Init();
    FreeResource();
    SetDispatcherLock( sal_True ); // Modal-Modus einschalten
}

//----------------------------------------------------------------------------
ScSimpleRefDlg::~ScSimpleRefDlg()
{
    SetDispatcherLock( false ); // Modal-Modus einschalten
}

//----------------------------------------------------------------------------
void ScSimpleRefDlg::FillInfo(SfxChildWinInfo& rWinInfo) const
{
    ScAnyRefDlg::FillInfo(rWinInfo);
    rWinInfo.bVisible=bAutoReOpen;
}

//----------------------------------------------------------------------------
void ScSimpleRefDlg::SetRefString(const String &rStr)
{
    aEdAssign.SetText(rStr);
}

//----------------------------------------------------------------------------
void ScSimpleRefDlg::Init()
{
    aBtnOk.SetClickHdl      ( LINK( this, ScSimpleRefDlg, OkBtnHdl ) );
    aBtnCancel.SetClickHdl  ( LINK( this, ScSimpleRefDlg, CancelBtnHdl ) );
    bCloseFlag=false;
}

//----------------------------------------------------------------------------
// Uebergabe eines mit der Maus selektierten Tabellenbereiches, der dann als
//  neue Selektion im Referenz-Fenster angezeigt wird.
void ScSimpleRefDlg::SetReference( const ScRange& rRef, ScDocument* pDocP )
{
    if ( aEdAssign.IsEnabled() )
    {
        if ( rRef.aStart != rRef.aEnd )
            RefInputStart( &aEdAssign );

        theCurArea = rRef;
        String aRefStr;
        if ( bSingleCell )
        {
            ScAddress aAdr = rRef.aStart;
            aRefStr = aAdr.Format(SCA_ABS_3D, pDocP, pDocP->GetAddressConvention());
        }
        else
            aRefStr = theCurArea.Format(ABS_DREF3D, pDocP, pDocP->GetAddressConvention());

        if ( bMultiSelection )
        {
            String aVal = aEdAssign.GetText();
            Selection aSel = aEdAssign.GetSelection();
            aSel.Justify();
            aVal.Erase( (xub_StrLen)aSel.Min(), (xub_StrLen)aSel.Len() );
            aVal.Insert( aRefStr, (xub_StrLen)aSel.Min() );
            Selection aNewSel( aSel.Min(), aSel.Min()+aRefStr.Len() );
            aEdAssign.SetRefString( aVal );
            aEdAssign.SetSelection( aNewSel );
        }
        else
            aEdAssign.SetRefString( aRefStr );

        aChangeHdl.Call( &aRefStr );
    }
}


//----------------------------------------------------------------------------
sal_Bool ScSimpleRefDlg::Close()
{
    CancelBtnHdl(&aBtnCancel);
    return sal_True;
}

//------------------------------------------------------------------------
void ScSimpleRefDlg::SetActive()
{
    aEdAssign.GrabFocus();

    //  kein NameModifyHdl, weil sonst Bereiche nicht geaendert werden koennen
    //  (nach dem Aufziehen der Referenz wuerde der alte Inhalt wieder angezeigt)
    //  (der ausgewaehlte DB-Name hat sich auch nicht veraendert)

    RefInputDone();
}
//------------------------------------------------------------------------
sal_Bool ScSimpleRefDlg::IsRefInputMode() const
{
    return sal_True;
}

String ScSimpleRefDlg::GetRefString() const
{
    return aEdAssign.GetText();
}

void ScSimpleRefDlg::SetCloseHdl( const Link& rLink )
{
    aCloseHdl=rLink;
}

void ScSimpleRefDlg::SetUnoLinks( const Link& rDone, const Link& rAbort,
                                    const Link& rChange )
{
    aDoneHdl    = rDone;
    aAbortedHdl = rAbort;
    aChangeHdl  = rChange;
}

void ScSimpleRefDlg::SetFlags( sal_Bool bSetCloseOnButtonUp, sal_Bool bSetSingleCell, sal_Bool bSetMultiSelection )
{
    bCloseOnButtonUp = bSetCloseOnButtonUp;
    bSingleCell = bSetSingleCell;
    bMultiSelection = bSetMultiSelection;
}

void ScSimpleRefDlg::StartRefInput()
{
    if ( bMultiSelection )
    {
        // initially select the whole string, so it gets replaced by default
        aEdAssign.SetSelection( Selection( 0, aEdAssign.GetText().getLength() ) );
    }

    aRbAssign.DoRef();
    bCloseFlag=sal_True;
}

void ScSimpleRefDlg::RefInputDone( sal_Bool bForced)
{
    ScAnyRefDlg::RefInputDone(bForced);
    if ( (bForced || bCloseOnButtonUp) && bCloseFlag )
        OkBtnHdl(&aBtnOk);
}
//------------------------------------------------------------------------
// Handler:
// ========
IMPL_LINK_NOARG(ScSimpleRefDlg, OkBtnHdl)
{
    bAutoReOpen=false;
    String aResult=aEdAssign.GetText();
    aCloseHdl.Call(&aResult);
    Link aUnoLink = aDoneHdl;       // stack var because this is deleted in DoClose
    DoClose( ScSimpleRefDlgWrapper::GetChildWindowId() );
    aUnoLink.Call( &aResult );
    return 0;
}

//------------------------------------------------------------------------
IMPL_LINK_NOARG(ScSimpleRefDlg, CancelBtnHdl)
{
    bAutoReOpen=false;
    String aResult=aEdAssign.GetText();
    aCloseHdl.Call(NULL);
    Link aUnoLink = aAbortedHdl;    // stack var because this is deleted in DoClose
    DoClose( ScSimpleRefDlgWrapper::GetChildWindowId() );
    aUnoLink.Call( &aResult );
    return 0;
}



//------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
