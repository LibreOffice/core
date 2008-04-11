/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: fontmenucontroller.cxx,v $
 * $Revision: 1.7 $
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
#include <uielement/fontmenucontroller.hxx>

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________
#include <threadhelp/resetableguard.hxx>
#include "services.h"

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/awt/MenuItemStyle.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>


//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#ifndef _VCL_MENU_HXX_
#include <vcl/menu.hxx>
#endif
#include <vcl/svapp.hxx>
#include <vcl/i18nhelp.hxx>
#include <tools/urlobj.hxx>
#include <rtl/ustrbuf.hxx>
#ifndef _VCL_MNEMONIC_HXX_
#include <vcl/mnemonic.hxx>
#endif

//_________________________________________________________________________________________________________________
//  Defines
//_________________________________________________________________________________________________________________
//

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::frame;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;

namespace framework
{

DEFINE_XSERVICEINFO_MULTISERVICE        (   FontMenuController                      ,
                                            OWeakObject                             ,
                                            SERVICENAME_POPUPMENUCONTROLLER         ,
                                            IMPLEMENTATIONNAME_FONTMENUCONTROLLER
                                        )

DEFINE_INIT_SERVICE                     (   FontMenuController, {} )

FontMenuController::FontMenuController( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager ) :
    PopupMenuControllerBase( xServiceManager )
{
}

FontMenuController::~FontMenuController()
{
}

// private function
void FontMenuController::fillPopupMenu( const Sequence< ::rtl::OUString >& rFontNameSeq, Reference< css::awt::XPopupMenu >& rPopupMenu )
{
    const rtl::OUString*    pFontNameArray = rFontNameSeq.getConstArray();
    VCLXPopupMenu*          pPopupMenu = (VCLXPopupMenu *)VCLXMenu::GetImplementation( rPopupMenu );
    PopupMenu*              pVCLPopupMenu = 0;

    vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );

    resetPopupMenu( rPopupMenu );
    if ( pPopupMenu )
        pVCLPopupMenu = (PopupMenu *)pPopupMenu->GetMenu();

    if ( pVCLPopupMenu )
    {
        rtl::OUString aEmpty;
        const vcl::I18nHelper& rI18nHelper = Application::GetSettings().GetUILocaleI18nHelper();
        const rtl::OUString aFontNameCommandPrefix( RTL_CONSTASCII_USTRINGPARAM( ".uno:CharFontName?CharFontName.FamilyName:string=" ));

        for ( USHORT i = 0; i < rFontNameSeq.getLength(); i++ )
        {
            const rtl::OUString& rName = pFontNameArray[i];

            USHORT j = m_xPopupMenu->getItemCount();
            while ( j )
            {
                rtl::OUString aText = m_xPopupMenu->getItemText( m_xPopupMenu->getItemId( j-1 ) );

                String aString = MnemonicGenerator::EraseAllMnemonicChars( aText );
                if ( rI18nHelper.CompareString( rName, aString ) > 0 )
                    break;
                j--;
            }

            m_xPopupMenu->insertItem( i+1, rName, css::awt::MenuItemStyle::RADIOCHECK | css::awt::MenuItemStyle::AUTOCHECK, j );
            if ( rName == m_aFontFamilyName )
                m_xPopupMenu->checkItem( i+1, sal_True );

            // use VCL popup menu pointer to set vital information that are not part of the awt implementation
            rtl::OUStringBuffer aCommandBuffer( aFontNameCommandPrefix );
            aCommandBuffer.append( INetURLObject::encode( rName, INetURLObject::PART_HTTP_QUERY, '%', INetURLObject::ENCODE_ALL ));

            rtl::OUString aFontNameCommand = aCommandBuffer.makeStringAndClear();
            pVCLPopupMenu->SetItemCommand( i+1, aFontNameCommand ); // Store font name into item command.
        }
    }
}

// XEventListener
void SAL_CALL FontMenuController::disposing( const EventObject& ) throw ( RuntimeException )
{
    Reference< css::awt::XMenuListener > xHolder(( OWeakObject *)this, UNO_QUERY );

    ResetableGuard aLock( m_aLock );
    m_xFrame.clear();
    m_xDispatch.clear();
    m_xFontListDispatch.clear();
    m_xServiceManager.clear();

    if ( m_xPopupMenu.is() )
        m_xPopupMenu->removeMenuListener( Reference< css::awt::XMenuListener >(( OWeakObject *)this, UNO_QUERY ));
    m_xPopupMenu.clear();
}

// XStatusListener
void SAL_CALL FontMenuController::statusChanged( const FeatureStateEvent& Event ) throw ( RuntimeException )
{
    com::sun::star::awt::FontDescriptor aFontDescriptor;
    Sequence< rtl::OUString >           aFontNameSeq;

    if ( Event.State >>= aFontDescriptor )
    {
        ResetableGuard aLock( m_aLock );
        m_aFontFamilyName = aFontDescriptor.Name;
    }
    else if ( Event.State >>= aFontNameSeq )
    {
        ResetableGuard aLock( m_aLock );
        if ( m_xPopupMenu.is() )
            fillPopupMenu( aFontNameSeq, m_xPopupMenu );
    }
}

// XMenuListener
void SAL_CALL FontMenuController::highlight( const css::awt::MenuEvent& ) throw (RuntimeException)
{
}

