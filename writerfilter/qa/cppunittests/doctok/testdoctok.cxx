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

#include <sal/config.h>

#include <osl/process.h>
#include <ucbhelper/contentbroker.hxx>
#include <ucbhelper/std_inputstream.hxx>
#include <cppuhelper/bootstrap.hxx>
#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/plugin/TestPlugIn.h"
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/container/XNameContainer.hpp>

#include <resourcemodel/exceptions.hxx>
#include <doctok/WW8Document.hxx>
#include <resourcemodel/WW8ResourceModel.hxx>

#include <iostream>

namespace testdoctok
{
    using namespace ::std;
    using namespace ::writerfilter::doctok;
    using namespace ::com::sun::star;

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

            // initialise UCB-Broker
            uno::Reference<uno::XComponentContext>
                xComponentContext
                (::cppu::defaultBootstrap_InitialComponentContext());
            OSL_ASSERT( xComponentContext.is() );

            xContext = xComponentContext;

            uno::Reference<lang::XMultiComponentFactory>
                xFactory(xComponentContext->getServiceManager() );
            OSL_ASSERT(xFactory.is());

            uno::Sequence<uno::Any> aUcbInitSequence(2);
            aUcbInitSequence[0] <<=
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Local"));
            aUcbInitSequence[1] <<=
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Office"));

            uno::Reference<lang::XMultiServiceFactory>
                xServiceFactory(xFactory, uno::UNO_QUERY);
            OSL_ASSERT( xServiceFactory.is() );

            if (xServiceFactory.is())
            {
                sal_Bool bRet =
                    ::ucb::ContentBroker::initialize(xServiceFactory,
                                                     aUcbInitSequence);

                OSL_ASSERT(bRet);
                if (bRet)
                {
                    uno::Reference< ::com::sun::star::ucb::XSimpleFileAccess >
                        xNameContainer(xFactory->createInstanceWithContext
                                       (::rtl::OUString::createFromAscii
                                        ("com.sun.star.ucb.SimpleFileAccess" ),
                                        xComponentContext), uno::UNO_QUERY );

                    if (xNameContainer.is())
                    {
                        xSimpleFileAccess = xNameContainer;

                        bResult = true;
                    }
                }
            }

            CPPUNIT_ASSERT_MESSAGE("UNO initialization failed",
                                   bResult);
        }

        // insert your test code here.
        void testOpenFile()
        {
            try
            {
                rtl_uString *dir=NULL;
                osl_getProcessWorkingDir(&dir);
                rtl_uString *fname=NULL;
                //rtl_uString_newFromAscii(&fname, "/OpenDocument-v1.doc");
                rtl_uString_newFromAscii(&fname, "/test.doc");
                //rtl_uString_newFromAscii(&fname, "/numbers.doc");
                rtl_uString *absfile=NULL;
                rtl_uString_newConcat(&absfile, dir, fname);

                rtl::OUString sInputFileURL( absfile );

                for (sal_uInt32 n = 0; n < sInputFileURL.getLength(); ++n)
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

                pDocument = WW8DocumentFactory::createDocument(pStream);
            }
            catch (writerfilter::Exception e)
            {
                clog << "Exception!!" << endl;
            }

            CPPUNIT_ASSERT_MESSAGE("creating document failed",
                                   pDocument != NULL);

#if 1
        }

        void testTraversal()
        {
#endif
            sal_uInt32 nResult = 0;

            try
            {
                WW8DocumentIterator::Pointer_t pIt = pDocument->begin();
                WW8DocumentIterator::Pointer_t pItEnd = pDocument->end();

                while (! pIt->equal(*pItEnd))
                {
                    pIt->dump(clog);

                    clog << endl;

                    WW8PropertySet::Pointer_t pAttrs = pIt->getProperties();

                    if (pAttrs != NULL)
                    {
                        pAttrs->dump(clog);
                    }

                    pIt->getText().dump(clog);
                    ++(*pIt);
                    ++nResult;
                }
            }
            catch (writerfilter::Exception e)
            {
                clog << "Exception!!" << endl;
            }

            char sBuffer[256];
            snprintf(sBuffer, 255, "%d", nResult);
            clog << "Iterator steps:" << sBuffer << endl;

            CPPUNIT_ASSERT_MESSAGE("traversing document failed",
                                   nResult > 0);
        }

        void testEvents()
        {
            try
            {
                Stream::Pointer_t pStream = doctok::createStreamHandler();

                pDocument->resolve(*pStream);
            }
            catch (writerfilter::Exception e)
            {
                clog << "Exception!!" << endl;
            }
        }

        void testEnd()
        {
            ::ucb::ContentBroker::deinitialize();
        }

        // Change the following lines only, if you add, remove or rename
        // member functions of the current class,
        // because these macros are need by auto register mechanism.

        CPPUNIT_TEST_SUITE(test);
        CPPUNIT_TEST(testInitUno);
        CPPUNIT_TEST(testOpenFile);
        //CPPUNIT_TEST(testTraversal);
        CPPUNIT_TEST(testEvents);
        CPPUNIT_TEST(testEnd);
        CPPUNIT_TEST_SUITE_END();
    }; // class test

    // -----------------------------------------------------------------------------
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(testdoctok::test, "doctok");
} // namespace doctok

// -----------------------------------------------------------------------------

// this macro creates an empty function, which will called by the RegisterAllFunctions()
// to let the user the possibility to also register some functions by hand.
NOADDITIONAL;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
