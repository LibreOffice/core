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

#ifndef CSV_DYN_HXX
#define CSV_DYN_HXX




namespace csv
{


/** Dyn owns an object on the heap, which will be automatically
    deleted in its D'tor.

    Dyn's main purpose is for class members on the heap:
    You can't forget to delete them in the D'tor. Constness will be transfered
    to the hold object.

    Dyn forbids the CopyC'tor and operator=(). So you can't incidentally
    run into problems with compiler defined CopyC'tor or operator=() of the
    owning class. If you need those, you have to define them explicitely - as
    you should do anyway with all classes, that own members on the heap.

    Dyn also works with incomplete types.
    You only need to write
    class DX;
    but needn't include  #include <DX>.hxx.
    This is a difference to std::auto_ptr, where it is not absolutely clear
    if it is allowed to use it with incomplete types.

    You can also use Dyn within function bodies, to make them exception safe.

    @attention
    If you use Dyn with an incomplete type, the owning class needs to
    define a non-inline D'tor. Else the compiler will complain.
*/
template <class DX>
class Dyn
{
  public:
    // LIFECYCLE
    /// From now on, let_dpObject is owned by this Dyn-object.
    explicit            Dyn(
                            DX *                let_dpObject = 0);
                        ~Dyn();
    // OPERATORS
    /** This deletes a prevoiusly existing dpObject!
        From now on, let_dpObject is owned by this Dyn-object.
    */
    Dyn<DX> &           operator=(
                            DX *                let_dpObject);
    /// @return true, if any valid object is hold, false else.
                        operator bool() const;

    const DX *          operator->() const;
    DX *                operator->();

    const DX &          operator*() const;
    DX &                operator*();

    // OPERATIONS
    /** @return The hold object on the heap.

        @ATTENTION
        The caller of the function is responsible to delete
        the returned object

        @postcond
        this->dpObject == 0.
    */
    DX *                Release();

    // INQUIRY
    /// Shorthand for operator->(), if implicit overloading of -> can not be used.
    const DX *          Ptr() const;

    // ACCESS
    /// Shorthand for operator->(), if implicit overloading of -> can not be used.
    DX *                Ptr();
    /// So const objects can return mutable pointers to the owned object.
    DX *                MutablePtr() const;

  private:
      /*  Does NOT set dpObject to zero! Because it is only used
          internally in situations where dpObject is set immediately
          after.
      */
      void              Delete();

      /**   Forbidden function!
          -------------------
          Help ensure, that classes with
          dynamic pointers use a selfdefined copy constructor
          and operator=(). If the default versions of these
          functions are used, the compiler will throw an error.
      **/
                          Dyn( const Dyn<DX> & );
      /**   Forbidden function!
          -------------------
          Help ensure, that classes with
          dynamic pointers use a selfdefined copy constructor
          and operator=(). If the default versions of these
          functions are used, the compiler will throw an error.
      **/
    Dyn<DX> &           operator=( const Dyn<DX> & );

    // DATA
    /// An owned heap object. Needs to be deleted by this class.
    DX *                dpObject;
};




// IMPLEMENTATION
template <class DX>
void
Dyn<DX>::Delete()
{
    if (dpObject != 0)
        delete dpObject;
}

template <class DX>
inline
Dyn<DX>::Dyn( DX * let_dpObject )
    : dpObject(let_dpObject) {}

template <class DX>
inline
Dyn<DX>::~Dyn()
{ Delete(); }


template <class DX>
inline Dyn<DX> &
Dyn<DX>::operator=( DX * let_dpObject )
{
    if ( dpObject == let_dpObject )
        return *this;

    Delete();
    dpObject = let_dpObject;
    return *this;
}

template <class DX>
inline
Dyn<DX>::operator bool() const
{ return dpObject != 0; }

template <class DX>
inline
const DX *
Dyn<DX>::operator->() const
{ return dpObject; }

template <class DX>
inline DX *
Dyn<DX>::operator->()
{ return dpObject; }

template <class DX>
inline const DX &
Dyn<DX>::operator*() const
{ csv_assert(dpObject != 0);
  return *dpObject;
}

template <class DX>
inline DX &
Dyn<DX>::operator*()
{ csv_assert(dpObject != 0);
  return *dpObject;
}

template <class DX>
inline DX *
Dyn<DX>::Release()
{ DX * ret = dpObject;
  dpObject = 0;
  return ret;
}

template <class DX>
inline const DX *
Dyn<DX>::Ptr() const
{ return dpObject; }

template <class DX>
inline DX *
Dyn<DX>::Ptr()
{ return dpObject; }

template <class DX>
inline DX *
Dyn<DX>::MutablePtr() const
{ return dpObject; }

}   // namespace csv




#ifndef CSV_HIDE_DYN
#define Dyn ::csv::Dyn
#endif




#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
