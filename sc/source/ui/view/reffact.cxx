/*************************************************************************
 *
 *  $RCSfile: reffact.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2000-11-14 15:42:44 $
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

// INCLUDE ---------------------------------------------------------------

#include <sfx2/app.hxx>
#include <sfx2/basedlgs.hxx>
#include <sfx2/viewfrm.hxx>

#include "reffact.hxx"
#include "tabvwsh.hxx"
#include "sc.hrc"
#include "acredlin.hxx"
#include "simpref.hxx"
#include "scmod.hxx"

// -----------------------------------------------------------------------

SFX_IMPL_MODELESSDIALOG(ScNameDlgWrapper, FID_DEFINE_NAME )
SFX_IMPL_MODELESSDIALOG(ScSolverDlgWrapper, SID_OPENDLG_SOLVE )
SFX_IMPL_MODELESSDIALOG(ScPivotLayoutWrapper, SID_OPENDLG_PIVOTTABLE )
SFX_IMPL_MODELESSDIALOG(ScTabOpDlgWrapper, SID_OPENDLG_TABOP )
SFX_IMPL_MODELESSDIALOG(ScFilterDlgWrapper, SID_FILTER )
SFX_IMPL_MODELESSDIALOG(ScSpecialFilterDlgWrapper, SID_SPECIAL_FILTER )
SFX_IMPL_MODELESSDIALOG(ScDbNameDlgWrapper, SID_DEFINE_DBNAME )
SFX_IMPL_MODELESSDIALOG(ScConsolidateDlgWrapper, SID_OPENDLG_CONSOLIDATE )
SFX_IMPL_MODELESSDIALOG(ScChartDlgWrapper, SID_OPENDLG_CHART )
SFX_IMPL_MODELESSDIALOG(ScPrintAreasDlgWrapper, SID_OPENDLG_EDIT_PRINTAREA )
SFX_IMPL_MODELESSDIALOG(ScCondFormatDlgWrapper, SID_OPENDLG_CONDFRMT )
SFX_IMPL_MODELESSDIALOG(ScColRowNameRangesDlgWrapper, SID_DEFINE_COLROWNAMERANGES )
SFX_IMPL_MODELESSDIALOG(ScFormulaDlgWrapper, SID_OPENDLG_FUNCTION )
SFX_IMPL_MODELESSDIALOG(ScAcceptChgDlgWrapper, FID_CHG_ACCEPT )
SFX_IMPL_MODELESSDIALOG(ScHighlightChgDlgWrapper, FID_CHG_SHOW )
SFX_IMPL_MODELESSDIALOG(ScSimpleRefDlgWrapper, WID_SIMPLE_REF )
/*!!! dafuer muss der Funktionsautopilot noch umgebaut werden
SFX_IMPL_CHILDWINDOW(ScFunctionDlgWrapper, SID_OPENDLG_FUNCTION )
SFX_IMPL_CHILDWINDOW(ScEditFunctionDlgWrapper, SID_OPENDLG_EDITFUNCTION )
SFX_IMPL_CHILDWINDOW(ScArgumentDlgWrapper, SID_OPENDLG_ARGUMENT )
*/

#define IMPL_CHILD_CTOR(Class,sid) \
    Class::Class( Window*               pParent,                    \
                    USHORT              nId,                        \
                    SfxBindings*        p,                          \
                    SfxChildWinInfo*    pInfo )                     \
        : SfxChildWindow(pParent, nId)                              \
    {                                                               \
        ScTabViewShell* pViewShell =                                \
            PTR_CAST( ScTabViewShell, SfxViewShell::Current() );    \
        DBG_ASSERT( pViewShell, "missing view shell :-(" );         \
        pWindow = pViewShell ?                                      \
            pViewShell->CreateRefDialog( p, this, pInfo, pParent, sid ) : NULL; \
        if (pViewShell && !pWindow)                                             \
            pViewShell->GetViewFrame()->SetChildWindow( nId, FALSE );           \
    }


//=========================================================================

//-------------------------------------------------------------------------
// ScNameDlgWrapper
//-------------------------------------------------------------------------

IMPL_CHILD_CTOR( ScNameDlgWrapper, FID_DEFINE_NAME )

//-------------------------------------------------------------------------
// ScSolverDlgWrapper
//-------------------------------------------------------------------------

