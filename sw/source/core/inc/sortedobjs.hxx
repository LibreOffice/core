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
#ifndef _SORTEDOBJS_HXX
#define _SORTEDOBJS_HXX
class SwSortedObjsImpl;
class SwAnchoredObject;

#include <sal/types.h>

/** class for collecting anchored objects

    for #i28701#
    Anchored objects can be inserted and deleted. The entries can be directly
    accessed via index.
    An anchored object is inserted sorted. The sort criteria are:
    - anchor type
      - order 1: to-page, 2: to-fly, 3: to-paragraph|to-character|as-character
    - anchor node
    - wrapping style (inclusive layer)
      - order 1: wrapping style != SURROUND_THROUGHT and not in hell layer,
        2: wrapping style = SURROUND_THROUGHT or in hell layer
    - wrapping style influence
      - order 1: NONE_SUCCESSIVE_POSITIONED, 2: NONE_CONCURRENT_POSITIONED
    - again anchor type
      - order 1: to-paragraph, 2: to-character, 3: as-character
    - anchor node position
    - internal anchor order number
    If one of the sort criteria attributes of an anchored object changes,
    the sorting has to be updated - use method <Update(..)>
*/
class SwSortedObjs
{
    private:
        SwSortedObjsImpl* mpImpl;

    public:
        SwSortedObjs();
        ~SwSortedObjs();

        sal_uInt32 Count() const;

        /** direct access to the entries

            @param _nIndex
            input parameter - index of entry, valid value range [0..Count()-1]
        */
        SwAnchoredObject* operator[]( sal_uInt32 _nIndex ) const;

        bool Insert( SwAnchoredObject& _rAnchoredObj );

        bool Remove( SwAnchoredObject& _rAnchoredObj );

        bool Contains( const SwAnchoredObject& _rAnchoredObj ) const;

        /** method to update the position of the given anchored object in the
            sorted list

            @return boolean, indicating success of the update.
        */
        bool Update( SwAnchoredObject& _rAnchoredObj );

        /** Position of object <_rAnchoredObj> in sorted list

            Returns the number of the list position of object <_rAnchoredObj>.
            Returns <Count()>, if object isn't contained in list.

            @return sal_uInt32
            Number of the list position of object <_rAnchoredObj>
        */
        sal_uInt32 ListPosOf( const SwAnchoredObject& _rAnchoredObj ) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
