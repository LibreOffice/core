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

#include <com/sun/star/awt/XControl.hpp>

#include <toolkit/helper/vclunohelper.hxx>
#include <tools/diagnose_ex.h>


namespace svt::uno
{


    using css::uno::Reference;
    using css::uno::UNO_SET_THROW;
    using css::uno::Exception;
    using css::ui::dialogs::XWizardController;
    using css::awt::XWindow;

    using namespace ::com::sun::star;


    //= WizardPageController


    WizardPageController::WizardPageController(weld::Container* pParent, const Reference< XWizardController >& i_rController,
            const sal_Int16 i_nPageId )
        :m_xController( i_rController )
        ,m_xWizardPage()
    {
        ENSURE_OR_THROW( m_xController.is(), "no controller" );
        try
        {
            // Plug a toplevel SalFrame into the native page which can host our awt widgetry
            m_xWizardPage.set(m_xController->createPage(pParent->CreateChildFrame(), i_nPageId), UNO_SET_THROW);

            Reference< XWindow > xPageWindow(m_xWizardPage->getWindow(), UNO_SET_THROW);
            xPageWindow->setVisible( true );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("svtools.uno");
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
            DBG_UNHANDLED_EXCEPTION("svtools.uno");
        }
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
            DBG_UNHANDLED_EXCEPTION("svtools.uno");
        }
    }

    bool WizardPageController::commitPage( vcl::WizardTypes::CommitPageReason i_eReason )
    {
        if ( !m_xWizardPage.is() )
            return true;

        try
        {
            return m_xWizardPage->commitPage( WizardShell::convertCommitReasonToTravelType( i_eReason ) );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("svtools.uno");
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
            DBG_UNHANDLED_EXCEPTION("svtools.uno");
        }

        return true;
    }


} // namespace svt::uno


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
