/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sot.hxx"

#define _SOT_OBJECT_CXX

#include <tools/debug.hxx>
#include <sot/object.hxx>
#include <sot/factory.hxx>
#include <sot/agg.hxx>

/************** class SotObject ******************************************/
class SotObjectFactory : public SotFactory
{
public:
        SotObjectFactory( const SvGlobalName & rName,
                              const String & rClassName,
                              CreateInstanceType pCreateFuncP )
            : SotFactory( rName, rClassName, pCreateFuncP )
        {}
};

SO2_IMPL_BASIC_CLASS_DLL(SotObject,SotObjectFactory,
                    SvGlobalName( 0xf44b7830, 0xf83c, 0x11d0,
                            0xaa, 0xa1, 0x0, 0xa0, 0x24, 0x9d, 0x55, 0x90 ) )
SO2_IMPL_INVARIANT(SotObject)

/*************************************************************************
|*  SotObject::TestMemberObjRef()
|*
|*  Beschreibung:
*************************************************************************/
void SotObject::TestMemberObjRef( sal_Bool /*bFree*/ )
{
}

/*************************************************************************
|*  SotObject::TestMemberObjRef()
|*
|*  Beschreibung:
*************************************************************************/
#ifdef TEST_INVARIANT
void SotObject::TestMemberInvariant( sal_Bool /*bPrint*/ )
{
}
#endif

/*************************************************************************
|*    SotObject::SotObject()
|*
|*    Beschreibung
*************************************************************************/
SotObject::SotObject()
    : nStrongLockCount( 0 )
    , nOwnerLockCount( 0 )
    , bOwner      ( sal_True )
    , bSVObject   ( sal_False )
    , bInClose    ( sal_False )
{
    SotFactory::IncSvObjectCount( this );
}

/*************************************************************************
|*
|*    SotObject::~SotObject()
|*
|*    Beschreibung
|*    Ersterstellung    MM 05.06.94
|*    Letzte Aenderung  MM 05.06.94
|*
*************************************************************************/
SotObject::~SotObject()
{
    SotFactory::DecSvObjectCount( this );
}

/*************************************************************************
|*    SotObject::GetInterface()
|*
|*    Beschreibung: Um so3 zu helfen
*************************************************************************/
IUnknown * SotObject::GetInterface( const SvGlobalName & )
{
    return NULL;
}

/*************************************************************************
|*    SotObject::CastAndAddRef()
|*
|*    Beschreibung
*************************************************************************/
void* SotObject::CastAndAddRef( const SotFactory * pFact )
{
    void * pCast = Cast( pFact );
    if( pCast )
        AddRef();
    return pCast;
}

//=========================================================================
sal_uInt16 SotObject::Lock( sal_Bool bLock )
{
    SotObjectRef xHoldAlive( this );
    sal_uInt16 nRet;
    if( bLock )
    {
        AddRef();
        nRet = ++nStrongLockCount;
    }
    else
    {
        nRet = --nStrongLockCount;
        ReleaseRef();
    }

    if( !nRet && !nOwnerLockCount )
        DoClose();

    return nRet;
}

//=========================================================================
void SotObject::OwnerLock
(
    sal_Bool bLock      /* sal_True, lock. sal_False, unlock. */
)
/*  [Beschreibung]

    Wenn der OwnerLock auf Null dekrementiert, dann wird die Methode
    DoClose gerufen. Dies geschieht unabh"angig vom Lock. bzw. RefCount.
    Ist der OwnerLock-Z"ahler != Null, dann wird kein DoClose durch
    <SotObject::FuzzyLock> gerufen.
*/
{
    if( bLock )
    {
        nOwnerLockCount++;
        AddRef();
    }
    else if ( nOwnerLockCount )
    {
        if( 0 == --nOwnerLockCount )
            DoClose();
        ReleaseRef();
    }
}

void SotObject::RemoveOwnerLock()
{
    if ( nOwnerLockCount )
    {
        --nOwnerLockCount;
        ReleaseRef();
    }
    else {
        DBG_ERROR("OwnerLockCount underflow!");
    }
}

//=========================================================================
sal_Bool SotObject::DoClose()
{
    sal_Bool bRet = sal_False;
    if( !bInClose )
    {
        SotObjectRef xHoldAlive( this );
        bInClose = sal_True;
        bRet = Close();
        bInClose = sal_False;
    }
    return bRet;
}

//=========================================================================
sal_Bool SotObject::Close()
{
    return sal_True;
}


