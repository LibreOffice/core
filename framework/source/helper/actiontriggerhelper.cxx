/*************************************************************************
 *
 *  $RCSfile: actiontriggerhelper.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mba $ $Date: 2002-06-27 07:28:56 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef __FRAMEWORK_HELPER_ACTIONTRIGGERHELPER_HXX_
#include <helper/actiontriggerhelper.hxx>
#endif

#ifndef __FRAMEWORK_CLASSES_ACTIONTRIGGERSEPARATORPROPERTYSET_HXX_
#include <classes/actiontriggerseparatorpropertyset.hxx>
#endif

#ifndef __FRAMEWORK_CLASSES_ROOTACTIONTRIGGERCONTAINER_HXX_
#include <classes/rootactiontriggercontainer.hxx>
#endif

#ifndef __FRAMEWORK_CLASSES_IMAGEWRAPPER_HXX_
#include <classes/imagewrapper.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XBITMAP_HPP_
#include <com/sun/star/awt/XBitmap.hpp>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif


const USHORT START_ITEMID = 1000;

using namespace rtl;
using namespace vos;
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
        return xServiceInfo->supportsService( OUString( RTL_CONSTASCII_USTRINGPARAM( SERVICENAME_ACTIONTRIGGERSEPARATOR )) );
    }
    catch ( Exception& )
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
        a = xActionTriggerPropertySet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "Text" )) );
        a >>= aMenuLabel;
        a = xActionTriggerPropertySet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "CommandURL" )) );
        a >>= aCommandURL;
        a = xActionTriggerPropertySet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "Image" )) );
        a >>= xBitmap;
        a = xActionTriggerPropertySet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "SubContainer" )) );
        a >>= xSubContainer;
    }
    catch ( Exception& )
    {
    }

    // optional properties
    try
    {
        a = xActionTriggerPropertySet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "HelpURL" )) );
        a >>= aHelpURL;
    }
    catch ( Exception& )
    {
    }
}

void InsertSubMenuItems( Menu* pSubMenu, USHORT& nItemId, Reference< XIndexContainer > xActionTriggerContainer )
{
    Reference< XIndexAccess > xIndexAccess( xActionTriggerContainer, UNO_QUERY );
    if ( xIndexAccess.is() )
    {
        OUString aSlotURL( RTL_CONSTASCII_USTRINGPARAM( "slot:" ));

        for ( sal_Int32 i = 0; i < xIndexAccess->getCount(); i++ )
        {
            try
            {
                Reference< XPropertySet > xPropSet;
                Any a = xIndexAccess->getByIndex( i );

                if (( a >>= xPropSet ) && ( xPropSet.is() ))
                {
                    if ( IsSeparator( xPropSet ))
                    {
                        // Separator
                        OGuard aGuard( Application::GetSolarMutex() );
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
                        sal_Bool bSpecialItemId = sal_False;

                        USHORT nNewItemId = nItemId++;
                        GetMenuItemAttributes( xPropSet, aLabel, aCommandURL, aHelpURL, xBitmap, xSubContainer );

                        OGuard aGuard( Application::GetSolarMutex() );
                        {
                            // insert new menu item
                            sal_Int32 nIndex = aCommandURL.indexOf( aSlotURL );
                            if ( nIndex >= 0 )
                            {
                                // Special code for our menu implementation: some menu items don't have a
                                // command url but uses the item id as a unqiue identifier. These entries
                                // got a special url during conversion from menu=>actiontriggercontainer.
                                // Now we have to extract this special url and set the correct item id!!!
                                bSpecialItemId = sal_True;
                                nNewItemId = (USHORT)aCommandURL.copy( nIndex+aSlotURL.getLength() ).toInt32();
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
                                        ImageWrapper* pImageWrapper = (ImageWrapper *)nPointer;
                                        Image aMenuImage = pImageWrapper->GetImage();

                                        if ( !!aMenuImage )
                                            pSubMenu->SetItemImage( nNewItemId, aMenuImage );

                                        bImageSet = sal_True;
                                    }
                                }

                                if ( !bImageSet )
                                {
                                    // This is a unknown implementation of XBitmap interface. We have to
                                    // use a more time consuming way to build an Image!
                                    // TODO: use memory streams to build a bitmap and this can be used
                                    // to create an image!!
                                    Image   aImage;
                                    Bitmap  aBitmap;

                                    Sequence< sal_Int8 > aDIBSeq;
                                    {
                                        aDIBSeq = xBitmap->getDIB();
                                        SvMemoryStream aMem( (void *)aDIBSeq.getConstArray(), aDIBSeq.getLength(), STREAM_READ );
                                        aMem >> aBitmap;
                                    }

                                    aDIBSeq = xBitmap->getMaskDIB();
                                    if ( aDIBSeq.getLength() > 0 )
                                    {
                                        Bitmap aMaskBitmap;
                                        SvMemoryStream aMem( (void *)aDIBSeq.getConstArray(), aDIBSeq.getLength(), STREAM_READ );
                                        aMem >> aMaskBitmap;
                                        aImage = Image( aBitmap, aMaskBitmap );
                                    }
                                    else
                                        aImage = Image( aBitmap );
                                }
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
            catch ( IndexOutOfBoundsException )
            {
                return;
            }
            catch ( WrappedTargetException )
            {
                return;
            }
            catch ( RuntimeException )
            {
                return;
            }
        }
    }
}


// ----------------------------------------------------------------------------
// implementation helper ( ActionTrigger => menu )
// ----------------------------------------------------------------------------

Reference< XPropertySet > CreateActionTrigger( USHORT nItemId, const Menu* pMenu, const Reference< XIndexContainer >& rActionTriggerContainer ) throw ( RuntimeException )
{
    Reference< XPropertySet > xPropSet;

    Reference< XMultiServiceFactory > xMultiServiceFactory( rActionTriggerContainer, UNO_QUERY );
    if ( xMultiServiceFactory.is() )
    {
        xPropSet = Reference< XPropertySet >(   xMultiServiceFactory->createInstance(
                                                    OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ui.ActionTrigger" )) ),
                                                UNO_QUERY );

        Any a;

        try
        {
            // Retrieve the menu attributes and set them in our PropertySet
            OUString aLabel = pMenu->GetItemText( nItemId );
            a <<= aLabel;
            xPropSet->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "Text" )), a );

            OUString aCommandURL = pMenu->GetItemCommand( nItemId );

            if ( aCommandURL.getLength() == 0 )
            {
                aCommandURL = OUString( RTL_CONSTASCII_USTRINGPARAM( "slot:" ));
                aCommandURL += OUString::valueOf( (sal_Int32)nItemId );
            }

            a <<= aCommandURL;
            xPropSet->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "CommandURL" )), a );

            Image aImage = pMenu->GetItemImage( nItemId );
            if ( !!aImage )
            {
                // We use our own optimized XBitmap implementation
                Reference< XBitmap > xBitmap( static_cast< cppu::OWeakObject* >( new ImageWrapper( aImage )), UNO_QUERY );
                a <<= xBitmap;
                xPropSet->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "Image" )), a );
            }
        }
        catch ( Exception& )
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
                                                OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ui.ActionTriggerSeparator" )) ),
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
                                                OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ui.ActionTriggerContainer" )) ),
                                             UNO_QUERY );
    }

    return Reference< XIndexContainer >();
}

void FillActionTriggerContainerWithMenu( const Menu* pMenu, Reference< XIndexContainer >& rActionTriggerContainer )
{
    OGuard aGuard( Application::GetSolarMutex() );

    for ( USHORT nPos = 0; nPos < pMenu->GetItemCount(); nPos++ )
    {
        USHORT          nItemId = pMenu->GetItemId( nPos );
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
                    Any a;
                    Reference< XIndexContainer > xSubContainer = CreateActionTriggerContainer( rActionTriggerContainer );

                    a <<= xSubContainer;
                    xPropSet->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "SubContainer" )), a );
                    FillActionTriggerContainerWithMenu( pPopupMenu, xSubContainer );
                }
            }
        }
        catch ( Exception& )
        {
        }
    }
}

void ActionTriggerHelper::CreateMenuFromActionTriggerContainer(
    Menu* pNewMenu,
    const Reference< XIndexContainer >& rActionTriggerContainer )
{
    USHORT nItemId = START_ITEMID;

    if ( rActionTriggerContainer.is() )
        InsertSubMenuItems( pNewMenu, nItemId, rActionTriggerContainer );
}

void ActionTriggerHelper::FillActionTriggerContainerFromMenu(
    Reference< XIndexContainer >& xActionTriggerContainer,
    const Menu* pMenu )
{
    FillActionTriggerContainerWithMenu( pMenu, xActionTriggerContainer );
}

Reference< XIndexContainer > ActionTriggerHelper::CreateActionTriggerContainerFromMenu( const Menu* pMenu )
{
    return new RootActionTriggerContainer( pMenu, ::comphelper::getProcessServiceFactory() );
}

}