void SAL_CALL FontMenuController::select( const css::awt::MenuEvent& rEvent ) throw (RuntimeException)
{
    Reference< css::awt::XPopupMenu >   xPopupMenu;
    Reference< XDispatch >              xDispatch;
    Reference< XMultiServiceFactory >   xServiceManager;

    ResetableGuard aLock( m_aLock );
    xPopupMenu      = m_xPopupMenu;
    xDispatch       = m_xDispatch;
    xServiceManager = m_xServiceManager;
    aLock.unlock();

    if ( xPopupMenu.is() && xDispatch.is() )
    {
        VCLXPopupMenu* pPopupMenu = (VCLXPopupMenu *)VCLXPopupMenu::GetImplementation( xPopupMenu );
        if ( pPopupMenu )
        {
            css::util::URL               aTargetURL;
            Sequence<PropertyValue>      aArgs;
            Reference< XURLTransformer > xURLTransformer( xServiceManager->createInstance(
                                                            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))),
                                                        UNO_QUERY );

            {
                vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );

                // Command URL used to dispatch the selected font family name
                PopupMenu* pVCLPopupMenu = (PopupMenu *)pPopupMenu->GetMenu();
                aTargetURL.Complete = pVCLPopupMenu->GetItemCommand( rEvent.MenuId );
            }

            xURLTransformer->parseStrict( aTargetURL );
            xDispatch->dispatch( aTargetURL, aArgs );
        }
    }
}

void SAL_CALL FontMenuController::activate( const css::awt::MenuEvent& ) throw (RuntimeException)
{
    ResetableGuard aLock( m_aLock );

    if ( m_xPopupMenu.is() )
    {
        // find new font name and set check mark!
        USHORT        nChecked = 0;
        USHORT        nItemCount = m_xPopupMenu->getItemCount();
        rtl::OUString aEmpty;
        for( USHORT i = 0; i < nItemCount; i++ )
        {
            USHORT nItemId = m_xPopupMenu->getItemId( i );

            if ( m_xPopupMenu->isItemChecked( nItemId ) )
                nChecked = nItemId;

            rtl::OUString aText = m_xPopupMenu->getItemText( nItemId );

            // TODO: must be replaced by implementation of VCL, when available
            sal_Int32 nIndex = aText.indexOf( (sal_Unicode)'~' );
            if ( nIndex >= 0 )
                aText = aText.replaceAt( nIndex, 1, aEmpty );
            // TODO: must be replaced by implementation of VCL, when available

            if ( aText == m_aFontFamilyName )
            {
                m_xPopupMenu->checkItem( nItemId, sal_True );
                return;
            }
        }

        if ( nChecked )
            m_xPopupMenu->checkItem( nChecked, sal_False );
    }
}

void SAL_CALL FontMenuController::deactivate( const css::awt::MenuEvent& ) throw (RuntimeException)
{
}

// XPopupMenuController
void SAL_CALL FontMenuController::setPopupMenu( const Reference< css::awt::XPopupMenu >& xPopupMenu ) throw (RuntimeException)
{
    ResetableGuard aLock( m_aLock );

    if ( m_bDisposed )
        throw DisposedException();

    if ( m_xFrame.is() && !m_xPopupMenu.is() )
    {
        // Create popup menu on demand
        vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );

        m_xPopupMenu = xPopupMenu;
        m_xPopupMenu->addMenuListener( Reference< css::awt::XMenuListener >( (OWeakObject*)this, UNO_QUERY ));


        Reference< XURLTransformer > xURLTransformer( m_xServiceManager->createInstance(
                                                        rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))),
                                                    UNO_QUERY );
        Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );

        com::sun::star::util::URL aTargetURL;
        aTargetURL.Complete = m_aCommandURL;
        xURLTransformer->parseStrict( aTargetURL );
        m_xDispatch = xDispatchProvider->queryDispatch( aTargetURL, ::rtl::OUString(), 0 );

        // Register for font list updates to get the current font list from the controller
        aTargetURL.Complete = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FontNameList" ));
        xURLTransformer->parseStrict( aTargetURL );
        m_xFontListDispatch = xDispatchProvider->queryDispatch( aTargetURL, ::rtl::OUString(), 0 );

        updatePopupMenu();
    }
}

void SAL_CALL FontMenuController::updatePopupMenu() throw ( ::com::sun::star::uno::RuntimeException )
{
    PopupMenuControllerBase::updatePopupMenu();

    ResetableGuard aLock( m_aLock );
    Reference< XDispatch > xDispatch( m_xFontListDispatch );
    Reference< XURLTransformer > xURLTransformer( m_xServiceManager->createInstance(
                                                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))),
                                                UNO_QUERY );
    com::sun::star::util::URL aTargetURL;
    aTargetURL.Complete = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:FontNameList" ));
    xURLTransformer->parseStrict( aTargetURL );
    aLock.unlock();

    if ( xDispatch.is() )
    {
        xDispatch->addStatusListener( SAL_STATIC_CAST( XStatusListener*, this ), aTargetURL );
        xDispatch->removeStatusListener( SAL_STATIC_CAST( XStatusListener*, this ), aTargetURL );
    }
}

// XInitialization
void SAL_CALL FontMenuController::initialize( const Sequence< Any >& aArguments ) throw ( Exception, RuntimeException )
{
    PopupMenuControllerBase::initialize( aArguments );
}

}
