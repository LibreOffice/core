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

#include "wizardshell.hxx"
#include "wizardpageshell.hxx"

#include <tools/diagnose_ex.h>

/** === begin UNO includes === **/
#include <com/sun/star/ui/dialogs/WizardTravelType.hpp>
/** === end UNO includes === **/

#include <vcl/msgbox.hxx>

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
    using ::com::sun::star::ui::dialogs::XWizard;
    /** === end UNO using === **/
    namespace WizardTravelType = ::com::sun::star::ui::dialogs::WizardTravelType;

    //==================================================================================================================
    namespace
    {
        //--------------------------------------------------------------------------------------------------------------
        sal_Int16 lcl_determineFirstPageID( const Sequence< Sequence< sal_Int16 > >& i_rPaths )
        {
            ENSURE_OR_THROW( ( i_rPaths.getLength() > 0 ) && ( i_rPaths[0].getLength() > 0 ), "illegal paths" );
            return i_rPaths[0][0];
        }
    }

    //==================================================================================================================
    //= WizardShell
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    WizardShell::WizardShell( Window* i_pParent, const Reference< XWizard >& i_rWizard, const Reference< XWizardController >& i_rController,
            const Sequence< Sequence< sal_Int16 > >& i_rPaths )
        :WizardShell_Base( i_pParent, WB_MOVEABLE | WB_CLOSEABLE )
        ,m_xWizard( i_rWizard )
        ,m_xController( i_rController )
        ,m_nFirstPageID( lcl_determineFirstPageID( i_rPaths ) )
    {
        ENSURE_OR_THROW( m_xWizard.is() && m_xController.is(), "invalid wizard/controller" );

        // declare the paths
        for ( sal_Int32 i=0; i<i_rPaths.getLength(); ++i )
        {
            const Sequence< sal_Int16 >& rPath( i_rPaths[i] );
            WizardPath aPath( rPath.getLength() );
            for ( sal_Int32 j=0; j<rPath.getLength(); ++j )
                aPath[j] = impl_pageIdToState( rPath[j] );
            declarePath( i, aPath );
        }

        // create the first page, to know the page size
        TabPage* pStartPage = GetOrCreatePage( impl_pageIdToState( i_rPaths[0][0] ) );
        SetPageSizePixel( pStartPage->GetSizePixel() );

        // some defaults
        ShowButtonFixedLine( true );
        SetRoadmapInteractive( true );
        enableAutomaticNextButtonState();

        // activate the first page
        ActivatePage();
    }

    //------------------------------------------------------------------------------------------------------------------
    WizardShell::~WizardShell()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    sal_Int16 WizardShell::convertCommitReasonToTravelType( const CommitPageReason i_eReason )
    {
        switch ( i_eReason )
        {
        case WizardTypes::eTravelForward:
            return WizardTravelType::FORWARD;

        case WizardTypes::eTravelBackward:
            return WizardTravelType::BACKWARD;

        case WizardTypes::eFinish:
            return WizardTravelType::FINISH;

        default:
            break;
        }
        OSL_ENSURE( false, "WizardShell::convertCommitReasonToTravelType: unsupported CommitPageReason!" );
        return WizardTravelType::FINISH;
    }

    //------------------------------------------------------------------------------------------------------------------
    void WizardShell::enterState( WizardState i_nState )
    {
        WizardShell_Base::enterState( i_nState );

        if ( !m_xController.is() )
            return;

        try
        {
            m_xController->onActivatePage( impl_stateToPageId( i_nState ) );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    sal_Bool WizardShell::prepareLeaveCurrentState( CommitPageReason i_eReason )
    {
        if ( !WizardShell_Base::prepareLeaveCurrentState( i_eReason ) )
            return sal_False;

        if ( !m_xController.is() )
            return sal_True;

        try
        {
            return m_xController->confirmDeactivatePage(
                impl_stateToPageId( getCurrentState() ),
                convertCommitReasonToTravelType( i_eReason )
            );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        return sal_True;
    }

    //------------------------------------------------------------------------------------------------------------------
    sal_Bool WizardShell::leaveState( WizardState i_nState )
    {
        if ( !WizardShell_Base::leaveState( i_nState ) )
            return sal_False;

        if ( !m_xController.is() )
            return sal_True;

        try
        {
            m_xController->onDeactivatePage( impl_stateToPageId( i_nState ) );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        return sal_True;
    }

    //------------------------------------------------------------------------------------------------------------------
    TabPage* WizardShell::createPage( WizardState i_nState )
    {
        ENSURE_OR_RETURN( m_xController.is(), "WizardShell::createPage: no WizardController!", NULL );
        return new WizardPageShell( *this, m_xController, impl_stateToPageId( i_nState ) );
    }

    //------------------------------------------------------------------------------------------------------------------
    String WizardShell::getStateDisplayName( WizardState i_nState ) const
    {
        try
        {
            if ( m_xController.is() )
                return m_xController->getPageTitle( impl_stateToPageId( i_nState ) );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return String::CreateFromInt32( i_nState );
    }

    //------------------------------------------------------------------------------------------------------------------
    bool WizardShell::canAdvance() const
    {
        try
        {
            if ( m_xController.is() && !m_xController->canAdvance() )
                return false;
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        return WizardShell_Base::canAdvance();
    }

    //------------------------------------------------------------------------------------------------------------------
    sal_Bool WizardShell::onFinish( sal_Int32 i_nResult )
    {
        try
        {
            if ( ( i_nResult == RET_OK ) && m_xController.is() && !m_xController->confirmFinish() )
                return sal_False;
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        return WizardShell_Base::onFinish( i_nResult );
    }

//......................................................................................................................
} } // namespace svt::uno
//......................................................................................................................
