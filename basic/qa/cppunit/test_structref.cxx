/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright 2012 LibreOffice contributors.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "basictest.hxx"
#include "osl/file.hxx"
#include "osl/process.h"

#include "basic/sbmod.hxx"
#include "basic/sbmeth.hxx"
#include "sbunoobj.hxx"
#include <com/sun/star/awt/WindowDescriptor.hpp>
#include <comphelper/anytostring.hxx>

using namespace com::sun::star;
namespace
{
    class Test_StructRef : public BasicTestBase
    {
        public:
        Test_StructRef() {};
        void testSimple();
        // Adds code needed to register the test suite
        CPPUNIT_TEST_SUITE(Test_StructRef);

        // Declares the method as a test to call
        CPPUNIT_TEST(testSimple);
        // End of test suite definition
        CPPUNIT_TEST_SUITE_END();
    };


void Test_StructRef::testSimple()
{

    uno::Any aWinDescAny;
    awt::WindowDescriptor aDesc;
    aWinDescAny <<= aDesc;
    typelib_TypeDescription * pDeclTD = 0;
    typelib_typedescription_getByName( &pDeclTD, aWinDescAny.getValueTypeName().pData );
    StructRefInfo aWinDesc( aWinDescAny, pDeclTD, 0 );
    SbUnoStructRefObject refObject( "aBasicWinDesc", aWinDesc );


//    StructRefInfo aInfo = refObject.getStructMember( "WindowServiceName" );
    StructRefInfo aBounds = refObject.getStructMember( "Bounds" );

    SbUnoStructRefObject aBoundRef( "Bound", aBounds );
    StructRefInfo xInfo = aBoundRef.getStructMember( "X" );
    xInfo.setValue( uno::Any((sal_Int32)300) );

    printf("\n\n\n");
    printf("aBasicWinDesc  %s\n", OUStringToOString( comphelper::anyToString(  aWinDescAny ), RTL_TEXTENCODING_UTF8 ).getStr() );

    uno::Any aWinDescAnyCopy;
    aWinDescAnyCopy = aWinDescAny;

    printf("aBasicWinDescCopy  %s\n", OUStringToOString( comphelper::anyToString(  aWinDescAnyCopy ), RTL_TEXTENCODING_UTF8 ).getStr() );


    CPPUNIT_ASSERT_EQUAL( true, false );
}
  // Put the test suite in the registry
  CPPUNIT_TEST_SUITE_REGISTRATION(Test_StructRef);
} // namespace
CPPUNIT_PLUGIN_IMPLEMENT();
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
