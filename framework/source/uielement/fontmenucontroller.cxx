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

#include <uielement/fontmenucontroller.hxx>

#include <threadhelp/resetableguard.hxx>
#include "services.h"

#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/awt/MenuItemStyle.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>

#include <vcl/menu.hxx>
#include <vcl/svapp.hxx>
#include <vcl/i18nhelp.hxx>
#include <tools/urlobj.hxx>
#include <rtl/ustrbuf.hxx>
#include <vcl/mnemonic.hxx>
#include <osl/mutex.hxx>

//_________________________________________________________________________________________________________________
//  Defines
//_________________________________________________________________________________________________________________

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::frame;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;

using namespace std;

bool lcl_I18nCompareString(const rtl::OUString& rStr1, const rtl::OUString& rStr2)
{
    const vcl::I18nHelper& rI18nHelper = Application::GetSettings().GetUILocaleI18nHelper();
    return rI18nHelper.CompareString( rStr1, rStr2 ) < 0 ? true : false;
}

namespace framework
{

DEFINE_XSERVICEINFO_MULTISERVICE        (   FontMenuController                      ,
                                            OWeakObject                             ,
                                            SERVICENAME_POPUPMENUCONTROLLER         ,
                                            IMPLEMENTATIONNAME_FONTMENUCONTROLLER
                                        )

DEFINE_INIT_SERVICE                     (   FontMenuController, {} )

FontMenuController::FontMenuController( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager ) :
    svt::PopupMenuControllerBase( xServiceManager )
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

    SolarMutexGuard aSolarMutexGuard;

    resetPopupMenu( rPopupMenu );
    if ( pPopupMenu )
        pVCLPopupMenu = (PopupMenu *)pPopupMenu->GetMenu();

    if ( pVCLPopupMenu )
    {
        vector<rtl::OUString> aVector;
        aVector.reserve(rFontNameSeq.getLength());
        for ( sal_uInt16 i = 0; i < rFontNameSeq.getLength(); i++ )
        {
            aVector.push_back(MnemonicGenerator::EraseAllMnemonicChars(pFontNameArray[i]));
        }
        sort(aVector.begin(), aVector.end(), lcl_I18nCompareString );

        const rtl::OUString aFontNameCommandPrefix( ".uno:CharFontName?CharFontName.FamilyName:string=" );
        const sal_Int16 nCount = (sal_Int16)aVector.size();
        for ( sal_Int16 i = 0; i < nCount; i++ )
        {
            const rtl::OUString& rName = aVector[i];
            m_xPopupMenu->insertItem( i+1, rName, css::awt::MenuItemStyle::RADIOCHECK | css::awt::MenuItemStyle::AUTOCHECK, i );
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

    osl::MutexGuard aLock( m_aMutex );
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
void FontMenuController::impl_select(const Reference< XDispatch >& _xDispatch,const ::com::sun::star::util::URL& aTargetURL)
{
    Sequence<PropertyValue>      aArgs;
    OSL_ENSURE(_xDispatch.is(),"FontMenuController::impl_select: No dispatch");
    if ( _xDispatch.is() )
        _xDispatch->dispatch( aTargetURL, aArgs );
}

void SAL_CALL FontMenuController::activate( const css::awt::MenuEvent& ) throw (RuntimeException)
{
    osl::MutexGuard aLock( m_aMutex );

    if ( m_xPopupMenu.is() )
    {
        // find new font name and set check mark!
        sal_uInt16        nChecked = 0;
        sal_uInt16        nItemCount = m_xPopupMenu->getItemCount();
        rtl::OUString aEmpty;
        for( sal_uInt16 i = 0; i < nItemCount; i++ )
        {
            sal_uInt16 nItemId = m_xPopupMenu->getItemId( i );

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

// XPopupMenuController
void FontMenuController::impl_setPopupMenu()
{
    Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );

    com::sun::star::util::URL aTargetURL;
    // Register for font list updates to get the current font list from the controller
    aTargetURL.Complete = rtl::OUString( ".uno:FontNameList" );
    m_xURLTransformer->parseStrict( aTargetURL );
    m_xFontListDispatch = xDispatchProvider->queryDispatch( aTargetURL, ::rtl::OUString(), 0 );
}

void SAL_CALL FontMenuController::updatePopupMenu() throw ( ::com::sun::star::uno::RuntimeException )
{
    svt::PopupMenuControllerBase::updatePopupMenu();

    osl::ClearableMutexGuard aLock( m_aMutex );
    Reference< XDispatch > xDispatch( m_xFontListDispatch );
    com::sun::star::util::URL aTargetURL;
    aTargetURL.Complete = rtl::OUString( ".uno:FontNameList" );
    m_xURLTransformer->parseStrict( aTargetURL );
    aLock.clear();

    if ( xDispatch.is() )
    {
        xDispatch->addStatusListener( (static_cast< XStatusListener* >(this)), aTargetURL );
        xDispatch->removeStatusListener( (static_cast< XStatusListener* >(this)), aTargetURL );
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
