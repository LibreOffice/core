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

#include "wizardpageshell.hxx"
#include "wizardshell.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/lang/XComponent.hpp>
/** === end UNO includes === **/

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
    /** === end UNO using === **/
    using namespace ::com::sun::star;

    //==================================================================================================================
    //= WizardPageShell
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    WizardPageShell::WizardPageShell( WizardShell& i_rParent, const Reference< XWizardController >& i_rController,
            const sal_Int16 i_nPageId )
        :OWizardPage( &i_rParent, WB_CHILDDLGCTRL | WB_NODIALOGCONTROL )
        ,m_xController( i_rController )
        ,m_xWizardPage()
        ,m_nPageId( i_nPageId )
    {
        ENSURE_OR_THROW( m_xController.is(), "no controller" );
        try
        {
            m_xWizardPage.set( m_xController->createPage(
                Reference< XWindow >( GetComponentInterface( TRUE ), UNO_QUERY_THROW ),
                m_nPageId
            ), UNO_SET_THROW );

            Reference< XWindow > xPageWindow( m_xWizardPage->getWindow(), UNO_SET_THROW );

            awt::Rectangle aContentRect( xPageWindow->getPosSize() );
            SetSizePixel( Size( aContentRect.X + aContentRect.Width, aContentRect.Y + aContentRect.Height ) );

            xPageWindow->setVisible( sal_True );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        Show();
    }

    //------------------------------------------------------------------------------------------------------------------
    WizardPageShell::~WizardPageShell()
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
    void WizardPageShell::initializePage()
    {
        OWizardPage::initializePage();
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
    sal_Bool WizardPageShell::commitPage( CommitPageReason i_eReason )
    {
        if ( !OWizardPage::commitPage( i_eReason ) )
            return sal_False;

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
    bool WizardPageShell::canAdvance() const
    {
        if ( !OWizardPage::canAdvance() )
            return false;

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
