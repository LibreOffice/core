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

#include <sot/object.hxx>
#include <sot/factory.hxx>

/************** class SotObject ******************************************/
class SotObjectFactory : public SotFactory
{
public:
    explicit SotObjectFactory(const SvGlobalName& rName)
        : SotFactory( rName )
    {
    }
};

SotFactory * SotObject::ClassFactory()
{
    SotFactory **ppFactory = GetFactoryAdress();
    if( !*ppFactory )
    {
        *ppFactory = new SotObjectFactory(
                           SvGlobalName( 0xf44b7830, 0xf83c, 0x11d0,
                               0xaa, 0xa1, 0x0, 0xa0, 0x24, 0x9d, 0x55, 0x90 ) );
    }
    return *ppFactory;
}

void * SotObject::Cast( const SotFactory * pFact )
{
    void * pRet = NULL;
    if( !pFact || pFact == ClassFactory() )
        pRet = this;
    return pRet;
}

/*************************************************************************
|*    SotObject::SotObject()
|*
|*    Beschreibung
*************************************************************************/
SotObject::SotObject()
    : nOwnerLockCount( 0 )
    , bOwner      ( true )
    , bSVObject   ( false )
    , bInClose    ( false )
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

void SotObject::OwnerLock
(
    bool bLock      /* true, lock. false, unlock. */
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
        AddFirstRef();
    }
    else if ( nOwnerLockCount )
    {
        if( 0 == --nOwnerLockCount )
            DoClose();
        ReleaseRef();
    }
}


bool SotObject::DoClose()
{
    bool bRet = false;
    if( !bInClose )
    {
        tools::SvRef<SotObject> xHoldAlive( this );
        bInClose = true;
        bRet = Close();
        bInClose = false;
    }
    return bRet;
}


bool SotObject::Close()
{
    return true;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
