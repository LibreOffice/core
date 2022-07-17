/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <DataTable.hxx>

#include <LinePropertiesHelper.hxx>
#include <FillProperties.hxx>
#include <CharacterProperties.hxx>
#include <ModifyListenerHelper.hxx>
#include <PropertyHelper.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <algorithm>

using namespace css;

namespace
{
enum
{
    DataTableProperty_HorizontalBorder,
    DataTableProperty_VerticalBorder,
    DataTableProperty_Outilne,
    DataTableProperty_Keys,
};

void lcl_AddPropertiesToVector(std::vector<beans::Property>& rProps)
{
    auto const nBound = beans::PropertyAttribute::BOUND;
    auto const nMaybeDefault = beans::PropertyAttribute::MAYBEDEFAULT;

    rProps.emplace_back("HBorder", DataTableProperty_HorizontalBorder, cppu::UnoType<bool>::get(),
                        nBound | nMaybeDefault);
    rProps.emplace_back("VBorder", DataTableProperty_VerticalBorder, cppu::UnoType<bool>::get(),
                        nBound | nMaybeDefault);
    rProps.emplace_back("Outline", DataTableProperty_Outilne, cppu::UnoType<bool>::get(),
                        nBound | nMaybeDefault);
    rProps.emplace_back("Keys", DataTableProperty_Keys, cppu::UnoType<bool>::get(),
                        nBound | nMaybeDefault);
}

struct StaticDataTableDefaults_Initializer
{
    ::chart::tPropertyValueMap* operator()()
    {
        static ::chart::tPropertyValueMap aStaticDefaults;
        lcl_AddDefaultsToMap(aStaticDefaults);
        return &aStaticDefaults;
    }

private:
    static void lcl_AddDefaultsToMap(::chart::tPropertyValueMap& aMap)
    {
        ::chart::LinePropertiesHelper::AddDefaultsToMap(aMap);
        ::chart::FillProperties::AddDefaultsToMap(aMap);
        ::chart::CharacterProperties::AddDefaultsToMap(aMap);

        ::chart::PropertyHelper::setPropertyValueDefault(aMap, DataTableProperty_HorizontalBorder,
                                                         true);
        ::chart::PropertyHelper::setPropertyValueDefault(aMap, DataTableProperty_VerticalBorder,
                                                         true);
        ::chart::PropertyHelper::setPropertyValueDefault(aMap, DataTableProperty_Outilne, true);
        ::chart::PropertyHelper::setPropertyValueDefault(aMap, DataTableProperty_Keys, false);
    }
};

struct StaticDataTableDefaults
    : public rtl::StaticAggregate<::chart::tPropertyValueMap, StaticDataTableDefaults_Initializer>
{
};

struct StaticDataTableInfoHelper_Initializer
{
    cppu::OPropertyArrayHelper* operator()()
    {
        static cppu::OPropertyArrayHelper aPropHelper(lcl_GetPropertySequence());
        return &aPropHelper;
    }

private:
    static uno::Sequence<beans::Property> lcl_GetPropertySequence()
    {
        std::vector<beans::Property> aProperties;
        lcl_AddPropertiesToVector(aProperties);
        ::chart::LinePropertiesHelper::AddPropertiesToVector(aProperties);
        ::chart::FillProperties::AddPropertiesToVector(aProperties);
        std::sort(aProperties.begin(), aProperties.end(), ::chart::PropertyNameLess());

        return comphelper::containerToSequence(aProperties);
    }
};

struct StaticDataTableInfoHelper
    : public rtl::StaticAggregate<::cppu::OPropertyArrayHelper,
                                  StaticDataTableInfoHelper_Initializer>
{
};

struct StaticDataTableInfo_Initializer
{
    uno::Reference<beans::XPropertySetInfo>* operator()()
    {
        static uno::Reference<beans::XPropertySetInfo> xPropertySetInfo(
            ::cppu::OPropertySetHelper::createPropertySetInfo(*StaticDataTableInfoHelper::get()));
        return &xPropertySetInfo;
    }
};

struct StaticDataTableInfo : public rtl::StaticAggregate<uno::Reference<beans::XPropertySetInfo>,
                                                         StaticDataTableInfo_Initializer>
{
};

} // anonymous namespace

