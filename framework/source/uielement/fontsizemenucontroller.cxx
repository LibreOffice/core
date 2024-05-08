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

#include <sal/config.h>

#include <uielement/fontsizemenucontroller.hxx>

#include <services.h>

#include <com/sun/star/awt/MenuItemStyle.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/view/XPrintable.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>

#include <vcl/svapp.hxx>
#include <vcl/i18nhelp.hxx>
#include <vcl/print.hxx>
#include <vcl/settings.hxx>
#include <svtools/ctrltool.hxx>
#include <toolkit/awt/vclxmenu.hxx>
#include <osl/mutex.hxx>
#include <memory>
#include <cppuhelper/supportsservice.hxx>

//  Defines

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::frame;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;
using namespace com::sun::star::view;

namespace framework
{

OUString SAL_CALL FontSizeMenuController::getImplementationName()
{
    return u"com.sun.star.comp.framework.FontSizeMenuController"_ustr;
}

sal_Bool SAL_CALL FontSizeMenuController::supportsService( const OUString& sServiceName )
{
    return cppu::supportsService(this, sServiceName);
}

css::uno::Sequence< OUString > SAL_CALL FontSizeMenuController::getSupportedServiceNames()
{
    return { SERVICENAME_POPUPMENUCONTROLLER };
}

FontSizeMenuController::FontSizeMenuController( const css::uno::Reference< css::uno::XComponentContext >& xContext ) :
    svt::PopupMenuControllerBase( xContext )
{
}

FontSizeMenuController::~FontSizeMenuController()
{
}

// private function
OUString FontSizeMenuController::retrievePrinterName( css::uno::Reference< css::frame::XFrame > const & rFrame )
{
    OUString aPrinterName;

    if ( rFrame.is() )
    {
        Reference< XController > xController = m_xFrame->getController();
        if ( xController.is() )
        {
            Reference< XPrintable > xPrintable( xController->getModel(), UNO_QUERY );
            if ( xPrintable.is() )
            {
                const Sequence< PropertyValue > aPrinterSeq = xPrintable->getPrinter();
                for ( PropertyValue const & prop : aPrinterSeq )
                {
                    if ( prop.Name == "Name" )
                    {
                        prop.Value >>= aPrinterName;
                        break;
                    }
                }
            }
        }
    }

    return aPrinterName;
}

// private function
void FontSizeMenuController::setCurHeight( tools::Long nHeight, Reference< css::awt::XPopupMenu > const & rPopupMenu )
{
    // check menu item
    sal_uInt16          nChecked    = 0;
    sal_uInt16          nItemCount  = rPopupMenu->getItemCount();
    for( sal_uInt16 i = 0; i < nItemCount; i++ )
    {
        sal_uInt16 nItemId = rPopupMenu->getItemId( i );

        if ( m_aHeightArray[i] == nHeight )
        {
            rPopupMenu->checkItem( nItemId, true );
            return;
        }

        if ( rPopupMenu->isItemChecked( nItemId ) )
            nChecked = nItemId;
    }

    if ( nChecked )
        rPopupMenu->checkItem( nChecked, false );
}

// private function
void FontSizeMenuController::fillPopupMenu( Reference< css::awt::XPopupMenu > const & rPopupMenu )
{
    resetPopupMenu( rPopupMenu );

    std::unique_ptr<FontList> pFontList;
    ScopedVclPtr<Printer>  pInfoPrinter;
    OUString   aPrinterName;

    SolarMutexGuard aSolarMutexGuard;

    // try to retrieve printer name of document
    aPrinterName = retrievePrinterName( m_xFrame );
    if ( !aPrinterName.isEmpty() )
    {
        pInfoPrinter.disposeAndReset(VclPtr<Printer>::Create( aPrinterName ));
        if ( pInfoPrinter && pInfoPrinter->GetFontFaceCollectionCount() > 0 )
            pFontList.reset(new FontList( pInfoPrinter.get() ));
    }

    if ( !pFontList )
        pFontList.reset(new FontList( Application::GetDefaultDevice() ));

    // setup font size array
    m_aHeightArray.clear();

    sal_uInt16 nPos = 0; // Id is nPos+1
    static constexpr OUString aFontHeightCommand( u".uno:FontHeight?FontHeight.Height:float="_ustr );

    // first insert font size names (for simplified/traditional chinese)
    FontSizeNames   aFontSizeNames( Application::GetSettings().GetUILanguageTag().getLanguageType() );
    OUString   aCommand;

    if (!aFontSizeNames.IsEmpty())
    {
        // for scalable fonts all font size names
        sal_Int32 nCount = aFontSizeNames.Count();
        for( sal_Int32 i = 0; i < nCount; i++ )
        {
            OUString aSizeName = aFontSizeNames.GetIndexName( i );
            sal_Int32 nSize = aFontSizeNames.GetIndexSize( i );
            m_aHeightArray.push_back(nSize);
            rPopupMenu->insertItem(nPos + 1, aSizeName, css::awt::MenuItemStyle::RADIOCHECK | css::awt::MenuItemStyle::AUTOCHECK, nPos);

            // Create dispatchable .uno command and set it
            float fPoint = float(nSize) / 10;
            aCommand = aFontHeightCommand + OUString::number( fPoint );
            rPopupMenu->setCommand(nPos + 1, aCommand);

            ++nPos;
        }
    }

    // then insert numerical font size values
    const vcl::I18nHelper& rI18nHelper = Application::GetSettings().GetUILocaleI18nHelper();
    const int* pAry = FontList::GetStdSizeAry();
    const int* pTempAry = pAry;
    while ( *pTempAry )
    {
        m_aHeightArray.push_back(*pTempAry);
        rPopupMenu->insertItem(nPos + 1, rI18nHelper.GetNum(*pTempAry, 1, true, false),
                               css::awt::MenuItemStyle::RADIOCHECK | css::awt::MenuItemStyle::AUTOCHECK, nPos);

        // Create dispatchable .uno command and set it
        float fPoint = float(*pTempAry) / 10;
        aCommand = aFontHeightCommand + OUString::number( fPoint );
        rPopupMenu->setCommand(nPos + 1, aCommand);

        ++nPos;
        pTempAry++;
    }

    setCurHeight( tools::Long( m_aFontHeight.Height * 10), rPopupMenu );
}

// XEventListener
void SAL_CALL FontSizeMenuController::disposing( const EventObject& )
{
    Reference< css::awt::XMenuListener > xHolder(this);

    std::unique_lock aLock( m_aMutex );
    m_xFrame.clear();
    m_xDispatch.clear();
    m_xCurrentFontDispatch.clear();
    if ( m_xPopupMenu.is() )
        m_xPopupMenu->removeMenuListener( Reference< css::awt::XMenuListener >(this) );
    m_xPopupMenu.clear();
}

// XStatusListener
void SAL_CALL FontSizeMenuController::statusChanged( const FeatureStateEvent& Event )
{
    css::awt::FontDescriptor                 aFontDescriptor;
    css::frame::status::FontHeight   aFontHeight;

    if ( Event.State >>= aFontDescriptor )
    {
        std::unique_lock aLock( m_aMutex );

        if ( m_xPopupMenu.is() )
            fillPopupMenu( m_xPopupMenu );
    }
    else if ( Event.State >>= aFontHeight )
    {
        std::unique_lock aLock( m_aMutex );
        m_aFontHeight = aFontHeight;

        if ( m_xPopupMenu.is() )
        {
            SolarMutexGuard aSolarMutexGuard;
            setCurHeight( tools::Long( m_aFontHeight.Height * 10), m_xPopupMenu );
        }
    }
}

// XPopupMenuController
void FontSizeMenuController::impl_setPopupMenu()
{
    Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );
    css::util::URL aTargetURL;
    // Register for font name updates which gives us info about the current font!
    aTargetURL.Complete = ".uno:CharFontName";
    m_xURLTransformer->parseStrict( aTargetURL );
    m_xCurrentFontDispatch = xDispatchProvider->queryDispatch( aTargetURL, OUString(), 0 );
}

void SAL_CALL FontSizeMenuController::updatePopupMenu()
{
    std::unique_lock aLock( m_aMutex );

    throwIfDisposed(aLock);

    Reference< XDispatch > xDispatch( m_xCurrentFontDispatch );
    css::util::URL aTargetURL;
    aTargetURL.Complete = ".uno:CharFontName";
    m_xURLTransformer->parseStrict( aTargetURL );
    aLock.unlock();

    if ( xDispatch.is() )
    {
        xDispatch->addStatusListener( static_cast< XStatusListener* >(this), aTargetURL );
        xDispatch->removeStatusListener( static_cast< XStatusListener* >(this), aTargetURL );
    }

    svt::PopupMenuControllerBase::updatePopupMenu();
}
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
framework_FontSizeMenuController_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const& )
{
    return cppu::acquire(new framework::FontSizeMenuController(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
