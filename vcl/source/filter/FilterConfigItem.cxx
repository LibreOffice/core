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
#include <comphelper/propertyvalue.hxx>
#include <o3tl/string_view.hxx>
#include <osl/diagnose.h>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>

using namespace ::com::sun::star::lang      ;   // XMultiServiceFactory
using namespace ::com::sun::star::beans     ;   // PropertyValue
using namespace ::com::sun::star::uno       ;   // Reference
using namespace ::com::sun::star::util      ;   // XChangesBatch
using namespace ::com::sun::star::container ;
using namespace ::com::sun::star::configuration;
using namespace ::com::sun::star::task      ;   // XStatusIndicator

static bool ImpIsTreeAvailable( Reference< XMultiServiceFactory > const & rXCfgProv, std::u16string_view rTree )
{
    bool bAvailable = !rTree.empty();
    if ( bAvailable )
    {
        sal_Int32 nIdx{0};
        if ( rTree[0] == '/' )
            ++nIdx;

        // creation arguments: nodepath
        PropertyValue aPathArgument = comphelper::makePropertyValue("nodepath",
                                                                    OUString(o3tl::getToken(rTree, 0, '/', nIdx)));
        Sequence< Any > aArguments{ Any(aPathArgument) };

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
            const sal_Int32 nEnd  = rTree.size();
            while (bAvailable && nIdx>=0 && nIdx<nEnd)
            {
                Reference< XHierarchicalNameAccess > xHierarchicalNameAccess
                    ( xReadAccess, UNO_QUERY );

                if ( !xHierarchicalNameAccess.is() )
                    bAvailable = false;
                else
                {
                    const OUString aNode( o3tl::getToken(rTree, 0, '/', nIdx) );
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

void FilterConfigItem::ImpInitTree( std::u16string_view rSubTree )
{
    bModified = false;

    Reference< XComponentContext > xContext( comphelper::getProcessComponentContext() );

    Reference< XMultiServiceFactory > xCfgProv = theDefaultProvider::get( xContext );

    OUString sTree = OUString::Concat("/org.openoffice.") + rSubTree;
    if ( !ImpIsTreeAvailable(xCfgProv, sTree) )
        return;

    // creation arguments: nodepath
    PropertyValue aPathArgument;
    aPathArgument.Name = "nodepath";
    aPathArgument.Value <<= sTree;

    Sequence< Any > aArguments{ Any(aPathArgument) };

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

FilterConfigItem::FilterConfigItem( std::u16string_view rSubTree )
{
    ImpInitTree( rSubTree );
}

FilterConfigItem::FilterConfigItem( css::uno::Sequence< css::beans::PropertyValue > const * pFilterData )
    : bModified(false)
{
    if ( pFilterData )
        aFilterData = *pFilterData;
}

FilterConfigItem::FilterConfigItem( std::u16string_view rSubTree,
    css::uno::Sequence< css::beans::PropertyValue > const * pFilterData )
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
    if ( !xUpdatableView.is() )
        return;

    if ( !(xPropSet.is() && bModified) )
        return;

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

bool FilterConfigItem::ImplGetPropertyValue( Any& rAny, const Reference< XPropertySet >& rXPropSet, const OUString& rString )
{
    bool bRetValue = true;

    if ( rXPropSet.is() )
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
const PropertyValue* FilterConfigItem::GetPropertyValue( const Sequence< PropertyValue >& rPropSeq, const OUString& rName )
{
    auto pProp = std::find_if(rPropSeq.begin(), rPropSeq.end(),
        [&rName](const PropertyValue& rProp) { return rProp.Name == rName; });
    if (pProp != rPropSeq.end())
        return pProp;
    return nullptr;
}

/* if PropertySequence already includes a PropertyValue using the same name, the
    corresponding PropertyValue is replaced, otherwise the given PropertyValue
    will be appended */

bool FilterConfigItem::WritePropertyValue( Sequence< PropertyValue >& rPropSeq, const PropertyValue& rPropValue )
{
    bool bRet = false;
    if ( !rPropValue.Name.isEmpty() )
    {
        auto pProp = std::find_if(std::cbegin(rPropSeq), std::cend(rPropSeq),
            [&rPropValue](const PropertyValue& rProp) { return rProp.Name == rPropValue.Name; });
        sal_Int32 i = std::distance(std::cbegin(rPropSeq), pProp);
        sal_Int32 nCount = rPropSeq.getLength();
        if ( i == nCount )
            rPropSeq.realloc( ++nCount );

        rPropSeq.getArray()[ i ] = rPropValue;

        bRet = true;
    }
    return bRet;
}

bool FilterConfigItem::IsReadOnly(const OUString& rName)
{
    if (!xPropSet.is())
        return false;

    const Reference<XPropertySetInfo> xInfo(xPropSet->getPropertySetInfo());
    if (!xInfo.is() || !xInfo->hasPropertyByName(rName))
        return false;

    const css::beans::Property aProp(xInfo->getPropertyByName(rName));
    return (aProp.Attributes & PropertyAttribute::READONLY);
}

bool FilterConfigItem::ReadBool( const OUString& rKey, bool bDefault )
{
    Any aAny;
    bool bRetValue = bDefault;
    const PropertyValue* pPropVal = GetPropertyValue( aFilterData, rKey );
    if ( pPropVal )
    {
        pPropVal->Value >>= bRetValue;
    }
    else if ( ImplGetPropertyValue( aAny, xPropSet, rKey ) )
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
    const PropertyValue* pPropVal = GetPropertyValue( aFilterData, rKey );
    if ( pPropVal )
    {
        pPropVal->Value >>= nRetValue;
    }
    else if ( ImplGetPropertyValue( aAny, xPropSet, rKey ) )
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
    const PropertyValue* pPropVal = GetPropertyValue( aFilterData, rKey );
    if ( pPropVal )
    {
        pPropVal->Value >>= aRetValue;
    }
    else if ( ImplGetPropertyValue( aAny, xPropSet, rKey ) )
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

    if ( !xPropSet.is() )
        return;

    Any aAny;
    if ( !ImplGetPropertyValue( aAny, xPropSet, rKey ) )
        return;

    bool bOldValue(true);
    if ( !(aAny >>= bOldValue) )
        return;

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

void FilterConfigItem::WriteInt32( const OUString& rKey, sal_Int32 nNewValue )
{
    PropertyValue aInt32;
    aInt32.Name = rKey;
    aInt32.Value <<= nNewValue;
    WritePropertyValue( aFilterData, aInt32 );

    if ( !xPropSet.is() )
        return;

    Any aAny;

    if ( !ImplGetPropertyValue( aAny, xPropSet, rKey ) )
        return;

    sal_Int32 nOldValue = 0;
    if ( !(aAny >>= nOldValue) )
        return;

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


Reference< XStatusIndicator > FilterConfigItem::GetStatusIndicator() const
{
    Reference< XStatusIndicator > xStatusIndicator;

    auto pPropVal = std::find_if(aFilterData.begin(), aFilterData.end(),
        [](const css::beans::PropertyValue& rPropVal) {
            return rPropVal.Name == "StatusIndicator"; });
    if (pPropVal != aFilterData.end())
    {
        pPropVal->Value >>= xStatusIndicator;
    }
    return xStatusIndicator;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
