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

#include <sot/factory.hxx>
#include <tools/debug.hxx>
#include <sot/object.hxx>
#include <sot/sotdata.hxx>
#include <comphelper/classids.hxx>
#include <osl/diagnose.h>
#include <rtl/instance.hxx>
#include <rtl/strbuf.hxx>

/************** class SotData_Impl *********************************************/
/*************************************************************************
|*    SotData_Impl::SotData_Impl
|*
|*    Beschreibung
*************************************************************************/
SotData_Impl::SotData_Impl()
    : nSvObjCount( 0 )
    , pFactoryList( nullptr )
    , pSotObjectFactory( nullptr )
    , pSotStorageStreamFactory( nullptr )
    , pSotStorageFactory( nullptr )
    , pDataFlavorList( nullptr )
{
}

SotData_Impl::~SotData_Impl()
{
    if (pDataFlavorList)
    {
        for( tDataFlavorList::iterator aI = pDataFlavorList->begin(),
             aEnd = pDataFlavorList->end(); aI != aEnd; ++aI)
        {
            delete *aI;
        }
        delete pDataFlavorList;
    }
    delete pFactoryList;
}

/*************************************************************************
|*    SOTDATA()
|*
|*    Beschreibung
*************************************************************************/
namespace { struct ImplData : public rtl::Static<SotData_Impl, ImplData> {}; }
SotData_Impl * SOTDATA()
{
    return &ImplData::get();
}

/************** class SotFactory *****************************************/
/*************************************************************************
|*    SotFactory::SotFactory()
|*
|*    Beschreibung
*************************************************************************/

SotFactory::SotFactory( const SvGlobalName & rName )
    : SvGlobalName  ( rName )
    , nSuperCount   ( 0 )
    , pSuperClasses ( nullptr )
{
#ifdef DBG_UTIL
    SvGlobalName aEmptyName;
    if( aEmptyName != *this )
    { // wegen Sfx-BasicFactories
    DBG_ASSERT( aEmptyName != *this, "create factory without SvGlobalName" );
    if( Find( *this ) )
    {
        OSL_FAIL( "create factories with the same unique name" );
    }
    }
#endif
    SotData_Impl * pSotData = SOTDATA();
    if( !pSotData->pFactoryList )
        pSotData->pFactoryList = new SotFactoryList();
    // muss nach hinten, wegen Reihenfolge beim zerstoeren
    pSotData->pFactoryList->push_back( this );
}



SotFactory::~SotFactory()
{
    delete [] pSuperClasses;
}

/*************************************************************************
|*    SotFactory::Find()
|*
|*    Beschreibung
*************************************************************************/
#ifdef DBG_UTIL
const SotFactory* SotFactory::Find( const SvGlobalName & rFactName )
{
    SvGlobalName aEmpty;
    SotData_Impl * pSotData = SOTDATA();
    if( rFactName != aEmpty && pSotData->pFactoryList )
    {
        for ( size_t i = 0, n = pSotData->pFactoryList->size(); i < n; ++i ) {
            SotFactory* pFact = (*pSotData->pFactoryList)[ i ];
            if( *pFact == rFactName )
                return pFact;
        }
    }

    return nullptr;
}
#endif

/*************************************************************************
|*    SotFactory::PutSuperClass()
|*
|*    Beschreibung
*************************************************************************/
void SotFactory::PutSuperClass( const SotFactory * pFact )
{
    nSuperCount++;
    if( !pSuperClasses )
        pSuperClasses = new const SotFactory * [ nSuperCount ];
    else
    {
        const SotFactory ** pTmp = new const SotFactory * [ nSuperCount ];
        memcpy( static_cast<void *>(pTmp), static_cast<void *>(pSuperClasses),
                sizeof( void * ) * (nSuperCount -1) );
        delete [] pSuperClasses;
        pSuperClasses = pTmp;
    }
    pSuperClasses[ nSuperCount -1 ] = pFact;
}


/*************************************************************************
|*    SotFactory::IncSvObjectCount()
|*
|*    Beschreibung
*************************************************************************/
void SotFactory::IncSvObjectCount( SotObject * pObj )
{
    SotData_Impl * pSotData = SOTDATA();
    pSotData->nSvObjCount++;

    if( pObj )
        pSotData->aObjectList.push_back( pObj );
}


/*************************************************************************
|*    SotFactory::DecSvObjectCount()
|*
|*    Beschreibung
*************************************************************************/
void SotFactory::DecSvObjectCount( SotObject * pObj )
{
    SotData_Impl * pSotData = SOTDATA();
    pSotData->nSvObjCount--;
    if( pObj )
        pSotData->aObjectList.remove( pObj );
    if( !pSotData->nSvObjCount )
    {
        //keine internen und externen Referenzen mehr
    }
}

/*************************************************************************
|*    SotFactory::Is()
|*
|*    Beschreibung
*************************************************************************/
bool SotFactory::Is( const SotFactory * pSuperCl ) const
{
    if( this == pSuperCl )
        return true;

    for( sal_uInt16 i = 0; i < nSuperCount; i++ )
    {
        if( pSuperClasses[ i ]->Is( pSuperCl ) )
            return true;
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
