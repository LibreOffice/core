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

#include <stdio.h>

#include <com/sun/star/bridge/XInstanceProvider.hpp>
#include <osl/thread.hxx>


void parseCommandLine( char *argv[] ,
                       OUString *pProtocol , OUString *pConnection ,
                       sal_Bool *pbLatency , sal_Bool *pbReverse);


Reference< XInterface > createComponent(
    const OUString &sServiceName,
    const OUString &sDllName,
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
    virtual void SAL_CALL call( const OUString& s, sal_Int32 nToDo )
        throw(::com::sun::star::uno::RuntimeException,
              ::test::TestBridgeException);
    virtual void SAL_CALL callOneway( const OUString& s, sal_Int32 nToDo )
        throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL drawLine( sal_Int32 x1, sal_Int32 y1 , sal_Int32 x2 , sal_Int32 y2 )
        throw(::com::sun::star::uno::RuntimeException);

    virtual OUString SAL_CALL getsAttribute() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setsAttribute( const OUString& _sattribute ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL callAgain( const ::com::sun::star::uno::Reference< ::test::XCallMe >& callAgain,
                                     sal_Int32 nToCall ) throw(::com::sun::star::uno::RuntimeException);

    virtual ::test::TestTypes SAL_CALL transport( const ::test::TestTypes& types )
        throw(::com::sun::star::uno::RuntimeException);

    ::osl::Mutex m_mutex;
    OUString m_sAttribute;
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
           getInstance( const OUString& sObjectName )
                 throw( ::com::sun::star::container::NoSuchElementException,
                        ::com::sun::star::uno::RuntimeException);

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_rSMgr;
};

void testRemote( const Reference< XInterface > &rRemote );

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
