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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cppuhelper.hxx"
#include <osl/mutex.hxx>

#include <cppuhelper/interfacecontainer.hxx>
#include <cppuhelper/implbase1.hxx>

#include <com/sun/star/beans/XVetoableChangeListener.hpp>

using namespace ::cppu;
using namespace ::osl;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;


class TestListener : public WeakImplHelper1< XVetoableChangeListener >
{
public:
    // Methods
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& /*Source*/ ) throw(::com::sun::star::uno::RuntimeException)
        {

        }

    virtual void SAL_CALL vetoableChange( const ::com::sun::star::beans::PropertyChangeEvent& /*aEvent*/ )
       throw(::com::sun::star::beans::PropertyVetoException, ::com::sun::star::uno::RuntimeException)
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
