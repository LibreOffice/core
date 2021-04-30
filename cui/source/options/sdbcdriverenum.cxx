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

#include "sdbcdriverenum.hxx"
#include <comphelper/processfactory.hxx>
#include <osl/diagnose.h>
#include <tools/diagnose_ex.h>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/sdbc/DriverManager.hpp>


namespace offapp
{


    using namespace css::uno;
    using namespace css::lang;
    using namespace css::container;
    using namespace css::sdbc;

    class ODriverEnumerationImpl
    {
    protected:
        std::vector< OUString >    m_aImplNames;

    public:
        ODriverEnumerationImpl();

        const std::vector< OUString >& getDriverImplNames() const { return m_aImplNames; }
    };


    ODriverEnumerationImpl::ODriverEnumerationImpl()
    {
        try
        {
            Reference< XComponentContext > xContext = ::comphelper::getProcessComponentContext();
            Reference< XDriverManager2 > xEnumAccess = DriverManager::create( xContext );

            Reference< XEnumeration > xEnumDrivers = xEnumAccess->createEnumeration();
            OSL_ENSURE(xEnumDrivers.is(), "ODriverEnumerationImpl::ODriverEnumerationImpl: invalid enumeration object!");

            Reference< XServiceInfo > xDriverSI;
            while (xEnumDrivers->hasMoreElements())
            {
                xEnumDrivers->nextElement() >>= xDriverSI;
                OSL_ENSURE(xDriverSI.is(), "ODriverEnumerationImpl::ODriverEnumerationImpl: driver without service info!");
                if (xDriverSI.is())
                    m_aImplNames.push_back(xDriverSI->getImplementationName());
            }
        }
        catch(const Exception&)
        {
            TOOLS_WARN_EXCEPTION( "cui.options", "ODriverEnumerationImpl::ODriverEnumerationImpl: caught an exception while enumerating the drivers!");
        }
    }

    ODriverEnumeration::ODriverEnumeration() noexcept
        :m_pImpl(new ODriverEnumerationImpl)
    {
    }


    ODriverEnumeration::~ODriverEnumeration() noexcept
    {
    }


    ODriverEnumeration::const_iterator ODriverEnumeration::begin() const noexcept
    {
        return m_pImpl->getDriverImplNames().begin();
    }


    ODriverEnumeration::const_iterator ODriverEnumeration::end() const noexcept
    {
        return m_pImpl->getDriverImplNames().end();
    }

}   // namespace offapp


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
