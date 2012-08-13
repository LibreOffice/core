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
using ::rtl::OString;
using ::rtl::OUString;
using ::rtl::OUStringToOString;

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
                { return ::getCppuType(static_cast< ::rtl::OUString*>(0)); };
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
            OUString(RTL_CONSTASCII_USTRINGPARAM("resource manager could not get initialized")),
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
        aResult[0] = OUString(RTL_CONSTASCII_USTRINGPARAM("String"));
        aResult[1] = OUString(RTL_CONSTASCII_USTRINGPARAM("StringList"));
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
            OUString(RTL_CONSTASCII_USTRINGPARAM("resource manager not available")),
            Reference<XInterface>());

    const ResId aId(static_cast<sal_uInt16>(nIdx), *m_pResMgr);
    aId.SetRT(RSC_STRING);

    if(!m_pResMgr->IsAvailable(aId))
        throw RuntimeException(
            OUString(RTL_CONSTASCII_USTRINGPARAM("string resource for id not available")),
            Reference<XInterface>());

    return makeAny(OUString(String(aId)));
}

Any SAL_CALL ResourceStringListIndexAccess::getByIndex(sal_Int32 nIdx)
    throw (IndexOutOfBoundsException, WrappedTargetException, RuntimeException)
{
    if(nIdx > SAL_MAX_UINT16 || nIdx < 0)
        throw IndexOutOfBoundsException();
    SolarMutexGuard aGuard;

    if(!m_pResMgr.get())
        throw RuntimeException(
            OUString(RTL_CONSTASCII_USTRINGPARAM("resource manager not available")),
            Reference<XInterface>());

    const ResId aId(static_cast<sal_uInt16>(nIdx), *m_pResMgr);
    aId.SetRT(RSC_STRINGARRAY);
    if(!m_pResMgr->IsAvailable(aId))
        throw RuntimeException(
            OUString(RTL_CONSTASCII_USTRINGPARAM("string list resource for id not available")),
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
