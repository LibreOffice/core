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

#include "typeselectionpage.hxx"
#include "addresssettings.hxx"
#include "abspilot.hxx"
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <com/sun/star/sdbc/XDriver.hpp>
#include <com/sun/star/sdbc/DriverManager.hpp>

namespace abp
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::sdbc;

    // TypeSelectionPage
    TypeSelectionPage::TypeSelectionPage(weld::Container* pPage, OAddressBookSourcePilot* pDialog)
      : AddressBookSourcePage(pPage, pDialog, "modules/sabpilot/ui/selecttypepage.ui", "SelectTypePage")
      , m_xEvolution(m_xBuilder->weld_radio_button("evolution"))
      , m_xEvolutionGroupwise(m_xBuilder->weld_radio_button("groupwise"))
      , m_xEvolutionLdap(m_xBuilder->weld_radio_button("evoldap"))
      , m_xMORK(m_xBuilder->weld_radio_button("firefox"))
      , m_xThunderbird(m_xBuilder->weld_radio_button("thunderbird"))
      , m_xKab(m_xBuilder->weld_radio_button("kde"))
      , m_xMacab(m_xBuilder->weld_radio_button("macosx"))
      , m_xOther(m_xBuilder->weld_radio_button("other"))
    {
        //TODO:  For now, try to keep offering the same choices like before the
        // Mozilla/MORK cleanup, even if the status of what driver actually
        // provides which functionality is somewhat unclear, see the discussions
        // of fdo#57285 "Address Book Data Source Wizard lists 'macOS address
        // book' on Linux" and fdo#57322 "Moz-free LDAP Address Book driver."
        // In accordance with ancient OOo 3.3, this is as follows:
        //
        // On Linux:
        // - EVOLUTION, EVOLUTION_GROUPWISE, EVOLUTION_LDAP (if applicable)
        // - MORK (via mork driver, which is built unconditionally)
        // - KAB (if applicable)
        // - OTHER
        //
        // On macOS:
        // - MACAB (if applicable)
        // - MORK (via mork driver, which is built unconditionally)
        // - OTHER
        //
        // On Windows:
        // - MORK, THUNDERBIRD
        // - OTHER

#if !defined(_WIN32)
        bool bHaveEvolution = false;
        bool bHaveKab = false;
        bool bHaveMacab = false;

        Reference< XDriverManager2 > xManager = DriverManager::create( pDialog->getORB() );

        try
        {
            // check whether Evolution is available
            Reference< XDriver > xDriver( xManager->getDriverByURL("sdbc:address:evolution:local") );
            if ( xDriver.is() )
                bHaveEvolution = true;
        }
        catch (...)
        {
        }

        // check whether KDE address book is available
        try
        {
            Reference< XDriver > xDriver( xManager->getDriverByURL("sdbc:address:kab") );
            if ( xDriver.is() )
                bHaveKab = true;
        }
        catch (...)
        {
        }

        try
        {
            // check whether macOS address book is available
            Reference< XDriver > xDriver( xManager->getDriverByURL("sdbc:address:macab") );
            if ( xDriver.is() )
                bHaveMacab = true;
        }
        catch(...)
        {
        }
#else
        bool const bHaveEvolution = false;
        bool const bHaveKab = false;
        bool const bHaveMacab = false;
#endif

        // Items are displayed in list order
        m_aAllTypes.push_back( ButtonItem( m_xEvolution.get(), AST_EVOLUTION, bHaveEvolution ) );
        m_aAllTypes.push_back( ButtonItem( m_xEvolutionGroupwise.get(), AST_EVOLUTION_GROUPWISE, bHaveEvolution ) );
        m_aAllTypes.push_back( ButtonItem( m_xEvolutionLdap.get(), AST_EVOLUTION_LDAP, bHaveEvolution ) );
        m_aAllTypes.push_back( ButtonItem( m_xMORK.get(), AST_MORK, true ) );
        m_aAllTypes.push_back( ButtonItem( m_xThunderbird.get(), AST_THUNDERBIRD, true ) );
        m_aAllTypes.push_back( ButtonItem( m_xKab.get(), AST_KAB, bHaveKab ) );
        m_aAllTypes.push_back( ButtonItem( m_xMacab.get(), AST_MACAB, bHaveMacab ) );
        m_aAllTypes.push_back( ButtonItem( m_xOther.get(), AST_OTHER, true ) );

        Link<weld::Button&,void> aTypeSelectionHandler = LINK(this, TypeSelectionPage, OnTypeSelected );
        for (auto const& elem : m_aAllTypes)
        {
            if (!elem.m_bVisible)
                elem.m_pItem->hide();
            else
            {
                elem.m_pItem->connect_clicked( aTypeSelectionHandler );
                elem.m_pItem->show();
            }
        }
    }

    TypeSelectionPage::~TypeSelectionPage()
    {
        for (auto & elem : m_aAllTypes)
        {
            elem.m_bVisible = false;
        }
    }

    void TypeSelectionPage::Activate()
    {
        AddressBookSourcePage::Activate();

        for (auto const& elem : m_aAllTypes)
        {
            if( elem.m_pItem->get_active() && elem.m_bVisible )
            {
                elem.m_pItem->grab_focus();
                break;
            }
        }

        getDialog()->enableButtons(WizardButtonFlags::PREVIOUS, false);
    }

    void TypeSelectionPage::Deactivate()
    {
        AddressBookSourcePage::Deactivate();
        getDialog()->enableButtons(WizardButtonFlags::PREVIOUS, true);
    }

    void TypeSelectionPage::selectType( AddressSourceType _eType )
    {
        for (auto const& elem : m_aAllTypes)
        {
            elem.m_pItem->set_active( _eType == elem.m_eType );
        }
    }

    AddressSourceType TypeSelectionPage::getSelectedType() const
    {
        for (auto const& elem : m_aAllTypes)
        {
            if ( elem.m_pItem->get_active() && elem.m_bVisible )
                return elem.m_eType;
        }

        return AST_INVALID;
    }

    void TypeSelectionPage::initializePage()
    {
        AddressBookSourcePage::initializePage();

        const AddressSettings& rSettings = getSettings();
        selectType(rSettings.eType);
    }

    bool TypeSelectionPage::commitPage( ::vcl::WizardTypes::CommitPageReason _eReason )
    {
        if (!AddressBookSourcePage::commitPage(_eReason))
            return false;

        if (AST_INVALID == getSelectedType( ))
        {
            std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_xContainer.get(),
                                                      VclMessageType::Warning, VclButtonsType::Ok,
                                                      compmodule::ModuleRes(RID_STR_NEEDTYPESELECTION)));
            xBox->run();
            return false;
        }

        AddressSettings& rSettings = getSettings();
        rSettings.eType = getSelectedType();

        return true;
    }


    bool TypeSelectionPage::canAdvance() const
    {
        return  AddressBookSourcePage::canAdvance()
            &&  (AST_INVALID != getSelectedType());
    }


    IMPL_LINK_NOARG( TypeSelectionPage, OnTypeSelected, weld::Button&, void )
    {
        getDialog()->typeSelectionChanged( getSelectedType() );
        updateDialogTravelUI();
    }


}   // namespace abp


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
