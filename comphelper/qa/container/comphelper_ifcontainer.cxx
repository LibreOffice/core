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

#include <sal/types.h>

#include <string.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <com/sun/star/lang/XEventListener.hpp>
#include <comphelper/interfacecontainer2.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/propshlp.hxx>

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;

struct ContainerStats {
    int m_nAlive;
    int m_nDisposed;
    ContainerStats() : m_nAlive(0), m_nDisposed(0) {}
};

class ContainerListener : public cppu::WeakImplHelper< XEventListener >
{
    ContainerStats * const m_pStats;
public:
    explicit ContainerListener(ContainerStats *pStats)
        : m_pStats(pStats) { m_pStats->m_nAlive++; }
    virtual ~ContainerListener() override { m_pStats->m_nAlive--; }
    virtual void SAL_CALL disposing( const EventObject& ) override
    {
        m_pStats->m_nDisposed++;
    }
};

namespace comphelper_ifcontainer
{
    static const int nTests = 10;
    class IfTest : public CppUnit::TestFixture
    {
        osl::Mutex m_aGuard;
    public:
        void testCreateDispose()
        {
            ContainerStats aStats;
            comphelper::OInterfaceContainerHelper2 *pContainer;

            pContainer = new comphelper::OInterfaceContainerHelper2(m_aGuard);

            CPPUNIT_ASSERT_EQUAL_MESSAGE("Empty container not empty",
                                   static_cast<sal_Int32>(0), pContainer->getLength());

            int i;
            for (i = 0; i < nTests; i++)
            {
                Reference<XEventListener> xRef = new ContainerListener(&aStats);
                int nNewLen = pContainer->addInterface(xRef);

                CPPUNIT_ASSERT_EQUAL_MESSAGE("addition length mismatch",
                                       i + 1, nNewLen);
                CPPUNIT_ASSERT_EQUAL_MESSAGE("addition length mismatch",
                                       static_cast<sal_Int32>(i + 1), pContainer->getLength());
            }
            CPPUNIT_ASSERT_EQUAL_MESSAGE("alive count mismatch",
                                   nTests, aStats.m_nAlive);

            EventObject aObj;
            pContainer->disposeAndClear(aObj);

            CPPUNIT_ASSERT_EQUAL_MESSAGE("dispose count mismatch",
                                   nTests, aStats.m_nDisposed);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("leaked container left alive",
                                   0, aStats.m_nAlive);

            delete pContainer;
        }

        void testEnumerate()
        {
            int i;
            ContainerStats aStats;
            comphelper::OInterfaceContainerHelper2 *pContainer;
            pContainer = new comphelper::OInterfaceContainerHelper2(m_aGuard);

            std::vector< Reference< XEventListener > > aListeners;
            for (i = 0; i < nTests; i++)
            {
                Reference<XEventListener> xRef = new ContainerListener(&aStats);
                pContainer->addInterface(xRef);
                aListeners.push_back(xRef);
            }
            std::vector< Reference< XInterface > > aElements = pContainer->getElements();

            CPPUNIT_ASSERT_EQUAL_MESSAGE("query contents",
                                   nTests, static_cast<int>(aElements.size()));
            if (static_cast<int>(aElements.size()) == nTests)
            {
                for (i = 0; i < nTests; i++)
                {
                    CPPUNIT_ASSERT_MESSAGE("mismatching elements",
                                           bool(aElements[i] == aListeners[i]));
                }
            }
            pContainer->clear();

            CPPUNIT_ASSERT_EQUAL_MESSAGE("non-empty container post clear",
                                   static_cast<sal_Int32>(0), pContainer->getLength());
            delete pContainer;
        }

        // Automatic registration code
        CPPUNIT_TEST_SUITE(IfTest);
        CPPUNIT_TEST(testCreateDispose);
        CPPUNIT_TEST(testEnumerate);
        CPPUNIT_TEST_SUITE_END();
    };
} // namespace cppu_ifcontainer

CPPUNIT_TEST_SUITE_REGISTRATION(comphelper_ifcontainer::IfTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
