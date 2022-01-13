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

#include <osl/diagnose.h>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <comphelper/propshlp.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <memory>
#include <sal/log.hxx>

using namespace osl;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace cppu;

extern "C" {

static int compare_OUString_Property_Impl(const void* arg1, const void* arg2) SAL_THROW_EXTERN_C()
{
    return static_cast<OUString const*>(arg1)->compareTo(static_cast<Property const*>(arg2)->Name);
}
}

namespace comphelper
{
/**
 * The class which implements the PropertySetInfo interface.
 */

namespace
{
class OPropertySetHelperInfo_Impl : public WeakImplHelper<css::beans::XPropertySetInfo>
{
    Sequence<Property> aInfos;

public:
    explicit OPropertySetHelperInfo_Impl(IPropertyArrayHelper& rHelper_);

    // XPropertySetInfo-methods
    virtual Sequence<Property> SAL_CALL getProperties() override;
    virtual Property SAL_CALL getPropertyByName(const OUString& PropertyName) override;
    virtual sal_Bool SAL_CALL hasPropertyByName(const OUString& PropertyName) override;
};
}

/**
 * Create an object that implements XPropertySetInfo IPropertyArrayHelper.
 */
OPropertySetHelperInfo_Impl::OPropertySetHelperInfo_Impl(IPropertyArrayHelper& rHelper_)
    : aInfos(rHelper_.getProperties())
{
}

/**
 * Return the sequence of properties, which are provided through the constructor.
 */
Sequence<Property> OPropertySetHelperInfo_Impl::getProperties() { return aInfos; }

/**
 * Return the sequence of properties, which are provided through the constructor.
 */
Property OPropertySetHelperInfo_Impl::getPropertyByName(const OUString& PropertyName)
{
    Property* pR;
    pR = static_cast<Property*>(bsearch(&PropertyName, aInfos.getConstArray(), aInfos.getLength(),
                                        sizeof(Property), compare_OUString_Property_Impl));
    if (!pR)
    {
        throw UnknownPropertyException(PropertyName);
    }

    return *pR;
}

/**
 * Return the sequence of properties, which are provided through the constructor.
 */
sal_Bool OPropertySetHelperInfo_Impl::hasPropertyByName(const OUString& PropertyName)
{
    Property* pR;
    pR = static_cast<Property*>(bsearch(&PropertyName, aInfos.getConstArray(), aInfos.getLength(),
                                        sizeof(Property), compare_OUString_Property_Impl));
    return pR != nullptr;
}

OPropertySetHelper::OPropertySetHelper(std::mutex& rMutex)
    : m_rMutex(rMutex)
    , m_bFireEvents(true)
{
}

/**
 * You must call disposing before.
 */
OPropertySetHelper::~OPropertySetHelper() {}

// XInterface
Any OPropertySetHelper::queryInterface(const css::uno::Type& rType)
{
    return ::cppu::queryInterface(rType, static_cast<XPropertySet*>(this),
                                  static_cast<XMultiPropertySet*>(this),
                                  static_cast<XFastPropertySet*>(this));
}

/**
 * called from the derivee's XTypeProvider::getTypes implementation
 */
css::uno::Sequence<css::uno::Type> OPropertySetHelper::getTypes()
{
    return { UnoType<css::beans::XPropertySet>::get(),
             UnoType<css::beans::XMultiPropertySet>::get(),
             UnoType<css::beans::XFastPropertySet>::get() };
}

// ComponentHelper
void OPropertySetHelper::disposing()
{
    // Create an event with this as sender
    Reference<XPropertySet> rSource = this;
    EventObject aEvt;
    aEvt.Source = rSource;

    // inform all listeners to release this object
    // The listener containers are automatically cleared
    // create a copy, because do not fire event in a guarded section
    std::unique_lock aGuard(m_rMutex);
    aBoundLC.disposeAndClear(aGuard, aEvt);
    aGuard.lock();
    aVetoableLC.disposeAndClear(aGuard, aEvt);
}

Reference<XPropertySetInfo>
OPropertySetHelper::createPropertySetInfo(IPropertyArrayHelper& rProperties)
{
    return new OPropertySetHelperInfo_Impl(rProperties);
}

// XPropertySet
void OPropertySetHelper::setPropertyValue(const OUString& rPropertyName, const Any& rValue)
{
    // get the map table
    IPropertyArrayHelper& rPH = getInfoHelper();
    // map the name to the handle
    sal_Int32 nHandle = rPH.getHandleByName(rPropertyName);
    // call the method of the XFastPropertySet interface
    setFastPropertyValue(nHandle, rValue);
}

// XPropertySet
Any OPropertySetHelper::getPropertyValue(const OUString& rPropertyName)
{
    // get the map table
    IPropertyArrayHelper& rPH = getInfoHelper();
    // map the name to the handle
    sal_Int32 nHandle = rPH.getHandleByName(rPropertyName);
    // call the method of the XFastPropertySet interface
    return getFastPropertyValue(nHandle);
}

// XPropertySet
void OPropertySetHelper::addPropertyChangeListener(
    const OUString& rPropertyName, const Reference<XPropertyChangeListener>& rxListener)
{
    std::unique_lock aGuard(m_rMutex);
    //    OSL_ENSURE( !rBHelper.bInDispose, "do not addPropertyChangeListener in the dispose call" );
    //    OSL_ENSURE( !rBHelper.bDisposed, "object is disposed" );
    //    if( rBHelper.bInDispose || rBHelper.bDisposed )
    //        return;

    // only add listeners if you are not disposed
    // a listener with no name means all properties
    if (!rPropertyName.isEmpty())
    {
        // get the map table
        IPropertyArrayHelper& rPH = getInfoHelper();
        // map the name to the handle
        sal_Int32 nHandle = rPH.getHandleByName(rPropertyName);
        if (nHandle == -1)
        {
            // property not known throw exception
            throw UnknownPropertyException(rPropertyName);
        }

        sal_Int16 nAttributes;
        rPH.fillPropertyMembersByHandle(nullptr, &nAttributes, nHandle);
        if (!(nAttributes & css::beans::PropertyAttribute::BOUND))
        {
            OSL_FAIL("add listener to an unbound property");
            // silent ignore this
            return;
        }
        // add the change listener to the helper container

        aBoundLC.addInterface(nHandle, rxListener);
    }
    else
        // add the change listener to the helper container
        aBoundLC.addInterface(-1, rxListener);
}

// XPropertySet
void OPropertySetHelper::removePropertyChangeListener(
    const OUString& rPropertyName, const Reference<XPropertyChangeListener>& rxListener)
{
    std::unique_lock aGuard(m_rMutex);
    //    OSL_ENSURE( !rBHelper.bDisposed, "object is disposed" );
    //    // all listeners are automatically released in a dispose call
    //    if( rBHelper.bInDispose || rBHelper.bDisposed )
    //        return;

    if (!rPropertyName.isEmpty())
    {
        // get the map table
        IPropertyArrayHelper& rPH = getInfoHelper();
        // map the name to the handle
        sal_Int32 nHandle = rPH.getHandleByName(rPropertyName);
        if (nHandle == -1)
            // property not known throw exception
            throw UnknownPropertyException(rPropertyName);
        aBoundLC.removeInterface(nHandle, rxListener);
    }
    else
    {
        // remove the change listener to the helper container
        aBoundLC.removeInterface(-1, rxListener);
    }
}

// XPropertySet
void OPropertySetHelper::addVetoableChangeListener(
    const OUString& rPropertyName, const Reference<XVetoableChangeListener>& rxListener)
{
    std::unique_lock aGuard(m_rMutex);
    //    OSL_ENSURE( !rBHelper.bInDispose, "do not addVetoableChangeListener in the dispose call" );
    //    OSL_ENSURE( !rBHelper.bDisposed, "object is disposed" );
    //    if( rBHelper.bInDispose || rBHelper.bDisposed )
    //        return;

    // only add listeners if you are not disposed
    // a listener with no name means all properties
    if (!rPropertyName.isEmpty())
    {
        // get the map table
        IPropertyArrayHelper& rPH = getInfoHelper();
        // map the name to the handle
        sal_Int32 nHandle = rPH.getHandleByName(rPropertyName);
        if (nHandle == -1)
        {
            // property not known throw exception
            throw UnknownPropertyException(rPropertyName);
        }

        sal_Int16 nAttributes;
        rPH.fillPropertyMembersByHandle(nullptr, &nAttributes, nHandle);
        if (!(nAttributes & PropertyAttribute::CONSTRAINED))
        {
            OSL_FAIL("addVetoableChangeListener, and property is not constrained");
            // silent ignore this
            return;
        }
        // add the vetoable listener to the helper container
        aVetoableLC.addInterface(nHandle, rxListener);
    }
    else
        // add the vetoable listener to the helper container
        aVetoableLC.addInterface(-1, rxListener);
}

// XPropertySet
void OPropertySetHelper::removeVetoableChangeListener(
    const OUString& rPropertyName, const Reference<XVetoableChangeListener>& rxListener)
{
    std::unique_lock aGuard(m_rMutex);
    //    OSL_ENSURE( !rBHelper.bDisposed, "object is disposed" );
    //    // all listeners are automatically released in a dispose call
    //    if( rBHelper.bInDispose || rBHelper.bDisposed )
    //        return;

    if (!rPropertyName.isEmpty())
    {
        // get the map table
        IPropertyArrayHelper& rPH = getInfoHelper();
        // map the name to the handle
        sal_Int32 nHandle = rPH.getHandleByName(rPropertyName);
        if (nHandle == -1)
        {
            // property not known throw exception
            throw UnknownPropertyException(rPropertyName);
        }
        // remove the vetoable listener to the helper container
        aVetoableLC.removeInterface(nHandle, rxListener);
    }
    else
        // remove the vetoable listener to the helper container
        aVetoableLC.removeInterface(-1, rxListener);
}

void OPropertySetHelper::setDependentFastPropertyValue(sal_Int32 i_handle,
                                                       const css::uno::Any& i_value)
{
    //OSL_PRECOND( rBHelper.rMutex.isAcquired(), "OPropertySetHelper::setDependentFastPropertyValue: to be called with a locked mutex only!" );
    // there is no such thing as Mutex.isAcquired, sadly ...

    sal_Int16 nAttributes(0);
    IPropertyArrayHelper& rInfo = getInfoHelper();
    if (!rInfo.fillPropertyMembersByHandle(nullptr, &nAttributes, i_handle))
        // unknown property
        throw UnknownPropertyException(OUString::number(i_handle));

    // no need to check for READONLY-ness of the property. The method is intended to be called internally, which
    // implies it might be invoked for properties which are read-only to the instance's clients, but well allowed
    // to change their value.

    Any aConverted, aOld;
    bool bChanged = convertFastPropertyValue(aConverted, aOld, i_handle, i_value);
    if (!bChanged)
        return;

    // don't fire vetoable events. This method is called with our mutex locked, so calling into listeners would not be
    // a good idea. The caller is responsible for not invoking this for constrained properties.
    OSL_ENSURE((nAttributes & PropertyAttribute::CONSTRAINED) == 0,
               "OPropertySetHelper::setDependentFastPropertyValue: not to be used for constrained "
               "properties!");

    // actually set the new value
    try
    {
        setFastPropertyValue_NoBroadcast(i_handle, aConverted);
    }
    catch (const UnknownPropertyException&)
    {
        throw; /* allowed to leave */
    }
    catch (const PropertyVetoException&)
    {
        throw; /* allowed to leave */
    }
    catch (const IllegalArgumentException&)
    {
        throw; /* allowed to leave */
    }
    catch (const WrappedTargetException&)
    {
        throw; /* allowed to leave */
    }
    catch (const RuntimeException&)
    {
        throw; /* allowed to leave */
    }
    catch (const Exception&)
    {
        // not allowed to leave this method
        WrappedTargetException aWrapped;
        aWrapped.TargetException = ::cppu::getCaughtException();
        aWrapped.Context = static_cast<XPropertySet*>(this);
        throw aWrapped;
    }

    // remember the handle/values, for the events to be fired later
    m_handles.push_back(i_handle);
    m_newValues.push_back(
        aConverted); // TODO: setFastPropertyValue notifies the unconverted value here ...?
    m_oldValues.push_back(aOld);
}

// XFastPropertySet
void OPropertySetHelper::setFastPropertyValue(sal_Int32 nHandle, const Any& rValue)
{
    //    OSL_ENSURE( !rBHelper.bInDispose, "do not setFastPropertyValue in the dispose call" );
    //    OSL_ENSURE( !rBHelper.bDisposed, "object is disposed" );

    IPropertyArrayHelper& rInfo = getInfoHelper();
    sal_Int16 nAttributes;
    if (!rInfo.fillPropertyMembersByHandle(nullptr, &nAttributes, nHandle))
    {
        // unknown property
        throw UnknownPropertyException(OUString::number(nHandle));
    }
    if (nAttributes & PropertyAttribute::READONLY)
        throw PropertyVetoException();

    Any aConvertedVal;
    Any aOldVal;

    // Will the property change?
    bool bChanged;
    {
        std::unique_lock aGuard(m_rMutex);
        bChanged = convertFastPropertyValue(aConvertedVal, aOldVal, nHandle, rValue);
        // release guard to fire events
    }
    if (!bChanged)
        return;

    // Is it a constrained property?
    if (nAttributes & PropertyAttribute::CONSTRAINED)
    {
        // In aValue is the converted rValue
        // fire a constrained event
        // second parameter NULL means constrained
        fire(&nHandle, &rValue, &aOldVal, 1, true);
    }

    {
        std::unique_lock aGuard(m_rMutex);
        try
        {
            // set the property to the new value
            setFastPropertyValue_NoBroadcast(nHandle, aConvertedVal);
        }
        catch (const css::beans::UnknownPropertyException&)
        {
            throw; /* allowed to leave */
        }
        catch (const css::beans::PropertyVetoException&)
        {
            throw; /* allowed to leave */
        }
        catch (const css::lang::IllegalArgumentException&)
        {
            throw; /* allowed to leave */
        }
        catch (const css::lang::WrappedTargetException&)
        {
            throw; /* allowed to leave */
        }
        catch (const css::uno::RuntimeException&)
        {
            throw; /* allowed to leave */
        }
        catch (const css::uno::Exception& e)
        {
            // not allowed to leave this method
            css::lang::WrappedTargetException aWrap;
            aWrap.Context = static_cast<css::beans::XPropertySet*>(this);
            aWrap.TargetException <<= e;

            throw aWrap;
        }

        // release guard to fire events
    }
    // file a change event, if the value changed
    impl_fireAll(&nHandle, &rValue, &aOldVal, 1);
}

// XFastPropertySet
Any OPropertySetHelper::getFastPropertyValue(sal_Int32 nHandle)

{
    IPropertyArrayHelper& rInfo = getInfoHelper();
    if (!rInfo.fillPropertyMembersByHandle(nullptr, nullptr, nHandle))
        // unknown property
        throw UnknownPropertyException(OUString::number(nHandle));

    Any aRet;
    std::unique_lock aGuard(m_rMutex);
    getFastPropertyValue(aRet, nHandle);
    return aRet;
}

void OPropertySetHelper::impl_fireAll(sal_Int32* i_handles, const Any* i_newValues,
                                      const Any* i_oldValues, sal_Int32 i_count)
{
    std::unique_lock aGuard(m_rMutex);
    if (m_handles.empty())
    {
        aGuard.unlock();
        fire(i_handles, i_newValues, i_oldValues, i_count, false);
        return;
    }

    const size_t additionalEvents = m_handles.size();
    OSL_ENSURE(additionalEvents == m_newValues.size() && additionalEvents == m_oldValues.size(),
               "OPropertySetHelper::impl_fireAll: inconsistency!");

    std::vector<sal_Int32> allHandles(additionalEvents + i_count);
    std::copy(m_handles.begin(), m_handles.end(), allHandles.begin());
    std::copy(i_handles, i_handles + i_count, allHandles.begin() + additionalEvents);

    std::vector<Any> allNewValues(additionalEvents + i_count);
    std::copy(m_newValues.begin(), m_newValues.end(), allNewValues.begin());
    std::copy(i_newValues, i_newValues + i_count, allNewValues.begin() + additionalEvents);

    std::vector<Any> allOldValues(additionalEvents + i_count);
    std::copy(m_oldValues.begin(), m_oldValues.end(), allOldValues.begin());
    std::copy(i_oldValues, i_oldValues + i_count, allOldValues.begin() + additionalEvents);

    m_handles.clear();
    m_newValues.clear();
    m_oldValues.clear();

    aGuard.unlock();
    fire(allHandles.data(), allNewValues.data(), allOldValues.data(), additionalEvents + i_count,
         false);
}

void OPropertySetHelper::fire(sal_Int32* pnHandles, const Any* pNewValues, const Any* pOldValues,
                              sal_Int32 nHandles, // This is the Count of the array
                              bool bVetoable)
{
    if (!m_bFireEvents)
        return;

    // Only fire, if one or more properties changed
    if (!nHandles)
        return;

    // create the event sequence of all changed properties
    std::vector<PropertyChangeEvent> aEvts;
    aEvts.reserve(nHandles);
    Reference<XInterface> xSource(static_cast<XPropertySet*>(this), UNO_QUERY);
    // Loop over all changed properties to fill the event struct
    for (sal_Int32 i = 0; i < nHandles; i++)
    {
        // Vetoable fire and constrained attribute set or
        // Change fire and Changed and bound attribute set
        IPropertyArrayHelper& rInfo = getInfoHelper();
        sal_Int16 nAttributes;
        OUString aPropName;
        rInfo.fillPropertyMembersByHandle(&aPropName, &nAttributes, pnHandles[i]);

        if ((bVetoable && (nAttributes & PropertyAttribute::CONSTRAINED))
            || (!bVetoable && (nAttributes & PropertyAttribute::BOUND)))
        {
            aEvts.emplace_back(xSource, aPropName, /*Further*/ false, pnHandles[i], pOldValues[i],
                               pNewValues[i]);
        }
    }

    // fire the events for all changed properties
    for (sal_Int32 i = 0; i < static_cast<sal_Int32>(aEvts.size()); i++)
    {
        // get the listener container for the property name
        // Iterate over all listeners and send events
        if (bVetoable) // fire change Events?
        {
            auto pLC = aVetoableLC.getContainer(aEvts[i].PropertyHandle);
            if (pLC)
                pLC->notifyEach(&XVetoableChangeListener::vetoableChange, aEvts[i]);
        }
        else
        {
            auto pLC = aBoundLC.getContainer(aEvts[i].PropertyHandle);
            if (pLC)
                pLC->notifyEach(&XPropertyChangeListener::propertyChange, aEvts[i]);
        }
        // broadcast to all listeners with "" property name
        if (bVetoable)
        {
            // fire change Events?
            auto pLC = aVetoableLC.getContainer(-1);
            if (pLC)
                pLC->notifyEach(&XVetoableChangeListener::vetoableChange, aEvts[i]);
        }
        else
        {
            auto pLC = aBoundLC.getContainer(-1);
            if (pLC)
                pLC->notifyEach(&XPropertyChangeListener::propertyChange, aEvts[i]);
        }
    }

    if (bVetoable || maPropertiesChangeListeners.getLength() == 0)
        return;

    // reduce array to changed properties
    auto aEvtsSeq = comphelper::containerToSequence(aEvts);

    // Here is a Bug, unbound properties are also fired
    maPropertiesChangeListeners.notifyEach(&XPropertiesChangeListener::propertiesChange, aEvtsSeq);
}

// OPropertySetHelper
void OPropertySetHelper::setFastPropertyValues(sal_Int32 nSeqLen, sal_Int32* pHandles,
                                               const Any* pValues, sal_Int32 nHitCount)
{
    //    OSL_ENSURE( !rBHelper.bInDispose, "do not getFastPropertyValue in the dispose call" );
    //    OSL_ENSURE( !rBHelper.bDisposed, "object is disposed" );

    // get the map table
    IPropertyArrayHelper& rPH = getInfoHelper();

    std::unique_ptr<Any[]> pConvertedValues(new Any[nHitCount]);
    std::unique_ptr<Any[]> pOldValues(new Any[nHitCount]);
    sal_Int32 n = 0;
    sal_Int32 i;

    {
        // must lock the mutex outside the loop. So all values are consistent.
        std::unique_lock aGuard(m_rMutex);
        for (i = 0; i < nSeqLen; i++)
        {
            if (pHandles[i] != -1)
            {
                sal_Int16 nAttributes;
                rPH.fillPropertyMembersByHandle(nullptr, &nAttributes, pHandles[i]);
                if (nAttributes & PropertyAttribute::READONLY)
                {
                    throw PropertyVetoException();
                }
                // Will the property change?
                if (convertFastPropertyValue(pConvertedValues[n], pOldValues[n], pHandles[i],
                                             pValues[i]))
                {
                    // only increment if the property really change
                    pHandles[n] = pHandles[i];
                    n++;
                }
            }
        }
        // release guard to fire events
    }

    // fire vetoable events
    fire(pHandles, pConvertedValues.get(), pOldValues.get(), n, true);

    {
        // must lock the mutex outside the loop.
        std::unique_lock aGuard(m_rMutex);
        // Loop over all changed properties
        for (i = 0; i < n; i++)
        {
            // Will the property change?
            setFastPropertyValue_NoBroadcast(pHandles[i], pConvertedValues[i]);
        }
        // release guard to fire events
    }

    // fire change events
    impl_fireAll(pHandles, pConvertedValues.get(), pOldValues.get(), n);
}

// XMultiPropertySet
/**
 * The sequence may be contain not known properties. The implementation
 * must ignore these properties.
 */
void OPropertySetHelper::setPropertyValues(const Sequence<OUString>& rPropertyNames,
                                           const Sequence<Any>& rValues)
{
    sal_Int32 nSeqLen = rPropertyNames.getLength();
    std::unique_ptr<sal_Int32[]> pHandles(new sal_Int32[nSeqLen]);
    // get the map table
    IPropertyArrayHelper& rPH = getInfoHelper();
    // fill the handle array
    sal_Int32 nHitCount = rPH.fillHandles(pHandles.get(), rPropertyNames);
    if (nHitCount != 0)
        setFastPropertyValues(nSeqLen, pHandles.get(), rValues.getConstArray(), nHitCount);
}

// XMultiPropertySet
Sequence<Any> OPropertySetHelper::getPropertyValues(const Sequence<OUString>& rPropertyNames)
{
    sal_Int32 nSeqLen = rPropertyNames.getLength();
    std::unique_ptr<sal_Int32[]> pHandles(new sal_Int32[nSeqLen]);
    Sequence<Any> aValues(nSeqLen);

    // get the map table
    IPropertyArrayHelper& rPH = getInfoHelper();
    // fill the handle array
    rPH.fillHandles(pHandles.get(), rPropertyNames);

    Any* pValues = aValues.getArray();

    std::unique_lock aGuard(m_rMutex);
    // fill the sequence with the values
    for (sal_Int32 i = 0; i < nSeqLen; i++)
        getFastPropertyValue(pValues[i], pHandles[i]);

    return aValues;
}

// XMultiPropertySet
void OPropertySetHelper::addPropertiesChangeListener(
    const Sequence<OUString>&, const Reference<XPropertiesChangeListener>& rListener)
{
    maPropertiesChangeListeners.addInterface(rListener);
}

// XMultiPropertySet
void OPropertySetHelper::removePropertiesChangeListener(
    const Reference<XPropertiesChangeListener>& rListener)
{
    maPropertiesChangeListeners.removeInterface(rListener);
}

// XMultiPropertySet
void OPropertySetHelper::firePropertiesChangeEvent(
    const Sequence<OUString>& rPropertyNames, const Reference<XPropertiesChangeListener>& rListener)
{
    sal_Int32 nLen = rPropertyNames.getLength();
    std::unique_ptr<sal_Int32[]> pHandles(new sal_Int32[nLen]);
    IPropertyArrayHelper& rPH = getInfoHelper();
    rPH.fillHandles(pHandles.get(), rPropertyNames);
    const OUString* pNames = rPropertyNames.getConstArray();

    // get the count of matching properties
    sal_Int32 nFireLen = 0;
    sal_Int32 i;
    for (i = 0; i < nLen; i++)
        if (pHandles[i] != -1)
            nFireLen++;

    Sequence<PropertyChangeEvent> aChanges(nFireLen);
    PropertyChangeEvent* pChanges = aChanges.getArray();

    {
        // must lock the mutex outside the loop. So all values are consistent.
        std::unique_lock aGuard(m_rMutex);
        Reference<XInterface> xSource(static_cast<XPropertySet*>(this), UNO_QUERY);
        sal_Int32 nFirePos = 0;
        for (i = 0; i < nLen; i++)
        {
            if (pHandles[i] != -1)
            {
                pChanges[nFirePos].Source = xSource;
                pChanges[nFirePos].PropertyName = pNames[i];
                pChanges[nFirePos].PropertyHandle = pHandles[i];
                getFastPropertyValue(pChanges[nFirePos].OldValue, pHandles[i]);
                pChanges[nFirePos].NewValue = pChanges[nFirePos].OldValue;
                nFirePos++;
            }
        }
        // release guard to fire events
    }
    if (nFireLen)
        rListener->propertiesChange(aChanges);
}

} // end namespace cppu

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
