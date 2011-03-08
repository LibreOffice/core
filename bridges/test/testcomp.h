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

#include <stdio.h>

#include <com/sun/star/bridge/XInstanceProvider.hpp>
#include <osl/thread.hxx>


void parseCommandLine( char *argv[] ,
                       ::rtl::OUString *pProtocol , ::rtl::OUString *pConnection ,
                       sal_Bool *pbLatency , sal_Bool *pbReverse);


Reference< XInterface > createComponent(
    const ::rtl::OUString &sServiceName,
    const ::rtl::OUString &sDllName,
    const Reference < XMultiServiceFactory > & rSMgr );

class OInterfaceTest :
    public ::cppu::OWeakObject,
    public XInterfaceTest
{
public:
    OInterfaceTest() {}
    ~OInterfaceTest() {}

public:
    // XInterface
    Any SAL_CALL queryInterface( const com::sun::star::uno::Type & aType) throw ( ::com::sun::star::uno::RuntimeException );
    void        SAL_CALL acquire() throw()                       { OWeakObject::acquire(); }
    void        SAL_CALL release() throw()                       { OWeakObject::release(); }

public:
    virtual void SAL_CALL setIn( const ::com::sun::star::uno::Reference< ::test::XCallMe >& callback ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setInOut( ::com::sun::star::uno::Reference< ::test::XCallMe >& callback ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL getOut( ::com::sun::star::uno::Reference< ::test::XCallMe >& callback ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::test::XCallMe > SAL_CALL get(  ) throw(::com::sun::star::uno::RuntimeException);
private:
    void call();

private:
    Reference < XCallMe > m_rCallMe;
};


class OCallMe :
    public ::cppu::OWeakObject,
    public XCallMe
{
public:
    OCallMe() : m_nLastToDos(-1) {}
    ~OCallMe() {}

public:
    // XInterface
    Any SAL_CALL queryInterface( const com::sun::star::uno::Type & aType) throw ( ::com::sun::star::uno::RuntimeException );
    void        SAL_CALL acquire()throw()                        { OWeakObject::acquire(); }
    void        SAL_CALL release()throw()                        { OWeakObject::release(); }
public:
    // XCallMe
    virtual void SAL_CALL call( const ::rtl::OUString& s, sal_Int32 nToDo )
        throw(::com::sun::star::uno::RuntimeException,
              ::test::TestBridgeException);
    virtual void SAL_CALL callOneway( const ::rtl::OUString& s, sal_Int32 nToDo )
        throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL drawLine( sal_Int32 x1, sal_Int32 y1 , sal_Int32 x2 , sal_Int32 y2 )
        throw(::com::sun::star::uno::RuntimeException);

    virtual ::rtl::OUString SAL_CALL getsAttribute() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setsAttribute( const ::rtl::OUString& _sattribute ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL callAgain( const ::com::sun::star::uno::Reference< ::test::XCallMe >& callAgain,
                                     sal_Int32 nToCall ) throw(::com::sun::star::uno::RuntimeException);

    virtual ::test::TestTypes SAL_CALL transport( const ::test::TestTypes& types )
        throw(::com::sun::star::uno::RuntimeException);

    ::osl::Mutex m_mutex;
    ::rtl::OUString m_sAttribute;
    sal_Int32 m_nLastToDos;
};

class OTestFactory :
    public ::cppu::OWeakObject,
    public XTestFactory
{
public:
    OTestFactory() {}
    ~OTestFactory() {}

public:
    // XInterface
    Any         SAL_CALL queryInterface( const  com::sun::star::uno::Type & aType ) throw ( ::com::sun::star::uno::RuntimeException );
    void        SAL_CALL acquire() throw()                       { OWeakObject::acquire(); }
    void        SAL_CALL release() throw()                       { OWeakObject::release(); }
public:
    virtual ::com::sun::star::uno::Reference< ::test::XCallMe > SAL_CALL createCallMe(  )
        throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::test::XInterfaceTest > SAL_CALL createInterfaceTest(  )
        throw(::com::sun::star::uno::RuntimeException);

};


class OInstanceProvider :
    public ::cppu::OWeakObject,
    public XInstanceProvider
{
public:
    OInstanceProvider( ){}
    OInstanceProvider( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & r ) :
        m_rSMgr( r )
        {}
    ~OInstanceProvider(){ printf( "instance provider dies\n" );}
public:
    // XInterface
    Any         SAL_CALL queryInterface( const Type & aType)throw ( ::com::sun::star::uno::RuntimeException );
    void        SAL_CALL acquire()throw()                        { OWeakObject::acquire(); }
    void        SAL_CALL release() throw()                       { OWeakObject::release(); }

public:
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
           getInstance( const ::rtl::OUString& sObjectName )
                 throw( ::com::sun::star::container::NoSuchElementException,
                        ::com::sun::star::uno::RuntimeException);

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_rSMgr;
};

void testRemote( const Reference< XInterface > &rRemote );

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
