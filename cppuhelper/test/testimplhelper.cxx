/*************************************************************************
 *
 *  $RCSfile: testimplhelper.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-23 15:02:56 $
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

#include <stdio.h>

#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/implbase4.hxx>
#include <cppuhelper/implbase5.hxx>
#include <cppuhelper/implbase6.hxx>
#include <cppuhelper/implbase7.hxx>
#include <cppuhelper/implbase8.hxx>
#include <cppuhelper/implbase9.hxx>
#include <cppuhelper/implbase10.hxx>
#include <cppuhelper/implbase11.hxx>
#include <cppuhelper/implbase12.hxx>

#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/compbase2.hxx>
#include <cppuhelper/compbase3.hxx>
#include <cppuhelper/compbase4.hxx>
#include <cppuhelper/compbase5.hxx>
#include <cppuhelper/compbase6.hxx>
#include <cppuhelper/compbase7.hxx>
#include <cppuhelper/compbase8.hxx>
#include <cppuhelper/compbase9.hxx>
#include <cppuhelper/compbase10.hxx>
#include <cppuhelper/compbase11.hxx>
#include <cppuhelper/compbase12.hxx>

#include <osl/diagnose.h>
#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/implbase4.hxx>
#include <cppuhelper/compbase4.hxx>
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
using namespace rtl;
using namespace osl;
using namespace cppu;
using namespace com::sun::star;
using namespace com::sun::star::uno;

//==================================================================================================
struct TestImpl : public ImplHelper4< CA, DBA, FE, G >
{
    sal_Int32 nRef;

    virtual ~TestImpl()
        { OSL_TRACE( "> TestImpl dtor called... <\n" ); }

    virtual Any SAL_CALL queryInterface( const Type & rType ) throw(RuntimeException)
        { return ImplHelper4< CA, DBA, FE, G >::queryInterface( rType ); }
    virtual void SAL_CALL acquire(  ) throw()
        { ++nRef; }
    virtual void SAL_CALL release(  ) throw()
        { if (! --nRef) delete this; }

    // A
    virtual OUString SAL_CALL a() throw(RuntimeException)
        { return OUString( RTL_CONSTASCII_USTRINGPARAM("a") ); }
    // BA
    virtual OUString SAL_CALL ba() throw(RuntimeException)
        { return OUString( RTL_CONSTASCII_USTRINGPARAM("ba") ); }
    // CA
    virtual OUString SAL_CALL ca() throw(RuntimeException)
        { return OUString( RTL_CONSTASCII_USTRINGPARAM("ca") ); }
    // DBA
    virtual OUString SAL_CALL dba() throw(RuntimeException)
        { return OUString( RTL_CONSTASCII_USTRINGPARAM("dba") ); }
    // E
    virtual OUString SAL_CALL e() throw(RuntimeException)
        { return OUString( RTL_CONSTASCII_USTRINGPARAM("e") ); }
    // FE
    virtual OUString SAL_CALL fe() throw(RuntimeException)
        { return OUString( RTL_CONSTASCII_USTRINGPARAM("fe") ); }
    // G
    virtual OUString SAL_CALL g() throw(RuntimeException)
        { return OUString( RTL_CONSTASCII_USTRINGPARAM("g") ); }
};

//==================================================================================================
struct TestWeakAggImpl : public WeakAggImplHelper4< CA, DBA, FE, G >
{
    virtual ~TestWeakAggImpl()
        { OSL_TRACE( "> TestWeakAggImpl dtor called... <\n" ); }

    // A
    virtual OUString SAL_CALL a() throw(RuntimeException)
        { return OUString( RTL_CONSTASCII_USTRINGPARAM("a") ); }
    // BA
    virtual OUString SAL_CALL ba() throw(RuntimeException)
        { return OUString( RTL_CONSTASCII_USTRINGPARAM("ba") ); }
    // CA
    virtual OUString SAL_CALL ca() throw(RuntimeException)
        { return OUString( RTL_CONSTASCII_USTRINGPARAM("ca") ); }
    // DBA
    virtual OUString SAL_CALL dba() throw(RuntimeException)
        { return OUString( RTL_CONSTASCII_USTRINGPARAM("dba") ); }
    // E
    virtual OUString SAL_CALL e() throw(RuntimeException)
        { return OUString( RTL_CONSTASCII_USTRINGPARAM("e") ); }
    // FE
    virtual OUString SAL_CALL fe() throw(RuntimeException)
        { return OUString( RTL_CONSTASCII_USTRINGPARAM("fe") ); }
    // G
    virtual OUString SAL_CALL g() throw(RuntimeException)
        { return OUString( RTL_CONSTASCII_USTRINGPARAM("g") ); }
};

//==================================================================================================
struct TestWeakImpl : public WeakImplHelper4< CA, DBA, FE, G >
{
    TestWeakImpl() {}

    virtual ~TestWeakImpl()
        { OSL_TRACE( "> TestWeakImpl dtor called... <\n" ); }

    // A
    virtual OUString SAL_CALL a() throw(RuntimeException)
        { return OUString( RTL_CONSTASCII_USTRINGPARAM("a") ); }
    // BA
    virtual OUString SAL_CALL ba() throw(RuntimeException)
        { return OUString( RTL_CONSTASCII_USTRINGPARAM("ba") ); }
    // CA
    virtual OUString SAL_CALL ca() throw(RuntimeException)
        { return OUString( RTL_CONSTASCII_USTRINGPARAM("ca") ); }
    // DBA
    virtual OUString SAL_CALL dba() throw(RuntimeException)
        { return OUString( RTL_CONSTASCII_USTRINGPARAM("dba") ); }
    // E
    virtual OUString SAL_CALL e() throw(RuntimeException)
        { return OUString( RTL_CONSTASCII_USTRINGPARAM("e") ); }
    // FE
    virtual OUString SAL_CALL fe() throw(RuntimeException)
        { return OUString( RTL_CONSTASCII_USTRINGPARAM("fe") ); }
    // G
    virtual OUString SAL_CALL g() throw(RuntimeException)
        { return OUString( RTL_CONSTASCII_USTRINGPARAM("g") ); }

protected:
    TestWeakImpl(int) {}

    TestWeakImpl(int, int) {}

    TestWeakImpl(int, int, int) {}

    TestWeakImpl(int, int, int, int) {}

    TestWeakImpl(int, int, int, int, int) {}

    TestWeakImpl(int, int, int, int, int, int, int = 0) {}
};

//==================================================================================================
struct TestWeakComponentImpl : public WeakComponentImplHelper4< CA, DBA, FE, G >
{
    Mutex m;
    TestWeakComponentImpl()
        : WeakComponentImplHelper4< CA, DBA, FE, G >( m )
        {}
    virtual ~TestWeakComponentImpl()
        { OSL_TRACE( "> TestWeakComponentImpl dtor called... <\n" ); }

    void SAL_CALL disposing()
        { OSL_TRACE( "> TestWeakComponentImpl disposing called... <\n" ); }

    // A
    virtual OUString SAL_CALL a() throw(RuntimeException)
        { return OUString( RTL_CONSTASCII_USTRINGPARAM("a") ); }
    // BA
    virtual OUString SAL_CALL ba() throw(RuntimeException)
        { return OUString( RTL_CONSTASCII_USTRINGPARAM("ba") ); }
    // CA
    virtual OUString SAL_CALL ca() throw(RuntimeException)
        { return OUString( RTL_CONSTASCII_USTRINGPARAM("ca") ); }
    // DBA
    virtual OUString SAL_CALL dba() throw(RuntimeException)
        { return OUString( RTL_CONSTASCII_USTRINGPARAM("dba") ); }
    // E
    virtual OUString SAL_CALL e() throw(RuntimeException)
        { return OUString( RTL_CONSTASCII_USTRINGPARAM("e") ); }
    // FE
    virtual OUString SAL_CALL fe() throw(RuntimeException)
        { return OUString( RTL_CONSTASCII_USTRINGPARAM("fe") ); }
    // G
    virtual OUString SAL_CALL g() throw(RuntimeException)
        { return OUString( RTL_CONSTASCII_USTRINGPARAM("g") ); }
};

//==================================================================================================
struct TestWeakAggComponentImpl : public WeakAggComponentImplHelper4< CA, DBA, FE, G >
{
    Mutex m;
    TestWeakAggComponentImpl()
        : WeakAggComponentImplHelper4< CA, DBA, FE, G >( m )
        {}
    virtual ~TestWeakAggComponentImpl()
        { OSL_TRACE( "> TestWeakAggComponentImpl dtor called... <\n" ); }

    void SAL_CALL disposing()
        { OSL_TRACE( "> TestWeakAggComponentImpl disposing called... <\n" ); }

    // A
    virtual OUString SAL_CALL a() throw(RuntimeException)
        { return OUString( RTL_CONSTASCII_USTRINGPARAM("a") ); }
    // BA
    virtual OUString SAL_CALL ba() throw(RuntimeException)
        { return OUString( RTL_CONSTASCII_USTRINGPARAM("ba") ); }
    // CA
    virtual OUString SAL_CALL ca() throw(RuntimeException)
        { return OUString( RTL_CONSTASCII_USTRINGPARAM("ca") ); }
    // DBA
    virtual OUString SAL_CALL dba() throw(RuntimeException)
        { return OUString( RTL_CONSTASCII_USTRINGPARAM("dba") ); }
    // E
    virtual OUString SAL_CALL e() throw(RuntimeException)
        { return OUString( RTL_CONSTASCII_USTRINGPARAM("e") ); }
    // FE
    virtual OUString SAL_CALL fe() throw(RuntimeException)
        { return OUString( RTL_CONSTASCII_USTRINGPARAM("fe") ); }
    // G
    virtual OUString SAL_CALL g() throw(RuntimeException)
        { return OUString( RTL_CONSTASCII_USTRINGPARAM("g") ); }
};

//==================================================================================================
struct TestImplInh : public ImplInheritanceHelper2< TestWeakImpl, H, I >
{
    TestImplInh() {}

    explicit TestImplInh(int):
        ImplInheritanceHelper2< TestWeakImpl, H, I >(1) {}

    TestImplInh(int, int):
        ImplInheritanceHelper2< TestWeakImpl, H, I >(1, 2) {}

    TestImplInh(int, int, int):
        ImplInheritanceHelper2< TestWeakImpl, H, I >(1, 2, 3) {}

    TestImplInh(int, int, int, int):
        ImplInheritanceHelper2< TestWeakImpl, H, I >(1, 2, 3, 4) {}

    TestImplInh(int, int, int, int, int):
        ImplInheritanceHelper2< TestWeakImpl, H, I >(1, 2, 3, 4, 5) {}

    TestImplInh(int, int, int, int, int, int):
        ImplInheritanceHelper2< TestWeakImpl, H, I >(1, 2, 3, 4, 5, 6) {}

    virtual ~TestImplInh()
        { OSL_TRACE( "> TestWeakImplInh dtor called... <\n" ); }

    // H
    virtual OUString SAL_CALL h() throw(RuntimeException)
        { return OUString( RTL_CONSTASCII_USTRINGPARAM("h") ); }
    // I
    virtual OUString SAL_CALL i() throw(RuntimeException)
        { return OUString( RTL_CONSTASCII_USTRINGPARAM("i") ); }
};

//==================================================================================================
struct TestAggImplInh : public AggImplInheritanceHelper2< TestWeakAggImpl, H, I >
{
    virtual ~TestAggImplInh()
        { OSL_TRACE( "> TestAggImplInh dtor called... <\n" ); }

    // H
    virtual OUString SAL_CALL h() throw(RuntimeException)
        { return OUString( RTL_CONSTASCII_USTRINGPARAM("h2") ); }
    // I
    virtual OUString SAL_CALL i() throw(RuntimeException)
        { return OUString( RTL_CONSTASCII_USTRINGPARAM("i2") ); }
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

//==================================================================================================
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
    OSL_ENSURE( xa->a().equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("a") ), "### A failed!" );
    Reference< BA > xba( xa, UNO_QUERY );
    OSL_ENSURE( xba->ba().equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("ba") ), "### BA failed!" );
    OSL_ENSURE( xba->a().equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("a") ), "### BA failed!" );
    Reference< CA > xca( xba, UNO_QUERY );
    OSL_ENSURE( xca->ca().equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("ca") ), "### CA failed!" );
    OSL_ENSURE( xca->a().equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("a") ), "### CA failed!" );
    Reference< DBA > xdba( xca, UNO_QUERY );
    OSL_ENSURE( xdba->dba().equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("dba") ), "### DBA failed!" );
    OSL_ENSURE( xdba->ba().equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("ba") ), "### DBA failed!" );
    OSL_ENSURE( xdba->a().equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("a") ), "### DBA failed!" );
    Reference< E > xe( xdba, UNO_QUERY );
    OSL_ENSURE( xe->e().equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("e") ), "### E failed!" );
    Reference< FE > xfe( xe, UNO_QUERY );
    OSL_ENSURE( xfe->fe().equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("fe") ), "### FE failed!" );
    Reference< G > xg( xfe, UNO_QUERY );
    OSL_ENSURE( xg->g().equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("g") ), "### G failed!" );
    OSL_ENSURE( xg->a().equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("a") ), "### G failed!" );

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


//==================================================================================================
void test_ImplHelper( const Reference< lang::XMultiServiceFactory > & xSF )
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
    //
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
    //

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
    OSL_ASSERT( xH->h().equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("h") ) );
    OSL_ASSERT( xI->i().equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("i") ) );

    xWeakAggImpl = (OWeakObject *)new TestAggImplInh();
    Reference< lang::XTypeProvider > xTP7( xWeakAggImpl, UNO_QUERY );
    dotest( xWeakAggImpl );
    xH.set( xWeakAggImpl, UNO_QUERY );
    xI.set( xH, UNO_QUERY );
    OSL_ASSERT( xH->h().equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("h2") ) );
    OSL_ASSERT( xI->i().equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("i2") ) );

    //
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
    //

    bool exc_succ = false;
    lang::IllegalAccessException exc(
        OUString( RTL_CONSTASCII_USTRINGPARAM("testtest") ),
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
            OUString( RTL_CONSTASCII_USTRINGPARAM("runtime exc") ), xImpl ) ) );
    }
    catch (RuntimeException & rExc)
    {
        OSL_ENSURE( rExc.Message.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("runtime exc") ) && rExc.Context == xImpl,
                     "### unexpected exception content!" );
        try
        {
            throwException( makeAny( Exception(
                OUString( RTL_CONSTASCII_USTRINGPARAM("exc") ), rExc.Context ) ) );
        }
        catch (lang::IllegalAccessException &)
        {
            OSL_ENSURE( sal_False, "### unexpected IllegalAccessException exception caught!" );
        }
        catch (Exception rExc)
        {
            OSL_ENSURE( rExc.Message.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("exc") ) && rExc.Context == xImpl,
                         "### unexpected exception content!" );
            try
            {
                throwException( makeAny( lang::IllegalAccessException(
                    OUString( RTL_CONSTASCII_USTRINGPARAM("axxess exc") ), rExc.Context ) ) );
            }
            catch (lang::IllegalAccessException & rExc)
            {
                OSL_ENSURE( rExc.Message.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("axxess exc") ) && rExc.Context == xImpl,
                             "### unexpected exception content!" );
                return;
            }
        }
    }
    catch (...)
    {
    }
    OSL_ENSURE( sal_False, "### exception test failed!" );
}

