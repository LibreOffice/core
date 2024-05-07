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
      : AddressBookSourcePage(pPage, pDialog, u"modules/sabpilot/ui/selecttypepage.ui"_ustr, u"SelectTypePage"_ustr)
      , m_xEvolution(m_xBuilder->weld_radio_button(u"evolution"_ustr))
      , m_xEvolutionGroupwise(m_xBuilder->weld_radio_button(u"groupwise"_ustr))
      , m_xEvolutionLdap(m_xBuilder->weld_radio_button(u"evoldap"_ustr))
      , m_xThunderbird(m_xBuilder->weld_radio_button(u"thunderbird"_ustr))
      , m_xKab(m_xBuilder->weld_radio_button(u"kde"_ustr))
      , m_xMacab(m_xBuilder->weld_radio_button(u"macosx"_ustr))
      , m_xOther(m_xBuilder->weld_radio_button(u"other"_ustr))
    {
        //TODO:  For now, try to keep offering the same choices like before the
        // Mozilla cleanup, even if the status of what driver actually
        // provides which functionality is somewhat unclear, see the discussions
        // of fdo#57285 "Address Book Data Source Wizard lists 'macOS address
        // book' on Linux" and fdo#57322 "Moz-free LDAP Address Book driver."
        // In accordance with ancient OOo 3.3, this is as follows:
        //
        // On Linux:
        // - EVOLUTION, EVOLUTION_GROUPWISE, EVOLUTION_LDAP (if applicable)
        // - KAB (if applicable)
        // - OTHER
        //
        // On macOS:
        // - MACAB (if applicable)
        // - OTHER
        //
        // On Windows:
        // - THUNDERBIRD
        // - OTHER

#if !defined(_WIN32)
        bool bHaveEvolution = false;
        bool bHaveKab = false;
        bool bHaveMacab = false;

        Reference< XDriverManager2 > xManager = DriverManager::create( pDialog->getORB() );

        try
        {
            // check whether Evolution is available
            Reference< XDriver > xDriver( xManager->getDriverByURL(u"sdbc:address:evolution:local"_ustr) );
            if ( xDriver.is() )
                bHaveEvolution = true;
        }
        catch (...)
        {
        }

        // check whether KDE address book is available
        try
        {
            Reference< XDriver > xDriver( xManager->getDriverByURL(u"sdbc:address:kab"_ustr) );
            if ( xDriver.is() )
                bHaveKab = true;
        }
        catch (...)
        {
        }

        try
        {
            // check whether macOS address book is available
            Reference< XDriver > xDriver( xManager->getDriverByURL(u"sdbc:address:macab"_ustr) );
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
        m_aAllTypes.emplace_back(m_xEvolution.get(), AST_EVOLUTION, bHaveEvolution);
        m_aAllTypes.emplace_back(m_xEvolutionGroupwise.get(), AST_EVOLUTION_GROUPWISE, bHaveEvolution);
        m_aAllTypes.emplace_back(m_xEvolutionLdap.get(), AST_EVOLUTION_LDAP, bHaveEvolution);
        m_aAllTypes.emplace_back(m_xThunderbird.get(), AST_THUNDERBIRD, true);
        m_aAllTypes.emplace_back(m_xKab.get(), AST_KAB, bHaveKab);
        m_aAllTypes.emplace_back(m_xMacab.get(), AST_MACAB, bHaveMacab);
        m_aAllTypes.emplace_back(m_xOther.get(), AST_OTHER, true);

        Link<weld::Toggleable&,void> aTypeSelectionHandler = LINK(this, TypeSelectionPage, OnTypeSelected );
        for (auto const& elem : m_aAllTypes)
        {
            if (!elem.m_bVisible)
                elem.m_pItem->hide();
            else
            {
                elem.m_pItem->connect_toggled( aTypeSelectionHandler );
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

    IMPL_LINK(TypeSelectionPage, OnTypeSelected, weld::Toggleable&, rButton, void)
    {
        if (!rButton.get_active())
            return;
        getDialog()->typeSelectionChanged( getSelectedType() );
        updateDialogTravelUI();
    }

}   // namespace abp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
