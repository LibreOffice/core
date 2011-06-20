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
#include "precompiled_sw.hxx"


#include "swstylemanager.hxx"
#include <boost/unordered_map.hpp>
#include <svl/stylepool.hxx>
#include <doc.hxx>
#include <charfmt.hxx>
#include <docary.hxx>
#include <swtypes.hxx>
#include <istyleaccess.hxx>

typedef ::boost::unordered_map< const ::rtl::OUString,
                         StylePool::SfxItemSet_Pointer_t,
                         ::rtl::OUStringHash,
                         ::std::equal_to< ::rtl::OUString > > SwStyleNameCache;

class SwStyleCache
{
    SwStyleNameCache mMap;
public:
    SwStyleCache() {}
    void addStyleName( StylePool::SfxItemSet_Pointer_t pStyle )
        { mMap[ StylePool::nameOf(pStyle) ] = pStyle; }
    void addCompletePool( StylePool& rPool );
    StylePool::SfxItemSet_Pointer_t getByName( const rtl::OUString& rName ) { return mMap[rName]; }
};

void SwStyleCache::addCompletePool( StylePool& rPool )
{
    IStylePoolIteratorAccess *pIter = rPool.createIterator();
    StylePool::SfxItemSet_Pointer_t pStyle = pIter->getNext();
    while( pStyle.get() )
    {
        rtl::OUString aName( StylePool::nameOf(pStyle) );
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
    SwStyleManager( SfxItemSet* pIgnorableParagraphItems )
        : aAutoCharPool(),
          aAutoParaPool( pIgnorableParagraphItems ),
          mpCharCache(0),
          mpParaCache(0)
    {}
    virtual ~SwStyleManager();
    virtual StylePool::SfxItemSet_Pointer_t getAutomaticStyle( const SfxItemSet& rSet,
                                                               IStyleAccess::SwAutoStyleFamily eFamily );
    virtual StylePool::SfxItemSet_Pointer_t getByName( const rtl::OUString& rName,
                                                               IStyleAccess::SwAutoStyleFamily eFamily );
    virtual void getAllStyles( std::vector<StylePool::SfxItemSet_Pointer_t> &rStyles,
                                                               IStyleAccess::SwAutoStyleFamily eFamily );
    virtual StylePool::SfxItemSet_Pointer_t cacheAutomaticStyle( const SfxItemSet& rSet,
                                                               SwAutoStyleFamily eFamily );
    virtual void clearCaches();
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

StylePool::SfxItemSet_Pointer_t SwStyleManager::getByName( const rtl::OUString& rName,
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
