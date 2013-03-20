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
#pragma once
#if 1

#include <sal/types.h>
#include <vector>

class SwAnchoredObject;

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
        std::vector< SwAnchoredObject* > maSortedObjLst;

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
