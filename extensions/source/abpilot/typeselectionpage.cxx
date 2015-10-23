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
#include <vcl/layout.hxx>
#include <com/sun/star/sdbc/XDriver.hpp>
#include <com/sun/star/sdbc/DriverManager.hpp>
#include <comphelper/processfactory.hxx>

namespace abp
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::sdbc;

    // TypeSelectionPage
    TypeSelectionPage::TypeSelectionPage( OAddessBookSourcePilot* _pParent )
      : AddressBookSourcePage(_pParent, "SelectTypePage",
          "modules/sabpilot/ui/selecttypepage.ui")
    {
        get(m_pEvolution, "evolution");
        get(m_pEvolutionGroupwise, "groupwise");
        get(m_pEvolutionLdap, "evoldap");
        get(m_pMORK, "firefox");
        get(m_pThunderbird, "thunderbird");
        get(m_pKab, "kde");
        get(m_pMacab, "macosx");
        get(m_pOther, "other");

        //TODO:  For now, try to keep offering the same choices like before the
        // Mozilla/MORK cleanup, even if the status of what driver actually
        // provides which functionality is somewhat unclear, see the discussions
        // of fdo#57285 "Address Book Data Source Wizard lists 'Mac OS X address
        // book' on Linux" and fdo#57322 "Moz-free LDAP Address Book driver."
        // In accordance with ancient OOo 3.3, this is as follows:
        //
        // On Linux:
        // - EVOLUTION, EVOLUTION_GROUPWISE, EVOLUTION_LDAP (if applicable)
        // - MORK (via mork driver, which is built unconditionally)
        // - KAB (if applicable)
        // - OTHER
        //
        // On Mac OS X:
        // - MACAB (if applicable)
        // - MORK (via mork driver, which is built unconditionally)
        // - OTHER
        //
        // On Windows:
        // - MORK, THUNDERBIRD
        // - OTHER

        bool bHaveEvolution = false;
        bool bHaveKab = false;
        bool bHaveMacab = false;

#if !defined WNT

        Reference< XDriverManager2 > xManager = DriverManager::create( _pParent->getORB() );

        try
        {
            // check whether Evolution is available
            Reference< XDriver > xDriver( xManager->getDriverByURL(OUString("sdbc:address:evolution:local")) );
            if ( xDriver.is() )
                bHaveEvolution = true;
        }
        catch (...)
        {
        }

        // check whether KDE address book is available
        try
        {
            Reference< XDriver > xDriver( xManager->getDriverByURL(OUString("sdbc:address:kab")) );
            if ( xDriver.is() )
                bHaveKab = true;
        }
        catch (...)
        {
        }

        try
        {
            // check whether Mac OS X address book is available
            Reference< XDriver > xDriver( xManager->getDriverByURL(OUString("sdbc:address:macab")) );
            if ( xDriver.is() )
                bHaveMacab = true;
        }
        catch(...)
        {
        }

#endif

        // Items are displayed in list order
        m_aAllTypes.push_back( ButtonItem( m_pEvolution, AST_EVOLUTION, bHaveEvolution ) );
        m_aAllTypes.push_back( ButtonItem( m_pEvolutionGroupwise, AST_EVOLUTION_GROUPWISE, bHaveEvolution ) );
        m_aAllTypes.push_back( ButtonItem( m_pEvolutionLdap, AST_EVOLUTION_LDAP, bHaveEvolution ) );
        m_aAllTypes.push_back( ButtonItem( m_pMORK, AST_MORK, true ) );
        m_aAllTypes.push_back( ButtonItem( m_pThunderbird, AST_THUNDERBIRD, true ) );
        m_aAllTypes.push_back( ButtonItem( m_pKab, AST_KAB, bHaveKab ) );
        m_aAllTypes.push_back( ButtonItem( m_pMacab, AST_MACAB, bHaveMacab ) );
        m_aAllTypes.push_back( ButtonItem( m_pOther, AST_OTHER, true ) );

        Link<Button*,void> aTypeSelectionHandler = LINK(this, TypeSelectionPage, OnTypeSelected );
        for ( ::std::vector< ButtonItem >::const_iterator loop = m_aAllTypes.begin();
              loop != m_aAllTypes.end(); ++loop )
        {
            ButtonItem aItem = *loop;
            if (!aItem.m_bVisible)
                aItem.m_pItem->Hide();
            else
            {
                aItem.m_pItem->SetClickHdl( aTypeSelectionHandler );
                aItem.m_pItem->Show();
            }
        }
    }


    TypeSelectionPage::~TypeSelectionPage()
    {
        disposeOnce();
    }

    void TypeSelectionPage::dispose()
    {
        for ( ::std::vector< ButtonItem >::iterator loop = m_aAllTypes.begin();
              loop != m_aAllTypes.end(); ++loop )
        {
            loop->m_bVisible = false;
        }
        m_pEvolution.clear();
        m_pEvolutionGroupwise.clear();
        m_pEvolutionLdap.clear();
        m_pMORK.clear();
        m_pThunderbird.clear();
        m_pKab.clear();
        m_pMacab.clear();
        m_pOther.clear();
        AddressBookSourcePage::dispose();
    }


    void TypeSelectionPage::ActivatePage()
    {
        AddressBookSourcePage::ActivatePage();

        for ( ::std::vector< ButtonItem >::const_iterator loop = m_aAllTypes.begin();
              loop != m_aAllTypes.end(); ++loop )
        {
            const ButtonItem& rItem = (*loop);
            if( rItem.m_pItem->IsChecked() && rItem.m_bVisible )
            {
                rItem.m_pItem->GrabFocus();
                break;
            }
        }

        getDialog()->enableButtons(WizardButtonFlags::PREVIOUS, false);
    }


    void TypeSelectionPage::DeactivatePage()
    {
        AddressBookSourcePage::DeactivatePage();
        getDialog()->enableButtons(WizardButtonFlags::PREVIOUS, true);
    }


    void TypeSelectionPage::selectType( AddressSourceType _eType )
    {
        for ( ::std::vector< ButtonItem >::const_iterator loop = m_aAllTypes.begin();
              loop != m_aAllTypes.end(); ++loop )
        {
            ButtonItem aItem = (*loop);
            aItem.m_pItem->Check( _eType == aItem.m_eType );
        }
    }


    AddressSourceType TypeSelectionPage::getSelectedType() const
    {
        for ( ::std::vector< ButtonItem >::const_iterator loop = m_aAllTypes.begin();
              loop != m_aAllTypes.end(); ++loop )
        {
            ButtonItem aItem = (*loop);
            if ( aItem.m_pItem->IsChecked() )
                return aItem.m_eType;
        }

        return AST_INVALID;
    }


    void TypeSelectionPage::initializePage()
    {
        AddressBookSourcePage::initializePage();

        const AddressSettings& rSettings = getSettings();
        selectType(rSettings.eType);
    }


    bool TypeSelectionPage::commitPage( ::svt::WizardTypes::CommitPageReason _eReason )
    {
        if (!AddressBookSourcePage::commitPage(_eReason))
            return false;

        if (AST_INVALID == getSelectedType( ))
        {
            ScopedVclPtrInstance< MessageDialog > aError(this, ModuleRes(RID_STR_NEEDTYPESELECTION));
            aError->Execute();
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


    IMPL_LINK_NOARG_TYPED( TypeSelectionPage, OnTypeSelected, Button*, void )
    {
        getDialog()->typeSelectionChanged( getSelectedType() );
        updateDialogTravelUI();
    }


}   // namespace abp


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
