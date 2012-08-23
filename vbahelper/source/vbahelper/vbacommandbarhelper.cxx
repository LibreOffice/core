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
#include "vbacommandbarhelper.hxx"
#include <com/sun/star/ui/ModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XUIConfigurationStorage.hpp>
#include <com/sun/star/ui/XModuleUIConfigurationManager.hpp>
#include <com/sun/star/ui/XUIConfigurationPersistence.hpp>
#include <com/sun/star/ui/XUIElement.hpp>
#include <com/sun/star/ui/UIElementType.hpp>
#include <comphelper/processfactory.hxx>
#include <vbahelper/vbahelper.hxx>
#include <rtl/ustrbuf.hxx>
#include <time.h>
#include <map>

using namespace com::sun::star;
using namespace ooo::vba;

#define CREATEOUSTRING(asciistr) rtl::OUString(asciistr)

typedef std::map< rtl::OUString, rtl::OUString > MSO2OOCommandbarMap;

class MSO2OOCommandbarHelper
{
private:
    static MSO2OOCommandbarHelper* pMSO2OOCommandbarHelper;
    MSO2OOCommandbarMap maBuildinToolbarMap;

    MSO2OOCommandbarHelper()
    {
        // Buildin toolbars
        maBuildinToolbarMap.insert( std::make_pair( CREATEOUSTRING("Standard"),CREATEOUSTRING("private:resource/toolbar/standardbar") ) );
        maBuildinToolbarMap.insert( std::make_pair( CREATEOUSTRING("Formatting"),CREATEOUSTRING("private:resource/toolbar/formatobjectbar") ) );
        maBuildinToolbarMap.insert( std::make_pair( CREATEOUSTRING("Drawing"),CREATEOUSTRING("private:resource/toolbar/drawbar") ) );
        maBuildinToolbarMap.insert( std::make_pair( CREATEOUSTRING("Toolbar List"),CREATEOUSTRING("private:resource/toolbar/toolbar") ) );
        maBuildinToolbarMap.insert( std::make_pair( CREATEOUSTRING("Forms"),CREATEOUSTRING("private:resource/toolbar/formcontrols") ) );
        maBuildinToolbarMap.insert( std::make_pair( CREATEOUSTRING("Form Controls"),CREATEOUSTRING("private:resource/toolbar/formcontrols") ) );
        maBuildinToolbarMap.insert( std::make_pair( CREATEOUSTRING("Full Screen"),CREATEOUSTRING("private:resource/toolbar/fullscreenbar") ) );
        maBuildinToolbarMap.insert( std::make_pair( CREATEOUSTRING("Chart"),CREATEOUSTRING("private:resource/toolbar/flowchartshapes") ) );
        maBuildinToolbarMap.insert( std::make_pair( CREATEOUSTRING("Picture"),CREATEOUSTRING("private:resource/toolbar/graphicobjectbar") ) );
        maBuildinToolbarMap.insert( std::make_pair( CREATEOUSTRING("WordArt"),CREATEOUSTRING("private:resource/toolbar/fontworkobjectbar") ) );
        maBuildinToolbarMap.insert( std::make_pair( CREATEOUSTRING("3-D Settings"),CREATEOUSTRING("private:resource/toolbar/extrusionobjectbar") ) );
    }

public:
    virtual ~MSO2OOCommandbarHelper() {};
    static MSO2OOCommandbarHelper* getMSO2OOCommandbarHelper()
    {
        if( pMSO2OOCommandbarHelper == NULL )
        {
            pMSO2OOCommandbarHelper = new MSO2OOCommandbarHelper();
        }
        return pMSO2OOCommandbarHelper;
    }

    rtl::OUString findBuildinToolbar( const rtl::OUString& sToolbarName )
    {
        MSO2OOCommandbarMap::iterator it = maBuildinToolbarMap.begin();
        for(; it != maBuildinToolbarMap.end(); ++it )
        {
            rtl::OUString sName = it->first;
            if( sName.equalsIgnoreAsciiCase( sToolbarName ) )
                return it->second;
        }
        return rtl::OUString();
    }
};

MSO2OOCommandbarHelper* MSO2OOCommandbarHelper::pMSO2OOCommandbarHelper = NULL;


VbaCommandBarHelper::VbaCommandBarHelper( const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::frame::XModel >& xModel ) throw (css::uno::RuntimeException) : mxContext( xContext ), mxModel( xModel )
{
    Init();
}

