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
#include "precompiled_sot.hxx"

#define _SOT_FACTORY_CXX
#define SOT_STRING_LIST

#include <sot/factory.hxx>
#include <tools/debug.hxx>
#include <tools/string.hxx>
#include <sot/object.hxx>
#include <sot/sotdata.hxx>
#include <sot/clsids.hxx>
#include <rtl/instance.hxx>
#include <com/sun/star/datatransfer/DataFlavor.hpp>

/************** class SotData_Impl *********************************************/
/*************************************************************************
|*    SotData_Impl::SotData_Impl
|*
|*    Beschreibung
*************************************************************************/
SotData_Impl::SotData_Impl()
    : nSvObjCount( 0 )
    , pObjectList( NULL )
    , pFactoryList( NULL )
    , pSotObjectFactory( NULL )
    , pSotStorageStreamFactory( NULL )
    , pSotStorageFactory( NULL )
    , pDataFlavorList( NULL )
{
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

/*************************************************************************
|*    SotFactory::DeInit()
|*
|*    Beschreibung
*************************************************************************/
void SotFactory::DeInit()
{
    SotData_Impl * pSotData = SOTDATA();

    if( pSotData->nSvObjCount )
    {
#ifdef DBG_UTIL
        ByteString aStr( "Objects alive: " );
        aStr.Append( ByteString::CreateFromInt32( pSotData->nSvObjCount ) );
        DBG_WARNING(  aStr.GetBuffer()  );
#endif
        return;
    }

    // Muss von hinten nach vorne zerstoert werden. Das ist die umgekehrte
    // Reihenfolge der Erzeugung
    SotFactoryList* pFactoryList = pSotData->pFactoryList;
    if( pFactoryList )
    {
        for ( size_t i = pFactoryList->size(); i > 0 ; )
            delete (*pFactoryList)[ --i ];
        pFactoryList->clear();
        delete pFactoryList;
        pSotData->pFactoryList = NULL;
    }

    delete pSotData->pObjectList;
    pSotData->pObjectList = NULL;
    if( pSotData->pDataFlavorList )
    {

        for( sal_uLong i = 0, nMax = pSotData->pDataFlavorList->Count(); i < nMax; i++ )
            delete (::com::sun::star::datatransfer::DataFlavor*) pSotData->pDataFlavorList->GetObject( i );
        delete pSotData->pDataFlavorList;
        pSotData->pDataFlavorList = NULL;
    }
}


/************** class SotFactory *****************************************/
/*************************************************************************
|*    SotFactory::SotFactory()
|*
|*    Beschreibung
*************************************************************************/
TYPEINIT0(SotFactory);

SotFactory::SotFactory( const SvGlobalName & rName,
                      const String & rClassName,
                      CreateInstanceType pCreateFuncP )
    : SvGlobalName  ( rName )
    , nSuperCount   ( 0 )
    , pSuperClasses ( NULL )
    , pCreateFunc   ( pCreateFuncP )
    , aClassName    ( rClassName )
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


//=========================================================================
SotFactory::~SotFactory()
{
    delete [] pSuperClasses;
}


/*************************************************************************
|*    SotFactory::
|*
|*    Beschreibung      Zugriffsmethoden auf SotData_Impl-Daten
*************************************************************************/
sal_uInt32 SotFactory::GetSvObjectCount()
{
    return SOTDATA()->nSvObjCount;
}


const SotFactoryList * SotFactory::GetFactoryList()
{
    return SOTDATA()->pFactoryList;
}

/*************************************************************************
|*    SotFactory::Find()
|*
|*    Beschreibung
*************************************************************************/
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

    return 0;
}

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
        memcpy( (void *)pTmp, (void *)pSuperClasses,
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
    if( !pSotData->pObjectList )
        pSotData->pObjectList = new SotObjectList();
    if( pObj )
        pSotData->pObjectList->Insert( pObj );
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
        pSotData->pObjectList->Remove( pObj );
    if( !pSotData->nSvObjCount )
    {
        //keine internen und externen Referenzen mehr
    }
}


/*************************************************************************
|*    SotFactory::TestInvariant()
|*
|*    Beschreibung
*************************************************************************/
void SotFactory::TestInvariant()
{
#ifdef TEST_INVARIANT
    SotData_Impl * pSotData = SOTDATA();
    if( pSotData->pObjectList )
    {
        sal_uLong nCount = pSotData->pObjectList->Count();
        for( sal_uLong i = 0; i < nCount ; i++ )
        {
            pSotData->pObjectList->GetObject( i )->TestInvariant( sal_False );
        }
    }
#endif
}

/*************************************************************************
|*    SotFactory::CreateInstance()
|*
|*    Beschreibung
*************************************************************************/
void * SotFactory::CreateInstance( SotObject ** ppObj ) const
{
    DBG_ASSERT( pCreateFunc, "SotFactory::CreateInstance: pCreateFunc == 0" );
    return pCreateFunc( ppObj );
}

//=========================================================================
void * SotFactory::CastAndAddRef
(
    SotObject * pObj /* Das Objekt von dem der Typ gepr"uft wird. */
) const
/*  [Beschreibung]

    Ist eine Optimierung, damit die Ref-Klassen k"urzer implementiert
    werden k"onnen. pObj wird auf den Typ der Factory gecastet.
    In c++ (wenn es immer erlaubt w"are) w"urde der void * wie im
    Beispiel gebildet.
    Factory der Klasse SvPersist.
    void * p = (void *)(SvPersist *)pObj;

    [R"uckgabewert]

    void *,     NULL, pObj war NULL oder das Objekt war nicht vom Typ
                der Factory.
                Ansonsten wird pObj zuerst auf den Typ der Factory
                gecastet und dann auf void *.

    [Querverweise]

    <SotObject::CastAndAddRef>
*/
{
    return pObj ? pObj->CastAndAddRef( this ) : NULL;
}

/*************************************************************************
|*    SotFactory::Is()
|*
|*    Beschreibung
*************************************************************************/
sal_Bool SotFactory::Is( const SotFactory * pSuperCl ) const
{
    if( this == pSuperCl )
        return sal_True;

    for( sal_uInt16 i = 0; i < nSuperCount; i++ )
    {
        if( pSuperClasses[ i ]->Is( pSuperCl ) )
            return sal_True;
    }
    return sal_False;
}




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