namespace chart
{
DataTable::DataTable()
    : ::property::OPropertySet(m_aMutex)
    , m_xModifyEventForwarder(new ModifyEventForwarder())
{
}

DataTable::DataTable(const DataTable& rOther)
    : DataTable_Base(rOther)
    , ::property::OPropertySet(rOther, m_aMutex)
    , m_xModifyEventForwarder(new ModifyEventForwarder())
{
}

DataTable::~DataTable() = default;

// ____ XCloneable ____
uno::Reference<util::XCloneable> SAL_CALL DataTable::createClone()
{
    return uno::Reference<util::XCloneable>(new DataTable(*this));
}

// ____ XModifyBroadcaster ____
void SAL_CALL DataTable::addModifyListener(const uno::Reference<util::XModifyListener>& aListener)
{
    m_xModifyEventForwarder->addModifyListener(aListener);
}

void SAL_CALL
DataTable::removeModifyListener(const uno::Reference<util::XModifyListener>& aListener)
{
    m_xModifyEventForwarder->removeModifyListener(aListener);
}

// ____ XModifyListener ____
void SAL_CALL DataTable::modified(const lang::EventObject& aEvent)
{
    m_xModifyEventForwarder->modified(aEvent);
}

// ____ XEventListener (base of XModifyListener) ____
void SAL_CALL DataTable::disposing(const lang::EventObject& /* Source */)
{
    // nothing
}

// ____ OPropertySet ____
void DataTable::firePropertyChangeEvent()
{
    m_xModifyEventForwarder->modified(lang::EventObject(static_cast<uno::XWeak*>(this)));
}

// ____ OPropertySet ____
void DataTable::GetDefaultValue(sal_Int32 nHandle, uno::Any& rAny) const
{
    const tPropertyValueMap& rStaticDefaults = *StaticDataTableDefaults::get();
    auto aFound = rStaticDefaults.find(nHandle);
    if (aFound == rStaticDefaults.end())
        rAny.clear();
    else
        rAny = (*aFound).second;
}

::cppu::IPropertyArrayHelper& SAL_CALL DataTable::getInfoHelper()
{
    return *StaticDataTableInfoHelper::get();
}

// ____ XPropertySet ____
uno::Reference<beans::XPropertySetInfo> SAL_CALL DataTable::getPropertySetInfo()
{
    return *StaticDataTableInfo::get();
}

// implement XServiceInfo methods basing upon getSupportedServiceNames_Static
OUString SAL_CALL DataTable::getImplementationName()
{
    return "com.sun.star.comp.chart2.DataTable";
}

sal_Bool SAL_CALL DataTable::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence<OUString> SAL_CALL DataTable::getSupportedServiceNames()
{
    return { "com.sun.star.chart2.DataTable", "com.sun.star.beans.PropertySet",
             "com.sun.star.drawing.FillProperties", "com.sun.star.drawing.LineProperties" };
}

IMPLEMENT_FORWARD_XINTERFACE2(DataTable, DataTable_Base, ::property::OPropertySet)
IMPLEMENT_FORWARD_XTYPEPROVIDER2(DataTable, DataTable_Base, ::property::OPropertySet)

} //  namespace chart

extern "C" SAL_DLLPUBLIC_EXPORT uno::XInterface*
com_sun_star_comp_chart2_DataTable_get_implementation(
    css::uno::XComponentContext* /*pComponentContext*/, uno::Sequence<uno::Any> const& /*rAny*/)
{
    return cppu::acquire(new ::chart::DataTable);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