IMPL_CHILD_CTOR( ScSolverDlgWrapper, SID_OPENDLG_SOLVE )

//-------------------------------------------------------------------------
// ScPivotLayoutWrapper
//-------------------------------------------------------------------------

IMPL_CHILD_CTOR( ScPivotLayoutWrapper, SID_OPENDLG_PIVOTTABLE )

//-------------------------------------------------------------------------
// ScTabOpDlgWrapper
//-------------------------------------------------------------------------

IMPL_CHILD_CTOR( ScTabOpDlgWrapper, SID_OPENDLG_TABOP )

//-------------------------------------------------------------------------
// ScFilterDlgWrapper
//-------------------------------------------------------------------------

IMPL_CHILD_CTOR( ScFilterDlgWrapper, SID_FILTER )

//-------------------------------------------------------------------------
// ScSpecialFilterDlgWrapper
//-------------------------------------------------------------------------

IMPL_CHILD_CTOR( ScSpecialFilterDlgWrapper, SID_SPECIAL_FILTER )

//-------------------------------------------------------------------------
// ScDbNameDlgWrapper
//-------------------------------------------------------------------------

IMPL_CHILD_CTOR( ScDbNameDlgWrapper, SID_DEFINE_DBNAME )

//-------------------------------------------------------------------------
// ScColRowNameRangesDlgWrapper
//-------------------------------------------------------------------------

IMPL_CHILD_CTOR( ScColRowNameRangesDlgWrapper, SID_DEFINE_COLROWNAMERANGES )

//-------------------------------------------------------------------------
// ScConsolidateDlgWrapper
//-------------------------------------------------------------------------

IMPL_CHILD_CTOR( ScConsolidateDlgWrapper, SID_OPENDLG_CONSOLIDATE )

//-------------------------------------------------------------------------
// ScChartDlgWrapper
//-------------------------------------------------------------------------

IMPL_CHILD_CTOR( ScChartDlgWrapper, SID_OPENDLG_CHART )

//-------------------------------------------------------------------------
// ScPrintAreasDlgWrapper
//-------------------------------------------------------------------------

IMPL_CHILD_CTOR( ScPrintAreasDlgWrapper, SID_OPENDLG_EDIT_PRINTAREA )

//-------------------------------------------------------------------------
// ScCondFormatDlgWrapper
//-------------------------------------------------------------------------

IMPL_CHILD_CTOR( ScCondFormatDlgWrapper, SID_OPENDLG_CONDFRMT )

//-------------------------------------------------------------------------
// ScFormulaDlgWrapper
//-------------------------------------------------------------------------

IMPL_CHILD_CTOR( ScFormulaDlgWrapper, SID_OPENDLG_FUNCTION )


//-------------------------------------------------------------------------
// ScSimpleRefDlgWrapper
//-------------------------------------------------------------------------

static BOOL     bScSimpleRefFlag;
static long     nScSimpleRefHeight;
static long     nScSimpleRefWidth;
static long     nScSimpleRefX;
static long     nScSimpleRefY;
static BOOL     bAutoReOpen=TRUE;

ScSimpleRefDlgWrapper::ScSimpleRefDlgWrapper( Window* pParent,
                                USHORT              nId,
                                SfxBindings*        p,
                                SfxChildWinInfo*    pInfo )
        : SfxChildWindow(pParent, nId)
{
    ScTabViewShell* pViewShell =
        PTR_CAST( ScTabViewShell, SfxViewShell::Current() );
    DBG_ASSERT( pViewShell, "missing view shell :-(" );

    if(pInfo!=NULL && bScSimpleRefFlag)
    {
        pInfo->aPos.X()=nScSimpleRefX;
        pInfo->aPos.Y()=nScSimpleRefY;
        pInfo->aSize.Height()=nScSimpleRefHeight;
        pInfo->aSize.Width()=nScSimpleRefWidth;
    }
    pWindow = NULL;

    if(bAutoReOpen)
        pWindow = pViewShell->CreateRefDialog( p, this, pInfo, pParent, WID_SIMPLE_REF);

    if (!pWindow)
    {
        SC_MOD()->SetRefDialog( nId, FALSE );
    }
}

void ScSimpleRefDlgWrapper::SetDefaultPosSize(Point aPos, Size aSize, BOOL bSet)
{
    bScSimpleRefFlag=bSet;
    if(bScSimpleRefFlag)
    {
        nScSimpleRefX=aPos.X();
        nScSimpleRefY=aPos.Y();
        nScSimpleRefHeight=aSize.Height();
        nScSimpleRefWidth=aSize.Width();
    }
}


