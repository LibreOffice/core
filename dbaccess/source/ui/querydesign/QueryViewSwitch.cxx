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

#include "QueryViewSwitch.hxx"
#include "QueryDesignView.hxx"
#include "QueryTextView.hxx"
#include "querycontainerwindow.hxx"
#include "dbu_qry.hrc"
#include "browserids.hxx"
#include "adtabdlg.hxx"
#include "querycontroller.hxx"
#include "sqledit.hxx"

using namespace dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

DBG_NAME(OQueryViewSwitch)
OQueryViewSwitch::OQueryViewSwitch(OQueryContainerWindow* _pParent, OQueryController& _rController,const Reference< XComponentContext >& _rxContext)
: m_bAddTableDialogWasVisible(sal_False)
{
    DBG_CTOR(OQueryViewSwitch,NULL);

    m_pTextView     = new OQueryTextView(_pParent);
    m_pDesignView   = new OQueryDesignView( _pParent, _rController, _rxContext );
}

OQueryViewSwitch::~OQueryViewSwitch()
{
    DBG_DTOR(OQueryViewSwitch,NULL);
    {
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        ::std::auto_ptr<Window> aTemp(m_pTextView);
        SAL_WNODEPRECATED_DECLARATIONS_POP
        m_pTextView = NULL;
    }
    {
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        ::std::auto_ptr<Window> aTemp(m_pDesignView);
        SAL_WNODEPRECATED_DECLARATIONS_POP
        m_pDesignView = NULL;
    }
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

void OQueryViewSwitch::resizeDocumentView(Rectangle& _rPlayground)
{
    m_pTextView->SetPosSizePixel( _rPlayground.TopLeft(), _rPlayground.GetSize() );
    m_pDesignView->SetPosSizePixel( _rPlayground.TopLeft(), _rPlayground.GetSize() );

    // just for completeness: there is no space left, we occupied it all ...
    _rPlayground.SetPos( _rPlayground.BottomRight() );
    _rPlayground.SetSize( Size( 0, 0 ) );
}

sal_Bool OQueryViewSwitch::checkStatement()
{
    if(m_pTextView->IsVisible())
        return m_pTextView->checkStatement();
    return m_pDesignView->checkStatement();
}

OUString OQueryViewSwitch::getStatement()
{
    if(m_pTextView->IsVisible())
        return m_pTextView->getStatement();
    return m_pDesignView->getStatement();
}

void OQueryViewSwitch::setReadOnly(sal_Bool _bReadOnly)
{
    if(m_pTextView->IsVisible())
        m_pTextView->setReadOnly(_bReadOnly);
    else
        m_pDesignView->setReadOnly(_bReadOnly);
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
    else
        m_pDesignView->setStatement(_rsStatement);
}

void OQueryViewSwitch::copy()
{
    if(m_pTextView->IsVisible())
        m_pTextView->copy();
    else
        m_pDesignView->copy();
}

sal_Bool OQueryViewSwitch::isCutAllowed()
{
    if(m_pTextView->IsVisible())
        return m_pTextView->isCutAllowed();
    return m_pDesignView->isCutAllowed();
}

sal_Bool OQueryViewSwitch::isCopyAllowed()
{
    if(m_pTextView->IsVisible())
        return m_pTextView->isCopyAllowed();
    return m_pDesignView->isCopyAllowed();
}

sal_Bool OQueryViewSwitch::isPasteAllowed()
{
    if(m_pTextView->IsVisible())
        return m_pTextView->isPasteAllowed();
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
    Window* pDesignParent = getDesignView() ? getDesignView()->GetParent() : NULL;
    return static_cast< OQueryContainerWindow* >( pDesignParent );
}

void OQueryViewSwitch::impl_forceSQLView()
{
    OAddTableDlg* pAddTabDialog( getAddTableDialog() );

    // hide the "Add Table" dialog
    m_bAddTableDialogWasVisible = pAddTabDialog ? pAddTabDialog->IsVisible() : false;
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
    const sal_Bool bGraphicalDesign = rQueryController.isGraphicalDesign();
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
    sal_Bool bRet = sal_True;
    sal_Bool bGraphicalDesign = static_cast<OQueryController&>(m_pDesignView->getController()).isGraphicalDesign();

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
        return NULL;
    return m_pDesignView->getController().getAddTableDialog();
}

sal_Bool OQueryViewSwitch::isSlotEnabled(sal_Int32 _nSlotId)
{
    return m_pDesignView->isSlotEnabled(_nSlotId);
}

void OQueryViewSwitch::setSlotEnabled(sal_Int32 _nSlotId,sal_Bool _bEnable)
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

bool OQueryViewSwitch::reset( ::dbtools::SQLExceptionInfo* _pErrorInfo )
{
    m_pDesignView->reset();
    if ( !m_pDesignView->initByParseIterator( _pErrorInfo ) )
        return false;

    if ( switchView( _pErrorInfo ) )
        return false;

    return true;
}

void OQueryViewSwitch::setNoneVisbleRow(sal_Int32 _nRows)
{
    if(m_pDesignView)
        m_pDesignView->setNoneVisbleRow(_nRows);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
