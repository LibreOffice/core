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

#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/plugin/TestPlugIn.h"
#include <osl/mutex.hxx>
#include <comphelper/interfacecontainer2.hxx>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/beans/XVetoableChangeListener.hpp>

using namespace ::osl;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;

namespace
{

class TestInterfaceContainer2: public CppUnit::TestFixture
{
public:
    void test1();

    CPPUNIT_TEST_SUITE(TestInterfaceContainer2);
    CPPUNIT_TEST(test1);
    CPPUNIT_TEST_SUITE_END();
};

class TestListener : public cppu::WeakImplHelper1< XVetoableChangeListener >
{
public:
    // Methods
    virtual void SAL_CALL disposing( const css::lang::EventObject& /*Source*/ ) throw(css::uno::RuntimeException) override
        {

        }

    virtual void SAL_CALL vetoableChange( const css::beans::PropertyChangeEvent& /*aEvent*/ )
       throw(css::beans::PropertyVetoException, css::uno::RuntimeException) override
        {

        }
};

void TestInterfaceContainer2::test1()
{
    Mutex mutex;

    {
        comphelper::OInterfaceContainerHelper2 helper( mutex );

        Reference< XVetoableChangeListener > r1 = new TestListener();
        Reference< XVetoableChangeListener > r2 = new TestListener();
        Reference< XVetoableChangeListener > r3 = new TestListener();

        helper.addInterface( r1 );
        helper.addInterface( r2 );
        helper.addInterface( r3 );

        helper.disposeAndClear( EventObject() );
    }

    {
        comphelper::OInterfaceContainerHelper2 helper( mutex );

        Reference< XVetoableChangeListener > r1 = new TestListener();
        Reference< XVetoableChangeListener > r2 = new TestListener();
        Reference< XVetoableChangeListener > r3 = new TestListener();

        helper.addInterface( r1 );
        helper.addInterface( r2 );
        helper.addInterface( r3 );

        comphelper::OInterfaceIteratorHelper2 iterator( helper );

        while( iterator.hasMoreElements() )
            static_cast<XVetoableChangeListener*>(iterator.next())->vetoableChange( PropertyChangeEvent() );

        helper.disposeAndClear( EventObject() );
    }

    {
        comphelper::OInterfaceContainerHelper2 helper( mutex );

        Reference< XVetoableChangeListener > r1 = new TestListener();
        Reference< XVetoableChangeListener > r2 = new TestListener();
        Reference< XVetoableChangeListener > r3 = new TestListener();

        helper.addInterface( r1 );
        helper.addInterface( r2 );
        helper.addInterface( r3 );

        comphelper::OInterfaceIteratorHelper2 iterator( helper );

        static_cast<XVetoableChangeListener*>(iterator.next())->vetoableChange( PropertyChangeEvent() );
        iterator.remove();
        static_cast<XVetoableChangeListener*>(iterator.next())->vetoableChange( PropertyChangeEvent() );
        iterator.remove();
        static_cast<XVetoableChangeListener*>(iterator.next())->vetoableChange( PropertyChangeEvent() );
        iterator.remove();

        CPPUNIT_ASSERT( helper.getLength() == 0 );
        helper.disposeAndClear( EventObject() );
    }

    {
        comphelper::OInterfaceContainerHelper2 helper( mutex );

        Reference< XVetoableChangeListener > r1 = new TestListener();
        Reference< XVetoableChangeListener > r2 = new TestListener();
        Reference< XVetoableChangeListener > r3 = new TestListener();

        helper.addInterface( r1 );
        helper.addInterface( r2 );
        helper.addInterface( r3 );

        {
            comphelper::OInterfaceIteratorHelper2 iterator( helper );
            while( iterator.hasMoreElements() )
            {
                Reference< XVetoableChangeListener > r = static_cast<XVetoableChangeListener*>(iterator.next());
                if( r == r1 )
                    iterator.remove();
            }
        }
        CPPUNIT_ASSERT( helper.getLength() == 2 );
        {
            comphelper::OInterfaceIteratorHelper2 iterator( helper );
            while( iterator.hasMoreElements() )
            {
                Reference< XVetoableChangeListener > r = static_cast<XVetoableChangeListener*>(iterator.next());
                CPPUNIT_ASSERT( r != r1 && ( r == r2 || r == r3 ) );
            }
        }

        helper.disposeAndClear( EventObject() );
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(TestInterfaceContainer2);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
