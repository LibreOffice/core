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

#ifndef INCLUDED_SW_SOURCE_FILTER_WW8_SORTEDARRAY_HXX
#define INCLUDED_SW_SOURCE_FILTER_WW8_SORTEDARRAY_HXX

#include <osl/diagnose.h>
#include <algorithm>

//simple template that manages a static [] array by sorting at construction

namespace ww
{
    /** simple template that manages a static sorted array */
    template<class C> class SortedArray
    {
    private:
        //The array e.g. of sprms.
        const C *mpWwSprmTab;
        size_t mnNoElems;

        SortedArray(const SortedArray&) = delete;
        SortedArray& operator=(const SortedArray&) = delete;
    public:
        //Find an entry, return its address if found and 0 if not
        const C *search(C aSrch) const
        {
            std::pair<const C*, const C*> aPair =
                std::equal_range(mpWwSprmTab, mpWwSprmTab + mnNoElems, aSrch);
            if (aPair.first != aPair.second)
                return aPair.first;
            else
                return nullptr;
        }

        SortedArray(const C *pWwSprmTab, size_t nNoElems)
            : mpWwSprmTab(pWwSprmTab), mnNoElems(nNoElems)
        {
            assert(mnNoElems && pWwSprmTab && "WW8: empty Array: Don't do that");
        }
    };
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
