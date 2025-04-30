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

#include <sfx2/basedlgs.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>

#include <reffact.hxx>
#include <tabvwsh.hxx>
#include <sc.hrc>
#include <acredlin.hxx>
#include <simpref.hxx>
#include <scmod.hxx>
#include <scres.hrc>
#include <validate.hxx>

SFX_IMPL_MODELESSDIALOGCONTOLLER_WITHID(ScNameDlgWrapper, FID_DEFINE_NAME)
SFX_IMPL_MODELESSDIALOGCONTOLLER_WITHID(ScNameDefDlgWrapper, FID_ADD_NAME)
SFX_IMPL_MODELESSDIALOGCONTOLLER_WITHID(ScSolverDlgWrapper, SID_OPENDLG_SOLVE)
SFX_IMPL_MODELESSDIALOGCONTOLLER_WITHID(ScOptSolverDlgWrapper, SID_OPENDLG_OPTSOLVER)
SFX_IMPL_MODELESSDIALOGCONTOLLER_WITHID(ScXMLSourceDlgWrapper, SID_MANAGE_XML_SOURCE)
SFX_IMPL_MODELESSDIALOGCONTOLLER_WITHID(ScPivotLayoutWrapper, SID_OPENDLG_PIVOTTABLE)
SFX_IMPL_MODELESSDIALOGCONTOLLER_WITHID(ScTabOpDlgWrapper, SID_OPENDLG_TABOP)
SFX_IMPL_MODELESSDIALOGCONTOLLER_WITHID(ScFilterDlgWrapper, SID_FILTER)
SFX_IMPL_MODELESSDIALOGCONTOLLER_WITHID(ScSpecialFilterDlgWrapper, SID_SPECIAL_FILTER)
SFX_IMPL_MODELESSDIALOGCONTOLLER_WITHID(ScDbNameDlgWrapper, SID_DEFINE_DBNAME)
SFX_IMPL_MODELESSDIALOGCONTOLLER_WITHID(ScConsolidateDlgWrapper, SID_OPENDLG_CONSOLIDATE)
SFX_IMPL_MODELESSDIALOGCONTOLLER_WITHID(ScPrintAreasDlgWrapper, SID_OPENDLG_EDIT_PRINTAREA)
SFX_IMPL_MODELESSDIALOGCONTOLLER_WITHID(ScColRowNameRangesDlgWrapper, SID_DEFINE_COLROWNAMERANGES)
SFX_IMPL_MODELESSDIALOGCONTOLLER_WITHID(ScFormulaDlgWrapper, SID_OPENDLG_FUNCTION)
SFX_IMPL_MODELESSDIALOGCONTOLLER_WITHID(ScAcceptChgDlgWrapper, FID_CHG_ACCEPT)
SFX_IMPL_MODELESSDIALOGCONTOLLER_WITHID(ScHighlightChgDlgWrapper, FID_CHG_SHOW)
SFX_IMPL_MODELESSDIALOGCONTOLLER_WITHID(ScSimpleRefDlgWrapper, WID_SIMPLE_REF)
SFX_IMPL_MODELESSDIALOGCONTOLLER_WITHID(ScCondFormatDlgWrapper, WID_CONDFRMT_REF)

SFX_IMPL_CHILDWINDOW_WITHID(ScValidityRefChildWin, SID_VALIDITY_REFERENCE)

SfxChildWinInfo ScValidityRefChildWin::GetInfo() const
{
    SfxChildWinInfo aInfo = SfxChildWindow::GetInfo();
    return aInfo;
}

namespace
{
    ScTabViewShell* lcl_GetTabViewShell( const SfxBindings* pBindings );
}

#define IMPL_CONTROLLER_CHILD_CTOR(Class,sid) \
    Class::Class( vcl::Window*               pParentP,                   \
                    sal_uInt16              nId,                        \
                    SfxBindings*        p,                          \
                    SfxChildWinInfo*  pInfo )                     \
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
            pViewShell = dynamic_cast<ScTabViewShell*>( SfxViewShell::Current()  ); \
        OSL_ENSURE( pViewShell, "missing view shell :-(" );         \
        SetController( pViewShell ?                                      \
            pViewShell->CreateRefDialogController( p, this, pInfo, pParentP->GetFrameWeld(), sid ) : nullptr );    \
        if (pViewShell && !GetController())                                     \
            pViewShell->GetViewFrame().SetChildWindow( nId, false );           \
    }


IMPL_CONTROLLER_CHILD_CTOR( ScNameDlgWrapper, FID_DEFINE_NAME )

