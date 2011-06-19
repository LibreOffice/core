/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"
#include "typeselectionpage.hxx"
#include "addresssettings.hxx"
#include "abspilot.hxx"
#include <vcl/msgbox.hxx>
#include <com/sun/star/sdbc/XDriverAccess.hpp>

//.........................................................................
namespace abp
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::sdbc;

    //=====================================================================
    //= TypeSelectionPage
    //=====================================================================
    //---------------------------------------------------------------------
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

        bool bWithMozilla = true, bUnx = true;
        bool bHaveEvolution = true, bHaveKab = true;
        bool bHaveMacab = true;

#if !defined WITH_MOZILLA || defined MACOSX
        bWithMozilla = false;
#endif
#ifndef UNX
        bUnx = false;
        bHaveEvolution = false;
        bHaveKab = false;
        bHaveMacab = false;
#else
        Reference< XDriverAccess> xManager(_pParent->getORB()->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdbc.DriverManager"))), UNO_QUERY);

        // check whether Evolution is available
        Reference< XDriver > xDriver( xManager->getDriverByURL(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdbc:address:evolution:local"))) );
        if ( !xDriver.is() )
            bHaveEvolution = false;

        // check whether KDE address book is available
        xDriver = xManager->getDriverByURL(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdbc:address:kab")));
        if ( !xDriver.is() )
            bHaveKab = false;

        // check whether Mac OS X address book is available
        xDriver = xManager->getDriverByURL(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdbc:address:macab")));
        if ( !xDriver.is() )
            bHaveMacab = false;
#endif

        // Items are displayed in list order
        m_aAllTypes.push_back( ButtonItem( &m_aEvolution, AST_EVOLUTION, bHaveEvolution ) );
        m_aAllTypes.push_back( ButtonItem( &m_aEvolutionGroupwise, AST_EVOLUTION_GROUPWISE, bHaveEvolution ) );
        m_aAllTypes.push_back( ButtonItem( &m_aEvolutionLdap, AST_EVOLUTION_LDAP, bHaveEvolution ) );
        m_aAllTypes.push_back( ButtonItem( &m_aMORK, AST_MORK, bWithMozilla ) );
        m_aAllTypes.push_back( ButtonItem( &m_aThunderbird, AST_THUNDERBIRD, bWithMozilla ) );
        m_aAllTypes.push_back( ButtonItem( &m_aKab, AST_KAB, bHaveKab ) );
        m_aAllTypes.push_back( ButtonItem( &m_aMacab, AST_MACAB, bHaveMacab ) );
        m_aAllTypes.push_back( ButtonItem( &m_aLDAP, AST_LDAP, bWithMozilla ) );
        m_aAllTypes.push_back( ButtonItem( &m_aOutlook, AST_OUTLOOK, bWithMozilla && !bUnx ) );
        m_aAllTypes.push_back( ButtonItem( &m_aOE, AST_OE, bWithMozilla && !bUnx ) );
        m_aAllTypes.push_back( ButtonItem( &m_aOther, AST_OTHER, true ) );

        bool bFirstVisible = true;
        Link aTypeSelectionHandler = LINK(this, TypeSelectionPage, OnTypeSelected );
        const Size aSpacing( LogicToPixel( Size( 0, 3 ), MAP_APPFONT ) );
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

                if ( bFirstVisible )
                {
                    aItem.m_pItem->SetStyle( aItem.m_pItem->GetStyle() | WB_GROUP );
                    bFirstVisible = false;
                }
            }
        }
    }

    //---------------------------------------------------------------------
    TypeSelectionPage::~TypeSelectionPage()
    {
        for ( ::std::vector< ButtonItem >::iterator loop = m_aAllTypes.begin();
              loop != m_aAllTypes.end(); ++loop )
        {
            loop->m_bVisible = false;
        }
    }

    //---------------------------------------------------------------------
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

        getDialog()->enableButtons(WZB_PREVIOUS, sal_False);
    }

    //---------------------------------------------------------------------
    void TypeSelectionPage::DeactivatePage()
    {
        AddressBookSourcePage::DeactivatePage();
        getDialog()->enableButtons(WZB_PREVIOUS, sal_True);
    }

    //---------------------------------------------------------------------
    void TypeSelectionPage::selectType( AddressSourceType _eType )
    {
        for ( ::std::vector< ButtonItem >::const_iterator loop = m_aAllTypes.begin();
              loop != m_aAllTypes.end(); ++loop )
        {
            ButtonItem aItem = (*loop);
            aItem.m_pItem->Check( _eType == aItem.m_eType );
        }
    }

    //---------------------------------------------------------------------
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

    //---------------------------------------------------------------------
    void TypeSelectionPage::initializePage()
    {
        AddressBookSourcePage::initializePage();

        const AddressSettings& rSettings = getSettings();
        selectType(rSettings.eType);
    }

    //---------------------------------------------------------------------
    sal_Bool TypeSelectionPage::commitPage( ::svt::WizardTypes::CommitPageReason _eReason )
    {
        if (!AddressBookSourcePage::commitPage(_eReason))
            return sal_False;

        if (AST_INVALID == getSelectedType( ))
        {
            ErrorBox aError(this, ModuleRes(RID_ERR_NEEDTYPESELECTION));
            aError.Execute();
            return sal_False;
        }

        AddressSettings& rSettings = getSettings();
        rSettings.eType = getSelectedType();

        return sal_True;
    }

    //---------------------------------------------------------------------
    bool TypeSelectionPage::canAdvance() const
    {
        return  AddressBookSourcePage::canAdvance()
            &&  (AST_INVALID != getSelectedType());
    }

    //---------------------------------------------------------------------
    IMPL_LINK( TypeSelectionPage, OnTypeSelected, void*, /*NOTINTERESTEDIN*/ )
    {
        getDialog()->typeSelectionChanged( getSelectedType() );
        updateDialogTravelUI();
        return 0L;
    }

//.........................................................................
}   // namespace abp
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
