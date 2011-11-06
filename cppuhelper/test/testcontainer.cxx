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
