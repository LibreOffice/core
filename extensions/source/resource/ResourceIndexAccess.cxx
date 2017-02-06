/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <ResourceIndexAccess.hxx>

#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <cppuhelper/implbase.hxx>
#include <osl/mutex.hxx>
#include <tools/rcid.h>
#include <tools/resary.hxx>
#include <tools/resmgr.hxx>
#include <vcl/svapp.hxx>

using namespace ::extensions::resource;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;

namespace
{
    std::shared_ptr<ResMgr> GetResMgr(Sequence<Any> const& rArgs)
    {
        if(rArgs.getLength()!=1)
            return std::shared_ptr<ResMgr>();
        OUString sFilename;
        rArgs[0] >>= sFilename;
        SolarMutexGuard aGuard;
        const OString sEncName(OUStringToOString(sFilename, osl_getThreadTextEncoding()));
        return std::shared_ptr<ResMgr>(ResMgr::CreateResMgr(sEncName.getStr()));
    }

    class ResourceStringIndexAccess : public cppu::WeakImplHelper< css::container::XIndexAccess>
    {
        public:
            explicit ResourceStringIndexAccess(const std::shared_ptr<ResMgr>& pResMgr)
                : m_pResMgr(pResMgr)
            {
                OSL_ENSURE(m_pResMgr, "no resource manager given");
            }

            // XIndexAccess
            virtual ::sal_Int32 SAL_CALL getCount(  ) override
                { return m_pResMgr.get() ? SAL_MAX_UINT16 : 0; };
            virtual css::uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) override;
            // XElementAccess
            virtual sal_Bool SAL_CALL hasElements(  ) override
                { return static_cast<bool>(m_pResMgr.get()); };
            virtual css::uno::Type SAL_CALL getElementType(  ) override
                { return ::cppu::UnoType<OUString>::get(); };
        private:
            // m_pResMgr should never be NULL
            const std::shared_ptr<ResMgr> m_pResMgr;
    };
}

ResourceIndexAccess::ResourceIndexAccess(Sequence<Any> const& rArgs, Reference<XComponentContext> const&)
    : m_pResMgr(GetResMgr(rArgs))
{};

Any SAL_CALL ResourceIndexAccess::getByName(const OUString& aName)
{
    const Sequence<OUString> aNames(getElementNames());
    Reference<XIndexAccess> xResult;
    switch(::std::find(aNames.begin(), aNames.end(), aName) - aNames.begin())
    {
        case 0:
            xResult.set(new ResourceStringIndexAccess(m_pResMgr));
            break;
        default:
            throw NoSuchElementException();
    }
    return makeAny(xResult);
}

Sequence<OUString> SAL_CALL ResourceIndexAccess::getElementNames(  )
{
    static Sequence<OUString> aResult;
    if( aResult.getLength() == 0)
    {
        aResult.realloc(1);
        aResult[0] = "String";
    }
    return aResult;
}

sal_Bool SAL_CALL ResourceIndexAccess::hasByName(const OUString& aName)
{
    const Sequence<OUString> aNames(getElementNames());
    return (::std::find(aNames.begin(), aNames.end(), aName) != aNames.end());
}

Any SAL_CALL ResourceStringIndexAccess::getByIndex(sal_Int32 nIdx)
{
    if(nIdx > SAL_MAX_UINT16 || nIdx < 0)
        throw IndexOutOfBoundsException();
    SolarMutexGuard aGuard;
    if(!m_pResMgr.get())
        throw RuntimeException("resource manager not available");

    const ResId aId(static_cast<sal_uInt16>(nIdx), *m_pResMgr);
    aId.SetRT(RSC_STRING);

    if(!m_pResMgr->IsAvailable(aId))
        throw RuntimeException("string resource for id not available");

    return makeAny(aId.toString());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
