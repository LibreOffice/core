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

#pragma once

#include <com/sun/star/ui/dialogs/XWizardController.hpp>
#include <vcl/roadmapwizardmachine.hxx>
#include <map>
#include <memory>

using vcl::WizardTypes::WizardState;
using vcl::WizardTypes::CommitPageReason;

namespace svt::uno
{


    class WizardPageController;
    typedef std::shared_ptr< WizardPageController > PWizardPageController;


    //= WizardShell

    typedef ::vcl::RoadmapWizardMachine WizardShell_Base;
    class WizardShell : public WizardShell_Base
    {
    public:
        WizardShell(
            weld::Window* pParent,
            const css::uno::Reference< css::ui::dialogs::XWizardController >& i_rController,
            const css::uno::Sequence< css::uno::Sequence< sal_Int16 > >& i_rPaths
        );

        // Dialog overridables
        virtual short   run() override;

        // OWizardMachine overridables
        virtual std::unique_ptr<BuilderPage> createPage( WizardState i_nState ) override;
        virtual void        enterState( WizardState i_nState ) override;
        virtual bool        leaveState( WizardState i_nState ) override;
        virtual OUString    getStateDisplayName( WizardState i_nState ) const override;
        virtual bool        canAdvance() const override;
        virtual bool        onFinish() override;
        virtual vcl::IWizardPageController* getPageController(BuilderPage* pCurrentPage) const override;

        static sal_Int16 convertCommitReasonToTravelType( const CommitPageReason i_eReason );

        // operations
        bool    advanceTo( const sal_Int16 i_nPageId )
        {
            return skipUntil( impl_pageIdToState( i_nPageId ) );
        }
        bool    goBackTo( const sal_Int16 i_nPageId )
        {
            return skipBackwardUntil( impl_pageIdToState( i_nPageId ) );
        }
        using WizardShell_Base::travelNext;
        using WizardShell_Base::travelPrevious;

        void        activatePath( const sal_Int16 i_nPathID, const bool i_bFinal )
        {
            WizardShell_Base::activatePath( vcl::RoadmapWizardTypes::PathId( i_nPathID ), i_bFinal );
        }

        css::uno::Reference< css::ui::dialogs::XWizardPage >
                    getCurrentWizardPage() const;

        sal_Int16   getCurrentPage() const
        {
            return impl_stateToPageId( getCurrentState() );
        }

        void        enablePage( const sal_Int16 i_PageID, const bool i_Enable );

        bool        knowsPage( const sal_Int16 i_nPageID ) const
        {
            return knowsState( impl_pageIdToState( i_nPageID ) );
        }

    private:
        sal_Int16   impl_stateToPageId( const WizardState i_nState ) const
        {
            return static_cast< sal_Int16 >( i_nState + m_nFirstPageID );
        }

        WizardState impl_pageIdToState( const sal_Int16 i_nPageId ) const
        {
            return static_cast<WizardState>(i_nPageId - m_nFirstPageID);
        }

        PWizardPageController impl_getController(BuilderPage* i_pPage) const;

        virtual OUString getPageIdentForState(WizardState nState) const override;
        virtual WizardState getStateFromPageIdent(const OUString& rIdent) const override;

        // prevent outside access to some base class members
        using WizardShell_Base::skip;
        using WizardShell_Base::skipUntil;
        using WizardShell_Base::skipBackwardUntil;
        using WizardShell_Base::getCurrentState;
        using WizardShell_Base::activatePath;

    private:
        typedef std::map<BuilderPage*, PWizardPageController> Page2ControllerMap;

        const css::uno::Reference< css::ui::dialogs::XWizardController >  m_xController;
        const sal_Int16                                                                             m_nFirstPageID;
        Page2ControllerMap                                                                          m_aPageControllers;
    };


} // namespace svt::uno



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
