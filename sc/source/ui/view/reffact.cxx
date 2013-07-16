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
#include "validate.hxx"

// -----------------------------------------------------------------------

SFX_IMPL_MODELESSDIALOG_WITHID(ScNameDlgWrapper, FID_DEFINE_NAME )
SFX_IMPL_MODELESSDIALOG_WITHID(ScNameDefDlgWrapper, FID_ADD_NAME )
SFX_IMPL_MODELESSDIALOG_WITHID(ScSolverDlgWrapper, SID_OPENDLG_SOLVE )
SFX_IMPL_MODELESSDIALOG_WITHID(ScOptSolverDlgWrapper, SID_OPENDLG_OPTSOLVER )
SFX_IMPL_MODELESSDIALOG_WITHID(ScXMLSourceDlgWrapper, SID_MANAGE_XML_SOURCE)
SFX_IMPL_MODELESSDIALOG_WITHID(ScPivotLayoutWrapper, SID_OPENDLG_PIVOTTABLE )
SFX_IMPL_MODELESSDIALOG_WITHID(ScTabOpDlgWrapper, SID_OPENDLG_TABOP )
SFX_IMPL_MODELESSDIALOG_WITHID(ScFilterDlgWrapper, SID_FILTER )
SFX_IMPL_MODELESSDIALOG_WITHID(ScSpecialFilterDlgWrapper, SID_SPECIAL_FILTER )
SFX_IMPL_MODELESSDIALOG_WITHID(ScDbNameDlgWrapper, SID_DEFINE_DBNAME )
SFX_IMPL_MODELESSDIALOG_WITHID(ScConsolidateDlgWrapper, SID_OPENDLG_CONSOLIDATE )
SFX_IMPL_MODELESSDIALOG_WITHID(ScPrintAreasDlgWrapper, SID_OPENDLG_EDIT_PRINTAREA )
SFX_IMPL_MODELESSDIALOG_WITHID(ScColRowNameRangesDlgWrapper, SID_DEFINE_COLROWNAMERANGES )
SFX_IMPL_MODELESSDIALOG_WITHID(ScFormulaDlgWrapper, SID_OPENDLG_FUNCTION )
SFX_IMPL_MODELESSDIALOG_WITHID(ScAcceptChgDlgWrapper, FID_CHG_ACCEPT )
SFX_IMPL_MODELESSDIALOG_WITHID(ScHighlightChgDlgWrapper, FID_CHG_SHOW )
SFX_IMPL_MODELESSDIALOG_WITHID(ScSimpleRefDlgWrapper, WID_SIMPLE_REF )

SFX_IMPL_MODELESSDIALOG_WITHID(ScRandomNumberGeneratorDialogWrapper, SID_OPENDLG_RANDOM_NUMBER_GENERATOR )
SFX_IMPL_MODELESSDIALOG_WITHID(ScSamplingDialogWrapper, SID_SAMPLING_DIALOG )

SFX_IMPL_CHILDWINDOW_WITHID(ScValidityRefChildWin, SID_VALIDITY_REFERENCE)

SfxChildWinInfo ScValidityRefChildWin::GetInfo() const
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

namespace
{
    ScTabViewShell* lcl_GetTabViewShell( SfxBindings* pBindings );
}

#define IMPL_CHILD_CTOR(Class,sid) \
    Class::Class( Window*               pParentP,                   \
                    sal_uInt16              nId,                        \
                    SfxBindings*        p,                          \
                    SfxChildWinInfo*    pInfo )                     \
        : SfxChildWindow(pParentP, nId)                             \
    {                                                               \
        /************************************************************************************/\
        /*      When a new document is creating, the SfxViewFrame may be ready,             */\
        /*      But the ScTabViewShell may have not been activated yet. In this             */\
        /*      situation, SfxViewShell::Current() does not get the correct shell,          */\
        /*      and we should lcl_GetTabViewShell( p ) instead of SfxViewShell::Current()   */\
        /************************************************************************************/\
        ScTabViewShell* pViewShell = lcl_GetTabViewShell( p );      \
        if (!pViewShell)                                            \
            pViewShell = PTR_CAST( ScTabViewShell, SfxViewShell::Current() ); \
        OSL_ENSURE( pViewShell, "missing view shell :-(" );         \
        pWindow = pViewShell ?                                      \
            pViewShell->CreateRefDialog( p, this, pInfo, pParentP, sid ) : NULL;    \
        if (pViewShell && !pWindow)                                             \
            pViewShell->GetViewFrame()->SetChildWindow( nId, false );           \
    }

IMPL_CHILD_CTOR( ScNameDlgWrapper, FID_DEFINE_NAME )

IMPL_CHILD_CTOR( ScNameDefDlgWrapper, FID_ADD_NAME )

IMPL_CHILD_CTOR( ScSolverDlgWrapper, SID_OPENDLG_SOLVE )

IMPL_CHILD_CTOR( ScOptSolverDlgWrapper, SID_OPENDLG_OPTSOLVER )

IMPL_CHILD_CTOR( ScXMLSourceDlgWrapper, SID_MANAGE_XML_SOURCE)

