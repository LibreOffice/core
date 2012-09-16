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

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/compbase.hxx>
#include <osl/diagnose.h>
#include <rtl/instance.hxx>
#include <rtl/uuid.h>

#include <com/sun/star/lang/XComponent.hpp>
#include "com/sun/star/uno/RuntimeException.hpp"

using namespace ::osl;
using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace
{
    class theImplHelperInitMutex : public rtl::Static<Mutex, theImplHelperInitMutex>{};
}

namespace cppu
{
//==================================================================================================
Mutex & SAL_CALL getImplHelperInitMutex(void) SAL_THROW(())
{
    return theImplHelperInitMutex::get();
}

// ClassDataBase
//__________________________________________________________________________________________________
ClassDataBase::ClassDataBase() SAL_THROW(())
    : bOffsetsInit( sal_False )
    , nType2Offset( 0 )
    , nClassCode( 0 )
    , pTypes( 0 )
    , pId( 0 )
{
}
//__________________________________________________________________________________________________
ClassDataBase::ClassDataBase( sal_Int32 nClassCode_ ) SAL_THROW(())
    : bOffsetsInit( sal_False )
    , nType2Offset( 0 )
    , nClassCode( nClassCode_ )
    , pTypes( 0 )
    , pId( 0 )
{
}
//__________________________________________________________________________________________________
ClassDataBase::~ClassDataBase() SAL_THROW(())
{
    delete pTypes;
    delete pId;

    for ( sal_Int32 nPos = nType2Offset; nPos--; )
    {
        typelib_typedescription_release(
            (typelib_TypeDescription *)((ClassData *)this)->arType2Offset[nPos].pTD );
    }
}

// ClassData
//__________________________________________________________________________________________________
void ClassData::writeTypeOffset( const Type & rType, sal_Int32 nOffset ) SAL_THROW(())
{
    arType2Offset[nType2Offset].nOffset = nOffset;

    arType2Offset[nType2Offset].pTD = 0;
    typelib_typedescriptionreference_getDescription(
        (typelib_TypeDescription **)&arType2Offset[nType2Offset].pTD, rType.getTypeLibType() );

    if (arType2Offset[nType2Offset].pTD)
        ++nType2Offset;
#if OSL_DEBUG_LEVEL > 1
    else
    {
        OString msg( "### cannot get type description for " );
        msg += OUStringToOString( rType.getTypeName(), RTL_TEXTENCODING_ASCII_US );
        OSL_FAIL( msg.getStr() );
    }
#endif
}
//__________________________________________________________________________________________________
void ClassData::initTypeProvider() SAL_THROW(())
{
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
    if (! pTypes)
    {
        // create id
        pId = new Sequence< sal_Int8 >( 16 );
        rtl_createUuid( (sal_uInt8 *)pId->getArray(), 0, sal_True );

        // collect types
        Sequence< Type > * types = new Sequence< Type >(
            nType2Offset + 1 + (nClassCode == 4 ? 2 : nClassCode) );
        Type * pTypeAr = types->getArray();

        // given types
        sal_Int32 nPos = nType2Offset;
        while (nPos--)
            pTypeAr[nPos] = ((typelib_TypeDescription *)arType2Offset[nPos].pTD)->pWeakRef;

        // XTypeProvider
        pTypeAr[nType2Offset] = ::getCppuType( (const Reference< lang::XTypeProvider > *)0 );

        // class code extra types: [[XComponent,] XWeak[, XAggregation]]
        switch (nClassCode)
        {
        case 4:
            pTypeAr[nType2Offset +2] = ::getCppuType( (const Reference< lang::XComponent > *)0 );
            pTypeAr[nType2Offset +1] = ::getCppuType( (const Reference< XWeak > *)0 );
            break;
        case 3:
            pTypeAr[nType2Offset +3] = ::getCppuType( (const Reference< lang::XComponent > *)0 );
        case 2:
            pTypeAr[nType2Offset +2] = ::getCppuType( (const Reference< XAggregation > *)0 );
        case 1:
            pTypeAr[nType2Offset +1] = ::getCppuType( (const Reference< XWeak > *)0 );
        }

        pTypes = types;
    }
}
//__________________________________________________________________________________________________
Sequence< Type > ClassData::getTypes() SAL_THROW(())
{
    if (! pTypes)
        initTypeProvider();
    return *pTypes;
}
//__________________________________________________________________________________________________
Sequence< sal_Int8 > ClassData::getImplementationId() SAL_THROW(())
{
    if (! pTypes)
        initTypeProvider();
    return *pId;
}

//--------------------------------------------------------------------------------------------------
static inline sal_Bool td_equals(
    typelib_TypeDescription * pTD, typelib_TypeDescriptionReference * pType )
    SAL_THROW(())
{
    return (pTD->pWeakRef == pType ||
            (pTD->pTypeName->length == pType->pTypeName->length &&
             rtl_ustr_compare( pTD->pTypeName->buffer, pType->pTypeName->buffer ) == 0));
}
//__________________________________________________________________________________________________
Any ClassData::query( const Type & rType, lang::XTypeProvider * pBase ) SAL_THROW(())
{
    if (rType == ::getCppuType( (const Reference< XInterface > *)0 ))
        return Any( &pBase, ::getCppuType( (const Reference< XInterface > *)0 ) );
    for ( sal_Int32 nPos = 0; nPos < nType2Offset; ++nPos )
    {
        const Type_Offset & rTO = arType2Offset[nPos];
        typelib_InterfaceTypeDescription * pTD = rTO.pTD;
        while (pTD)
        {
            if (td_equals( (typelib_TypeDescription *)pTD,
                           *(typelib_TypeDescriptionReference **)&rType ))
            {
                void * pInterface = (char *)pBase + rTO.nOffset;
                return Any( &pInterface, (typelib_TypeDescription *)pTD );
            }
            pTD = pTD->pBaseTypeDescription;
        }
    }
    if (rType == ::getCppuType( (const Reference< lang::XTypeProvider > *)0 ))
        return Any( &pBase, ::getCppuType( (const Reference< lang::XTypeProvider > *)0 ) );

    return Any();
}

//##################################################################################################
//##################################################################################################
//##################################################################################################

// WeakComponentImplHelperBase
//__________________________________________________________________________________________________
WeakComponentImplHelperBase::WeakComponentImplHelperBase( Mutex & rMutex )
    SAL_THROW(())
    : rBHelper( rMutex )
{
}
//__________________________________________________________________________________________________
WeakComponentImplHelperBase::~WeakComponentImplHelperBase()
    SAL_THROW(())
{
}
//__________________________________________________________________________________________________
void WeakComponentImplHelperBase::disposing()
{
}
//__________________________________________________________________________________________________
Any WeakComponentImplHelperBase::queryInterface( Type const & rType )
    throw (RuntimeException)
{
    if (rType == ::getCppuType( (Reference< lang::XComponent > const *)0 ))
    {
        void * p = static_cast< lang::XComponent * >( this );
        return Any( &p, rType );
    }
    return OWeakObject::queryInterface( rType );
}
//__________________________________________________________________________________________________
void WeakComponentImplHelperBase::acquire()
    throw ()
{
    OWeakObject::acquire();
}
//__________________________________________________________________________________________________
void WeakComponentImplHelperBase::release()
    throw ()
{
    if (osl_atomic_decrement( &m_refCount ) == 0) {
        // ensure no other references are created, via the weak connection point, from now on
        disposeWeakConnectionPoint();
        // restore reference count:
        osl_atomic_increment( &m_refCount );
        if (! rBHelper.bDisposed) {
            try {
                dispose();
            }
            catch (RuntimeException const& exc) { // don't break throw ()
                OSL_FAIL(
                    OUStringToOString(
                        exc.Message, RTL_TEXTENCODING_ASCII_US ).getStr() );
                static_cast<void>(exc);
            }
            OSL_ASSERT( rBHelper.bDisposed );
        }
        OWeakObject::release();
    }
}
//__________________________________________________________________________________________________
void WeakComponentImplHelperBase::dispose()
    throw (RuntimeException)
{
    ClearableMutexGuard aGuard( rBHelper.rMutex );
    if (!rBHelper.bDisposed && !rBHelper.bInDispose)
    {
        rBHelper.bInDispose = sal_True;
        aGuard.clear();
        try
        {
            // side effect: keeping a reference to this
            lang::EventObject aEvt( static_cast< OWeakObject * >( this ) );
            try
            {
                rBHelper.aLC.disposeAndClear( aEvt );
                disposing();
            }
            catch (...)
            {
                MutexGuard aGuard2( rBHelper.rMutex );
                // bDisposed and bInDispose must be set in this order:
                rBHelper.bDisposed = sal_True;
                rBHelper.bInDispose = sal_False;
                throw;
            }
            MutexGuard aGuard2( rBHelper.rMutex );
            // bDisposed and bInDispose must be set in this order:
            rBHelper.bDisposed = sal_True;
            rBHelper.bInDispose = sal_False;
        }
        catch (RuntimeException &)
        {
            throw;
        }
        catch (Exception & exc)
        {
            throw RuntimeException(
                OUString( RTL_CONSTASCII_USTRINGPARAM(
                              "unexpected UNO exception caught: ") ) +
                exc.Message, Reference< XInterface >() );
        }
    }
}
//__________________________________________________________________________________________________
void WeakComponentImplHelperBase::addEventListener(
    Reference< lang::XEventListener > const & xListener )
    throw (RuntimeException)
{
    ClearableMutexGuard aGuard( rBHelper.rMutex );
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        aGuard.clear();
        lang::EventObject aEvt( static_cast< OWeakObject * >( this ) );
        xListener->disposing( aEvt );
    }
    else
    {
        rBHelper.addListener( ::getCppuType( &xListener ), xListener );
    }
}
//__________________________________________________________________________________________________
void WeakComponentImplHelperBase::removeEventListener(
    Reference< lang::XEventListener > const & xListener )
    throw (RuntimeException)
{
    rBHelper.removeListener( ::getCppuType( &xListener ), xListener );
}

