/*************************************************************************
 *
 *  $RCSfile: apitools.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: oj $ $Date: 2000-10-27 07:34:13 $
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

#ifndef _DBASHARED_APITOOLS_HXX_
#include "apitools.hxx"
#endif
#ifndef DBACCESS_SHARED_DBASTRINGS_HRC
#include "dbastrings.hrc"
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace cppu;
using namespace osl;
using namespace dbaccess;

//==================================================================================
//= various helper functions
//==================================================================================
//============================================================
//= OSubComponent
//============================================================
//--------------------------------------------------------------------------
OSubComponent::OSubComponent(Mutex& _rMutex, const Reference< XInterface > & xParent)
              :OComponentHelper(_rMutex)
              ,m_xParent(xParent)
{
}

// com::sun::star::lang::XTypeProvider
//--------------------------------------------------------------------------
Sequence< Type > OSubComponent::getTypes() throw (RuntimeException)
{
    OTypeCollection aTypes(::getCppuType( (const Reference< XComponent > *)0 ),
                           ::getCppuType( (const Reference< XTypeProvider > *)0 ),
                           ::getCppuType( (const Reference< XWeak > *)0 ));

    return aTypes.getTypes();
}

// XInterface
//--------------------------------------------------------------------------
void OSubComponent::acquire() throw (RuntimeException)
{
    OComponentHelper::acquire();
}

//--------------------------------------------------------------------------
void OSubComponent::release() throw (RuntimeException)
{
    Reference< XInterface > x( xDelegator );
    if (! x.is())
    {
        if (osl_decrementInterlockedCount( &m_refCount ) == 0)
        {
            if (! rBHelper.bDisposed)
            {
                Reference< XInterface > xHoldAlive( *this );

                // remember the parent
                Reference< XInterface > xParent;
                {
                    MutexGuard aGuard( rBHelper.rMutex );
                    xParent = m_xParent;
                    m_xParent = NULL;
                }

                // First dispose
                dispose();

                // only the alive ref holds the object
                OSL_ASSERT( m_refCount == 1 );

                // release the parent in the ~
                if (xParent.is())
                {
                    MutexGuard aGuard( rBHelper.rMutex );
                    m_xParent = xParent;
                }

                // destroy the object if xHoldAlive decrement the refcount to 0
                return;
            }
        }
        // restore the reference count
        osl_incrementInterlockedCount( &m_refCount );
    }

    // as we cover the job of the componenthelper we use the ...
    OWeakAggObject::release();
}

//--------------------------------------------------------------------------
Any OSubComponent::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aReturn;
    if (!rType.equals(::getCppuType(static_cast< Reference< XAggregation >* >(NULL))))
        aReturn = OComponentHelper::queryInterface(rType);

    return aReturn;
}

//--------------------------------------------------------------------------
void OSubComponent::disposing()
{
    MutexGuard aGuard( rBHelper.rMutex );
    m_xParent = NULL;
}