IMPL_CHILD_CTOR( ScPivotLayoutWrapper, SID_OPENDLG_PIVOTTABLE )

IMPL_CHILD_CTOR( ScTabOpDlgWrapper, SID_OPENDLG_TABOP )

IMPL_CHILD_CTOR( ScFilterDlgWrapper, SID_FILTER )

IMPL_CHILD_CTOR( ScSpecialFilterDlgWrapper, SID_SPECIAL_FILTER )

IMPL_CHILD_CTOR( ScDbNameDlgWrapper, SID_DEFINE_DBNAME )

IMPL_CHILD_CTOR( ScColRowNameRangesDlgWrapper, SID_DEFINE_COLROWNAMERANGES )

IMPL_CHILD_CTOR( ScConsolidateDlgWrapper, SID_OPENDLG_CONSOLIDATE )

IMPL_CHILD_CTOR( ScPrintAreasDlgWrapper, SID_OPENDLG_EDIT_PRINTAREA )

IMPL_CHILD_CTOR( ScFormulaDlgWrapper, SID_OPENDLG_FUNCTION )

IMPL_CHILD_CTOR( ScRandomNumberGeneratorDialogWrapper, SID_OPENDLG_RANDOM_NUMBER_GENERATOR )

IMPL_CHILD_CTOR( ScSamplingDialogWrapper, SID_SAMPLING_DIALOG )

//-------------------------------------------------------------------------
// ScSimpleRefDlgWrapper
//-------------------------------------------------------------------------

static sal_Bool     bScSimpleRefFlag;
static long         nScSimpleRefHeight;
static long         nScSimpleRefWidth;
static long         nScSimpleRefX;
static long         nScSimpleRefY;
static sal_Bool     bAutoReOpen=sal_True;

ScSimpleRefDlgWrapper::ScSimpleRefDlgWrapper( Window* pParentP,
                                sal_uInt16              nId,
                                SfxBindings*        p,
                                SfxChildWinInfo*    pInfo )
        : SfxChildWindow(pParentP, nId)
{

    ScTabViewShell* pViewShell = NULL;
    SfxDispatcher* pDisp = p->GetDispatcher();
    if ( pDisp )
    {
        SfxViewFrame* pViewFrm = pDisp->GetFrame();
        if ( pViewFrm )
            pViewShell = PTR_CAST( ScTabViewShell, pViewFrm->GetViewShell() );
    }

    OSL_ENSURE( pViewShell, "missing view shell :-(" );

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
        SC_MOD()->SetRefDialog( nId, false );
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
// ScAcceptChgDlgWrapper //FIXME: should be moved into ViewShell
//-------------------------------------------------------------------------

ScAcceptChgDlgWrapper::ScAcceptChgDlgWrapper(   Window* pParentP,
                                            sal_uInt16 nId,
                                            SfxBindings* pBindings,
                                            SfxChildWinInfo* pInfo ) :
                                            SfxChildWindow( pParentP, nId )
{
        ScTabViewShell* pViewShell =
            PTR_CAST( ScTabViewShell, SfxViewShell::Current() );
        OSL_ENSURE( pViewShell, "missing view shell :-(" );
        pWindow = pViewShell ?
            new ScAcceptChgDlg( pBindings, this, pParentP, pViewShell->GetViewData() ) :
            NULL;
        if(pWindow!=NULL)
        {
            ((ScAcceptChgDlg*)pWindow)->Initialize( pInfo );
        }
        if (pViewShell && !pWindow)
            pViewShell->GetViewFrame()->SetChildWindow( nId, false );
}

void ScAcceptChgDlgWrapper::ReInitDlg()
{
    ScTabViewShell* pViewShell =
        PTR_CAST( ScTabViewShell, SfxViewShell::Current() );
    OSL_ENSURE( pViewShell, "missing view shell :-(" );

    if(pWindow!=NULL && pViewShell)
    {
        ((ScAcceptChgDlg*)pWindow)->ReInit(pViewShell->GetViewData());
    }
}

//-------------------------------------------------------------------------
// ScHighlightChgDlgWrapper
//-------------------------------------------------------------------------

IMPL_CHILD_CTOR( ScHighlightChgDlgWrapper, FID_CHG_SHOW )


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
    SetWantsFocus( false );\
        ScTabViewShell* pViewShell =                                \
            NULL != ( pWindow =  ScValidationDlg::Find1AliveObject( pParentP ) ) ? static_cast<ScValidationDlg*>(pWindow)->GetTabViewShell() :
            lcl_GetTabViewShell( p );
        if (!pViewShell)
            pViewShell = PTR_CAST( ScTabViewShell, SfxViewShell::Current() );
        OSL_ENSURE( pViewShell, "missing view shell :-(" );         \
        if (pViewShell && !pWindow)                                             \
            pViewShell->GetViewFrame()->SetChildWindow( nId, false );           \

    if( pWindow ) m_pSavedWndParent = pWindow->GetParent();
}

ScValidityRefChildWin::~ScValidityRefChildWin()
{
    if( pWindow ) pWindow->SetParent( m_pSavedWndParent );

    if( m_bFreeWindowLock )
        pWindow = NULL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
