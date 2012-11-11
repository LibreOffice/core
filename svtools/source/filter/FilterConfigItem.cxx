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

#include <svtools/FilterConfigItem.hxx>

#include <unotools/configmgr.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>

using ::rtl::OUString;
using namespace ::utl                       ;   // getProcessServiceFactory
using namespace ::com::sun::star::lang      ;   // XMultiServiceFactory
using namespace ::com::sun::star::beans     ;   // PropertyValue
using namespace ::com::sun::star::uno       ;   // Reference
using namespace ::com::sun::star::util      ;   // XChangesBatch
using namespace ::com::sun::star::awt       ;   // Size
using namespace ::com::sun::star::container ;   //
using namespace ::com::sun::star::configuration;   //
using namespace ::com::sun::star::task      ;   // XStatusIndicator

static sal_Bool ImpIsTreeAvailable( Reference< XMultiServiceFactory >& rXCfgProv, const rtl::OUString& rTree )
{
    sal_Bool bAvailable = !rTree.isEmpty();
    if ( bAvailable )
    {
        using comphelper::string::getTokenCount;
        using comphelper::string::getToken;

        sal_Int32 nTokenCount = getTokenCount(rTree, '/');
        sal_Int32 i = 0;

        if ( rTree[0] == '/' )
            ++i;
        if ( rTree[rTree.getLength() - 1] == '/' )
            --nTokenCount;

        Any aAny;
        aAny <<= getToken(rTree, i++, '/');

        // creation arguments: nodepath
        PropertyValue aPathArgument;
        aPathArgument.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "nodepath" ) );
        aPathArgument.Value = aAny;

        Sequence< Any > aArguments( 1 );
        aArguments[ 0 ] <<= aPathArgument;

        Reference< XInterface > xReadAccess;
        try
        {
            xReadAccess = rXCfgProv->createInstanceWithArguments(
                OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.configuration.ConfigurationAccess" ) ),
                    aArguments );
        }
        catch (const ::com::sun::star::uno::Exception&)
        {
            bAvailable = sal_False;
        }
        if ( xReadAccess.is() )
        {
            for ( ; bAvailable && ( i < nTokenCount ); i++ )
            {
                Reference< XHierarchicalNameAccess > xHierarchicalNameAccess
                    ( xReadAccess, UNO_QUERY );

                if ( !xHierarchicalNameAccess.is() )
                    bAvailable = sal_False;
                else
                {
                    rtl::OUString aNode( getToken(rTree, i, '/') );
                    if ( !xHierarchicalNameAccess->hasByHierarchicalName( aNode ) )
                        bAvailable = sal_False;
                    else
                    {
                        Any a( xHierarchicalNameAccess->getByHierarchicalName( aNode ) );
                        try
                        {
                            a >>= xReadAccess;
                        }
                        catch ( ::com::sun::star::uno::Exception& )
                        {
                            bAvailable = sal_False;
                        }
                    }
                }
            }
        }
    }
    return bAvailable;
}

void FilterConfigItem::ImpInitTree( const String& rSubTree )
{
    bModified = sal_False;

    Reference< XComponentContext > xContext( comphelper::getProcessComponentContext() );

    Reference< XMultiServiceFactory > xCfgProv = theDefaultProvider::get( xContext );

    OUString sTree(
        OUString(RTL_CONSTASCII_USTRINGPARAM("/org.openoffice.")) +
        rSubTree);
    if ( ImpIsTreeAvailable(xCfgProv, sTree) )
    {
        Any aAny;
        // creation arguments: nodepath
        PropertyValue aPathArgument;
        aAny <<= sTree;
        aPathArgument.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "nodepath" ) );
        aPathArgument.Value = aAny;

        // creation arguments: commit mode
        PropertyValue aModeArgument;
        sal_Bool bAsyncron = sal_True;
        aAny <<= bAsyncron;
        aModeArgument.Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "lazywrite" ) );
        aModeArgument.Value = aAny;

        Sequence< Any > aArguments( 2 );
        aArguments[ 0 ] <<= aPathArgument;
        aArguments[ 1 ] <<= aModeArgument;

        try
        {
            xUpdatableView = xCfgProv->createInstanceWithArguments(
                OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.configuration.ConfigurationUpdateAccess" ) ),
                    aArguments );
            if ( xUpdatableView.is() )
                xPropSet = Reference< XPropertySet >( xUpdatableView, UNO_QUERY );
        }
        catch ( ::com::sun::star::uno::Exception& )
        {
            OSL_FAIL( "FilterConfigItem::FilterConfigItem - Could not access configuration Key" );
        }
    }
}

FilterConfigItem::FilterConfigItem( const OUString& rSubTree )
{
    ImpInitTree( rSubTree );
}

FilterConfigItem::FilterConfigItem( ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >* pFilterData )
{
    if ( pFilterData )
        aFilterData = *pFilterData;
}

