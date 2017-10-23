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

#include <QueryViewSwitch.hxx>
#include <QueryDesignView.hxx>
#include <QueryTextView.hxx>
#include <querycontainerwindow.hxx>
#include <strings.hrc>
#include <browserids.hxx>
#include <adtabdlg.hxx>
#include <querycontroller.hxx>
#include <sqledit.hxx>

using namespace dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

OQueryViewSwitch::OQueryViewSwitch(OQueryContainerWindow* _pParent, OQueryController& _rController,const Reference< XComponentContext >& _rxContext)
: m_bAddTableDialogWasVisible(false)
{

    m_pTextView     = VclPtr<OQueryTextView>::Create(_pParent);
    m_pDesignView   = VclPtr<OQueryDesignView>::Create( _pParent, _rController, _rxContext );
}

OQueryViewSwitch::~OQueryViewSwitch()
{
    // destroy children
    m_pDesignView.disposeAndClear();
    m_pTextView.disposeAndClear();
}

void OQueryViewSwitch::Construct()
{
    m_pDesignView->Construct( );
}

void OQueryViewSwitch::initialize()
{
    // initially be in SQL mode
    m_pTextView->Show();
    m_pDesignView->initialize();
}

bool OQueryViewSwitch::checkStatement()
{
    if(m_pTextView->IsVisible())
        return true;
    return m_pDesignView->checkStatement();
}

OUString OQueryViewSwitch::getStatement()
{
    if(m_pTextView->IsVisible())
        return m_pTextView->getStatement();
    return m_pDesignView->getStatement();
}

void OQueryViewSwitch::clear()
{
    if(m_pTextView->IsVisible())
        m_pTextView->clear();
    else
        m_pDesignView->clear();
}

void OQueryViewSwitch::GrabFocus()
{
    if ( m_pTextView && m_pTextView->IsVisible() )
        m_pTextView->GrabFocus();
    else if ( m_pDesignView && m_pDesignView->IsVisible() )
        m_pDesignView->GrabFocus();
}

void OQueryViewSwitch::setStatement(const OUString& _rsStatement)
{
    if(m_pTextView->IsVisible())
        m_pTextView->setStatement(_rsStatement);
}

void OQueryViewSwitch::copy()
{
    if(m_pTextView->IsVisible())
        m_pTextView->copy();
    else
        m_pDesignView->copy();
}

bool OQueryViewSwitch::isCutAllowed()
{
    if(m_pTextView->IsVisible())
        return m_pTextView->isCutAllowed();
    return m_pDesignView->isCutAllowed();
}

bool OQueryViewSwitch::isCopyAllowed()
{
    if(m_pTextView->IsVisible())
        return true;
    return m_pDesignView->isCopyAllowed();
}

bool OQueryViewSwitch::isPasteAllowed()
{
    if(m_pTextView->IsVisible())
        return true;
    return m_pDesignView->isPasteAllowed();
}

void OQueryViewSwitch::cut()
{
    if(m_pTextView->IsVisible())
        m_pTextView->cut();
    else
        m_pDesignView->cut();
}

void OQueryViewSwitch::paste()
{
    if(m_pTextView->IsVisible())
        m_pTextView->paste();
    else
        m_pDesignView->paste();
}

OQueryContainerWindow* OQueryViewSwitch::getContainer() const
{
    vcl::Window* pDesignParent = getDesignView() ? getDesignView()->GetParent() : nullptr;
    return static_cast< OQueryContainerWindow* >( pDesignParent );
}

void OQueryViewSwitch::impl_forceSQLView()
{
    OAddTableDlg* pAddTabDialog( getAddTableDialog() );

    // hide the "Add Table" dialog
    m_bAddTableDialogWasVisible = pAddTabDialog && pAddTabDialog->IsVisible();
    if ( m_bAddTableDialogWasVisible )
        pAddTabDialog->Hide();

    // tell the views they're in/active
    m_pDesignView->stopTimer();
    m_pTextView->getSqlEdit()->startTimer();

    // set the most recent statement at the text view
    m_pTextView->clear();
    m_pTextView->setStatement(static_cast<OQueryController&>(m_pDesignView->getController()).getStatement());
}

