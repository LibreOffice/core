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
            OSL_FAIL( OUStringToOString(aMsg, RTL_TEXTENCODING_ASCII_US).getStr());
        }
    }

    return aComponent;
}

//.........................................................................
}   // namespace comphelper
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
