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

#include "DataPointCustomLabelField.hxx"

#include <CharacterProperties.hxx>
#include <PropertyHelper.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <cppuhelper/supportsservice.hxx>

using namespace ::com::sun::star;

using ::com::sun::star::beans::Property;
using ::com::sun::star::uno::Sequence;
using ::osl::MutexGuard;

namespace
{
struct StaticDataPointCustomLabelFieldDefaults_Initializer
{
    ::chart::tPropertyValueMap* operator()()
    {
        static ::chart::tPropertyValueMap aStaticDefaults;
        ::chart::CharacterProperties::AddDefaultsToMap(aStaticDefaults);
        return &aStaticDefaults;
    }
};

struct StaticDataPointCustomLabelFieldDefaults
    : public rtl::StaticAggregate<::chart::tPropertyValueMap,
                                  StaticDataPointCustomLabelFieldDefaults_Initializer>
{
};

struct StaticDataPointCustomLabelFieldInfoHelper_Initializer
{
    ::cppu::OPropertyArrayHelper* operator()()
    {
        static ::cppu::OPropertyArrayHelper aPropHelper(lcl_GetPropertySequence());
        return &aPropHelper;
    }

private:
    static Sequence<Property> lcl_GetPropertySequence()
    {
        std::vector<css::beans::Property> aProperties;
        ::chart::CharacterProperties::AddPropertiesToVector(aProperties);

        std::sort(aProperties.begin(), aProperties.end(), ::chart::PropertyNameLess());

        return comphelper::containerToSequence(aProperties);
    }
};

struct StaticDataPointCustomLabelFieldInfoHelper
    : public rtl::StaticAggregate<::cppu::OPropertyArrayHelper,
                                  StaticDataPointCustomLabelFieldInfoHelper_Initializer>
{
};

struct StaticDataPointCustomLabelFieldInfo_Initializer
{
    uno::Reference<beans::XPropertySetInfo>* operator()()
    {
        static uno::Reference<beans::XPropertySetInfo> xPropertySetInfo(
            ::cppu::OPropertySetHelper::createPropertySetInfo(
                *StaticDataPointCustomLabelFieldInfoHelper::get()));
        return &xPropertySetInfo;
    }
};

struct StaticDataPointCustomLabelFieldInfo
    : public rtl::StaticAggregate<uno::Reference<beans::XPropertySetInfo>,
                                  StaticDataPointCustomLabelFieldInfo_Initializer>
{
};

} // anonymous namespace

