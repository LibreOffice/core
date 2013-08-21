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

#include <framework/actiontriggerhelper.hxx>
#include <classes/actiontriggerseparatorpropertyset.hxx>
#include <classes/rootactiontriggercontainer.hxx>
#include <classes/imagewrapper.hxx>
#include <framework/addonsoptions.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/XBitmap.hpp>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>
#include <tools/stream.hxx>
#include <cppuhelper/weak.hxx>
#include <comphelper/processfactory.hxx>
#include <vcl/dibtools.hxx>

const sal_uInt16 START_ITEMID = 1000;

using namespace com::sun::star::awt;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;


namespace framework
{

// ----------------------------------------------------------------------------
// implementation helper ( menu => ActionTrigger )
// ----------------------------------------------------------------------------

sal_Bool IsSeparator( Reference< XPropertySet > xPropertySet )
{
    Reference< XServiceInfo > xServiceInfo( xPropertySet, UNO_QUERY );
    try
    {
        return xServiceInfo->supportsService( OUString( SERVICENAME_ACTIONTRIGGERSEPARATOR ) );
    }
    catch (const Exception&)
    {
    }

    return sal_False;
}

void GetMenuItemAttributes( Reference< XPropertySet > xActionTriggerPropertySet,
                            OUString& aMenuLabel,
                            OUString& aCommandURL,
                            OUString& aHelpURL,
                            Reference< XBitmap >& xBitmap,
                            Reference< XIndexContainer >& xSubContainer )
{
    Any a;

    try
    {
        // mandatory properties
        a = xActionTriggerPropertySet->getPropertyValue("Text");
        a >>= aMenuLabel;
        a = xActionTriggerPropertySet->getPropertyValue("CommandURL");
        a >>= aCommandURL;
        a = xActionTriggerPropertySet->getPropertyValue("Image");
        a >>= xBitmap;
        a = xActionTriggerPropertySet->getPropertyValue("SubContainer");
        a >>= xSubContainer;
    }
    catch (const Exception&)
    {
    }

    // optional properties
    try
    {
        a = xActionTriggerPropertySet->getPropertyValue("HelpURL");
        a >>= aHelpURL;
    }
    catch (const Exception&)
    {
    }
}

void InsertSubMenuItems( Menu* pSubMenu, sal_uInt16& nItemId, Reference< XIndexContainer > xActionTriggerContainer )
{
    if ( xActionTriggerContainer.is() )
    {
        AddonsOptions aAddonOptions;
        OUString aSlotURL( "slot:" );

        for ( sal_Int32 i = 0; i < xActionTriggerContainer->getCount(); i++ )
        {
            try
            {
                Reference< XPropertySet > xPropSet;
                if (( xActionTriggerContainer->getByIndex( i ) >>= xPropSet ) && ( xPropSet.is() ))
                {
                    if ( IsSeparator( xPropSet ))
                    {
                        // Separator
                        SolarMutexGuard aGuard;
                        pSubMenu->InsertSeparator();
                    }
                    else
                    {
                        // Menu item
                        OUString aLabel;
                        OUString aCommandURL;
                        OUString aHelpURL;
                        Reference< XBitmap > xBitmap;
                        Reference< XIndexContainer > xSubContainer;

                        sal_uInt16 nNewItemId = nItemId++;
                        GetMenuItemAttributes( xPropSet, aLabel, aCommandURL, aHelpURL, xBitmap, xSubContainer );

                        SolarMutexGuard aGuard;
                        {
                            // insert new menu item
                            sal_Int32 nIndex = aCommandURL.indexOf( aSlotURL );
                            if ( nIndex >= 0 )
                            {
                                // Special code for our menu implementation: some menu items don't have a
                                // command url but uses the item id as a unqiue identifier. These entries
                                // got a special url during conversion from menu=>actiontriggercontainer.
                                // Now we have to extract this special url and set the correct item id!!!
                                nNewItemId = (sal_uInt16)aCommandURL.copy( nIndex+aSlotURL.getLength() ).toInt32();
                                pSubMenu->InsertItem( nNewItemId, aLabel );
                            }
                            else
                            {
                                pSubMenu->InsertItem( nNewItemId, aLabel );
                                pSubMenu->SetItemCommand( nNewItemId, aCommandURL );
                            }

                            // handle bitmap
                            if ( xBitmap.is() )
                            {
                                sal_Bool bImageSet = sal_False;

                                Reference< XUnoTunnel > xUnoTunnel( xBitmap, UNO_QUERY );
                                if ( xUnoTunnel.is() )
                                {
                                    // Try to get implementation pointer through XUnoTunnel
                                    sal_Int64 nPointer = xUnoTunnel->getSomething( ImageWrapper::GetUnoTunnelId() );
                                    if ( nPointer )
                                    {
                                        // This is our own optimized implementation of menu images!
                                        ImageWrapper* pImageWrapper = reinterpret_cast< ImageWrapper * >( nPointer );
                                        Image aMenuImage = pImageWrapper->GetImage();

                                        if ( !!aMenuImage )
                                            pSubMenu->SetItemImage( nNewItemId, aMenuImage );

                                        bImageSet = sal_True;
                                    }
                                }

                                if ( !bImageSet )
                                {
                                    // This is an unknown implementation of a XBitmap interface. We have to
                                    // use a more time consuming way to build an Image!
                                    Image   aImage;
                                    Bitmap  aBitmap;

                                    Sequence< sal_Int8 > aDIBSeq;
                                    {
                                        aDIBSeq = xBitmap->getDIB();
                                        SvMemoryStream aMem( (void *)aDIBSeq.getConstArray(), aDIBSeq.getLength(), STREAM_READ );
                                        ReadDIB(aBitmap, aMem, true);
                                    }

                                    aDIBSeq = xBitmap->getMaskDIB();
                                    if ( aDIBSeq.getLength() > 0 )
                                    {
                                        Bitmap aMaskBitmap;
                                        SvMemoryStream aMem( (void *)aDIBSeq.getConstArray(), aDIBSeq.getLength(), STREAM_READ );
                                        ReadDIB(aMaskBitmap, aMem, true);
                                        aImage = Image( aBitmap, aMaskBitmap );
                                    }
                                    else
                                        aImage = Image( aBitmap );

                                    if ( !!aImage )
                                        pSubMenu->SetItemImage( nNewItemId, aImage );
                                }
                            }
                            else
                            {
                                // Support add-on images for context menu interceptors
                                Image aImage = aAddonOptions.GetImageFromURL( aCommandURL, sal_False, sal_True );
                                if ( !!aImage )
                                    pSubMenu->SetItemImage( nNewItemId, aImage );
                            }

                            if ( xSubContainer.is() )
                            {
                                PopupMenu* pNewSubMenu = new PopupMenu;

                                // Sub menu (recursive call CreateSubMenu )
                                InsertSubMenuItems( pNewSubMenu, nItemId, xSubContainer );
                                pSubMenu->SetPopupMenu( nNewItemId, pNewSubMenu );
                            }
                        }
                    }
                }
            }
            catch (const IndexOutOfBoundsException&)
            {
                return;
            }
            catch (const WrappedTargetException&)
            {
                return;
            }
            catch (const RuntimeException&)
            {
                return;
            }
        }
    }
}


// ----------------------------------------------------------------------------
// implementation helper ( ActionTrigger => menu )
// ----------------------------------------------------------------------------

Reference< XPropertySet > CreateActionTrigger( sal_uInt16 nItemId, const Menu* pMenu, const Reference< XIndexContainer >& rActionTriggerContainer ) throw ( RuntimeException )
{
    Reference< XPropertySet > xPropSet;

    Reference< XMultiServiceFactory > xMultiServiceFactory( rActionTriggerContainer, UNO_QUERY );
    if ( xMultiServiceFactory.is() )
    {
        xPropSet = Reference< XPropertySet >(   xMultiServiceFactory->createInstance(
                                                    OUString( "com.sun.star.ui.ActionTrigger" ) ),
                                                UNO_QUERY );

        Any a;

        try
        {
            // Retrieve the menu attributes and set them in our PropertySet
            OUString aLabel = pMenu->GetItemText( nItemId );
            a <<= aLabel;
            xPropSet->setPropertyValue("Text", a );

            OUString aCommandURL = pMenu->GetItemCommand( nItemId );

            if ( aCommandURL.isEmpty() )
            {
                aCommandURL = OUString( "slot:" );
                aCommandURL += OUString::number( nItemId );
            }

            a <<= aCommandURL;
            xPropSet->setPropertyValue("CommandURL", a );

            Image aImage = pMenu->GetItemImage( nItemId );
            if ( !!aImage )
            {
                // We use our own optimized XBitmap implementation
                Reference< XBitmap > xBitmap( static_cast< cppu::OWeakObject* >( new ImageWrapper( aImage )), UNO_QUERY );
                a <<= xBitmap;
                xPropSet->setPropertyValue("Image", a );
            }
        }
        catch (const Exception&)
        {
        }
    }

    return xPropSet;
}

Reference< XPropertySet > CreateActionTriggerSeparator( const Reference< XIndexContainer >& rActionTriggerContainer ) throw ( RuntimeException )
{
    Reference< XMultiServiceFactory > xMultiServiceFactory( rActionTriggerContainer, UNO_QUERY );
    if ( xMultiServiceFactory.is() )
    {
        return Reference< XPropertySet >(   xMultiServiceFactory->createInstance(
                                                OUString( "com.sun.star.ui.ActionTriggerSeparator" ) ),
                                            UNO_QUERY );
    }

    return Reference< XPropertySet >();
}

Reference< XIndexContainer > CreateActionTriggerContainer( const Reference< XIndexContainer >& rActionTriggerContainer ) throw ( RuntimeException )
{
    Reference< XMultiServiceFactory > xMultiServiceFactory( rActionTriggerContainer, UNO_QUERY );
    if ( xMultiServiceFactory.is() )
    {
        return Reference< XIndexContainer >( xMultiServiceFactory->createInstance(
                                                OUString( "com.sun.star.ui.ActionTriggerContainer" ) ),
                                             UNO_QUERY );
    }

    return Reference< XIndexContainer >();
}

void FillActionTriggerContainerWithMenu( const Menu* pMenu, Reference< XIndexContainer >& rActionTriggerContainer )
{
    SolarMutexGuard aGuard;

    for ( sal_uInt16 nPos = 0; nPos < pMenu->GetItemCount(); nPos++ )
    {
        sal_uInt16          nItemId = pMenu->GetItemId( nPos );
        MenuItemType    nType   = pMenu->GetItemType( nPos );

        try
        {
            Any a;
            Reference< XPropertySet > xPropSet;

            if ( nType == MENUITEM_SEPARATOR )
            {
                xPropSet = CreateActionTriggerSeparator( rActionTriggerContainer );

                a <<= xPropSet;
                rActionTriggerContainer->insertByIndex( nPos, a );
            }
            else
            {
                xPropSet = CreateActionTrigger( nItemId, pMenu, rActionTriggerContainer );

                a <<= xPropSet;
                rActionTriggerContainer->insertByIndex( nPos, a );

                PopupMenu* pPopupMenu = pMenu->GetPopupMenu( nItemId );
                if ( pPopupMenu )
                {
                    // recursive call to build next sub menu
                    Reference< XIndexContainer > xSubContainer = CreateActionTriggerContainer( rActionTriggerContainer );

                    a <<= xSubContainer;
                    xPropSet->setPropertyValue("SubContainer", a );
                    FillActionTriggerContainerWithMenu( pPopupMenu, xSubContainer );
                }
            }
        }
        catch (const Exception&)
        {
        }
    }
}

void ActionTriggerHelper::CreateMenuFromActionTriggerContainer(
    Menu* pNewMenu,
    const Reference< XIndexContainer >& rActionTriggerContainer )
{
    sal_uInt16 nItemId = START_ITEMID;

    if ( rActionTriggerContainer.is() )
        InsertSubMenuItems( pNewMenu, nItemId, rActionTriggerContainer );
}

void ActionTriggerHelper::FillActionTriggerContainerFromMenu(
    Reference< XIndexContainer >& xActionTriggerContainer,
    const Menu* pMenu )
{
    FillActionTriggerContainerWithMenu( pMenu, xActionTriggerContainer );
}

Reference< XIndexContainer > ActionTriggerHelper::CreateActionTriggerContainerFromMenu(
    const Menu* pMenu,
    const OUString* pMenuIdentifier )
{
    return new RootActionTriggerContainer( pMenu, pMenuIdentifier );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
