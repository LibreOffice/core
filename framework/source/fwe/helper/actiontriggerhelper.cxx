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
#include <framework/addonsoptions.hxx>
#include <com/sun/star/awt/XBitmap.hpp>
#include <com/sun/star/awt/XPopupMenu.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <toolkit/awt/vclxmenu.hxx>
#include <tools/stream.hxx>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/weak.hxx>
#include <vcl/image.hxx>
#include <vcl/svapp.hxx>
#include <vcl/graph.hxx>
#include <vcl/dibtools.hxx>

const sal_uInt16 START_ITEMID = 1000;

using namespace com::sun::star::awt;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::container;

namespace framework
{

// implementation helper ( menu => ActionTrigger )

static bool IsSeparator( const Reference< XPropertySet >& xPropertySet )
{
    Reference< XServiceInfo > xServiceInfo( xPropertySet, UNO_QUERY );
    try
    {
        return xServiceInfo->supportsService( SERVICENAME_ACTIONTRIGGERSEPARATOR );
    }
    catch (const Exception&)
    {
    }

    return false;
}

static void GetMenuItemAttributes( const Reference< XPropertySet >& xActionTriggerPropertySet,
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

static void InsertSubMenuItems(const Reference<XPopupMenu>& rSubMenu, sal_uInt16& nItemId,
                               const Reference<XIndexContainer>& xActionTriggerContainer)
{
    if ( !xActionTriggerContainer.is() )
        return;

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
                    rSubMenu->insertSeparator(i);
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
                            // command url but uses the item id as a unique identifier. These entries
                            // got a special url during conversion from menu=>actiontriggercontainer.
                            // Now we have to extract this special url and set the correct item id!!!
                            nNewItemId = static_cast<sal_uInt16>(aCommandURL.copy( nIndex+aSlotURL.getLength() ).toInt32());
                            rSubMenu->insertItem(nNewItemId, aLabel, 0, i);
                        }
                        else
                        {
                            rSubMenu->insertItem(nNewItemId, aLabel, 0, i);
                            rSubMenu->setCommand(nNewItemId, aCommandURL);
                        }

                        // handle bitmap
                        if ( xBitmap.is() )
                        {
                            bool bImageSet = false;

                            Reference<css::graphic::XGraphic> xGraphic(xBitmap, UNO_QUERY);
                            if (xGraphic.is())
                            {
                                // we can take the optimized route if XGraphic is supported
                                rSubMenu->setItemImage(nNewItemId, xGraphic, false);
                                bImageSet = true;
                            }

                            if ( !bImageSet )
                            {
                                // This is an unknown implementation of a XBitmap interface. We have to
                                // use a more time consuming way to build an Image!
                                BitmapEx aBitmap;

                                Sequence< sal_Int8 > aDIBSeq;
                                {
                                    aDIBSeq = xBitmap->getDIB();
                                    SvMemoryStream aMem( const_cast<sal_Int8 *>(aDIBSeq.getConstArray()), aDIBSeq.getLength(), StreamMode::READ );
                                    ReadDIBBitmapEx(aBitmap, aMem);
                                }

                                aDIBSeq = xBitmap->getMaskDIB();
                                if ( aDIBSeq.hasElements() )
                                {
                                    Bitmap aMaskBitmap;
                                    SvMemoryStream aMem( const_cast<sal_Int8 *>(aDIBSeq.getConstArray()), aDIBSeq.getLength(), StreamMode::READ );
                                    ReadDIB(aMaskBitmap, aMem, true);
                                    aBitmap = BitmapEx(aBitmap.GetBitmap(), aMaskBitmap);
                                }

                                if (!aBitmap.IsEmpty())
                                    rSubMenu->setItemImage(nNewItemId, Graphic(aBitmap).GetXGraphic(), false);
                            }
                        }
                        else
                        {
                            // Support add-on images for context menu interceptors
                            BitmapEx aBitmap(aAddonOptions.GetImageFromURL(aCommandURL, false, true));
                            if (!aBitmap.IsEmpty())
                                rSubMenu->setItemImage(nNewItemId, Graphic(aBitmap).GetXGraphic(), false);
                        }

                        if ( xSubContainer.is() )
                        {
                            rtl::Reference xNewSubMenu(new VCLXPopupMenu);

                            // Sub menu (recursive call CreateSubMenu )
                            InsertSubMenuItems(xNewSubMenu, nItemId, xSubContainer);
                            rSubMenu->setPopupMenu(nNewItemId, xNewSubMenu);
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

// implementation helper ( ActionTrigger => menu )

/// @throws RuntimeException
static Reference< XPropertySet > CreateActionTrigger( sal_uInt16 nItemId, const Menu* pMenu, const Reference< XIndexContainer >& rActionTriggerContainer )
{
    Reference< XPropertySet > xPropSet;

    Reference< XMultiServiceFactory > xMultiServiceFactory( rActionTriggerContainer, UNO_QUERY );
    if ( xMultiServiceFactory.is() )
    {
        xPropSet.set( xMultiServiceFactory->createInstance( "com.sun.star.ui.ActionTrigger" ),
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
                aCommandURL = "slot:" + OUString::number( nItemId );
            }

            a <<= aCommandURL;
            xPropSet->setPropertyValue("CommandURL", a );

            Image aImage = pMenu->GetItemImage( nItemId );
            if ( !!aImage )
            {
                Reference<css::graphic::XGraphic> xGraphic = Graphic(aImage.GetBitmapEx()).GetXGraphic();
                Reference<XBitmap> xBitmap(xGraphic, UNO_QUERY);
                assert(xGraphic.is() == xBitmap.is());
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

/// @throws RuntimeException
static Reference< XPropertySet > CreateActionTriggerSeparator( const Reference< XIndexContainer >& rActionTriggerContainer )
{
    Reference< XMultiServiceFactory > xMultiServiceFactory( rActionTriggerContainer, UNO_QUERY );
    if ( xMultiServiceFactory.is() )
    {
        return Reference< XPropertySet >(   xMultiServiceFactory->createInstance(
                                                "com.sun.star.ui.ActionTriggerSeparator" ),
                                            UNO_QUERY );
    }

    return Reference< XPropertySet >();
}

/// @throws RuntimeException
static Reference< XIndexContainer > CreateActionTriggerContainer( const Reference< XIndexContainer >& rActionTriggerContainer )
{
    Reference< XMultiServiceFactory > xMultiServiceFactory( rActionTriggerContainer, UNO_QUERY );
    if ( xMultiServiceFactory.is() )
    {
        return Reference< XIndexContainer >( xMultiServiceFactory->createInstance(
                                                "com.sun.star.ui.ActionTriggerContainer" ),
                                             UNO_QUERY );
    }

    return Reference< XIndexContainer >();
}

static void FillActionTriggerContainerWithMenu( const Menu* pMenu, Reference< XIndexContainer > const & rActionTriggerContainer )
{
    SolarMutexGuard aGuard;

    for ( sal_uInt16 nPos = 0; nPos < pMenu->GetItemCount(); nPos++ )
    {
        sal_uInt16          nItemId = pMenu->GetItemId( nPos );
        ::MenuItemType nType   = pMenu->GetItemType( nPos );

        try
        {
            Any a;
            Reference< XPropertySet > xPropSet;

            if ( nType == ::MenuItemType::SEPARATOR )
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
    const Reference<XPopupMenu>& rNewMenu,
    const Reference<XIndexContainer>& rActionTriggerContainer)
{
    sal_uInt16 nItemId = START_ITEMID;

    if ( rActionTriggerContainer.is() )
        InsertSubMenuItems(rNewMenu, nItemId, rActionTriggerContainer);
}

void ActionTriggerHelper::FillActionTriggerContainerFromMenu(
    Reference< XIndexContainer > const & xActionTriggerContainer,
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
