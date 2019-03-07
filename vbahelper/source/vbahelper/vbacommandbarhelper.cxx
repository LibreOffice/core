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
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/ui/theModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XUIConfigurationStorage.hpp>
#include <com/sun/star/ui/XModuleUIConfigurationManager.hpp>
#include <com/sun/star/ui/XUIConfigurationPersistence.hpp>
#include <com/sun/star/ui/XUIElement.hpp>
#include <com/sun/star/ui/UIElementType.hpp>
#include <com/sun/star/ui/theWindowStateConfiguration.hpp>
#include <comphelper/random.hxx>
#include <comphelper/processfactory.hxx>
#include <vbahelper/vbahelper.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <time.h>
#include <map>
#include <string_view>

using namespace com::sun::star;
using namespace ooo::vba;

typedef std::map< OUString, OUString > MSO2OOCommandbarMap;

class MSO2OOCommandbarHelper final
{
private:
    static MSO2OOCommandbarHelper* pMSO2OOCommandbarHelper;
    MSO2OOCommandbarMap maBuildinToolbarMap;

    MSO2OOCommandbarHelper()
    {
        // Buildin toolbars
        maBuildinToolbarMap.insert( std::make_pair( OUString("Standard") ,    OUString("private:resource/toolbar/standardbar" ) ) );
        maBuildinToolbarMap.insert( std::make_pair( OUString("Formatting"),   OUString("private:resource/toolbar/formatobjectbar") ) );
        maBuildinToolbarMap.insert( std::make_pair( OUString("Drawing"),      OUString("private:resource/toolbar/drawbar") ) );
        maBuildinToolbarMap.insert( std::make_pair( OUString("Toolbar List"), OUString("private:resource/toolbar/toolbar") ) );
        maBuildinToolbarMap.insert( std::make_pair( OUString("Forms"),        OUString("private:resource/toolbar/formcontrols") ) );
        maBuildinToolbarMap.insert( std::make_pair( OUString("Form Controls"),OUString("private:resource/toolbar/formcontrols") ) );
        maBuildinToolbarMap.insert( std::make_pair( OUString("Full Screen"),  OUString("private:resource/toolbar/fullscreenbar") ) );
        maBuildinToolbarMap.insert( std::make_pair( OUString("Chart"),        OUString("private:resource/toolbar/flowchartshapes") ) );
        maBuildinToolbarMap.insert( std::make_pair( OUString("Picture"),      OUString("private:resource/toolbar/graphicobjectbar") ) );
        maBuildinToolbarMap.insert( std::make_pair( OUString("WordArt"),      OUString("private:resource/toolbar/fontworkobjectbar") ) );
        maBuildinToolbarMap.insert( std::make_pair( OUString("3-D Settings"), OUString("private:resource/toolbar/extrusionobjectbar") ) );
    }

public:
    static MSO2OOCommandbarHelper* getMSO2OOCommandbarHelper()
    {
        if( pMSO2OOCommandbarHelper == nullptr )
        {
            pMSO2OOCommandbarHelper = new MSO2OOCommandbarHelper();
        }
        return pMSO2OOCommandbarHelper;
    }

    OUString findBuildinToolbar( const OUString& sToolbarName )
    {
        auto it = std::find_if(maBuildinToolbarMap.begin(), maBuildinToolbarMap.end(),
            [&sToolbarName](const MSO2OOCommandbarMap::value_type& rItem) { return rItem.first.equalsIgnoreAsciiCase( sToolbarName ); });
        if( it != maBuildinToolbarMap.end() )
            return it->second;
        return OUString();
    }
};

MSO2OOCommandbarHelper* MSO2OOCommandbarHelper::pMSO2OOCommandbarHelper = nullptr;


VbaCommandBarHelper::VbaCommandBarHelper( const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::frame::XModel >& xModel ) : mxContext( xContext ), mxModel( xModel )
{
    Init();
}

void VbaCommandBarHelper::Init( )
{
    uno::Reference< css::ui::XUIConfigurationManagerSupplier > xUICfgSupplier( mxModel, uno::UNO_QUERY_THROW );
    m_xDocCfgMgr = xUICfgSupplier->getUIConfigurationManager();

    uno::Reference< lang::XServiceInfo > xServiceInfo( mxModel, uno::UNO_QUERY_THROW );
    if( xServiceInfo->supportsService( "com.sun.star.sheet.SpreadsheetDocument" ) )
    {
        maModuleId = "com.sun.star.sheet.SpreadsheetDocument";
    }
    else if( xServiceInfo->supportsService( "com.sun.star.text.TextDocument" ) )
    {
        maModuleId = "com.sun.star.text.TextDocument";
    }

    if( maModuleId.isEmpty() )
    {
        throw uno::RuntimeException( "Not implemented" );
    }

    css::uno::Reference< css::ui::XModuleUIConfigurationManagerSupplier > xUICfgMgrSupp(
        css::ui::theModuleUIConfigurationManagerSupplier::get(mxContext) );

    m_xAppCfgMgr.set( xUICfgMgrSupp->getUIConfigurationManager( maModuleId ), uno::UNO_QUERY_THROW );

    css::uno::Reference< css::container::XNameAccess > xNameAccess = css::ui::theWindowStateConfiguration::get( mxContext );

    m_xWindowState.set( xNameAccess->getByName( maModuleId ), uno::UNO_QUERY_THROW );
}

