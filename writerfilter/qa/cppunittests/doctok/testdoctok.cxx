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

#include <osl/process.h>
#include <cppuhelper/bootstrap.hxx>
#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/plugin/TestPlugIn.h"
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <iostream>

#include <doctok/WW8Document.hxx>

namespace testdoctok
{
using namespace ::std;
using namespace ::com::sun::star;
using namespace ::writerfilter::doctok;

uno::Reference<io::XInputStream> xStream;
uno::Reference<uno::XComponentContext> xContext;
WW8Document::Pointer_t pDocument;
uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess > xSimpleFileAccess;

class test : public CppUnit::TestFixture
{

public:
    // initialise your test code values here.
    void setUp()
    {
    }

    void tearDown()
    {
    }

    void testInitUno()
    {
        bool bResult = false;

        uno::Reference<uno::XComponentContext>
            xComponentContext
            (::cppu::defaultBootstrap_InitialComponentContext());
        OSL_ASSERT( xComponentContext.is() );

        xContext = xComponentContext;

        uno::Reference<lang::XMultiComponentFactory>
            xFactory(xComponentContext->getServiceManager() );
        OSL_ASSERT(xFactory.is());

        uno::Reference<lang::XMultiServiceFactory>
            xServiceFactory(xFactory, uno::UNO_QUERY);
        OSL_ASSERT( xServiceFactory.is() );

        if (xServiceFactory.is())
        {
            uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess >
                xNameContainer(xFactory->createInstanceWithContext
                               ("com.sun.star.ucb.SimpleFileAccess",
                                xComponentContext), uno::UNO_QUERY );

            if (xNameContainer.is())
            {
                xSimpleFileAccess = xNameContainer;

                bResult = true;
            }
        }

        CPPUNIT_ASSERT_MESSAGE("UNO initialization failed",
                               bResult);
    }

    // insert your test code here.
    void testOpenFile()
    {
        rtl_uString *dir=NULL;
        osl_getProcessWorkingDir(&dir);
        rtl_uString *fname=NULL;
        rtl_uString_newFromAscii(&fname, "/test.doc");
        rtl_uString *absfile=NULL;
        rtl_uString_newConcat(&absfile, dir, fname);

        OUString sInputFileURL( absfile );

        for (sal_Int32 n = 0; n < sInputFileURL.getLength(); ++n)
        {
            sal_uChar nC = sInputFileURL[n];

            if (nC < 0xff && isprint(nC))
                clog << static_cast<char>(nC);
            else
                clog << ".";
        }

        clog << endl;

        xStream = xSimpleFileAccess->openFileRead(sInputFileURL);

        WW8Stream::Pointer_t pStream =
            WW8DocumentFactory::createStream(xContext, xStream);

        pDocument.reset(WW8DocumentFactory::createDocument(pStream));

        CPPUNIT_ASSERT_MESSAGE("creating document failed",
                               pDocument != NULL);
    }

    void testEvents()
    {
        ::writerfilter::Stream::Pointer_t pStream = ::writerfilter::createStreamHandler();

        pDocument->resolve(*pStream);
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(test);
    CPPUNIT_TEST(testInitUno);
    CPPUNIT_TEST(testOpenFile);
    CPPUNIT_TEST(testEvents);
    CPPUNIT_TEST_SUITE_END();
}; // class test

// -----------------------------------------------------------------------------
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(testdoctok::test, "doctok");
} // namespace doctok


// -----------------------------------------------------------------------------

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
