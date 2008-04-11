/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: fontsizemenucontroller.cxx,v $
 * $Revision: 1.9 $
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
#include <uielement/fontsizemenucontroller.hxx>

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
#include <com/sun/star/view/XPrintable.hpp>

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#ifndef _VCL_MENU_HXX_
#include <vcl/menu.hxx>
#endif
#include <vcl/mapunit.hxx>
#ifndef _VCL_SVAPP_HXX_
#include <vcl/svapp.hxx>
#endif
#include <vcl/i18nhelp.hxx>
#ifndef _VCL_OUTPUTDEVICE_HXX_
#include <vcl/outdev.hxx>
#endif
#include <vcl/print.hxx>
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
        const rtl::OUString aFontHeightCommand( RTL_CONSTASCII_USTRINGPARAM( ".uno:FontHeight?FontHeight.Height:float=" ));

        // first insert font size names (for simplified/traditional chinese)
        float           fPoint;
        rtl::OUString   aHeightString;
        FontSizeNames   aFontSizeNames( Application::GetSettings().GetUILanguage() );
        m_pHeightArray = new long[nSizeCount+aFontSizeNames.Count()];
        rtl::OUString   aCommand;

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

                    // Create dispatchable .uno command and set it
                    aCommand = aFontHeightCommand + rtl::OUString::valueOf( fPoint );
                    pVCLPopupMenu->SetItemCommand( nPos, aCommand );
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

                        // Create dispatchable .uno command and set it
                        aCommand = aFontHeightCommand + rtl::OUString::valueOf( fPoint );
                        pVCLPopupMenu->SetItemCommand( nPos, aCommand );
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

            // Create dispatchable .uno command and set it
            aCommand = aFontHeightCommand + rtl::OUString::valueOf( fPoint );
            pVCLPopupMenu->SetItemCommand( nPos, aCommand );

            pTempAry++;
        }

        setCurHeight( long( m_aFontHeight.Height * 10), rPopupMenu );

        delete pFontList;
        delete pInfoPrinter;
    }
}

// XEventListener
void SAL_CALL FontSizeMenuController::disposing( const EventObject& ) throw ( RuntimeException )
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
    ::com::sun::star::frame::status::FontHeight   aFontHeight;

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
void SAL_CALL FontSizeMenuController::highlight( const css::awt::MenuEvent& ) throw (RuntimeException)
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
                aTargetURL.Complete = pVCLPopupMenu->GetItemCommand( rEvent.MenuId );
            }

            xURLTransformer->parseStrict( aTargetURL );
            xDispatch->dispatch( aTargetURL, aArgs );
        }
    }
}

void SAL_CALL FontSizeMenuController::activate( const css::awt::MenuEvent& ) throw (RuntimeException)
{
}

void SAL_CALL FontSizeMenuController::deactivate( const css::awt::MenuEvent& ) throw (RuntimeException)
{
}

// XPopupMenuController
void SAL_CALL FontSizeMenuController::setPopupMenu( const Reference< css::awt::XPopupMenu >& xPopupMenu ) throw ( RuntimeException )
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

    if ( m_bDisposed )
        throw DisposedException();

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
    PopupMenuControllerBase::initialize( aArguments );
}

}