IMPL_CONTROLLER_CHILD_CTOR( ScNameDefDlgWrapper, FID_ADD_NAME )

IMPL_CONTROLLER_CHILD_CTOR( ScSolverDlgWrapper, SID_OPENDLG_SOLVE )

IMPL_CONTROLLER_CHILD_CTOR( ScOptSolverDlgWrapper, SID_OPENDLG_OPTSOLVER )

IMPL_CONTROLLER_CHILD_CTOR( ScXMLSourceDlgWrapper, SID_MANAGE_XML_SOURCE)

IMPL_CONTROLLER_CHILD_CTOR( ScPivotLayoutWrapper, SID_OPENDLG_PIVOTTABLE )

IMPL_CONTROLLER_CHILD_CTOR( ScTabOpDlgWrapper, SID_OPENDLG_TABOP )

IMPL_CONTROLLER_CHILD_CTOR( ScFilterDlgWrapper, SID_FILTER )

IMPL_CONTROLLER_CHILD_CTOR( ScSpecialFilterDlgWrapper, SID_SPECIAL_FILTER )

IMPL_CONTROLLER_CHILD_CTOR( ScDbNameDlgWrapper, SID_DEFINE_DBNAME )

IMPL_CONTROLLER_CHILD_CTOR( ScColRowNameRangesDlgWrapper, SID_DEFINE_COLROWNAMERANGES )

IMPL_CONTROLLER_CHILD_CTOR( ScConsolidateDlgWrapper, SID_OPENDLG_CONSOLIDATE )

IMPL_CONTROLLER_CHILD_CTOR( ScPrintAreasDlgWrapper, SID_OPENDLG_EDIT_PRINTAREA )

IMPL_CONTROLLER_CHILD_CTOR( ScFormulaDlgWrapper, SID_OPENDLG_FUNCTION )


// ScSimpleRefDlgWrapper

static bool         bScSimpleRefFlag;
static tools::Long         nScSimpleRefHeight;
static tools::Long         nScSimpleRefWidth;
static tools::Long         nScSimpleRefX;
static tools::Long         nScSimpleRefY;
static bool         bAutoReOpen = true;

ScSimpleRefDlgWrapper::ScSimpleRefDlgWrapper( vcl::Window* pParentP,
                                sal_uInt16              nId,
                                SfxBindings*        p,
                                SfxChildWinInfo*    pInfo )
        : SfxChildWindow(pParentP, nId)
{

    ScTabViewShell* pViewShell = nullptr;
    SfxDispatcher* pDisp = p->GetDispatcher();
    if ( pDisp )
    {
        SfxViewFrame* pViewFrm = pDisp->GetFrame();
        if ( pViewFrm )
            pViewShell = dynamic_cast<ScTabViewShell*>( pViewFrm->GetViewShell()  );
    }

    OSL_ENSURE( pViewShell, "missing view shell :-(" );

    if(pInfo!=nullptr && bScSimpleRefFlag)
    {
        pInfo->aPos.setX(nScSimpleRefX );
        pInfo->aPos.setY(nScSimpleRefY );
        pInfo->aSize.setHeight(nScSimpleRefHeight );
        pInfo->aSize.setWidth(nScSimpleRefWidth );
    }
    SetController(nullptr);

    if (bAutoReOpen && pViewShell)
        SetController(pViewShell->CreateRefDialogController(p, this, pInfo, pParentP->GetFrameWeld(), WID_SIMPLE_REF));

    if (!GetController())
    {
        ScModule::get()->SetRefDialog(nId, false);
    }
}

void ScSimpleRefDlgWrapper::SetAutoReOpen(bool bFlag)
{
    bAutoReOpen=bFlag;
}

void ScSimpleRefDlgWrapper::SetRefString(const OUString& rStr)
{
    auto xDlgController = GetController();
    if (xDlgController)
    {
        static_cast<ScSimpleRefDlg*>(xDlgController.get())->SetRefString(rStr);
    }
}

void ScSimpleRefDlgWrapper::SetCloseHdl( const Link<const OUString*,void>& rLink )
{
    auto xDlgController = GetController();
    if (xDlgController)
    {
        static_cast<ScSimpleRefDlg*>(xDlgController.get())->SetCloseHdl(rLink);
    }
}

