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

/*TODO
    - change "singleton" behaviour by using new helper ::comhelper::SingletonRef
    - rename method exist() to existHandlerForURL() or similar one
    - may it's a good idea to replace struct ProtocolHandler by css::beans::NamedValue type?!
*/

#include <classes/protocolhandlercache.hxx>
#include <classes/converter.hxx>

#include <tools/wldcrd.hxx>
#include <unotools/configpaths.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <vcl/svapp.hxx>

#define SETNAME_HANDLER "HandlerSet" // name of configuration set inside package

namespace framework{

/**
    @short      overloaded index operator of hash map to support pattern key search
    @descr      All keys inside this hash map are URL pattern which points to an uno
                implementation name of a protocol handler service which is registered
                for this pattern. This operator makes it easy to find such registered
                handler by using a full qualified URL and compare it with all pattern
                keys.

    @param      sURL
                the full qualified URL which should match to a registered pattern

    @return     An iterator which points to the found item inside the hash or PatternHash::end()
                if no pattern match this given <var>sURL</var>.
 */
namespace {

PatternHash::const_iterator findPatternKey(PatternHash const * hash, const OUString& sURL)
{
    return std::find_if(hash->begin(), hash->end(),
        [&sURL](const PatternHash::value_type& rEntry) {
            WildCard aPattern(rEntry.first);
            return aPattern.Matches(sURL);
        });
}

}

/**
    @short      initialize static member of class HandlerCache
    @descr      We use a singleton pattern to implement this handler cache.
                That means it use two static member list to hold all necessary information
                and a ref count mechanism to create/destroy it on demand.
 */
std::unique_ptr<HandlerHash> HandlerCache::s_pHandler;
std::unique_ptr<PatternHash> HandlerCache::s_pPattern;
sal_Int32    HandlerCache::m_nRefCount = 0;
HandlerCFGAccess* HandlerCache::s_pConfig = nullptr;

/**
    @short      ctor of the cache of all registered protocol handler
    @descr      It tries to open the right configuration package automatically
                and fill the internal structures. After that the cache can be
                used for read access on this data and perform some search
                operations on it.
 */
HandlerCache::HandlerCache()
{
    SolarMutexGuard aGuard;

    if (m_nRefCount==0)
    {
        s_pHandler.reset(new HandlerHash);
        s_pPattern.reset(new PatternHash);
        s_pConfig = new HandlerCFGAccess(PACKAGENAME_PROTOCOLHANDLER);
        s_pConfig->read(*s_pHandler, *s_pPattern);
        s_pConfig->setCache(this);
    }

    ++m_nRefCount;
}

/**
    @short      dtor of the cache
    @descr      It frees all used memory. In further implementations (may if we support write access too)
                it's a good place to flush changes back to the configuration - but not needed yet.
 */
HandlerCache::~HandlerCache()
{
    SolarMutexGuard aGuard;

    if( m_nRefCount==1)
    {
        s_pConfig->setCache(nullptr);

        delete s_pConfig;
        s_pConfig = nullptr;
        s_pHandler.reset();
        s_pPattern.reset();
    }

    --m_nRefCount;
}

/**
    @short      dtor of the cache
    @descr      It frees all used memory. In further implementations (may if we support write access too)
                it's a good place to flush changes back to the configuration - but not needed yet.
 */
bool HandlerCache::search( const OUString& sURL, ProtocolHandler* pReturn ) const
{
    bool bFound = false;

    SolarMutexGuard aGuard;

    PatternHash::const_iterator pItem = findPatternKey(s_pPattern.get(), sURL);
    if (pItem != s_pPattern->end())
    {
        *pReturn = (*s_pHandler)[pItem->second];
        bFound = true;
    }

    return bFound;
}

/**
    @short      search for a registered handler by using an URL struct
    @descr      We combine necessary parts of this struct to a valid URL string
                and call our other search method ...
                It's a helper for outside code.
 */
bool HandlerCache::search( const css::util::URL& aURL, ProtocolHandler* pReturn ) const
{
    return search( aURL.Complete, pReturn );
}

void HandlerCache::takeOver(std::unique_ptr<HandlerHash> pHandler, std::unique_ptr<PatternHash> pPattern)
{
    SolarMutexGuard aGuard;

    s_pHandler = std::move(pHandler);
    s_pPattern = std::move(pPattern);
}

/**
    @short      dtor of the config access class
    @descr      It opens the configuration package automatically by using base class mechanism.
                After that "read()" method of this class should be called to use it.

    @param      sPackage
                specifies the package name of the configuration data which should be used
 */
HandlerCFGAccess::HandlerCFGAccess( const OUString& sPackage )
    : ConfigItem(sPackage)
    , m_pCache(nullptr)
{
    css::uno::Sequence< OUString > lListenPaths { SETNAME_HANDLER };
    EnableNotification(lListenPaths);
}

/**
    @short      use base class mechanism to fill given structures
    @descr      User use us as a wrapper between configuration api and his internal structures.
                He give us some pointer to his member and we fill it.

    @param      rHandlerHash
                list of protocol handler infos

    @param      rPatternHash
                reverse map of handler pattern to her uno names
 */
void HandlerCFGAccess::read( HandlerHash& rHandlerHash, PatternHash& rPatternHash )
{
    // list of all uno implementation names without encoding
    css::uno::Sequence< OUString > lNames = GetNodeNames( SETNAME_HANDLER, ::utl::ConfigNameFormat::LocalPath );
    sal_Int32 nSourceCount = lNames.getLength();
    sal_Int32 nTargetCount = nSourceCount;
    // list of all full qualified path names of configuration entries
    css::uno::Sequence< OUString > lFullNames ( nTargetCount );

    // expand names to full path names
    sal_Int32 nSource=0;
    sal_Int32 nTarget=0;
    for( nSource=0; nSource<nSourceCount; ++nSource )
    {
        OUStringBuffer sPath( SETNAME_HANDLER );
        sPath.append(CFG_PATH_SEPARATOR);
        sPath.append(lNames[nSource]);
        sPath.append(CFG_PATH_SEPARATOR);
        sPath.append(PROPERTY_PROTOCOLS);

        lFullNames[nTarget]  = sPath.makeStringAndClear();
        ++nTarget;
    }

    // get values at all
    css::uno::Sequence< css::uno::Any > lValues = GetProperties( lFullNames );
    SAL_WARN_IF( lFullNames.getLength()!=lValues.getLength(), "fwk", "HandlerCFGAccess::read(): Miss some configuration values of handler set!" );

    // fill structures
    nSource = 0;
    for( nTarget=0; nTarget<nTargetCount; ++nTarget )
    {
        // create it new for every loop to guarantee a real empty object!
        ProtocolHandler aHandler;
        aHandler.m_sUNOName = ::utl::extractFirstFromConfigurationPath(lNames[nSource]);

        // unpack all values of this handler
        css::uno::Sequence< OUString > lTemp;
        lValues[nTarget] >>= lTemp;
        aHandler.m_lProtocols = Converter::convert_seqOUString2OUStringList(lTemp);

        // register his pattern into the performance search hash
        for (auto const& item : aHandler.m_lProtocols)
        {
            rPatternHash[item] = lNames[nSource];
        }

        // insert the handler info into the normal handler cache
        rHandlerHash[lNames[nSource]] = aHandler;
        ++nSource;
    }
}

void HandlerCFGAccess::Notify(const css::uno::Sequence< OUString >& /*lPropertyNames*/)
{
    std::unique_ptr<HandlerHash> pHandler(new HandlerHash);
    std::unique_ptr<PatternHash> pPattern(new PatternHash);

    read(*pHandler, *pPattern);
    if (m_pCache)
        m_pCache->takeOver(std::move(pHandler), std::move(pPattern));
}

void HandlerCFGAccess::ImplCommit()
{
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
