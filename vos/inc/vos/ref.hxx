/*************************************************************************
 *
 *  $RCSfile: ref.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:18:12 $
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


#ifndef _VOS_REF_HXX_
#define _VOS_REF_HXX_

/**
    ORef<T>

    template type to implement handle/body behaviour
    with reference-counting.

    Note that the template-type T MUST implement IReferenceCounter.

*/

#ifndef _VOS_REFERNCE_HXX_
#   include <vos/refernce.hxx>
#endif
#ifndef _VOS_DIAGNOSE_HXX_
#   include <vos/diagnose.hxx>
#endif

#ifdef _USE_NAMESPACE
namespace vos
{
#endif


template <class T>
class ORef
{
public:

    /** Creates an "empty" reference, use "create()" or
        assignment/copy-operator to make it a valid reference.
    */
    ORef();

    /** Creates a reference which points to pBody.
        pBodys reference is not changed!. (like create(...)).
    */
    ORef(T* pBody);

    /** Overload copy-constr. to implement ref-counting.
        As a result of the following code:

            ORef<x> h1, h2;

            h1.create();
            h2= h1;

        h1 and h2 will represent the same body.

    */
    ORef(const ORef<T>& handle);

    /** Decreases ref-count of underlying body.
    */
    inline ~ORef();

    /** Overload assignment-operator to implement ref-counting.
        Unbinds this instance from its body (if bound) and
        bind it to the body represented by the handle.
    */
    ORef<T>& SAL_CALL operator= (const ORef<T>& handle);



    /** Binds the body to this handle.
        The "pBody"s reference-count is increased.

        If you call bind() on an existing body,
        the old body is unbound before the new one is
        assigned.

    */
    void SAL_CALL bind(T* pBody);

    /** Unbind the body from this handle.
        Note that for a handle representing a large body,
        "handle.unbind().bind(new body());" _might_
        perform a little bit better than "handle.bind(new body());",
        since in the second case two large objects exist in memory
        (the old body and the new body).
    */
    ORef<T>& SAL_CALL unbind();

    /** Same as bind().
    */
    void SAL_CALL operator= (T* pBody);

    /** Just in case you want to call handle().someBodyOp()...
    */
    T& SAL_CALL operator() () const;

    /** Allows (*handle).someBodyOp().
    */
    T& SAL_CALL operator* () const;

    /** Probably most common used: handle->someBodyOp().
    */
    T* SAL_CALL operator->() const;

    /** Gives access to the handles body.
    */
    T& SAL_CALL getBody() const;

    /** Can be used instead of operator->.
        I.e. handle->someBodyOp() and handle.getBodyPtr()->someBodyOp()
        are the same.
    */
    T* SAL_CALL getBodyPtr() const;

    /** Returns True is the body is empty (the handle
        does not point to a valid body).
    */
    sal_Bool SAL_CALL isEmpty() const;

    /** Returns True is the body is "full" (the handle
        does point to a valid body).
    */
    sal_Bool SAL_CALL isValid() const;

    /** Returns True is handle points to the same body.
    */
    sal_Bool SAL_CALL isEqualBody(const ORef<T>& handle) const;

    /** Delegates comparison to the body.
    */
    sal_Bool SAL_CALL operator== (const ORef<T>& handle) const;

    /** Delegates comparison to the body.
    */
    sal_Bool SAL_CALL operator!= (const ORef<T>& handle) const;

    /** Returns True is "this" points to pBody.
    */
    sal_Bool SAL_CALL operator== (const T* pBody) const;

    /** Needed to place ORefs into STL collection.
        Delegates comparison to the body.
    */
    sal_Bool SAL_CALL operator< (const ORef<T>& handle) const;

    /** Needed to place ORefs into STL collection.
        Delegates comparison to the body.
    */
    sal_Bool SAL_CALL operator> (const ORef<T>& handle) const;

protected:

    T* m_refBody;
};

// include template implementation
#include <vos/ref.inl>

#ifdef _USE_NAMESPACE
}
#endif


#endif // _VOS_REF_HXX_
