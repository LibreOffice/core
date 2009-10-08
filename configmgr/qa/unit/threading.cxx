/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: threading.cxx,v $
 * $Revision: 1.3 $
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

#include "common.hxx"
#include <osl/thread.hxx>
#include <osl/conditn.hxx>
#include <cppuhelper/implbase1.hxx>
#include "com/sun/star/beans/XPropertyChangeListener.hpp"

class KeyTester : public osl::Thread
{
    osl::Condition *m_pCond;
  protected:
    rtl::OString    m_aPath;
    rtl::OString    m_aKey;
    Test           *m_pTest;
  public:
    KeyTester (osl::Condition *pCond, Test *pTest,
               const char *pPath, const char *pKey)
        : m_pCond (pCond)
        , m_aPath (pPath)
        , m_aKey (pKey)
        , m_pTest (pTest)
    {
    }
    virtual ~KeyTester ()
    {
    }
    virtual void run ()
    {
        testIteration();
        while (!m_pCond->check())
            testIteration();
    }
    virtual void testIteration()
    {
        css::uno::Any a = m_pTest->getKey (m_aPath, m_aKey);
        CPPUNIT_ASSERT_MESSAGE ("no value", a.hasValue());
    }
};

class KeyReader : public KeyTester
{
public:
    KeyReader (osl::Condition *pCond, Test *pTest,
               const char *pPath, const char *pKey)
        : KeyTester (pCond, pTest, pPath, pKey)
    {
        // to ensure we have the right vtable when we hit 'run'
        create();
    }
};

class KeyWriter : public KeyTester
{
    int curOpt;
  public:
    KeyWriter (osl::Condition *pCond, Test *pTest,
               const char *pPath, const char *pKey)
        : KeyTester (pCond, pTest, pPath, pKey)
        , curOpt(0)
    {
        m_pTest->normalizePathKey (m_aPath, m_aKey);
        create();
    }
    virtual void testIteration ()
    {
        try {
            static const char *options[] = { "fish", "chips", "kippers", "bloaters" };
//            fprintf (stderr, "set key %d\n",
//                     (int) osl_getThreadIdentifier(NULL));
            m_pTest->setKey (m_aPath, rtl::OUString::createFromAscii (m_aKey),
                             css::uno::makeAny (rtl::OUString::createFromAscii(options[(curOpt++ & 3)])));
        } CATCH_FAIL ("setting keys")
    }
};

void Test::threadTests()
{
    osl::Condition stop;
    stop.reset();

    struct {
        const char *pPath;
        const char *pKey;
    } keyList[] = {
        { "/org.openoffice.Setup", "Test/AString" },
        { "/org.openoffice.Setup", "Test/AString" },
        { "/org.openoffice.UI.GenericCommands", "UserInterface/Commands/dotuno:WebHtml/Label" },
        { "/org.openoffice.UI.GenericCommands", "UserInterface/Commands/dotuno:NewPresentation/Label" },
        { "/org.openoffice.UI.GenericCommands", "UserInterface/Commands/dotuno:RecentFileList/Label" },

        { "/org.openoffice.Setup", "L10N/ooLocale" },
        { "/org.openoffice.Setup", "Test/ABoolean" }
    };
    const int numReaders = sizeof (keyList) / sizeof (keyList[0]);
    const int numWriters = (sizeof (keyList) / sizeof (keyList[0])) - 2;
    KeyReader *pReaders[numReaders];
    KeyWriter *pWriters[numReaders];

    int i;
    try {
        for (i = 0; i < numReaders; i++) {
            css::uno::Any a = getKey (keyList[i].pPath, keyList[i].pKey);
            CPPUNIT_ASSERT_MESSAGE ("check key", a.hasValue());
        }

        // a few readers
        for (i = 0; i < numReaders; i++)
            pReaders[i] = new KeyReader (&stop, this, keyList[i].pPath,
                                         keyList[i].pKey);
        // a few writers
        for (i = 0; i < numWriters; i++)
            pWriters[i] = new KeyWriter (&stop, this, keyList[i].pPath,
                                         keyList[i].pKey);

        // Threads are running ...
        const int numIters = 5;
        for (int j = 0; j < numIters; j++) {
            for (i = 0; i < numReaders; i++)
            {
                try {
                    rtl::OString aPath (keyList[i].pPath);
                    rtl::OString aKey (keyList[i].pKey);
                    normalizePathKey (aPath, aKey);
                    resetKey (aPath, rtl::OUString::createFromAscii (aKey));
                    osl::Thread::yield();
                } CATCH_FAIL ("resetting keys");
            }
        }
        stop.set();

        for (i = 0; i < numReaders; i++) {
            pReaders[i]->join();
            delete pReaders[i];
        }
        for (i = 0; i < numWriters; i++) {
            pWriters[i]->join();
            delete pWriters[i];
        }

    } CATCH_FAIL ("checking keys exist")
}

