/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/compbase.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>

namespace comphelper
{
WeakComponentImplHelperBase::~WeakComponentImplHelperBase() {}

// css::lang::XComponent
void SAL_CALL WeakComponentImplHelperBase::dispose()
{
    std::unique_lock aGuard(m_aMutex);
    if (m_bDisposed)
        return;
    m_bDisposed = true;
    disposing(aGuard);
    if (!aGuard.owns_lock())
        aGuard.lock();
    css::lang::EventObject aEvt(static_cast<OWeakObject*>(this));
    maEventListeners.disposeAndClear(aGuard, aEvt);
}

void WeakComponentImplHelperBase::disposing(std::unique_lock<std::mutex>&) {}

void SAL_CALL WeakComponentImplHelperBase::addEventListener(
    css::uno::Reference<css::lang::XEventListener> const& rxListener)
{
    std::unique_lock aGuard(m_aMutex);
    if (m_bDisposed)
        return;
    maEventListeners.addInterface(aGuard, rxListener);
}

void SAL_CALL WeakComponentImplHelperBase::removeEventListener(
    css::uno::Reference<css::lang::XEventListener> const& rxListener)
{
    std::unique_lock aGuard(m_aMutex);
    maEventListeners.removeInterface(aGuard, rxListener);
}

css::uno::Any SAL_CALL WeakComponentImplHelperBase::queryInterface(css::uno::Type const& rType)
{
    css::uno::Any aReturn = ::cppu::queryInterface(rType, static_cast<css::uno::XWeak*>(this),
                                                   static_cast<css::lang::XComponent*>(this));
    if (aReturn.hasValue())
        return aReturn;
    return OWeakObject::queryInterface(rType);
}

static void checkInterface(css::uno::Type const& rType)
{
    if (css::uno::TypeClass_INTERFACE != rType.getTypeClass())
    {
        OUString msg("querying for interface \"" + rType.getTypeName() + "\": no interface type!");
        SAL_WARN("cppuhelper", msg);
        throw css::uno::RuntimeException(msg);
    }
}

static bool isXInterface(rtl_uString* pStr)
{
    return OUString::unacquired(&pStr) == "com.sun.star.uno.XInterface";
}

static bool td_equals(typelib_TypeDescriptionReference const* pTDR1,
                      typelib_TypeDescriptionReference const* pTDR2)
{
    return ((pTDR1 == pTDR2)
            || OUString::unacquired(&pTDR1->pTypeName) == OUString::unacquired(&pTDR2->pTypeName));
}

static cppu::type_entry* getTypeEntries(cppu::class_data* cd)
{
    cppu::type_entry* pEntries = cd->m_typeEntries;
    if (!cd->m_storedTypeRefs) // not inited?
    {
        static std::mutex aMutex;
        std::scoped_lock guard(aMutex);
        if (!cd->m_storedTypeRefs) // not inited?
        {
            // get all types
            for (sal_Int32 n = cd->m_nTypes; n--;)
            {
                cppu::type_entry* pEntry = &pEntries[n];
                css::uno::Type const& rType = (*pEntry->m_type.getCppuType)(nullptr);
                OSL_ENSURE(rType.getTypeClass() == css::uno::TypeClass_INTERFACE,
                           "### wrong helper init: expected interface!");
                OSL_ENSURE(
                    !isXInterface(rType.getTypeLibType()->pTypeName),
                    "### want to implement XInterface: template argument is XInterface?!?!?!");
                if (rType.getTypeClass() != css::uno::TypeClass_INTERFACE)
                {
                    OUString msg("type \"" + rType.getTypeName() + "\" is no interface type!");
                    SAL_WARN("cppuhelper", msg);
                    throw css::uno::RuntimeException(msg);
                }
                // ref is statically held by getCppuType()
                pEntry->m_type.typeRef = rType.getTypeLibType();
            }
            OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
            cd->m_storedTypeRefs = true;
        }
    }
    else
    {
        OSL_DOUBLE_CHECKED_LOCKING_MEMORY_BARRIER();
    }
    return pEntries;
}

static void* makeInterface(sal_IntPtr nOffset, void* that)
{
    return (static_cast<char*>(that) + nOffset);
}

static bool recursivelyFindType(typelib_TypeDescriptionReference const* demandedType,
                                typelib_InterfaceTypeDescription const* type, sal_IntPtr* offset)
{
    // This code assumes that the vtables of a multiple-inheritance class (the
    // offset amount by which to adjust the this pointer) follow one another in
    // the object layout, and that they contain slots for the inherited classes
    // in a specific order.  In theory, that need not hold for any given
    // platform; in practice, it seems to work well on all supported platforms:
next:
    for (sal_Int32 i = 0; i < type->nBaseTypes; ++i)
    {
        if (i > 0)
        {
            *offset += sizeof(void*);
        }
        typelib_InterfaceTypeDescription const* base = type->ppBaseTypes[i];
        // ignore XInterface:
        if (base->nBaseTypes > 0)
        {
            if (td_equals(reinterpret_cast<typelib_TypeDescriptionReference const*>(base),
                          demandedType))
            {
                return true;
            }
            // Profiling showed that it is important to speed up the common case
            // of only one base:
            if (type->nBaseTypes == 1)
            {
                type = base;
                goto next;
            }
            if (recursivelyFindType(demandedType, base, offset))
            {
                return true;
            }
        }
    }
    return false;
}

static void* queryDeepNoXInterface(typelib_TypeDescriptionReference const* pDemandedTDR,
                                   cppu::class_data* cd, void* that)
{
    cppu::type_entry* pEntries = getTypeEntries(cd);
    sal_Int32 nTypes = cd->m_nTypes;
    sal_Int32 n;

    // try top interfaces without getting td
    for (n = 0; n < nTypes; ++n)
    {
        if (td_equals(pEntries[n].m_type.typeRef, pDemandedTDR))
        {
            return makeInterface(pEntries[n].m_offset, that);
        }
    }
    // query deep getting td
    for (n = 0; n < nTypes; ++n)
    {
        typelib_TypeDescription* pTD = nullptr;
        TYPELIB_DANGER_GET(&pTD, pEntries[n].m_type.typeRef);
        if (pTD)
        {
            // exclude top (already tested) and bottom (XInterface) interface
            OSL_ENSURE(reinterpret_cast<typelib_InterfaceTypeDescription*>(pTD)->nBaseTypes > 0,
                       "### want to implement XInterface:"
                       " template argument is XInterface?!?!?!");
            sal_IntPtr offset = pEntries[n].m_offset;
            bool found = recursivelyFindType(
                pDemandedTDR, reinterpret_cast<typelib_InterfaceTypeDescription*>(pTD), &offset);
            TYPELIB_DANGER_RELEASE(pTD);
            if (found)
            {
                return makeInterface(offset, that);
            }
        }
        else
        {
            OUString msg("cannot get type description for type \""
                         + OUString::unacquired(&pEntries[n].m_type.typeRef->pTypeName) + "\"!");
            SAL_WARN("cppuhelper", msg);
            throw css::uno::RuntimeException(msg);
        }
    }
    return nullptr;
}

css::uno::Any WeakComponentImplHelper_query(css::uno::Type const& rType, cppu::class_data* cd,
                                            WeakComponentImplHelperBase* pBase)
{
    checkInterface(rType);
    typelib_TypeDescriptionReference* pTDR = rType.getTypeLibType();

    // shortcut XInterface to WeakComponentImplHelperBase
    if (!isXInterface(pTDR->pTypeName))
    {
        void* p = queryDeepNoXInterface(pTDR, cd, pBase);
        if (p)
        {
            return css::uno::Any(&p, pTDR);
        }
    }
    return pBase->comphelper::WeakComponentImplHelperBase::queryInterface(rType);
}

WeakImplHelperBase::~WeakImplHelperBase() {}

css::uno::Any SAL_CALL WeakImplHelperBase::queryInterface(css::uno::Type const& rType)
{
    css::uno::Any aReturn = ::cppu::queryInterface(rType, static_cast<css::uno::XWeak*>(this));
    if (aReturn.hasValue())
        return aReturn;
    return OWeakObject::queryInterface(rType);
}

css::uno::Any WeakImplHelper_query(css::uno::Type const& rType, cppu::class_data* cd,
                                   WeakImplHelperBase* pBase)
{
    checkInterface(rType);
    typelib_TypeDescriptionReference* pTDR = rType.getTypeLibType();

    // shortcut XInterface to WeakComponentImplHelperBase
    if (!isXInterface(pTDR->pTypeName))
    {
        void* p = queryDeepNoXInterface(pTDR, cd, pBase);
        if (p)
        {
            return css::uno::Any(&p, pTDR);
        }
    }
    return pBase->comphelper::WeakImplHelperBase::queryInterface(rType);
}

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
