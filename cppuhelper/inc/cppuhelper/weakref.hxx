/*************************************************************************
 *
 *  $RCSfile: weakref.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2001-11-14 12:56:58 $
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


namespace com
{
namespace sun
{
namespace star
{
namespace uno
{

/** @internal */
class OWeakRefListener;

/** The WeakReferenceHelper holds a weak reference to an object. This object must implement
    the ::com::sun::star::uno::XWeak interface.  The implementation is thread safe.
*/
class WeakReferenceHelper
{
public:
    /** Default ctor.  Creates an empty weak reference.
    */
    inline WeakReferenceHelper() SAL_THROW( () )
        : m_pImpl( 0 )
        {}

    /** Copy ctor.  Initialize this reference with the same interface as in rWeakRef.

        @param rWeakRef another weak ref
    */
    WeakReferenceHelper( const WeakReferenceHelper & rWeakRef ) SAL_THROW( () );
    /** Initialize this reference with the hard interface reference xInt. If the implementation
        behind xInt does not support XWeak or XInt is null then this reference will be null.

        @param xInt another hard interface reference
    */
    WeakReferenceHelper( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > & xInt )
        SAL_THROW( () );
    /** Releases this reference.
    */
    ~WeakReferenceHelper() SAL_THROW( () );

    /** Releases this reference and takes over rWeakRef.

        @param rWeakRef another weak ref
    */
    WeakReferenceHelper & SAL_CALL operator = ( const WeakReferenceHelper & rWeakRef ) SAL_THROW( () );

    /** Releases this reference and takes over hard reference xInt. If the implementation behind
        xInt does not support XWeak or XInt is null, than this reference is null.

        @param xInt another hard reference
    */
    inline WeakReferenceHelper & SAL_CALL operator = ( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > & xInt ) SAL_THROW( () )
        { return operator = ( WeakReferenceHelper( xInt ) ); }

    /** Returns true if both weak refs reference to the same object.

        @param rObj another weak ref
        @return true, if both weak refs reference to the same object.
    */
    inline sal_Bool SAL_CALL operator == ( const WeakReferenceHelper & rObj ) const SAL_THROW( () )
        { return (get() == rObj.get()); }

    /**  Gets a hard reference to the object.

         @return hard reference or null, if the weakly referenced interface has gone
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL get() const SAL_THROW( () );
    /**  Gets a hard reference to the object.

         @return hard reference or null, if the weakly referenced interface has gone
    */
    inline SAL_CALL operator Reference< XInterface > () const SAL_THROW( () )
        { return get(); }

protected:
    /** @internal */
    OWeakRefListener * m_pImpl;
};

/** The WeakReference<> holds a weak reference to an object. This object must implement
    the ::com::sun::star::uno::XWeak interface.  The implementation is thread safe.

    @tplparam interface_type type of interface
*/
template< class interface_type >
class WeakReference : public WeakReferenceHelper
{
public:
    /** Default ctor.  Creates an empty weak reference.
    */
    inline WeakReference() SAL_THROW( () )
        : WeakReferenceHelper()
        {}

    /** Copy ctor.  Initialize this reference with a hard reference.

        @param rRef another hard ref
    */
    inline WeakReference( const Reference< interface_type > & rRef ) SAL_THROW( () )
        : WeakReferenceHelper( rRef )
        {}

    /**  Gets a hard reference to the object.

         @return hard reference or null, if the weakly referenced interface has gone
    */
    inline SAL_CALL operator Reference< interface_type > () const SAL_THROW( () )
        { return Reference< interface_type >::query( get() ); }
};

}
}
}
}

#endif