class RecursiveListener : public cppu::WeakImplHelper1< css::beans::XPropertyChangeListener >
{
public:
    sal_Int32 m_nRecurse;
    css::uno::Reference< css::beans::XPropertySet > mxPropSet;
  protected:
    Test *m_pTest;
    rtl::OString m_pPath;
    rtl::OString m_pKey;
  public:
    RecursiveListener (Test *pTest, int nCount,
                       const char *pPath, const char *pKey)
        : m_nRecurse (nCount)
        , m_pTest (pTest)
        , m_pPath (pPath)
        , m_pKey (pKey)
    {
        mxPropSet = css::uno::Reference< css::beans::XPropertySet > (
            pTest->createView (pPath, true), css::uno::UNO_QUERY_THROW );

        CPPUNIT_ASSERT_MESSAGE ("is prop set", mxPropSet.is());
        mxPropSet->addPropertyChangeListener (rtl::OUString::createFromAscii (m_pKey),
                                              css::uno::Reference<css::beans::XPropertyChangeListener>(this));
    }
    virtual ~RecursiveListener()
    {
        disposeComponent (mxPropSet);
    }

    virtual void SAL_CALL acquire() throw() { cppu::WeakImplHelper1< css::beans::XPropertyChangeListener >::acquire(); }
    virtual void SAL_CALL release() throw() { cppu::WeakImplHelper1< css::beans::XPropertyChangeListener >::acquire(); }
    // XPropertyChangeListener
    virtual void SAL_CALL propertyChange( const ::css::beans::PropertyChangeEvent& ) throw (::css::uno::RuntimeException)
    {
        if (m_nRecurse-- > 0)
            runTest();
    }
    // XEventListener
    virtual void SAL_CALL disposing( const ::css::lang::EventObject& ) throw (::css::uno::RuntimeException)
    {
    }
    virtual void runTest()
    {
        m_pTest->setKey (m_pPath, rtl::OUString::createFromAscii (m_pKey),
                         css::uno::makeAny(
                             rtl::OUString::valueOf (m_nRecurse) ) );
    }
};

class CrossThreadListener : public RecursiveListener
{
  public:
    CrossThreadListener (Test *pTest, int nCount,
                         const char *pPath, const char *pKey)
        : RecursiveListener (pTest, nCount, pPath, pKey)
    {
    }
    virtual ~CrossThreadListener()
    {
    }
    virtual void runTest()
    {
        osl::Condition stopAfterOne;
        stopAfterOne.set();
        KeyWriter aWriter (&stopAfterOne, m_pTest, m_pPath, m_pKey);
        aWriter.join();

        rtl::OString aPath (m_pPath), aKey (m_pKey);
        m_pTest->normalizePathKey (aPath, aKey);
        m_pTest->resetKey (aPath, rtl::OUString::createFromAscii (aKey));
    }
};

void Test::recursiveTests()
{
    RecursiveListener *pList = new RecursiveListener(this, 100,
                                                     "/org.openoffice.UI.GenericCommands/UserInterface/Commands/dotuno:WebHtml",
                                                     "Label");
    css::uno::Reference< css::beans::XPropertyChangeListener > xListener(pList);
    pList->runTest();
}

void Test::eventTests()
{
    CrossThreadListener *pList = new CrossThreadListener(this, 10,
                                                         "/org.openoffice.UI.GenericCommands/UserInterface/Commands/dotuno:WebHtml",
                                                         "Label");
    css::uno::Reference< css::beans::XPropertyChangeListener > xListener(pList);
    pList->runTest();
}

