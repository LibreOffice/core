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
#include "precompiled_comphelper.hxx"

/** === begin UNO includes === **/
#include "com/sun/star/frame/XDispatchProvider.hpp"
#include "com/sun/star/frame/XSynchronousDispatch.hpp"
#include "com/sun/star/lang/XComponent.hpp"
#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#include "com/sun/star/util/XURLTransformer.hpp"
/** === end UNO includes === **/

#include "comphelper/synchronousdispatch.hxx"
#include "comphelper/processfactory.hxx"

#define UNISTRING(s) rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(s))

//.........................................................................
namespace comphelper
{
//.........................................................................

using namespace ::com::sun::star;

//====================================================================
//= SynchronousDispatch
//====================================================================

uno::Reference< lang::XComponent > SynchronousDispatch::dispatch(
        const uno::Reference< uno::XInterface > &xStartPoint,
        const rtl::OUString &sURL,
        const rtl::OUString &sTarget,
        const sal_Int32 nFlags,
        const uno::Sequence< beans::PropertyValue > &lArguments )
{
    util::URL aURL;
    aURL.Complete = sURL;
    uno::Reference < util::XURLTransformer > xTrans( ::comphelper::getProcessServiceFactory()->createInstance(
                                                                   UNISTRING("com.sun.star.util.URLTransformer" )),
                                                     uno::UNO_QUERY );
    if ( xTrans.is() )
        xTrans->parseStrict( aURL );

    uno::Reference < frame::XDispatch > xDispatcher;
    uno::Reference < frame::XDispatchProvider > xProvider( xStartPoint, uno::UNO_QUERY );

    if ( xProvider.is() )
        xDispatcher = xProvider->queryDispatch( aURL, sTarget, nFlags );

    uno::Reference < lang::XComponent > aComponent;

    if ( xDispatcher.is() )
    {
        try
        {
            uno::Any aRet;
            uno::Reference < frame::XSynchronousDispatch > xSyncDisp( xDispatcher, uno::UNO_QUERY_THROW );

            aRet = xSyncDisp->dispatchWithReturnValue( aURL, lArguments );

            aRet >>= aComponent;
        }
        catch ( uno::Exception& )
        {
            rtl::OUString aMsg = UNISTRING( "SynchronousDispatch::dispatch() Error while dispatching! ");
            OSL_ENSURE( sal_False, OUStringToOString(aMsg, RTL_TEXTENCODING_ASCII_US).getStr());
        }
    }

    return aComponent;
}

//.........................................................................
}   // namespace comphelper
//.........................................................................

