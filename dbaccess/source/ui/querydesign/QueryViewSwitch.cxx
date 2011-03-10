/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"
#include "QueryViewSwitch.hxx"
#include "QueryDesignView.hxx"
#include "QueryTextView.hxx"
#include "querycontainerwindow.hxx"
#include "dbu_qry.hrc"
#include "browserids.hxx"
#include "adtabdlg.hxx"
#include "querycontroller.hxx"
#include "sqledit.hxx"
#include "querycontainerwindow.hxx"

using namespace dbaui;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

DBG_NAME(OQueryViewSwitch)
OQueryViewSwitch::OQueryViewSwitch(OQueryContainerWindow* _pParent, OQueryController& _rController,const Reference< XMultiServiceFactory >& _rFactory)
: m_bAddTableDialogWasVisible(sal_False)
{
    DBG_CTOR(OQueryViewSwitch,NULL);

    m_pTextView     = new OQueryTextView(_pParent);
    m_pDesignView   = new OQueryDesignView( _pParent, _rController, _rFactory );
}
// -----------------------------------------------------------------------------
OQueryViewSwitch::~OQueryViewSwitch()
{
    DBG_DTOR(OQueryViewSwitch,NULL);
    {
        ::std::auto_ptr<Window> aTemp(m_pTextView);
        m_pTextView = NULL;
    }
    {
        ::std::auto_ptr<Window> aTemp(m_pDesignView);
        m_pDesignView = NULL;
    }
}
// -------------------------------------------------------------------------
void OQueryViewSwitch::Construct()
{
    m_pDesignView->Construct( );
}
// -----------------------------------------------------------------------------
void OQueryViewSwitch::initialize()
{
    // initially be in SQL mode
    m_pTextView->Show();
    m_pDesignView->initialize();
}
// -------------------------------------------------------------------------
void OQueryViewSwitch::resizeDocumentView(Rectangle& _rPlayground)
{
    m_pTextView->SetPosSizePixel( _rPlayground.TopLeft(), _rPlayground.GetSize() );
    m_pDesignView->SetPosSizePixel( _rPlayground.TopLeft(), _rPlayground.GetSize() );

    // just for completeness: there is no space left, we occupied it all ...
    _rPlayground.SetPos( _rPlayground.BottomRight() );
    _rPlayground.SetSize( Size( 0, 0 ) );
}
// -----------------------------------------------------------------------------
sal_Bool OQueryViewSwitch::checkStatement()
{
    if(m_pTextView->IsVisible())
        return m_pTextView->checkStatement();
    return m_pDesignView->checkStatement();
}
// -----------------------------------------------------------------------------
::rtl::OUString OQueryViewSwitch::getStatement()
{
    if(m_pTextView->IsVisible())
        return m_pTextView->getStatement();
    return m_pDesignView->getStatement();
}
// -----------------------------------------------------------------------------
void OQueryViewSwitch::setReadOnly(sal_Bool _bReadOnly)
{
    if(m_pTextView->IsVisible())
        m_pTextView->setReadOnly(_bReadOnly);
    else
        m_pDesignView->setReadOnly(_bReadOnly);
}
// -----------------------------------------------------------------------------
void OQueryViewSwitch::clear()
{
    if(m_pTextView->IsVisible())
        m_pTextView->clear();
    else
        m_pDesignView->clear();
}
// -----------------------------------------------------------------------------
void OQueryViewSwitch::GrabFocus()
{
    if ( m_pTextView && m_pTextView->IsVisible() )
        m_pTextView->GrabFocus();
    else if ( m_pDesignView && m_pDesignView->IsVisible() )
        m_pDesignView->GrabFocus();
}
// -----------------------------------------------------------------------------
void OQueryViewSwitch::setStatement(const ::rtl::OUString& _rsStatement)
{
    if(m_pTextView->IsVisible())
        m_pTextView->setStatement(_rsStatement);
    else
        m_pDesignView->setStatement(_rsStatement);
}
// -----------------------------------------------------------------------------
void OQueryViewSwitch::copy()
{
    if(m_pTextView->IsVisible())
        m_pTextView->copy();
    else
        m_pDesignView->copy();
}
// -----------------------------------------------------------------------------
sal_Bool OQueryViewSwitch::isCutAllowed()
{
    if(m_pTextView->IsVisible())
        return m_pTextView->isCutAllowed();
    return m_pDesignView->isCutAllowed();
}
// -----------------------------------------------------------------------------
sal_Bool OQueryViewSwitch::isCopyAllowed()
{
    if(m_pTextView->IsVisible())
        return m_pTextView->isCopyAllowed();
    return m_pDesignView->isCopyAllowed();
}
// -----------------------------------------------------------------------------
sal_Bool OQueryViewSwitch::isPasteAllowed()
{
    if(m_pTextView->IsVisible())
        return m_pTextView->isPasteAllowed();
    return m_pDesignView->isPasteAllowed();
}
// -----------------------------------------------------------------------------
void OQueryViewSwitch::cut()
{
    if(m_pTextView->IsVisible())
        m_pTextView->cut();
    else
        m_pDesignView->cut();
}
// -----------------------------------------------------------------------------
void OQueryViewSwitch::paste()
{
    if(m_pTextView->IsVisible())
        m_pTextView->paste();
    else
        m_pDesignView->paste();
}
// -----------------------------------------------------------------------------
OQueryContainerWindow* OQueryViewSwitch::getContainer() const
{
    Window* pDesignParent = getDesignView() ? getDesignView()->GetParent() : NULL;
    return static_cast< OQueryContainerWindow* >( pDesignParent );
}

