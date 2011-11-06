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
#include "precompiled_forms.hxx"

#include "unohelper.hxx"

#include <osl/diagnose.h>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <unotools/processfactory.hxx>


using com::sun::star::uno::Reference;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::Exception;
using com::sun::star::uno::XInterface;
using com::sun::star::lang::XMultiServiceFactory;
using com::sun::star::beans::Property;
using com::sun::star::beans::XPropertySet;
using com::sun::star::beans::XPropertySetInfo;
using com::sun::star::beans::PropertyAttribute::READONLY;
using rtl::OUString;


Reference<XInterface> xforms::createInstance( const OUString& sServiceName )
{
    Reference<XMultiServiceFactory> xFactory = utl::getProcessServiceFactory();
    OSL_ENSURE( xFactory.is(), "can't get service factory" );

    Reference<XInterface> xInstance = xFactory->createInstance( sServiceName );
    OSL_ENSURE( xInstance.is(), "failed to create instance" );

    return xInstance;
}

void xforms::copy( const Reference<XPropertySet>& xFrom,
                   Reference<XPropertySet>& xTo )
{
    OSL_ENSURE( xFrom.is(), "no source" );
    OSL_ENSURE( xTo.is(), "no target" );

    // get property names & infos, and iterate over target properties
    Sequence<Property> aProperties =
        xTo->getPropertySetInfo()->getProperties();
    sal_Int32 nProperties = aProperties.getLength();
    const Property* pProperties = aProperties.getConstArray();
    Reference<XPropertySetInfo> xFromInfo = xFrom->getPropertySetInfo();
    for( sal_Int32 n = 0; n < nProperties; n++ )
    {
        const OUString& rName = pProperties[n].Name;

        // if both set have the property, copy the value
        // (catch and ignore exceptions, if any)
        if( xFromInfo->hasPropertyByName( rName ) )
        {
            try
            {
                Property aProperty = xFromInfo->getPropertyByName( rName );
                if ( ( aProperty.Attributes & READONLY ) == 0 )
                    xTo->setPropertyValue(rName, xFrom->getPropertyValue( rName ));
            }
            catch( const Exception& )
            {
                // ignore any errors; we'll copy as good as we can
            }
        }
        // else: no property? then ignore.
    }
}