void VbaCommandBarHelper::Init( ) throw (css::uno::RuntimeException)
{
    uno::Reference< css::ui::XUIConfigurationManagerSupplier > xUICfgSupplier( mxModel, uno::UNO_QUERY_THROW );
    m_xDocCfgMgr = xUICfgSupplier->getUIConfigurationManager();

    uno::Reference< lang::XServiceInfo > xServiceInfo( mxModel, uno::UNO_QUERY_THROW );
    if( xServiceInfo->supportsService( rtl::OUString( "com.sun.star.sheet.SpreadsheetDocument" ) ) )
    {
        maModuleId = rtl::OUString( "com.sun.star.sheet.SpreadsheetDocument" );
    }
    else if( xServiceInfo->supportsService( rtl::OUString( "com.sun.star.text.TextDocument"  ) ) )
    {
        maModuleId = rtl::OUString( "com.sun.star.text.TextDocument" );
    }

    if( maModuleId.isEmpty() )
    {
        throw uno::RuntimeException( rtl::OUString( "Not implemented" ), uno::Reference< uno::XInterface >() );
    }

    uno::Reference< lang::XMultiServiceFactory > xServiceManager( mxContext->getServiceManager(), uno::UNO_QUERY_THROW );

    css::uno::Reference< css::ui::XModuleUIConfigurationManagerSupplier > xUICfgMgrSupp(
        css::ui::ModuleUIConfigurationManagerSupplier::create(mxContext) );

    m_xAppCfgMgr.set( xUICfgMgrSupp->getUIConfigurationManager( maModuleId ), uno::UNO_QUERY_THROW );

    css::uno::Reference< css::container::XNameAccess > xNameAccess( xServiceManager->createInstance( rtl::OUString(  "com.sun.star.ui.WindowStateConfiguration" )), uno::UNO_QUERY_THROW );

    m_xWindowState.set( xNameAccess->getByName( maModuleId ), uno::UNO_QUERY_THROW );
}

css::uno::Reference< css::container::XIndexAccess > VbaCommandBarHelper::getSettings( const rtl::OUString& sResourceUrl ) throw (css::uno::RuntimeException)
{
    if( m_xDocCfgMgr->hasSettings( sResourceUrl ) )
        return m_xDocCfgMgr->getSettings( sResourceUrl, sal_True );
    else if( m_xAppCfgMgr->hasSettings( sResourceUrl ) )
        return m_xAppCfgMgr->getSettings( sResourceUrl, sal_True );
    else
    {
        css::uno::Reference< css::container::XIndexAccess > xSettings( m_xAppCfgMgr->createSettings( ), uno::UNO_QUERY_THROW );
        return xSettings;
    }
}

void VbaCommandBarHelper::removeSettings( const rtl::OUString& sResourceUrl ) throw (css::uno::RuntimeException)
{
    if( m_xDocCfgMgr->hasSettings( sResourceUrl ) )
        m_xDocCfgMgr->removeSettings( sResourceUrl );
    else if( m_xAppCfgMgr->hasSettings( sResourceUrl ) )
        m_xAppCfgMgr->removeSettings( sResourceUrl );

    // persistChanges();
}

void VbaCommandBarHelper::ApplyChange( const rtl::OUString& sResourceUrl, const css::uno::Reference< css::container::XIndexAccess >& xSettings, sal_Bool bTemporary ) throw (css::uno::RuntimeException)
{
    if( m_xDocCfgMgr->hasSettings( sResourceUrl ) )
    {
        m_xDocCfgMgr->replaceSettings( sResourceUrl, xSettings );
    }
    else
    {
        m_xDocCfgMgr->insertSettings( sResourceUrl, xSettings );
    }
    if( !bTemporary )
    {
        persistChanges();
    }
}

sal_Bool VbaCommandBarHelper::persistChanges() throw (css::uno::RuntimeException)
{
    uno::Reference< css::ui::XUIConfigurationPersistence > xConfigPersistence( m_xDocCfgMgr, uno::UNO_QUERY_THROW );
    sal_Bool result = sal_False;
    if( xConfigPersistence->isModified() )
    {
        xConfigPersistence->store();
        result = sal_True;
    }
    return result;
}

