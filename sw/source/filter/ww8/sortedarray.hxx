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



/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */

#ifndef WW_SORTEDARRAY_HXX
#define WW_SORTEDARRAY_HXX

#include <algorithm>
#include <errhdl.hxx>       // ASSERT()
#include <tools/debug.hxx>

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
            ASSERT(mnNoElems && pWwSprmTab, "WW8: empty Array: Don't do that");
            std::sort(mpWwSprmTab, mpWwSprmTab + mnNoElems);
#if OSL_DEBUG_LEVEL > 1
            bool bBroken=false;
            rtl::OUString sError;
            const C *pIter = mpWwSprmTab;
            const C *pBeforeEnd = mpWwSprmTab + mnNoElems - 1;
            while (pIter < pBeforeEnd)
            {
                if (*pIter == *(pIter+1))
                {
                    if (!bBroken)
                    {
                        sError = rtl::OUString::createFromAscii(
                            "WW8: Duplicate in list, almost certainly don't "
                            "want that!\n"
                            "(You will not see this message again unless you "
                            "restart)\n"
                            "Extra entries are...\n");
                        bBroken=true;
                    }

                    size_t nSize = sizeof(C);
                    const sal_uInt8 *pHack =
                        reinterpret_cast<const sal_uInt8 *>(&(*pIter));
                    for (size_t i=0; i < nSize; ++i)
                    {
                        sError += rtl::OUString::valueOf(
                            static_cast<sal_Int32>(pHack[i]), 16);
                        sError += rtl::OUString::valueOf(sal_Unicode(' '));
                    }
                    sError += rtl::OUString::valueOf(sal_Unicode('\n'));
                    while (*pIter == *(pIter+1) && pIter < pBeforeEnd)
                        ++pIter;
                }
                else
                    ++pIter;
            }
            if (bBroken)
            {
               DBG_ERROR(rtl::OUStringToOString(sError, RTL_TEXTENCODING_ASCII_US));
            }
#endif
        }
    };
}
#endif

/* vi:set tabstop=4 shiftwidth=4 expandtab: */
