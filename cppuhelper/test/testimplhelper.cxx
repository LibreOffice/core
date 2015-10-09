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

#if !defined(OSL_DEBUG_LEVEL) || OSL_DEBUG_LEVEL == 0
# undef OSL_DEBUG_LEVEL
# define OSL_DEBUG_LEVEL 2
#endif


#include <stdio.h>

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/compbase.hxx>

#include <osl/diagnose.h>
#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/exc_hlp.hxx>

#include <test/A.hpp>
#include <test/BA.hpp>
#include <test/CA.hpp>
#include <test/DBA.hpp>
#include <test/E.hpp>
#include <test/FE.hpp>
#include <test/G.hpp>
#include <test/H.hpp>
#include <test/I.hpp>

#include <com/sun/star/lang/IllegalAccessException.hpp>

using namespace test;
using namespace osl;
using namespace cppu;
using namespace com::sun::star;
using namespace com::sun::star::uno;

using ::rtl::OUString;
using ::rtl::OUStringToOString;
using ::rtl::OString;


struct TestImpl : public ImplHelper< CA, DBA, FE, G >
{
    sal_Int32 nRef;

    virtual ~TestImpl()
        { OSL_TRACE( "> TestImpl dtor called... <\n" ); }

    virtual Any SAL_CALL queryInterface( const Type & rType ) throw(RuntimeException)
        { return ImplHelper< CA, DBA, FE, G >::queryInterface( rType ); }
    virtual void SAL_CALL acquire(  ) throw()
        { ++nRef; }
    virtual void SAL_CALL release(  ) throw()
        { if (! --nRef) delete this; }

    // A
    virtual OUString SAL_CALL a() throw(RuntimeException)
        { return OUString("a"); }
    // BA
    virtual OUString SAL_CALL ba() throw(RuntimeException)
        { return OUString("ba"); }
    // CA
    virtual OUString SAL_CALL ca() throw(RuntimeException)
        { return OUString("ca"); }
    // DBA
    virtual OUString SAL_CALL dba() throw(RuntimeException)
        { return OUString("dba"); }
    // E
    virtual OUString SAL_CALL e() throw(RuntimeException)
        { return OUString("e"); }
    // FE
    virtual OUString SAL_CALL fe() throw(RuntimeException)
        { return OUString("fe"); }
    // G
    virtual OUString SAL_CALL g() throw(RuntimeException)
        { return OUString("g"); }
};


struct TestWeakAggImpl : public WeakAggImplHelper< CA, DBA, FE, G >
{
    virtual ~TestWeakAggImpl()
        { OSL_TRACE( "> TestWeakAggImpl dtor called... <\n" ); }

    // A
    virtual OUString SAL_CALL a() throw(RuntimeException)
        { return OUString("a"); }
    // BA
    virtual OUString SAL_CALL ba() throw(RuntimeException)
        { return OUString("ba"); }
    // CA
    virtual OUString SAL_CALL ca() throw(RuntimeException)
        { return OUString("ca"); }
    // DBA
    virtual OUString SAL_CALL dba() throw(RuntimeException)
        { return OUString("dba"); }
    // E
    virtual OUString SAL_CALL e() throw(RuntimeException)
        { return OUString("e"); }
    // FE
    virtual OUString SAL_CALL fe() throw(RuntimeException)
        { return OUString("fe"); }
    // G
    virtual OUString SAL_CALL g() throw(RuntimeException)
        { return OUString("g"); }
};


struct TestWeakImpl : public WeakImplHelper< CA, DBA, FE, G >
{
    TestWeakImpl() {}

    virtual ~TestWeakImpl()
        { OSL_TRACE( "> TestWeakImpl dtor called... <\n" ); }

