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

#include <vcl/FilterConfigItem.hxx>

#include <unotools/configmgr.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <osl/diagnose.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>

using namespace ::com::sun::star::lang      ;   // XMultiServiceFactory
using namespace ::com::sun::star::beans     ;   // PropertyValue
using namespace ::com::sun::star::uno       ;   // Reference
using namespace ::com::sun::star::util      ;   // XChangesBatch
using namespace ::com::sun::star::awt       ;   // Size
using namespace ::com::sun::star::container ;
using namespace ::com::sun::star::configuration;
using namespace ::com::sun::star::task      ;   // XStatusIndicator

static bool ImpIsTreeAvailable( Reference< XMultiServiceFactory >& rXCfgProv, const OUString& rTree )
{
    bool bAvailable = !rTree.isEmpty();
    if ( bAvailable )
    {
        using comphelper::string::getTokenCount;

        sal_Int32 nTokenCount = getTokenCount(rTree, '/');
        sal_Int32 i = 0;

        if ( rTree[0] == '/' )
            ++i;
        if ( rTree.endsWith("/") )
            --nTokenCount;

        // creation arguments: nodepath
        PropertyValue aPathArgument;
        aPathArgument.Name = "nodepath";
        aPathArgument.Value = Any(rTree.getToken(i++, '/'));

        Sequence< Any > aArguments( 1 );
        aArguments[ 0 ] <<= aPathArgument;

        Reference< XInterface > xReadAccess;
        try
        {
            xReadAccess = rXCfgProv->createInstanceWithArguments(
                    "com.sun.star.configuration.ConfigurationAccess",
                    aArguments );
        }
        catch (const css::uno::Exception&)
        {
            bAvailable = false;
        }
        if ( xReadAccess.is() )
        {
            for ( ; bAvailable && ( i < nTokenCount ); i++ )
            {
                Reference< XHierarchicalNameAccess > xHierarchicalNameAccess
                    ( xReadAccess, UNO_QUERY );

                if ( !xHierarchicalNameAccess.is() )
                    bAvailable = false;
                else
                {
                    OUString aNode( rTree.getToken(i, '/') );
                    if ( !xHierarchicalNameAccess->hasByHierarchicalName( aNode ) )
                        bAvailable = false;
                    else
                    {
                        Any a( xHierarchicalNameAccess->getByHierarchicalName( aNode ) );
                        bAvailable = (a >>= xReadAccess);
                    }
                }
            }
        }
    }
    return bAvailable;
}

void FilterConfigItem::ImpInitTree( const OUString& rSubTree )
{
    bModified = false;

    Reference< XComponentContext > xContext( comphelper::getProcessComponentContext() );

    Reference< XMultiServiceFactory > xCfgProv = theDefaultProvider::get( xContext );

    OUString sTree = "/org.openoffice." + rSubTree;
    if ( ImpIsTreeAvailable(xCfgProv, sTree) )
    {
        // creation arguments: nodepath
        PropertyValue aPathArgument;
        aPathArgument.Name = "nodepath";
        aPathArgument.Value = Any(sTree);

        // creation arguments: commit mode
        PropertyValue aModeArgument;
        bool bAsynchron = true;
        aModeArgument.Name = "lazywrite";
        aModeArgument.Value = Any(bAsynchron);

        Sequence< Any > aArguments( 2 );
        aArguments[ 0 ] <<= aPathArgument;
        aArguments[ 1 ] <<= aModeArgument;

        try
        {
            xUpdatableView = xCfgProv->createInstanceWithArguments(
                    "com.sun.star.configuration.ConfigurationUpdateAccess",
                    aArguments );
            if ( xUpdatableView.is() )
                xPropSet.set( xUpdatableView, UNO_QUERY );
        }
        catch ( css::uno::Exception& )
        {
            OSL_FAIL( "FilterConfigItem::FilterConfigItem - Could not access configuration Key" );
        }
    }
}

