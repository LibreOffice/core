/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

#include "precompiled_cppuhelper.hxx"

#include "com/sun/star/lang/XEventListener.hpp"
#include "cppuhelper/interfacecontainer.hxx"
#include "cppuhelper/queryinterface.hxx"
#include "cppuhelper/implbase1.hxx"
#include "cppuhelper/propshlp.hxx"
#include "gtest/gtest.h"

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
    class IfTest : public ::testing::Test
    {
    protected:
        osl::Mutex m_aGuard;
        static const int nTests = 10;
    public:

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

                ASSERT_TRUE(pHelper != NULL) << "no helper";
                Sequence<Reference< XInterface > > aSeq = pHelper->getElements();
                ASSERT_TRUE(aSeq.getLength() == 2) << "wrong num elements";
                ASSERT_TRUE(aSeq[0] == xRefs[i*2]) << "match";
                ASSERT_TRUE(aSeq[1] == xRefs[i*2+1]) << "match";
            }

            // remove every other interface
            for (i = 0; i < nTests; i++)
                pContainer->removeInterface(pTypes[i], xRefs[i*2+1]);

            // check it is half there
            for (i = 0; i < nTests; i++)
            {
                cppu::OInterfaceContainerHelper *pHelper;

                pHelper = pContainer->getContainer(pTypes[i]);

                ASSERT_TRUE(pHelper != NULL) << "no helper";
                Sequence<Reference< XInterface > > aSeq = pHelper->getElements();
                ASSERT_TRUE(aSeq.getLength() == 1) << "wrong num elements";
                ASSERT_TRUE(aSeq[0] == xRefs[i*2]) << "match";
            }

            // remove the 1st half of the rest
            for (i = 0; i < nTests / 2; i++)
                pContainer->removeInterface(pTypes[i], xRefs[i*2]);

            // check it is half there
            for (i = 0; i < nTests / 2; i++)
            {
                cppu::OInterfaceContainerHelper *pHelper;

                pHelper = pContainer->getContainer(pTypes[i]);
                ASSERT_TRUE(pHelper != NULL) << "no helper";
                Sequence<Reference< XInterface > > aSeq = pHelper->getElements();
                ASSERT_TRUE(aSeq.getLength() == 0) << "wrong num elements";
            }

            delete pContainer;
        }
    };

    TEST_F(IfTest, testCreateDispose)
    {
        ContainerStats aStats;
        cppu::OInterfaceContainerHelper *pContainer;

        pContainer = new cppu::OInterfaceContainerHelper(m_aGuard);

        ASSERT_TRUE(pContainer->getLength() == 0) << "Empty container not empty";

        int i;
        for (i = 0; i < nTests; i++)
        {
            Reference<XEventListener> xRef = new ContainerListener(&aStats);
            int nNewLen = pContainer->addInterface(xRef);

            ASSERT_TRUE(nNewLen == i + 1) << "addition length mismatch";
            ASSERT_TRUE(pContainer->getLength() == i + 1) << "addition length mismatch";
        }
        ASSERT_TRUE(aStats.m_nAlive == nTests) << "alive count mismatch";

        EventObject aObj;
        pContainer->disposeAndClear(aObj);

        ASSERT_TRUE(aStats.m_nDisposed == nTests) << "dispose count mismatch";
        ASSERT_TRUE(aStats.m_nAlive == 0) << "leaked container left alive";

        delete pContainer;
    }

    TEST_F(IfTest, testEnumerate)
    {
        int i;
        ContainerStats aStats;
        cppu::OInterfaceContainerHelper *pContainer;
        pContainer = new cppu::OInterfaceContainerHelper(m_aGuard);

        std::vector< Reference< XEventListener > > aListeners;
        for (i = 0; i < nTests; i++)
        {
            Reference<XEventListener> xRef = new ContainerListener(&aStats);
            int nNewLen = pContainer->addInterface(xRef);
            aListeners.push_back(xRef);
        }
        Sequence< Reference< XInterface > > aElements;
        aElements = pContainer->getElements();

        ASSERT_TRUE((int)aElements.getLength() == nTests) << "query contents";
        if ((int)aElements.getLength() == nTests)
        {
            for (i = 0; i < nTests; i++)
            {
                ASSERT_TRUE(aElements[i] == aListeners[i]) << "mismatching elements";
            }
        }
        pContainer->clear();

        ASSERT_TRUE(pContainer->getLength() == 0) << "non-empty container post clear";
        delete pContainer;
    }

    TEST_F(IfTest, testOMultiTypeInterfaceContainerHelper)
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

    TEST_F(IfTest, testOMultiTypeInterfaceContainerHelperInt32)
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

    TEST_F(IfTest, testOMultiTypeInterfaceContainerHelperVar)
    {
        typedef ::cppu::OMultiTypeInterfaceContainerHelperVar<
            const char*, rtl::CStringHash, rtl::CStringEqual> StrContainer;

        const char *pTypes[nTests] =
        {
            "this_is", "such", "fun", "writing", "unit", "tests", "when", "it", "works", "anyway"
        };
        doContainerTest< StrContainer, const char *> (pTypes);
    }


} // namespace cppu_ifcontainer

