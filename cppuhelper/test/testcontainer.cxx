/*************************************************************************
 *
 *  $RCSfile: testcontainer.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jbu $ $Date: 2001-02-05 13:23:12 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
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
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException)
        {

        }

    virtual void SAL_CALL vetoableChange( const ::com::sun::star::beans::PropertyChangeEvent& aEvent )
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
