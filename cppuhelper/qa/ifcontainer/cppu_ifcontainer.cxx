/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <string.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include "com/sun/star/lang/XEventListener.hpp"
#include "cppuhelper/interfacecontainer.hxx"
#include "cppuhelper/queryinterface.hxx"
#include "cppuhelper/implbase1.hxx"
#include "cppuhelper/propshlp.hxx"

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;


struct equalStr
{
    bool operator()(
        const char * const &rA,
        const char * const &rB) const
        { return !strcmp(rA, rB); }
};
struct hashStr
{
    size_t operator()( const char * &rName ) const
    {
        return rtl::OString(rName).hashCode();
    }
};

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
    ContainerListener(ContainerStats *pStats)
        : m_pStats(pStats) { m_pStats->m_nAlive++; }
    virtual ~ContainerListener() { m_pStats->m_nAlive--; }
    virtual void SAL_CALL disposing( const EventObject& )
        throw (RuntimeException)
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
                                   (int)aElements.getLength() == nTests);
            if ((int)aElements.getLength() == nTests)
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

                CPPUNIT_ASSERT_MESSAGE("no helper", pHelper != NULL);
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

                CPPUNIT_ASSERT_MESSAGE("no helper", pHelper != NULL);
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
                CPPUNIT_ASSERT_MESSAGE("no helper", pHelper != NULL);
                Sequence<Reference< XInterface > > aSeq = pHelper->getElements();
                CPPUNIT_ASSERT_MESSAGE("wrong num elements", aSeq.getLength() == 0);
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
                ::cppu::UnoType< ::rtl::OUString >::get(),
                ::cppu::UnoType< ::sal_Int8 >::get()
            };
            doContainerTest< cppu::OMultiTypeInterfaceContainerHelper,
                uno::Type> (pTypes);
        }

        void testOMultiTypeInterfaceContainerHelperInt32()
        {
            sal_Int32 pTypes[nTests] =
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
            typedef ::cppu::OMultiTypeInterfaceContainerHelperVar<
                const char *,hashStr,equalStr> StrContainer;

            const char *pTypes[nTests] =
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