String ScSimpleRefDlgWrapper::GetRefString()
{
    String aResult;
    if(pWindow!=NULL)
    {
        aResult=((ScSimpleRefDlg*)pWindow)->GetRefString();
    }
    return aResult;
}

void ScSimpleRefDlgWrapper::SetAutoReOpen(BOOL bFlag)
{
    bAutoReOpen=bFlag;
}

void ScSimpleRefDlgWrapper::SetRefString(const String& rStr)
{
    if(pWindow!=NULL)
    {
        ((ScSimpleRefDlg*)pWindow)->SetRefString(rStr);
    }
}

void ScSimpleRefDlgWrapper::SetCloseHdl( const Link& rLink )
{
    if(pWindow!=NULL)
    {
        ((ScSimpleRefDlg*)pWindow)->SetCloseHdl( rLink );
    }
}

void ScSimpleRefDlgWrapper::SetUnoLinks( const Link& rDone,
                    const Link& rAbort, const Link& rChange )
{
    if(pWindow!=NULL)
    {
        ((ScSimpleRefDlg*)pWindow)->SetUnoLinks( rDone, rAbort, rChange );
    }
}

void ScSimpleRefDlgWrapper::SetFlags( BOOL bCloseOnButtonUp )
{
    if(pWindow!=NULL)
    {
        ((ScSimpleRefDlg*)pWindow)->SetFlags( bCloseOnButtonUp );
    }
}

void ScSimpleRefDlgWrapper::StartRefInput()
{
    if(pWindow!=NULL)
    {
        ((ScSimpleRefDlg*)pWindow)->StartRefInput();
    }
}



//-------------------------------------------------------------------------
// ScAcceptChgDlgWrapper //Kommentar: sollte in die ViewShell
//-------------------------------------------------------------------------

ScAcceptChgDlgWrapper::ScAcceptChgDlgWrapper(   Window* pParent,
                                            USHORT nId,
                                            SfxBindings* pBindings,
                                            SfxChildWinInfo* pInfo ) :
                                            SfxChildWindow( pParent, nId )
{
        ScTabViewShell* pViewShell =
            PTR_CAST( ScTabViewShell, SfxViewShell::Current() );
        DBG_ASSERT( pViewShell, "missing view shell :-(" );
        pWindow = new ScAcceptChgDlg( pBindings, this, pParent, pViewShell->GetViewData() );
        if(pWindow!=NULL)
        {
            ((ScAcceptChgDlg*)pWindow)->Initialize( pInfo );
        }
        if (pViewShell && !pWindow)
            pViewShell->GetViewFrame()->SetChildWindow( nId, FALSE );
}

void ScAcceptChgDlgWrapper::ReInitDlg()
{
    ScTabViewShell* pViewShell =
        PTR_CAST( ScTabViewShell, SfxViewShell::Current() );
    DBG_ASSERT( pViewShell, "missing view shell :-(" );

    if(pWindow!=NULL)
    {
        ((ScAcceptChgDlg*)pWindow)->ReInit(pViewShell->GetViewData());
    }
}

//-------------------------------------------------------------------------
// ScHighlightChgDlgWrapper
//-------------------------------------------------------------------------

IMPL_CHILD_CTOR( ScHighlightChgDlgWrapper, FID_CHG_SHOW )

/*------------------------------------------------------------------------*/
/*@@@
        //-------------------------------------------------------------------------
        // ScFunctionDlgWrapper
        //-------------------------------------------------------------------------

        IMPL_CHILD_CTOR( ScFunctionDlgWrapper, SID_OPENDLG_FUNCTION )

        //-------------------------------------------------------------------------
        // ScEditFunctionDlgWrapper
        //-------------------------------------------------------------------------

        IMPL_CHILD_CTOR( ScEditFunctionDlgWrapper, SID_OPENDLG_EDITFUNCTION )

        //-------------------------------------------------------------------------
        // ScArgumentDlgWrapper
        //-------------------------------------------------------------------------

        IMPL_CHILD_CTOR( ScArgumentDlgWrapper, SID_OPENDLG_ARGUMENT )
@@@*/
/*------------------------------------------------------------------------*/


