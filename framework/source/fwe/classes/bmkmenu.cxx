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

#include <limits.h>

#include "framework/bmkmenu.hxx"
#include <general.h>
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
    OUString&            rTitle,
    OUString&            rURL,
    OUString&            rFrame,
    OUString&            rImageId );

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
    SAL_INFO( "fwk", "framework (cd100003) ::BmkMenu::Initialize" );

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

    OUString aTitle;
    OUString aURL;
    OUString aTargetFrame;
    OUString aImageId;

    sal_uInt32 i, nCount = aDynamicMenuEntries.getLength();
    for ( i = 0; i < nCount; ++i )
    {
        GetMenuEntry( aDynamicMenuEntries[i], aTitle, aURL, aTargetFrame, aImageId );

        if ( aTitle.isEmpty() && aURL.isEmpty() )
            continue;

        if ( aURL == OUString( "private:separator" ))
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
    OUString& rTitle,
    OUString& rURL,
    OUString& rFrame,
    OUString& rImageId
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
