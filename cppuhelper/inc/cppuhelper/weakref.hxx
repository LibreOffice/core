/*************************************************************************
 *
 *  $RCSfile: weakref.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:26:09 $
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

#ifndef _CPPUHELPER_WEAKREF_HXX_
#define _CPPUHELPER_WEAKREF_HXX_

#ifndef _COM_SUN_STAR_UNO_XINTERFACE_HPP_
#include <com/sun/star/uno/XInterface.hpp>
#endif

/** */ //for docpp
namespace com
{
/** */ //for docpp
namespace sun
{
/** */ //for docpp
namespace star
{
/** */ //for docpp
namespace uno
{

class OWeakRefListener;

//===================================================================
/**
 * Hold a weak reference to an object. This object must implement the XWeak interface.
 * The implementation is thread safe. This means you can call set, assign and queryHardRef
 * multible at one time.
 *
 * @see OWeakObject
 * @see OWeakAggObject
 * @author  Markus Meyer
 * @since   12/04/98
 */
class WeakReferenceHelper
{
public:
    /** Create an empty weak reference. */
    WeakReferenceHelper()
        : m_pImpl( 0 )
        {}

    /** Initialize this reference with the same interface as in rWeakRef.*/
    WeakReferenceHelper( const WeakReferenceHelper & rWeakRef );
    /**
     * Initialize this reference with the interface xInt. If the implementation behind
     * xInt does not support XWeak or XInt is empty then this reference is empty too.
     */
    WeakReferenceHelper( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > & xInt );
    /**
     * Release the reference.
     */
    ~WeakReferenceHelper();

    /**
     * Release the reference and take the from rWeakRef.
     */
    WeakReferenceHelper & SAL_CALL operator = ( const WeakReferenceHelper & rWeakRef );

    /**
     * Release the reference and take the from xInt. If the implementation behind
     * xInt does not support XWeak or XInt is empty, than this reference is empty too.
     */
    WeakReferenceHelper & SAL_CALL operator = ( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > & xInt )
        { return operator = ( WeakReferenceHelper( xInt ) ); }

    /**
     * Return true if both OWeakRefs refer to the same object.
     */
    sal_Bool SAL_CALL operator == ( const WeakReferenceHelper & rObj ) const
        { return (get() == rObj.get()); }

    /**
     * Get a real reference to the object.
     * @return Null, if the interface was not found, otherwise the
     *          interface.
     */
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL get() const;

    /**
     * Return the reference to the interface. It is null, if the referenced object
     * was destroyed or the weak reference was not set.
     */
    SAL_CALL operator Reference< XInterface > () const
        { return get(); }

protected:

    OWeakRefListener * m_pImpl;
};

template< class interface_type >
class WeakReference : public WeakReferenceHelper
{
public:
    /** Set an empty weak reference. */
    WeakReference()
        : WeakReferenceHelper()
        {}

    /**
     * Create a new weak reference with the given interface pInterface and acquire this one.
     */
    WeakReference( const Reference<interface_type > & rRef )
        : WeakReferenceHelper( rRef )
        {}

    /**
     * Return the reference to the interface. May be null.
     */
    SAL_CALL operator Reference< interface_type > () const
        { return Reference< interface_type >::query( get() ); }
};

}
}
}
}

#endif


