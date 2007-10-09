/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: test_reference.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2007-10-09 15:19:47 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cppu.hxx"

#include "sal/config.h"

#include "Interface1.hpp"

#include "cppunit/simpleheader.hxx"
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
        osl_incrementInterlockedCount( &m_refCount );
    }

    virtual void SAL_CALL release() throw ()
    {
        if ( 0 == osl_decrementInterlockedCount( &m_refCount ) )
            delete this;
    }

protected:
    virtual ~Foo()
    {
    }

private:
    Foo(Foo &); // not declared
    Foo& operator =(const Foo&); // not declared

private:
    oslInterlockedCount m_refCount;
};

class Test: public ::CppUnit::TestFixture
{

public:
    void testUnoSetThrow();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testUnoSetThrow);
    CPPUNIT_TEST_SUITE_END();
};

void Test::testUnoSetThrow()
{
    Reference< Interface1 > xNull;
    Reference< Interface1 > xFoo( new Foo );

    // ctor taking Reference< interface_type >
    bool bCaughtException = false;
    try { Reference< Interface1 > x( xNull, UNO_SET_THROW ); (void)x; } catch( const RuntimeException& ) { bCaughtException = true; }
    CPPUNIT_ASSERT_EQUAL( true, bCaughtException );

    bCaughtException = false;
    try { Reference< Interface1 > x( xFoo, UNO_SET_THROW ); (void)x; } catch( const RuntimeException& ) { bCaughtException = true; }
    CPPUNIT_ASSERT_EQUAL( false, bCaughtException );

    // ctor taking interface_type*
    bCaughtException = false;
    try { Reference< Interface1 > x( xNull.get(), UNO_SET_THROW ); (void)x; } catch( const RuntimeException& ) { bCaughtException = true; }
    CPPUNIT_ASSERT_EQUAL( true, bCaughtException );

    bCaughtException = false;
    try { Reference< Interface1 > x( xFoo.get(), UNO_SET_THROW ); (void)x; } catch( const RuntimeException& ) { bCaughtException = true; }
    CPPUNIT_ASSERT_EQUAL( false, bCaughtException );

    Reference< Interface1 > x;
    // "set" taking Reference< interface_type >
    bCaughtException = false;
    try { x.set( xNull, UNO_SET_THROW ); } catch( const RuntimeException& ) { bCaughtException = true; }
    CPPUNIT_ASSERT_EQUAL( true, bCaughtException );

    bCaughtException = false;
    try { x.set( xFoo, UNO_SET_THROW ); } catch( const RuntimeException& ) { bCaughtException = true; }
    CPPUNIT_ASSERT_EQUAL( false, bCaughtException );

    // "set" taking interface_type*
    bCaughtException = false;
    try { x.set( xNull.get(), UNO_SET_THROW ); } catch( const RuntimeException& ) { bCaughtException = true; }
    CPPUNIT_ASSERT_EQUAL( true, bCaughtException );

    bCaughtException = false;
    try { x.set( xFoo.get(), UNO_SET_THROW ); } catch( const RuntimeException& ) { bCaughtException = true; }
    CPPUNIT_ASSERT_EQUAL( false, bCaughtException );
}

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(Test, "alltests");

}   // namespace

NOADDITIONAL;
