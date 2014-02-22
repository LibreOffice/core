/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <sal/types.h>

#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestCase.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <cppunit/extensions/HelperMacros.h>

#include "Interface1.hpp"

#include "rtl/ustring.hxx"
#include "sal/types.h"

namespace
{

using ::com::sun::star::uno::Type;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::UNO_SET_THROW;

class Foo: public Interface1
{
public:
    Foo()
        :m_refCount(0)
    {
    }

    virtual Any SAL_CALL queryInterface(const Type & _type)
        throw (RuntimeException)
    {
        Any aInterface;
        if (_type == getCppuType< Reference< XInterface > >())
        {
            Reference< XInterface > ref( static_cast< XInterface * >( this ) );
            aInterface.setValue( &ref, _type );
        }
        else if (_type == getCppuType< Reference< Interface1 > >())
        {
            Reference< Interface1 > ref( this );
            aInterface.setValue( &ref, _type );
        }

        return Any();
    }

    virtual void SAL_CALL acquire() throw ()
    {
        osl_atomic_increment( &m_refCount );
    }

    virtual void SAL_CALL release() throw ()
    {
        if ( 0 == osl_atomic_decrement( &m_refCount ) )
            delete this;
    }

protected:
    virtual ~Foo()
    {
    }

private:
    Foo(Foo &); 
    Foo& operator =(const Foo&); 

private:
    oslInterlockedCount m_refCount;
};




struct Base1: public css::uno::XInterface { virtual ~Base1() {} };
struct Base2: public Base1 { virtual ~Base2() {} };
struct Base3: public Base1 { virtual ~Base3() {} };
struct Derived: public Base2, public Base3 { virtual ~Derived() {} };


css::uno::Reference< css::uno::XInterface > testUpcast1(
    css::uno::Reference< Derived > const & ref)
{ return ref; }


css::uno::Reference< Base1 > testUpcast2(
    css::uno::Reference< Base2 > const & ref)
{ return ref; }


/*
css::uno::Reference< Base1 > testFailingUpcast3(
    css::uno::Reference< Derived > const & ref)
{ return ref; }
*/


/*
css::uno::Reference< Base2 > testFailingUpcast4(
    css::uno::Reference< Base1 > const & ref)
{ return ref; }
*/


/*
css::uno::Reference< Base1 > testFailingUpcast5(
    css::uno::Reference< css::uno::XInterface > const & ref)
{ return ref; }
*/

class Test: public ::CppUnit::TestFixture
{

public:
    void testUnoSetThrow();
    void testUpcastCompilation();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testUnoSetThrow);
    CPPUNIT_TEST(testUpcastCompilation);
    CPPUNIT_TEST_SUITE_END();
};

void Test::testUnoSetThrow()
{
    Reference< Interface1 > xNull;
    Reference< Interface1 > xFoo( new Foo );

    
    bool bCaughtException = false;
    try { Reference< Interface1 > x( xNull, UNO_SET_THROW ); (void)x; } catch( const RuntimeException& ) { bCaughtException = true; }
    CPPUNIT_ASSERT_EQUAL( true, bCaughtException );

    bCaughtException = false;
    try { Reference< Interface1 > x( xFoo, UNO_SET_THROW ); (void)x; } catch( const RuntimeException& ) { bCaughtException = true; }
    CPPUNIT_ASSERT_EQUAL( false, bCaughtException );

    
    bCaughtException = false;
    try { Reference< Interface1 > x( xNull.get(), UNO_SET_THROW ); (void)x; } catch( const RuntimeException& ) { bCaughtException = true; }
    CPPUNIT_ASSERT_EQUAL( true, bCaughtException );

    bCaughtException = false;
    try { Reference< Interface1 > x( xFoo.get(), UNO_SET_THROW ); (void)x; } catch( const RuntimeException& ) { bCaughtException = true; }
    CPPUNIT_ASSERT_EQUAL( false, bCaughtException );

    Reference< Interface1 > x;
    
    bCaughtException = false;
    try { x.set( xNull, UNO_SET_THROW ); } catch( const RuntimeException& ) { bCaughtException = true; }
    CPPUNIT_ASSERT_EQUAL( true, bCaughtException );

    bCaughtException = false;
    try { x.set( xFoo, UNO_SET_THROW ); } catch( const RuntimeException& ) { bCaughtException = true; }
    CPPUNIT_ASSERT_EQUAL( false, bCaughtException );

    
    bCaughtException = false;
    try { x.set( xNull.get(), UNO_SET_THROW ); } catch( const RuntimeException& ) { bCaughtException = true; }
    CPPUNIT_ASSERT_EQUAL( true, bCaughtException );

    bCaughtException = false;
    try { x.set( xFoo.get(), UNO_SET_THROW ); } catch( const RuntimeException& ) { bCaughtException = true; }
    CPPUNIT_ASSERT_EQUAL( false, bCaughtException );
}



void Test::testUpcastCompilation()
{
    testUpcast1(css::uno::Reference< Derived >());
    testUpcast2(css::uno::Reference< Base2 >());
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}   

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
