/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: reffact.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: ihi $ $Date: 2008-02-05 15:48:39 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



// INCLUDE ---------------------------------------------------------------

#include <sfx2/app.hxx>
#include <sfx2/basedlgs.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
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
SFX_IMPL_MODELESSDIALOG(ScOptSolverDlgWrapper, SID_OPENDLG_OPTSOLVER )
SFX_IMPL_MODELESSDIALOG(ScPivotLayoutWrapper, SID_OPENDLG_PIVOTTABLE )
SFX_IMPL_MODELESSDIALOG(ScTabOpDlgWrapper, SID_OPENDLG_TABOP )
SFX_IMPL_MODELESSDIALOG(ScFilterDlgWrapper, SID_FILTER )
SFX_IMPL_MODELESSDIALOG(ScSpecialFilterDlgWrapper, SID_SPECIAL_FILTER )
SFX_IMPL_MODELESSDIALOG(ScDbNameDlgWrapper, SID_DEFINE_DBNAME )
SFX_IMPL_MODELESSDIALOG(ScConsolidateDlgWrapper, SID_OPENDLG_CONSOLIDATE )
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
    Class::Class( Window*               pParentP,                   \
                    USHORT              nId,                        \
                    SfxBindings*        p,                          \
                    SfxChildWinInfo*    pInfo )                     \
        : SfxChildWindow(pParentP, nId)                             \
    {                                                               \
        ScTabViewShell* pViewShell =                                \
            PTR_CAST( ScTabViewShell, SfxViewShell::Current() );    \
        DBG_ASSERT( pViewShell, "missing view shell :-(" );         \
        pWindow = pViewShell ?                                      \
            pViewShell->CreateRefDialog( p, this, pInfo, pParentP, sid ) : NULL;    \
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
// ScOptSolverDlgWrapper
//-------------------------------------------------------------------------

IMPL_CHILD_CTOR( ScOptSolverDlgWrapper, SID_OPENDLG_OPTSOLVER )

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

ScSimpleRefDlgWrapper::ScSimpleRefDlgWrapper( Window* pParentP,
                                USHORT              nId,
                                SfxBindings*        p,
                                SfxChildWinInfo*    pInfo )
        : SfxChildWindow(pParentP, nId)
{
//  ScTabViewShell* pViewShell =
//      PTR_CAST( ScTabViewShell, SfxViewShell::Current() );

    ScTabViewShell* pViewShell = NULL;
    SfxDispatcher* pDisp = p->GetDispatcher();
    if ( pDisp )
    {
        SfxViewFrame* pViewFrm = pDisp->GetFrame();
        if ( pViewFrm )
            pViewShell = PTR_CAST( ScTabViewShell, pViewFrm->GetViewShell() );
    }

    DBG_ASSERT( pViewShell, "missing view shell :-(" );

    if(pInfo!=NULL && bScSimpleRefFlag)
    {
        pInfo->aPos.X()=nScSimpleRefX;
        pInfo->aPos.Y()=nScSimpleRefY;
        pInfo->aSize.Height()=nScSimpleRefHeight;
        pInfo->aSize.Width()=nScSimpleRefWidth;
    }
    pWindow = NULL;

    if(bAutoReOpen && pViewShell)
        pWindow = pViewShell->CreateRefDialog( p, this, pInfo, pParentP, WID_SIMPLE_REF);

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

void ScSimpleRefDlgWrapper::SetFlags( BOOL bCloseOnButtonUp, BOOL bSingleCell, BOOL bMultiSelection )
{
    if(pWindow!=NULL)
    {
        ((ScSimpleRefDlg*)pWindow)->SetFlags( bCloseOnButtonUp, bSingleCell, bMultiSelection );
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

ScAcceptChgDlgWrapper::ScAcceptChgDlgWrapper(   Window* pParentP,
                                            USHORT nId,
                                            SfxBindings* pBindings,
                                            SfxChildWinInfo* pInfo ) :
                                            SfxChildWindow( pParentP, nId )
{
        ScTabViewShell* pViewShell =
            PTR_CAST( ScTabViewShell, SfxViewShell::Current() );
        DBG_ASSERT( pViewShell, "missing view shell :-(" );
        pWindow = pViewShell ?
            new ScAcceptChgDlg( pBindings, this, pParentP, pViewShell->GetViewData() ) :
            NULL;
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

    if(pWindow!=NULL && pViewShell)
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


