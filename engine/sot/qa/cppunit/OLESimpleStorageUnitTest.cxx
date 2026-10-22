/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <rtl/ref.hxx>
#include <rtl/random.h>
#include <comphelper/processfactory.hxx>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <test/bootstrapfixture.hxx>

#include <com/sun/star/io/XTempFile.hpp>
#include <com/sun/star/embed/XOLESimpleStorage.hpp>

using namespace cpo::uno;
using namespace css::io;
using namespace css::embed;

namespace
{

class OLESimpleStorageUnitTest : public test::BootstrapFixture
{
public:
    void ExecuteTest01()
    {
        const int pStreamCnt = 5;
        const int pBytesCnt = 10;

        //create a new temporary stream
        Reference<XInterface> oTempFile
            = getMultiServiceFactory()->createInstance(u"com.sun.star.io.TempFile"_ustr);
        Reference<XTempFile> xTempFile(oTempFile, UNO_QUERY);

        //create OLESimpleStorage based on it
        Sequence<Any> pArgs{ Any(xTempFile), Any(true) };
        Reference<XInterface> oOLESimpleStorage
            = getMultiServiceFactory()->createInstanceWithArguments(
                u"com.sun.star.embed.OLESimpleStorage"_ustr, pArgs);
        Reference<XOLESimpleStorage> xOLESimpleStorage(oOLESimpleStorage, UNO_QUERY);

        // Fill it with some streams
        std::vector<Reference<XInterface>> oStream(pStreamCnt);
        std::vector<Sequence<sal_Int8>> pBytesIn(pStreamCnt, Sequence<sal_Int8>(pBytesCnt));
        std::vector<Sequence<sal_Int8>> pBytesOut(pStreamCnt, Sequence<sal_Int8>(pBytesCnt));
        std::vector<Reference<XTempFile>> xTempStream(pStreamCnt);
        const OUString sSubStreamPrefix = u"SubStream"_ustr;
        for (int i = 0; i < pStreamCnt; i++)
        {
            rtl_random_getBytes(pBytesOut[i].getArray(), pBytesCnt);
            oStream[i] = getMultiServiceFactory()->createInstance(u"com.sun.star.io.TempFile"_ustr);
            xTempStream[i] = Reference<XTempFile>(oStream[i], UNO_QUERY);
            xTempStream[i]->getOutputStream()->writeBytes(pBytesOut[i]);
            xTempStream[i]->seek(0);
            // Substream i initialized.
            if (xOLESimpleStorage->hasByName(sSubStreamPrefix + OUString::number(i)))
            {
                xOLESimpleStorage->replaceByName(sSubStreamPrefix + OUString::number(i),
                                                 Any(xTempStream[i]));
            }
            else
            {
                xOLESimpleStorage->insertByName(sSubStreamPrefix + OUString::number(i),
                                                Any(xTempStream[i]));
            }
        }

        //commit the storage and close it
        xOLESimpleStorage->commit();
        xOLESimpleStorage->dispose();
        for (int i = 0; i < pStreamCnt; ++i)
        {
            xTempStream[i]->setRemoveFile(true);
            xTempStream[i]->getInputStream()->closeInput();
            xTempStream[i]->getOutputStream()->closeOutput();
        }

        //open the same stream with the constructor for inputstream
        oOLESimpleStorage = getMultiServiceFactory()->createInstanceWithArguments(
            u"com.sun.star.embed.OLESimpleStorage"_ustr, { Any(xTempFile->getInputStream()), Any(true) });
        xOLESimpleStorage = Reference<XOLESimpleStorage>(oOLESimpleStorage, UNO_QUERY);

        //check that all the streams contain correct information
        for (int i = 0; i < pStreamCnt; ++i)
        {
            if (xOLESimpleStorage->hasByName(sSubStreamPrefix + OUString::number(i)))
            {
                xTempStream[i] = Reference<XTempFile>(
                    xOLESimpleStorage->getByName(sSubStreamPrefix + OUString::number(i)),
                    UNO_QUERY);
                xTempStream[i]->seek(0);
                xTempStream[i]->getInputStream()->readBytes(pBytesIn[i], pBytesIn[i].getLength());
                for (int j = 0; j < pBytesCnt; ++j)
                {
                    if (pBytesIn[i][j] != pBytesOut[i][j])
                    {
                        CPPUNIT_FAIL("Stream " + std::to_string(i) + " byte " + std::to_string(j)
                                     + ": INCORRECT DATA!");
                    }
                }
            }
            else
            {
                CPPUNIT_FAIL("Stream " + std::to_string(i) + " is lost!");
            }
        }
    }

    CPPUNIT_TEST_SUITE(OLESimpleStorageUnitTest);
    CPPUNIT_TEST(ExecuteTest01);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(OLESimpleStorageUnitTest);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
