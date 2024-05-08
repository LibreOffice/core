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

#include <uielement/fontmenucontroller.hxx>

#include <services.h>

#include <com/sun/star/awt/MenuItemStyle.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>

#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/i18nhelp.hxx>
#include <tools/urlobj.hxx>
#include <vcl/mnemonic.hxx>
#include <osl/mutex.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <toolkit/awt/vclxmenu.hxx>

//  Defines

using namespace css::uno;
using namespace css::lang;
using namespace css::frame;
using namespace css::util;

static bool lcl_I18nCompareString(const OUString& rStr1, const OUString& rStr2)
{
    const vcl::I18nHelper& rI18nHelper = Application::GetSettings().GetUILocaleI18nHelper();
    return rI18nHelper.CompareString( rStr1, rStr2 ) < 0;
}

namespace framework
{

// XInterface, XTypeProvider, XServiceInfo

OUString SAL_CALL FontMenuController::getImplementationName()
{
    return u"com.sun.star.comp.framework.FontMenuController"_ustr;
}

sal_Bool SAL_CALL FontMenuController::supportsService( const OUString& sServiceName )
{
    return cppu::supportsService(this, sServiceName);
}

css::uno::Sequence< OUString > SAL_CALL FontMenuController::getSupportedServiceNames()
{
    return { SERVICENAME_POPUPMENUCONTROLLER };
}

FontMenuController::FontMenuController( const css::uno::Reference< css::uno::XComponentContext >& xContext ) :
    svt::PopupMenuControllerBase( xContext )
{
}

FontMenuController::~FontMenuController()
{
}

// private function
void FontMenuController::fillPopupMenu( const Sequence< OUString >& rFontNameSeq, Reference< css::awt::XPopupMenu > const & rPopupMenu )
{
    SolarMutexGuard aSolarMutexGuard;

    resetPopupMenu( rPopupMenu );

    std::vector<OUString> aVector;
    aVector.reserve(rFontNameSeq.getLength());
    for ( OUString const & s : rFontNameSeq )
    {
        aVector.push_back(MnemonicGenerator::EraseAllMnemonicChars(s));
    }
    sort(aVector.begin(), aVector.end(), lcl_I18nCompareString );

    static constexpr OUStringLiteral aFontNameCommandPrefix( u".uno:CharFontName?CharFontName.FamilyName:string=" );
    const sal_Int16 nCount = static_cast<sal_Int16>(aVector.size());
    for ( sal_Int16 i = 0; i < nCount; i++ )
    {
        const OUString& rName = aVector[i];
        m_xPopupMenu->insertItem( i+1, rName, css::awt::MenuItemStyle::RADIOCHECK | css::awt::MenuItemStyle::AUTOCHECK, i );
        if ( rName == m_aFontFamilyName )
            m_xPopupMenu->checkItem( i+1, true );
        OUString aFontNameCommand = aFontNameCommandPrefix + INetURLObject::encode( rName, INetURLObject::PART_HTTP_QUERY, INetURLObject::EncodeMechanism::All );
        m_xPopupMenu->setCommand(i + 1, aFontNameCommand); // Store font name into item command.
    }
}

// XEventListener
void SAL_CALL FontMenuController::disposing( const EventObject& )
{
    Reference< css::awt::XMenuListener > xHolder(this);

    std::unique_lock aLock( m_aMutex );
    m_xFrame.clear();
    m_xDispatch.clear();
    m_xFontListDispatch.clear();

    if ( m_xPopupMenu.is() )
        m_xPopupMenu->removeMenuListener( Reference< css::awt::XMenuListener >(this) );
    m_xPopupMenu.clear();
}

// XStatusListener
void SAL_CALL FontMenuController::statusChanged( const FeatureStateEvent& Event )
{
    css::awt::FontDescriptor aFontDescriptor;
    Sequence< OUString >           aFontNameSeq;

    if ( Event.State >>= aFontDescriptor )
    {
        std::unique_lock aLock( m_aMutex );
        m_aFontFamilyName = aFontDescriptor.Name;
    }
    else if ( Event.State >>= aFontNameSeq )
    {
        std::unique_lock aLock( m_aMutex );
        if ( m_xPopupMenu.is() )
            fillPopupMenu( aFontNameSeq, m_xPopupMenu );
    }
}

// XMenuListener
void SAL_CALL FontMenuController::itemActivated( const css::awt::MenuEvent& )
{
    std::unique_lock aLock( m_aMutex );

    if ( !m_xPopupMenu.is() )
        return;

    // find new font name and set check mark!
    sal_uInt16        nChecked = 0;
    sal_uInt16        nItemCount = m_xPopupMenu->getItemCount();
    for( sal_uInt16 i = 0; i < nItemCount; i++ )
    {
        sal_uInt16 nItemId = m_xPopupMenu->getItemId( i );

        if ( m_xPopupMenu->isItemChecked( nItemId ) )
            nChecked = nItemId;

        OUString aText = m_xPopupMenu->getItemText( nItemId );

        // TODO: must be replaced by implementation of VCL, when available
        sal_Int32 nIndex = aText.indexOf( '~' );
        if ( nIndex >= 0 )
            aText = aText.replaceAt( nIndex, 1, u"" );
        // TODO: must be replaced by implementation of VCL, when available

        if ( aText == m_aFontFamilyName )
        {
            m_xPopupMenu->checkItem( nItemId, true );
            return;
        }
    }

    if ( nChecked )
        m_xPopupMenu->checkItem( nChecked, false );
}

// XPopupMenuController
void FontMenuController::impl_setPopupMenu()
{
    Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );

    css::util::URL aTargetURL;
    // Register for font list updates to get the current font list from the controller
    aTargetURL.Complete = ".uno:FontNameList";
    m_xURLTransformer->parseStrict( aTargetURL );
    m_xFontListDispatch = xDispatchProvider->queryDispatch( aTargetURL, OUString(), 0 );
}

void SAL_CALL FontMenuController::updatePopupMenu()
{
    svt::PopupMenuControllerBase::updatePopupMenu();

    std::unique_lock aLock( m_aMutex );
    Reference< XDispatch > xDispatch( m_xFontListDispatch );
    css::util::URL aTargetURL;
    aTargetURL.Complete = ".uno:FontNameList";
    m_xURLTransformer->parseStrict( aTargetURL );
    aLock.unlock();

    if ( xDispatch.is() )
    {
        xDispatch->addStatusListener( static_cast< XStatusListener* >(this), aTargetURL );
        xDispatch->removeStatusListener( static_cast< XStatusListener* >(this), aTargetURL );
    }
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
framework_FontMenuController_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const& )
{
    return cppu::acquire(new framework::FontMenuController(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
