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
#ifndef _SFXMINSTACK_HXX
#define _SFXMINSTACK_HXX

#include <sfx2/minarray.hxx>

#define DECL_PTRSTACK( ARR, T, nI, nG ) \
DECL_PTRARRAY( ARR##arr_, T, nI, nG ) \
class ARR: private ARR##arr_ \
{ \
public: \
    ARR( sal_uInt8 nInitSize = nI, sal_uInt8 nGrowSize = nG ): \
        ARR##arr_( nInitSize, nGrowSize ) \
    {} \
\
    ARR( const ARR& rOrig ): \
        ARR##arr_( rOrig ) \
    {} \
\
    sal_uInt16      Count() const { return ARR##arr_::Count(); } \
    void        Push( T rElem ) { Append( rElem ); } \
    T           Top( sal_uInt16 nLevel = 0 ) const \
                { return (*this)[Count()-nLevel-1]; } \
    T           Bottom() const { return (*this)[0]; } \
    T           Pop() \
                {   T aRet = (*this)[Count()-1]; \
                    Remove( Count()-1, 1 ); \
                    return aRet; \
                } \
    T*       operator*() \
                { return &(*this)[Count()-1]; } \
    void        Clear() { ARR##arr_::Clear(); } \
    sal_Bool        Contains( const T pItem ) const \
                { return ARR##arr_::Contains( pItem ); } \
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
