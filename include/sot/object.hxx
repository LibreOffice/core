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

#ifndef INCLUDED_SOT_OBJECT_HXX
#define INCLUDED_SOT_OBJECT_HXX

#include <sot/sotdata.hxx>
#include <tools/globname.hxx>
#include <tools/ref.hxx>
#include <sot/sotdllapi.h>

#define SO2_IMPL_BASIC_CLASS_DLL(ClassName,FactoryName,GlobalName)        \
SotFactory * ClassName::ClassFactory()                                     \
{                                                                         \
    SotFactory **ppFactory = GetFactoryAdress();                           \
    if( !*ppFactory )                                                     \
    {                                                                     \
        *ppFactory = new FactoryName( GlobalName,                         \
            OUString( #ClassName ), ClassName::CreateInstance );     \
    }                                                                     \
    return *ppFactory;                                                    \
}                                                                         \
void * ClassName::CreateInstance( SotObject ** ppObj )            \
{                                                                         \
    ClassName * p = new ClassName();                                      \
    if( ppObj )                                                           \
        *ppObj = p;                                                       \
    return p;                                                             \
}                                                                         \
const SotFactory * ClassName::GetSvFactory() const                         \
{                                                                         \
    return ClassFactory();                                                \
}                                                                         \
void * ClassName::Cast( const SotFactory * pFact )                         \
{                                                                         \
    void * pRet = NULL;                                                   \
    if( !pFact || pFact == ClassFactory() )                               \
        pRet = this;                                                      \
    return pRet;                                                          \
}

#define SO2_IMPL_BASIC_CLASS1_DLL(ClassName,FactoryName,Super1,GlobalName)\
SotFactory * ClassName::ClassFactory()                                     \
{                                                                         \
    SotFactory **ppFactory = GetFactoryAdress();                           \
    if( !*ppFactory )                                                     \
    {                                                                     \
        *ppFactory = new FactoryName( GlobalName,                         \
            OUString( #ClassName ), ClassName::CreateInstance );     \
        (*ppFactory)->PutSuperClass( Super1::ClassFactory() );            \
    }                                                                     \
    return *ppFactory;                                                    \
}                                                                         \
void * ClassName::CreateInstance( SotObject ** ppObj )            \
{                                                                         \
    ClassName * p = new ClassName();                                      \
    Super1* pSuper1 = p;                                                  \
    SotObject* pBasicObj = pSuper1;                                        \
    if( ppObj )                                                           \
        *ppObj = pBasicObj;                                               \
    return p;                                                             \
}                                                                         \
const SotFactory * ClassName::GetSvFactory() const                \
{                                                                         \
    return ClassFactory();                                                \
}                                                                         \
void * ClassName::Cast( const SotFactory * pFact )                \
{                                                                         \
    void * pRet = NULL;                                                   \
    if( !pFact || pFact == ClassFactory() )                               \
        pRet = this;                                                      \
    if( !pRet )                                                           \
        pRet = Super1::Cast( pFact );                                     \
    return pRet;                                                          \
}

struct IUnknown;
class SOT_DLLPUBLIC SotObject : virtual public SvRefBase
{
friend class SotFactory;
    sal_uInt16  nOwnerLockCount;
    bool        bOwner;
    bool        bSVObject;        // is proxy, then TRUE if other side is SV
    bool        bInClose;         // TRUE, in DoClose

protected:
    virtual             ~SotObject();
    void                SetExtern() { bOwner = false; }
    virtual bool        Close();
public:
                        SotObject();

private:
    static SotFactory **       GetFactoryAdress()
                              { return &(SOTDATA()->pSotObjectFactory); }
public:
    static void *              CreateInstance( SotObject ** = NULL );
    static SotFactory *        ClassFactory();
    virtual const SotFactory * GetSvFactory() const;
    virtual void *             Cast( const SotFactory * );

    bool                Owner() const { return bOwner; }

    sal_uInt16          GetOwnerLockCount() const { return nOwnerLockCount; }

    void                OwnerLock( bool bLock );
    bool                DoClose();
    bool                IsInClose() const { return bInClose; }

private:
    SotObject & operator = ( const SotObject & ) SAL_DELETED_FUNCTION;
    SotObject( const SotObject & ) SAL_DELETED_FUNCTION;
};

//==================class SotObjectRef======================================
typedef tools::SvRef<SotObject> SotObjectRef;

#endif // _IFACE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
