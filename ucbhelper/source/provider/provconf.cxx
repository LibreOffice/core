/*************************************************************************
 *
 *  $RCSfile: provconf.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: sb $ $Date: 2000-11-09 13:22:49 $
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
 *  Contributor(s): Kai Sommerfeld ( kso@sun.com )
 *
 *
 ************************************************************************/

/**************************************************************************
                                TODO
 **************************************************************************

  *************************************************************************/

#ifndef _UCBHELPER_PROVCONF_HXX_
#include <provconf.hxx>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XHIERARCHICALNAMEACCESS_HPP_
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

using namespace com::sun::star;
using namespace rtl;

//=========================================================================

#define CONFIG_CONTENTPROVIDERS_KEY \
                "/org.openoffice.ucb.Configuration/ContentProviders"

//=========================================================================

namespace ucb {

bool getContentProviderData(
            const uno::Reference< lang::XMultiServiceFactory > & rServiceMgr,
            const OUString & rKey1,
            const OUString & rKey2,
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
                    OUString::createFromAscii(
                        "com.sun.star.configuration.ConfigurationProvider" ) ),
                uno::UNO_QUERY );

        OUString aFullPath
                    = OUString::createFromAscii( CONFIG_CONTENTPROVIDERS_KEY );
        aFullPath += OUString::createFromAscii( "/" );
        aFullPath += rKey1;
        aFullPath += OUString::createFromAscii( "/SecondaryKeys/" );
        aFullPath += rKey2;
        aFullPath += OUString::createFromAscii( "/ProviderData" );

        uno::Sequence< uno::Any > aArguments( 1 );
        aArguments[ 0 ] <<= aFullPath;

        uno::Reference< uno::XInterface > xInterface(
                xConfigProv->createInstanceWithArguments(
                    OUString::createFromAscii(
                        "com.sun.star.configuration.ConfigurationAccess" ),
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

        uno::Sequence< OUString > aElems = xNameAccess->getElementNames();
        const OUString* pElems = aElems.getConstArray();
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

            OUString aServiceKey  = OUString::createFromAscii( "/ServiceName" );
            OUString aTemplateKey = OUString::createFromAscii( "/URLTemplate" );
            OUString aArgsKey     = OUString::createFromAscii( "/Arguments" );

            // Iterate over children.
            for ( sal_Int32 n = 0; n < nCount; ++n )
            {
                const OUString& rElem = pElems[ n ];

                try
                {
                    ContentProviderData aInfo;

                    // Obtain service name.

                    OUString aKey = rElem;
                    aKey += aServiceKey;

                    OUString aValue;
                    if ( !( xHierNameAccess->getByHierarchicalName( aKey )
                        >>= aValue ) )
                    {
                        OSL_ENSURE( false,
                                    "getContentProviderData - "
                                    "Error getting item value!" );
                        continue;
                    }

                    aInfo.ServiceName = aValue;

                    // Obtain URL Template.

                    aKey = rElem;
                    aKey += aTemplateKey;

                    if ( !( xHierNameAccess->getByHierarchicalName( aKey )
                        >>= aValue ) )
                    {
                        OSL_ENSURE( false,
                                    "getContentProviderData - "
                                    "Error getting item value!" );
                        continue;
                    }

                    aInfo.URLTemplate = aValue;

                    // Obtain Arguments.

                    aKey = rElem;
                    aKey += aArgsKey;

                    if ( !( xHierNameAccess->getByHierarchicalName( aKey )
                        >>= aValue ) )
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
