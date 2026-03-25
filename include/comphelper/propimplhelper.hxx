/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <algorithm>
#include <cassert>
#include <concepts>
#include <memory>
#include <utility>
#include <vector>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XFastPropertySet.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

#include <comphelper/compbase.hxx>
#include <comphelper/multiinterfacecontainer4.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/propshlp.hxx>
#include <sal/log.hxx>

namespace comphelper
{
/// BaseClass must inherit from comphelper::UnoImplBase (provides m_aMutex, m_bDisposed).
template <typename T> concept IsUnoImplBase = std::derived_from<T, comphelper::UnoImplBase>;

namespace detail
{
// Filter out interfaces that BaseClass already provides, to avoid duplicate
// base classes (e.g. when BaseClass inherits from XPathSettings which already
// extends XPropertySet). TODO: maybe move closer to ImplInheritanceHelper?
template <typename... Ts> struct TypeList
{
};

template <typename... As, typename... Bs>
TypeList<As..., Bs...> operator+(TypeList<As...>, TypeList<Bs...>);

template <bool Use, typename T> using MaybeType = std::conditional_t<Use, TypeList<T>, TypeList<>>;

template <typename Base, typename... Ifc>
using FilterNewInterfaces
    = decltype((TypeList<Base>{} + ... + MaybeType<!std::is_base_of_v<Ifc, Base>, Ifc>{}));

template <typename... Ifc> cppu::ImplInheritanceHelper<Ifc...> makeImplBase(TypeList<Ifc...>);

template <typename Base, typename... Ifc>
using MakeImplBase = decltype(makeImplBase(FilterNewInterfaces<Base, Ifc...>{}));

template <typename Base, typename... ExtraIfc>
using PropertyImplBase = MakeImplBase<Base, css::beans::XPropertySet, css::beans::XMultiPropertySet,
                                      css::beans::XFastPropertySet, ExtraIfc...>;

/// XPropertySetInfo implementation shared by all OPropertyImplHelper instantiations.
class PropertyImplHelperInfo : public comphelper::WeakImplHelper<css::beans::XPropertySetInfo>
{
    css::uno::Sequence<css::beans::Property> m_aProperties;

public:
    explicit PropertyImplHelperInfo(cppu::IPropertyArrayHelper& rHelper)
        : m_aProperties(rHelper.getProperties())
    {
    }

    css::uno::Sequence<css::beans::Property> SAL_CALL getProperties() override
    {
        return m_aProperties;
    }

    css::beans::Property SAL_CALL getPropertyByName(const OUString& rName) override
    {
        auto it = findByName(rName);
        if (it == m_aProperties.end() || it->Name != rName)
            throw css::beans::UnknownPropertyException(rName);
        return *it;
    }

