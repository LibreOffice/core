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
#include "precompiled_extensions.hxx"
#include "unourl.hxx"
#include <com/sun/star/util/XURLTransformer.hpp>

//........................................................................
namespace pcr
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::util;

    //====================================================================
    //= UnoURL
    //====================================================================
    UnoURL::UnoURL( const ::rtl::OUString& _rCompleteURL, const Reference< XMultiServiceFactory >& _rxORB )
    {
        m_aURL.Complete = _rCompleteURL;

        OSL_ENSURE( _rxORB.is(), "UnoURL::UnoURL: invalid ORB!" );
        Reference< XURLTransformer > xTransform;
        try
        {
            if ( _rxORB.is() )
            {
                xTransform = xTransform.query( _rxORB->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.util.URLTransformer" ) ) ) );
                OSL_ENSURE( xTransform.is(), "UnoURL::UnoURL: could not create an URL transformer!" );
                if ( xTransform.is() )
                    xTransform->parseStrict( m_aURL );
            }
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "UnoURL::UnoURL: caught an exception!" );
        }
    }

//........................................................................
}   // namespace pcr
//........................................................................

