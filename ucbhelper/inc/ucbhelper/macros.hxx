/*************************************************************************
 *
 *  $RCSfile: macros.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:37 $
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

#ifndef _UCBHELPER_MACROS_HXX
#define _UCBHELPER_MACROS_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _CPPUHELPER_QUERYINTERFACE_HXX_
#include <cppuhelper/queryinterface.hxx>
#endif
#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif
#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

//=========================================================================

#define CPPU_TYPE( T )      getCppuType( static_cast< T * >( 0 ) )
#define CPPU_TYPE_REF( T )  CPPU_TYPE( com::sun::star::uno::Reference< T > )

//=========================================================================
//
// XInterface decl.
//
//=========================================================================

#define XINTERFACE_DECL()                                                   \
    virtual com::sun::star::uno::Any SAL_CALL                               \
    queryInterface( const com::sun::star::uno::Type & rType )               \
        throw( com::sun::star::uno::RuntimeException );                     \
    virtual void SAL_CALL                                                   \
    acquire()                                                               \
        throw( com::sun::star::uno::RuntimeException );                     \
    virtual void SAL_CALL                                                   \
    release()                                                               \
        throw( com::sun::star::uno::RuntimeException );

//=========================================================================
//
// XInterface impl. internals.
//
//=========================================================================

#define XINTERFACE_COMMON_IMPL( Class )                                     \
void SAL_CALL Class::acquire()                                              \
    throw( com::sun::star::uno::RuntimeException )                          \
{                                                                           \
    OWeakObject::acquire();                                                 \
}                                                                           \
                                                                            \
void SAL_CALL Class::release()                                              \
    throw( com::sun::star::uno::RuntimeException )                          \
{                                                                           \
    OWeakObject::release();                                                 \
}

#define QUERYINTERFACE_IMPL_START( Class )                                  \
com::sun::star::uno::Any SAL_CALL Class::queryInterface(                    \
                                const com::sun::star::uno::Type & rType )   \
    throw( com::sun::star::uno::RuntimeException )                          \
{                                                                           \
    com::sun::star::uno::Any aRet = cppu::queryInterface( rType,

#define QUERYINTERFACE_IMPL_END                                             \
                    );                                                      \
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );   \
}

//=========================================================================
//
// XInterface impl.
//
//=========================================================================

// 1 interface implemented
#define XINTERFACE_IMPL_1( Class, Ifc1 )                                    \
XINTERFACE_COMMON_IMPL( Class )                                             \
QUERYINTERFACE_IMPL_START( Class )                                          \
    SAL_STATIC_CAST( Ifc1*, this )                                          \
QUERYINTERFACE_IMPL_END

// 2 interfaces implemented
#define XINTERFACE_IMPL_2( Class, Ifc1, Ifc2 )                              \
XINTERFACE_COMMON_IMPL( Class )                                             \
QUERYINTERFACE_IMPL_START( Class )                                          \
    SAL_STATIC_CAST( Ifc1*, this ),                                         \
    SAL_STATIC_CAST( Ifc2*, this )                                          \
QUERYINTERFACE_IMPL_END

// 3 interfaces implemented
#define XINTERFACE_IMPL_3( Class, Ifc1, Ifc2, Ifc3 )                        \
XINTERFACE_COMMON_IMPL( Class )                                             \
QUERYINTERFACE_IMPL_START( Class )                                          \
    SAL_STATIC_CAST( Ifc1*, this ),                                         \
    SAL_STATIC_CAST( Ifc2*, this ),                                         \
    SAL_STATIC_CAST( Ifc3*, this )                                          \
QUERYINTERFACE_IMPL_END

// 4 interfaces implemented
#define XINTERFACE_IMPL_4( Class, Ifc1, Ifc2, Ifc3, Ifc4 )                  \
XINTERFACE_COMMON_IMPL( Class )                                             \
QUERYINTERFACE_IMPL_START( Class )                                          \
    SAL_STATIC_CAST( Ifc1*, this ),                                         \
    SAL_STATIC_CAST( Ifc2*, this ),                                         \
    SAL_STATIC_CAST( Ifc3*, this ),                                         \
    SAL_STATIC_CAST( Ifc4*, this )                                          \
QUERYINTERFACE_IMPL_END

// 5 interfaces implemented
#define XINTERFACE_IMPL_5( Class, Ifc1, Ifc2, Ifc3, Ifc4, Ifc5 )            \
XINTERFACE_COMMON_IMPL( Class )                                             \
QUERYINTERFACE_IMPL_START( Class )                                          \
    SAL_STATIC_CAST( Ifc1*, this ),                                         \
    SAL_STATIC_CAST( Ifc2*, this ),                                         \
    SAL_STATIC_CAST( Ifc3*, this ),                                         \
    SAL_STATIC_CAST( Ifc4*, this ),                                         \
    SAL_STATIC_CAST( Ifc5*, this )                                          \
QUERYINTERFACE_IMPL_END

// 6 interfaces implemented
#define XINTERFACE_IMPL_6( Class,I1,I2,I3,I4,I5,I6 )                        \
XINTERFACE_COMMON_IMPL( Class )                                             \
QUERYINTERFACE_IMPL_START( Class )                                          \
    SAL_STATIC_CAST( I1*, this ),                                           \
    SAL_STATIC_CAST( I2*, this ),                                           \
    SAL_STATIC_CAST( I3*, this ),                                           \
    SAL_STATIC_CAST( I4*, this ),                                           \
    SAL_STATIC_CAST( I5*, this ),                                           \
    SAL_STATIC_CAST( I6*, this )                                            \
QUERYINTERFACE_IMPL_END

// 7 interfaces implemented
#define XINTERFACE_IMPL_7( Class,I1,I2,I3,I4,I5,I6,I7 )                     \
XINTERFACE_COMMON_IMPL( Class )                                             \
QUERYINTERFACE_IMPL_START( Class )                                          \
    SAL_STATIC_CAST( I1*, this ),                                           \
    SAL_STATIC_CAST( I2*, this ),                                           \
    SAL_STATIC_CAST( I3*, this ),                                           \
    SAL_STATIC_CAST( I4*, this ),                                           \
    SAL_STATIC_CAST( I5*, this ),                                           \
    SAL_STATIC_CAST( I6*, this ),                                           \
    SAL_STATIC_CAST( I7*, this )                                            \
QUERYINTERFACE_IMPL_END

// 8 interfaces implemented
#define XINTERFACE_IMPL_8( Class,I1,I2,I3,I4,I5,I6,I7,I8 )                  \
XINTERFACE_COMMON_IMPL( Class )                                             \
QUERYINTERFACE_IMPL_START( Class )                                          \
    SAL_STATIC_CAST( I1*, this ),                                           \
    SAL_STATIC_CAST( I2*, this ),                                           \
    SAL_STATIC_CAST( I3*, this ),                                           \
    SAL_STATIC_CAST( I4*, this ),                                           \
    SAL_STATIC_CAST( I5*, this ),                                           \
    SAL_STATIC_CAST( I6*, this ),                                           \
    SAL_STATIC_CAST( I7*, this ),                                           \
    SAL_STATIC_CAST( I8*, this )                                            \
QUERYINTERFACE_IMPL_END

// 9 interfaces implemented
#define XINTERFACE_IMPL_9( Class,I1,I2,I3,I4,I5,I6,I7,I8,I9 )               \
XINTERFACE_COMMON_IMPL( Class )                                             \
QUERYINTERFACE_IMPL_START( Class )                                          \
    SAL_STATIC_CAST( I1*, this ),                                           \
    SAL_STATIC_CAST( I2*, this ),                                           \
    SAL_STATIC_CAST( I3*, this ),                                           \
    SAL_STATIC_CAST( I4*, this ),                                           \
    SAL_STATIC_CAST( I5*, this ),                                           \
    SAL_STATIC_CAST( I6*, this ),                                           \
    SAL_STATIC_CAST( I7*, this ),                                           \
    SAL_STATIC_CAST( I8*, this ),                                           \
    SAL_STATIC_CAST( I9*, this )                                            \
QUERYINTERFACE_IMPL_END

// 10 interfaces implemented
#define XINTERFACE_IMPL_10( Class,I1,I2,I3,I4,I5,I6,I7,I8,I9,I10 )          \
XINTERFACE_COMMON_IMPL( Class )                                             \
QUERYINTERFACE_IMPL_START( Class )                                          \
    SAL_STATIC_CAST( I1*, this ),                                           \
    SAL_STATIC_CAST( I2*, this ),                                           \
    SAL_STATIC_CAST( I3*, this ),                                           \
    SAL_STATIC_CAST( I4*, this ),                                           \
    SAL_STATIC_CAST( I5*, this ),                                           \
    SAL_STATIC_CAST( I6*, this ),                                           \
    SAL_STATIC_CAST( I7*, this ),                                           \
    SAL_STATIC_CAST( I8*, this ),                                           \
    SAL_STATIC_CAST( I9*, this ),                                           \
    SAL_STATIC_CAST( I10*, this )                                           \
QUERYINTERFACE_IMPL_END

// 11 interfaces implemented
#define XINTERFACE_IMPL_11( Class,I1,I2,I3,I4,I5,I6,I7,I8,I9,I10,I11 )      \
XINTERFACE_COMMON_IMPL( Class )                                             \
QUERYINTERFACE_IMPL_START( Class )                                          \
    SAL_STATIC_CAST( I1*, this ),                                           \
    SAL_STATIC_CAST( I2*, this ),                                           \
    SAL_STATIC_CAST( I3*, this ),                                           \
    SAL_STATIC_CAST( I4*, this ),                                           \
    SAL_STATIC_CAST( I5*, this ),                                           \
    SAL_STATIC_CAST( I6*, this ),                                           \
    SAL_STATIC_CAST( I7*, this ),                                           \
    SAL_STATIC_CAST( I8*, this ),                                           \
    SAL_STATIC_CAST( I9*, this ),                                           \
    SAL_STATIC_CAST( I10*, this ),                                          \
    SAL_STATIC_CAST( I11*, this )                                           \
QUERYINTERFACE_IMPL_END

// 12 interfaces implemented
#define XINTERFACE_IMPL_12( Class,I1,I2,I3,I4,I5,I6,I7,I8,I9,I10,I11,I12 )  \
XINTERFACE_COMMON_IMPL( Class )                                             \
QUERYINTERFACE_IMPL_START( Class )                                          \
    SAL_STATIC_CAST( I1*, this ),                                           \
    SAL_STATIC_CAST( I2*, this ),                                           \
    SAL_STATIC_CAST( I3*, this ),                                           \
    SAL_STATIC_CAST( I4*, this ),                                           \
    SAL_STATIC_CAST( I5*, this ),                                           \
    SAL_STATIC_CAST( I6*, this ),                                           \
    SAL_STATIC_CAST( I7*, this ),                                           \
    SAL_STATIC_CAST( I8*, this ),                                           \
    SAL_STATIC_CAST( I9*, this ),                                           \
    SAL_STATIC_CAST( I10*, this ),                                          \
    SAL_STATIC_CAST( I11*, this ),                                          \
    SAL_STATIC_CAST( I12*, this )                                           \
QUERYINTERFACE_IMPL_END

// 13 interfaces implemented
#define XINTERFACE_IMPL_13( Class,I1,I2,I3,I4,I5,I6,I7,I8,I9,I10,I11,I12,I13 ) \
XINTERFACE_COMMON_IMPL( Class )                                             \
QUERYINTERFACE_IMPL_START( Class )                                          \
    SAL_STATIC_CAST( I1*, this ),                                           \
    SAL_STATIC_CAST( I2*, this ),                                           \
    SAL_STATIC_CAST( I3*, this ),                                           \
    SAL_STATIC_CAST( I4*, this ),                                           \
    SAL_STATIC_CAST( I5*, this ),                                           \
    SAL_STATIC_CAST( I6*, this ),                                           \
    SAL_STATIC_CAST( I7*, this ),                                           \
    SAL_STATIC_CAST( I8*, this ),                                           \
    SAL_STATIC_CAST( I9*, this ),                                           \
    SAL_STATIC_CAST( I10*, this ),                                          \
    SAL_STATIC_CAST( I11*, this ),                                          \
    SAL_STATIC_CAST( I12*, this ),                                          \
    SAL_STATIC_CAST( I13*, this )                                           \
QUERYINTERFACE_IMPL_END

// 14 interfaces implemented
#define XINTERFACE_IMPL_14( Class,I1,I2,I3,I4,I5,I6,I7,I8,I9,I10,I11,I12,I13,I14 )  \
XINTERFACE_COMMON_IMPL( Class )                                             \
QUERYINTERFACE_IMPL_START( Class )                                          \
    SAL_STATIC_CAST( I1*, this ),                                           \
    SAL_STATIC_CAST( I2*, this ),                                           \
    SAL_STATIC_CAST( I3*, this ),                                           \
    SAL_STATIC_CAST( I4*, this ),                                           \
    SAL_STATIC_CAST( I5*, this ),                                           \
    SAL_STATIC_CAST( I6*, this ),                                           \
    SAL_STATIC_CAST( I7*, this ),                                           \
    SAL_STATIC_CAST( I8*, this ),                                           \
    SAL_STATIC_CAST( I9*, this ),                                           \
    SAL_STATIC_CAST( I10*, this ),                                          \
    SAL_STATIC_CAST( I11*, this ),                                          \
    SAL_STATIC_CAST( I12*, this ),                                          \
    SAL_STATIC_CAST( I13*, this ),                                          \
    SAL_STATIC_CAST( I14*, this )                                           \
QUERYINTERFACE_IMPL_END

// 15 interfaces implemented
#define XINTERFACE_IMPL_15( Class,I1,I2,I3,I4,I5,I6,I7,I8,I9,I10,I11,I12,I13,I14,I15 )  \
XINTERFACE_COMMON_IMPL( Class )                                             \
QUERYINTERFACE_IMPL_START( Class )                                          \
    SAL_STATIC_CAST( I1*, this ),                                           \
    SAL_STATIC_CAST( I2*, this ),                                           \
    SAL_STATIC_CAST( I3*, this ),                                           \
    SAL_STATIC_CAST( I4*, this ),                                           \
    SAL_STATIC_CAST( I5*, this ),                                           \
    SAL_STATIC_CAST( I6*, this ),                                           \
    SAL_STATIC_CAST( I7*, this ),                                           \
    SAL_STATIC_CAST( I8*, this ),                                           \
    SAL_STATIC_CAST( I9*, this ),                                           \
    SAL_STATIC_CAST( I10*, this ),                                          \
    SAL_STATIC_CAST( I11*, this ),                                          \
    SAL_STATIC_CAST( I12*, this ),                                          \
    SAL_STATIC_CAST( I13*, this ),                                          \
    SAL_STATIC_CAST( I14*, this ),                                          \
    SAL_STATIC_CAST( I15*, this )                                           \
QUERYINTERFACE_IMPL_END

//=========================================================================
//
// XTypeProvider decl.
//
//=========================================================================

#define XTYPEPROVIDER_DECL()                                                \
    virtual com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL              \
    getImplementationId()                                                   \
        throw( com::sun::star::uno::RuntimeException );                     \
    virtual com::sun::star::uno::Sequence< com::sun::star::uno::Type > SAL_CALL \
    getTypes()                                                              \
        throw( com::sun::star::uno::RuntimeException );

//=========================================================================
//
// XTypeProvider impl. internals
//
//=========================================================================

#define XTYPEPROVIDER_COMMON_IMPL( Class )                                  \
com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL                          \
Class::getImplementationId()                                                \
    throw( com::sun::star::uno::RuntimeException )                          \
{                                                                           \
    static cppu::OImplementationId* pId = NULL;                             \
      if ( !pId )                                                             \
      {                                                                       \
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );    \
          if ( !pId )                                                         \
          {                                                                   \
              static cppu::OImplementationId id( sal_False );                 \
              pId = &id;                                                      \
          }                                                                   \
      }                                                                       \
      return (*pId).getImplementationId();                                    \
}

#define GETTYPES_IMPL_START( Class )                                        \
com::sun::star::uno::Sequence< com::sun::star::uno::Type > SAL_CALL         \
Class::getTypes()                                                           \
    throw( com::sun::star::uno::RuntimeException )                          \
{                                                                           \
    static cppu::OTypeCollection* pCollection = NULL;                       \
      if ( !pCollection )                                                     \
      {                                                                       \
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );    \
        if ( !pCollection )                                                 \
        {                                                                   \
            static cppu::OTypeCollection collection(

#define GETTYPES_IMPL_END                                                   \
                );                                                          \
            pCollection = &collection;                                      \
        }                                                                   \
    }                                                                       \
    return (*pCollection).getTypes();                                       \
}

//=========================================================================
//
// XTypeProvider impl.
//
//=========================================================================

// 1 interface supported
#define XTYPEPROVIDER_IMPL_1( Class, I1 )                                   \
XTYPEPROVIDER_COMMON_IMPL( Class )                                          \
GETTYPES_IMPL_START( Class )                                                \
    CPPU_TYPE_REF( I1 )                                                 \
GETTYPES_IMPL_END

// 2 interfaces supported
#define XTYPEPROVIDER_IMPL_2( Class, I1,I2 )                                \
XTYPEPROVIDER_COMMON_IMPL( Class )                                          \
GETTYPES_IMPL_START( Class )                                                \
    CPPU_TYPE_REF( I1 ),                                                    \
    CPPU_TYPE_REF( I2 )                                                     \
GETTYPES_IMPL_END

// 3 interfaces supported
#define XTYPEPROVIDER_IMPL_3( Class, I1,I2,I3 )                             \
XTYPEPROVIDER_COMMON_IMPL( Class )                                          \
GETTYPES_IMPL_START( Class )                                                \
    CPPU_TYPE_REF( I1 ),                                                    \
    CPPU_TYPE_REF( I2 ),                                                    \
    CPPU_TYPE_REF( I3 )                                                     \
GETTYPES_IMPL_END

// 4 interfaces supported
#define XTYPEPROVIDER_IMPL_4( Class, I1,I2,I3,I4 )                          \
XTYPEPROVIDER_COMMON_IMPL( Class )                                          \
GETTYPES_IMPL_START( Class )                                                \
    CPPU_TYPE_REF( I1 ),                                                    \
    CPPU_TYPE_REF( I2 ),                                                    \
    CPPU_TYPE_REF( I3 ),                                                    \
    CPPU_TYPE_REF( I4 )                                                     \
GETTYPES_IMPL_END

// 5 interfaces supported
#define XTYPEPROVIDER_IMPL_5( Class, I1,I2,I3,I4,I5 )                       \
XTYPEPROVIDER_COMMON_IMPL( Class )                                          \
GETTYPES_IMPL_START( Class )                                                \
    CPPU_TYPE_REF( I1 ),                                                    \
    CPPU_TYPE_REF( I2 ),                                                    \
    CPPU_TYPE_REF( I3 ),                                                    \
    CPPU_TYPE_REF( I4 ),                                                    \
    CPPU_TYPE_REF( I5 )                                                     \
GETTYPES_IMPL_END

// 6 interfaces supported
#define XTYPEPROVIDER_IMPL_6( Class, I1,I2,I3,I4,I5,I6 )                    \
XTYPEPROVIDER_COMMON_IMPL( Class )                                          \
GETTYPES_IMPL_START( Class )                                                \
    CPPU_TYPE_REF( I1 ),                                                    \
    CPPU_TYPE_REF( I2 ),                                                    \
    CPPU_TYPE_REF( I3 ),                                                    \
    CPPU_TYPE_REF( I4 ),                                                    \
    CPPU_TYPE_REF( I5 ),                                                    \
    CPPU_TYPE_REF( I6 )                                                     \
GETTYPES_IMPL_END

// 7 interfaces supported
#define XTYPEPROVIDER_IMPL_7( Class, I1,I2,I3,I4,I5,I6,I7 )                 \
XTYPEPROVIDER_COMMON_IMPL( Class )                                          \
GETTYPES_IMPL_START( Class )                                                \
    CPPU_TYPE_REF( I1 ),                                                    \
    CPPU_TYPE_REF( I2 ),                                                    \
    CPPU_TYPE_REF( I3 ),                                                    \
    CPPU_TYPE_REF( I4 ),                                                    \
    CPPU_TYPE_REF( I5 ),                                                    \
    CPPU_TYPE_REF( I6 ),                                                    \
    CPPU_TYPE_REF( I7 )                                                     \
GETTYPES_IMPL_END

// 8 interfaces supported
#define XTYPEPROVIDER_IMPL_8( Class, I1,I2,I3,I4,I5,I6,I7,I8 )              \
XTYPEPROVIDER_COMMON_IMPL( Class )                                          \
GETTYPES_IMPL_START( Class )                                                \
    CPPU_TYPE_REF( I1 ),                                                    \
    CPPU_TYPE_REF( I2 ),                                                    \
    CPPU_TYPE_REF( I3 ),                                                    \
    CPPU_TYPE_REF( I4 ),                                                    \
    CPPU_TYPE_REF( I5 ),                                                    \
    CPPU_TYPE_REF( I6 ),                                                    \
    CPPU_TYPE_REF( I7 ),                                                    \
    CPPU_TYPE_REF( I8 )                                                     \
GETTYPES_IMPL_END

// 9 interfaces supported
#define XTYPEPROVIDER_IMPL_9( Class, I1,I2,I3,I4,I5,I6,I7,I8,I9 )           \
XTYPEPROVIDER_COMMON_IMPL( Class )                                          \
GETTYPES_IMPL_START( Class )                                                \
    CPPU_TYPE_REF( I1 ),                                                    \
    CPPU_TYPE_REF( I2 ),                                                    \
    CPPU_TYPE_REF( I3 ),                                                    \
    CPPU_TYPE_REF( I4 ),                                                    \
    CPPU_TYPE_REF( I5 ),                                                    \
    CPPU_TYPE_REF( I6 ),                                                    \
    CPPU_TYPE_REF( I7 ),                                                    \
    CPPU_TYPE_REF( I8 ),                                                    \
    CPPU_TYPE_REF( I9 )                                                     \
GETTYPES_IMPL_END

// 10 interfaces supported
#define XTYPEPROVIDER_IMPL_10( Class, I1,I2,I3,I4,I5,I6,I7,I8,I9,I10 )      \
XTYPEPROVIDER_COMMON_IMPL( Class )                                          \
GETTYPES_IMPL_START( Class )                                                \
    CPPU_TYPE_REF( I1 ),                                                    \
    CPPU_TYPE_REF( I2 ),                                                    \
    CPPU_TYPE_REF( I3 ),                                                    \
    CPPU_TYPE_REF( I4 ),                                                    \
    CPPU_TYPE_REF( I5 ),                                                    \
    CPPU_TYPE_REF( I6 ),                                                    \
    CPPU_TYPE_REF( I7 ),                                                    \
    CPPU_TYPE_REF( I8 ),                                                    \
    CPPU_TYPE_REF( I9 ),                                                    \
    CPPU_TYPE_REF( I10 )                                                    \
GETTYPES_IMPL_END

// 11 interfaces supported
#define XTYPEPROVIDER_IMPL_11( Class, I1,I2,I3,I4,I5,I6,I7,I8,I9,I10,I11 )  \
XTYPEPROVIDER_COMMON_IMPL( Class )                                          \
GETTYPES_IMPL_START( Class )                                                \
    CPPU_TYPE_REF( I1 ),                                                    \
    CPPU_TYPE_REF( I2 ),                                                    \
    CPPU_TYPE_REF( I3 ),                                                    \
    CPPU_TYPE_REF( I4 ),                                                    \
    CPPU_TYPE_REF( I5 ),                                                    \
    CPPU_TYPE_REF( I6 ),                                                    \
    CPPU_TYPE_REF( I7 ),                                                    \
    CPPU_TYPE_REF( I8 ),                                                    \
    CPPU_TYPE_REF( I9 ),                                                    \
    CPPU_TYPE_REF( I10 ),                                                   \
    CPPU_TYPE_REF( I11 )                                                    \
GETTYPES_IMPL_END

// 12 interfaces supported
#define XTYPEPROVIDER_IMPL_12( Class, I1,I2,I3,I4,I5,I6,I7,I8,I9,I10,I11,I12 )  \
XTYPEPROVIDER_COMMON_IMPL( Class )                                          \
GETTYPES_IMPL_START( Class )                                                \
    CPPU_TYPE_REF( I1 ),                                                    \
    CPPU_TYPE_REF( I2 ),                                                    \
    CPPU_TYPE_REF( I3 ),                                                    \
    CPPU_TYPE_REF( I4 ),                                                    \
    CPPU_TYPE_REF( I5 ),                                                    \
    CPPU_TYPE_REF( I6 ),                                                    \
    CPPU_TYPE_REF( I7 ),                                                    \
    CPPU_TYPE_REF( I8 ),                                                    \
    CPPU_TYPE_REF( I9 ),                                                    \
    CPPU_TYPE_REF( I10 ),                                                   \
    CPPU_TYPE_REF( I11 ),                                                   \
    CPPU_TYPE_REF( I12 )                                                    \
GETTYPES_IMPL_END

// 13 interfaces supported
#define XTYPEPROVIDER_IMPL_13( Class, I1,I2,I3,I4,I5,I6,I7,I8,I9,I10,I11,I12,I13 )  \
XTYPEPROVIDER_COMMON_IMPL( Class )                                          \
GETTYPES_IMPL_START( Class )                                                \
    CPPU_TYPE_REF( I1 ),                                                    \
    CPPU_TYPE_REF( I2 ),                                                    \
    CPPU_TYPE_REF( I3 ),                                                    \
    CPPU_TYPE_REF( I4 ),                                                    \
    CPPU_TYPE_REF( I5 ),                                                    \
    CPPU_TYPE_REF( I6 ),                                                    \
    CPPU_TYPE_REF( I7 ),                                                    \
    CPPU_TYPE_REF( I8 ),                                                    \
    CPPU_TYPE_REF( I9 ),                                                    \
    CPPU_TYPE_REF( I10 ),                                                   \
    CPPU_TYPE_REF( I11 ),                                                   \
    CPPU_TYPE_REF( I12 ),                                                   \
    CPPU_TYPE_REF( I13 )                                                    \
GETTYPES_IMPL_END

// 14 interfaces supported
#define XTYPEPROVIDER_IMPL_14( Class, I1,I2,I3,I4,I5,I6,I7,I8,I9,I10,I11,I12,I13,I14 )  \
XTYPEPROVIDER_COMMON_IMPL( Class )                                          \
GETTYPES_IMPL_START( Class )                                                \
    CPPU_TYPE_REF( I1 ),                                                    \
    CPPU_TYPE_REF( I2 ),                                                    \
    CPPU_TYPE_REF( I3 ),                                                    \
    CPPU_TYPE_REF( I4 ),                                                    \
    CPPU_TYPE_REF( I5 ),                                                    \
    CPPU_TYPE_REF( I6 ),                                                    \
    CPPU_TYPE_REF( I7 ),                                                    \
    CPPU_TYPE_REF( I8 ),                                                    \
    CPPU_TYPE_REF( I9 ),                                                    \
    CPPU_TYPE_REF( I10 ),                                                   \
    CPPU_TYPE_REF( I11 ),                                                   \
    CPPU_TYPE_REF( I12 ),                                                   \
    CPPU_TYPE_REF( I13 ),                                                   \
    CPPU_TYPE_REF( I14 )                                                    \
GETTYPES_IMPL_END

// 15 interfaces supported
#define XTYPEPROVIDER_IMPL_15( Class, I1,I2,I3,I4,I5,I6,I7,I8,I9,I10,I11,I12,I13,I14,I15 )  \
XTYPEPROVIDER_COMMON_IMPL( Class )                                          \
GETTYPES_IMPL_START( Class )                                                \
    CPPU_TYPE_REF( I1 ),                                                    \
    CPPU_TYPE_REF( I2 ),                                                    \
    CPPU_TYPE_REF( I3 ),                                                    \
    CPPU_TYPE_REF( I4 ),                                                    \
    CPPU_TYPE_REF( I5 ),                                                    \
    CPPU_TYPE_REF( I6 ),                                                    \
    CPPU_TYPE_REF( I7 ),                                                    \
    CPPU_TYPE_REF( I8 ),                                                    \
    CPPU_TYPE_REF( I9 ),                                                    \
    CPPU_TYPE_REF( I10 ),                                                   \
    CPPU_TYPE_REF( I11 ),                                                   \
    CPPU_TYPE_REF( I12 ),                                                   \
    CPPU_TYPE_REF( I13 ),                                                   \
    CPPU_TYPE_REF( I14 ),                                                   \
    CPPU_TYPE_REF( I15 )                                                    \
GETTYPES_IMPL_END

//=========================================================================
//
// XServiceInfo decl.
//
//=========================================================================

#define XSERVICEINFO_NOFACTORY_DECL()                                       \
    virtual rtl::OUString SAL_CALL                                          \
    getImplementationName()                                                 \
        throw( com::sun::star::uno::RuntimeException );                     \
    virtual sal_Bool SAL_CALL                                               \
    supportsService( const rtl::OUString& ServiceName )                     \
        throw( com::sun::star::uno::RuntimeException );                     \
    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL         \
    getSupportedServiceNames()                                              \
        throw( com::sun::star::uno::RuntimeException );                     \
                                                                            \
    static rtl::OUString                                                    \
    getImplementationName_Static();                                         \
    static com::sun::star::uno::Sequence< rtl::OUString >                   \
    getSupportedServiceNames_Static();

#define XSERVICEINFO_DECL()                                                 \
    XSERVICEINFO_NOFACTORY_DECL()                                           \
                                                                            \
    static com::sun::star::uno::Reference<                                  \
            com::sun::star::lang::XSingleServiceFactory >                   \
    createServiceFactory( const com::sun::star::uno::Reference<             \
            com::sun::star::lang::XMultiServiceFactory >& rxServiceMgr );

//=========================================================================
//
// XServiceInfo impl. internals
//
//=========================================================================

#define XSERVICEINFO_COMMOM_IMPL( Class, ImplName )                         \
rtl::OUString SAL_CALL Class::getImplementationName()                       \
    throw( com::sun::star::uno::RuntimeException )                          \
{                                                                           \
    return getImplementationName_Static();                                  \
}                                                                           \
                                                                            \
rtl::OUString Class::getImplementationName_Static()                         \
{                                                                           \
    return ImplName;                                                        \
}                                                                           \
                                                                            \
sal_Bool SAL_CALL                                                           \
Class::supportsService( const rtl::OUString& ServiceName )                  \
    throw( com::sun::star::uno::RuntimeException )                          \
{                                                                           \
    com::sun::star::uno::Sequence< rtl::OUString > aSNL =                   \
                                        getSupportedServiceNames();         \
    const rtl::OUString* pArray = aSNL.getArray();                          \
    for( sal_Int32 i = 0; i < aSNL.getLength(); i++ )                       \
    {                                                                       \
        if( pArray[ i ] == ServiceName )                                    \
            return sal_True;                                                \
    }                                                                       \
                                                                            \
    return sal_False;                                                       \
}                                                                           \
                                                                            \
com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL                     \
Class::getSupportedServiceNames()                                           \
    throw( com::sun::star::uno::RuntimeException )                          \
{                                                                           \
    return getSupportedServiceNames_Static();                               \
}

#define XSERVICEINFO_CREATE_INSTANCE_IMPL( Class )                          \
static com::sun::star::uno::Reference<                                      \
                                com::sun::star::uno::XInterface > SAL_CALL  \
Class##_CreateInstance( const com::sun::star::uno::Reference<               \
                com::sun::star::lang::XMultiServiceFactory> & rSMgr )       \
    throw( com::sun::star::uno::Exception )                                 \
{                                                                           \
    com::sun::star::lang::XServiceInfo* pX =                                \
                (com::sun::star::lang::XServiceInfo*)new Class( rSMgr );    \
    return com::sun::star::uno::Reference<                                  \
                            com::sun::star::uno::XInterface >::query( pX ); \
}

//=========================================================================
//
// XServiceInfo impl.
//
//=========================================================================

#define ONE_INSTANCE_SERVICE_FACTORY_IMPL( Class )                          \
com::sun::star::uno::Reference<                                             \
                        com::sun::star::lang::XSingleServiceFactory >       \
Class::createServiceFactory( const com::sun::star::uno::Reference<          \
            com::sun::star::lang::XMultiServiceFactory >& rxServiceMgr )    \
{                                                                           \
    return com::sun::star::uno::Reference<                                  \
        com::sun::star::lang::XSingleServiceFactory >(                      \
            cppu::createOneInstanceFactory(                                 \
                rxServiceMgr,                                               \
                Class::getImplementationName_Static(),                      \
                Class##_CreateInstance,                                     \
                Class::getSupportedServiceNames_Static() ) );               \
}

#define SINGLE_SERVICE_FACTORY_IMPL( Class )                                \
com::sun::star::uno::Reference<                                             \
                        com::sun::star::lang::XSingleServiceFactory >       \
Class::createServiceFactory( const com::sun::star::uno::Reference<          \
            com::sun::star::lang::XMultiServiceFactory >& rxServiceMgr )    \
{                                                                           \
    return com::sun::star::uno::Reference<                                  \
        com::sun::star::lang::XSingleServiceFactory >(                      \
            cppu::createSingleFactory(                                      \
                rxServiceMgr,                                               \
                Class::getImplementationName_Static(),                      \
                Class##_CreateInstance,                                     \
                Class::getSupportedServiceNames_Static() ) );               \
}

// Service without service factory.

// Own implementation of getSupportedServiceNames_Static().
#define XSERVICEINFO_NOFACTORY_IMPL_0( Class, ImplName )                    \
XSERVICEINFO_COMMOM_IMPL( Class, ImplName )                                 \
                                                                            \
com::sun::star::uno::Sequence< rtl::OUString >                              \
Class::getSupportedServiceNames_Static()

// 1 service name
#define XSERVICEINFO_NOFACTORY_IMPL_1( Class, ImplName, Service1 )          \
XSERVICEINFO_COMMOM_IMPL( Class, ImplName )                                 \
                                                                            \
com::sun::star::uno::Sequence< rtl::OUString >                              \
Class::getSupportedServiceNames_Static()                                    \
{                                                                           \
    com::sun::star::uno::Sequence< rtl::OUString > aSNS( 1 );               \
    aSNS.getArray()[ 0 ] = Service1;                                        \
    return aSNS;                                                            \
}

// Service with service factory.

// Own implementation of getSupportedServiceNames_Static().
#define XSERVICEINFO_IMPL_0( Class, ImplName )                              \
XSERVICEINFO_COMMOM_IMPL( Class, ImplName )                                 \
XSERVICEINFO_CREATE_INSTANCE_IMPL( Class )                                  \
                                                                            \
com::sun::star::uno::Sequence< rtl::OUString >                              \
Class::getSupportedServiceNames_Static()

// 1 service name
#define XSERVICEINFO_IMPL_1( Class, ImplName, Service1 )                    \
XSERVICEINFO_COMMOM_IMPL( Class, ImplName )                                 \
XSERVICEINFO_CREATE_INSTANCE_IMPL( Class )                                  \
                                                                            \
com::sun::star::uno::Sequence< rtl::OUString >                              \
Class::getSupportedServiceNames_Static()                                    \
{                                                                           \
    com::sun::star::uno::Sequence< rtl::OUString > aSNS( 1 );               \
    aSNS.getArray()[ 0 ] = Service1;                                        \
    return aSNS;                                                            \
}

#endif /* !_UCBHELPER_MACROS_HXX */
