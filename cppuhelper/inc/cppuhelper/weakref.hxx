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


#ifndef _CPPUHELPER_WEAKREF_HXX_
#define _CPPUHELPER_WEAKREF_HXX_

#include <com/sun/star/uno/XInterface.hpp>

#include "cppuhelper/cppuhelperdllapi.h"


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
class CPPUHELPER_DLLPUBLIC WeakReferenceHelper
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

    /** Releases this reference and takes over hard reference xInt.
        If the implementation behind xInt does not support XWeak
        or XInt is null, then this reference is null.

        @param xInt another hard reference
    */
    WeakReferenceHelper & SAL_CALL operator = (
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::uno::XInterface > & xInt ) SAL_THROW( () );

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

    /** Releases this reference.

        @since UDK 3.2.12
    */
    void SAL_CALL clear() SAL_THROW( () );

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

    /** Releases this reference and takes over hard reference xInt.
        If the implementation behind xInt does not support XWeak
        or XInt is null, then this reference is null.

        @param xInt another hard reference

        @since UDK 3.2.12
    */
    WeakReference & SAL_CALL operator = (
            const ::com::sun::star::uno::Reference< interface_type > & xInt )
        SAL_THROW( () )
        { WeakReferenceHelper::operator=(xInt); return *this; }

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
