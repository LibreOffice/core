/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *      Bjoern Michaelsen <bjoern.michaelsen@canonical.com>
 * Portions created by the Initial Developer are Copyright (C) 2010 the
 * Initial Developer. All Rights Reserved.
 *
 * Major Contributor(s):
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <ResourceStringIndexAccess.hxx>

#include <osl/mutex.hxx>
#include <tools/rcid.h>
#include <tools/resary.hxx>
#include <tools/resmgr.hxx>
#include <vcl/svapp.hxx>

using namespace ::extensions::resource;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;

using ::rtl::OUString;
using ::rtl::OString;
using ::rtl::OUStringToOString;

namespace
{
    static ::std::auto_ptr<ResMgr> GetResMgr(Sequence<Any> const& rArgs)
    {
        if(rArgs.getLength()!=1)
            return ::std::auto_ptr<ResMgr>();
        OUString sFilename;
        rArgs[0] >>= sFilename;
        SolarMutexGuard aGuard;
        const OString sEncName(OUStringToOString(sFilename, osl_getThreadTextEncoding()));
        return ::std::auto_ptr<ResMgr>(ResMgr::CreateResMgr(sEncName));
    }
}


ResourceStringIndexAccess::ResourceStringIndexAccess(Sequence<Any> const& rArgs, Reference<XComponentContext> const&)
    : m_pResMgr(GetResMgr(rArgs))
{};

Reference<XInterface> initResourceStringIndexAccess(ResourceStringIndexAccess* pResourceStringIndexAccess)
{
    Reference<XInterface> xResult(static_cast<cppu::OWeakObject*>(pResourceStringIndexAccess));
    if(!pResourceStringIndexAccess->hasElements())
        // xResult does not help the client to analyse the problem
        // and will crash on getByIndex calls, better just give back an empty Reference
        // so that such ResourceStringIndexAccess instances are never release into the wild
        throw RuntimeException(
            OUString(RTL_CONSTASCII_USTRINGPARAM("ressource manager could not get initialized")),
            /* xResult */ Reference<XInterface>());
    return xResult;
}

Any SAL_CALL ResourceStringIndexAccess::getByIndex(sal_Int32 nIdx)
    throw (IndexOutOfBoundsException, WrappedTargetException, RuntimeException)
{
    if(nIdx > SAL_MAX_UINT16 || nIdx < 0)
        throw IndexOutOfBoundsException();
    SolarMutexGuard aGuard;
    const ResId aId(static_cast<sal_uInt16>(nIdx), *m_pResMgr);
    aId.SetRT(RSC_STRING);
    if(!m_pResMgr->IsAvailable(aId))
        throw RuntimeException(
            OUString(RTL_CONSTASCII_USTRINGPARAM("string ressource for id not available")),
            Reference<XInterface>());
    return makeAny(OUString(String(aId)));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
