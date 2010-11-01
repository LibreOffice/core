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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
