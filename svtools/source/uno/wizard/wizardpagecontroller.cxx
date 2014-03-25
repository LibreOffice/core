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


#include "wizardpagecontroller.hxx"
#include "wizardshell.hxx"

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/awt/XControl.hpp>

#include <toolkit/helper/vclunohelper.hxx>
#include <tools/diagnose_ex.h>


namespace svt { namespace uno
{


    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::ui::dialogs::XWizardController;
    using ::com::sun::star::awt::XWindow;
    using ::com::sun::star::lang::XComponent;
    using ::com::sun::star::awt::XControl;

    using namespace ::com::sun::star;


    //= WizardPageController


    WizardPageController::WizardPageController( WizardShell& i_rParent, const Reference< XWizardController >& i_rController,
            const sal_Int16 i_nPageId )
        :m_xController( i_rController )
        ,m_xWizardPage()
        ,m_nPageId( i_nPageId )
    {
        ENSURE_OR_THROW( m_xController.is(), "no controller" );
        try
        {
            m_xWizardPage.set( m_xController->createPage(
                Reference< XWindow >( i_rParent.GetComponentInterface( sal_True ), UNO_QUERY_THROW ),
                m_nPageId
            ), UNO_SET_THROW );

            Reference< XWindow > xPageWindow( m_xWizardPage->getWindow(), UNO_SET_THROW );
            xPageWindow->setVisible( sal_True );

            TabPage* pTabPage( getTabPage() );
            if ( pTabPage )
                pTabPage->SetStyle( pTabPage->GetStyle() | WB_CHILDDLGCTRL | WB_DIALOGCONTROL );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }


    WizardPageController::~WizardPageController()
    {
        try
        {
            if ( m_xWizardPage.is() )
                m_xWizardPage->dispose();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }


    TabPage* WizardPageController::getTabPage() const
    {
        ENSURE_OR_RETURN( m_xWizardPage.is(), "WizardPageController::getTabPage: no external wizard page!", NULL );
        try
        {
            Reference< XWindow > xPageWindow( m_xWizardPage->getWindow(), UNO_SET_THROW );
            Window* pPageWindow = VCLUnoHelper::GetWindow( xPageWindow );
            if ( pPageWindow == NULL )
            {
                // windows created via the XContainerWindowProvider might be controls, not real windows, so resolve
                // that one indirection
                const Reference< XControl > xPageControl( m_xWizardPage->getWindow(), UNO_QUERY_THROW );
                xPageWindow.set( xPageControl->getPeer(), UNO_QUERY_THROW );
                pPageWindow = VCLUnoHelper::GetWindow( xPageWindow );
            }

            OSL_ENSURE( pPageWindow != NULL, "WizardPageController::getTabPage: unable to find the Window implementation for the page's window!" );
            return dynamic_cast< TabPage* >( pPageWindow );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return NULL;
    }


    void WizardPageController::initializePage()
    {
        if ( !m_xWizardPage.is() )
            return;

        try
        {
            m_xWizardPage->activatePage();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }


    bool WizardPageController::commitPage( WizardTypes::CommitPageReason i_eReason )
    {
        if ( !m_xWizardPage.is() )
            return true;

        try
        {
            return m_xWizardPage->commitPage( WizardShell::convertCommitReasonToTravelType( i_eReason ) );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        return true;
    }


    bool WizardPageController::canAdvance() const
    {
        if ( !m_xWizardPage.is() )
            return true;

        try
        {
            return m_xWizardPage->canAdvance();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        return true;
    }


} } // namespace svt::uno


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
