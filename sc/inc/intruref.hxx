/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: intruref.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:43:30 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SC_INTRUREF_HXX
#define SC_INTRUREF_HXX

/** A simple intrusive refcounting template, not thread safe, but therefore
    also a bit faster than boost's smart_ptr or uno::Reference equivalents, and
    the type to be refcounted has full control over its behavior.

    Mainly used in formula compiler and interpreter context, e.g. ScTokenRef,
    ScMatrixRef.

    Type T must implement methods IncRef() and DecRef(), in case typename T is
    const they must be const as well and the reference counter be mutable.
  */
template< typename T > class ScSimpleIntrusiveReference
{
    T* p;
public:
    inline ScSimpleIntrusiveReference() : p(0) {}
    inline ScSimpleIntrusiveReference( const ScSimpleIntrusiveReference& r )
    {
        p = r.p;
        if ( p )
            p->IncRef();
    }
    inline ScSimpleIntrusiveReference( T *t )
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
    inline ~ScSimpleIntrusiveReference()
    {
        if ( p )
            p->DecRef();
    }
    inline ScSimpleIntrusiveReference& operator=( T* t )
    {
        if ( t )
            t->IncRef();
        if ( p )
            p->DecRef();
        p = t;
        return *this;
    }
    inline ScSimpleIntrusiveReference& operator=( const ScSimpleIntrusiveReference& r )
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
};

#endif // SC_INTRUREF_HXX