uno::Reference< frame::XLayoutManager > VbaCommandBarHelper::getLayoutManager() throw (uno::RuntimeException)
{
    uno::Reference< frame::XFrame > xFrame( getModel()->getCurrentController()->getFrame(), uno::UNO_QUERY_THROW );
    uno::Reference< beans::XPropertySet > xPropertySet( xFrame, uno::UNO_QUERY_THROW );
    uno::Reference< frame::XLayoutManager > xLayoutManager( xPropertySet->getPropertyValue( rtl::OUString("LayoutManager") ), uno::UNO_QUERY_THROW );
    return xLayoutManager;
}

sal_Bool VbaCommandBarHelper::hasToolbar( const rtl::OUString& sResourceUrl, const rtl::OUString& sName ) throw (css::uno::RuntimeException)
{
    if( m_xDocCfgMgr->hasSettings( sResourceUrl ) )
    {
        rtl::OUString sUIName;
        uno::Reference< beans::XPropertySet > xPropertySet( m_xDocCfgMgr->getSettings( sResourceUrl, sal_False ), uno::UNO_QUERY_THROW );
        xPropertySet->getPropertyValue( rtl::OUString(ITEM_DESCRIPTOR_UINAME) ) >>= sUIName;
        if( sName.equalsIgnoreAsciiCase( sUIName ) )
            return sal_True;
    }
    return sal_False;
}

// return the resource url if found
rtl::OUString VbaCommandBarHelper::findToolbarByName( const css::uno::Reference< css::container::XNameAccess >& xNameAccess, const rtl::OUString& sName ) throw (css::uno::RuntimeException)
{
    rtl::OUString sResourceUrl;

    // check if it is an buildin toolbar
    sResourceUrl = MSO2OOCommandbarHelper::getMSO2OOCommandbarHelper()->findBuildinToolbar( sName );
    if( !sResourceUrl.isEmpty() )
        return sResourceUrl;

    uno::Sequence< ::rtl::OUString > allNames = xNameAccess->getElementNames();
    for( sal_Int32 i = 0; i < allNames.getLength(); i++ )
    {
        sResourceUrl = allNames[i];
        if(sResourceUrl.indexOf( ITEM_TOOLBAR_URL ) == 0 )
        {
            if( hasToolbar( sResourceUrl, sName ) )
                return sResourceUrl;
        }
    }

    // the customize toolbars creating during importing, shoud found there.
    static rtl::OUString sToolbarPrefix(  "private:resource/toolbar/custom_"  );
    sResourceUrl = sToolbarPrefix.concat( sName );
    if( hasToolbar( sResourceUrl, sName ) )
        return sResourceUrl;

    return rtl::OUString();
}

// if found, return the position of the control. if not found, return -1
sal_Int32 VbaCommandBarHelper::findControlByName( const css::uno::Reference< css::container::XIndexAccess >& xIndexAccess, const rtl::OUString& sName, bool bMenu ) throw (css::uno::RuntimeException)
{
    sal_Int32 nCount = xIndexAccess->getCount();
    css::uno::Sequence< css::beans::PropertyValue > aProps;
    for( sal_Int32 i = 0; i < nCount; i++ )
    {
        rtl::OUString sLabel;
        xIndexAccess->getByIndex( i ) >>= aProps;
        getPropertyValue( aProps, rtl::OUString(ITEM_DESCRIPTOR_LABEL) ) >>= sLabel;
        // handle the hotkey marker '~' (remove in toolbars (?), replace by '&' in menus)
        ::rtl::OUStringBuffer aBuffer;
        sal_Int32 index = sLabel.indexOf( sal_Unicode('~') );
        if( index < 0 )
        {
            aBuffer = sLabel;
        }
        else
        {
            aBuffer.append( sLabel.copy( 0, index ) );
            if( bMenu )
                aBuffer.append( sal_Unicode( '&' ) );
            aBuffer.append( sLabel.copy( index + 1 ) );
        }
        rtl::OUString sNewLabel = aBuffer.makeStringAndClear();
        OSL_TRACE("VbaCommandBarHelper::findControlByName, control name: %s", rtl::OUStringToOString( sNewLabel, RTL_TEXTENCODING_UTF8 ).getStr() );
        if( sName.equalsIgnoreAsciiCase( sNewLabel ) )
            return i;
    }

    // not found
    return -1;
}

rtl::OUString VbaCommandBarHelper::generateCustomURL()
{
    rtl::OUString url( ITEM_TOOLBAR_URL );
    url += rtl::OUString( CUSTOM_TOOLBAR_STR );

    // use a random number to minimize possible clash with existing custom toolbars
    srand( unsigned( time( NULL ) ));
    url += rtl::OUString::valueOf( sal_Int64( rand() ), 16 );
    return url;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