// WeakAggComponentImplHelperBase
//__________________________________________________________________________________________________
WeakAggComponentImplHelperBase::WeakAggComponentImplHelperBase( Mutex & rMutex )
    SAL_THROW(())
    : rBHelper( rMutex )
{
}
//__________________________________________________________________________________________________
WeakAggComponentImplHelperBase::~WeakAggComponentImplHelperBase()
    SAL_THROW(())
{
}
//__________________________________________________________________________________________________
void WeakAggComponentImplHelperBase::disposing()
{
}
//__________________________________________________________________________________________________
Any WeakAggComponentImplHelperBase::queryInterface( Type const & rType )
    throw (RuntimeException)
{
    return OWeakAggObject::queryInterface( rType );
}
//__________________________________________________________________________________________________
Any WeakAggComponentImplHelperBase::queryAggregation( Type const & rType )
    throw (RuntimeException)
{
    if (rType == ::getCppuType( (Reference< lang::XComponent > const *)0 ))
    {
        void * p = static_cast< lang::XComponent * >( this );
        return Any( &p, rType );
    }
    return OWeakAggObject::queryAggregation( rType );
}
//__________________________________________________________________________________________________
void WeakAggComponentImplHelperBase::acquire()
    throw ()
{
    OWeakAggObject::acquire();
}
//__________________________________________________________________________________________________
void WeakAggComponentImplHelperBase::release()
    throw ()
{
    Reference<XInterface> const xDelegator_(xDelegator);
    if (xDelegator_.is()) {
        OWeakAggObject::release();
    }
    else if (osl_atomic_decrement( &m_refCount ) == 0) {
        // ensure no other references are created, via the weak connection point, from now on
        disposeWeakConnectionPoint();
        // restore reference count:
        osl_atomic_increment( &m_refCount );
        if (! rBHelper.bDisposed) {
            try {
                dispose();
            }
            catch (RuntimeException const& exc) { // don't break throw ()
                OSL_FAIL(
                    OUStringToOString(
                        exc.Message, RTL_TEXTENCODING_ASCII_US ).getStr() );
                static_cast<void>(exc);
            }
            OSL_ASSERT( rBHelper.bDisposed );
        }
        OWeakAggObject::release();
    }
}
//__________________________________________________________________________________________________
void WeakAggComponentImplHelperBase::dispose()
    throw (RuntimeException)
{
    ClearableMutexGuard aGuard( rBHelper.rMutex );
    if (!rBHelper.bDisposed && !rBHelper.bInDispose)
    {
        rBHelper.bInDispose = sal_True;
        aGuard.clear();
        try
        {
            // side effect: keeping a reference to this
            lang::EventObject aEvt( static_cast< OWeakObject * >( this ) );
            try
            {
                rBHelper.aLC.disposeAndClear( aEvt );
                disposing();
            }
            catch (...)
            {
                MutexGuard aGuard2( rBHelper.rMutex );
                // bDisposed and bInDispose must be set in this order:
                rBHelper.bDisposed = sal_True;
                rBHelper.bInDispose = sal_False;
                throw;
            }
            MutexGuard aGuard2( rBHelper.rMutex );
            // bDisposed and bInDispose must be set in this order:
            rBHelper.bDisposed = sal_True;
            rBHelper.bInDispose = sal_False;
        }
        catch (RuntimeException &)
        {
            throw;
        }
        catch (Exception & exc)
        {
            throw RuntimeException(
                OUString( RTL_CONSTASCII_USTRINGPARAM(
                              "unexpected UNO exception caught: ") ) +
                exc.Message, Reference< XInterface >() );
        }
    }
}
//__________________________________________________________________________________________________
void WeakAggComponentImplHelperBase::addEventListener(
    Reference< lang::XEventListener > const & xListener )
    throw (RuntimeException)
{
    ClearableMutexGuard aGuard( rBHelper.rMutex );
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        aGuard.clear();
        lang::EventObject aEvt( static_cast< OWeakObject * >( this ) );
        xListener->disposing( aEvt );
    }
    else
    {
        rBHelper.addListener( ::getCppuType( &xListener ), xListener );
    }
}
//__________________________________________________________________________________________________
void WeakAggComponentImplHelperBase::removeEventListener(
    Reference< lang::XEventListener > const & xListener )
    throw (RuntimeException)
{
    rBHelper.removeListener( ::getCppuType( &xListener ), xListener );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