namespace chart
{
DataPointCustomLabelField::DataPointCustomLabelField()
    : ::property::OPropertySet(m_aMutex)
    , m_aType(chart2::DataPointCustomLabelFieldType::DataPointCustomLabelFieldType_TEXT)
    , m_aString()
    , m_xModifyEventForwarder(ModifyListenerHelper::createModifyEventForwarder())
{
}

DataPointCustomLabelField::DataPointCustomLabelField(const DataPointCustomLabelField& rOther)
    : MutexContainer()
    , impl::DataPointCustomLabelField_Base()
    , ::property::OPropertySet(rOther, m_aMutex)
    , m_aType(rOther.m_aType)
    , m_aString(rOther.m_aString)
    , m_xModifyEventForwarder(ModifyListenerHelper::createModifyEventForwarder())
{
}

DataPointCustomLabelField::~DataPointCustomLabelField() {}

// ____ XCloneable ____
uno::Reference<util::XCloneable> SAL_CALL DataPointCustomLabelField::createClone()
{
    return uno::Reference<util::XCloneable>(new DataPointCustomLabelField(*this));
}

// ____ XDataPointCustomLabelField ____
css::chart2::DataPointCustomLabelFieldType SAL_CALL DataPointCustomLabelField::getFieldType()
{
    MutexGuard aGuard(GetMutex());
    return m_aType;
}

void SAL_CALL
DataPointCustomLabelField::setFieldType(const css::chart2::DataPointCustomLabelFieldType Type)
{
    {
        MutexGuard aGuard(GetMutex());
        m_aType = Type;
    }
    //don't keep the mutex locked while calling out
    fireModifyEvent();
}

// ____ XFormattedString ____
OUString SAL_CALL DataPointCustomLabelField::getString()
{
    MutexGuard aGuard(GetMutex());
    return m_aString;
}

void SAL_CALL DataPointCustomLabelField::setString(const OUString& String)
{
    {
        MutexGuard aGuard(GetMutex());
        m_aString = String;
    }
    //don't keep the mutex locked while calling out
    fireModifyEvent();
}

// ____ XModifyBroadcaster ____
void SAL_CALL
DataPointCustomLabelField::addModifyListener(const uno::Reference<util::XModifyListener>& aListener)
{
    try
    {
        uno::Reference<util::XModifyBroadcaster> xBroadcaster(m_xModifyEventForwarder,
                                                              uno::UNO_QUERY_THROW);
        xBroadcaster->addModifyListener(aListener);
    }
    catch (const uno::Exception& ex)
    {
        SAL_WARN("chart2", "Exception caught. " << ex);
    }
}

void SAL_CALL DataPointCustomLabelField::removeModifyListener(
    const uno::Reference<util::XModifyListener>& aListener)
{
    try
    {
        uno::Reference<util::XModifyBroadcaster> xBroadcaster(m_xModifyEventForwarder,
                                                              uno::UNO_QUERY_THROW);
        xBroadcaster->removeModifyListener(aListener);
    }
    catch (const uno::Exception& ex)
    {
        SAL_WARN("chart2", "Exception caught. " << ex);
    }
}

// ____ XModifyListener ____
void SAL_CALL DataPointCustomLabelField::modified(const lang::EventObject& aEvent)
{
    m_xModifyEventForwarder->modified(aEvent);
}

// ____ XEventListener (base of XModifyListener) ____
void SAL_CALL DataPointCustomLabelField::disposing(const lang::EventObject& /* Source */)
{
    // nothing
}

// ____ OPropertySet ____
void DataPointCustomLabelField::firePropertyChangeEvent() { fireModifyEvent(); }

void DataPointCustomLabelField::fireModifyEvent()
{
    m_xModifyEventForwarder->modified(lang::EventObject(static_cast<uno::XWeak*>(this)));
}

// ____ OPropertySet ____
uno::Any DataPointCustomLabelField::GetDefaultValue(sal_Int32 nHandle) const
{
    const tPropertyValueMap& rStaticDefaults = *StaticDataPointCustomLabelFieldDefaults::get();
    tPropertyValueMap::const_iterator aFound(rStaticDefaults.find(nHandle));
    if (aFound == rStaticDefaults.end())
        return uno::Any();
    return (*aFound).second;
}

// ____ OPropertySet ____
::cppu::IPropertyArrayHelper& SAL_CALL DataPointCustomLabelField::getInfoHelper()
{
    return *StaticDataPointCustomLabelFieldInfoHelper::get();
}

// ____ XPropertySet ____
uno::Reference<beans::XPropertySetInfo> SAL_CALL DataPointCustomLabelField::getPropertySetInfo()
{
    return *StaticDataPointCustomLabelFieldInfo::get();
}

using impl::DataPointCustomLabelField_Base;

IMPLEMENT_FORWARD_XINTERFACE2(DataPointCustomLabelField, DataPointCustomLabelField_Base,
                              ::property::OPropertySet)
IMPLEMENT_FORWARD_XTYPEPROVIDER2(DataPointCustomLabelField, DataPointCustomLabelField_Base,
                                 ::property::OPropertySet)

// do this in derived classes!

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
OUString SAL_CALL DataPointCustomLabelField::getImplementationName()
{
    return OUString("com.sun.star.comp.chart.DataPointCustomLabelField");
}

sal_Bool SAL_CALL DataPointCustomLabelField::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence<OUString> SAL_CALL DataPointCustomLabelField::getSupportedServiceNames()
{
    return { "com.sun.star.chart2.DataPointCustomLabelField", "com.sun.star.chart2.FormattedString",
             "com.sun.star.beans.PropertySet" };
}

} //  namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_chart_DataPointCustomLabelField_get_implementation(
    css::uno::XComponentContext*, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new ::chart::DataPointCustomLabelField);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
