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

#include <osl/mutex.hxx>

#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/implbase.hxx>

#include <com/sun/star/beans/XVetoableChangeListener.hpp>

using namespace ::cppu;
using namespace ::osl;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;


class TestListener : public WeakImplHelper < XVetoableChangeListener >
{
public:
    // Methods
    virtual void SAL_CALL disposing( const css::lang::EventObject& /*Source*/ ) throw(css::uno::RuntimeException)
        {

        }

    virtual void SAL_CALL vetoableChange( const css::beans::PropertyChangeEvent& /*aEvent*/ )
       throw(css::beans::PropertyVetoException, css::uno::RuntimeException)
        {

        }
};

void test_interfacecontainer()
{
    Mutex mutex;

    {
        OInterfaceContainerHelper helper( mutex );

        Reference< XVetoableChangeListener > r1 = new TestListener();
        Reference< XVetoableChangeListener > r2 = new TestListener();
        Reference< XVetoableChangeListener > r3 = new TestListener();

        helper.addInterface( r1 );
        helper.addInterface( r2 );
        helper.addInterface( r3 );

        helper.disposeAndClear( EventObject() );
    }

    {
        OInterfaceContainerHelper helper( mutex );

        Reference< XVetoableChangeListener > r1 = new TestListener();
        Reference< XVetoableChangeListener > r2 = new TestListener();
        Reference< XVetoableChangeListener > r3 = new TestListener();

        helper.addInterface( r1 );
        helper.addInterface( r2 );
        helper.addInterface( r3 );

        OInterfaceIteratorHelper iterator( helper );

        while( iterator.hasMoreElements() )
            ((XVetoableChangeListener*)iterator.next())->vetoableChange( PropertyChangeEvent() );

        helper.disposeAndClear( EventObject() );
    }

    {
        OInterfaceContainerHelper helper( mutex );

        Reference< XVetoableChangeListener > r1 = new TestListener();
        Reference< XVetoableChangeListener > r2 = new TestListener();
        Reference< XVetoableChangeListener > r3 = new TestListener();

        helper.addInterface( r1 );
        helper.addInterface( r2 );
        helper.addInterface( r3 );

        OInterfaceIteratorHelper iterator( helper );

        ((XVetoableChangeListener*)iterator.next())->vetoableChange( PropertyChangeEvent() );
        iterator.remove();
        ((XVetoableChangeListener*)iterator.next())->vetoableChange( PropertyChangeEvent() );
        iterator.remove();
        ((XVetoableChangeListener*)iterator.next())->vetoableChange( PropertyChangeEvent() );
        iterator.remove();

        OSL_ASSERT( helper.getLength() == 0 );
        helper.disposeAndClear( EventObject() );
    }

    {
        OInterfaceContainerHelper helper( mutex );

        Reference< XVetoableChangeListener > r1 = new TestListener();
        Reference< XVetoableChangeListener > r2 = new TestListener();
        Reference< XVetoableChangeListener > r3 = new TestListener();

        helper.addInterface( r1 );
        helper.addInterface( r2 );
        helper.addInterface( r3 );

        {
            OInterfaceIteratorHelper iterator( helper );
            while( iterator.hasMoreElements() )
            {
                Reference< XVetoableChangeListener > r = ((XVetoableChangeListener*)iterator.next());
                if( r == r1 )
                    iterator.remove();
            }
        }
        OSL_ASSERT( helper.getLength() == 2 );
        {
            OInterfaceIteratorHelper iterator( helper );
            while( iterator.hasMoreElements() )
            {
                Reference< XVetoableChangeListener > r = ((XVetoableChangeListener*)iterator.next());
                OSL_ASSERT( r != r1 && ( r == r2 || r == r3 ) );
            }
        }

        helper.disposeAndClear( EventObject() );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
