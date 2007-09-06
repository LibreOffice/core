/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: swstylemanager.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2007-09-06 14:00:34 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#ifdef _MSC_VER
#pragma hdrstop
#endif

#include "swstylemanager.hxx"
#include <hash_map>
#include <svtools/stylepool.hxx>

#ifndef _DOC_HXX
#include <doc.hxx>
#endif

#ifndef _CHARFMT_HXX
#include <charfmt.hxx>
#endif

#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif

#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif

#ifndef _ISTYLEACCESS_HXX
#include <istyleaccess.hxx>
#endif

typedef ::std::hash_map< const ::rtl::OUString,
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
    SwStyleManager() : mpCharCache(0), mpParaCache(0) {}
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

IStyleAccess *createStyleManager()
{
    return new SwStyleManager();
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
        ASSERT( false, "Don't ask for uncached styles" );
        rpCache->addCompletePool( rAutoPool );
        pStyle = rpCache->getByName( rName );
    }
    return pStyle;
}

void SwStyleManager::getAllStyles( std::vector<StylePool::SfxItemSet_Pointer_t> &rStyles,
                                   IStyleAccess::SwAutoStyleFamily eFamily )
{
    StylePool& rAutoPool = eFamily == IStyleAccess::AUTO_STYLE_CHAR ? aAutoCharPool : aAutoParaPool;
    IStylePoolIteratorAccess *pIter = rAutoPool.createIterator();
    StylePool::SfxItemSet_Pointer_t pStyle = pIter->getNext();
    while( pStyle.get() )
    {
        // Do not consider styles which aren't used.
        if ( pStyle.use_count() > 2 )
            rStyles.push_back( pStyle );
        pStyle = pIter->getNext();
    }
    delete pIter;
}
