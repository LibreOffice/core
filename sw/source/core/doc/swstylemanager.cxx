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

typedef std::unordered_map< OUString,
                            StylePool::SfxItemSet_Pointer_t,
                            OUStringHash,
                            std::equal_to< OUString > > SwStyleNameCache;

class SwStyleCache
{
    SwStyleNameCache mMap;
public:
    SwStyleCache() {}
    void addStyleName( StylePool::SfxItemSet_Pointer_t pStyle )
        { mMap[ StylePool::nameOf(pStyle) ] = pStyle; }
    void addCompletePool( StylePool& rPool );
    StylePool::SfxItemSet_Pointer_t getByName( const OUString& rName ) { return mMap[rName]; }
};

void SwStyleCache::addCompletePool( StylePool& rPool )
{
    IStylePoolIteratorAccess *pIter = rPool.createIterator();
    StylePool::SfxItemSet_Pointer_t pStyle = pIter->getNext();
    while( pStyle.get() )
    {
        OUString aName( StylePool::nameOf(pStyle) );
        mMap[ aName ] = pStyle;
        pStyle = pIter->getNext();
    }
    delete pIter;
}

class SwStyleManager : public IStyleAccess
{
    StylePool aAutoCharPool;
    StylePool aAutoParaPool;
    SwStyleCache *mpCharCache;
    SwStyleCache *mpParaCache;

public:
    // accept empty item set for ignorable paragraph items.
    explicit SwStyleManager( SfxItemSet* pIgnorableParagraphItems )
        : aAutoCharPool(),
          aAutoParaPool( pIgnorableParagraphItems ),
          mpCharCache(0),
          mpParaCache(0)
    {}
    virtual ~SwStyleManager();
    virtual StylePool::SfxItemSet_Pointer_t getAutomaticStyle( const SfxItemSet& rSet,
                                                               IStyleAccess::SwAutoStyleFamily eFamily ) override;
    virtual StylePool::SfxItemSet_Pointer_t getByName( const OUString& rName,
                                                               IStyleAccess::SwAutoStyleFamily eFamily ) override;
    virtual void getAllStyles( std::vector<StylePool::SfxItemSet_Pointer_t> &rStyles,
                                                               IStyleAccess::SwAutoStyleFamily eFamily ) override;
    virtual StylePool::SfxItemSet_Pointer_t cacheAutomaticStyle( const SfxItemSet& rSet,
                                                               SwAutoStyleFamily eFamily ) override;
    virtual void clearCaches() override;
};

IStyleAccess *createStyleManager( SfxItemSet* pIgnorableParagraphItems )
{
    return new SwStyleManager( pIgnorableParagraphItems );
}

SwStyleManager::~SwStyleManager()
{
    delete mpCharCache;
    delete mpParaCache;
}

void SwStyleManager::clearCaches()
{
    delete mpCharCache;
    mpCharCache = 0;
    delete mpParaCache;
    mpParaCache = 0;
}

StylePool::SfxItemSet_Pointer_t SwStyleManager::getAutomaticStyle( const SfxItemSet& rSet,
                                                                   IStyleAccess::SwAutoStyleFamily eFamily )
{
    StylePool& rAutoPool = eFamily == IStyleAccess::AUTO_STYLE_CHAR ? aAutoCharPool : aAutoParaPool;
    return rAutoPool.insertItemSet( rSet );
}

StylePool::SfxItemSet_Pointer_t SwStyleManager::cacheAutomaticStyle( const SfxItemSet& rSet,
                                                                   IStyleAccess::SwAutoStyleFamily eFamily )
{
    StylePool& rAutoPool = eFamily == IStyleAccess::AUTO_STYLE_CHAR ? aAutoCharPool : aAutoParaPool;
    StylePool::SfxItemSet_Pointer_t pStyle = rAutoPool.insertItemSet( rSet );
    SwStyleCache* &rpCache = eFamily == IStyleAccess::AUTO_STYLE_CHAR ?
                             mpCharCache : mpParaCache;
    if( !rpCache )
        rpCache = new SwStyleCache();
    rpCache->addStyleName( pStyle );
    return pStyle;
}

StylePool::SfxItemSet_Pointer_t SwStyleManager::getByName( const OUString& rName,
                                                           IStyleAccess::SwAutoStyleFamily eFamily )
{
    StylePool& rAutoPool = eFamily == IStyleAccess::AUTO_STYLE_CHAR ? aAutoCharPool : aAutoParaPool;
    SwStyleCache* &rpCache = eFamily == IStyleAccess::AUTO_STYLE_CHAR ? mpCharCache : mpParaCache;
    if( !rpCache )
        rpCache = new SwStyleCache();
    StylePool::SfxItemSet_Pointer_t pStyle = rpCache->getByName( rName );
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

void SwStyleManager::getAllStyles( std::vector<StylePool::SfxItemSet_Pointer_t> &rStyles,
                                   IStyleAccess::SwAutoStyleFamily eFamily )
{
    StylePool& rAutoPool = eFamily == IStyleAccess::AUTO_STYLE_CHAR ? aAutoCharPool : aAutoParaPool;
    // setup <StylePool> iterator, which skips unused styles and ignorable items
    IStylePoolIteratorAccess *pIter = rAutoPool.createIterator( true, true );
    StylePool::SfxItemSet_Pointer_t pStyle = pIter->getNext();
    while( pStyle.get() )
    {
        rStyles.push_back( pStyle );

        pStyle = pIter->getNext();
    }
    delete pIter;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
