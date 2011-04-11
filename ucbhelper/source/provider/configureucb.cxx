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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucbhelper.hxx"
#include <ucbhelper/configureucb.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ucb/XContentProvider.hpp>
#include <com/sun/star/ucb/XContentProviderManager.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <rtl/ustrbuf.hxx>

#include "osl/diagnose.h"

#include <provconf.hxx>
#include <registerucb.hxx>

using namespace com::sun::star;

namespace {

bool fillPlaceholders(rtl::OUString const & rInput,
                      uno::Sequence< uno::Any > const & rReplacements,
                      rtl::OUString * pOutput)
{
    sal_Unicode const * p = rInput.getStr();
    sal_Unicode const * pEnd = p + rInput.getLength();
    sal_Unicode const * pCopy = p;
    rtl::OUStringBuffer aBuffer;
    while (p != pEnd)
        switch (*p++)
        {
            case '&':
                if (pEnd - p >= 4
                    && p[0] == 'a' && p[1] == 'm' && p[2] == 'p'
                    && p[3] == ';')
                {
                    aBuffer.append(pCopy, p - 1 - pCopy);
                    aBuffer.append(sal_Unicode('&'));
                    p += 4;
                    pCopy = p;
                }
                else if (pEnd - p >= 3
                         && p[0] == 'l' && p[1] == 't' && p[2] == ';')
                {
                    aBuffer.append(pCopy, p - 1 - pCopy);
                    aBuffer.append(sal_Unicode('<'));
                    p += 3;
                    pCopy = p;
                }
                else if (pEnd - p >= 3
                         && p[0] == 'g' && p[1] == 't' && p[2] == ';')
                {
                    aBuffer.append(pCopy, p - 1 - pCopy);
                    aBuffer.append(sal_Unicode('>'));
                    p += 3;
                    pCopy = p;
                }
                break;

            case '<':
                sal_Unicode const * q = p;
                while (q != pEnd && *q != '>')
                    ++q;
                if (q == pEnd)
                    break;
                rtl::OUString aKey(p, q - p);
                rtl::OUString aValue;
                bool bFound = false;
                for (sal_Int32 i = 2; i + 1 < rReplacements.getLength();
                     i += 2)
                {
                    rtl::OUString aReplaceKey;
                    if ((rReplacements[i] >>= aReplaceKey)
                        && aReplaceKey == aKey
                        && (rReplacements[i + 1] >>= aValue))
                    {
                        bFound = true;
                        break;
                    }
                }
                if (!bFound)
                    return false;
                aBuffer.append(pCopy, p - 1 - pCopy);
                aBuffer.append(aValue);
                p = q + 1;
                pCopy = p;
                break;
        }
    aBuffer.append(pCopy, pEnd - pCopy);
    *pOutput = aBuffer.makeStringAndClear();
    return true;
}

}

namespace ucbhelper {

//============================================================================
//
//  configureUcb
//
//============================================================================

bool
configureUcb(
    uno::Reference< ucb::XContentProviderManager > const & rManager,
    uno::Reference< lang::XMultiServiceFactory > const & rServiceFactory,
    ContentProviderDataList const & rData,
    ContentProviderRegistrationInfoList * pInfos)
    throw (uno::RuntimeException)
{
    ContentProviderDataList::const_iterator aEnd(rData.end());
    for (ContentProviderDataList::const_iterator aIt(rData.begin());
         aIt != aEnd; ++aIt)
    {
        ContentProviderRegistrationInfo aInfo;
        bool bSuccess = registerAtUcb(rManager,
                                      rServiceFactory,
                                      aIt->ServiceName,
                                      aIt->Arguments,
                                      aIt->URLTemplate,
                                      &aInfo);

        if (bSuccess && pInfos)
            pInfos->push_back(aInfo);
    }

    return true;
}

//============================================================================
//
//  configureUcb
//
//============================================================================

bool
configureUcb(
    uno::Reference< ucb::XContentProviderManager > const & rManager,
    uno::Reference< lang::XMultiServiceFactory > const & rServiceFactory,
    uno::Sequence< uno::Any > const & rArguments,
    std::vector< ContentProviderRegistrationInfo > * pInfos)
    throw (uno::RuntimeException)
{
    rtl::OUString aKey1;
    rtl::OUString aKey2;
    if (rArguments.getLength() < 2
        || !(rArguments[0] >>= aKey1) || !(rArguments[1] >>= aKey2))
    {
        OSL_FAIL("ucb::configureUcb(): Bad arguments");
        return false;
    }

    ContentProviderDataList aData;
    if (!getContentProviderData(rServiceFactory, aKey1, aKey2, aData))
    {
        OSL_FAIL("ucb::configureUcb(): No configuration");
        return false;
    }

    ContentProviderDataList::const_iterator aEnd(aData.end());
    for (ContentProviderDataList::const_iterator aIt(aData.begin());
         aIt != aEnd; ++aIt)
    {
        rtl::OUString aProviderArguments;
        if (fillPlaceholders(aIt->Arguments,
                             rArguments,
                             &aProviderArguments))
        {
            ContentProviderRegistrationInfo aInfo;
            bool bSuccess = registerAtUcb(rManager,
                                          rServiceFactory,
                                          aIt->ServiceName,
                                          aProviderArguments,
                                          aIt->URLTemplate,
                                          &aInfo);
            OSL_ENSURE(bSuccess, "ucb::configureUcb(): Bad content provider");

            if (bSuccess && pInfos)
                pInfos->push_back(aInfo);
        }
        else
            OSL_FAIL("ucb::configureUcb(): Bad argument placeholders");
    }

    return true;
}

}

//============================================================================
//
//  unconfigureUcb
//
//============================================================================

namespace ucbhelper {

void
unconfigureUcb(
    uno::Reference< ucb::XContentProviderManager > const & rManager,
    std::vector< ContentProviderRegistrationInfo > const & rInfos)
    throw (uno::RuntimeException)
{
    std::vector< ContentProviderRegistrationInfo >::const_iterator
        aEnd(rInfos.end());
    for (std::vector< ContentProviderRegistrationInfo >::const_iterator
             aIt(rInfos.begin());
         aIt != aEnd; ++aIt)
        deregisterFromUcb(rManager, *aIt);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
