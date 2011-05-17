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
#ifndef _SWITERATOR_HXX
#define _SWITERATOR_HXX

#include <calbck.hxx>
#include <tools/debug.hxx>

template< class TElementType, class TSource > class SwIterator
{
    SwClientIter aClientIter;
public:

    SwIterator( const TSource& rSrc ) : aClientIter(rSrc) { DBG_ASSERT( TElementType::IsOf( TYPE(SwClient) ), "Incompatible types!" ); }
    TElementType* First()     { SwClient* p = aClientIter.First(TYPE(TElementType)); return PTR_CAST(TElementType,p); }
    TElementType* Last()      { SwClient* p = aClientIter.Last( TYPE(TElementType)); return PTR_CAST(TElementType,p); }
    TElementType* Next()      { SwClient* p = aClientIter.Next();     return PTR_CAST(TElementType,p); }
    TElementType* Previous()  { SwClient* p = aClientIter.Previous(); return PTR_CAST(TElementType,p);  }
    static TElementType* FirstElement( const TSource& rMod ) { SwClient* p = SwClientIter(rMod).First(TYPE(TElementType)); return PTR_CAST(TElementType,p); }
    bool IsChanged()          { return aClientIter.IsChanged(); }
};

#endif
