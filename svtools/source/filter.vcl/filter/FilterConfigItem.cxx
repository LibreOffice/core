/*************************************************************************
 *
 *  $RCSfile: FilterConfigItem.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 14:38:23 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _FILTER_CONFIG_ITEM_HXX_
#include "FilterConfigItem.hxx"
#endif

#include <tools/debug.hxx>
#ifndef _UTL_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <unotools/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCHANGESBATCH_HPP_
#include <com/sun/star/util/XChangesBatch.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HPP_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XHIERARCHICALNAMEACCESS_HPP_
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#endif

using namespace ::rtl;
using namespace ::utl                       ;   // getProcessServiceFactory
using namespace ::com::sun::star::lang      ;   // XMultiServiceFactory
using namespace ::com::sun::star::beans     ;   // PropertyValue
using namespace ::com::sun::star::uno       ;   // Reference
using namespace ::com::sun::star::util      ;   // XChangesBatch
using namespace ::com::sun::star::awt       ;   // Size
using namespace ::com::sun::star::container ;   //

static sal_Bool ImpIsTreeAvailable( Reference< XMultiServiceFactory >& rXCfgProv, const String& rTree )
{
    sal_Bool    bAvailable = rTree.Len() != 0;
    if ( bAvailable )
    {
        xub_StrLen  nTokenCount = rTree.GetTokenCount( (sal_Unicode)'/' );
        xub_StrLen  i = 0;

        if ( rTree.GetChar( 0 ) == (sal_Unicode)'/' )
            i++;
        if ( rTree.GetChar( rTree.Len() - 1 ) == (sal_Unicode)'/' )
            nTokenCount--;

        Any aAny;
        aAny <<= (OUString)rTree.GetToken( i++, (sal_Unicode)'/' );

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
        catch ( ::com::sun::star::uno::Exception& )
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
                    String aNode( rTree.GetToken( i, (sal_Unicode)'/' ) );
                    if ( !xHierarchicalNameAccess->hasByHierarchicalName( aNode ) )
                        bAvailable = sal_False;
                    else
                    {
                        Any aAny( xHierarchicalNameAccess->getByHierarchicalName( aNode ) );
                        try
                        {
                            aAny >>= xReadAccess;
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

    OUString sTree( ConfigManager::GetConfigBaseURL() );
    sTree += rSubTree;
    Reference< XMultiServiceFactory > xSMGR = getProcessServiceFactory();   // get global uno service manager

    Reference< XMultiServiceFactory > xCfgProv(
        xSMGR->createInstance( OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.configuration.ConfigurationProvider" ) ) ),
            UNO_QUERY );

    if ( xCfgProv.is() )
    {
        if ( ImpIsTreeAvailable( xCfgProv, String( sTree ) ) )
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
                DBG_ERROR( "FilterConfigItem::FilterConfigItem - Could not access configuration Key" );
            }
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
                    DBG_ERROR( "FilterConfigItem::FilterConfigItem - Could not update configuration data" );
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
    if ( rPropValue.Name.getLength() )
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


Size FilterConfigItem::ReadSize( const OUString& rKey, const Size& rDefault )
{
    Any aAny;
    Size aRetValue( rDefault );

    const OUString sWidth( RTL_CONSTASCII_USTRINGPARAM( "LogicalWidth" ) );
    const OUString sHeight( RTL_CONSTASCII_USTRINGPARAM( "LogicalHeight" ) );

    Reference< XPropertySet > aXPropSet;
    try
    {
        PropertyValue* pPropWidth = GetPropertyValue( aFilterData, sWidth  );
        PropertyValue* pPropHeight= GetPropertyValue( aFilterData, sHeight );
        if ( pPropWidth && pPropHeight )
        {
            pPropWidth->Value >>= aRetValue.Width;
            pPropHeight->Value >>= aRetValue.Height;
        }
        else if ( ImplGetPropertyValue( aAny, xPropSet, rKey, sal_True ) )
        {
            if ( aAny >>= aXPropSet )
            {
                if ( ImplGetPropertyValue( aAny, aXPropSet, sWidth, sal_True ) )
                    aAny >>= aRetValue.Width;
                if ( ImplGetPropertyValue( aAny, aXPropSet, sHeight, sal_True ) )
                    aAny >>= aRetValue.Height;
            }
        }
    }
    catch ( ::com::sun::star::uno::Exception& )
    {
        DBG_ERROR( "FilterConfigItem::ReadSize - could not read PropertyValue" );
    }
    PropertyValue aWidth;
    aWidth.Name = sWidth;
    aWidth.Value <<= aRetValue.Width;
    WritePropertyValue( aFilterData, aWidth );
    PropertyValue aHeight;
    aHeight.Name = sHeight;
    aHeight.Value <<= aRetValue.Height;
    WritePropertyValue( aFilterData, aHeight );
    return aRetValue;
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

Any FilterConfigItem::ReadAny( const ::rtl::OUString& rKey, const Any& rDefault )
{
    Any aAny, aRetValue( rDefault );
    PropertyValue* pPropVal = GetPropertyValue( aFilterData, rKey );
    if ( pPropVal )
    {
        aRetValue = pPropVal->Value;
    }
    else if ( ImplGetPropertyValue( aAny, xPropSet, rKey, sal_True ) )
    {
        aRetValue = aAny;
    }
    PropertyValue aPropValue;
    aPropValue.Name = rKey;
    aPropValue.Value = aRetValue;
    WritePropertyValue( aFilterData, aPropValue );
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
            sal_Bool bOldValue;
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
                        DBG_ERROR( "FilterConfigItem::WriteBool - could not set PropertyValue" );
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
            sal_Int32 nOldValue;
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
                        DBG_ERROR( "FilterConfigItem::WriteInt32 - could not set PropertyValue" );
                    }
                }
            }
        }
    }
}

void FilterConfigItem::WriteSize( const OUString& rKey, const Size& rNewValue )
{
    const OUString sWidth( RTL_CONSTASCII_USTRINGPARAM( "LogicalWidth" ) );
    const OUString sHeight( RTL_CONSTASCII_USTRINGPARAM( "LogicalHeight" ) );

    PropertyValue aWidth;
    aWidth.Name = sWidth;
    aWidth.Value <<= rNewValue.Width;
    WritePropertyValue( aFilterData, aWidth );

    PropertyValue aHeight;
    aHeight.Name = sHeight;
    aHeight.Value <<= rNewValue.Height;
    WritePropertyValue( aFilterData, aHeight );

    if ( xPropSet.is() )
    {
        Any aAny;
        sal_Int32 nOldWidth = rNewValue.Width;
        sal_Int32 nOldHeight = rNewValue.Height;

        if ( ImplGetPropertyValue( aAny, xPropSet, rKey, sal_True ) )
        {
            try
            {
                Reference< XPropertySet > aXPropSet;
                if ( aAny >>= aXPropSet )
                {
                    if ( ImplGetPropertyValue( aAny, aXPropSet, sWidth, sal_True ) )
                        aAny >>= nOldWidth;
                    if ( ImplGetPropertyValue( aAny, aXPropSet, sHeight, sal_True ) )
                        aAny >>= nOldHeight;
                }
                if ( ( nOldWidth != rNewValue.Width ) || ( nOldHeight != rNewValue.Height ) )
                {
                    aAny <<= rNewValue.Width;
                    aXPropSet->setPropertyValue( sWidth, aAny );
                    aAny <<= rNewValue.Height;
                    aXPropSet->setPropertyValue( sHeight, aAny );
                    bModified = sal_True;
                }
            }
            catch ( ::com::sun::star::uno::Exception& )
            {
                DBG_ERROR( "FilterConfigItem::WriteSize - could not read PropertyValue" );
            }
        }
    }
}

void FilterConfigItem::WriteString( const OUString& rKey, const OUString& rNewValue )
{
    PropertyValue aString;
    aString.Name = rKey;
    aString.Value <<= rNewValue;
    WritePropertyValue( aFilterData, aString );

    if ( xPropSet.is() )
    {
        Any aAny;

        if ( ImplGetPropertyValue( aAny, xPropSet, rKey, sal_True ) )
        {
            OUString aOldValue;
            if ( aAny >>= aOldValue )
            {
                if ( aOldValue != rNewValue )
                {
                    aAny <<= rNewValue;
                    try
                    {
                        xPropSet->setPropertyValue( rKey, aAny );
                        bModified = sal_True;
                    }
                    catch ( ::com::sun::star::uno::Exception& )
                    {
                        DBG_ERROR( "FilterConfigItem::WriteInt32 - could not set PropertyValue" );
                    }
                }
            }
        }
    }
}

void FilterConfigItem::WriteAny( const OUString& rKey, const Any& rNewAny )
{
    PropertyValue aPropValue;
    aPropValue.Name = rKey;
    aPropValue.Value = rNewAny;
    WritePropertyValue( aFilterData, aPropValue );
    if ( xPropSet.is() )
    {
        Any aAny;
        if ( ImplGetPropertyValue( aAny, xPropSet, rKey, sal_True ) )
        {
            if ( aAny != rNewAny )
            {
                try
                {
                    xPropSet->setPropertyValue( rKey, rNewAny );
                    bModified = sal_True;
                }
                catch ( com::sun::star::uno::Exception& )
                {
                    DBG_ERROR( "FilterConfigItem::WriteAny - could not set PropertyValue" );

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