css::uno::Reference< css::container::XIndexAccess > VbaCommandBarHelper::getSettings( const OUString& sResourceUrl )
{
    if( m_xDocCfgMgr->hasSettings( sResourceUrl ) )
        return m_xDocCfgMgr->getSettings( sResourceUrl, true );
    else if( m_xAppCfgMgr->hasSettings( sResourceUrl ) )
        return m_xAppCfgMgr->getSettings( sResourceUrl, true );
    else
    {
        css::uno::Reference< css::container::XIndexAccess > xSettings( m_xAppCfgMgr->createSettings( ), uno::UNO_QUERY_THROW );
        return xSettings;
    }
}

void VbaCommandBarHelper::removeSettings( const OUString& sResourceUrl )
{
    if( m_xDocCfgMgr->hasSettings( sResourceUrl ) )
        m_xDocCfgMgr->removeSettings( sResourceUrl );
    else if( m_xAppCfgMgr->hasSettings( sResourceUrl ) )
        m_xAppCfgMgr->removeSettings( sResourceUrl );

    // persistChanges();
}

void VbaCommandBarHelper::ApplyTempChange( const OUString& sResourceUrl, const css::uno::Reference< css::container::XIndexAccess >& xSettings )
{
    if( m_xDocCfgMgr->hasSettings( sResourceUrl ) )
    {
        m_xDocCfgMgr->replaceSettings( sResourceUrl, xSettings );
    }
    else
    {
        m_xDocCfgMgr->insertSettings( sResourceUrl, xSettings );
    }
}

uno::Reference< frame::XLayoutManager > VbaCommandBarHelper::getLayoutManager()
{
    uno::Reference< frame::XFrame > xFrame( getModel()->getCurrentController()->getFrame(), uno::UNO_QUERY_THROW );
    uno::Reference< beans::XPropertySet > xPropertySet( xFrame, uno::UNO_QUERY_THROW );
    uno::Reference< frame::XLayoutManager > xLayoutManager( xPropertySet->getPropertyValue( "LayoutManager" ), uno::UNO_QUERY_THROW );
    return xLayoutManager;
}

bool VbaCommandBarHelper::hasToolbar( const OUString& sResourceUrl, const OUString& sName )
{
    if( m_xDocCfgMgr->hasSettings( sResourceUrl ) )
    {
        OUString sUIName;
        uno::Reference< beans::XPropertySet > xPropertySet( m_xDocCfgMgr->getSettings( sResourceUrl, false ), uno::UNO_QUERY_THROW );
        xPropertySet->getPropertyValue( ITEM_DESCRIPTOR_UINAME ) >>= sUIName;
        if( sName.equalsIgnoreAsciiCase( sUIName ) )
            return true;
    }
    return false;
}

// return the resource url if found
OUString VbaCommandBarHelper::findToolbarByName( const css::uno::Reference< css::container::XNameAccess >& xNameAccess, const OUString& sName )
{
    OUString sResourceUrl;

    // check if it is an buildin toolbar
    sResourceUrl = MSO2OOCommandbarHelper::getMSO2OOCommandbarHelper()->findBuildinToolbar( sName );
    if( !sResourceUrl.isEmpty() )
        return sResourceUrl;

    uno::Sequence< OUString > allNames = xNameAccess->getElementNames();
    for( sal_Int32 i = 0; i < allNames.getLength(); i++ )
    {
        sResourceUrl = allNames[i];
        if(sResourceUrl.startsWith( ITEM_TOOLBAR_URL ) )
        {
            if( hasToolbar( sResourceUrl, sName ) )
                return sResourceUrl;
        }
    }

    // the customize toolbars creating during importing, should found there.
    sResourceUrl = "private:resource/toolbar/custom_" + sName;
    if( hasToolbar( sResourceUrl, sName ) )
        return sResourceUrl;

    return OUString();
}

// if found, return the position of the control. if not found, return -1
sal_Int32 VbaCommandBarHelper::findControlByName( const css::uno::Reference< css::container::XIndexAccess >& xIndexAccess, const OUString& sName, bool bMenu )
{
    sal_Int32 nCount = xIndexAccess->getCount();
    css::uno::Sequence< css::beans::PropertyValue > aProps;
    for( sal_Int32 i = 0; i < nCount; i++ )
    {
        OUString sLabel;
        xIndexAccess->getByIndex( i ) >>= aProps;
        getPropertyValue( aProps, ITEM_DESCRIPTOR_LABEL ) >>= sLabel;
        // handle the hotkey marker '~' (remove in toolbars (?), replace by '&' in menus)
        OUStringBuffer aBuffer;
        sal_Int32 index = sLabel.indexOf( '~' );
        if( index < 0 )
        {
            aBuffer = sLabel;
        }
        else
        {
            aBuffer.append( std::u16string_view(sLabel).substr(0, index) );
            if( bMenu )
                aBuffer.append( '&' );
            aBuffer.append( std::u16string_view(sLabel).substr(index + 1) );
        }
        OUString sNewLabel = aBuffer.makeStringAndClear();
        SAL_INFO("vbahelper", "VbaCommandBarHelper::findControlByName, control name: " << sNewLabel);
        if( sName.equalsIgnoreAsciiCase( sNewLabel ) )
            return i;
    }

    // not found
    return -1;
}

OUString VbaCommandBarHelper::generateCustomURL()
{
    OUString url( ITEM_TOOLBAR_URL );
    url += CUSTOM_TOOLBAR_STR;

    // use a random number to minimize possible clash with existing custom toolbars
    url += OUString::number(comphelper::rng::uniform_int_distribution(0, std::numeric_limits<int>::max()), 16);
    return url;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