FilterConfigItem::FilterConfigItem( const OUString& rSubTree,
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >* pFilterData )
{
    ImpInitTree( rSubTree );

    if ( pFilterData )
        aFilterData = *pFilterData;
};

FilterConfigItem::~FilterConfigItem()
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
                }
                catch ( ::com::sun::star::uno::Exception& )
                {
                    OSL_FAIL( "FilterConfigItem::FilterConfigItem - Could not update configuration data" );
                }
            }
        }
    }
}

sal_Bool FilterConfigItem::ImplGetPropertyValue( Any& rAny, const Reference< XPropertySet >& rXPropSet, const OUString& rString, sal_Bool bTestPropertyAvailability )
{
    sal_Bool bRetValue = sal_True;

    if ( rXPropSet.is() )
    {
        if ( bTestPropertyAvailability )
        {
            bRetValue = sal_False;
            try
            {
                Reference< XPropertySetInfo >
                    aXPropSetInfo( rXPropSet->getPropertySetInfo() );
                if ( aXPropSetInfo.is() )
                    bRetValue = aXPropSetInfo->hasPropertyByName( rString );
            }
            catch( ::com::sun::star::uno::Exception& )
            {
                //
            }
        }
        if ( bRetValue )
        {
            try
            {
                rAny = rXPropSet->getPropertyValue( rString );
                if ( !rAny.hasValue() )
                    bRetValue = sal_False;
            }
            catch( ::com::sun::star::uno::Exception& )
            {
                bRetValue = sal_False;
            }
        }
    }
    else
        bRetValue = sal_False;
    return bRetValue;
}


// if property is available it returns a pointer,
// otherwise the result is null
PropertyValue* FilterConfigItem::GetPropertyValue( Sequence< PropertyValue >& rPropSeq, const OUString& rName )
{
    PropertyValue* pPropValue = NULL;

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

sal_Bool FilterConfigItem::WritePropertyValue( Sequence< PropertyValue >& rPropSeq, const PropertyValue& rPropValue )
{
    sal_Bool bRet = sal_False;
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

        bRet = sal_True;
    }
    return bRet;
}

sal_Bool FilterConfigItem::ReadBool( const OUString& rKey, sal_Bool bDefault )
{
    Any aAny;
    sal_Bool bRetValue = bDefault;
    PropertyValue* pPropVal = GetPropertyValue( aFilterData, rKey );
    if ( pPropVal )
    {
        pPropVal->Value >>= bRetValue;
    }
    else if ( ImplGetPropertyValue( aAny, xPropSet, rKey, sal_True ) )
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
    else if ( ImplGetPropertyValue( aAny, xPropSet, rKey, sal_True ) )
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
    else if ( ImplGetPropertyValue( aAny, xPropSet, rKey, sal_True ) )
    {
        aAny >>= aRetValue;
    }
    PropertyValue aString;
    aString.Name = rKey;
    aString.Value <<= aRetValue;
    WritePropertyValue( aFilterData, aString );
    return aRetValue;
}

void FilterConfigItem::WriteBool( const OUString& rKey, sal_Bool bNewValue )
{
    PropertyValue aBool;
    aBool.Name = rKey;
    aBool.Value <<= bNewValue;
    WritePropertyValue( aFilterData, aBool );

    if ( xPropSet.is() )
    {
        Any aAny;
        if ( ImplGetPropertyValue( aAny, xPropSet, rKey, sal_True ) )
        {
            sal_Bool bOldValue(sal_True);
            if ( aAny >>= bOldValue )
            {
                if ( bOldValue != bNewValue )
                {
                    aAny <<= bNewValue;
                    try
                    {
                        xPropSet->setPropertyValue( rKey, aAny );
                        bModified = sal_True;
                    }
                    catch ( ::com::sun::star::uno::Exception& )
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

        if ( ImplGetPropertyValue( aAny, xPropSet, rKey, sal_True ) )
        {
            sal_Int32 nOldValue = 0;
            if ( aAny >>= nOldValue )
            {
                if ( nOldValue != nNewValue )
                {
                    aAny <<= nNewValue;
                    try
                    {
                        xPropSet->setPropertyValue( rKey, aAny );
                        bModified = sal_True;
                    }
                    catch ( ::com::sun::star::uno::Exception& )
                    {
                        OSL_FAIL( "FilterConfigItem::WriteInt32 - could not set PropertyValue" );
                    }
                }
            }
        }
    }
}

// ------------------------------------------------------------------------

Sequence< PropertyValue > FilterConfigItem::GetFilterData() const
{
    return aFilterData;
}

// ------------------------------------------------------------------------

Reference< XStatusIndicator > FilterConfigItem::GetStatusIndicator() const
{
    Reference< XStatusIndicator > xStatusIndicator;
    const rtl::OUString sStatusIndicator( RTL_CONSTASCII_USTRINGPARAM( "StatusIndicator" ) );

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
