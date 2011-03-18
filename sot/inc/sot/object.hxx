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

#ifndef _SOT_OBJECT_HXX
#define _SOT_OBJECT_HXX

#include <sot/sotref.hxx>
#include <sot/sotdata.hxx>
#include <tools/globname.hxx>
#include "sot/sotdllapi.h"

/*************************************************************************
*************************************************************************/

#define TEST_INVARIANT
#ifdef TEST_INVARIANT
#define SO2_DECL_INVARIANT()                                            \
        virtual void TestObjRef( sal_Bool bFree );                          \
        void         TestMemberObjRef( sal_Bool bFree );                    \
        virtual void TestInvariant( sal_Bool bPrint );                      \
        void         TestMemberInvariant( sal_Bool bPrint );

#define SO2_IMPL_INVARIANT(ClassName)                                   \
void ClassName::TestObjRef( sal_Bool bFree )                       \
{                                                                       \
    TestMemberObjRef( bFree );                                          \
}                                                                       \
void ClassName::TestInvariant( sal_Bool bPrint )                   \
{                                                                       \
    TestMemberInvariant( bPrint );                                      \
}

#define SO2_IMPL_INVARIANT1(ClassName,Super1)                           \
void ClassName::TestObjRef( sal_Bool bFree )                       \
{                                                                       \
    TestMemberObjRef( bFree );                                          \
    Super1::TestObjRef( bFree );                                        \
}                                                                       \
void ClassName::TestInvariant( sal_Bool bPrint )                   \
{                                                                       \
    TestMemberInvariant( bPrint );                                      \
    Super1::TestInvariant( bPrint );                                    \
}

#define SO2_IMPL_INVARIANT2(ClassName,Super1,Super2)                    \
void ClassName::TestObjRef( sal_Bool bFree )                       \
{                                                                       \
    TestMemberObjRef( bFree );                                          \
    Super1::TestObjRef( bFree );                                        \
    Super2::TestObjRef( bFree );                                        \
}                                                                       \
void ClassName::TestInvariant( sal_Bool bPrint )                   \
{                                                                       \
    TestMemberInvariant( bPrint );                                      \
    Super1::TestInvariant( bPrint );                                    \
    Super2::TestInvariant( bPrint );                                    \
}

#define SO2_IMPL_INVARIANT3(ClassName,Super1,Super2,Super3)             \
void ClassName::TestObjRef( sal_Bool bFree )                       \
{                                                                       \
    TestMemberObjRef( bFree );                                          \
    Super1::TestObjRef( bFree );                                        \
    Super2::TestObjRef( bFree );                                        \
    Super3::TestObjRef( bFree );                                        \
}                                                                       \
void ClassName::TestInvariant( sal_Bool bPrint )                   \
{                                                                       \
    TestMemberInvariant( bPrint );                                      \
    Super1::TestInvariant( bPrint );                                    \
    Super2::TestInvariant( bPrint );                                    \
    Super3::TestInvariant( bPrint );                                    \
}

#define SO2_IMPL_INVARIANT4(ClassName,Super1,Super2,Super3,Super4)      \
void ClassName::TestObjRef( sal_Bool bFree )                       \
{                                                                       \
    TestMemberObjRef( bFree );                                          \
    Super1::TestObjRef( bFree );                                        \
    Super2::TestObjRef( bFree );                                        \
    Super3::TestObjRef( bFree );                                        \
    Super4::TestObjRef( bFree );                                        \
}                                                                       \
void ClassName::TestInvariant( sal_Bool bPrint )                   \
{                                                                       \
    TestMemberInvariant( bPrint );                                      \
    Super1::TestInvariant( bPrint );                                    \
    Super2::TestInvariant( bPrint );                                    \
    Super3::TestInvariant( bPrint );                                    \
    Super4::TestInvariant( bPrint );                                    \
}

#ifdef DBG_UTIL
#define CALL_TEST_INVARIANT() SotFactory::TestInvariant()
#else
#define CALL_TEST_INVARIANT()
#endif  // DBG_UTIL

#else   // TEST_INVARIANT

#define SO2_DECL_INVARIANT()

#define SO2_IMPL_INVARIANT(ClassName)
#define SO2_IMPL_INVARIANT1(ClassName,Super1)
#define SO2_IMPL_INVARIANT2(ClassName,Super1,Super2)
#define SO2_IMPL_INVARIANT3(ClassName,Super1,Super2,Super3)
#define SO2_IMPL_INVARIANT4(ClassName,Super1,Super2,Super3,Super4)

#define CALL_TEST_INVARIANT()

#endif  // TEST_INVARIANT