    sal_Bool SAL_CALL hasPropertyByName(const OUString& rName) override
    {
        auto it = findByName(rName);
        return it != m_aProperties.end() && it->Name == rName;
    }

private:
    const css::beans::Property* findByName(const OUString& rName) const
    {
        return std::lower_bound(
            m_aProperties.begin(), m_aProperties.end(), rName,
            [](const css::beans::Property& p, const OUString& n) { return p.Name < n; });
    }
};

} // namespace detail

/**
   Template reimplementation of comphelper::OPropertySetHelper based on
   cppu::ImplInheritanceHelper.

   Instead of requiring multiple inheritance from both a WeakImplHelper and
   OPropertySetHelper (with the associated XInterface diamond and manual
   queryInterface resolution), this template combines everything in a single
   inheritance chain:

   @code
   // Old pattern (multiple inheritance, diamond, manual QI resolution):
   class MyService : public comphelper::WeakComponentImplHelper<XFoo, XBar>,
                     public comphelper::OPropertySetHelper
   {
       css::uno::Any SAL_CALL queryInterface(const css::uno::Type& rType) override
       {
           auto ret = WeakComponentImplHelper::queryInterface(rType);
           if (!ret.hasValue())
               ret = OPropertySetHelper::queryInterface(rType);
           return ret;
       }
       // Must also resolve getTypes(), acquire(), release()...
   };

   // New pattern (single chain, no diamond, all handled automatically):
   class MyService : public comphelper::OPropertyImplHelper<
                         comphelper::WeakComponentImplHelper<XFoo, XBar>>
   {
       // queryInterface, acquire, release, getTypes all work automatically.
       // XPropertySet, XMultiPropertySet, XFastPropertySet are included.
   };
   @endcode

   The template adds css::beans::XPropertySet, css::beans::XMultiPropertySet
   and css::beans::XFastPropertySet on top of whatever interfaces BaseClass
   and Ifc... already provide.

   Derived classes must implement:
   - getInfoHelper()
   - convertFastPropertyValue()
   - setFastPropertyValue_NoBroadcast()
   - getFastPropertyValue()

   @tparam BaseClass  Must inherit from comphelper::UnoImplBase (enforced
                       by the IsUnoImplBase concept), e.g.
                       comphelper::WeakComponentImplHelper<...> or
                       comphelper::WeakImplHelper<...>.
   @tparam Ifc        Optional additional UNO interfaces to implement beyond
                       the three property interfaces.
*/
template <IsUnoImplBase BaseClass, typename... Ifc>
class SAL_NO_VTABLE SAL_DLLPUBLIC_TEMPLATE OPropertyImplHelper
    : public detail::PropertyImplBase<BaseClass, Ifc...>
{
public:
    template <typename... Arg>
    explicit OPropertyImplHelper(Arg&&... arg)
        : detail::PropertyImplBase<BaseClass, Ifc...>(std::forward<Arg>(arg)...)
    {
    }

    // XPropertySet

    void SAL_CALL setPropertyValue(const OUString& rPropertyName,
                                   const css::uno::Any& aValue) override
    {
        cppu::IPropertyArrayHelper& rPH = getInfoHelper();
        sal_Int32 nHandle = rPH.getHandleByName(rPropertyName);
        std::unique_lock aGuard(this->m_aMutex);
        setFastPropertyValueImpl(aGuard, nHandle, aValue);
    }

    css::uno::Any SAL_CALL getPropertyValue(const OUString& aPropertyName) override
    {
        std::unique_lock aGuard(this->m_aMutex);
        return getPropertyValueImpl(aGuard, aPropertyName);
    }

    css::uno::Reference<css::beans::XPropertySetInfo> SAL_CALL getPropertySetInfo() override
    {
        return createPropertySetInfo(getInfoHelper());
    }

    void SAL_CALL addPropertyChangeListener(
        const OUString& rPropertyName,
        const css::uno::Reference<css::beans::XPropertyChangeListener>& rxListener) override;

    void SAL_CALL removePropertyChangeListener(
        const OUString& rPropertyName,
        const css::uno::Reference<css::beans::XPropertyChangeListener>& rxListener) override;

    void SAL_CALL addVetoableChangeListener(
        const OUString& rPropertyName,
        const css::uno::Reference<css::beans::XVetoableChangeListener>& rxListener) override;

    void SAL_CALL removeVetoableChangeListener(
        const OUString& rPropertyName,
        const css::uno::Reference<css::beans::XVetoableChangeListener>& rxListener) override;

    // XFastPropertySet

    void SAL_CALL setFastPropertyValue(sal_Int32 nHandle,
                                       const css::uno::Any& rValue) override final
    {
        std::unique_lock aGuard(this->m_aMutex);
        setFastPropertyValueImpl(aGuard, nHandle, rValue);
    }

    css::uno::Any SAL_CALL getFastPropertyValue(sal_Int32 nHandle) override final;

    // XMultiPropertySet

    void SAL_CALL setPropertyValues(const css::uno::Sequence<OUString>& PropertyNames,
                                    const css::uno::Sequence<css::uno::Any>& Values) override;

    css::uno::Sequence<css::uno::Any> SAL_CALL
    getPropertyValues(const css::uno::Sequence<OUString>& PropertyNames) override final;

    void SAL_CALL addPropertiesChangeListener(
        const css::uno::Sequence<OUString>&,
        const css::uno::Reference<css::beans::XPropertiesChangeListener>& Listener) override final
    {
        std::unique_lock aGuard(this->m_aMutex);
        m_aPropertiesChangeListeners.addInterface(aGuard, Listener);
    }

    void SAL_CALL removePropertiesChangeListener(
        const css::uno::Reference<css::beans::XPropertiesChangeListener>& Listener) override final
    {
        std::unique_lock aGuard(this->m_aMutex);
        m_aPropertiesChangeListeners.removeInterface(aGuard, Listener);
    }

    void SAL_CALL firePropertiesChangeEvent(
        const css::uno::Sequence<OUString>& PropertyNames,
        const css::uno::Reference<css::beans::XPropertiesChangeListener>& Listener) override final;

    // Utilities

    static css::uno::Reference<css::beans::XPropertySetInfo>
    createPropertySetInfo(cppu::IPropertyArrayHelper& rProperties)
    {
        return new detail::PropertyImplHelperInfo(rProperties);
    }

protected:
    /// Call from your disposing() override to clear all property listeners.
    void disposePropertySetListeners(std::unique_lock<std::mutex>& rGuard)
    {
        // prevent double-destruction: aEvt.Source holds a ref to this; if we're called from the
        // destructor, when it's released, ref count drops to 0, which would delete this again
        osl_atomic_increment(&this->m_refCount);
        {
            css::lang::EventObject aEvt;
            aEvt.Source = static_cast<css::beans::XPropertySet*>(this);
            m_aBoundLC.disposeAndClear(rGuard, aEvt);
            m_aVetoableLC.disposeAndClear(rGuard, aEvt);
        }
        osl_atomic_decrement(&this->m_refCount);
    }

    /// Override this if you need to do something special during setFastPropertyValue.
    virtual void setFastPropertyValueImpl(std::unique_lock<std::mutex>& rGuard, sal_Int32 nHandle,
                                          const css::uno::Any& rValue);

    css::uno::Any getPropertyValueImpl(std::unique_lock<std::mutex>& rGuard,
                                       const OUString& aPropertyName)
    {
        cppu::IPropertyArrayHelper& rPH = getInfoHelper();
        sal_Int32 nHandle = rPH.getHandleByName(aPropertyName);
        if (nHandle == -1)
            throw css::beans::UnknownPropertyException(aPropertyName);
        css::uno::Any aAny;
        getFastPropertyValue(rGuard, aAny, nHandle);
        return aAny;
    }

    void fire(std::unique_lock<std::mutex>& rGuard, const sal_Int32* pnHandles,
              const css::uno::Any* pNewValues, const css::uno::Any* pOldValues, sal_Int32 nCount,
              bool bVetoable);

    void setFastPropertyValues(std::unique_lock<std::mutex>& rGuard, sal_Int32 nSeqLen,
                               sal_Int32* pHandles, const css::uno::Any* pValues,
                               sal_Int32 nHitCount);

    /// Return the property name-to-handle mapping.
    virtual cppu::IPropertyArrayHelper& getInfoHelper() = 0;

    /// Convert and validate a property value.
    virtual bool convertFastPropertyValue(std::unique_lock<std::mutex>& rGuard,
                                          css::uno::Any& rConvertedValue, css::uno::Any& rOldValue,
                                          sal_Int32 nHandle, const css::uno::Any& rValue)
        = 0;

    /// Set a property without broadcasting.
    virtual void setFastPropertyValue_NoBroadcast(std::unique_lock<std::mutex>& rGuard,
                                                  sal_Int32 nHandle, const css::uno::Any& rValue)
        = 0;

    /// Get a property value by handle.
    virtual void getFastPropertyValue(std::unique_lock<std::mutex>& rGuard, css::uno::Any& rValue,
                                      sal_Int32 nHandle) const = 0;

    /** Set a dependent property from within setFastPropertyValue_NoBroadcast.
        The change event is deferred and fired together with the triggering
        property's event. */
    void setDependentFastPropertyValue(std::unique_lock<std::mutex>& rGuard, sal_Int32 nHandle,
                                       const css::uno::Any& rValue);

    /// Enable/disable ignoring RuntimeExceptions during listener notification.
    void setIgnoreRuntimeExceptionsWhileFiring(bool b)
    {
        m_bIgnoreRuntimeExceptionsWhileFiring = b;
    }

private:
    template <typename Func> void callNoBroadcast(Func&& fn);

    void impl_fireAll(std::unique_lock<std::mutex>& rGuard, sal_Int32* pHandles,
                      const css::uno::Any* pNewValues, const css::uno::Any* pOldValues,
                      sal_Int32 nCount);

    template <typename ListenerT>
    void addListener(const OUString& rPropertyName,
                     const css::uno::Reference<ListenerT>& rxListener, sal_Int16 nRequiredAttribute,
                     OMultiTypeInterfaceContainerHelperVar4<sal_Int32, ListenerT>& rByHandleLC,
                     comphelper::OInterfaceContainerHelper4<ListenerT>& rAllLC);

    template <typename ListenerT>
    void removeListener(const OUString& rPropertyName,
                        const css::uno::Reference<ListenerT>& rxListener,
                        OMultiTypeInterfaceContainerHelperVar4<sal_Int32, ListenerT>& rByHandleLC,
                        comphelper::OInterfaceContainerHelper4<ListenerT>& rAllLC);

    template <typename ListenerT>
    void fireListeners(std::unique_lock<std::mutex>& rGuard,
                       comphelper::OInterfaceContainerHelper4<ListenerT>* pListeners,
                       const css::beans::PropertyChangeEvent& rChangeEvent);

    OMultiTypeInterfaceContainerHelperVar4<sal_Int32, css::beans::XPropertyChangeListener>
        m_aBoundLC;
    OMultiTypeInterfaceContainerHelperVar4<sal_Int32, css::beans::XVetoableChangeListener>
        m_aVetoableLC;
    comphelper::OInterfaceContainerHelper4<css::beans::XPropertyChangeListener>
        m_aPropertyChangeListeners;
    comphelper::OInterfaceContainerHelper4<css::beans::XPropertiesChangeListener>
        m_aPropertiesChangeListeners;
    comphelper::OInterfaceContainerHelper4<css::beans::XVetoableChangeListener>
        m_aVetoableChangeListeners;
    std::vector<sal_Int32> m_aPendingHandles;
    std::vector<css::uno::Any> m_aPendingNewValues;
    std::vector<css::uno::Any> m_aPendingOldValues;
    bool m_bIgnoreRuntimeExceptionsWhileFiring = false;
};

// XPropertySet

template <IsUnoImplBase BaseClass, typename... Ifc>
void OPropertyImplHelper<BaseClass, Ifc...>::addPropertyChangeListener(
    const OUString& rPropertyName,
    const css::uno::Reference<css::beans::XPropertyChangeListener>& rxListener)
{
    addListener(rPropertyName, rxListener, css::beans::PropertyAttribute::BOUND, m_aBoundLC,
                m_aPropertyChangeListeners);
}

template <IsUnoImplBase BaseClass, typename... Ifc>
void OPropertyImplHelper<BaseClass, Ifc...>::removePropertyChangeListener(
    const OUString& rPropertyName,
    const css::uno::Reference<css::beans::XPropertyChangeListener>& rxListener)
{
    removeListener(rPropertyName, rxListener, m_aBoundLC, m_aPropertyChangeListeners);
}

template <IsUnoImplBase BaseClass, typename... Ifc>
void OPropertyImplHelper<BaseClass, Ifc...>::addVetoableChangeListener(
    const OUString& rPropertyName,
    const css::uno::Reference<css::beans::XVetoableChangeListener>& rxListener)
{
    addListener(rPropertyName, rxListener, css::beans::PropertyAttribute::CONSTRAINED,
                m_aVetoableLC, m_aVetoableChangeListeners);
}

template <IsUnoImplBase BaseClass, typename... Ifc>
void OPropertyImplHelper<BaseClass, Ifc...>::removeVetoableChangeListener(
    const OUString& rPropertyName,
    const css::uno::Reference<css::beans::XVetoableChangeListener>& rxListener)
{
    removeListener(rPropertyName, rxListener, m_aVetoableLC, m_aVetoableChangeListeners);
}

template <IsUnoImplBase BaseClass, typename... Ifc>
template <typename ListenerT>
void OPropertyImplHelper<BaseClass, Ifc...>::addListener(
    const OUString& rPropertyName, const css::uno::Reference<ListenerT>& rxListener,
    sal_Int16 nRequiredAttribute,
    OMultiTypeInterfaceContainerHelperVar4<sal_Int32, ListenerT>& rByHandleLC,
    comphelper::OInterfaceContainerHelper4<ListenerT>& rAllLC)
{
    std::unique_lock aGuard(this->m_aMutex);
    assert(!this->m_bDisposed);
    if (this->m_bDisposed)
        return;

    if (!rPropertyName.isEmpty())
    {
        cppu::IPropertyArrayHelper& rPH = getInfoHelper();
        sal_Int32 nHandle = rPH.getHandleByName(rPropertyName);
        if (nHandle == -1)
            throw css::beans::UnknownPropertyException(rPropertyName);

        sal_Int16 nAttributes;
        rPH.fillPropertyMembersByHandle(nullptr, &nAttributes, nHandle);
        if (!(nAttributes & nRequiredAttribute))
        {
            SAL_WARN("comphelper", "addListener: property lacks required attribute");
            return;
        }
        rByHandleLC.addInterface(aGuard, nHandle, rxListener);
    }
    else
        rAllLC.addInterface(aGuard, rxListener);
}

template <IsUnoImplBase BaseClass, typename... Ifc>
template <typename ListenerT>
void OPropertyImplHelper<BaseClass, Ifc...>::removeListener(
    const OUString& rPropertyName, const css::uno::Reference<ListenerT>& rxListener,
    OMultiTypeInterfaceContainerHelperVar4<sal_Int32, ListenerT>& rByHandleLC,
    comphelper::OInterfaceContainerHelper4<ListenerT>& rAllLC)
{
    std::unique_lock aGuard(this->m_aMutex);
    assert(!this->m_bDisposed);
    if (this->m_bDisposed)
        return;

    if (!rPropertyName.isEmpty())
    {
        cppu::IPropertyArrayHelper& rPH = getInfoHelper();
        sal_Int32 nHandle = rPH.getHandleByName(rPropertyName);
        if (nHandle == -1)
            throw css::beans::UnknownPropertyException(rPropertyName);
        rByHandleLC.removeInterface(aGuard, nHandle, rxListener);
    }
    else
        rAllLC.removeInterface(aGuard, rxListener);
}

template <IsUnoImplBase BaseClass, typename... Ifc>
template <typename Func>
void OPropertyImplHelper<BaseClass, Ifc...>::callNoBroadcast(Func&& fn)
{
    try
    {
        fn();
    }
    catch (const css::beans::UnknownPropertyException&)
    {
        throw;
    }
    catch (const css::beans::PropertyVetoException&)
    {
        throw;
    }
    catch (const css::lang::IllegalArgumentException&)
    {
        throw;
    }
    catch (const css::lang::WrappedTargetException&)
    {
        throw;
    }
    catch (const css::uno::RuntimeException&)
    {
        throw;
    }
    catch (const css::uno::Exception&)
    {
        css::lang::WrappedTargetException aWrap;
        aWrap.TargetException = cppu::getCaughtException();
        aWrap.Context = static_cast<css::beans::XPropertySet*>(this);
        throw aWrap;
    }
}

// XFastPropertySet

template <IsUnoImplBase BaseClass, typename... Ifc>
css::uno::Any SAL_CALL
OPropertyImplHelper<BaseClass, Ifc...>::getFastPropertyValue(sal_Int32 nHandle)
{
    cppu::IPropertyArrayHelper& rPH = getInfoHelper();
    if (!rPH.fillPropertyMembersByHandle(nullptr, nullptr, nHandle))
        throw css::beans::UnknownPropertyException(OUString::number(nHandle));

    css::uno::Any aRet;
    std::unique_lock aGuard(this->m_aMutex);
    getFastPropertyValue(aGuard, aRet, nHandle);
    return aRet;
}

template <IsUnoImplBase BaseClass, typename... Ifc>
void OPropertyImplHelper<BaseClass, Ifc...>::setFastPropertyValueImpl(
    std::unique_lock<std::mutex>& rGuard, sal_Int32 nHandle, const css::uno::Any& rValue)
{
    assert(!this->m_bDisposed);

    cppu::IPropertyArrayHelper& rPH = getInfoHelper();
    sal_Int16 nAttributes;
    if (!rPH.fillPropertyMembersByHandle(nullptr, &nAttributes, nHandle))
        throw css::beans::UnknownPropertyException(OUString::number(nHandle));
    if (nAttributes & css::beans::PropertyAttribute::READONLY)
        throw css::beans::PropertyVetoException();

    css::uno::Any aConvertedVal;
    css::uno::Any aOldVal;

    bool bChanged = convertFastPropertyValue(rGuard, aConvertedVal, aOldVal, nHandle, rValue);
    if (!bChanged)
        return;

    if (nAttributes & css::beans::PropertyAttribute::CONSTRAINED)
        fire(rGuard, &nHandle, &rValue, &aOldVal, 1, true);

    callNoBroadcast([&] { setFastPropertyValue_NoBroadcast(rGuard, nHandle, aConvertedVal); });

    impl_fireAll(rGuard, &nHandle, &rValue, &aOldVal, 1);
}

// XMultiPropertySet

template <IsUnoImplBase BaseClass, typename... Ifc>
void OPropertyImplHelper<BaseClass, Ifc...>::setPropertyValues(
    const css::uno::Sequence<OUString>& rPropertyNames,
    const css::uno::Sequence<css::uno::Any>& rValues)
{
    sal_Int32 nSeqLen = rPropertyNames.getLength();
    if (nSeqLen != rValues.getLength())
        throw css::lang::IllegalArgumentException(u"lengths do not match"_ustr,
                                                  static_cast<css::beans::XPropertySet*>(this), -1);
    auto pHandles = std::make_unique<sal_Int32[]>(nSeqLen);
    cppu::IPropertyArrayHelper& rPH = getInfoHelper();
    sal_Int32 nHitCount = rPH.fillHandles(pHandles.get(), rPropertyNames);
    if (nHitCount == 0)
        return;
    std::unique_lock aGuard(this->m_aMutex);
    setFastPropertyValues(aGuard, nSeqLen, pHandles.get(), rValues.getConstArray(), nHitCount);
}

template <IsUnoImplBase BaseClass, typename... Ifc>
css::uno::Sequence<css::uno::Any> OPropertyImplHelper<BaseClass, Ifc...>::getPropertyValues(
    const css::uno::Sequence<OUString>& rPropertyNames)
{
    sal_Int32 nSeqLen = rPropertyNames.getLength();
    auto pHandles = std::make_unique<sal_Int32[]>(nSeqLen);
    css::uno::Sequence<css::uno::Any> aValues(nSeqLen);

    cppu::IPropertyArrayHelper& rPH = getInfoHelper();
    rPH.fillHandles(pHandles.get(), rPropertyNames);

    css::uno::Any* pValues = aValues.getArray();
    std::unique_lock aGuard(this->m_aMutex);
    for (sal_Int32 i = 0; i < nSeqLen; i++)
        getFastPropertyValue(aGuard, pValues[i], pHandles[i]);

    return aValues;
}

template <IsUnoImplBase BaseClass, typename... Ifc>
void OPropertyImplHelper<BaseClass, Ifc...>::firePropertiesChangeEvent(
    const css::uno::Sequence<OUString>& rPropertyNames,
    const css::uno::Reference<css::beans::XPropertiesChangeListener>& rListener)
{
    sal_Int32 nLen = rPropertyNames.getLength();
    auto pHandles = std::make_unique<sal_Int32[]>(nLen);
    cppu::IPropertyArrayHelper& rPH = getInfoHelper();
    rPH.fillHandles(pHandles.get(), rPropertyNames);

    sal_Int32 nFireLen
        = std::count_if(pHandles.get(), pHandles.get() + nLen, [](sal_Int32 h) { return h != -1; });
    if (nFireLen)
    {
        css::uno::Sequence<css::beans::PropertyChangeEvent> aChanges(nFireLen);
        css::beans::PropertyChangeEvent* pChanges = aChanges.getArray();
        std::unique_lock aGuard(this->m_aMutex);
        css::uno::Reference<css::uno::XInterface> xSource(
            static_cast<css::beans::XPropertySet*>(this), css::uno::UNO_QUERY);
        sal_Int32 nFirePos = 0;
        for (sal_Int32 i = 0; i < nLen; i++)
        {
            if (pHandles[i] != -1)
            {
                pChanges[nFirePos].Source = xSource;
                pChanges[nFirePos].PropertyName = rPropertyNames[i];
                pChanges[nFirePos].PropertyHandle = pHandles[i];
                getFastPropertyValue(aGuard, pChanges[nFirePos].OldValue, pHandles[i]);
                pChanges[nFirePos].NewValue = pChanges[nFirePos].OldValue;
                nFirePos++;
            }
        }
        rListener->propertiesChange(aChanges);
    }
}

template <IsUnoImplBase BaseClass, typename... Ifc>
void OPropertyImplHelper<BaseClass, Ifc...>::setDependentFastPropertyValue(
    std::unique_lock<std::mutex>& rGuard, sal_Int32 nHandle, const css::uno::Any& rValue)
{
    sal_Int16 nAttributes(0);
    cppu::IPropertyArrayHelper& rPH = getInfoHelper();
    if (!rPH.fillPropertyMembersByHandle(nullptr, &nAttributes, nHandle))
        throw css::beans::UnknownPropertyException(OUString::number(nHandle));

    css::uno::Any aConverted, aOld;
    bool bChanged = convertFastPropertyValue(rGuard, aConverted, aOld, nHandle, rValue);
    if (!bChanged)
        return;

    assert(!(nAttributes & css::beans::PropertyAttribute::CONSTRAINED));

    callNoBroadcast([&] { setFastPropertyValue_NoBroadcast(rGuard, nHandle, aConverted); });

    m_aPendingHandles.push_back(nHandle);
    m_aPendingNewValues.push_back(aConverted);
    m_aPendingOldValues.push_back(aOld);
}

template <IsUnoImplBase BaseClass, typename... Ifc>
void OPropertyImplHelper<BaseClass, Ifc...>::setFastPropertyValues(
    std::unique_lock<std::mutex>& rGuard, sal_Int32 nSeqLen, sal_Int32* pHandles,
    const css::uno::Any* pValues, sal_Int32 nHitCount)
{
    assert(!this->m_bDisposed);

    cppu::IPropertyArrayHelper& rPH = getInfoHelper();

    auto pConvertedValues = std::make_unique<css::uno::Any[]>(nHitCount);
    auto pOldValues = std::make_unique<css::uno::Any[]>(nHitCount);
    sal_Int32 n = 0;

    for (sal_Int32 i = 0; i < nSeqLen; i++)
    {
        if (pHandles[i] != -1)
        {
            sal_Int16 nAttributes;
            rPH.fillPropertyMembersByHandle(nullptr, &nAttributes, pHandles[i]);
            if (nAttributes & css::beans::PropertyAttribute::READONLY)
                throw css::beans::PropertyVetoException();
            if (convertFastPropertyValue(rGuard, pConvertedValues[n], pOldValues[n], pHandles[i],
                                         pValues[i]))
            {
                pHandles[n] = pHandles[i];
                n++;
            }
        }
    }

    fire(rGuard, pHandles, pConvertedValues.get(), pOldValues.get(), n, true);

    for (sal_Int32 i = 0; i < n; i++)
        setFastPropertyValue_NoBroadcast(rGuard, pHandles[i], pConvertedValues[i]);

    impl_fireAll(rGuard, pHandles, pConvertedValues.get(), pOldValues.get(), n);
}

template <IsUnoImplBase BaseClass, typename... Ifc>
void OPropertyImplHelper<BaseClass, Ifc...>::impl_fireAll(std::unique_lock<std::mutex>& rGuard,
                                                          sal_Int32* pHandles,
                                                          const css::uno::Any* pNewValues,
                                                          const css::uno::Any* pOldValues,
                                                          sal_Int32 nCount)
{
    if (m_aPendingHandles.empty())
    {
        fire(rGuard, pHandles, pNewValues, pOldValues, nCount, false);
        return;
    }

    assert(m_aPendingHandles.size() == m_aPendingNewValues.size()
           && m_aPendingHandles.size() == m_aPendingOldValues.size());

    auto allHandles = std::exchange(m_aPendingHandles, {});
    allHandles.insert(allHandles.end(), pHandles, pHandles + nCount);

    auto allNewValues = std::exchange(m_aPendingNewValues, {});
    allNewValues.insert(allNewValues.end(), pNewValues, pNewValues + nCount);

    auto allOldValues = std::exchange(m_aPendingOldValues, {});
    allOldValues.insert(allOldValues.end(), pOldValues, pOldValues + nCount);

    fire(rGuard, allHandles.data(), allNewValues.data(), allOldValues.data(), allHandles.size(),
         false);
}

template <IsUnoImplBase BaseClass, typename... Ifc>
void OPropertyImplHelper<BaseClass, Ifc...>::fire(std::unique_lock<std::mutex>& rGuard,
                                                  const sal_Int32* pnHandles,
                                                  const css::uno::Any* pNewValues,
                                                  const css::uno::Any* pOldValues,
                                                  sal_Int32 nHandles, bool bVetoable)
{
    if (!nHandles)
        return;

    css::uno::Sequence<css::beans::PropertyChangeEvent> aEvts(nHandles);
    css::beans::PropertyChangeEvent* pEvts = aEvts.getArray();
    css::uno::Reference<css::uno::XInterface> xSource(static_cast<css::beans::XPropertySet*>(this),
                                                      css::uno::UNO_QUERY);

    cppu::IPropertyArrayHelper& rPH = getInfoHelper();
    sal_Int32 nChangesLen = 0;
    for (sal_Int32 i = 0; i < nHandles; i++)
    {
        sal_Int16 nAttributes;
        OUString aPropName;
        rPH.fillPropertyMembersByHandle(&aPropName, &nAttributes, pnHandles[i]);

        if ((bVetoable && (nAttributes & css::beans::PropertyAttribute::CONSTRAINED))
            || (!bVetoable && (nAttributes & css::beans::PropertyAttribute::BOUND)))
        {
            pEvts[nChangesLen].Source = xSource;
            pEvts[nChangesLen].PropertyName = aPropName;
            pEvts[nChangesLen].PropertyHandle = pnHandles[i];
            pEvts[nChangesLen].OldValue = pOldValues[i];
            pEvts[nChangesLen].NewValue = pNewValues[i];
            nChangesLen++;
        }
    }

    bool bIgnoreRuntimeExceptionsWhileFiring = m_bIgnoreRuntimeExceptionsWhileFiring;

    for (sal_Int32 i = 0; i < nChangesLen; i++)
    {
        if (bVetoable)
        {
            fireListeners(rGuard, m_aVetoableLC.getContainer(rGuard, aEvts[i].PropertyHandle),
                          aEvts[i]);
            fireListeners(rGuard, &m_aVetoableChangeListeners, aEvts[i]);
        }
        else
        {
            fireListeners(rGuard, m_aBoundLC.getContainer(rGuard, aEvts[i].PropertyHandle),
                          aEvts[i]);
            fireListeners(rGuard, &m_aPropertyChangeListeners, aEvts[i]);
        }
    }

    aEvts.realloc(nChangesLen);

    if (bVetoable)
        return;

    // TODO: here is a bug — unbound properties are also fired
    if (!m_aPropertiesChangeListeners.getLength(rGuard))
        return;

    OInterfaceIteratorHelper4 aIt(rGuard, m_aPropertiesChangeListeners);
    rGuard.unlock();
    while (aIt.hasMoreElements())
    {
        css::beans::XPropertiesChangeListener* pL = aIt.next().get();
        try
        {
            try
            {
                pL->propertiesChange(aEvts);
            }
            catch (css::lang::DisposedException& exc)
            {
                SAL_WARN_IF(!exc.Context.is(), "comphelper", "DisposedException without Context!");
                if (exc.Context == pL)
                {
                    rGuard.lock();
                    aIt.remove(rGuard);
                    rGuard.unlock();
                }
                else
                    throw;
            }
        }
        catch (css::uno::RuntimeException& exc)
        {
            SAL_INFO("comphelper", "caught RuntimeException while firing listeners: " << exc);
            if (!bIgnoreRuntimeExceptionsWhileFiring)
                throw;
        }
    }
    rGuard.lock();
}

template <IsUnoImplBase BaseClass, typename... Ifc>
template <typename ListenerT>
void OPropertyImplHelper<BaseClass, Ifc...>::fireListeners(
    std::unique_lock<std::mutex>& rGuard,
    comphelper::OInterfaceContainerHelper4<ListenerT>* pListeners,
    const css::beans::PropertyChangeEvent& rChangeEvent)
{
    if (!pListeners || !pListeners->getLength(rGuard))
        return;
    OInterfaceIteratorHelper4 aIt(rGuard, *pListeners);
    rGuard.unlock();
    while (aIt.hasMoreElements())
    {
        ListenerT* pL = aIt.next().get();
        try
        {
            try
            {
                if constexpr (std::is_same_v<ListenerT, css::beans::XVetoableChangeListener>)
                    pL->vetoableChange(rChangeEvent);
                else
                    pL->propertyChange(rChangeEvent);
            }
            catch (css::lang::DisposedException& exc)
            {
                SAL_WARN_IF(!exc.Context.is(), "comphelper", "DisposedException without Context!");
                if (exc.Context == pL)
                {
                    rGuard.lock();
                    aIt.remove(rGuard);
                    rGuard.unlock();
                }
                else
                    throw;
            }
        }
        catch (css::uno::RuntimeException& exc)
        {
            SAL_INFO("comphelper", "caught RuntimeException while firing listeners: " << exc);
            if (!m_bIgnoreRuntimeExceptionsWhileFiring)
                throw;
        }
    }
    rGuard.lock();
}

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