void ScSimpleRefDlgWrapper::SetUnoLinks( const Link<const OUString&,void>& rDone,
                    const Link<const OUString&,void>& rAbort, const Link<const OUString&,void>& rChange )
{
    auto xDlgController = GetController();
    if (xDlgController)
    {
        static_cast<ScSimpleRefDlg*>(xDlgController.get())->SetUnoLinks( rDone, rAbort, rChange );
    }
}

void ScSimpleRefDlgWrapper::SetFlags( bool bCloseOnButtonUp, bool bSingleCell, bool bMultiSelection )
{
    auto xDlgController = GetController();
    if (xDlgController)
    {
        static_cast<ScSimpleRefDlg*>(xDlgController.get())->SetFlags( bCloseOnButtonUp, bSingleCell, bMultiSelection );
    }
}

void ScSimpleRefDlgWrapper::StartRefInput()
{
    auto xDlgController = GetController();
    if (xDlgController)
    {
        static_cast<ScSimpleRefDlg*>(xDlgController.get())->StartRefInput();
    }
}

// ScAcceptChgDlgWrapper //FIXME: should be moved into ViewShell

ScAcceptChgDlgWrapper::ScAcceptChgDlgWrapper(vcl::Window* pParentP,
                                            sal_uInt16 nId,
                                            SfxBindings* pBindings,
                                            SfxChildWinInfo* pInfo ) :
                                            SfxChildWindow( pParentP, nId )
{
    ScTabViewShell* pViewShell =
        dynamic_cast<ScTabViewShell*>( SfxViewShell::Current()  );
    OSL_ENSURE( pViewShell, "missing view shell :-(" );
    if (pViewShell)
    {
        auto xDlg = std::make_shared<ScAcceptChgDlg>(pBindings, this, pParentP->GetFrameWeld(), pViewShell->GetViewData());
        SetController(xDlg);
        pInfo->nFlags = SfxChildWindowFlags::NEVERHIDE;
        xDlg->Initialize( pInfo );
    }
    else
        SetController( nullptr );
    if (pViewShell && !GetController())
        pViewShell->GetViewFrame().SetChildWindow( nId, false );
}

void ScAcceptChgDlgWrapper::ReInitDlg()
{
    ScTabViewShell* pViewShell =
        dynamic_cast<ScTabViewShell*>( SfxViewShell::Current()  );
    OSL_ENSURE( pViewShell, "missing view shell :-(" );

    if (GetController() && pViewShell)
    {
        static_cast<ScAcceptChgDlg*>(GetController().get())->ReInit(pViewShell->GetViewData());
    }
}

// ScHighlightChgDlgWrapper

IMPL_CONTROLLER_CHILD_CTOR(ScHighlightChgDlgWrapper, FID_CHG_SHOW)

namespace
{
    ScTabViewShell * lcl_GetTabViewShell( const SfxBindings *pBindings )
    {
        if( pBindings )
            if( SfxDispatcher* pDisp = pBindings ->GetDispatcher() )
                if( SfxViewFrame *pFrm = pDisp->GetFrame() )
                    if( SfxViewShell* pViewSh = pFrm->GetViewShell() )
                        return dynamic_cast<ScTabViewShell*>( pViewSh );

        return nullptr;
    }
}

ScValidityRefChildWin::ScValidityRefChildWin(vcl::Window* pParentP,
                                             sal_uInt16 nId,
                                             const SfxBindings* p,
                                             SAL_UNUSED_PARAMETER SfxChildWinInfo* /*pInfo*/ )
                                             : SfxChildWindow(pParentP, nId)
                                             , m_bVisibleLock(false)
                                             , m_bFreeWindowLock(false)
{
    SetWantsFocus( false );
    std::shared_ptr<SfxDialogController> xDlg(ScValidationDlg::Find1AliveObject(pParentP->GetFrameWeld()));
    SetController(xDlg);
    ScTabViewShell* pViewShell;
    if (xDlg)
        pViewShell = static_cast<ScValidationDlg*>(xDlg.get())->GetTabViewShell();
    else
        pViewShell = lcl_GetTabViewShell( p );
    if (!pViewShell)
        pViewShell = dynamic_cast<ScTabViewShell*>( SfxViewShell::Current()  );
    OSL_ENSURE( pViewShell, "missing view shell :-(" );
    if (pViewShell && !xDlg)
        pViewShell->GetViewFrame().SetChildWindow( nId, false );
}

ScValidityRefChildWin::~ScValidityRefChildWin()
{
    if (m_bFreeWindowLock)
        SetController(nullptr);
}

IMPL_CONTROLLER_CHILD_CTOR( ScCondFormatDlgWrapper, WID_CONDFRMT_REF )

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