void OQueryViewSwitch::forceInitialView()
{
    OQueryController& rQueryController( static_cast< OQueryController& >( m_pDesignView->getController() ) );
    const bool bGraphicalDesign = rQueryController.isGraphicalDesign();
    if ( !bGraphicalDesign )
        impl_forceSQLView();
    else
    {
        // tell the text view it's inactive now
        m_pTextView->getSqlEdit()->stopTimer();

        // update the "Add Table" dialog
        OAddTableDlg* pAddTabDialog( getAddTableDialog() );
        if ( pAddTabDialog )
            pAddTabDialog->Update();

        // initialize the design view
        m_pDesignView->initByFieldDescriptions( rQueryController.getFieldInformation() );

        // tell the design view it's active now
        m_pDesignView->startTimer();
    }

    impl_postViewSwitch( bGraphicalDesign, true );
}

bool OQueryViewSwitch::switchView( ::dbtools::SQLExceptionInfo* _pErrorInfo )
{
    bool bRet = true;
    bool bGraphicalDesign = static_cast<OQueryController&>(m_pDesignView->getController()).isGraphicalDesign();

    if ( !bGraphicalDesign )
    {
        impl_forceSQLView();
    }
    else
    {
        // tell the text view it's inactive now
        m_pTextView->getSqlEdit()->stopTimer();

        // update the "Add Table" dialog
        OAddTableDlg* pAddTabDialog( getAddTableDialog() );
        if ( pAddTabDialog )
            pAddTabDialog->Update();

        // initialize the design view
        bRet = m_pDesignView->initByParseIterator( _pErrorInfo );

        // tell the design view it's active now
        m_pDesignView->startTimer();
    }

    return impl_postViewSwitch( bGraphicalDesign, bRet );
}

bool OQueryViewSwitch::impl_postViewSwitch( const bool i_bGraphicalDesign, const bool i_bSuccess )
{
    if ( i_bSuccess )
    {
        m_pTextView->Show   ( !i_bGraphicalDesign );
        m_pDesignView->Show ( i_bGraphicalDesign );
        OAddTableDlg* pAddTabDialog( getAddTableDialog() );
        if ( pAddTabDialog )
            if ( i_bGraphicalDesign && m_bAddTableDialogWasVisible )
                pAddTabDialog->Show();

        GrabFocus();
    }

    OQueryContainerWindow* pContainer = getContainer();
    if ( pContainer )
        pContainer->Resize();

    m_pDesignView->getController().ClearUndoManager();
    m_pDesignView->getController().InvalidateAll();

    return i_bSuccess;
}

OAddTableDlg* OQueryViewSwitch::getAddTableDialog()
{
    if ( !m_pDesignView )
        return nullptr;
    return m_pDesignView->getController().getAddTableDialog();
}

bool OQueryViewSwitch::isSlotEnabled(sal_Int32 _nSlotId)
{
    return m_pDesignView->isSlotEnabled(_nSlotId);
}

void OQueryViewSwitch::setSlotEnabled(sal_Int32 _nSlotId, bool _bEnable)
{
    m_pDesignView->setSlotEnabled(_nSlotId,_bEnable);
}

void OQueryViewSwitch::SaveUIConfig()
{
    if(m_pDesignView->IsVisible())
        m_pDesignView->SaveUIConfig();
}

void OQueryViewSwitch::SetPosSizePixel( Point _rPt,Size _rSize)
{
    m_pDesignView->SetPosSizePixel( _rPt,_rSize);
    m_pDesignView->Resize();
    m_pTextView->SetPosSizePixel( _rPt,_rSize);
}

Reference< XComponentContext > OQueryViewSwitch::getORB() const
{
    return m_pDesignView->getORB();
}

bool OQueryViewSwitch::reset()
{
    m_pDesignView->reset();
    if ( !m_pDesignView->initByParseIterator( nullptr ) )
        return false;

    if ( switchView( nullptr ) )
        return false;

    return true;
}

void OQueryViewSwitch::setNoneVisbleRow(sal_Int32 _nRows)
{
    if(m_pDesignView)
        m_pDesignView->setNoneVisbleRow(_nRows);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
