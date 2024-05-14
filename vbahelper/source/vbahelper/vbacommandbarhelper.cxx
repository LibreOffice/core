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
#include <com/sun/star/ui/theWindowStateConfiguration.hpp>
#include <comphelper/random.hxx>
#include <utility>
#include <vbahelper/vbahelper.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <o3tl/string_view.hxx>
#include <map>
#include <string_view>

using namespace com::sun::star;
using namespace ooo::vba;

typedef std::map< OUString, OUString > MSO2OOCommandbarMap;

namespace {

class MSO2OOCommandbarHelper final
{
private:
    static MSO2OOCommandbarHelper* pMSO2OOCommandbarHelper;
    MSO2OOCommandbarMap maBuildinToolbarMap;

    MSO2OOCommandbarHelper()
    {
        // Buildin toolbars
        maBuildinToolbarMap.insert( std::make_pair( u"Standard"_ustr ,    u"private:resource/toolbar/standardbar"_ustr ) );
        maBuildinToolbarMap.insert( std::make_pair( u"Formatting"_ustr,   u"private:resource/toolbar/formatobjectbar"_ustr ) );
        maBuildinToolbarMap.insert( std::make_pair( u"Drawing"_ustr,      u"private:resource/toolbar/drawbar"_ustr ) );
        maBuildinToolbarMap.insert( std::make_pair( u"Toolbar List"_ustr, u"private:resource/toolbar/toolbar"_ustr ) );
        maBuildinToolbarMap.insert( std::make_pair( u"Forms"_ustr,        u"private:resource/toolbar/formcontrols"_ustr ) );
        maBuildinToolbarMap.insert( std::make_pair( u"Form Controls"_ustr,u"private:resource/toolbar/formcontrols"_ustr ) );
        maBuildinToolbarMap.insert( std::make_pair( u"Full Screen"_ustr,  u"private:resource/toolbar/fullscreenbar"_ustr ) );
        maBuildinToolbarMap.insert( std::make_pair( u"Chart"_ustr,        u"private:resource/toolbar/flowchartshapes"_ustr ) );
        maBuildinToolbarMap.insert( std::make_pair( u"Picture"_ustr,      u"private:resource/toolbar/graphicobjectbar"_ustr ) );
        maBuildinToolbarMap.insert( std::make_pair( u"WordArt"_ustr,      u"private:resource/toolbar/fontworkobjectbar"_ustr ) );
        maBuildinToolbarMap.insert( std::make_pair( u"3-D Settings"_ustr, u"private:resource/toolbar/extrusionobjectbar"_ustr ) );
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

}

MSO2OOCommandbarHelper* MSO2OOCommandbarHelper::pMSO2OOCommandbarHelper = nullptr;


VbaCommandBarHelper::VbaCommandBarHelper( css::uno::Reference< css::uno::XComponentContext > xContext, css::uno::Reference< css::frame::XModel >  xModel ) : mxContext(std::move( xContext )), mxModel(std::move( xModel ))
{
    Init();
}

void VbaCommandBarHelper::Init( )
{
    uno::Reference< css::ui::XUIConfigurationManagerSupplier > xUICfgSupplier( mxModel, uno::UNO_QUERY_THROW );
    m_xDocCfgMgr = xUICfgSupplier->getUIConfigurationManager();

    uno::Reference< lang::XServiceInfo > xServiceInfo( mxModel, uno::UNO_QUERY_THROW );
    if( xServiceInfo->supportsService( u"com.sun.star.sheet.SpreadsheetDocument"_ustr ) )
    {
        maModuleId = "com.sun.star.sheet.SpreadsheetDocument";
    }
    else if( xServiceInfo->supportsService( u"com.sun.star.text.TextDocument"_ustr ) )
    {
        maModuleId = "com.sun.star.text.TextDocument";
    }

    if( maModuleId.isEmpty() )
    {
        throw uno::RuntimeException( u"Not implemented"_ustr );
    }

    css::uno::Reference< css::ui::XModuleUIConfigurationManagerSupplier > xUICfgMgrSupp(
        css::ui::theModuleUIConfigurationManagerSupplier::get(mxContext) );

    m_xAppCfgMgr.set( xUICfgMgrSupp->getUIConfigurationManager( maModuleId ), uno::UNO_SET_THROW );

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

uno::Reference< frame::XLayoutManager > VbaCommandBarHelper::getLayoutManager() const
{
    uno::Reference< frame::XFrame > xFrame( getModel()->getCurrentController()->getFrame(), uno::UNO_SET_THROW );
    uno::Reference< beans::XPropertySet > xPropertySet( xFrame, uno::UNO_QUERY_THROW );
    uno::Reference< frame::XLayoutManager > xLayoutManager( xPropertySet->getPropertyValue( u"LayoutManager"_ustr ), uno::UNO_QUERY_THROW );
    return xLayoutManager;
}

bool VbaCommandBarHelper::hasToolbar( const OUString& sResourceUrl, std::u16string_view sName )
{
    if( m_xDocCfgMgr->hasSettings( sResourceUrl ) )
    {
        OUString sUIName;
        uno::Reference< beans::XPropertySet > xPropertySet( m_xDocCfgMgr->getSettings( sResourceUrl, false ), uno::UNO_QUERY_THROW );
        xPropertySet->getPropertyValue( ITEM_DESCRIPTOR_UINAME ) >>= sUIName;
        if( o3tl::equalsIgnoreAsciiCase( sName, sUIName ) )
            return true;
    }
    return false;
}

// return the resource url if found
OUString VbaCommandBarHelper::findToolbarByName( const css::uno::Reference< css::container::XNameAccess >& xNameAccess, const OUString& sName )
{
    // check if it is a buildin toolbar
    OUString sResourceUrl = MSO2OOCommandbarHelper::getMSO2OOCommandbarHelper()->findBuildinToolbar( sName );
    if( !sResourceUrl.isEmpty() )
        return sResourceUrl;

    const uno::Sequence< OUString > allNames = xNameAccess->getElementNames();
    auto pName = std::find_if(allNames.begin(), allNames.end(),
        [this, &sName](const OUString& rName) {
            return rName.startsWith( ITEM_TOOLBAR_URL )
                && hasToolbar( rName, sName );
        });
    if (pName != allNames.end())
        return *pName;

    // the customize toolbars creating during importing, should found there.
    sResourceUrl = "private:resource/toolbar/custom_" + sName;
    if( hasToolbar( sResourceUrl, sName ) )
        return sResourceUrl;

    return OUString();
}

// if found, return the position of the control. if not found, return -1
sal_Int32 VbaCommandBarHelper::findControlByName( const css::uno::Reference< css::container::XIndexAccess >& xIndexAccess, std::u16string_view sName, bool bMenu )
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
            aBuffer.append( sLabel.subView(0, index) );
            if( bMenu )
                aBuffer.append( '&' );
            aBuffer.append( sLabel.subView(index + 1) );
        }
        OUString sNewLabel = aBuffer.makeStringAndClear();
        SAL_INFO("vbahelper", "VbaCommandBarHelper::findControlByName, control name: " << sNewLabel);
        if( o3tl::equalsIgnoreAsciiCase( sName, sNewLabel ) )
            return i;
    }

    // not found
    return -1;
}

OUString VbaCommandBarHelper::generateCustomURL()
{
    return OUString::Concat(ITEM_TOOLBAR_URL) + CUSTOM_TOOLBAR_STR +
        // use a random number to minimize possible clash with existing custom toolbars
        OUString::number(comphelper::rng::uniform_int_distribution(0, std::numeric_limits<int>::max()), 16);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
