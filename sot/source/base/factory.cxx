/*************************************************************************
 *
 *  $RCSfile: factory.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:56:51 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#define _SOT_FACTORY_CXX
#define SOT_STRING_LIST

#include <factory.hxx>
#include <tools/debug.hxx>
#include <tools/string.hxx>
#include <object.hxx>
#include <dtrans.hxx>
#include <sotdata.hxx>
#pragma hdrstop

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
    , pSotDataObjectFactory( NULL )
    , pEmptyList( NULL )
    , pSotDataMemberObjectFactory( NULL )
    , pAtomList( NULL )
{
}

/*************************************************************************
|*    SOTDATA()
|*
|*    Beschreibung
*************************************************************************/
static SotData_Impl aData;
SotData_Impl * SOTDATA()
{
    return &aData;
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
        DBG_WARNING(  aStr.GetBuffer()  )

/*
        SotObjectList *pObjList = pSotData->pObjectList;

        if( pObjList )
        {
            SotObject * p = pObjList->First();
            while( p )
            {
                String aStr( "Factory: " );
                aStr += p->GetSvFactory()->GetClassName();
                aStr += " Count: ";
                aStr += p->GetRefCount();
                DBG_TRACE( "\tReferences:" );
                p->TestObjRef( FALSE );
#ifdef TEST_INVARIANT
                DBG_TRACE( "\tInvariant:" );
                p->TestInvariant( TRUE );
#endif
                p = pObjList->Next();
            }
        }
*/
#endif
        return;
    }

    // Muss von hinten nach vorne zerstoert werden. Das ist die umgekehrte
    // Reihenfolge der Erzeugung
    SotFactoryList* pFactoryList = pSotData->pFactoryList;
    if( pFactoryList )
    {
        SotFactory * pFact = pFactoryList->Last();
        while( NULL != (pFact = pFactoryList->Remove()) )
        {
            delete pFact;
            pFact = pFactoryList->Last();
        }
        delete pFactoryList;
        pSotData->pFactoryList = NULL;
    }

    delete pSotData->pObjectList;
    pSotData->pObjectList = NULL;
    delete pSotData->pEmptyList;
    pSotData->pEmptyList = NULL;
    if( pSotData->pAtomList )
    {

        ULONG nMax = pSotData->pAtomList->Count();
        for( ULONG i = 0; i < nMax; i++ )
            delete pSotData->pAtomList->GetObject( i );
        delete pSotData->pAtomList;
        pSotData->pAtomList = NULL;
    }
    //delete pSOTDATA();
    //SOTDATA() = NULL;
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
    , pCreateFunc   ( pCreateFuncP )
    , aClassName    ( rClassName )
    , nSuperCount   ( 0 )
    , pSuperClasses ( NULL )
{
#ifdef DBG_UTIL
    SvGlobalName aEmptyName;
    if( aEmptyName != *this )
    { // wegen Sfx-BasicFactories
    DBG_ASSERT( aEmptyName != *this, "create factory without SvGlobalName" )
    if( Find( *this ) )
    {
        /*
        String aStr( GetClassName() );
        aStr += ", UniqueName: ";
        aStr += GetHexName();
        aStr += ", create factories with the same unique name";
        DBG_ERROR( aStr );
        */
        DBG_ERROR( "create factories with the same unique name" );
    }
    }
#endif
    SotData_Impl * pSotData = SOTDATA();
    if( !pSotData->pFactoryList )
        pSotData->pFactoryList = new SotFactoryList();
    // muss nach hinten, wegen Reihenfolge beim zerstoeren
    pSotData->pFactoryList->Insert( this, LIST_APPEND );
}


//=========================================================================
SotFactory::~SotFactory()
{
    delete (void*)pSuperClasses;
}


/*************************************************************************
|*    SotFactory::
|*
|*    Beschreibung      Zugriffsmethoden auf SotData_Impl-Daten
*************************************************************************/
UINT32 SotFactory::GetSvObjectCount()
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
const SotFactory * SotFactory::Find( const SvGlobalName & rFactName )
{
    SvGlobalName aEmpty;
    SotData_Impl * pSotData = SOTDATA();
    if( rFactName != aEmpty && pSotData->pFactoryList )
    {
        SotFactory * pFact = pSotData->pFactoryList->First();
        while( pFact )
        {
            if( *pFact == rFactName )
                return pFact;
            pFact = pSotData->pFactoryList->Next();
        }
    }
    return NULL;
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
        delete (void *)pSuperClasses;
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
        ULONG nCount = pSotData->pObjectList->Count();
        for( ULONG i = 0; i < nCount ; i++ )
        {
            pSotData->pObjectList->GetObject( i )->TestInvariant( FALSE );
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

//=========================================================================
void * SotFactory::AggCastAndAddRef
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
    Hinzu kommt noch, dass ein Objekt aus meheren c++ Objekten
    zusammengesetzt sein kann. Diese Methode sucht nach einem
    passenden Objekt.

    [R"uckgabewert]

    void *,     NULL, pObj war NULL oder das Objekt war nicht vom Typ
                der Factory.
                Ansonsten wird pObj zuerst auf den Typ der Factory
                gecastet und dann auf void *.

    [Querverweise]

    <SvObject::AggCast>
*/
{
    void * pRet = NULL;
    if( pObj )
    {
        pRet = pObj->AggCast( this );
        if( pRet )
            pObj->AddRef();
    }
    return pRet;
}

/*************************************************************************
|*    SotFactory::Is()
|*
|*    Beschreibung
*************************************************************************/
BOOL SotFactory::Is( const SotFactory * pSuperCl ) const
{
    if( this == pSuperCl )
        return TRUE;

    for( USHORT i = 0; i < nSuperCount; i++ )
    {
        if( pSuperClasses[ i ]->Is( pSuperCl ) )
            return TRUE;
    }
    return FALSE;
}




