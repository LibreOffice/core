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

#define _SOT_OBJECT_CXX

#include <tools/debug.hxx>
#include <sot/object.hxx>
#include <sot/factory.hxx>
#include <sot/agg.hxx>

/************** class SotObject ******************************************/
class SotObjectFactory : public SotFactory
{
public:
         TYPEINFO();
        SotObjectFactory( const SvGlobalName & rName,
                              const String & rClassName,
                              CreateInstanceType pCreateFuncP )
            : SotFactory( rName, rClassName, pCreateFuncP )
        {}
};
TYPEINIT1(SotObjectFactory,SotFactory);


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
        OSL_FAIL("OwnerLockCount underflow!");
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
