/*************************************************************************
 *
 *  $RCSfile: fontsizemenucontroller.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-06 17:00:47 $
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

#ifndef __FRAMEWORK_UIELEMENT_FONTSIZEMENUCONTROLLER_HXX_
#include <uielement/fontsizemenucontroller.hxx>
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

#ifndef _COM_SUN_STAR_VIEW_XPRINTABLE_HPP_
#include <com/sun/star/view/XPrintable.hpp>
#endif

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#ifndef _VCL_MENU_HXX_
#include <vcl/menu.hxx>
#endif
#ifndef _VCL_MAPUNIT_HXX
#include <vcl/mapunit.hxx>
#endif
#ifndef _VCL_SVAPP_HXX_
#include <vcl/svapp.hxx>
#endif
#ifndef _VCL_I18NHELP_HXX
#include <vcl/i18nhelp.hxx>
#endif
#ifndef _VCL_OUTPUTDEVICE_HXX_
#include <vcl/outdev.hxx>
#endif
#ifndef _SV_PRINT_HXX
#include <vcl/print.hxx>
#endif
#ifndef _SVTOOLS_CTRLTOOL_HXX_
#include <svtools/ctrltool.hxx>
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
using namespace com::sun::star::view;
using namespace com::sun::star::beans;

namespace framework
{

DEFINE_XSERVICEINFO_MULTISERVICE        (   FontSizeMenuController                      ,
                                            OWeakObject                                 ,
                                            SERVICENAME_POPUPMENUCONTROLLER             ,
                                            IMPLEMENTATIONNAME_FONTSIZEMENUCONTROLLER
                                        )

DEFINE_INIT_SERVICE                     (   FontSizeMenuController, {} )

FontSizeMenuController::FontSizeMenuController( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager ) :
    PopupMenuControllerBase( xServiceManager ),
    m_pHeightArray( 0 ),
    m_bRebuildMenu( sal_True )
{
}

FontSizeMenuController::~FontSizeMenuController()
{
    delete []m_pHeightArray;
}

// private function
rtl::OUString FontSizeMenuController::retrievePrinterName( com::sun::star::uno::Reference< com::sun::star::frame::XFrame >& rFrame )
{
    rtl::OUString aPrinterName;

    if ( rFrame.is() )
    {
        Reference< XController > xController = m_xFrame->getController();
        if ( xController.is() )
        {
            Reference< XPrintable > xPrintable( xController->getModel(), UNO_QUERY );
            if ( xPrintable.is() )
            {
                Sequence< PropertyValue > aPrinterSeq = xPrintable->getPrinter();
                for ( int i = 0; i < aPrinterSeq.getLength(); i++ )
                {
                    if ( aPrinterSeq[i].Name.equalsAscii( "Name" ))
                    {
                        aPrinterSeq[i].Value >>= aPrinterName;
                        break;
                    }
                }
            }
        }
    }

    return aPrinterName;
}

// private function
void FontSizeMenuController::setCurHeight( long nHeight, Reference< css::awt::XPopupMenu >& rPopupMenu )
{
    // check menu item
    rtl::OUString   aHeight     = Application::GetSettings().GetUILocaleI18nHelper().GetNum( nHeight, 1, TRUE, FALSE  );
    USHORT          nChecked    = 0;
    USHORT          nItemCount  = rPopupMenu->getItemCount();
    for( USHORT i = 0; i < nItemCount; i++ )
    {
        USHORT nItemId = rPopupMenu->getItemId( i );

        if ( m_pHeightArray[i] == nHeight )
        {
            rPopupMenu->checkItem( nItemId, TRUE );
            return;
        }

        if ( rPopupMenu->isItemChecked( nItemId ) )
            nChecked = nItemId;
    }

    if ( nChecked )
        rPopupMenu->checkItem( nChecked, FALSE );
}

// private function
void FontSizeMenuController::fillPopupMenu( Reference< css::awt::XPopupMenu >& rPopupMenu )
{
    const rtl::OUString     aFontNameCommand( RTL_CONSTASCII_USTRINGPARAM( ".uno:FontHeight?FontHeight=" ));
    VCLXPopupMenu*          pPopupMenu = (VCLXPopupMenu *)VCLXMenu::GetImplementation( rPopupMenu );
    PopupMenu*              pVCLPopupMenu = 0;

    resetPopupMenu( rPopupMenu );
    if ( pPopupMenu )
        pVCLPopupMenu = (PopupMenu *)pPopupMenu->GetMenu();

    if ( pVCLPopupMenu )
    {
        FontList*       pFontList = 0;
        Printer*        pInfoPrinter = 0;
        rtl::OUString   aPrinterName;

        vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );

        // try to retrieve printer name of document
        aPrinterName = retrievePrinterName( m_xFrame );
        if ( aPrinterName.getLength() > 0 )
        {
            pInfoPrinter = new Printer( aPrinterName );
            if ( pInfoPrinter && pInfoPrinter->GetDevFontCount() > 0 )
                pFontList = new FontList( pInfoPrinter );
        }

        if ( pFontList == 0 )
            pFontList   = new FontList( Application::GetDefaultDevice() );

        FontInfo aFntInfo = pFontList->Get( m_aFontDescriptor.Name, m_aFontDescriptor.StyleName );

        // setup font size array
        if ( m_pHeightArray )
            delete m_pHeightArray;

        const long* pTempAry;
        const long* pAry = pFontList->GetSizeAry( aFntInfo );
        USHORT nSizeCount = 0;
        while ( pAry[nSizeCount] )
            nSizeCount++;

        USHORT nPos = 0;

        // first insert font size names (for simplified/traditional chinese)
        float           fPoint;
        rtl::OUString   aHeightString;
        FontSizeNames   aFontSizeNames( Application::GetSettings().GetUILanguage() );
        m_pHeightArray = new long[nSizeCount+aFontSizeNames.Count()];
        if ( !aFontSizeNames.IsEmpty() )
        {
            if ( pAry == pFontList->GetStdSizeAry() )
            {
                // for scalable fonts all font size names
                ULONG nCount = aFontSizeNames.Count();
                for( ULONG i = 0; i < nCount; i++ )
                {
                    String  aSizeName = aFontSizeNames.GetIndexName( i );
                    long    nSize = aFontSizeNames.GetIndexSize( i );
                    m_pHeightArray[nPos] = nSize;
                    nPos++; // Id is nPos+1
                    pVCLPopupMenu->InsertItem( nPos, aSizeName, MIB_RADIOCHECK | MIB_AUTOCHECK );
                    fPoint = float( m_pHeightArray[nPos-1] ) / 10;
                    pVCLPopupMenu->SetItemCommand( nPos, rtl::OUString::valueOf( fPoint )); // Store font height into command
                }
            }
            else
            {
                // for fixed size fonts only selectable font size names
                pTempAry = pAry;
                while ( *pTempAry )
                {
                    String aSizeName = aFontSizeNames.Size2Name( *pTempAry );
                    if ( aSizeName.Len() )
                    {
                        m_pHeightArray[nPos] = *pTempAry;
                        nPos++; // Id is nPos+1
                        pVCLPopupMenu->InsertItem( nPos, aSizeName, MIB_RADIOCHECK | MIB_AUTOCHECK );
                        fPoint = float( m_pHeightArray[nPos-1] ) / 10;
                        pVCLPopupMenu->SetItemCommand( nPos, rtl::OUString::valueOf( fPoint )); // Store font height into command
                    }
                    pTempAry++;
                }
            }
        }

        // then insert numerical font size values
        const vcl::I18nHelper& rI18nHelper = Application::GetSettings().GetUILocaleI18nHelper();
        pTempAry = pAry;
        while ( *pTempAry )
        {
            m_pHeightArray[nPos] = *pTempAry;
            nPos++; // Id is nPos+1
            pVCLPopupMenu->InsertItem( nPos, rI18nHelper.GetNum( *pTempAry, 1, TRUE, FALSE ), MIB_RADIOCHECK | MIB_AUTOCHECK );
            fPoint = float( m_pHeightArray[nPos-1] ) / 10;
            pVCLPopupMenu->SetItemCommand( nPos, rtl::OUString::valueOf( fPoint )); // Store font height into command
            pTempAry++;
        }

        setCurHeight( long( m_aFontHeight.Height * 10), rPopupMenu );

        delete pFontList;
        delete pInfoPrinter;
    }
}

// XEventListener
void SAL_CALL FontSizeMenuController::disposing( const EventObject& Source ) throw ( RuntimeException )
{
    Reference< css::awt::XMenuListener > xHolder(( OWeakObject *)this, UNO_QUERY );

    ResetableGuard aLock( m_aLock );
    m_xFrame.clear();
    m_xDispatch.clear();
    m_xCurrentFontDispatch.clear();
    if ( m_xPopupMenu.is() )
        m_xPopupMenu->removeMenuListener( Reference< css::awt::XMenuListener >(( OWeakObject *)this, UNO_QUERY ));
    m_xPopupMenu.clear();
}

// XStatusListener
void SAL_CALL FontSizeMenuController::statusChanged( const FeatureStateEvent& Event ) throw ( RuntimeException )
{
    com::sun::star::awt::FontDescriptor                 aFontDescriptor;
    drafts::com::sun::star::frame::status::FontHeight   aFontHeight;

    if ( Event.State >>= aFontDescriptor )
    {
        ResetableGuard aLock( m_aLock );
        m_aFontDescriptor = aFontDescriptor;

        if ( m_xPopupMenu.is() )
            fillPopupMenu( m_xPopupMenu );

    }
    else if ( Event.State >>= aFontHeight )
    {
        ResetableGuard aLock( m_aLock );
        m_aFontHeight = aFontHeight;

        if ( m_xPopupMenu.is() )
        {
            vos::OGuard aSolarMutexGuard( Application::GetSolarMutex() );
            setCurHeight( long( m_aFontHeight.Height * 10), m_xPopupMenu );
        }
    }
}

// XMenuListener
void SAL_CALL FontSizeMenuController::highlight( const css::awt::MenuEvent& rEvent ) throw (RuntimeException)
{
}

void SAL_CALL FontSizeMenuController::select( const css::awt::MenuEvent& rEvent ) throw (RuntimeException)
{
    Reference< css::awt::XPopupMenu >   xPopupMenu;
    Reference< XDispatch >              xDispatch;
    Reference< XMultiServiceFactory >   xServiceManager;

    ResetableGuard aLock( m_aLock );
    xPopupMenu      = m_xPopupMenu;
    xDispatch       = m_xDispatch;
    xServiceManager = m_xServiceManager;
    aLock.unlock();

    // Command URL used to dispatch the selected font family name
    const rtl::OUString aFontHeightCommand( RTL_CONSTASCII_USTRINGPARAM( ".uno:FontHeight?FontHeight.Height:float=" ));

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
                PopupMenu* pVCLPopupMenu = (PopupMenu *)pPopupMenu->GetMenu();
                aTargetURL.Complete = aFontHeightCommand;
                aTargetURL.Complete += pVCLPopupMenu->GetItemCommand( rEvent.MenuId );
            }

            xURLTransformer->parseStrict( aTargetURL );
            xDispatch->dispatch( aTargetURL, aArgs );
        }
    }
}

void SAL_CALL FontSizeMenuController::activate( const css::awt::MenuEvent& rEvent ) throw (RuntimeException)
{
}

void SAL_CALL FontSizeMenuController::deactivate( const css::awt::MenuEvent& rEvent ) throw (RuntimeException)
{
}

// XPopupMenuController
void SAL_CALL FontSizeMenuController::setPopupMenu( const Reference< css::awt::XPopupMenu >& xPopupMenu ) throw ( RuntimeException )
{
    ResetableGuard aLock( m_aLock );

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

        // Register for font name updates which gives us info about the current font!
        aTargetURL.Complete = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:CharFontName" ));
        xURLTransformer->parseStrict( aTargetURL );
        m_xCurrentFontDispatch = xDispatchProvider->queryDispatch( aTargetURL, ::rtl::OUString(), 0 );

        // Register for font height updates
        aTargetURL.Complete = m_aCommandURL;
        xURLTransformer->parseStrict( aTargetURL );
        m_xDispatch = xDispatchProvider->queryDispatch( aTargetURL, ::rtl::OUString(), 0 );

        updatePopupMenu();
    }
}

void SAL_CALL FontSizeMenuController::updatePopupMenu() throw ( ::com::sun::star::uno::RuntimeException )
{
    ResetableGuard aLock( m_aLock );
    Reference< XDispatch > xDispatch( m_xCurrentFontDispatch );
    Reference< XURLTransformer > xURLTransformer( m_xServiceManager->createInstance(
                                                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ))),
                                                UNO_QUERY );
    com::sun::star::util::URL aTargetURL;
    aTargetURL.Complete = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:CharFontName" ));
    xURLTransformer->parseStrict( aTargetURL );
    aLock.unlock();

    if ( xDispatch.is() )
    {
        xDispatch->addStatusListener( SAL_STATIC_CAST( XStatusListener*, this ), aTargetURL );
        xDispatch->removeStatusListener( SAL_STATIC_CAST( XStatusListener*, this ), aTargetURL );
    }

    PopupMenuControllerBase::updatePopupMenu();
}

// XInitialization
void SAL_CALL FontSizeMenuController::initialize( const Sequence< Any >& aArguments ) throw ( Exception, RuntimeException )
{
    const rtl::OUString aFrameName( RTL_CONSTASCII_USTRINGPARAM( "Frame" ));
    const rtl::OUString aCommandURLName( RTL_CONSTASCII_USTRINGPARAM( "CommandURL" ));

    ResetableGuard aLock( m_aLock );

    sal_Bool bInitalized( m_bInitialized );
    if ( !bInitalized )
    {
        PropertyValue       aPropValue;
        rtl::OUString       aCommandURL;
        Reference< XFrame > xFrame;

        for ( int i = 0; i < aArguments.getLength(); i++ )
        {
            if ( aArguments[i] >>= aPropValue )
            {
                if ( aPropValue.Name.equalsAscii( "Frame" ))
                    aPropValue.Value >>= xFrame;
                else if ( aPropValue.Name.equalsAscii( "CommandURL" ))
                    aPropValue.Value >>= aCommandURL;
            }
        }

        if ( xFrame.is() && aCommandURL.getLength() )
        {
            ResetableGuard aLock( m_aLock );
            m_xFrame        = xFrame;
            m_aCommandURL   = aCommandURL;

            m_bInitialized = sal_True;
        }
    }
}

}
