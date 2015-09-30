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
    static std::shared_ptr<ResMgr> GetResMgr(Sequence<Any> const& rArgs)
    {
        if(rArgs.getLength()!=1)
            return std::shared_ptr<ResMgr>();
        OUString sFilename;
        rArgs[0] >>= sFilename;
        SolarMutexGuard aGuard;
        const OString sEncName(OUStringToOString(sFilename, osl_getThreadTextEncoding()));
        return std::shared_ptr<ResMgr>(ResMgr::CreateResMgr(sEncName.getStr()));
    }

    class ResourceIndexAccessBase : public cppu::WeakImplHelper< css::container::XIndexAccess>
    {
        public:
            ResourceIndexAccessBase( std::shared_ptr<ResMgr> pResMgr)
                : m_pResMgr(pResMgr)
            {
                OSL_ENSURE(m_pResMgr, "no resource manager given");
            }

            // XIndexAccess
            virtual ::sal_Int32 SAL_CALL getCount(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE
                { return m_pResMgr.get() ? SAL_MAX_UINT16 : 0; };
            // XElementAccess
            virtual sal_Bool SAL_CALL hasElements(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE
                { return static_cast<bool>(m_pResMgr.get()); };

        protected:
            // m_pResMgr should never be NULL
            const std::shared_ptr<ResMgr> m_pResMgr;
    };

    class ResourceStringIndexAccess : public ResourceIndexAccessBase
    {
        public:
            ResourceStringIndexAccess( std::shared_ptr<ResMgr> pResMgr)
                : ResourceIndexAccessBase(pResMgr) {}
            // XIndexAccess
            virtual css::uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) throw (css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
            // XElementAccessBase
            virtual css::uno::Type SAL_CALL getElementType(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE
                { return ::cppu::UnoType<OUString>::get(); };
    };

    class ResourceStringListIndexAccess : public ResourceIndexAccessBase
    {
        public:
            ResourceStringListIndexAccess( std::shared_ptr<ResMgr> pResMgr)
                : ResourceIndexAccessBase(pResMgr) {}
            // XIndexAccess
            virtual css::uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) throw (css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
            // XElementAccessBase
            virtual css::uno::Type SAL_CALL getElementType(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE
                { return cppu::UnoType<Sequence<PropertyValue>>::get(); };
    };
}

ResourceIndexAccess::ResourceIndexAccess(Sequence<Any> const& rArgs, Reference<XComponentContext> const&)
    : m_pResMgr(GetResMgr(rArgs))
{};

Any SAL_CALL ResourceIndexAccess::getByName(const OUString& aName)
    throw (NoSuchElementException, WrappedTargetException, RuntimeException, std::exception)
{
    const Sequence<OUString> aNames(getElementNames());
    Reference<XIndexAccess> xResult;
    switch(::std::find(aNames.begin(), aNames.end(), aName) - aNames.begin())
    {
        case 0:
            xResult = Reference<XIndexAccess>(new ResourceStringIndexAccess(m_pResMgr));
            break;
        case 1:
            xResult = Reference<XIndexAccess>(new ResourceStringListIndexAccess(m_pResMgr));
            break;
        default:
            throw NoSuchElementException();
    }
    return makeAny(xResult);
}

Sequence<OUString> SAL_CALL ResourceIndexAccess::getElementNames(  )
    throw (RuntimeException, std::exception)
{
    static Sequence<OUString> aResult;
    if( aResult.getLength() == 0)
    {
        aResult.realloc(2);
        aResult[0] = "String";
        aResult[1] = "StringList";
    }
    return aResult;
}

sal_Bool SAL_CALL ResourceIndexAccess::hasByName(const OUString& aName)
    throw (RuntimeException, std::exception)
{
    const Sequence<OUString> aNames(getElementNames());
    return (::std::find(aNames.begin(), aNames.end(), aName) != aNames.end());
}

Any SAL_CALL ResourceStringIndexAccess::getByIndex(sal_Int32 nIdx)
    throw (IndexOutOfBoundsException, WrappedTargetException, RuntimeException, std::exception)
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

Any SAL_CALL ResourceStringListIndexAccess::getByIndex(sal_Int32 nIdx)
    throw (IndexOutOfBoundsException, WrappedTargetException, RuntimeException, std::exception)
{
    if(nIdx > SAL_MAX_UINT16 || nIdx < 0)
        throw IndexOutOfBoundsException();
    SolarMutexGuard aGuard;

    if(!m_pResMgr.get())
        throw RuntimeException("resource manager not available");

    const ResId aId(static_cast<sal_uInt16>(nIdx), *m_pResMgr);
    aId.SetRT(RSC_STRINGARRAY);
    if(!m_pResMgr->IsAvailable(aId))
        throw RuntimeException("string list resource for id not available");
    const ResStringArray aStringList(aId);
    Sequence<PropertyValue> aPropList(aStringList.Count());
    for(sal_Int32 nCount = 0; nCount != aPropList.getLength(); ++nCount)
    {
        aPropList[nCount].Name = aStringList.GetString(nCount);
        aPropList[nCount].Handle = -1;
        aPropList[nCount].Value <<= aStringList.GetValue(nCount);
        aPropList[nCount].State = PropertyState_DIRECT_VALUE;
    }
    return makeAny(aPropList);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
