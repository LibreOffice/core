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
#ifndef _TOOLS_WEAKBASE_H_
#define _TOOLS_WEAKBASE_H_

#include <sal/types.h>
#include <osl/diagnose.h>

/** the template classes in this header are helper to implement weak references
    to implementation objects that are not refcounted.

    THIS IS NOT THREADSAFE

    Use this only to have 'safe' pointers to implementation objects that you
    don't own but that you reference with a pointer.

    Example:

    class ImplClass : public tools::WeakBase< ImplClass >
    {
        ~ImplClass() { clearWeek(); } // not needed but safer, see method description
        ...
    };

    class UserClass
    {
        tools::WeakReference< ImplClass > mxWeakRef;

        UserClass( ImplClass* pOjbect ) : mxWeakRef( pObject ) {}

        DoSomething()
        {
            if( mxWeakRef.is() )
                mxWeakRef->DoSomethingMore();
        }
    };
*/
namespace tools
{

/** private connection helper, do not use directly */
template <class reference_type>
struct WeakConnection
{
    sal_Int32   mnRefCount;
    reference_type* mpReference;

    WeakConnection( reference_type* pReference ) : mnRefCount( 0 ), mpReference( pReference ) {};
    void acquire() { mnRefCount++; }
    void release() { mnRefCount--; if( mnRefCount == 0 ) delete this; }
};

/** template implementation to hold a weak reference to an instance of type reference_type */
template <class reference_type>
class WeakReference
{
public:
    /** constructs an empty reference */
    inline WeakReference();

    /** constructs a reference with a pointer to a class derived from WeakBase */
    inline WeakReference( reference_type* pReference );

    /** constructs a reference with another reference */
    inline WeakReference( const WeakReference< reference_type >& rWeakRef );

    inline ~WeakReference();

    /** returns true if the reference object is not null and still alive */
    inline bool is() const;

    /** returns the pointer to the reference object or null */
    inline reference_type * get() const;

    /** sets this reference to the given object or null */
    inline void reset( reference_type* pReference );

    /** returns the pointer to the reference object or null */
    inline reference_type * operator->() const;

    /** returns true if this instance references pReferenceObject */
    inline sal_Bool operator== (const reference_type * pReferenceObject) const;

    /** returns true if this instance and the given weakref reference the same object */
    inline sal_Bool operator== (const WeakReference<reference_type> & handle) const;

    /** only needed for using this class with stl containers */
    inline sal_Bool operator!= (const WeakReference<reference_type> & handle) const;

    /** only needed for using this class with stl containers */
    inline sal_Bool operator< (const WeakReference<reference_type> & handle) const;

    /** only needed for using this class with stl containers */
    inline sal_Bool operator> (const WeakReference<reference_type> & handle) const;

    /** the assignment operator */
    inline WeakReference<reference_type>& operator= (const WeakReference<reference_type> & handle);

private:
    WeakConnection< reference_type >* mpWeakConnection;
};

/** derive your implementation classes from this class if you want them to support weak references */
template <class reference_type>
class WeakBase
{
    friend class WeakReference<reference_type>;

public:
    inline WeakBase();

    inline ~WeakBase();
    /** clears the reference pointer in all living weak references for this instance.
        Further created weak references will also be invalid.
        You should call this method in the d'tor of your derived classes for an early
        invalidate of all living weak references while youre object is already inside
        it d'tor.
    */
    inline void clearWeak();

private:
    inline WeakConnection< reference_type >* getWeakConnection();
    WeakConnection< reference_type >* mpWeakConnection;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
