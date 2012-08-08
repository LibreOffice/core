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

#include <uielement/fontsizemenucontroller.hxx>

#include <threadhelp/resetableguard.hxx>
#include "services.h"

#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/awt/MenuItemStyle.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/view/XPrintable.hpp>

#include <vcl/menu.hxx>
#include <tools/mapunit.hxx>
#include <vcl/svapp.hxx>
#include <vcl/i18nhelp.hxx>
#include <vcl/outdev.hxx>
#include <vcl/print.hxx>
#include <svtools/ctrltool.hxx>
#include <osl/mutex.hxx>

//_________________________________________________________________________________________________________________
//  Defines
//_________________________________________________________________________________________________________________

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
    svt::PopupMenuControllerBase( xServiceManager ),
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
                    if ( aPrinterSeq[i].Name == "Name" )
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
    rtl::OUString   aHeight     = Application::GetSettings().GetUILocaleI18nHelper().GetNum( nHeight, 1, sal_True, sal_False  );
    sal_uInt16          nChecked    = 0;
    sal_uInt16          nItemCount  = rPopupMenu->getItemCount();
    for( sal_uInt16 i = 0; i < nItemCount; i++ )
    {
        sal_uInt16 nItemId = rPopupMenu->getItemId( i );

        if ( m_pHeightArray[i] == nHeight )
        {
            rPopupMenu->checkItem( nItemId, sal_True );
            return;
        }

        if ( rPopupMenu->isItemChecked( nItemId ) )
            nChecked = nItemId;
    }

    if ( nChecked )
        rPopupMenu->checkItem( nChecked, sal_False );
}

// private function
void FontSizeMenuController::fillPopupMenu( Reference< css::awt::XPopupMenu >& rPopupMenu )
{
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

        SolarMutexGuard aSolarMutexGuard;

        // try to retrieve printer name of document
        aPrinterName = retrievePrinterName( m_xFrame );
        if ( !aPrinterName.isEmpty() )
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
        sal_uInt16 nSizeCount = 0;
        while ( pAry[nSizeCount] )
            nSizeCount++;

        sal_uInt16 nPos = 0;
        const rtl::OUString aFontHeightCommand( ".uno:FontHeight?FontHeight.Height:float=" );

        // first insert font size names (for simplified/traditional chinese)
        float           fPoint;
        FontSizeNames   aFontSizeNames( Application::GetSettings().GetUILanguage() );
        m_pHeightArray = new long[nSizeCount+aFontSizeNames.Count()];
        rtl::OUString   aCommand;

        if ( !aFontSizeNames.IsEmpty() )
        {
            if ( pAry == pFontList->GetStdSizeAry() )
            {
                // for scalable fonts all font size names
                sal_uLong nCount = aFontSizeNames.Count();
                for( sal_uLong i = 0; i < nCount; i++ )
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
            pVCLPopupMenu->InsertItem( nPos, rI18nHelper.GetNum( *pTempAry, 1, sal_True, sal_False ), MIB_RADIOCHECK | MIB_AUTOCHECK );
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

    osl::MutexGuard aLock( m_aMutex );
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
        osl::MutexGuard aLock( m_aMutex );
        m_aFontDescriptor = aFontDescriptor;

        if ( m_xPopupMenu.is() )
            fillPopupMenu( m_xPopupMenu );

    }
    else if ( Event.State >>= aFontHeight )
    {
        osl::MutexGuard aLock( m_aMutex );
        m_aFontHeight = aFontHeight;

        if ( m_xPopupMenu.is() )
        {
            SolarMutexGuard aSolarMutexGuard;
            setCurHeight( long( m_aFontHeight.Height * 10), m_xPopupMenu );
        }
    }
}

// XMenuListener
void FontSizeMenuController::impl_select(const Reference< XDispatch >& _xDispatch,const ::com::sun::star::util::URL& aTargetURL)
{
    Sequence<PropertyValue>      aArgs;
    OSL_ENSURE(_xDispatch.is(),"FontSizeMenuController::impl_select: No dispatch");
    if ( _xDispatch.is() )
        _xDispatch->dispatch( aTargetURL, aArgs );
}

// XPopupMenuController
void FontSizeMenuController::impl_setPopupMenu()
{
    Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );
    com::sun::star::util::URL aTargetURL;
    // Register for font name updates which gives us info about the current font!
    aTargetURL.Complete = rtl::OUString( ".uno:CharFontName" );
    m_xURLTransformer->parseStrict( aTargetURL );
    m_xCurrentFontDispatch = xDispatchProvider->queryDispatch( aTargetURL, ::rtl::OUString(), 0 );
}

void SAL_CALL FontSizeMenuController::updatePopupMenu() throw ( ::com::sun::star::uno::RuntimeException )
{
    osl::ClearableMutexGuard aLock( m_aMutex );

    throwIfDisposed();

    Reference< XDispatch > xDispatch( m_xCurrentFontDispatch );
    com::sun::star::util::URL aTargetURL;
    aTargetURL.Complete = rtl::OUString( ".uno:CharFontName" );
    m_xURLTransformer->parseStrict( aTargetURL );
    aLock.clear();

    if ( xDispatch.is() )
    {
        xDispatch->addStatusListener( (static_cast< XStatusListener* >(this)), aTargetURL );
        xDispatch->removeStatusListener( (static_cast< XStatusListener* >(this)), aTargetURL );
    }

    svt::PopupMenuControllerBase::updatePopupMenu();
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