// -----------------------------------------------------------------------------
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

// -----------------------------------------------------------------------------
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

// -----------------------------------------------------------------------------
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

// -----------------------------------------------------------------------------
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

// -----------------------------------------------------------------------------
OAddTableDlg* OQueryViewSwitch::getAddTableDialog()
{
    if ( !m_pDesignView )
        return NULL;
    return m_pDesignView->getController().getAddTableDialog();
}
// -----------------------------------------------------------------------------
sal_Bool OQueryViewSwitch::isSlotEnabled(sal_Int32 _nSlotId)
{
    return m_pDesignView->isSlotEnabled(_nSlotId);
}
// -----------------------------------------------------------------------------
void OQueryViewSwitch::setSlotEnabled(sal_Int32 _nSlotId,sal_Bool _bEnable)
{
    m_pDesignView->setSlotEnabled(_nSlotId,_bEnable);
}
// -----------------------------------------------------------------------------
void OQueryViewSwitch::SaveUIConfig()
{
    if(m_pDesignView->IsVisible())
        m_pDesignView->SaveUIConfig();
}
// -----------------------------------------------------------------------------
void OQueryViewSwitch::SetPosSizePixel( Point _rPt,Size _rSize)
{
    m_pDesignView->SetPosSizePixel( _rPt,_rSize);
    m_pDesignView->Resize();
    m_pTextView->SetPosSizePixel( _rPt,_rSize);
}
// -----------------------------------------------------------------------------
Reference< XMultiServiceFactory > OQueryViewSwitch::getORB() const
{
    return m_pDesignView->getORB();
}
// -----------------------------------------------------------------------------
bool OQueryViewSwitch::reset( ::dbtools::SQLExceptionInfo* _pErrorInfo )
{
    m_pDesignView->reset();
    if ( !m_pDesignView->initByParseIterator( _pErrorInfo ) )
        return false;

    if ( switchView( _pErrorInfo ) )
        return false;

    return true;
}
// -----------------------------------------------------------------------------
void OQueryViewSwitch::setNoneVisbleRow(sal_Int32 _nRows)
{
    if(m_pDesignView)
        m_pDesignView->setNoneVisbleRow(_nRows);
}
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
