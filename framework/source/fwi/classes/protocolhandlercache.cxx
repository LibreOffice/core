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
    - may its a good idea to replace struct ProtocolHandler by css::beans::NamedValue type?!
*/

#include <classes/protocolhandlercache.hxx>
#include <classes/converter.hxx>
#include <threadhelp/readguard.hxx>
#include <threadhelp/writeguard.hxx>
#include <threadhelp/lockhelper.hxx>

#include <tools/wldcrd.hxx>
#include <unotools/configpaths.hxx>
#include <rtl/ustrbuf.hxx>

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
PatternHash::iterator PatternHash::findPatternKey( const OUString& sURL )
{
    PatternHash::iterator pItem = this->begin();
    while( pItem!=this->end() )
    {
        WildCard aPattern(pItem->first);
        if (aPattern.Matches(sURL))
            break;
        ++pItem;
    }
    return pItem;
}

//_________________________________________________________________________________________________________________

/**
    @short      initialize static member of class HandlerCache
    @descr      We use a singleton pattern to implement this handler cache.
                That means it use two static member list to hold all neccessary information
                and a ref count mechanism to create/destroy it on demand.
 */
HandlerHash* HandlerCache::m_pHandler  = NULL;
PatternHash* HandlerCache::m_pPattern  = NULL;
sal_Int32    HandlerCache::m_nRefCount = 0   ;
HandlerCFGAccess* HandlerCache::m_pConfig = NULL;

//_________________________________________________________________________________________________________________

/**
    @short      ctor of the cache of all registered protoco handler
    @descr      It tries to open the right configuration package automaticly
                and fill the internal structures. After that the cache can be
                used for read access on this data and perform some search
                operations on it.
 */
HandlerCache::HandlerCache()
{
    /* SAFE */{
        WriteGuard aGlobalLock( LockHelper::getGlobalLock() );

        if (m_nRefCount==0)
        {
            m_pHandler = new HandlerHash();
            m_pPattern = new PatternHash();
            m_pConfig  = new HandlerCFGAccess(PACKAGENAME_PROTOCOLHANDLER);
            m_pConfig->read(&m_pHandler,&m_pPattern);
            m_pConfig->setCache(this);
        }

        ++m_nRefCount;
    /* SAFE */}
}

//_________________________________________________________________________________________________________________

/**
    @short      dtor of the cache
    @descr      It frees all used memory. In further implementations (may if we support write access too)
                it's a good place to flush changes back to the configuration - but not needed yet.
 */
HandlerCache::~HandlerCache()
{
    /* SAFE */{
        WriteGuard aGlobalLock( LockHelper::getGlobalLock() );

        if( m_nRefCount==1)
        {
            m_pConfig->setCache(NULL);
            m_pHandler->free();
            m_pPattern->free();

            delete m_pConfig;
            delete m_pHandler;
            delete m_pPattern;
            m_pConfig = NULL;
            m_pHandler= NULL;
            m_pPattern= NULL;
        }

        --m_nRefCount;
    /* SAFE */}
}

//_________________________________________________________________________________________________________________

/**
    @short      dtor of the cache
    @descr      It frees all used memory. In further implementations (may if we support write access too)
                it's a good place to flush changes back to the configuration - but not needed yet.
 */
sal_Bool HandlerCache::search( const OUString& sURL, ProtocolHandler* pReturn ) const
{
    sal_Bool bFound = sal_False;
    /* SAFE */{
        ReadGuard aReadLock( LockHelper::getGlobalLock() );
        PatternHash::const_iterator pItem = m_pPattern->findPatternKey(sURL);
        if (pItem!=m_pPattern->end())
        {
            *pReturn = (*m_pHandler)[pItem->second];
            bFound = sal_True;
        }
    /* SAFE */}
    return bFound;
}

//_________________________________________________________________________________________________________________

/**
    @short      search for a registered handler by using an URL struct
    @descr      We combine neccessary parts of this struct to a valid URL string
                and call our other search method ...
                It's a helper for outside code.
 */
sal_Bool HandlerCache::search( const css::util::URL& aURL, ProtocolHandler* pReturn ) const
{
    return search( aURL.Complete, pReturn );
}

//_________________________________________________________________________________________________________________
void HandlerCache::takeOver(HandlerHash* pHandler, PatternHash* pPattern)
{
    // SAFE ->
    WriteGuard aWriteLock( LockHelper::getGlobalLock() );

    HandlerHash* pOldHandler = m_pHandler;
    PatternHash* pOldPattern = m_pPattern;

    m_pHandler = pHandler;
    m_pPattern = pPattern;

    pOldHandler->free();
    pOldPattern->free();
    delete pOldHandler;
    delete pOldPattern;

    aWriteLock.unlock();
    // <- SAFE
}

//_________________________________________________________________________________________________________________

/**
    @short      dtor of the config access class
    @descr      It opens the configuration package automaticly by using base class mechanism.
                After that "read()" method of this class should be called to use it.

    @param      sPackage
                specifies the package name of the configuration data which should be used
 */
HandlerCFGAccess::HandlerCFGAccess( const OUString& sPackage )
    : ConfigItem( sPackage )
{
    css::uno::Sequence< OUString > lListenPaths(1);
    lListenPaths[0] = SETNAME_HANDLER;
    EnableNotification(lListenPaths);
}

//_________________________________________________________________________________________________________________

/**
    @short      use base class mechanism to fill given structures
    @descr      User use us as a wrapper between configuration api and his internal structures.
                He give us some pointer to his member and we fill it.

    @param      pHandler
                pointer to a list of protocol handler infos

    @param      pPattern
                reverse map of handler pattern to her uno names
 */
void HandlerCFGAccess::read( HandlerHash** ppHandler ,
                             PatternHash** ppPattern )
{
    // list of all uno implementation names without encoding
    css::uno::Sequence< OUString > lNames = GetNodeNames( SETNAME_HANDLER, ::utl::CONFIG_NAME_LOCAL_PATH );
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
    LOG_ASSERT2( lFullNames.getLength()!=lValues.getLength(), "HandlerCFGAccess::read()", "Miss some configuration values of handler set!" )

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
        for (OUStringList::iterator pItem =aHandler.m_lProtocols.begin();
                                    pItem!=aHandler.m_lProtocols.end()  ;
                                    ++pItem                             )
        {
            (**ppPattern)[*pItem] = lNames[nSource];
        }

        // insert the handler info into the normal handler cache
        (**ppHandler)[lNames[nSource]] = aHandler;
        ++nSource;
    }
}

//_________________________________________________________________________________________________________________
void HandlerCFGAccess::Notify(const css::uno::Sequence< OUString >& /*lPropertyNames*/)
{
    HandlerHash* pHandler = new HandlerHash;
    PatternHash* pPattern = new PatternHash;

    read(&pHandler, &pPattern);
    if (m_pCache)
        m_pCache->takeOver(pHandler, pPattern);
    else
    {
        delete pHandler;
        delete pPattern;
    }
}

void HandlerCFGAccess::Commit()
{
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
