/*************************************************************************
 *
 *  $RCSfile: testcomp.h,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:28:51 $
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
//#include <com/sun/star/bridge/XServer.hpp>
//#include <com/sun/star/bridge/XClient.hpp>
#include <stdio.h>

#include <com/sun/star/bridge/XInstanceProvider.hpp>
//#include <com/sun/star/bridge/XConnectionAdministration.hpp>
#include <vos/thread.hxx>
#include <vos/timer.hxx>


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
    Any SAL_CALL queryInterface( const com::sun::star::uno::Type & aType);
    void        SAL_CALL acquire()                       { OWeakObject::acquire(); }
    void        SAL_CALL release()                       { OWeakObject::release(); }

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
    Any SAL_CALL queryInterface( const com::sun::star::uno::Type & aType);
    void        SAL_CALL acquire()                       { OWeakObject::acquire(); }
    void        SAL_CALL release()                       { OWeakObject::release(); }
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
    Any         SAL_CALL queryInterface( const  com::sun::star::uno::Type & aType );
    void        SAL_CALL acquire()                       { OWeakObject::acquire(); }
    void        SAL_CALL release()                       { OWeakObject::release(); }
public:
    virtual ::com::sun::star::uno::Reference< ::test::XCallMe > SAL_CALL createCallMe(  )
        throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::test::XInterfaceTest > SAL_CALL createInterfaceTest(  )
        throw(::com::sun::star::uno::RuntimeException);

};

/*
class OConnectCallback :
    public ::cppu::OWeakObject,
    public XConnectCallback
{
public:
    OConnectCallback( ){}
    ~OConnectCallback(){ printf( "callback dies\n" );}
public:
    // XInterface
    Any         SAL_CALL queryInterface( const Type & aType);
    void        SAL_CALL acquire()                       { OWeakObject::acquire(); }
    void        SAL_CALL release()                       { OWeakObject::release(); }

public:
    // XConnectCallback
    virtual void SAL_CALL attemptConnect(
        const Reference< XConnectionServerSide >& connection )
        throw(SecurityException, RuntimeException);
};

*/

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
    Any         SAL_CALL queryInterface( const Type & aType);
    void        SAL_CALL acquire()                       { OWeakObject::acquire(); }
    void        SAL_CALL release()                       { OWeakObject::release(); }

public:
    // XConnectCallback
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
           getInstance( const ::rtl::OUString& sObjectName )
                 throw( ::com::sun::star::container::NoSuchElementException,
                        ::com::sun::star::uno::RuntimeException);

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_rSMgr;
};

void testRemote( const Reference< XInterface > &rRemote );
