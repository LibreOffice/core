/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fontmenucontroller.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-13 15:07:08 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

#ifndef __FRAMEWORK_UIELEMENT_FONTMENUCONTROLLER_HXX_
#include <uielement/fontmenucontroller.hxx>
#endif

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_THREADHELP_RESETABLEGUARD_HXX_
#include <threadhelp/resetableguard.hxx>
#endif

#ifndef __FRAMEWORK_SERVICES_H_
#include "services.h"
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_AWT_XDEVICE_HPP_
#include <com/sun/star/awt/XDevice.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_MENUITEMSTYLE_HPP_
#include <com/sun/star/awt/MenuItemStyle.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif


//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#ifndef _VCL_MENU_HXX_
#include <vcl/menu.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _VCL_I18NHELP_HXX
#include <vcl/i18nhelp.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
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
