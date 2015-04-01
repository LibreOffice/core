/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

#include <osl/interlck.h>
#include <osl/mutex.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <my_module/XSomething.hpp>


using namespace ::rtl; // for OUString
using namespace ::com::sun::star; // for odk interfaces
using namespace ::com::sun::star::uno; // for basic types

namespace my_sc_impl
{

Sequence< OUString > SAL_CALL getSupportedServiceNames_MyService1Impl()
{
    Sequence< OUString > names(1);
    names[0] = "my_module.MyService1";
    return names;
}

OUString SAL_CALL getImplementationName_MyService1Impl()
{
    return OUString("my_module.my_sc_implementation.MyService1");
}


class MyService1Impl
    : public ::my_module::XSomething
    , public lang::XServiceInfo
    , public lang::XTypeProvider
{
    oslInterlockedCount m_refcount;
    OUString m_sData;
    // it's good practise to store the context for further use when you use
    // other UNO API's in your implementation
    Reference< XComponentContext > m_xContext;
public:
    inline MyService1Impl(Reference< XComponentContext > const & xContext) throw ()
        : m_refcount( 0 ),
          m_xContext(xContext)
        {}

    virtual ~MyService1Impl() {}

    // XInterface
    virtual Any SAL_CALL queryInterface( Type const & type )
        throw (RuntimeException);
    virtual void SAL_CALL acquire()
        throw ();
    virtual void SAL_CALL release()
        throw ();
    // XTypeProvider
    virtual Sequence< Type > SAL_CALL getTypes()
        throw (RuntimeException);
    virtual Sequence< sal_Int8 > SAL_CALL getImplementationId()
        throw (RuntimeException);
    // XSomething
    virtual OUString SAL_CALL methodOne( OUString const & str )
        throw (RuntimeException);
    virtual OUString SAL_CALL methodTwo( )
        throw (RuntimeException);
    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw (RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( OUString const & serviceName )
        throw (RuntimeException);
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw (RuntimeException);
};

// XInterface implementation
Any MyService1Impl::queryInterface( Type const & type )
    throw (RuntimeException)
{
    if (type.equals(::cppu::UnoType<XInterface>::get()))
    {
        // return XInterface interface
        // (resolve ambiguity by casting to lang::XTypeProvider)
        Reference< XInterface > x(
            static_cast< lang::XTypeProvider * >( this ) );
        return makeAny( x );
    }
    if (type.equals(::cppu::UnoType<lang::XTypeProvider>::get()))
    {
        // return XInterface interface
        Reference< XInterface > x(
            static_cast< lang::XTypeProvider * >( this ) );
        return makeAny( x );
    }
    if (type.equals(::cppu::UnoType<lang::XServiceInfo>::get()))
    {
        // return XServiceInfo interface
        Reference< lang::XServiceInfo > x(
            static_cast< lang::XServiceInfo * >( this ) );
        return makeAny( x );
    }
    if (type.equals(::cppu::UnoType<my_module::XSomething>::get()))
    {
        // return sample interface
        Reference< ::my_module::XSomething > x(
            static_cast< ::my_module::XSomething * >( this ) );
        return makeAny( x );
    }
    // querying for unsupported type
    return Any();
}

void MyService1Impl::acquire()
    throw ()
{
    // thread-safe incrementation of reference count
    ::osl_atomic_increment( &m_refcount );
}

void MyService1Impl::release()
    throw ()
{
    // thread-safe decrementation of reference count
    if (0 == ::osl_atomic_decrement( &m_refcount ))
    {
        delete this; // shutdown this object
    }
}

// XTypeProvider implementation
Sequence< Type > MyService1Impl::getTypes()
    throw (RuntimeException)
{
    Sequence< Type > seq( 3 );
    seq[ 0 ] = ::cppu::UnoType<lang::XTypeProvider>::get();
    seq[ 1 ] = ::cppu::UnoType<lang::XServiceInfo>::get();
    seq[ 2 ] = ::cppu::UnoType<my_module::XSomething>::get();
    return seq;
}

Sequence< sal_Int8 > MyService1Impl::getImplementationId()
    throw (RuntimeException)
{
    return css::uno::Sequence<sal_Int8>();
}

// XSomething implementation
OUString MyService1Impl::methodOne( OUString const & str )
    throw (RuntimeException)
{
    m_sData = str;
    return OUString( "called methodOne() of MyService1 implementation: " ) + m_sData;
}

OUString MyService1Impl::methodTwo( )
    throw (RuntimeException)
{
    return OUString( "called methodTwo() of MyService1 implementation: " ) + m_sData;
}

// XServiceInfo implementation
OUString MyService1Impl::getImplementationName()
    throw (RuntimeException)
{
    // unique implementation name
    return OUString("my_module.my_sc_implementation.MyService1");
}
sal_Bool MyService1Impl::supportsService( OUString const & serviceName )
    throw (RuntimeException)
{
    return cppu::supportsService(this, serviceName);
}
Sequence< OUString > MyService1Impl::getSupportedServiceNames()
    throw (RuntimeException)
{
    // this object only supports one service
    OUString serviceName("my_module.MyService1");
    return Sequence< OUString >( &serviceName, 1 );
}

Reference< XInterface > SAL_CALL create_MyService1Impl(
    Reference< XComponentContext > const & xContext )
{
    return static_cast< lang::XTypeProvider * >( new MyService1Impl( xContext) );
}

// forward decl: implemented in service2_impl.cxx
Reference< XInterface > SAL_CALL create_MyService2Impl(
    Reference< XComponentContext > const & );

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
