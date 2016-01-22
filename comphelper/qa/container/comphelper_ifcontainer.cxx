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
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/propshlp.hxx>

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;

class ContainerListener;

struct ContainerStats {
    int m_nAlive;
    int m_nDisposed;
    ContainerStats() : m_nAlive(0), m_nDisposed(0) {}
};

class ContainerListener : public ::cppu::WeakImplHelper1< XEventListener >
{
    ContainerStats *m_pStats;
public:
    explicit ContainerListener(ContainerStats *pStats)
        : m_pStats(pStats) { m_pStats->m_nAlive++; }
    virtual ~ContainerListener() { m_pStats->m_nAlive--; }
    virtual void SAL_CALL disposing( const EventObject& )
        throw (RuntimeException, std::exception) override
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

            CPPUNIT_ASSERT_MESSAGE("Empty container not empty",
                                   pContainer->getLength() == 0);

            int i;
            for (i = 0; i < nTests; i++)
            {
                Reference<XEventListener> xRef = new ContainerListener(&aStats);
                int nNewLen = pContainer->addInterface(xRef);

                CPPUNIT_ASSERT_MESSAGE("addition length mismatch",
                                       nNewLen == i + 1);
                CPPUNIT_ASSERT_MESSAGE("addition length mismatch",
                                       pContainer->getLength() == i + 1);
            }
            CPPUNIT_ASSERT_MESSAGE("alive count mismatch",
                                   aStats.m_nAlive == nTests);

            EventObject aObj;
            pContainer->disposeAndClear(aObj);

            CPPUNIT_ASSERT_MESSAGE("dispose count mismatch",
                                   aStats.m_nDisposed == nTests);
            CPPUNIT_ASSERT_MESSAGE("leaked container left alive",
                                   aStats.m_nAlive == 0);

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
                                   (int)aElements.size(), nTests);
            if ((int)aElements.size() == nTests)
            {
                for (i = 0; i < nTests; i++)
                {
                    CPPUNIT_ASSERT_MESSAGE("mismatching elements",
                                           aElements[i] == aListeners[i]);
                }
            }
            pContainer->clear();

            CPPUNIT_ASSERT_MESSAGE("non-empty container post clear",
                                   pContainer->getLength() == 0);
            delete pContainer;
        }

        template < typename ContainerType, typename ContainedType >
        void doContainerTest(const ContainedType *pTypes)
        {
            ContainerStats aStats;
            ContainerType *pContainer;
            pContainer = new ContainerType(m_aGuard);

            int i;
            Reference<XEventListener> xRefs[nTests * 2];

            // add these interfaces
            for (i = 0; i < nTests * 2; i++)
            {
                xRefs[i] = new ContainerListener(&aStats);
                pContainer->addInterface(pTypes[i / 2], xRefs[i]);
            }

            // check it is all there
            for (i = 0; i < nTests; i++)
            {
                cppu::OInterfaceContainerHelper *pHelper;

                pHelper = pContainer->getContainer(pTypes[i]);

                CPPUNIT_ASSERT_MESSAGE("no helper", pHelper != nullptr);
                Sequence<Reference< XInterface > > aSeq = pHelper->getElements();
                CPPUNIT_ASSERT_MESSAGE("wrong num elements", aSeq.getLength() == 2);
                CPPUNIT_ASSERT_MESSAGE("match", aSeq[0] == xRefs[i*2]);
                CPPUNIT_ASSERT_MESSAGE("match", aSeq[1] == xRefs[i*2+1]);
            }

            // remove every other interface
            for (i = 0; i < nTests; i++)
                pContainer->removeInterface(pTypes[i], xRefs[i*2+1]);

            // check it is half there
            for (i = 0; i < nTests; i++)
            {
                cppu::OInterfaceContainerHelper *pHelper;

                pHelper = pContainer->getContainer(pTypes[i]);

                CPPUNIT_ASSERT_MESSAGE("no helper", pHelper != nullptr);
                Sequence<Reference< XInterface > > aSeq = pHelper->getElements();
                CPPUNIT_ASSERT_MESSAGE("wrong num elements", aSeq.getLength() == 1);
                CPPUNIT_ASSERT_MESSAGE("match", aSeq[0] == xRefs[i*2]);
            }

            // remove the 1st half of the rest
            for (i = 0; i < nTests / 2; i++)
                pContainer->removeInterface(pTypes[i], xRefs[i*2]);

            // check it is half there
            for (i = 0; i < nTests / 2; i++)
            {
                cppu::OInterfaceContainerHelper *pHelper;

                pHelper = pContainer->getContainer(pTypes[i]);
                CPPUNIT_ASSERT_MESSAGE("no helper", pHelper != nullptr);
                Sequence<Reference< XInterface > > aSeq = pHelper->getElements();
                CPPUNIT_ASSERT_MESSAGE("wrong num elements", aSeq.getLength() == 0);
            }

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
