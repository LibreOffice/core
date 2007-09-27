/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hash_wrap.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 09:59:19 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */

//this is a shameless rip from sortedarray.hxx but changed to hash_set

#ifndef WW_HASH_WRAP_HXX
#define WW_HASH_WRAP_HXX

#include <hash_set>

#ifndef _TOOLS_DEBUG_HXX    // DBG_ERROR
#include <tools/debug.hxx>
#endif

#ifndef _ERRHDL_HXX
#include <errhdl.hxx>       // ASSERT()
#endif

//simple wrapper around hash_set to behave like sorted array
namespace ww
{
    /** simple template that manages a hash


        @author
        <a href="mailto:mikeleib@openoffice.org">Michael Leibowitz</a>
    */
    template<class C, class HashFcn = std::hash<C> > class WrappedHash
    {
    private:
        std::hash_set<C, HashFcn> mHashSet;

        //No copying
        WrappedHash(const WrappedHash&);
        WrappedHash& operator=(const WrappedHash&);
    public:
        //Find an entry, return its address if found and 0 if not
        const C* search(C aSrch) const
        {
            typename std::hash_set<C, HashFcn>::const_iterator it;
            it= mHashSet.find(aSrch);
            if (it != mHashSet.end())
                return &(*it);
            else
                return 0;
        }

        WrappedHash(const C *pWwSprmTab, const size_t nNoElems)
        {
            ASSERT(nNoElems && pWwSprmTab, "WW8: empty Array: Don't do that");
            const C *pIter = pWwSprmTab;
            const C *pEnd  = pWwSprmTab + nNoElems;
            while (pIter < pEnd)
            {
                mHashSet.insert(*pIter);
                pIter++;
            }
#if OSL_DEBUG_LEVEL > 1
            bool bBroken=false;
            rtl::OUString sError;
            pIter = pWwSprmTab;
            const C *pBeforeEnd = pWwSprmTab + nNoElems - 1;
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
