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

#include <limits.h>

#include "framework/bmkmenu.hxx"
#include <general.h>
#include <macros/debug/assertion.hxx>
#include <framework/imageproducer.hxx>
#include <framework/menuconfiguration.hxx>

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/util/DateTime.hpp>

#include <vcl/svapp.hxx>
#include <unotools/dynamicmenuoptions.hxx>
#include <svtools/menuoptions.hxx>
#include <rtl/logfile.hxx>

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

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
        static sal_uInt16        m_nMID;

    public:
        sal_Bool                 m_bInitialized;

        BmkMenu_Impl();
        ~BmkMenu_Impl();

        static sal_uInt16       GetMID();
};

sal_uInt16 BmkMenu_Impl::m_nMID = BMKMENU_ITEMID_START;


BmkMenu_Impl::BmkMenu_Impl() :
    m_bInitialized(sal_False)
{
}

BmkMenu_Impl::~BmkMenu_Impl()
{
}

sal_uInt16 BmkMenu_Impl::GetMID()
{
    m_nMID++;
    if( !m_nMID )
        m_nMID = BMKMENU_ITEMID_START;
    return m_nMID;
}

// ------------------------------------------------------------------------

BmkMenu::BmkMenu( Reference< XFrame >& xFrame, BmkMenu::BmkMenuType nType )
    :AddonMenu(xFrame)
    ,m_nType( nType )
{
    _pImp = new BmkMenu_Impl();
    Initialize();
}

BmkMenu::~BmkMenu()
{
    delete _pImp;
}

void BmkMenu::Initialize()
{
    RTL_LOGFILE_CONTEXT( aLog, "framework (cd100003) ::BmkMenu::Initialize" );

    if( _pImp->m_bInitialized )
        return;

    _pImp->m_bInitialized = sal_True;

    Sequence< Sequence< PropertyValue > > aDynamicMenuEntries;

    if ( m_nType == BmkMenu::BMK_NEWMENU )
        aDynamicMenuEntries = SvtDynamicMenuOptions().GetMenu( E_NEWMENU );
    else if ( m_nType == BmkMenu::BMK_WIZARDMENU )
        aDynamicMenuEntries = SvtDynamicMenuOptions().GetMenu( E_WIZARDMENU );

    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
    sal_Bool bShowMenuImages = rSettings.GetUseImagesInMenus();

    ::rtl::OUString aTitle;
    ::rtl::OUString aURL;
    ::rtl::OUString aTargetFrame;
    ::rtl::OUString aImageId;

    sal_uInt32 i, nCount = aDynamicMenuEntries.getLength();
    for ( i = 0; i < nCount; ++i )
    {
        GetMenuEntry( aDynamicMenuEntries[i], aTitle, aURL, aTargetFrame, aImageId );

        if ( aTitle.isEmpty() && aURL.isEmpty() )
            continue;

        if ( aURL == ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "private:separator" )))
            InsertSeparator();
        else
        {
            sal_Bool    bImageSet = sal_False;
            sal_uInt16      nId = CreateMenuId();

            if ( bShowMenuImages )
            {
                if ( !aImageId.isEmpty() )
                {
                    Image aImage = GetImageFromURL( m_xFrame, aImageId, false );
                    if ( !!aImage )
                    {
                        bImageSet = sal_True;
                        InsertItem( nId, aTitle, aImage );
                    }
                }

                if ( !bImageSet )
                {
                    Image aImage = GetImageFromURL( m_xFrame, aURL, false );
                    if ( !aImage )
                        InsertItem( nId, aTitle );
                    else
                        InsertItem( nId, aTitle, aImage );
                }
            }
            else
                InsertItem( nId, aTitle );

            MenuConfiguration::Attributes* pUserAttributes = new MenuConfiguration::Attributes( aTargetFrame, aImageId );
            SetUserValue( nId, (sal_uIntPtr)pUserAttributes );

            SetItemCommand( nId, aURL );
        }
    }
}

sal_uInt16 BmkMenu::CreateMenuId()
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
