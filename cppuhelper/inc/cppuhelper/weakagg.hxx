/*************************************************************************
 *
 *  $RCSfile: weakagg.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: dbo $ $Date: 2001-11-09 13:49:15 $
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
#ifndef _CPPUHELPER_WEAKAGG_HXX_
#define _CPPUHELPER_WEAKAGG_HXX_

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_XAGGREGATION_HPP_
#include <com/sun/star/uno/XAggregation.hpp>
#endif


namespace cppu
{

/** Base class to implement an UNO object supporting weak references, i.e. the object can be held
    weakly (by a ::com::sun::star::uno::WeakReference) and aggregation, i.e. the object can be
    aggregated by another (delegator).
    This implementation copes with reference counting.  Upon last release(), the virtual dtor
    is called.

    @derive
    Inherit from this class and delegate acquire()/ release() calls.  Re-implement
    XAggregation::queryInterface().
*/
class OWeakAggObject
    : public ::cppu::OWeakObject
    , public ::com::sun::star::uno::XAggregation
{
public:
    /** Constructor.  No delegator set.
    */
    inline OWeakAggObject() SAL_THROW( () )
        {}

    /** If a delegator is set, then the delegators gets acquired.  Otherwise call is delegated to
        base class ::cppu::OWeakObject.
    */
    virtual void SAL_CALL acquire() throw();
    /** If a delegator is set, then the delegators gets released.  Otherwise call is delegated to
        base class ::cppu::OWeakObject.
    */
    virtual void SAL_CALL release() throw();
    /** If a delegator is set, then the delegator is queried for the demanded interface.  If the
        delegator cannot provide the demanded interface, it calls queryAggregation() on its
        aggregated objects.

        @param rType demanded interface type
        @return demanded type or empty any
        @see queryAggregation.
    */
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType )
        throw(::com::sun::star::uno::RuntimeException);

    /** Set the delegator.  The delegator member reference is a weak reference.

        @param Delegator the object that delegate its queryInterface to this aggregate.
    */
    virtual void SAL_CALL setDelegator( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > & Delegator )
        throw(::com::sun::star::uno::RuntimeException);
    /** Called by the delegator or queryInterface. Re-implement this method instead of
        queryInterface.

        @see queryInterface
    */
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType )
        throw(::com::sun::star::uno::RuntimeException);

protected:
    /** Virtual dtor. Called when reference count is 0.

        @attention
        Despite the fact that a RuntimeException is allowed to be thrown, you must not throw any
        exception upon destruction!
    */
    virtual ~OWeakAggObject() SAL_THROW( (::com::sun::star::uno::RuntimeException) );

    /** weak reference to delegator.
    */
    ::com::sun::star::uno::WeakReferenceHelper xDelegator;
private:
    /** @internal */
    OWeakAggObject( const OWeakObject & rObj ) SAL_THROW( () );
    /** @internal */
    OWeakObject &  operator = ( const OWeakObject & rObj ) SAL_THROW( () );
};

}

#endif
