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

//this is a shameless rip from sortedarray.hxx but changed to boost::unordered_set

#ifndef WW_HASH_WRAP_HXX
#define WW_HASH_WRAP_HXX

#include <boost/unordered_set.hpp>

//simple wrapper around boost::unordered_set to behave like sorted array
namespace ww
{
    /** simple template that manages a hash


        @author
        <a href="mailto:mikeleib@openoffice.org">Michael Leibowitz</a>
    */
    template<class C, class HashFcn = boost::hash<C> > class WrappedHash
    {
    private:
        boost::unordered_set<C, HashFcn> mHashSet;

        //No copying
        WrappedHash(const WrappedHash&);
        WrappedHash& operator=(const WrappedHash&);
    public:
        //Find an entry, return its address if found and 0 if not
        const C* search(C aSrch) const
        {
            typename boost::unordered_set<C, HashFcn>::const_iterator it;
            it= mHashSet.find(aSrch);
            if (it != mHashSet.end())
                return &(*it);
            else
                return 0;
        }

        WrappedHash(const C *pWwSprmTab, const size_t nNoElems)
        {
            OSL_ENSURE(nNoElems && pWwSprmTab, "WW8: empty Array: Don't do that");
            const C *pIter = pWwSprmTab;
            const C *pEnd  = pWwSprmTab + nNoElems;
            while (pIter < pEnd)
            {
                mHashSet.insert(*pIter);
                pIter++;
            }
#if OSL_DEBUG_LEVEL > 1
            bool bBroken=false;
            OUString sError;
            pIter = pWwSprmTab;
            const C *pBeforeEnd = pWwSprmTab + nNoElems - 1;
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
