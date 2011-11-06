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



#ifndef FORMULA_INTRUREF_HXX
#define FORMULA_INTRUREF_HXX

namespace formula
{

/** A simple intrusive refcounting template, not thread safe, but therefore
    also a bit faster than boost's smart_ptr or uno::Reference equivalents, and
    the type to be refcounted has full control over its behavior.

    Mainly used in formula compiler and interpreter context, e.g. ScTokenRef,
    ScMatrixRef.

    Type T must implement methods IncRef() and DecRef(), in case typename T is
    const they must be const as well and the reference counter be mutable.
  */
template< typename T > class SimpleIntrusiveReference
{
    T* p;
public:
    inline SimpleIntrusiveReference() : p(0) {}
    inline SimpleIntrusiveReference( const SimpleIntrusiveReference& r )
    {
        p = r.p;
        if ( p )
            p->IncRef();
    }
    inline SimpleIntrusiveReference( T *t )
    {
        p = t;
        if ( p )
            t->IncRef();
    }
    inline void Clear()
    {
        if ( p )
        {
            p->DecRef();
            p = 0;
        }
    }
    inline ~SimpleIntrusiveReference()
    {
        if ( p )
            p->DecRef();
    }
    inline SimpleIntrusiveReference& operator=( T* t )
    {
        if ( t )
            t->IncRef();
        if ( p )
            p->DecRef();
        p = t;
        return *this;
    }
    inline SimpleIntrusiveReference& operator=( const SimpleIntrusiveReference& r )
    {
        *this = r.p;
        return *this;
    }
    inline bool Is() const              { return p != 0; }
    inline bool operator ! () const     { return p == 0; }
    inline T* operator&() const         { return p; }
    inline T* operator->() const        { return p; }
    inline T& operator*() const         { return *p; }
    inline operator T*() const          { return p; }
    inline T* get() const               { return p; }
};
// =============================================================================
} // formula
// =============================================================================

#endif // SC_INTRUREF_HXX

