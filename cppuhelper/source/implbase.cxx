/*************************************************************************
 *
 *  $RCSfile: implbase.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: vg $ $Date: 2003-07-11 10:41:51 $
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

#ifndef _CPPUHELPER_IMPLBASE_HXX_
#include <cppuhelper/implbase.hxx>
#endif
#ifndef _CPPUHELPER_COMPBASE_HXX_
#include <cppuhelper/compbase.hxx>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif

#include <com/sun/star/lang/XComponent.hpp>

using namespace ::osl;
using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

#if defined(MACOSX)
// Initialize static template data here to break circular reference to libstatic
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/reflection/XIdlMethod.hpp>
#include <com/sun/star/reflection/XIdlClass.hpp>
#include <com/sun/star/reflection/XMethodParameter.hpp>
#include <com/sun/star/reflection/XInterfaceMemberTypeDescription.hpp>
typelib_TypeDescriptionReference * com::sun::star::uno::Sequence<com::sun::star::uno::Type>::s_pType;
typelib_TypeDescriptionReference * com::sun::star::uno::Sequence<signed char>::s_pType;
typelib_TypeDescriptionReference * com::sun::star::uno::Sequence<com::sun::star::uno::Any>::s_pType;
typelib_TypeDescriptionReference * com::sun::star::uno::Sequence<com::sun::star::uno::Reference<com::sun::star::registry::XSimpleRegistry> >::s_pType;
typelib_TypeDescriptionReference * com::sun::star::uno::Sequence<com::sun::star::beans::PropertyChangeEvent>::s_pType;
typelib_TypeDescriptionReference * com::sun::star::uno::Sequence<com::sun::star::beans::Property>::s_pType;
typelib_TypeDescriptionReference * com::sun::star::uno::Sequence<com::sun::star::uno::Reference<com::sun::star::uno::XInterface> >::s_pType;
typelib_TypeDescriptionReference * com::sun::star::uno::Sequence<long>::s_pType;
typelib_TypeDescriptionReference * com::sun::star::uno::Sequence<rtl::OUString>::s_pType;
typelib_TypeDescriptionReference * com::sun::star::uno::Sequence<com::sun::star::uno::Reference<com::sun::star::reflection::XIdlMethod> >::s_pType;
typelib_TypeDescriptionReference * com::sun::star::uno::Sequence<com::sun::star::uno::Reference<com::sun::star::reflection::XIdlClass> >::s_pType;
typelib_TypeDescriptionReference * com::sun::star::uno::Sequence<com::sun::star::uno::Reference<com::sun::star::reflection::XIdlField> >::s_pType;
typelib_TypeDescriptionReference * com::sun::star::uno::Sequence<com::sun::star::uno::Reference<com::sun::star::registry::XRegistryKey> >::s_pType;
typelib_TypeDescriptionReference * com::sun::star::uno::Sequence<com::sun::star::uno::Reference<com::sun::star::reflection::XMethodParameter> >::s_pType;
typelib_TypeDescriptionReference * com::sun::star::uno::Sequence<com::sun::star::uno::Reference<com::sun::star::reflection::XTypeDescription> >::s_pType;
typelib_TypeDescriptionReference * com::sun::star::uno::Sequence<com::sun::star::uno::Reference<com::sun::star::reflection::XInterfaceMemberTypeDescription> >::s_pType;
#endif

namespace cppu
{
//==================================================================================================
Mutex & SAL_CALL getImplHelperInitMutex(void) SAL_THROW( () )
{
    static Mutex * s_pMutex = 0;
    if (! s_pMutex)
    {
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        if (! s_pMutex)
        {
            static Mutex s_aMutex;
            s_pMutex = & s_aMutex;
        }
    }
    return * s_pMutex;
}

// ClassDataBase
//__________________________________________________________________________________________________
ClassDataBase::ClassDataBase() SAL_THROW( () )
    : bOffsetsInit( sal_False )
    , nType2Offset( 0 )
    , nClassCode( 0 )
    , pTypes( 0 )
    , pId( 0 )
{
}
//__________________________________________________________________________________________________
ClassDataBase::ClassDataBase( sal_Int32 nClassCode_ ) SAL_THROW( () )
    : bOffsetsInit( sal_False )
    , nType2Offset( 0 )
    , nClassCode( nClassCode_ )
    , pTypes( 0 )
    , pId( 0 )
{
}
//__________________________________________________________________________________________________
ClassDataBase::~ClassDataBase() SAL_THROW( () )
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
void ClassData::writeTypeOffset( const Type & rType, sal_Int32 nOffset ) SAL_THROW( () )
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
        OSL_ENSURE( sal_False, msg.getStr() );
    }
#endif
}
//__________________________________________________________________________________________________
void ClassData::initTypeProvider() SAL_THROW( () )
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
Sequence< Type > ClassData::getTypes() SAL_THROW( () )
{
    if (! pTypes)
        initTypeProvider();
    return *pTypes;
}
//__________________________________________________________________________________________________
Sequence< sal_Int8 > ClassData::getImplementationId() SAL_THROW( () )
{
    if (! pTypes)
        initTypeProvider();
    return *pId;
}

//--------------------------------------------------------------------------------------------------
static inline sal_Bool td_equals(
    typelib_TypeDescription * pTD, typelib_TypeDescriptionReference * pType )
    SAL_THROW( () )
{
    return (pTD->pWeakRef == pType ||
            (pTD->pTypeName->length == pType->pTypeName->length &&
             rtl_ustr_compare( pTD->pTypeName->buffer, pType->pTypeName->buffer ) == 0));
}
//__________________________________________________________________________________________________
Any ClassData::query( const Type & rType, lang::XTypeProvider * pBase ) SAL_THROW( () )
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
    SAL_THROW( () )
    : rBHelper( rMutex )
{
}
//__________________________________________________________________________________________________
WeakComponentImplHelperBase::~WeakComponentImplHelperBase()
    SAL_THROW( () )
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
    if (1 == m_refCount && !rBHelper.bDisposed)
    {
        try
        {
            dispose();
        }
        catch (RuntimeException & exc) // don't break throw ()
        {
#if OSL_DEBUG_LEVEL > 0
            OString msg( OUStringToOString( exc.Message, RTL_TEXTENCODING_ASCII_US ) );
            OSL_ENSURE( 0, msg.getStr() );
#endif
        }
    }
    OWeakObject::release();
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
                MutexGuard aGuard( rBHelper.rMutex );
                // bDisposed and bInDispose must be set in this order:
                rBHelper.bDisposed = sal_True;
                rBHelper.bInDispose = sal_False;
                throw;
            }
            MutexGuard aGuard( rBHelper.rMutex );
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
    SAL_THROW( () )
    : rBHelper( rMutex )
{
}
//__________________________________________________________________________________________________
WeakAggComponentImplHelperBase::~WeakAggComponentImplHelperBase()
    SAL_THROW( () )
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
    Reference< XInterface > x( xDelegator );
    if (!x.is() && 1 == m_refCount && !rBHelper.bDisposed)
    {
        try
        {
            dispose();
        }
        catch (RuntimeException & exc) // don't break throw ()
        {
#if OSL_DEBUG_LEVEL > 0
            OString msg( OUStringToOString( exc.Message, RTL_TEXTENCODING_ASCII_US ) );
            OSL_ENSURE( 0, msg.getStr() );
#endif
        }
    }
    OWeakAggObject::release();
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
                MutexGuard aGuard( rBHelper.rMutex );
                // bDisposed and bInDispose must be set in this order:
                rBHelper.bDisposed = sal_True;
                rBHelper.bInDispose = sal_False;
                throw;
            }
            MutexGuard aGuard( rBHelper.rMutex );
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
