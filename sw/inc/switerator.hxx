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
#ifndef _SWITERATOR_HXX
#define _SWITERATOR_HXX

#include <calbck.hxx>
#include <sal/log.hxx>

template< class TElementType, class TSource > class SwIterator
{
    SwClientIter aClientIter;
public:

    SwIterator( const TSource& rSrc ) : aClientIter(rSrc) { SAL_WARN_IF( !TElementType::IsOf( TYPE(SwClient) ), "sw", "Incompatible types!" ); }
    TElementType* First()     { SwClient* p = aClientIter.First(TYPE(TElementType)); return PTR_CAST(TElementType,p); }
    TElementType* Last()      { SwClient* p = aClientIter.Last( TYPE(TElementType)); return PTR_CAST(TElementType,p); }
    TElementType* Next()      { SwClient* p = aClientIter.Next();     return PTR_CAST(TElementType,p); }
    TElementType* Previous()  { SwClient* p = aClientIter.Previous(); return PTR_CAST(TElementType,p);  }
    static TElementType* FirstElement( const TSource& rMod ) { SwClient* p = SwClientIter(rMod).First(TYPE(TElementType)); return PTR_CAST(TElementType,p); }
    bool IsChanged()          { return aClientIter.IsChanged(); }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