    // A
    virtual OUString SAL_CALL a() throw(RuntimeException)
        { return OUString("a"); }
    // BA
    virtual OUString SAL_CALL ba() throw(RuntimeException)
        { return OUString("ba"); }
    // CA
    virtual OUString SAL_CALL ca() throw(RuntimeException)
        { return OUString("ca"); }
    // DBA
    virtual OUString SAL_CALL dba() throw(RuntimeException)
        { return OUString("dba"); }
    // E
    virtual OUString SAL_CALL e() throw(RuntimeException)
        { return OUString("e"); }
    // FE
    virtual OUString SAL_CALL fe() throw(RuntimeException)
        { return OUString("fe"); }
    // G
    virtual OUString SAL_CALL g() throw(RuntimeException)
        { return OUString("g"); }

protected:
    explicit TestWeakImpl(int) {}

    TestWeakImpl(int, int) {}

    TestWeakImpl(int, int, int) {}

    TestWeakImpl(int, int, int, int) {}

    TestWeakImpl(int, int, int, int, int) {}

    TestWeakImpl(int, int, int, int, int, int, int = 0) {}
};


struct TestWeakComponentImpl : public WeakComponentImplHelper< CA, DBA, FE, G >
{
    Mutex m;
    TestWeakComponentImpl()
        : WeakComponentImplHelper< CA, DBA, FE, G >( m )
        {}
    virtual ~TestWeakComponentImpl()
        { OSL_TRACE( "> TestWeakComponentImpl dtor called... <\n" ); }

    void SAL_CALL disposing()
        { OSL_TRACE( "> TestWeakComponentImpl disposing called... <\n" ); }

    // A
    virtual OUString SAL_CALL a() throw(RuntimeException)
        { return OUString("a"); }
    // BA
    virtual OUString SAL_CALL ba() throw(RuntimeException)
        { return OUString("ba"); }
    // CA
    virtual OUString SAL_CALL ca() throw(RuntimeException)
        { return OUString("ca"); }
    // DBA
    virtual OUString SAL_CALL dba() throw(RuntimeException)
        { return OUString("dba"); }
    // E
    virtual OUString SAL_CALL e() throw(RuntimeException)
        { return OUString("e"); }
    // FE
    virtual OUString SAL_CALL fe() throw(RuntimeException)
        { return OUString("fe"); }
    // G
    virtual OUString SAL_CALL g() throw(RuntimeException)
        { return OUString("g"); }
};


struct TestWeakAggComponentImpl : public WeakAggComponentImplHelper< CA, DBA, FE, G >
{
    Mutex m;
    TestWeakAggComponentImpl()
        : WeakAggComponentImplHelper< CA, DBA, FE, G >( m )
        {}
    virtual ~TestWeakAggComponentImpl()
        { OSL_TRACE( "> TestWeakAggComponentImpl dtor called... <\n" ); }

    void SAL_CALL disposing()
        { OSL_TRACE( "> TestWeakAggComponentImpl disposing called... <\n" ); }

    // A
    virtual OUString SAL_CALL a() throw(RuntimeException)
        { return OUString("a"); }
    // BA
    virtual OUString SAL_CALL ba() throw(RuntimeException)
        { return OUString("ba"); }
    // CA
    virtual OUString SAL_CALL ca() throw(RuntimeException)
        { return OUString("ca"); }
    // DBA
    virtual OUString SAL_CALL dba() throw(RuntimeException)
        { return OUString("dba"); }
    // E
    virtual OUString SAL_CALL e() throw(RuntimeException)
        { return OUString("e"); }
    // FE
    virtual OUString SAL_CALL fe() throw(RuntimeException)
        { return OUString("fe"); }
    // G
    virtual OUString SAL_CALL g() throw(RuntimeException)
        { return OUString("g"); }
};


struct TestImplInh : public ImplInheritanceHelper< TestWeakImpl, H, I >
{
    TestImplInh() {}

    explicit TestImplInh(int):
        ImplInheritanceHelper< TestWeakImpl, H, I >(1) {}

    TestImplInh(int, int):
        ImplInheritanceHelper< TestWeakImpl, H, I >(1, 2) {}

