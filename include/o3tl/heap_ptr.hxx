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

#ifndef INCLUDED_O3TL_HEAP_PTR_HXX
#define INCLUDED_O3TL_HEAP_PTR_HXX


#include <boost/assert.hpp>
#include <boost/checked_delete.hpp>


namespace o3tl
{
/** heap_ptr<> owns an object on the heap, which will be automatically
    deleted, when ~heap_ptr<>() is called.

    Applicability
    -------------
    heap_ptr<> can be used for class members on the heap.
      - One cannot forget to delete them in the destructor.
      - Constness will be transfered from the owning instance.

    heap_ptr<> can also be used as smart pointer in function bodies to
    ensure exception safety.

    Special Characteristics
    -----------------------
      - heap_ptr<> transfers constness from the owning object to
        the pointed to object. Such it behaves like if *get() would be
        a normal member of the owning object, not a pointer member.
        This is preferable to the normal pointer behaviour, because
        if an object is owned by another one, it is normally part of
        its state.

      - heap_ptr<> provides a ->release() function

      - For reasons of simplicity and portability ->is()
        is preferred over the safe-bool idiom.

    Copyability
    -----------
    heap_ptr is non copyable.
      - It forbids the copyconstructor and operator=(self).

      - Owning classes will automatically be non copyable, if they do not
        redefine those two functions themselves.

    Incomplete Types
    ----------------
    heap_ptr<> also works with incomplete types. You only need to write
        class T;
    but need not include <T>.hxx.

    If you use heap_ptr<> with an incomplete type, the owning class
    needs to define a non-inline destructor. Else the compiler will
    complain.
*/
template <class T>
class heap_ptr
{
  public:
    typedef T               element_type;       /// Provided for generic programming.
    typedef heap_ptr<T>     self;

    typedef T * (self::*    safe_bool )();

    /// Now, pass_heapObject is owned by this.
    explicit            heap_ptr(
                            T *                 pass_heapObject = 0 );
                        ~heap_ptr();


    /** Identical to reset(), except of the return value.
        @see heap_ptr<>::reset()
    */
    self &              operator=(
                            T *                 pass_heapObject );
    const T &           operator*() const;
    T &                 operator*();
    const T *           operator->() const;
    T *                 operator->();

                        operator safe_bool() const;

    /** This deletes any prevoiusly existing ->pHeapObject.
        Now, pass_heapObject, if != 0, is owned by this.

        @onerror
        Ignores self-assignment.
        Such, multiple assignment of the same pointer to the same
        instance of heap_ptr<> is possible (though not recommended).
    */
    void                reset(
                            T *                 pass_heapObject );
    /** @return     An object on the heap that must be deleted by the caller,
                    or 0.

        @postcond   get() == 0;
    */
    T *                 release();
    void                swap(
                            self &              io_other );

    /// True, if pHeapObject != 0.
    bool                is() const;
    const T *           get() const;
    T *                 get();

  private:
    // Forbidden functions:
                          heap_ptr( const self & );   /// Prevent copies.
    self &              operator=( const self & );  /// Prevent copies.

    /// @attention Does not set ->pHeapObject = 0.
      void              internal_delete();

  // DATA
    /// Will be deleted, when *this is destroyed.
    T *                 pHeapObject;
};


/** Supports the semantic of std::swap(). Provided as an aid to
    generic programming.
*/
template<class T>
inline void
swap( heap_ptr<T> &       io_a,
      heap_ptr<T> &       io_b )
{
    io_a.swap(io_b);
}



// IMPLEMENTATION

template <class T>
inline void
heap_ptr<T>::internal_delete()
{
    ::boost::checked_delete(pHeapObject);

    // Do not set pHeapObject to 0, because
    //   that is reset to a value in all code
    //   where internal_delete() is used.
}

template <class T>
inline
heap_ptr<T>::heap_ptr( T * pass_heapObject )
    : pHeapObject(pass_heapObject)
{
}

template <class T>
inline
heap_ptr<T>::~heap_ptr()
{
    internal_delete();
}

template <class T>
inline heap_ptr<T> &
heap_ptr<T>::operator=(T * pass_heapObject)
{
    reset(pass_heapObject);
    return *this;
}

template <class T>
inline const T &
heap_ptr<T>::operator*() const
{
    BOOST_ASSERT( pHeapObject != 0
                  && "Accessing a heap_ptr<>(0)." );
    return *pHeapObject;
}

template <class T>
inline T &
heap_ptr<T>::operator*()
{
    BOOST_ASSERT( pHeapObject != 0
                  && "Accessing a heap_ptr<>(0)." );
    return *pHeapObject;
}

template <class T>
inline const T *
heap_ptr<T>::operator->() const
{
    return pHeapObject;
}

template <class T>
inline T *
heap_ptr<T>::operator->()
{
    return pHeapObject;
}

template <class T>
inline
heap_ptr<T>::operator typename heap_ptr<T>::safe_bool() const
{
    return is()
            ? safe_bool(&self::get)
            : safe_bool(0);
}

template <class T>
void
heap_ptr<T>::reset(T * pass_heapObject)
{
    if (    pHeapObject != 0
        &&  pHeapObject == pass_heapObject)
        return;

    internal_delete();
    pHeapObject = pass_heapObject;
}

template <class T>
T *
heap_ptr<T>::release()
{
    T * ret = pHeapObject;
    pHeapObject = 0;
    return ret;
}

template <class T>
void
heap_ptr<T>::swap(self & io_other)
{
    T * temp = io_other.pHeapObject;
    io_other.pHeapObject = pHeapObject;
    pHeapObject = temp;
}

template <class T>
inline bool
heap_ptr<T>::is() const
{
    return pHeapObject != 0;
}

template <class T>
inline const T *
heap_ptr<T>::get() const
{
    return pHeapObject;
}

template <class T>
inline T *
heap_ptr<T>::get()
{
    return pHeapObject;
}


}   // namespace o3tl
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
