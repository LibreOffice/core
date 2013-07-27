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
#include <comphelper/stlunosequence.hxx>
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

using ::comphelper::stl_begin;
using ::comphelper::stl_end;

namespace
{
    static ::boost::shared_ptr<ResMgr> GetResMgr(Sequence<Any> const& rArgs)
    {
        if(rArgs.getLength()!=1)
            return ::boost::shared_ptr<ResMgr>();
        OUString sFilename;
        rArgs[0] >>= sFilename;
        SolarMutexGuard aGuard;
        const OString sEncName(OUStringToOString(sFilename, osl_getThreadTextEncoding()));
        return ::boost::shared_ptr<ResMgr>(ResMgr::CreateResMgr(sEncName.getStr()));
    }

    class ResourceIndexAccessBase : public cppu::WeakImplHelper1< ::com::sun::star::container::XIndexAccess>
    {
        public:
            ResourceIndexAccessBase( ::boost::shared_ptr<ResMgr> pResMgr)
                : m_pResMgr(pResMgr)
            {
                OSL_ENSURE(m_pResMgr, "no resource manager given");
            }

            // XIndexAccess
            virtual ::sal_Int32 SAL_CALL getCount(  ) throw (::com::sun::star::uno::RuntimeException)
                { return m_pResMgr.get() ? SAL_MAX_UINT16 : 0; };
            // XElementAccess
            virtual ::sal_Bool SAL_CALL hasElements(  ) throw (::com::sun::star::uno::RuntimeException)
                { return static_cast<bool>(m_pResMgr.get()); };

        protected:
            // m_pResMgr should never be NULL
            const ::boost::shared_ptr<ResMgr> m_pResMgr;
    };

    class ResourceStringIndexAccess : public ResourceIndexAccessBase
    {
        public:
            ResourceStringIndexAccess( ::boost::shared_ptr<ResMgr> pResMgr)
                : ResourceIndexAccessBase(pResMgr) {}
            // XIndexAccess
            virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
            // XElementAccessBase
            virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw (::com::sun::star::uno::RuntimeException)
                { return ::getCppuType(static_cast< OUString*>(0)); };
    };

    class ResourceStringListIndexAccess : public ResourceIndexAccessBase
    {
        public:
            ResourceStringListIndexAccess( ::boost::shared_ptr<ResMgr> pResMgr)
                : ResourceIndexAccessBase(pResMgr) {}
            // XIndexAccess
            virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
            // XElementAccessBase
            virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw (::com::sun::star::uno::RuntimeException)
                { return ::getCppuType(static_cast<Sequence<PropertyValue> * >(0)); };
    };
}

ResourceIndexAccess::ResourceIndexAccess(Sequence<Any> const& rArgs, Reference<XComponentContext> const&)
    : m_pResMgr(GetResMgr(rArgs))
{};

Reference<XInterface> initResourceIndexAccess(ResourceIndexAccess* pResourceIndexAccess)
{
    Reference<XInterface> xResult(static_cast<cppu::OWeakObject*>(pResourceIndexAccess));
    if(!pResourceIndexAccess->hasElements())
        // xResult does not help the client to analyse the problem
        // and will crash on getByIndex calls, better just give back an empty Reference
        // so that such ResourceStringIndexAccess instances are never release into the wild
        throw RuntimeException(
            OUString("resource manager could not get initialized"),
            /* xResult */ Reference<XInterface>());
    return xResult;
}

Any SAL_CALL ResourceIndexAccess::getByName(const OUString& aName)
    throw (NoSuchElementException, WrappedTargetException, RuntimeException)
{
    const Sequence<OUString> aNames(getElementNames());
    Reference<XIndexAccess> xResult;
    switch(::std::find(stl_begin(aNames), stl_end(aNames), aName)-stl_begin(aNames))
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
    throw (RuntimeException)
{
    static Sequence<OUString> aResult;
    if( aResult.getLength() == 0)
    {
        aResult.realloc(2);
        aResult[0] = OUString("String");
        aResult[1] = OUString("StringList");
    }
    return aResult;
}

::sal_Bool SAL_CALL ResourceIndexAccess::hasByName(const OUString& aName)
    throw (RuntimeException)
{
    const Sequence<OUString> aNames(getElementNames());
    return (::std::find(stl_begin(aNames), stl_end(aNames), aName) != stl_end(aNames));
}

Any SAL_CALL ResourceStringIndexAccess::getByIndex(sal_Int32 nIdx)
    throw (IndexOutOfBoundsException, WrappedTargetException, RuntimeException)
{
    if(nIdx > SAL_MAX_UINT16 || nIdx < 0)
        throw IndexOutOfBoundsException();
    SolarMutexGuard aGuard;
    if(!m_pResMgr.get())
        throw RuntimeException(
            OUString("resource manager not available"),
            Reference<XInterface>());

    const ResId aId(static_cast<sal_uInt16>(nIdx), *m_pResMgr);
    aId.SetRT(RSC_STRING);

    if(!m_pResMgr->IsAvailable(aId))
        throw RuntimeException(
            OUString("string resource for id not available"),
            Reference<XInterface>());

    return makeAny(aId.toString());
}

Any SAL_CALL ResourceStringListIndexAccess::getByIndex(sal_Int32 nIdx)
    throw (IndexOutOfBoundsException, WrappedTargetException, RuntimeException)
{
    if(nIdx > SAL_MAX_UINT16 || nIdx < 0)
        throw IndexOutOfBoundsException();
    SolarMutexGuard aGuard;

    if(!m_pResMgr.get())
        throw RuntimeException(
            OUString("resource manager not available"),
            Reference<XInterface>());

    const ResId aId(static_cast<sal_uInt16>(nIdx), *m_pResMgr);
    aId.SetRT(RSC_STRINGARRAY);
    if(!m_pResMgr->IsAvailable(aId))
        throw RuntimeException(
            OUString("string list resource for id not available"),
            Reference<XInterface>());
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
