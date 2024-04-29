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

#include <mutex>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/DeploymentException.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

namespace com::sun::star::uno { class XComponentContext; }

using namespace ::com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;

namespace comphelper
{

namespace {

class LocalProcessFactory {
public:
    void set( const Reference< XMultiServiceFactory >& xSMgr )
    {
        std::unique_lock aGuard( maMutex );

        xProcessFactory = xSMgr;
    }

    Reference< XMultiServiceFactory > get() const
    {
        std::unique_lock aGuard( maMutex );

        return xProcessFactory;
    }

private:
    mutable std::mutex maMutex;
    Reference< XMultiServiceFactory > xProcessFactory;
};

/*
    This var preserves only that the above xProcessFactory variable will not be create when
    the library is loaded.
*/
LocalProcessFactory localProcessFactory;

}

void setProcessServiceFactory(const Reference< XMultiServiceFactory >& xSMgr)
{
    localProcessFactory.set( xSMgr );
}

Reference< XMultiServiceFactory > getProcessServiceFactory()
{
    Reference< XMultiServiceFactory> xReturn = localProcessFactory.get();
    if ( !xReturn.is() )
    {
        throw DeploymentException( u"null process service factory"_ustr );
    }
    return xReturn;
}

Reference< XComponentContext > getComponentContext(
    Reference< XMultiServiceFactory > const & factory)
{
    Reference< XComponentContext > xRet;
    uno::Reference<beans::XPropertySet> const xProps( factory, uno::UNO_QUERY );
    if (xProps.is()) {
        static constexpr OUStringLiteral DEFAULT_CONTEXT = u"DefaultContext";
        try {
            xRet.set( xProps->getPropertyValue(DEFAULT_CONTEXT),
                      uno::UNO_QUERY );
        }
        catch (beans::UnknownPropertyException & e) {
            throw DeploymentException(
                "unknown service factory DefaultContext property: " + e.Message,
                Reference<XInterface>(factory, UNO_QUERY) );
        }
    }
    if ( !xRet.is() )
    {
        throw DeploymentException(
            u"no service factory DefaultContext"_ustr,
            Reference<XInterface>(factory, UNO_QUERY) );
    }
    return xRet;
}

Reference< XComponentContext > getProcessComponentContext()
{
    static const uno::Reference<XComponentContext> processComponentContext = getComponentContext( getProcessServiceFactory() );
    return processComponentContext;
}

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