    TestImplInh(int, int, int):
        ImplInheritanceHelper< TestWeakImpl, H, I >(1, 2, 3) {}

    TestImplInh(int, int, int, int):
        ImplInheritanceHelper< TestWeakImpl, H, I >(1, 2, 3, 4) {}

    TestImplInh(int, int, int, int, int):
        ImplInheritanceHelper< TestWeakImpl, H, I >(1, 2, 3, 4, 5) {}

    TestImplInh(int, int, int, int, int, int):
        ImplInheritanceHelper< TestWeakImpl, H, I >(1, 2, 3, 4, 5, 6) {}

    virtual ~TestImplInh()
        { OSL_TRACE( "> TestWeakImplInh dtor called... <\n" ); }

    // H
    virtual OUString SAL_CALL h() throw(RuntimeException)
        { return OUString("h"); }
    // I
    virtual OUString SAL_CALL i() throw(RuntimeException)
        { return OUString("i"); }
};


struct TestAggImplInh : public AggImplInheritanceHelper< TestWeakAggImpl, H, I >
{
    virtual ~TestAggImplInh()
        { OSL_TRACE( "> TestAggImplInh dtor called... <\n" ); }

    // H
    virtual OUString SAL_CALL h() throw(RuntimeException)
        { return OUString("h2"); }
    // I
    virtual OUString SAL_CALL i() throw(RuntimeException)
        { return OUString("i2"); }
};


static bool isIn( Sequence< Type > const & rTypes, char const * name )
{
    OUString str( OUString::createFromAscii( name ) );
    Type const * pTypes = rTypes.getConstArray();
    for ( sal_Int32 nPos = rTypes.getLength(); nPos--; )
    {
        if (pTypes[ nPos ].getTypeName().equals( str ))
            return true;
    }
    return false;
}


static void dotest( const Reference< XInterface > & xOriginal )
{
    Reference< lang::XTypeProvider > xTP( xOriginal, UNO_QUERY );
    Sequence< sal_Int8 > id( xTP->getImplementationId() );
    Sequence< Type > types( xTP->getTypes() );
    ::fprintf( stderr, "> supported types: " );
    long n = 0;
    for ( ; n < types.getLength(); ++n )
    {
        OString str( OUStringToOString( types[ n ].getTypeName(), RTL_TEXTENCODING_ASCII_US ) );
        ::fprintf( stderr, (n < (types.getLength()-1)) ? "%s, " : "%s; type-id=\n", str.getStr() );
    }
    for ( n = 0; n < 16; ++n )
    {
        ::fprintf( stderr, n < 15 ? "%x " : "%x \n", id[ n ] );
    }

    Reference< A > xa( xOriginal, UNO_QUERY );
    OSL_ENSURE( xa->a() == "a", "### A failed!" );
    Reference< BA > xba( xa, UNO_QUERY );
    OSL_ENSURE( xba->ba() == "ba", "### BA failed!" );
    OSL_ENSURE( xba->a() == "a", "### BA failed!" );
    Reference< CA > xca( xba, UNO_QUERY );
    OSL_ENSURE( xca->ca() == "ca", "### CA failed!" );
    OSL_ENSURE( xca->a() == "a", "### CA failed!" );
    Reference< DBA > xdba( xca, UNO_QUERY );
    OSL_ENSURE( xdba->dba() == "dba", "### DBA failed!" );
    OSL_ENSURE( xdba->ba() == "ba", "### DBA failed!" );
    OSL_ENSURE( xdba->a() == "a", "### DBA failed!" );
    Reference< E > xe( xdba, UNO_QUERY );
    OSL_ENSURE( xe->e() == "e", "### E failed!" );
    Reference< FE > xfe( xe, UNO_QUERY );
    OSL_ENSURE( xfe->fe() == "fe", "### FE failed!" );
    Reference< G > xg( xfe, UNO_QUERY );
    OSL_ENSURE( xg->g() == "g", "### G failed!" );
    OSL_ENSURE( xg->a() == "a", "### G failed!" );

    // type provider
    Reference< lang::XTypeProvider > xProv( xg, UNO_QUERY );
    Sequence< Type > aTypes( xProv->getTypes() );

    // CA, DBA, FE, G, XTypeProvider
    OSL_ASSERT( isIn( aTypes, "test.CA" ) );
    OSL_ASSERT( isIn( aTypes, "test.DBA" ) );
    OSL_ASSERT( isIn( aTypes, "test.FE") );
    OSL_ASSERT( isIn( aTypes, "test.G") );
    OSL_ASSERT( isIn( aTypes, "com.sun.star.lang.XTypeProvider") );

    Reference< XWeak > xWeak( xg, UNO_QUERY );
    if (xWeak.is())
    {
        OSL_ASSERT( isIn( aTypes, "com.sun.star.uno.XWeak") );
    }
    Reference< lang::XComponent > xComp( xg, UNO_QUERY );
    if (xComp.is())
    {
        OSL_ASSERT( isIn( aTypes, "com.sun.star.lang.XComponent") );
    }

    Reference< XAggregation > xAgg( xg, UNO_QUERY );
    if (xAgg.is())
    {
        OSL_ASSERT( isIn( aTypes, "com.sun.star.uno.XAggregation") );
    }
    Reference< H > xH( xg, UNO_QUERY );
    if (xH.is())
    {
        OSL_ASSERT( isIn( aTypes, "test.H") );
    }
    Reference< I > xI( xg, UNO_QUERY );
    if (xI.is())
    {
        OSL_ASSERT( isIn( aTypes, "test.I") );
    }

    OSL_ENSURE( xg == xOriginal, "### root!" );
}