FilterConfigItem::FilterConfigItem( const OUString& rSubTree )
{
    ImpInitTree( rSubTree );
}

FilterConfigItem::FilterConfigItem( css::uno::Sequence< css::beans::PropertyValue >* pFilterData )
    : bModified(false)
{
    if ( pFilterData )
        aFilterData = *pFilterData;
}

FilterConfigItem::FilterConfigItem( const OUString& rSubTree,
    css::uno::Sequence< css::beans::PropertyValue >* pFilterData )
{
    ImpInitTree( rSubTree );

    if ( pFilterData )
        aFilterData = *pFilterData;
};

FilterConfigItem::~FilterConfigItem()
{
    WriteModifiedConfig();
}

void FilterConfigItem::WriteModifiedConfig()
{
    if ( xUpdatableView.is() )
    {
        if ( xPropSet.is() && bModified )
        {
            Reference< XChangesBatch > xUpdateControl( xUpdatableView, UNO_QUERY );
            if ( xUpdateControl.is() )
            {
                try
                {
                    xUpdateControl->commitChanges();
                    bModified = false;
                }
                catch ( css::uno::Exception& )
                {
                    OSL_FAIL( "FilterConfigItem::FilterConfigItem - Could not update configuration data" );
                }
            }
        }
    }
}

bool FilterConfigItem::ImplGetPropertyValue( Any& rAny, const Reference< XPropertySet >& rXPropSet, const OUString& rString, bool bTestPropertyAvailability )
{
    bool bRetValue = true;

    if ( rXPropSet.is() )
    {
        if ( bTestPropertyAvailability )
        {
            bRetValue = false;
            try
            {
                Reference< XPropertySetInfo >
                    aXPropSetInfo( rXPropSet->getPropertySetInfo() );
                if ( aXPropSetInfo.is() )
                    bRetValue = aXPropSetInfo->hasPropertyByName( rString );
            }
            catch( css::uno::Exception& )
            {

            }
        }
        if ( bRetValue )
        {
            try
            {
                rAny = rXPropSet->getPropertyValue( rString );
                if ( !rAny.hasValue() )
                    bRetValue = false;
            }
            catch( css::uno::Exception& )
            {
                bRetValue = false;
            }
        }
    }
    else
        bRetValue = false;
    return bRetValue;
}

// if property is available it returns a pointer,
// otherwise the result is null
PropertyValue* FilterConfigItem::GetPropertyValue( Sequence< PropertyValue >& rPropSeq, const OUString& rName )
{
    PropertyValue* pPropValue = nullptr;

    sal_Int32 i, nCount;
    for ( i = 0, nCount = rPropSeq.getLength(); i < nCount; i++ )
    {
        if ( rPropSeq[ i ].Name == rName )
        {
            pPropValue = &rPropSeq[ i ];
            break;
        }
    }
    return pPropValue;
}

/* if PropertySequence already includes a PropertyValue using the same name, the
    corresponding PropertyValue is replaced, otherwise the given PropertyValue
    will be appended */

bool FilterConfigItem::WritePropertyValue( Sequence< PropertyValue >& rPropSeq, const PropertyValue& rPropValue )
{
    bool bRet = false;
    if ( !rPropValue.Name.isEmpty() )
    {
        sal_Int32 i, nCount;
        for ( i = 0, nCount = rPropSeq.getLength(); i < nCount; i++ )
        {
            if ( rPropSeq[ i ].Name == rPropValue.Name )
                break;
        }
        if ( i == nCount )
            rPropSeq.realloc( ++nCount );

        rPropSeq[ i ] = rPropValue;

        bRet = true;
    }
    return bRet;
}

bool FilterConfigItem::ReadBool( const OUString& rKey, bool bDefault )
{
    Any aAny;
    bool bRetValue = bDefault;
    PropertyValue* pPropVal = GetPropertyValue( aFilterData, rKey );
    if ( pPropVal )
    {
        pPropVal->Value >>= bRetValue;
    }
    else if ( ImplGetPropertyValue( aAny, xPropSet, rKey, true ) )
    {
        aAny >>= bRetValue;
    }
    PropertyValue aBool;
    aBool.Name = rKey;
    aBool.Value <<= bRetValue;
    WritePropertyValue( aFilterData, aBool );
    return bRetValue;
}

