/*************************************************************************
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

#include "precompiled_svtools.hxx"

#include "wizardpagecontroller.hxx"
#include "wizardshell.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/awt/XControl.hpp>
/** === end UNO includes === **/

#include <toolkit/helper/vclunohelper.hxx>
#include <tools/diagnose_ex.h>

//......................................................................................................................
namespace svt { namespace uno
{
//......................................................................................................................

    /** === begin UNO using === **/
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
    /** === end UNO using === **/
    using namespace ::com::sun::star;

    //==================================================================================================================
    //= WizardPageController
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
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

    //------------------------------------------------------------------------------------------------------------------
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

    //------------------------------------------------------------------------------------------------------------------
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

    //------------------------------------------------------------------------------------------------------------------
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

    //------------------------------------------------------------------------------------------------------------------
    sal_Bool WizardPageController::commitPage( WizardTypes::CommitPageReason i_eReason )
    {
        if ( !m_xWizardPage.is() )
            return sal_True;

        try
        {
            return m_xWizardPage->commitPage( WizardShell::convertCommitReasonToTravelType( i_eReason ) );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        return sal_True;
    }

    //------------------------------------------------------------------------------------------------------------------
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

//......................................................................................................................
} } // namespace svt::uno
//......................................................................................................................