/**************************************************************************
**************************************************************************/
#define SO2_DECL_BASIC_CLASS_DLL(ClassName,FacName)                       \
private:                                                                  \
    static SotFactory **       GetFactoryAdress()                          \
                              { return &(FacName->p##ClassName##Factory); } \
public:                                                                   \
    static void *             CreateInstance( SotObject ** = NULL );       \
    static SotFactory *        ClassFactory();                             \
    virtual const SotFactory * GetSvFactory() const;                       \
    virtual void *            Cast( const SotFactory * );

#define SO2_DECL_BASIC_CLASS(ClassName)                                   \
private:                                                                  \
    static SotFactory *        pFactory;                                   \
    static SotFactory **       GetFactoryAdress() { return &pFactory; }    \
public:                                                                   \
    static void *             CreateInstance( SotObject ** = NULL );       \
    static SotFactory *        ClassFactory();                             \
    virtual const SotFactory * GetSvFactory() const;                       \
    virtual void *            Cast( const SotFactory * );

/**************************************************************************
**************************************************************************/
#define SO2_IMPL_BASIC_CLASS_DLL(ClassName,FactoryName,GlobalName)        \
SotFactory * ClassName::ClassFactory()                                     \
{                                                                         \
    SotFactory **ppFactory = GetFactoryAdress();                           \
    if( !*ppFactory )                                                     \
    {                                                                     \
        *ppFactory = new FactoryName( GlobalName,                         \
            String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( #ClassName ) ), \
                                 ClassName::CreateInstance );             \
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

#define SO2_IMPL_BASIC_CLASS(ClassName,FactoryName,GlobalName)                        \
SotFactory * ClassName::pFactory = NULL;                                   \
    SO2_IMPL_BASIC_CLASS_DLL(ClassName,FactoryName,GlobalName)

/**************************************************************************
**************************************************************************/
#define SO2_IMPL_BASIC_CLASS1_DLL(ClassName,FactoryName,Super1,GlobalName)\
SotFactory * ClassName::ClassFactory()                                     \
{                                                                         \
    SotFactory **ppFactory = GetFactoryAdress();                           \
    if( !*ppFactory )                                                     \
    {                                                                     \
        *ppFactory = new FactoryName( GlobalName,                         \
            String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( #ClassName ) ), \
                                ClassName::CreateInstance );                \
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

#define SO2_IMPL_BASIC_CLASS1(ClassName,FactoryName,Super1,GlobalName)    \
SotFactory * ClassName::pFactory = NULL;                                   \
    SO2_IMPL_BASIC_CLASS1_DLL(ClassName,FactoryName,Super1,GlobalName)

/**************************************************************************
**************************************************************************/
#define SO2_IMPL_BASIC_CLASS2_DLL(ClassName,FactoryName,Super1,Super2,GlobalName)  \
SotFactory * ClassName::ClassFactory()                                     \
{                                                                         \
    SotFactory **ppFactory = GetFactoryAdress();                           \
    if( !*ppFactory )                                                     \
    {                                                                     \
        *ppFactory = new FactoryName( GlobalName,                         \
            String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( #ClassName ) ), \
                                 ClassName::CreateInstance );             \
        (*ppFactory)->PutSuperClass( Super1::ClassFactory() );            \
        (*ppFactory)->PutSuperClass( Super2::ClassFactory() );            \
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
    if( !pRet )                                                           \
        pRet = Super2::Cast( pFact );                                     \
    return pRet;                                                          \
}
#define SO2_IMPL_BASIC_CLASS2(ClassName,FactoryName,Super1,Super2,GlobalName)  \
SotFactory * ClassName::pFactory = NULL;                                   \
    SO2_IMPL_BASIC_CLASS2_DLL(ClassName,FactoryName,Super1,Super2,GlobalName)

/**************************************************************************
**************************************************************************/
#define SO2_IMPL_BASIC_CLASS3_DLL(ClassName,FactoryName,Super1,Super2,Super3,GlobalName)  \
SotFactory * ClassName::ClassFactory()                                     \
{                                                                         \
    SotFactory **ppFactory = GetFactoryAdress();                           \
    if( !*ppFactory )                                                     \
    {                                                                     \
        *ppFactory = new FactoryName( GlobalName,                         \
            String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( #ClassName ) ), \
                                 ClassName::CreateInstance );\
        (*ppFactory)->PutSuperClass( Super1::ClassFactory() );            \
        (*ppFactory)->PutSuperClass( Super2::ClassFactory() );            \
        (*ppFactory)->PutSuperClass( Super3::ClassFactory() );            \
    }                                                                     \
    return *pFactory;                                                     \
}                                                                         \
void * ClassName::CreateInstance( SotObject ** ppObj )            \
{                                                                         \
    ClassName * p = new ClassName();                                      \
    if( ppObj )                                                           \
        *ppObj = p;                                                       \
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
    if( !pRet )                                                           \
        pRet = Super2::Cast( pFact );                                     \
    if( !pRet )                                                           \
        pRet = Super3::Cast( pFact );                                     \
    return pRet;                                                          \
}

#define SO2_IMPL_BASIC_CLASS3(ClassName,FactoryName,Super1,Super2,Super3,GlobalName)  \
SotFactory * ClassName::pFactory = NULL;                                   \
    SO2_IMPL_BASIC_CLASS3_DLL(ClassName,FactoryName,Super1,Super2,Super3,GlobalName)

/**************************************************************************
**************************************************************************/
#define SO2_IMPL_BASIC_CLASS4_DLL(ClassName,FactoryName,Super1,Super2,Super3,Super4,GlobalName)  \
SotFactory * ClassName::ClassFactory()                                     \
{                                                                         \
    SotFactory **ppFactory = GetFactoryAdress();                           \
    if( !*ppFactory )                                                     \
    {                                                                     \
        *ppFactory = new SotFactory( GlobalName,                           \
            String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( #ClassName ) ), \
                                 ClassName::CreateInstance );\
        (*ppFactory)->PutSuperClass( Super1::ClassFactory() );            \
        (*ppFactory)->PutSuperClass( Super2::ClassFactory() );            \
        (*ppFactory)->PutSuperClass( Super3::ClassFactory() );            \
        (*ppFactory)->PutSuperClass( Super4::ClassFactory() );            \
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
    if( !pRet )                                                           \
        pRet = Super2::Cast( pFact );                                     \
    if( !pRet )                                                           \
        pRet = Super3::Cast( pFact );                                     \
    if( !pRet )                                                           \
        pRet = Super4::Cast( pFact );                                     \
    return pRet;                                                          \
}

#define SO2_IMPL_BASIC_CLASS4(ClassName,FactoryName,Super1,Super2,Super3,Super4,GlobalName)  \
SotFactory * ClassName::pFactory = NULL;                                   \
    SO2_IMPL_BASIC_CLASS4_DLL(ClassName,FactoryName,Super1,Super2,Super3,Super4,GlobalName)

//==================class SotObject========================================
#ifdef _MSC_VER
#pragma warning(disable: 4250)
#endif

struct IUnknown;
class SOT_DLLPUBLIC SotObject : virtual public SvRefBase
{
friend class SotFactory;
friend class SvObject;
    sal_uInt16  nStrongLockCount;
    sal_uInt16  nOwnerLockCount;
    sal_Bool    bOwner:1,
                bSVObject:1,        // Ist Proxy, dann TRUE wenn andere Seite SV ist
                bInClose:1;         // TRUE, im DoClose

#if defined (GCC) && (defined (C281) || defined (C290) || defined (C291))
public:
#else
protected:
#endif
    virtual             ~SotObject();
    void                SetExtern() { bOwner = sal_False; }
    virtual sal_Bool        Close();
public:
                        SotObject();
                        SO2_DECL_BASIC_CLASS_DLL(SotObject,SOTDATA())
                        SO2_DECL_INVARIANT()

                        // Nur damit die Makros in So3 nicht ganz ausufern
    virtual IUnknown *  GetInterface( const SvGlobalName & );

    sal_Bool                Owner() const { return bOwner; }

    void*               CastAndAddRef( const SotFactory * pFact );

    sal_uInt16              Lock( sal_Bool bLock ); // affects nStrongLockCount
    sal_uInt16              GetOwnerLockCount() const { return nOwnerLockCount; }
    sal_uInt16              GetStrongLockCount() const { return nStrongLockCount; }

    void                OwnerLock( sal_Bool bLock );
    void                RemoveOwnerLock();
    sal_Bool                DoClose();
    sal_Bool                IsInClose() const { return bInClose; }

private:
    // Kopieren und Zuweisen dieses Objekttyps ist nicht erlaubt
    SOT_DLLPRIVATE SotObject & operator = ( const SotObject & );
    SOT_DLLPRIVATE SotObject( const SotObject & );
};

//==================class SotObjectRef======================================
SV_IMPL_REF(SotObject)

//==================class SotObject*List====================================
SV_DECL_REF_LIST(SotObject,SotObject*)
SV_IMPL_REF_LIST(SotObject,SotObject*)

#endif // _IFACE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
