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

#ifndef _IFACE_HXX
#define _IFACE_HXX

#include <bf_svtools/hint.hxx>
#include <tools/gen.hxx>
#include <tools/svborder.hxx>
#include <tools/shl.hxx>
#include <bf_so3/factory.hxx>
#include <bf_so3/so2dll.hxx>

#include <sot/object.hxx>


#include "bf_so3/so3dllapi.h"

struct IUnknown;

namespace binfilter {

/*************************************************************************
*************************************************************************/
class  SvSoIPCClient;
class  SvSoIPCService;

// Zugriff auf Dlldaten
#define SOAPP  SoDll::GetOrCreate()

/**************************************************************************
**************************************************************************/
#define SO2_DECL_INTERFACE()                                              \
    virtual ::IUnknown *        GetInterface( const SvGlobalName & );       \
    ::IUnknown *                GetMemberInterface(  const SvGlobalName & );\


#define SO2_DECL_STANDARD_CLASS_DLL(ClassName,FacName)                    \
    SO2_DECL_BASIC_CLASS_DLL(ClassName,FacName)                           \
    SO2_DECL_INTERFACE()                                                  \
    SO2_DECL_INVARIANT()

#define SO2_DECL_STANDARD_CLASS(ClassName)                                \
    SO2_DECL_BASIC_CLASS(ClassName)                                       \
    SO2_DECL_INTERFACE()                                                  \
    SO2_DECL_INVARIANT()

/**************************************************************************
**************************************************************************/
#define SO2_IMPL_INTERFACE(ClassName)                                     \
::IUnknown * __EXPORT ClassName::GetInterface( const SvGlobalName & rName ) \
{                                                                         \
    ::IUnknown * p = GetMemberInterface( rName );                           \
    return p;                                                             \
}                                                                         \


#define SO2_IMPL_CLASS_DLL(ClassName,FactoryName,GlobalName)              \
SO2_IMPL_BASIC_CLASS_DLL(ClassName,FactoryName,GlobalName)                \
SO2_IMPL_INTERFACE(ClassName)                                             \
SO2_IMPL_INVARIANT(ClassName)

#define SO2_IMPL_CLASS(ClassName,FactoryName,GlobalName)                  \
SotFactory * ClassName::pFactory = NULL;                                   \
    SO2_IMPL_CLASS_DLL(ClassName,FactoryName,GlobalName)

/**************************************************************************
**************************************************************************/
#define SO2_IMPL_INTERFACE1(ClassName,Super1)                             \
::IUnknown * __EXPORT ClassName::GetInterface( const SvGlobalName & rName ) \
{                                                                         \
    ::IUnknown * p = GetMemberInterface( rName );                           \
    if( !p )                                                              \
        p = Super1::GetInterface( rName );                                \
    return p;                                                             \
}                                                                         \

#define SO2_IMPL_CLASS1_DLL(ClassName,FactoryName,Super1,GlobalName)      \
SO2_IMPL_BASIC_CLASS1_DLL(ClassName,FactoryName,Super1,GlobalName)        \
SO2_IMPL_INTERFACE1(ClassName,Super1)                                     \
SO2_IMPL_INVARIANT1(ClassName,Super1)

#define SO2_IMPL_CLASS1(ClassName,FactoryName,Super1,GlobalName)          \
SotFactory * ClassName::pFactory = NULL;                                   \
    SO2_IMPL_CLASS1_DLL(ClassName,FactoryName,Super1,GlobalName)

#define SO2_IMPL_STANDARD_CLASS1_DLL(ClassName,FactoryName,Super1,n1,n2,n3,b8,b9,b10,b11,b12,b13,b14,b15)  \
SO2_IMPL_CLASS1_DLL(ClassName,FactoryName,Super1,                     \
                        SvGlobalName(n1,n2,n3,b8,b9,b10,b11,b12,b13,b14,b15))

#define SO2_IMPL_STANDARD_CLASS1(ClassName,FactoryName,Super1,n1,n2,n3,b8,b9,b10,b11,b12,b13,b14,b15)  \
    SO2_IMPL_CLASS1(ClassName,FactoryName,Super1,            \
                            SvGlobalName(n1,n2,n3,b8,b9,b10,b11,b12,b13,b14,b15))

/**************************************************************************
**************************************************************************/
#define SO2_IMPL_INTERFACE2(ClassName,Super1,Super2)                          \
::IUnknown * __EXPORT ClassName::GetInterface( const SvGlobalName & rName ) \
{                                                                         \
    ::IUnknown * p = GetMemberInterface( rName );                           \
    if( !p )                                                              \
        p = Super1::GetInterface( rName );                                \
    if( !p )                                                              \
        p = Super2::GetInterface( rName );                                \
    return p;                                                             \
}                                                                         \

#define SO2_IMPL_CLASS2_DLL(ClassName,FactoryName,Super1,Super2,GlobalName)  \
SO2_IMPL_BASIC_CLASS2_DLL(ClassName,FactoryName,Super1,Super2,GlobalName)  \
SO2_IMPL_INTERFACE2(ClassName,Super1,Super2)                              \
SO2_IMPL_INVARIANT2(ClassName,Super1,Super2)

#define SO2_IMPL_CLASS2(ClassName,FactoryName,Super1,Super2,GlobalName)   \
SotFactory * ClassName::pFactory = NULL;                                   \
    SO2_IMPL_CLASS2_DLL(ClassName,FactoryName,Super1,Super2,GlobalName)

#define SO2_IMPL_STANDARD_CLASS2(ClassName,FactoryName,Super1,Super2,n1,n2,n3,b8,b9,b10,b11,b12,b13,b14,b15)  \
    SO2_IMPL_CLASS2(ClassName,FactoryName,Super1,Super2,              \
                        SvGlobalName(n1,n2,n3,b8,b9,b10,b11,b12,b13,b14,b15))

#define SO2_IMPL_STANDARD_CLASS2_DLL(ClassName,FactoryName,Super1,Super2,n1,n2,n3,b8,b9,b10,b11,b12,b13,b14,b15)  \
    SO2_IMPL_CLASS2_DLL(ClassName,FactoryName,Super1,Super2,                  \
                        SvGlobalName(n1,n2,n3,b8,b9,b10,b11,b12,b13,b14,b15))

/**************************************************************************
**************************************************************************/
#define SO2_IMPL_INTERFACE3(ClassName,Super1,Super2,Super3)               \
::IUnknown * __EXPORT ClassName::GetInterface( const SvGlobalName & rName ) \
{                                                                         \
    ::IUnknown * p = GetMemberInterface( rName );                           \
    if( !p )                                                              \
        p = Super1::GetInterface( rName );                                \
    if( !p )                                                              \
        p = Super2::GetInterface( rName );                                \
    if( !p )                                                              \
        p = Super3::GetInterface( rName );                                \
    return p;                                                             \
}                                                                         \

#define SO2_IMPL_CLASS3_DLL(ClassName,FactoryName,Super1,Super2,Super3,GlobalName)    \
SO2_IMPL_BASIC_CLASS3_DLL(ClassName,FactoryName,Super1,Super2,Super3,GlobalName)  \
SO2_IMPL_INTERFACE3(ClassName,Super1,Super2,Super3)                       \
SO2_IMPL_INVARIANT3(ClassName,Super1,Super2,Super3)

#define SO2_IMPL_CLASS3(ClassName,FactoryName,Super1,Super2,Super3,GlobalName)  \
SotFactory * ClassName::pFactory = NULL;                                   \
    SO2_IMPL_CLASS3_DLL(ClassName,FactoryName,Super1,Super2,Super3,GlobalName)

#define SO2_IMPL_STANDARD_CLASS3(ClassName,FactoryName,Super1,Super2,Super3,n1,n2,n3,b8,b9,b10,b11,b12,b13,b14,b15)  \
    SO2_IMPL_CLASS3(ClassName,FactoryName,Super1,Super2,Super3,           \
                    SvGlobalName(n1,n2,n3,b8,b9,b10,b11,b12,b13,b14,b15))

#define SO2_IMPL_STANDARD_CLASS3_DLL(ClassName,FactoryName,Super1,Super2,Super3,n1,n2,n3,b8,b9,b10,b11,b12,b13,b14,b15)  \
    SO2_IMPL_CLASS3_DLL(ClassName,FactoryName,Super1,Super2,Super3,       \
                        SvGlobalName(n1,n2,n3,b8,b9,b10,b11,b12,b13,b14,b15)

/**************************************************************************
**************************************************************************/
#define SO2_IMPL_INTERFACE4(ClassName,Super1,Super2,Super3,Super4)        \
::IUnknown * __EXPORT ClassName::GetInterface( const SvGlobalName & rName ) \
{                                                                         \
    ::IUnknown * p = GetMemberInterface( rName );                           \
    if( !p )                                                              \
        p = Super1::GetInterface( rName );                                \
    if( !p )                                                              \
        p = Super2::GetInterface( rName );                                \
    if( !p )                                                              \
        p = Super3::GetInterface( rName );                                \
    if( !p )                                                              \
        p = Super4::GetInterface( rName );                                \
    return p;                                                             \
}                                                                         \

#define SO2_IMPL_CLASS4_DLL(ClassName,FactoryName,Super1,Super2,Super3,Super4,GlobalName )  \
SO2_IMPL_BASIC_CLASS4_DLL(ClassName,FactoryName,Super1,Super2,Super3,Super4,GlobalName )  \
SO2_IMPL_INTERFACE4(ClassName,Super1,Super2,Super3,Super4)                \
SO2_IMPL_INVARIANT4(ClassName,Super1,Super2,Super3,Super4)

#define SO2_IMPL_CLASS4(ClassName,FactoryName,Super1,Super2,Super3,Super4,GlobalName)  \
SotFactory * ClassName::pFactory = NULL;                                   \
    SO2_IMPL_CLASS4_DLL(ClassName,FactoryName,Super1,Super2,Super3,Super4,GlobalName)

#define SO2_IMPL_STANDARD_CLASS4(ClassName,FactoryName,Super1,Super2,Super3,Super4,n1,n2,n3,b8,b9,b10,b11,b12,b13,b14,b15)  \
    SO2_IMPL_CLASS4(ClassName,FactoryName,Super1,Super2,Super3,Super4,\
                    SvGlobalName(n1,n2,n3,b8,b9,b10,b11,b12,b13,b14,b15))

#define SO2_IMPL_STANDARD_CLASS4_DLL(ClassName,FactoryName,Super1,Super2,Super3,Super4,n1,n2,n3,b8,b9,b10,b11,b12,b13,b14,b15)  \
    SO2_IMPL_CLASS4_DLL(ClassName,FactoryName,Super1,Super2,Super3,Super4,\
                        SvGlobalName(n1,n2,n3,b8,b9,b10,b11,b12,b13,b14,b15))


//==================class SvObject========================================
#ifdef MSC
#pragma warning(disable: 4250)
#endif

class SO3_DLLPUBLIC SvObject : virtual public SotObject
{
friend struct IUnknown;
friend class SvSoIPCService;
friend class SvSoIPCClient;
friend class SvFactory;

private:
    SO3_DLLPRIVATE union
    {
        ::IUnknown *      pObj;       // IUnknown Interface
        SvSoIPCClient * pClient;    // Client, falls !Owner()
        SvSoIPCService * pService;  // Service Punkt, falls Owner()
    };
    USHORT      nExtCount;          // externer Referenzzaehler

#if defined (GCC) && (defined(C281) || defined(C290) || defined(C291))
public:
#else
protected:
#endif
    virtual             ~SvObject();
    SO3_DLLPRIVATE void                DeInit( ::IUnknown * );
    SO3_DLLPRIVATE void                DeInit( SvSoIPCService * );
    SO3_DLLPRIVATE void                DeInit( SvSoIPCClient * );
    virtual void        MakeUnknown();
public:
                        SvObject();
                        SO2_DECL_STANDARD_CLASS_DLL(SvObject,SOAPP)
    void                Init( SvSoIPCClient * pObj );
    void                OwnerInit( SvSoIPCService * pObj );
    SvSoIPCService *    GetIPCService() const;
    SvSoIPCClient  *    GetIPCClient() const;

    ::IUnknown *          DownAggInterface( const SvGlobalName & rName );
    ::IUnknown *          AggInterface( const SvGlobalName & rName );

    UINT32				ReleaseRef(); // Nur fur DBG_UTIL
private:
    // Kopieren und Zuweisen dieses Objekttyps ist nicht erlaubt
    SO3_DLLPRIVATE SvObject & operator = ( const SvObject & );
    SO3_DLLPRIVATE SvObject( const SvObject & );
};

//==================class SvObjectRef======================================
#ifndef SO2_DECL_SVOBJECT_DEFINED
#define SO2_DECL_SVOBJECT_DEFINED
class SO3_DLLPUBLIC SvObjectRef
{
    PRV_SV_DECL_REF(SvObject)
};
#endif
SO2_IMPL_REF(SvObject)

//==================class SvObject*List====================================
#if !defined( SO_EXTERN ) || defined( SO2_SVOBJECTLIST )
SV_DECL_REF_LIST(SvObject,SvObject*)
SV_IMPL_REF_LIST(SvObject,SvObject*)
#endif

//==================class SvFactory_Impl===================================
#if !defined( SO_EXTERN )
struct IClassFactory;
class SO3_DLLPRIVATE SvFactory_Impl : public SvObject
/*  [Beschreibung]

*/
{
friend class ImpFactory;
friend class SvFactory;
private:
    IClassFactory * pObjI;
    SvFactory *     pFact;
public:
    SO3_DLLPRIVATE			SvFactory_Impl();
    SO3_DLLPRIVATE			SvFactory_Impl( SvFactory * );
    SO3_DLLPRIVATE			~SvFactory_Impl();

    SO2_DECL_STANDARD_CLASS_DLL( SvFactory_Impl, SOAPP )

    SO3_DLLPRIVATE IClassFactory * GetClassFactory() const;
};
#endif

//=========================================================================
#define OLE2_MAPMODE    MAP_100TH_MM

#if defined (_INC_WINDOWS) || defined (_WINDOWS_)
RECT GetSysRect( const Rectangle & rRect );
Rectangle GetSvRect( const RECT & rRect );
#endif

#ifdef DBG_UTIL
ByteString  SvPrint( const SvBorder & );
ByteString  SvPrint( const Rectangle & );
ByteString  SvPrint( const Point & );
ByteString  SvPrint( const Size & );
#endif

//=========================================================================

}

#endif // _IFACE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
