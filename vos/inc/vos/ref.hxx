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




#ifndef _VOS_REF_HXX_
#define _VOS_REF_HXX_

/**
    ORef<T>

    template type to implement handle/body behaviour
    with reference-counting.

    Note that the template-type T MUST implement IReferenceCounter.

*/

#   include <vos/refernce.hxx>
#ifndef _VOS_DIAGNOSE_HXX_
#   include <vos/diagnose.hxx>
#endif

namespace vos
{


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

}


#endif // _VOS_REF_HXX_