void rethrow()
{
    throw;
}

void throw_one(
    lang::IllegalAccessException exc )
{
    try
    {
        throw exc;
    }
    catch (...)
    {
        rethrow();
    }
}



void test_ImplHelper( const Reference< lang::XMultiServiceFactory > & /*xSF*/ )
{
    Reference< XInterface > xImpl( (lang::XTypeProvider *)new TestImpl() );
    Reference< lang::XTypeProvider > xTP1( xImpl, UNO_QUERY );
    Reference< XInterface > xWeakImpl( (OWeakObject *)new TestWeakImpl() );
    Reference< lang::XTypeProvider > xTP2( xWeakImpl, UNO_QUERY );
    Reference< XInterface > xWeakAggImpl( (OWeakObject *)new TestWeakAggImpl() );
    Reference< lang::XTypeProvider > xTP3( xWeakAggImpl, UNO_QUERY );
    Reference< XInterface > xWeakComponentImpl( (OWeakObject *)new TestWeakComponentImpl() );
    Reference< lang::XTypeProvider > xTP4( xWeakComponentImpl, UNO_QUERY );
    Reference< XInterface > xWeakAggComponentImpl( (OWeakObject *)new TestWeakAggComponentImpl() );
    Reference< lang::XTypeProvider > xTP5( xWeakAggComponentImpl, UNO_QUERY );

    OSL_ASSERT(
        xTP1->getImplementationId() != xTP2->getImplementationId() &&
        xTP1->getImplementationId() != xTP3->getImplementationId() &&
        xTP1->getImplementationId() != xTP4->getImplementationId() &&
        xTP1->getImplementationId() != xTP5->getImplementationId() &&
        xTP2->getImplementationId() != xTP3->getImplementationId() &&
        xTP2->getImplementationId() != xTP4->getImplementationId() &&
        xTP2->getImplementationId() != xTP5->getImplementationId() &&
        xTP3->getImplementationId() != xTP4->getImplementationId() &&
        xTP3->getImplementationId() != xTP5->getImplementationId() &&
        xTP4->getImplementationId() != xTP5->getImplementationId() );


    dotest( xImpl );
    dotest( xWeakImpl );
    dotest( xWeakAggImpl );
    dotest( xWeakComponentImpl );
    dotest( xWeakAggComponentImpl );

    xWeakImpl = (OWeakObject *)new TestImplInh();
    Reference< lang::XTypeProvider > xTP6( xWeakImpl, UNO_QUERY );
    dotest( xWeakImpl );
    Reference< H > xH( xWeakImpl, UNO_QUERY );
    Reference< I > xI( xH, UNO_QUERY );
    OSL_ASSERT( xH->h() == "h" );
    OSL_ASSERT( xI->i() == "i" );

    xWeakAggImpl = (OWeakObject *)new TestAggImplInh();
    Reference< lang::XTypeProvider > xTP7( xWeakAggImpl, UNO_QUERY );
    dotest( xWeakAggImpl );
    xH.set( xWeakAggImpl, UNO_QUERY );
    xI.set( xH, UNO_QUERY );
    OSL_ASSERT( xH->h() == "h2" );
    OSL_ASSERT( xI->i() == "i2" );


    OSL_ASSERT(
        xTP6->getImplementationId() != xTP1->getImplementationId() &&
        xTP6->getImplementationId() != xTP2->getImplementationId() &&
        xTP6->getImplementationId() != xTP3->getImplementationId() &&
        xTP6->getImplementationId() != xTP4->getImplementationId() &&
        xTP6->getImplementationId() != xTP5->getImplementationId() &&
        xTP6->getImplementationId() != xTP7->getImplementationId() &&
        xTP7->getImplementationId() != xTP1->getImplementationId() &&
        xTP7->getImplementationId() != xTP2->getImplementationId() &&
        xTP7->getImplementationId() != xTP3->getImplementationId() &&
        xTP7->getImplementationId() != xTP4->getImplementationId() &&
        xTP7->getImplementationId() != xTP5->getImplementationId() );


    bool exc_succ = false;
    lang::IllegalAccessException exc(
        OUString("testtest"),
        xWeakAggImpl );
    // exception helper tests
    try
    {
        throw exc;
    }
    catch (Exception &)
    {
        Any a( getCaughtException() );
        OSL_ASSERT( a == exc );

        try
        {
            throwException( a );
        }
        catch (lang::IllegalAccessException & e)
        {
            OSL_ASSERT( exc.Message == e.Message && exc.Context == e.Context );

            try
            {
                throw_one( exc );
            }
            catch (Exception &)
            {
                Any a2( getCaughtException() );
                OSL_ASSERT( (a2 == a) && (a2 == exc) );

                try
                {
                    throw_one( exc );
                }
                catch (lang::IllegalAccessException &)
                {
                    Any a3( getCaughtException() );
                    OSL_ASSERT( (a3 == a) && (a3 == a2) && (a3 == exc) );
                    exc_succ = true;
                }
            }
        }
    }
    OSL_ASSERT( exc_succ );

    try
    {
        throwException( makeAny( RuntimeException(
            OUString("runtime exc"), xImpl ) ) );
    }
    catch (RuntimeException & rExc)
    {
        OSL_ENSURE( rExc.Message == "runtime exc" && rExc.Context == xImpl, "### unexpected exception content!" );
        try
        {
            throwException( makeAny( Exception(
                OUString("exc"), rExc.Context ) ) );
        }
        catch (lang::IllegalAccessException &)
        {
            OSL_FAIL( "### unexpected IllegalAccessException exception caught!" );
        }
        catch (Exception & rExc2)
        {
            OSL_ENSURE( rExc2.Message == "exc" && rExc2.Context == xImpl, "### unexpected exception content!" );
            try
            {
                throwException( makeAny( lang::IllegalAccessException(
                    OUString("axxess exc"), rExc2.Context ) ) );
            }
            catch (lang::IllegalAccessException & rExc3)
            {
                OSL_ENSURE( rExc3.Message == "axxess exc" && rExc3.Context == xImpl, "### unexpected exception content!" );
                return;
            }
        }
    }
    catch (...)
    {
    }
    OSL_FAIL( "### exception test failed!" );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
