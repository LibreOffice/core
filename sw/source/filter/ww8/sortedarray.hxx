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

#ifndef WW_SORTEDARRAY_HXX
#define WW_SORTEDARRAY_HXX

#include <algorithm>
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
            OSL_ENSURE(mnNoElems && pWwSprmTab, "WW8: empty Array: Don't do that");
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
                        sError = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
                            "WW8: Duplicate in list, almost certainly don't "
                            "want that!\n"
                            "(You will not see this message again unless you "
                            "restart)\n"
                            "Extra entries are...\n"));
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
