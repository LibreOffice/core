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

#include <comphelper/diagnose_ex.hxx>

#include <com/sun/star/ui/dialogs/WizardTravelType.hpp>

using vcl::RoadmapWizardTypes::WizardPath;

namespace svt::uno
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
            ENSURE_OR_THROW( i_rPaths.hasElements() && i_rPaths[0].hasElements(), "illegal paths" );
            return i_rPaths[0][0];
        }
    }

    //= WizardShell
    WizardShell::WizardShell(weld::Window* i_pParent, const Reference< XWizardController >& i_rController,
            const Sequence< Sequence< sal_Int16 > >& i_rPaths)
        :WizardShell_Base( i_pParent )
        ,m_xController( i_rController )
        ,m_nFirstPageID( lcl_determineFirstPageID( i_rPaths ) )
    {
        ENSURE_OR_THROW( m_xController.is(), "invalid controller" );

        // declare the paths
        for ( sal_Int32 i=0; i<i_rPaths.getLength(); ++i )
        {
            const Sequence< sal_Int16 >& rPath( i_rPaths[i] );
            WizardPath aPath( rPath.getLength() );
            std::transform(rPath.begin(), rPath.end(), aPath.begin(),
                [this](const sal_Int16 nPageId) -> WizardPath::value_type { return impl_pageIdToState(nPageId); });
            declarePath( i, aPath );
        }

        // create the first page, to know the page size
        GetOrCreatePage( impl_pageIdToState( i_rPaths[0][0] ) );
        m_xAssistant->set_current_page(0);

        // some defaults
        enableAutomaticNextButtonState();
    }

    short WizardShell::run()
    {
        ActivatePage();
        return WizardShell_Base::run();
    }

    OUString WizardShell::getPageIdentForState(WizardState nState) const
    {
        return OUString::number(impl_stateToPageId(nState));
    }

    WizardState WizardShell::getStateFromPageIdent(const OUString& rIdent) const
    {
        return impl_pageIdToState(rIdent.toInt32());
    }

    sal_Int16 WizardShell::convertCommitReasonToTravelType( const CommitPageReason i_eReason )
    {
        switch ( i_eReason )
        {
        case vcl::WizardTypes::eTravelForward:
            return WizardTravelType::FORWARD;

        case vcl::WizardTypes::eTravelBackward:
            return WizardTravelType::BACKWARD;

        case vcl::WizardTypes::eFinish:
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
            DBG_UNHANDLED_EXCEPTION("svtools.uno");
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
            DBG_UNHANDLED_EXCEPTION("svtools.uno");
        }

        return true;
    }


    PWizardPageController WizardShell::impl_getController(BuilderPage* i_pPage) const
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

    namespace
    {
        class EmptyPage : public BuilderPage
        {
        public:
            EmptyPage(weld::Widget* pParent, weld::DialogController* pController)
                : BuilderPage(pParent, pController, u"svt/ui/emptypage.ui"_ustr, u"EmptyPage"_ustr)
            {
                m_xContainer->set_size_request(m_xContainer->get_approximate_digit_width() * 70,
                                               m_xContainer->get_text_height() * 10);
            }
            weld::Container* GetContainer() const { return m_xContainer.get(); }
        };
    }

    std::unique_ptr<BuilderPage> WizardShell::createPage( WizardState i_nState )
    {
        ENSURE_OR_RETURN( m_xController.is(), "WizardShell::createPage: no WizardController!", nullptr );

        sal_Int16 nPageId = impl_stateToPageId(i_nState);

        OUString sIdent(OUString::number(nPageId));
        weld::Container* pPageContainer = m_xAssistant->append_page(sIdent);

        auto xPage = std::make_unique<EmptyPage>(pPageContainer, this);
        m_aPageControllers[xPage.get()] =
            std::make_shared<WizardPageController>(xPage->GetContainer(), m_xController, nPageId);

        return xPage;
    }

    vcl::IWizardPageController* WizardShell::getPageController(BuilderPage* i_pCurrentPage) const
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
            DBG_UNHANDLED_EXCEPTION("svtools.uno");
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
            DBG_UNHANDLED_EXCEPTION("svtools.uno");
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
            DBG_UNHANDLED_EXCEPTION("svtools.uno");
        }

        return WizardShell_Base::onFinish();
    }


} // namespace svt::uno


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
