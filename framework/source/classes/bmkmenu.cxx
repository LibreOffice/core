/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: bmkmenu.cxx,v $
 * $Revision: 1.17 $
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
#include "precompiled_framework.hxx"

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#include <limits.h>

#include "classes/bmkmenu.hxx"
#include <general.h>
#include <macros/debug/assertion.hxx>
#include <helper/imageproducer.hxx>
#include <classes/menuconfiguration.hxx>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/util/DateTime.hpp>

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________
#include <tools/config.hxx>
#include <vcl/svapp.hxx>
#include <svtools/dynamicmenuoptions.hxx>
#include <svtools/menuoptions.hxx>
#include <rtl/logfile.hxx>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

using namespace ::rtl;
using namespace ::comphelper;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;

namespace framework
{

void GetMenuEntry(
    Sequence< PropertyValue >&  aDynamicMenuEntry,
    ::rtl::OUString&            rTitle,
    ::rtl::OUString&            rURL,
    ::rtl::OUString&            rFrame,
    ::rtl::OUString&            rImageId );

class BmkMenu_Impl
{
    private:
        static USHORT        m_nMID;

    public:
        BmkMenu*             m_pRoot;
        BOOL                 m_bInitialized;

        BmkMenu_Impl( BmkMenu* pRoot );
        BmkMenu_Impl();
        ~BmkMenu_Impl();

        static USHORT       GetMID();
};

USHORT BmkMenu_Impl::m_nMID = BMKMENU_ITEMID_START;

BmkMenu_Impl::BmkMenu_Impl( BmkMenu* pRoot ) :
    m_pRoot(pRoot),
    m_bInitialized(FALSE)
{
}

BmkMenu_Impl::BmkMenu_Impl() :
    m_pRoot(0),
    m_bInitialized(FALSE)
{
}

BmkMenu_Impl::~BmkMenu_Impl()
{
}

USHORT BmkMenu_Impl::GetMID()
{
    m_nMID++;
    if( !m_nMID )
        m_nMID = BMKMENU_ITEMID_START;
    return m_nMID;
}

// ------------------------------------------------------------------------

BmkMenu::BmkMenu( Reference< XFrame >& xFrame, BmkMenu::BmkMenuType nType, BmkMenu* pRoot ) :
    m_nType( nType ), m_xFrame( xFrame )
{
    _pImp = new BmkMenu_Impl( pRoot );
    Initialize();
}

BmkMenu::BmkMenu( Reference< XFrame >& xFrame, BmkMenu::BmkMenuType nType ) :
    m_nType( nType ), m_xFrame( xFrame )
{
    _pImp = new BmkMenu_Impl();
    Initialize();
}

BmkMenu::~BmkMenu()
{
    delete _pImp;

    for ( USHORT i = 0; i < GetItemCount(); i++ )
    {
        if ( GetItemType( i ) != MENUITEM_SEPARATOR )
        {
            // delete user attributes created with new!
            USHORT nId = GetItemId( i );
            MenuConfiguration::Attributes* pUserAttributes = (MenuConfiguration::Attributes*)GetUserValue( nId );
            delete pUserAttributes;
        }
    }
}

void BmkMenu::Initialize()
{
    RTL_LOGFILE_CONTEXT( aLog, "framework (cd100003) ::BmkMenu::Initialize" );

    if( _pImp->m_bInitialized )
        return;

    _pImp->m_bInitialized = TRUE;

    Sequence< Sequence< PropertyValue > > aDynamicMenuEntries;

    if ( m_nType == BmkMenu::BMK_NEWMENU )
        aDynamicMenuEntries = SvtDynamicMenuOptions().GetMenu( E_NEWMENU );
    else if ( m_nType == BmkMenu::BMK_WIZARDMENU )
        aDynamicMenuEntries = SvtDynamicMenuOptions().GetMenu( E_WIZARDMENU );
    BOOL bShowMenuImages = SvtMenuOptions().IsMenuIconsEnabled();

    ::rtl::OUString aTitle;
    ::rtl::OUString aURL;
    ::rtl::OUString aTargetFrame;
    ::rtl::OUString aImageId;

    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
    BOOL bIsHiContrastMode = rSettings.GetMenuColor().IsDark();

    UINT32 i, nCount = aDynamicMenuEntries.getLength();
    for ( i = 0; i < nCount; ++i )
    {
        GetMenuEntry( aDynamicMenuEntries[i], aTitle, aURL, aTargetFrame, aImageId );

        if ( !aTitle.getLength() && !aURL.getLength() )
            continue;

        if ( aURL == ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "private:separator" )))
            InsertSeparator();
        else
        {
            sal_Bool    bImageSet = sal_False;
            USHORT      nId = CreateMenuId();

            if ( bShowMenuImages )
            {
                if ( aImageId.getLength() > 0 )
                {
                    Image aImage = GetImageFromURL( m_xFrame, aImageId, FALSE, bIsHiContrastMode );
                    if ( !!aImage )
                    {
                        bImageSet = sal_True;
                        InsertItem( nId, aTitle, aImage );
                    }
                }

                if ( !bImageSet )
                {
                    Image aImage = GetImageFromURL( m_xFrame, aURL, FALSE, bIsHiContrastMode );
                    if ( !aImage )
                        InsertItem( nId, aTitle );
                    else
                        InsertItem( nId, aTitle, aImage );
                }
            }
            else
                InsertItem( nId, aTitle );

            // Store values from configuration to the New and Wizard menu entries to enable
            // sfx2 based code to support high contrast mode correctly!
            MenuConfiguration::Attributes* pUserAttributes = new MenuConfiguration::Attributes( aTargetFrame, aImageId );
            SetUserValue( nId, (ULONG)pUserAttributes );

            SetItemCommand( nId, aURL );
        }
    }
}

USHORT BmkMenu::CreateMenuId()
{
    return BmkMenu_Impl::GetMID();
}

void GetMenuEntry
(
    Sequence< PropertyValue >& aDynamicMenuEntry,
    ::rtl::OUString& rTitle,
    ::rtl::OUString& rURL,
    ::rtl::OUString& rFrame,
    ::rtl::OUString& rImageId
)
{
    for ( int i = 0; i < aDynamicMenuEntry.getLength(); i++ )
    {
        if ( aDynamicMenuEntry[i].Name == DYNAMICMENU_PROPERTYNAME_URL )
            aDynamicMenuEntry[i].Value >>= rURL;
        else if ( aDynamicMenuEntry[i].Name == DYNAMICMENU_PROPERTYNAME_TITLE )
            aDynamicMenuEntry[i].Value >>= rTitle;
        else if ( aDynamicMenuEntry[i].Name == DYNAMICMENU_PROPERTYNAME_IMAGEIDENTIFIER )
            aDynamicMenuEntry[i].Value >>= rImageId;
        else if ( aDynamicMenuEntry[i].Name == DYNAMICMENU_PROPERTYNAME_TARGETNAME )
            aDynamicMenuEntry[i].Value >>= rFrame;
    }
}

}
