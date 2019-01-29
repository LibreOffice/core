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

#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/awt/MenuItemStyle.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>

#include <vcl/menu.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/i18nhelp.hxx>
#include <tools/urlobj.hxx>
#include <rtl/ustrbuf.hxx>
#include <vcl/mnemonic.hxx>
#include <osl/mutex.hxx>

//  Defines

using namespace css::uno;
using namespace css::lang;
using namespace css::frame;
using namespace css::beans;
using namespace css::util;

using namespace std;

static bool lcl_I18nCompareString(const OUString& rStr1, const OUString& rStr2)
{
    const vcl::I18nHelper& rI18nHelper = Application::GetSettings().GetUILocaleI18nHelper();
    return rI18nHelper.CompareString( rStr1, rStr2 ) < 0;
}

namespace framework
{

DEFINE_XSERVICEINFO_MULTISERVICE_2      (   FontMenuController                      ,
                                            OWeakObject                             ,
                                            SERVICENAME_POPUPMENUCONTROLLER         ,
                                            IMPLEMENTATIONNAME_FONTMENUCONTROLLER
                                        )

DEFINE_INIT_SERVICE                     (   FontMenuController, {} )

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
    const OUString*    pFontNameArray = rFontNameSeq.getConstArray();
    VCLXPopupMenu*     pPopupMenu = static_cast<VCLXPopupMenu *>(VCLXMenu::GetImplementation( rPopupMenu ));
    PopupMenu*         pVCLPopupMenu = nullptr;

    SolarMutexGuard aSolarMutexGuard;

    resetPopupMenu( rPopupMenu );
    if ( pPopupMenu )
        pVCLPopupMenu = static_cast<PopupMenu *>(pPopupMenu->GetMenu());

    if ( pVCLPopupMenu )
    {
        vector<OUString> aVector;
        aVector.reserve(rFontNameSeq.getLength());
        for ( sal_Int32 i = 0; i < rFontNameSeq.getLength(); i++ )
        {
            aVector.push_back(MnemonicGenerator::EraseAllMnemonicChars(pFontNameArray[i]));
        }
        sort(aVector.begin(), aVector.end(), lcl_I18nCompareString );

        const OUString aFontNameCommandPrefix( ".uno:CharFontName?CharFontName.FamilyName:string=" );
        const sal_Int16 nCount = static_cast<sal_Int16>(aVector.size());
        for ( sal_Int16 i = 0; i < nCount; i++ )
        {
            const OUString& rName = aVector[i];
            m_xPopupMenu->insertItem( i+1, rName, css::awt::MenuItemStyle::RADIOCHECK | css::awt::MenuItemStyle::AUTOCHECK, i );
            if ( rName == m_aFontFamilyName )
                m_xPopupMenu->checkItem( i+1, true );
            // use VCL popup menu pointer to set vital information that are not part of the awt implementation
            OUStringBuffer aCommandBuffer( aFontNameCommandPrefix );
            aCommandBuffer.append( INetURLObject::encode( rName, INetURLObject::PART_HTTP_QUERY, INetURLObject::EncodeMechanism::All ));
            OUString aFontNameCommand = aCommandBuffer.makeStringAndClear();
            pVCLPopupMenu->SetItemCommand( i+1, aFontNameCommand ); // Store font name into item command.
        }

    }
}

// XEventListener
void SAL_CALL FontMenuController::disposing( const EventObject& )
{
    Reference< css::awt::XMenuListener > xHolder(static_cast<OWeakObject *>(this), UNO_QUERY );

    osl::MutexGuard aLock( m_aMutex );
    m_xFrame.clear();
    m_xDispatch.clear();
    m_xFontListDispatch.clear();

    if ( m_xPopupMenu.is() )
        m_xPopupMenu->removeMenuListener( Reference< css::awt::XMenuListener >(static_cast<OWeakObject *>(this), UNO_QUERY ));
    m_xPopupMenu.clear();
}

// XStatusListener
void SAL_CALL FontMenuController::statusChanged( const FeatureStateEvent& Event )
{
    css::awt::FontDescriptor aFontDescriptor;
    Sequence< OUString >           aFontNameSeq;

    if ( Event.State >>= aFontDescriptor )
    {
        osl::MutexGuard aLock( m_aMutex );
        m_aFontFamilyName = aFontDescriptor.Name;
    }
    else if ( Event.State >>= aFontNameSeq )
    {
        osl::MutexGuard aLock( m_aMutex );
        if ( m_xPopupMenu.is() )
            fillPopupMenu( aFontNameSeq, m_xPopupMenu );
    }
}

// XMenuListener
void SAL_CALL FontMenuController::itemActivated( const css::awt::MenuEvent& )
{
    osl::MutexGuard aLock( m_aMutex );

    if ( m_xPopupMenu.is() )
    {
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
                aText = aText.replaceAt( nIndex, 1, "" );
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

    osl::ClearableMutexGuard aLock( m_aMutex );
    Reference< XDispatch > xDispatch( m_xFontListDispatch );
    css::util::URL aTargetURL;
    aTargetURL.Complete = ".uno:FontNameList";
    m_xURLTransformer->parseStrict( aTargetURL );
    aLock.clear();

    if ( xDispatch.is() )
    {
        xDispatch->addStatusListener( static_cast< XStatusListener* >(this), aTargetURL );
        xDispatch->removeStatusListener( static_cast< XStatusListener* >(this), aTargetURL );
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
