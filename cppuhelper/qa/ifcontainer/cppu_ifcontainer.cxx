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

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <com/sun/star/lang/XEventListener.hpp>
#include <cppuhelper/interfacecontainer.hxx>
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

class ContainerListener : public ::cppu::WeakImplHelper< XEventListener >
{
    ContainerStats *m_pStats;
public:
    explicit ContainerListener(ContainerStats *pStats)
        : m_pStats(pStats) { m_pStats->m_nAlive++; }
    virtual ~ContainerListener() override { m_pStats->m_nAlive--; }
    virtual void SAL_CALL disposing( const EventObject& ) override
    {
        m_pStats->m_nDisposed++;
    }
};

namespace cppu_ifcontainer
{
    class IfTest : public CppUnit::TestFixture
    {
        osl::Mutex m_aGuard;
        static const int nTests = 10;
    public:
        void testCreateDispose()
        {
            ContainerStats aStats;
            cppu::OInterfaceContainerHelper *pContainer;

            pContainer = new cppu::OInterfaceContainerHelper(m_aGuard);

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
            CPPUNIT_ASSERT_MESSAGE("alive count mismatch",
                                   bool(aStats.m_nAlive == nTests));

            EventObject aObj;
            pContainer->disposeAndClear(aObj);

            CPPUNIT_ASSERT_MESSAGE("dispose count mismatch",
                                   bool(aStats.m_nDisposed == nTests));
            CPPUNIT_ASSERT_EQUAL_MESSAGE("leaked container left alive",
                                   0, aStats.m_nAlive);

            delete pContainer;
        }

        void testEnumerate()
        {
            int i;
            ContainerStats aStats;
            cppu::OInterfaceContainerHelper *pContainer;
            pContainer = new cppu::OInterfaceContainerHelper(m_aGuard);

            std::vector< Reference< XEventListener > > aListeners;
            for (i = 0; i < nTests; i++)
            {
                Reference<XEventListener> xRef = new ContainerListener(&aStats);
                pContainer->addInterface(xRef);
                aListeners.push_back(xRef);
            }
            Sequence< Reference< XInterface > > aElements;
            aElements = pContainer->getElements();

            CPPUNIT_ASSERT_MESSAGE("query contents",
                                   bool(static_cast<int>(aElements.getLength()) == nTests));
            if (static_cast<int>(aElements.getLength()) == nTests)
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
                CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong num elements", static_cast<sal_Int32>(2), aSeq.getLength());
                CPPUNIT_ASSERT_MESSAGE("match", bool(aSeq[0] == xRefs[i*2]));
                CPPUNIT_ASSERT_MESSAGE("match", bool(aSeq[1] == xRefs[i*2+1]));
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
                CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong num elements", static_cast<sal_Int32>(1), aSeq.getLength());
                CPPUNIT_ASSERT_MESSAGE("match", bool(aSeq[0] == xRefs[i*2]));
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
                CPPUNIT_ASSERT_EQUAL_MESSAGE("wrong num elements", static_cast<sal_Int32>(0), aSeq.getLength());
            }

            delete pContainer;
        }

        void testOMultiTypeInterfaceContainerHelper()
        {
            uno::Type pTypes[nTests] =
            {
                ::cppu::UnoType< bool >::get(),
                ::cppu::UnoType< float >::get(),
                ::cppu::UnoType< double >::get(),
                ::cppu::UnoType< ::sal_uInt64 >::get(),
                ::cppu::UnoType< ::sal_Int64 >::get(),
                ::cppu::UnoType< ::sal_uInt32 >::get(),
                ::cppu::UnoType< ::sal_Int32 >::get(),
                ::cppu::UnoType< ::sal_Int16 >::get(),
                ::cppu::UnoType< OUString >::get(),
                ::cppu::UnoType< ::sal_Int8 >::get()
            };
            doContainerTest< cppu::OMultiTypeInterfaceContainerHelper,
                uno::Type> (pTypes);
        }

        void testOMultiTypeInterfaceContainerHelperInt32()
        {
            sal_Int32 const pTypes[nTests] =
            {
                0,
                -1,
                1,
                256,
                1024,
                3,
                7,
                8,
                9,
                10
            };
            doContainerTest< cppu::OMultiTypeInterfaceContainerHelperInt32, sal_Int32> (pTypes);
        }

        void testOMultiTypeInterfaceContainerHelperVar()
        {
            typedef cppu::OMultiTypeInterfaceContainerHelperVar<
                char const *, void, rtl::CStringEqual> StrContainer;

            const char * const pTypes[nTests] =
            {
                "this_is", "such", "fun", "writing", "unit", "tests", "when", "it", "works", "anyway"
            };
            doContainerTest< StrContainer, const char *> (pTypes);
        }

        // Automatic registration code
        CPPUNIT_TEST_SUITE(IfTest);
        CPPUNIT_TEST(testCreateDispose);
        CPPUNIT_TEST(testEnumerate);
        CPPUNIT_TEST(testOMultiTypeInterfaceContainerHelper);
        CPPUNIT_TEST(testOMultiTypeInterfaceContainerHelperVar);
        CPPUNIT_TEST(testOMultiTypeInterfaceContainerHelperInt32);
        CPPUNIT_TEST_SUITE_END();
    };
} // namespace cppu_ifcontainer

CPPUNIT_TEST_SUITE_REGISTRATION(cppu_ifcontainer::IfTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
