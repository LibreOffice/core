/*************************************************************************
 *
 *  $RCSfile: simpref.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: nn $ $Date: 2000-11-09 19:56:06 $
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

// System - Includes ---------------------------------------------------------

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#ifndef PCH
#include <segmentc.hxx>
#endif

// INCLUDE -------------------------------------------------------------------

#include <vcl/msgbox.hxx>

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
#define ABS_SREF3D      ABS_SREF | SCA_TAB_3D
#define ABS_DREF3D      ABS_DREF | SCA_TAB_3D

//----------------------------------------------------------------------------

#define ERRORBOX(s) ErrorBox(this,WinBits(WB_OK|WB_DEF_OK),s).Execute()
#define QUERYBOX(m) QueryBox(this,WinBits(WB_YES_NO|WB_DEF_YES),m).Execute()

static String aPrivLastRefString;

//============================================================================
//  class ScSimpleRefDlg

//----------------------------------------------------------------------------
ScSimpleRefDlg::ScSimpleRefDlg( SfxBindings* pB, SfxChildWindow* pCW, Window* pParent,
                          ScViewData*   ptrViewData )

    :   ScAnyRefDlg ( pB, pCW, pParent, RID_SCDLG_SIMPLEREF ),
        //
        aFtAssign       ( this, ScResId( FT_ASSIGN ) ),
        aEdAssign       ( this, ScResId( ED_ASSIGN ) ),
        aRbAssign       ( this, ScResId( RB_ASSIGN ), &aEdAssign ),

        aBtnOk          ( this, ScResId( BTN_OK ) ),
        aBtnCancel      ( this, ScResId( BTN_CANCEL ) ),
        aBtnHelp        ( this, ScResId( BTN_HELP ) ),

        //
        pViewData       ( ptrViewData ),
        pDoc            ( ptrViewData->GetDocument() ),
        bRefInputMode   ( FALSE ),
        bAutoReOpen     ( TRUE ),
        bCloseOnButtonUp( FALSE )
{
    //  damit die Strings in der Resource bei den FixedTexten bleiben koennen:
    Init();
    FreeResource();
    SFX_APP()->LockDispatcher( TRUE ); // Modal-Modus einschalten

    aPrivLastRefString=String();
}

//----------------------------------------------------------------------------
__EXPORT ScSimpleRefDlg::~ScSimpleRefDlg()
{
    aPrivLastRefString=aEdAssign.GetText();

    SFX_APP()->LockDispatcher( FALSE); // Modal-Modus einschalten
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
    bCloseFlag=FALSE;
}

//----------------------------------------------------------------------------
// Uebergabe eines mit der Maus selektierten Tabellenbereiches, der dann als
//  neue Selektion im Referenz-Fenster angezeigt wird.
void ScSimpleRefDlg::SetReference( const ScRange& rRef, ScDocument* pDoc )
{
    if ( aEdAssign.IsEnabled() )
    {
        if ( rRef.aStart != rRef.aEnd )
            RefInputStart( &aEdAssign );

        theCurArea = rRef;

        String aRefStr;
        theCurArea.Format( aRefStr, ABS_DREF3D, pDoc );
        aEdAssign.SetRefString( aRefStr );

        aChangeHdl.Call( &aRefStr );
    }
}


//----------------------------------------------------------------------------
BOOL __EXPORT ScSimpleRefDlg::Close()
{
    CancelBtnHdl(&aBtnCancel);
    return TRUE;
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
BOOL ScSimpleRefDlg::IsRefInputMode() const
{
    return TRUE;
}

String ScSimpleRefDlg::GetRefString() const
{
    return aEdAssign.GetText();
}

String ScSimpleRefDlg::GetLastRefString()
{
    return aPrivLastRefString;
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

void ScSimpleRefDlg::SetFlags( BOOL bSetCloseOnButtonUp )
{
    bCloseOnButtonUp = bSetCloseOnButtonUp;
}

void ScSimpleRefDlg::StartRefInput()
{
    aRbAssign.DoRef();
    bCloseFlag=TRUE;
}

void ScSimpleRefDlg::RefInputDone( BOOL bForced)
{
    ScAnyRefDlg::RefInputDone(bForced);
    if ( (bForced || bCloseOnButtonUp) && bCloseFlag )
        OkBtnHdl(&aBtnOk);
}
//------------------------------------------------------------------------
// Handler:
// ========
IMPL_LINK( ScSimpleRefDlg, OkBtnHdl, void *, EMPTYARG )
{
    bAutoReOpen=FALSE;
    String aResult=aEdAssign.GetText();
    aCloseHdl.Call(&aResult);
    Link aUnoLink = aDoneHdl;       // stack var because this is deleted in DoClose
    DoClose( ScSimpleRefDlgWrapper::GetChildWindowId() );
    aUnoLink.Call( &aResult );
    return 0;
}

//------------------------------------------------------------------------
IMPL_LINK( ScSimpleRefDlg, CancelBtnHdl, void *, EMPTYARG )
{
    bAutoReOpen=FALSE;
    String aResult=aEdAssign.GetText();
    aCloseHdl.Call(NULL);
    Link aUnoLink = aAbortedHdl;    // stack var because this is deleted in DoClose
    DoClose( ScSimpleRefDlgWrapper::GetChildWindowId() );
    aUnoLink.Call( &aResult );
    return 0;
}



//------------------------------------------------------------------------

