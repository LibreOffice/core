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
//<!--Added by PengYunQuan for Validity Cell Range Picker
#include "validate.hxx"
//<!--Added by PengYunQuan for Validity Cell Range Picker

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
//<!--Added by PengYunQuan for Validity Cell Range Picker
//SFX_IMPL_MODELESSDIALOG(ScValidityRefChildWin, SID_VALIDITY_REFERENCE )
SFX_IMPL_CHILDWINDOW(ScValidityRefChildWin, SID_VALIDITY_REFERENCE)
SfxChildWinInfo __EXPORT ScValidityRefChildWin::GetInfo() const
{
    SfxChildWinInfo anInfo = SfxChildWindow::GetInfo();

    if( Window *pWnd = GetWindow() )
    {
        anInfo.aSize  = pWnd->GetSizePixel();

        if( pWnd->IsDialog() )
            if ( static_cast<Dialog*>(pWnd)->IsRollUp() )
                anInfo.nFlags |= SFX_CHILDWIN_ZOOMIN;
    }

    return anInfo;
}

namespace { ScTabViewShell * lcl_GetTabViewShell( SfxBindings *pBindings ); }
//<!--Added by PengYunQuan for Validity Cell Range Picker

#define IMPL_CHILD_CTOR(Class,sid) \
    Class::Class( Window*               pParentP,                   \
                    sal_uInt16              nId,                        \
                    SfxBindings*        p,                          \
                    SfxChildWinInfo*    pInfo )                     \
        : SfxChildWindow(pParentP, nId)                             \
    {                                                               \
        /*//<!--Added by PengYunQuan for Validity Cell Range Picker*/\
        /************************************************************************************/\
        /*      When a new document is creating, the SfxViewFrame may be ready,             */\
        /*      But the ScTabViewShell may have not been activated yet. In this             */\
        /*      situation, SfxViewShell::Current() does not get the correct shell,          */\
        /*      and we should lcl_GetTabViewShell( p ) instead of SfxViewShell::Current()   */\
        /************************************************************************************/\
        ScTabViewShell* pViewShell = lcl_GetTabViewShell( p );      \
        /*//-->Added by PengYunQuan for Validity Cell Range Picker*/\
        if (!pViewShell)                                            \
            pViewShell = dynamic_cast< ScTabViewShell* >( SfxViewShell::Current() ); \
        DBG_ASSERT( pViewShell, "missing view shell :-(" );         \
        pWindow = pViewShell ?                                      \
            pViewShell->CreateRefDialog( p, this, pInfo, pParentP, sid ) : NULL;    \
        if (pViewShell && !pWindow)                                             \
            pViewShell->GetViewFrame()->SetChildWindow( nId, sal_False );           \
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

static sal_Bool     bScSimpleRefFlag;
static long     nScSimpleRefHeight;
static long     nScSimpleRefWidth;
static long     nScSimpleRefX;
static long     nScSimpleRefY;
static sal_Bool     bAutoReOpen=sal_True;

ScSimpleRefDlgWrapper::ScSimpleRefDlgWrapper( Window* pParentP,
                                sal_uInt16              nId,
                                SfxBindings*        p,
                                SfxChildWinInfo*    pInfo )
        : SfxChildWindow(pParentP, nId)
{
//  ScTabViewShell* pViewShell =
//      dynamic_cast< ScTabViewShell* >( SfxViewShell::Current() );

    ScTabViewShell* pViewShell = NULL;
    SfxDispatcher* pDisp = p->GetDispatcher();
    if ( pDisp )
    {
        SfxViewFrame* pViewFrm = pDisp->GetFrame();
        if ( pViewFrm )
            pViewShell = dynamic_cast< ScTabViewShell* >( pViewFrm->GetViewShell() );
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
        SC_MOD()->SetRefDialog( nId, sal_False );
    }
}

void ScSimpleRefDlgWrapper::SetDefaultPosSize(Point aPos, Size aSize, sal_Bool bSet)
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

void ScSimpleRefDlgWrapper::SetAutoReOpen(sal_Bool bFlag)
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

void ScSimpleRefDlgWrapper::SetFlags( sal_Bool bCloseOnButtonUp, sal_Bool bSingleCell, sal_Bool bMultiSelection )
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
                                            sal_uInt16 nId,
                                            SfxBindings* pBindings,
                                            SfxChildWinInfo* pInfo ) :
                                            SfxChildWindow( pParentP, nId )
{
        ScTabViewShell* pViewShell = dynamic_cast< ScTabViewShell* >( SfxViewShell::Current() );
        DBG_ASSERT( pViewShell, "missing view shell :-(" );
        pWindow = pViewShell ?
            new ScAcceptChgDlg( pBindings, this, pParentP, pViewShell->GetViewData() ) :
            NULL;
        if(pWindow!=NULL)
        {
            ((ScAcceptChgDlg*)pWindow)->Initialize( pInfo );
        }
        if (pViewShell && !pWindow)
            pViewShell->GetViewFrame()->SetChildWindow( nId, sal_False );
}

void ScAcceptChgDlgWrapper::ReInitDlg()
{
    ScTabViewShell* pViewShell = dynamic_cast< ScTabViewShell* >( SfxViewShell::Current() );
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


//<!--Added by PengYunQuan for Validity Cell Range Picker
namespace
{
    ScTabViewShell * lcl_GetTabViewShell( SfxBindings *pBindings )
    {
        if( pBindings )
            if( SfxDispatcher* pDisp = pBindings ->GetDispatcher() )
                if( SfxViewFrame *pFrm = pDisp->GetFrame() )
                    if( SfxViewShell* pViewSh = pFrm->GetViewShell() )
                        return dynamic_cast<ScTabViewShell*>( pViewSh );

        return NULL;
    }
}

ScValidityRefChildWin::ScValidityRefChildWin( Window*               pParentP,                   \
                                             sal_uInt16             nId,                        \
                                             SfxBindings*       p,                          \
                                             SfxChildWinInfo*   /*pInfo*/ )                     \
                                             : SfxChildWindow(pParentP, nId),
                                             m_bVisibleLock( false ),
                                             m_bFreeWindowLock( false ),
                                             m_pSavedWndParent( NULL )
{
    SetWantsFocus( sal_False );\
        ScTabViewShell* pViewShell =                                \
            NULL != ( pWindow =  ScValidationDlg::Find1AliveObject( pParentP ) ) ? static_cast<ScValidationDlg*>(pWindow)->GetTabViewShell() :
            lcl_GetTabViewShell( p );
        if (!pViewShell)
            pViewShell = dynamic_cast< ScTabViewShell* >( SfxViewShell::Current() );
        DBG_ASSERT( pViewShell, "missing view shell :-(" );         \
        if (pViewShell && !pWindow)                                             \
            pViewShell->GetViewFrame()->SetChildWindow( nId, sal_False );           \
        else if( pWindow /*&& dynamic_cast< ScValidationDlg* >(pWindow)*/ )
        {}//pWindow = new Window( pParentP, WB_HIDE );

    if( pWindow ) m_pSavedWndParent = pWindow->GetParent();
}

ScValidityRefChildWin::~ScValidityRefChildWin()
{
    if( pWindow ) pWindow->SetParent( m_pSavedWndParent );

    if( m_bFreeWindowLock )
        pWindow = NULL;
}
//-->Added by PengYunQuan for Validity Cell Range Picker
