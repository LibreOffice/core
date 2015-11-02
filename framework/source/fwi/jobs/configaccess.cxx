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

#include <jobs/configaccess.hxx>
#include <general.h>
#include <services.h>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XMultiHierarchicalPropertySet.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>

#include <unotools/configpaths.hxx>
#include <rtl/ustrbuf.hxx>

namespace framework{

/**
    @short  open the configuration of this job
    @descr  We open the configuration of this job only. Not the whole package or the whole
            job set. We are interested on our own properties only.
            We set the opened configuration access as our member. So any following method,
            which needs cfg access, can use it. That prevent us against multiple open/close requests.
            But you can use this method to upgrade an already opened configuration too.

    @param  eMode
                force opening of the configuration access in readonly or in read/write mode
 */
ConfigAccess::ConfigAccess( /*IN*/ const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                            /*IN*/ const OUString&                                    sRoot )
    : m_xContext    ( rxContext)
    , m_sRoot       ( sRoot    )
    , m_eMode       ( E_CLOSED )
{
}

/**
    @short  last chance to close an open configuration access point
    @descr  In case our user forgot to close this configuration point
            in the right way, normally he will run into some trouble -
            e.g. losing data.
 */
ConfigAccess::~ConfigAccess()
{
    close();
}

/**
    @short  return the internal mode of this instance
    @descr  May be the outside user need any information about successfully opened
            or closed config access point objects. He can control the internal mode to do so.

    @return The internal open state of this object.
 */
ConfigAccess::EOpenMode ConfigAccess::getMode() const
{
    osl::MutexGuard g(m_mutex);
    return m_eMode;
}

/**
    @short  open the configuration access in the specified mode
    @descr  We set the opened configuration access as our member. So any following method,
            which needs cfg access, can use it. That prevent us against multiple open/close requests.
            But you can use this method to upgrade an already opened configuration too.
            It's possible to open a config access in READONLY mode first and "open" it at a second
            time within the mode READWRITE. Then we will upgrade it. Downgrade will be possible too.

            But note: closing will be done explicitly by calling method close() ... not by
            downgrading with mode CLOSED!

    @param  eMode
                force (re)opening of the configuration access in readonly or in read/write mode
 */
void ConfigAccess::open( /*IN*/ EOpenMode eMode )
{
    osl::MutexGuard g(m_mutex);

    // check if configuration is already open in the right mode.
    // By the way: Don't allow closing by using this method!
    if (
        (eMode  !=E_CLOSED) &&
        (m_eMode!=eMode   )
       )
    {
        // We have to close the old access point without any question here.
        // It will be open again using the new mode.
        // can be called without checks! It does the checks by itself ...
        // e.g. for already closed or not opened configuration.
        // Flushing of all made changes will be done here too.
        close();

        // create the configuration provider, which provides sub access points
        css::uno::Reference< css::lang::XMultiServiceFactory > xConfigProvider = css::configuration::theDefaultProvider::get(m_xContext);
        css::beans::PropertyValue aParam;
        aParam.Name    = "nodepath";
        aParam.Value <<= m_sRoot;

        css::uno::Sequence< css::uno::Any > lParams(1);
        lParams[0] <<= aParam;

        // open it
        try
        {
            if (eMode==E_READONLY)
                m_xConfig = xConfigProvider->createInstanceWithArguments(SERVICENAME_CFGREADACCESS  , lParams);
            else
            if (eMode==E_READWRITE)
                m_xConfig = xConfigProvider->createInstanceWithArguments(SERVICENAME_CFGUPDATEACCESS, lParams);
        }
        catch(const css::uno::Exception& ex)
        {
            (void) ex; // avoid warning
            SAL_INFO("fwk", "open config: " << ex.Message);
        }

        m_eMode = E_CLOSED;
        if (m_xConfig.is())
            m_eMode = eMode;
    }
}

/**
    @short  close the internal opened configuration access and flush all changes
    @descr  It checks, if the given access is valid and react in the right way.
            It flushes all changes ... so nobody else must know this state.
 */
void ConfigAccess::close()
{
    osl::MutexGuard g(m_mutex);
    // check already closed configuration
    if (m_xConfig.is())
    {
        css::uno::Reference< css::util::XChangesBatch > xFlush(m_xConfig, css::uno::UNO_QUERY);
        if (xFlush.is())
            xFlush->commitChanges();
        m_xConfig.clear();
        m_eMode   = E_CLOSED;
    }
}

/**
    @short  provides an access to the internal wrapped configuration access
    @descr  It's not allowed to safe this c++ (!) reference outside. You have
            to use it directly. Further you must use our public lock member m_aLock
            to synchronize your code with our internal structures and our interface
            methods. Acquire it before you call cfg() and release it afterwards immediately.

            E.g.:   ConfigAccess aAccess(...);
                    Guard aReadLock(aAccess.m_aLock);
                    Reference< XPropertySet > xSet(aAccess.cfg(), UNO_QUERY);
                    Any aProp = xSet->getPropertyValue("...");
                    aReadLock.unlock();

    @attention  During this time it's not allowed to call the methods open() or close()!
                Otherwhise you will change your own referenced config access. Anything will
                be possible then.

    @return A c++(!) reference to the uno instance of the configuration access point.
 */
const css::uno::Reference< css::uno::XInterface >& ConfigAccess::cfg()
{
    // must be synchronized from outside!
    // => no lock here ...
    return m_xConfig;
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
