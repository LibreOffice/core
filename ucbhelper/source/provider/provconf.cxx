/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucbhelper.hxx"

/**************************************************************************
                                TODO
 **************************************************************************

  *************************************************************************/

#ifndef _UCBHELPER_PROVCONF_HXX_
#include <provconf.hxx>
#endif
#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

using namespace com::sun::star;

//=========================================================================

#define CONFIG_CONTENTPROVIDERS_KEY \
                "/org.openoffice.ucb.Configuration/ContentProviders"

//=========================================================================

namespace ucbhelper {

void makeAndAppendXMLName(
                rtl::OUStringBuffer & rBuffer, const rtl::OUString & rIn )
{
    sal_Int32 nCount = rIn.getLength();
    for ( sal_Int32 n = 0; n < nCount; ++n )
    {
        const sal_Unicode c = rIn.getStr()[ n ];
        switch ( c )
        {
            case '&':
                rBuffer.appendAscii( "&amp;" );
                break;

            case '"':
                rBuffer.appendAscii( "&quot;" );
                break;

            case '\'':
                rBuffer.appendAscii( "&apos;" );
                break;

            case '<':
                rBuffer.appendAscii( "&lt;" );
                break;

            case '>':
                rBuffer.appendAscii( "&gt;" );
                break;

            default:
                rBuffer.append( c );
                break;
        }
    }
}

//=========================================================================
bool getContentProviderData(
            const uno::Reference< lang::XMultiServiceFactory > & rServiceMgr,
            const rtl::OUString & rKey1,
            const rtl::OUString & rKey2,
            ContentProviderDataList & rListToFill )
{
    if ( !rServiceMgr.is() || !rKey1.getLength() || !rKey2.getLength() )
    {
        OSL_ENSURE( false,
                    "getContentProviderData - Invalid argument!" );
        return false;
    }

    try
    {
        uno::Reference< lang::XMultiServiceFactory > xConfigProv(
                rServiceMgr->createInstance(
                    rtl::OUString::createFromAscii(
                        "com.sun.star.configuration.ConfigurationProvider" ) ),
                uno::UNO_QUERY );

        if ( !xConfigProv.is() )
        {
            OSL_ENSURE( false,
                        "getContentProviderData - No config provider!" );
            return false;
        }

        rtl::OUStringBuffer aFullPath;
        aFullPath.appendAscii( CONFIG_CONTENTPROVIDERS_KEY "/['" );
        makeAndAppendXMLName( aFullPath, rKey1 );
        aFullPath.appendAscii( "']/SecondaryKeys/['" );
        makeAndAppendXMLName( aFullPath, rKey2 );
        aFullPath.appendAscii( "']/ProviderData" );

        uno::Sequence< uno::Any > aArguments( 1 );
        beans::PropertyValue      aProperty;
        aProperty.Name
            = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "nodepath" ) );
        aProperty.Value <<= aFullPath.makeStringAndClear();
        aArguments[ 0 ] <<= aProperty;

        uno::Reference< uno::XInterface > xInterface(
                xConfigProv->createInstanceWithArguments(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                        "com.sun.star.configuration.ConfigurationAccess" ) ),
                    aArguments ) );

        if ( !xInterface.is() )
        {
            OSL_ENSURE( false,
                        "getContentProviderData - No config access!" );
            return false;
        }

        uno::Reference< container::XNameAccess > xNameAccess(
                                            xInterface, uno::UNO_QUERY );

        if ( !xNameAccess.is() )
        {
            OSL_ENSURE( false,
                        "getContentProviderData - No XNameAccess!" );
            return false;
        }

        uno::Sequence< rtl::OUString > aElems = xNameAccess->getElementNames();
        const rtl::OUString* pElems = aElems.getConstArray();
        sal_Int32 nCount = aElems.getLength();

        if ( nCount > 0 )
        {
            uno::Reference< container::XHierarchicalNameAccess >
                                xHierNameAccess( xInterface, uno::UNO_QUERY );

            if ( !xHierNameAccess.is() )
            {
                OSL_ENSURE( false,
                            "getContentProviderData - "
                            "No XHierarchicalNameAccess!" );
                return false;
            }

            // Iterate over children.
            for ( sal_Int32 n = 0; n < nCount; ++n )
            {
                rtl::OUStringBuffer aElemBuffer;
                aElemBuffer.appendAscii( "['" );
                makeAndAppendXMLName( aElemBuffer, pElems[ n ] );

                try
                {
                    ContentProviderData aInfo;

                    // Obtain service name.
                    rtl::OUStringBuffer aKeyBuffer = aElemBuffer;
                    aKeyBuffer.appendAscii( "']/ServiceName" );

                    rtl::OUString aValue;
                    if ( !( xHierNameAccess->getByHierarchicalName(
                                aKeyBuffer.makeStringAndClear() ) >>= aValue ) )
                    {
                        OSL_ENSURE( false,
                                    "getContentProviderData - "
                                    "Error getting item value!" );
                        continue;
                    }

                    aInfo.ServiceName = aValue;

                    // Obtain URL Template.
                    aKeyBuffer = aElemBuffer;
                    aKeyBuffer.appendAscii( "']/URLTemplate" );

                    if ( !( xHierNameAccess->getByHierarchicalName(
                                aKeyBuffer.makeStringAndClear() ) >>= aValue ) )
                    {
                        OSL_ENSURE( false,
                                    "getContentProviderData - "
                                    "Error getting item value!" );
                        continue;
                    }

                    aInfo.URLTemplate = aValue;

                    // Obtain Arguments.
                    aKeyBuffer = aElemBuffer;
                    aKeyBuffer.appendAscii( "']/Arguments" );

                    if ( !( xHierNameAccess->getByHierarchicalName(
                                aKeyBuffer.makeStringAndClear() ) >>= aValue ) )
                    {
                        OSL_ENSURE( false,
                                    "getContentProviderData - "
                                    "Error getting item value!" );
                        continue;
                    }

                    aInfo.Arguments = aValue;

                    // Append info to list.
                    rListToFill.push_back( aInfo );
                }
                catch ( container::NoSuchElementException& )
                {
                    // getByHierarchicalName

                    OSL_ENSURE( false,
                                "getContentProviderData - "
                                "caught NoSuchElementException!" );
                }
            }
        }
    }
    catch ( uno::RuntimeException& )
    {
        OSL_ENSURE( false,
                    "getContentProviderData - caught RuntimeException!" );
        return false;
    }
    catch ( uno::Exception& )
    {
        // createInstance, createInstanceWithArguments

        OSL_ENSURE( false,
                    "getContentProviderData - caught Exception!" );
        return false;
    }

    return true;
}

}
