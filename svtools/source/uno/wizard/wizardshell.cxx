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


#include "wizardshell.hxx"
#include "wizardpagecontroller.hxx"

#include <tools/diagnose_ex.h>

#include <com/sun/star/ui/dialogs/WizardTravelType.hpp>

#include <vcl/msgbox.hxx>


namespace svt { namespace uno
{


    using css::uno::Reference;
    using css::uno::Exception;
    using css::uno::Sequence;
    using css::ui::dialogs::XWizardController;
    using css::ui::dialogs::XWizardPage;

    namespace WizardTravelType = css::ui::dialogs::WizardTravelType;


    namespace
    {

        sal_Int16 lcl_determineFirstPageID( const Sequence< Sequence< sal_Int16 > >& i_rPaths )
        {
            ENSURE_OR_THROW( ( i_rPaths.getLength() > 0 ) && ( i_rPaths[0].getLength() > 0 ), "illegal paths" );
            return i_rPaths[0][0];
        }
    }


    //= WizardShell
    WizardShell::WizardShell( vcl::Window* i_pParent, const Reference< XWizardController >& i_rController,
            const Sequence< Sequence< sal_Int16 > >& i_rPaths )
        :WizardShell_Base( i_pParent, WB_MOVEABLE | WB_CLOSEABLE )
        ,m_xController( i_rController )
        ,m_nFirstPageID( lcl_determineFirstPageID( i_rPaths ) )
    {
        ENSURE_OR_THROW( m_xController.is(), "invalid controller" );

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
        SetRoadmapInteractive( true );
        enableAutomaticNextButtonState();
    }


    short WizardShell::Execute()
    {
        ActivatePage();
        return WizardShell_Base::Execute();
    }


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
        OSL_FAIL( "WizardShell::convertCommitReasonToTravelType: unsupported CommitPageReason!" );
        return WizardTravelType::FINISH;
    }


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


    bool WizardShell::leaveState( WizardState i_nState )
    {
        if ( !WizardShell_Base::leaveState( i_nState ) )
            return false;

        if ( !m_xController.is() )
            return true;

        try
        {
            m_xController->onDeactivatePage( impl_stateToPageId( i_nState ) );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        return true;
    }


    PWizardPageController WizardShell::impl_getController( TabPage* i_pPage ) const
    {
        Page2ControllerMap::const_iterator pos = m_aPageControllers.find( i_pPage );
        ENSURE_OR_RETURN( pos != m_aPageControllers.end(), "WizardShell::impl_getController: no controller for this page!", PWizardPageController() );
        return pos->second;
    }


    Reference< XWizardPage > WizardShell::getCurrentWizardPage() const
    {
        const WizardState eState = getCurrentState();

        PWizardPageController pController( impl_getController( GetPage( eState ) ) );
        ENSURE_OR_RETURN( pController, "WizardShell::getCurrentWizardPage: invalid page/controller!", nullptr );

        return pController->getWizardPage();
    }


    void WizardShell::enablePage( const sal_Int16 i_nPageID, const bool i_bEnable )
    {
        enableState( impl_pageIdToState( i_nPageID ), i_bEnable );
    }


    VclPtr<TabPage> WizardShell::createPage( WizardState i_nState )
    {
        ENSURE_OR_RETURN( m_xController.is(), "WizardShell::createPage: no WizardController!", nullptr );

        std::shared_ptr< WizardPageController > pController( new WizardPageController( *this, m_xController, impl_stateToPageId( i_nState ) ) );
        VclPtr<TabPage> pPage = pController->getTabPage();
        OSL_ENSURE( pPage, "WizardShell::createPage: illegal tab page!" );
        if ( !pPage )
        {
            // fallback for ill-behaved clients: empty page
            pPage = VclPtr<TabPage>::Create( this, 0 );
            pPage->SetSizePixel( LogicToPixel( Size( 280, 185 ), MAP_APPFONT ) );
        }

        m_aPageControllers[ pPage ] = pController;
        return pPage;
    }

    IWizardPageController* WizardShell::getPageController( TabPage* i_pCurrentPage ) const
    {
        return impl_getController( i_pCurrentPage ).get();
    }


    OUString WizardShell::getStateDisplayName( WizardState i_nState ) const
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
        // fallback for ill-behaved clients: the numeric state
        return OUString::number(i_nState);
    }


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


    bool WizardShell::onFinish()
    {
        try
        {
            if ( m_xController.is() && !m_xController->confirmFinish() )
                return false;
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        return WizardShell_Base::onFinish();
    }


} } // namespace svt::uno


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
