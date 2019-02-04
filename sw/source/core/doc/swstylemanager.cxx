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

#include "swstylemanager.hxx"
#include <svl/stylepool.hxx>
#include <doc.hxx>
#include <charfmt.hxx>
#include <docary.hxx>
#include <swtypes.hxx>
#include <istyleaccess.hxx>
#include <unordered_map>
#include <osl/diagnose.h>

typedef std::unordered_map< OUString,
                            std::shared_ptr<SfxItemSet> > SwStyleNameCache;

class SwStyleCache
{
    SwStyleNameCache mMap;
public:
    SwStyleCache() {}
    void addStyleName( const std::shared_ptr<SfxItemSet>& pStyle )
        { mMap[ StylePool::nameOf(pStyle) ] = pStyle; }
    void addCompletePool( StylePool& rPool );
    std::shared_ptr<SfxItemSet> getByName( const OUString& rName ) { return mMap[rName]; }
};

void SwStyleCache::addCompletePool( StylePool& rPool )
{
    std::unique_ptr<IStylePoolIteratorAccess> pIter = rPool.createIterator();
    std::shared_ptr<SfxItemSet> pStyle = pIter->getNext();
    while( pStyle.get() )
    {
        OUString aName( StylePool::nameOf(pStyle) );
        mMap[ aName ] = pStyle;
        pStyle = pIter->getNext();
    }
}

class SwStyleManager : public IStyleAccess
{
    StylePool aAutoCharPool;
    StylePool aAutoParaPool;
    std::unique_ptr<SwStyleCache> mpCharCache;
    std::unique_ptr<SwStyleCache> mpParaCache;

public:
    // accept empty item set for ignorable paragraph items.
    explicit SwStyleManager( SfxItemSet const * pIgnorableParagraphItems )
        : aAutoCharPool(),
          aAutoParaPool( pIgnorableParagraphItems )
    {}
    virtual std::shared_ptr<SfxItemSet> getAutomaticStyle( const SfxItemSet& rSet,
                                                               IStyleAccess::SwAutoStyleFamily eFamily ) override;
    virtual std::shared_ptr<SfxItemSet> getByName( const OUString& rName,
                                                               IStyleAccess::SwAutoStyleFamily eFamily ) override;
    virtual void getAllStyles( std::vector<std::shared_ptr<SfxItemSet>> &rStyles,
                                                               IStyleAccess::SwAutoStyleFamily eFamily ) override;
    virtual std::shared_ptr<SfxItemSet> cacheAutomaticStyle( const SfxItemSet& rSet,
                                                               SwAutoStyleFamily eFamily ) override;
    virtual void clearCaches() override;
};

std::unique_ptr<IStyleAccess> createStyleManager( SfxItemSet const * pIgnorableParagraphItems )
{
    return std::make_unique<SwStyleManager>( pIgnorableParagraphItems );
}

void SwStyleManager::clearCaches()
{
    mpCharCache.reset();
    mpParaCache.reset();
}

std::shared_ptr<SfxItemSet> SwStyleManager::getAutomaticStyle( const SfxItemSet& rSet,
                                                                   IStyleAccess::SwAutoStyleFamily eFamily )
{
    StylePool& rAutoPool = eFamily == IStyleAccess::AUTO_STYLE_CHAR ? aAutoCharPool : aAutoParaPool;
    return rAutoPool.insertItemSet( rSet );
}

std::shared_ptr<SfxItemSet> SwStyleManager::cacheAutomaticStyle( const SfxItemSet& rSet,
                                                                   IStyleAccess::SwAutoStyleFamily eFamily )
{
    StylePool& rAutoPool = eFamily == IStyleAccess::AUTO_STYLE_CHAR ? aAutoCharPool : aAutoParaPool;
    std::shared_ptr<SfxItemSet> pStyle = rAutoPool.insertItemSet( rSet );
    if (eFamily == IStyleAccess::AUTO_STYLE_CHAR)
    {
        if (!mpCharCache)
            mpCharCache.reset(new SwStyleCache());
        mpCharCache->addStyleName( pStyle );
    }
    else
    {
        if (!mpParaCache)
            mpParaCache.reset(new SwStyleCache());
        mpParaCache->addStyleName( pStyle );
    }
    return pStyle;
}

std::shared_ptr<SfxItemSet> SwStyleManager::getByName( const OUString& rName,
                                                           IStyleAccess::SwAutoStyleFamily eFamily )
{
    StylePool& rAutoPool = eFamily == IStyleAccess::AUTO_STYLE_CHAR ? aAutoCharPool : aAutoParaPool;
    std::unique_ptr<SwStyleCache> &rpCache = eFamily == IStyleAccess::AUTO_STYLE_CHAR ? mpCharCache : mpParaCache;
    if( !rpCache )
        rpCache.reset(new SwStyleCache());
    std::shared_ptr<SfxItemSet> pStyle = rpCache->getByName( rName );
    if( !pStyle.get() )
    {
        // Ok, ok, it's allowed to ask for uncached styles (from UNO) but it should not be done
        // during loading a document
        OSL_FAIL( "Don't ask for uncached styles" );
        rpCache->addCompletePool( rAutoPool );
        pStyle = rpCache->getByName( rName );
    }
    return pStyle;
}

void SwStyleManager::getAllStyles( std::vector<std::shared_ptr<SfxItemSet>> &rStyles,
                                   IStyleAccess::SwAutoStyleFamily eFamily )
{
    StylePool& rAutoPool = eFamily == IStyleAccess::AUTO_STYLE_CHAR ? aAutoCharPool : aAutoParaPool;
    // setup <StylePool> iterator, which skips unused styles and ignorable items
    std::unique_ptr<IStylePoolIteratorAccess> pIter = rAutoPool.createIterator( true, true );
    std::shared_ptr<SfxItemSet> pStyle = pIter->getNext();
    while( pStyle.get() )
    {
        rStyles.push_back( pStyle );

        pStyle = pIter->getNext();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
