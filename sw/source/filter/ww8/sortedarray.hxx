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

#ifndef WW_SORTEDARRAY_HXX
#define WW_SORTEDARRAY_HXX

#include <algorithm>

//simple template that manages a static [] array by sorting at construction

namespace ww
{
    /** simple template that manages a static array

        The template sorts the array at construction in place.

        @author
        <a href="mailto:cmc@openoffice.org">Caol&aacute;n McNamara</a>
    */
    template<class C> class SortedArray
    {
    private:
        //The array e.g. of sprms.
        C *mpWwSprmTab;
        size_t mnNoElems;

        //No copying
        SortedArray(const SortedArray&);
        SortedArray& operator=(const SortedArray&);
    public:
        //Find an entry, return its address if found and 0 if not
        const C *search(C aSrch) const
        {
            std::pair<C *, C *> aPair =
                std::equal_range(mpWwSprmTab, mpWwSprmTab + mnNoElems, aSrch);
            if (aPair.first != aPair.second)
                return aPair.first;
            else
                return 0;
        }

        SortedArray(C *pWwSprmTab, size_t nNoElems)
            : mpWwSprmTab(pWwSprmTab), mnNoElems(nNoElems)
        {
            OSL_ENSURE(mnNoElems && pWwSprmTab, "WW8: empty Array: Don't do that");
            std::sort(mpWwSprmTab, mpWwSprmTab + mnNoElems);
#if OSL_DEBUG_LEVEL > 1
            bool bBroken=false;
            OUString sError;
            const C *pIter = mpWwSprmTab;
            const C *pBeforeEnd = mpWwSprmTab + mnNoElems - 1;
            while (pIter < pBeforeEnd)
            {
                if (*pIter == *(pIter+1))
                {
                    if (!bBroken)
                    {
                        sError =
                            "WW8: Duplicate in list, almost certainly don't "
                            "want that!\n"
                            "(You will not see this message again unless you "
                            "restart)\n"
                            "Extra entries are...\n";
                        bBroken=true;
                    }

                    size_t nSize = sizeof(C);
                    const sal_uInt8 *pHack =
                        reinterpret_cast<const sal_uInt8 *>(&(*pIter));
                    for (size_t i=0; i < nSize; ++i)
                    {
                        sError += OUString::number(
                            static_cast<sal_Int32>(pHack[i]), 16);
                        sError += OUString(' ');
                    }
                    sError += OUString('\n');
                    while (*pIter == *(pIter+1) && pIter < pBeforeEnd)
                        ++pIter;
                }
                else
                    ++pIter;
            }
            if (bBroken)
            {
               OSL_FAIL( OUStringToOString( sError, RTL_TEXTENCODING_ASCII_US ).getStr() );
            }
#endif
        }
    };
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
