/*************************************************************************
 *
 *  $RCSfile: weakagg.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: dbo $ $Date: 2001-02-14 10:17:30 $
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

/** */ //for docpp
namespace cppu
{

/**
 * The basic implementation to support weak references and aggregation. The aggregation
 * implementation is based on interfaces, but you should use this class to avoid
 * problems against changes in the future. Overload queryAggregation() instead of queryInterface()
 * to return your interfaces.
 * <BR><B>Not tested.</B>
 *
 * @author  Markus Meyer
 * @since   98/04/12
 */
class OWeakAggObject : public ::cppu::OWeakObject, public ::com::sun::star::uno::XAggregation
{
public:
    /**
     * Set the delegator to null.
     */
    OWeakAggObject()
        {}

    // XInterface
    /**
     * Increment the reference count if no delegator is set, otherwise call
     * acquire at the delegator.
     */
    virtual void SAL_CALL acquire() throw();
    /**
     * Decrement the reference count if no delegator is set, otherwise call
     * acquire at the delegator. If the reference count goes to zero the
     * virtual destructor gets called.
     */
    virtual void SAL_CALL release() throw();
    /**
     * Delegates this call to the delegator, if one is set. Otherwise
     * call the method queryAggregation.
     * @see queryAggregation.
     */
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType )
        throw(::com::sun::star::uno::RuntimeException);

    // XAggregation
    /**
     * Set the Delegator to the xDelegator member. This member is a weak
     * reference.
     * @param Delegator the object that delegate the queryInterface calls.
     */
    virtual void SAL_CALL setDelegator( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > & Delegator )
        throw(::com::sun::star::uno::RuntimeException);
    /**
     * Called from the delegator or queryInterface. Overload this method instead of
     * queryInterface.
     * @see queryInterfaces
     */
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType )
        throw(::com::sun::star::uno::RuntimeException);

protected:
    /**
     * Call the destructor is only allowed if the reference count is zero.
     */
    virtual ~OWeakAggObject()
        throw (::com::sun::star::uno::RuntimeException);

    /**
     * The delegator set with setDelegator.
     * @see setDelegator
     */
    ::com::sun::star::uno::WeakReferenceHelper xDelegator;
private:
    OWeakAggObject( const OWeakObject & rObj );

    OWeakObject &  operator = ( const OWeakObject & rObj );
};

}

#endif
