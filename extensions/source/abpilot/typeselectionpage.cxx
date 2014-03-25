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
#include <vcl/msgbox.hxx>
#include <com/sun/star/sdbc/XDriver.hpp>
#include <com/sun/star/sdbc/DriverManager.hpp>
#include <comphelper/processfactory.hxx>

namespace abp
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::sdbc;

    // TypeSelectionPage
    TypeSelectionPage::TypeSelectionPage( OAddessBookSourcePilot* _pParent )
        :AddressBookSourcePage(_pParent, ModuleRes(RID_PAGE_SELECTABTYPE))
        ,m_aHint                (this,  ModuleRes(FT_TYPE_HINTS))
        ,m_aTypeSep             (this,  ModuleRes(FL_TYPE))
        ,m_aEvolution           (this,  ModuleRes(RB_EVOLUTION))
        ,m_aEvolutionGroupwise  (this,  ModuleRes(RB_EVOLUTION_GROUPWISE))
        ,m_aEvolutionLdap       (this,  ModuleRes(RB_EVOLUTION_LDAP))
        ,m_aMORK                (this,  ModuleRes(RB_MORK))
        ,m_aThunderbird         (this,  ModuleRes(RB_THUNDERBIRD))
        ,m_aKab                 (this,  ModuleRes(RB_KAB))
        ,m_aMacab               (this,  ModuleRes(RB_MACAB))
        ,m_aLDAP                (this,  ModuleRes(RB_LDAP))
        ,m_aOutlook             (this,  ModuleRes(RB_OUTLOOK))
        ,m_aOE                  (this,  ModuleRes(RB_OUTLOOKEXPRESS))
        ,m_aOther               (this,  ModuleRes(RB_OTHER))
    {
        FreeResource();

        Point aTopLeft( LogicToPixel( Point( 15, 68 ), MAP_APPFONT ) );
        Size  aItemSize( LogicToPixel( Size( 0, 8 ), MAP_APPFONT ) );
        aItemSize.Width() = GetOutputSizePixel().Width() - 30;

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
        // - MORK, THUNDERBIRD, LDAP, OUTLOOK, OUTLOOKEXPRESS (via mozab driver,
        //   if WITH_MOZILLA)
        // - OTHER

        bool bWithMozilla = false;
        bool bHaveEvolution = false;
        bool bHaveKab = false;
        bool bHaveMacab = false;
        bool bWithMork = false;

#if defined WNT
#if defined WITH_MOZILLA
        bWithMozilla = true;
#endif
#else
        bWithMork = true;

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
        m_aAllTypes.push_back( ButtonItem( &m_aEvolution, AST_EVOLUTION, bHaveEvolution ) );
        m_aAllTypes.push_back( ButtonItem( &m_aEvolutionGroupwise, AST_EVOLUTION_GROUPWISE, bHaveEvolution ) );
        m_aAllTypes.push_back( ButtonItem( &m_aEvolutionLdap, AST_EVOLUTION_LDAP, bHaveEvolution ) );
        m_aAllTypes.push_back( ButtonItem( &m_aMORK, AST_MORK, bWithMozilla || bWithMork) );
        m_aAllTypes.push_back( ButtonItem( &m_aThunderbird, AST_THUNDERBIRD, bWithMozilla || bWithMork) );
        m_aAllTypes.push_back( ButtonItem( &m_aKab, AST_KAB, bHaveKab ) );
        m_aAllTypes.push_back( ButtonItem( &m_aMacab, AST_MACAB, bHaveMacab ) );
        m_aAllTypes.push_back( ButtonItem( &m_aLDAP, AST_LDAP, bWithMozilla ) );
        m_aAllTypes.push_back( ButtonItem( &m_aOutlook, AST_OUTLOOK, bWithMozilla ) );
        m_aAllTypes.push_back( ButtonItem( &m_aOE, AST_OE, bWithMozilla ) );
        m_aAllTypes.push_back( ButtonItem( &m_aOther, AST_OTHER, true ) );

        Link aTypeSelectionHandler = LINK(this, TypeSelectionPage, OnTypeSelected );
        const Size aSpacing( LogicToPixel( Size( 0, 3 ), MAP_APPFONT ) );
        if ( ! m_aAllTypes.empty() )
        {
            ButtonItem aItem = m_aAllTypes[0];
            aItem.m_pItem->SetStyle( aItem.m_pItem->GetStyle() | WB_GROUP );
        }
        for ( ::std::vector< ButtonItem >::const_iterator loop = m_aAllTypes.begin();
              loop != m_aAllTypes.end(); ++loop )
        {
            ButtonItem aItem = *loop;
            if (!aItem.m_bVisible)
                aItem.m_pItem->Hide();
            else
            {
                aItem.m_pItem->SetPosPixel( aTopLeft );
                aTopLeft.Y() += aItemSize.Height() + aSpacing.Height();
                aItem.m_pItem->SetClickHdl( aTypeSelectionHandler );
                aItem.m_pItem->Show();
            }
        }
    }


    TypeSelectionPage::~TypeSelectionPage()
    {
        for ( ::std::vector< ButtonItem >::iterator loop = m_aAllTypes.begin();
              loop != m_aAllTypes.end(); ++loop )
        {
            loop->m_bVisible = false;
        }
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

        getDialog()->enableButtons(WZB_PREVIOUS, false);
    }


    void TypeSelectionPage::DeactivatePage()
    {
        AddressBookSourcePage::DeactivatePage();
        getDialog()->enableButtons(WZB_PREVIOUS, true);
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
            ErrorBox aError(this, ModuleRes(RID_ERR_NEEDTYPESELECTION));
            aError.Execute();
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


    IMPL_LINK( TypeSelectionPage, OnTypeSelected, void*, /*NOTINTERESTEDIN*/ )
    {
        getDialog()->typeSelectionChanged( getSelectedType() );
        updateDialogTravelUI();
        return 0L;
    }


}   // namespace abp


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