sal_Int32 FilterConfigItem::ReadInt32( const OUString& rKey, sal_Int32 nDefault )
{
    Any aAny;
    sal_Int32 nRetValue = nDefault;
    PropertyValue* pPropVal = GetPropertyValue( aFilterData, rKey );
    if ( pPropVal )
    {
        pPropVal->Value >>= nRetValue;
    }
    else if ( ImplGetPropertyValue( aAny, xPropSet, rKey, true ) )
    {
        aAny >>= nRetValue;
    }
    PropertyValue aInt32;
    aInt32.Name = rKey;
    aInt32.Value <<= nRetValue;
    WritePropertyValue( aFilterData, aInt32 );
    return nRetValue;
}

OUString FilterConfigItem::ReadString( const OUString& rKey, const OUString& rDefault )
{
    Any aAny;
    OUString aRetValue( rDefault );
    PropertyValue* pPropVal = GetPropertyValue( aFilterData, rKey );
    if ( pPropVal )
    {
        pPropVal->Value >>= aRetValue;
    }
    else if ( ImplGetPropertyValue( aAny, xPropSet, rKey, true ) )
    {
        aAny >>= aRetValue;
    }
    PropertyValue aString;
    aString.Name = rKey;
    aString.Value <<= aRetValue;
    WritePropertyValue( aFilterData, aString );
    return aRetValue;
}

void FilterConfigItem::WriteBool( const OUString& rKey, bool bNewValue )
{
    PropertyValue aBool;
    aBool.Name = rKey;
    aBool.Value <<= bNewValue;
    WritePropertyValue( aFilterData, aBool );

    if ( xPropSet.is() )
    {
        Any aAny;
        if ( ImplGetPropertyValue( aAny, xPropSet, rKey, true ) )
        {
            bool bOldValue(true);
            if ( aAny >>= bOldValue )
            {
                if ( bOldValue != bNewValue )
                {
                    try
                    {
                        xPropSet->setPropertyValue( rKey, Any(bNewValue) );
                        bModified = true;
                    }
                    catch ( css::uno::Exception& )
                    {
                        OSL_FAIL( "FilterConfigItem::WriteBool - could not set PropertyValue" );
                    }
                }
            }
        }
    }
}

void FilterConfigItem::WriteInt32( const OUString& rKey, sal_Int32 nNewValue )
{
    PropertyValue aInt32;
    aInt32.Name = rKey;
    aInt32.Value <<= nNewValue;
    WritePropertyValue( aFilterData, aInt32 );

    if ( xPropSet.is() )
    {
        Any aAny;

        if ( ImplGetPropertyValue( aAny, xPropSet, rKey, true ) )
        {
            sal_Int32 nOldValue = 0;
            if ( aAny >>= nOldValue )
            {
                if ( nOldValue != nNewValue )
                {
                    try
                    {
                        xPropSet->setPropertyValue( rKey, Any(nNewValue) );
                        bModified = true;
                    }
                    catch ( css::uno::Exception& )
                    {
                        OSL_FAIL( "FilterConfigItem::WriteInt32 - could not set PropertyValue" );
                    }
                }
            }
        }
    }
}


Reference< XStatusIndicator > FilterConfigItem::GetStatusIndicator() const
{
    Reference< XStatusIndicator > xStatusIndicator;
    const OUString sStatusIndicator( "StatusIndicator" );

    sal_Int32 i, nCount = aFilterData.getLength();
    for ( i = 0; i < nCount; i++ )
    {
        if ( aFilterData[ i ].Name == sStatusIndicator )
        {
            aFilterData[ i ].Value >>= xStatusIndicator;
            break;
        }
    }
    return xStatusIndicator;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
