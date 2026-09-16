/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <com/sun/star/chart2/data/XDataProvider.hpp>
#include <com/sun/star/chart2/data/PivotTableFieldEntry.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>

#include <chart2/AbstractPivotTableDataProvider.hxx>
#include <svl/lstner.hxx>
#include <cppuhelper/implbase.hxx>
#include <rtl/ustring.hxx>
#include <svl/itemprop.hxx>

#include <vector>
#include <unordered_map>

namespace com::sun::star::chart2::data { class XDataSource; }
namespace com::sun::star::chart2::data { class XLabeledDataSequence; }
namespace cpo::uno { class XComponentContext; }

class ScDocument;

namespace sc
{

struct ValueAndFormat;
class PivotTableDataSequence;

typedef cppu::WeakImplHelper<css::chart2::data::XDataProvider,
                             css::beans::XPropertySet,
                             css::lang::XServiceInfo,
                             css::util::XModifyBroadcaster>
            PivotTableDataProvider_Base;

class PivotTableDataProvider final : public PivotTableDataProvider_Base,
                                     public chart2api::AbstractPivotTableDataProvider,
                                     public SfxListener
{
public:

    explicit PivotTableDataProvider(ScDocument& rDoc);
    virtual ~PivotTableDataProvider() override;
    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override;

    // XDataProvider
    virtual bool
        createDataSourcePossible(const cpo::uno::Sequence<css::beans::PropertyValue>& aArguments) override;

    virtual cpo::uno::Reference<css::chart2::data::XDataSource>
        createDataSource(const cpo::uno::Sequence<css::beans::PropertyValue>& aArguments) override;

    virtual cpo::uno::Sequence<css::beans::PropertyValue>
        detectArguments(const cpo::uno::Reference<css::chart2::data::XDataSource>& xDataSource) override;

    virtual bool
        createDataSequenceByRangeRepresentationPossible(const OUString& aRangeRepresentation) override;

    virtual cpo::uno::Reference<css::chart2::data::XDataSequence>
            createDataSequenceByRangeRepresentation(const OUString& aRangeRepresentation) override;

    virtual cpo::uno::Reference<css::chart2::data::XDataSequence>
        createDataSequenceByValueArray(const OUString& aRole, const OUString& aRangeRepresentation,
            const OUString& aRoleQualifier) override;

    virtual cpo::uno::Reference<css::sheet::XRangeSelection> getRangeSelection() override;

    // AbstractPivotTableDataProvider
    virtual const std::vector<css::chart2::data::PivotTableFieldEntry>&
        getColumnFields() const override;
    virtual const std::vector<css::chart2::data::PivotTableFieldEntry>&
        getRowFields() const override;
    virtual const std::vector<css::chart2::data::PivotTableFieldEntry>&
        getPageFields() const override;
    virtual const std::vector<css::chart2::data::PivotTableFieldEntry>&
        getDataFields() const override;

    virtual const OUString & getPivotTableName() const override;

    virtual void setPivotTableName(const OUString& sPivotTableName) override;

    virtual bool hasPivotTable() const override;

    virtual cpo::uno::Reference<css::chart2::data::XDataSequence>
        createDataSequenceOfValuesByIndex(sal_Int32 nIndex) override;
    virtual cpo::uno::Reference<css::chart2::data::XDataSequence>
        createDataSequenceOfLabelsByIndex(sal_Int32 nIndex) override;
    virtual cpo::uno::Reference<css::chart2::data::XDataSequence>
        createDataSequenceOfCategories() override;

    virtual OUString getFieldOutputDescription(sal_Int32 nPageFieldIndex) const override;

    // XPropertySet
    virtual cpo::uno::Reference<css::beans::XPropertySetInfo> getPropertySetInfo() override;

    virtual void
        setPropertyValue(const OUString& rPropertyName, const cpo::uno::Any& rValue) override;

    virtual cpo::uno::Any
        getPropertyValue(const OUString& rPropertyName) override;

    virtual void addPropertyChangeListener(
            const OUString& rPropertyName,
            const cpo::uno::Reference<css::beans::XPropertyChangeListener>& xListener) override;

    virtual void removePropertyChangeListener(
            const OUString& rPropertyName,
            const cpo::uno::Reference<css::beans::XPropertyChangeListener>& rListener) override;

    virtual void addVetoableChangeListener(
            const OUString& rPropertyName,
            const cpo::uno::Reference<css::beans::XVetoableChangeListener>& rListener) override;

    virtual void removeVetoableChangeListener(
            const OUString& rPropertyName,
            const cpo::uno::Reference<css::beans::XVetoableChangeListener>& rListener) override;

    // XModifyBroadcaster
    virtual void
        addModifyListener(const cpo::uno::Reference<css::util::XModifyListener>& aListener) override;

    virtual void
        removeModifyListener(const cpo::uno::Reference<css::util::XModifyListener>& aListener) override;

    // XServiceInfo
    virtual OUString getImplementationName() override;

    virtual bool supportsService(const OUString& rServiceName) override;

    virtual cpo::uno::Sequence<OUString> getSupportedServiceNames() override;

private:

    cpo::uno::Reference<css::chart2::data::XDataSource>
        createValuesDataSource();

    cpo::uno::Reference<css::chart2::data::XDataSource>
        createCategoriesDataSource(bool bOrientationIsColumn);

    cpo::uno::Reference<css::chart2::data::XLabeledDataSequence> newLabeledDataSequence();

    rtl::Reference<PivotTableDataSequence> assignLabelsToDataSequence(size_t nIndex);

    rtl::Reference<PivotTableDataSequence> assignValuesToDataSequence(size_t nIndex);

    rtl::Reference<PivotTableDataSequence> assignFirstCategoriesToDataSequence();

    void collectPivotTableData();

    ScDocument*        m_pDocument;
    OUString           m_sPivotTableName;
    SfxItemPropertySet m_aPropSet;
    bool               m_bIncludeHiddenCells;

    std::vector<std::vector<ValueAndFormat>> m_aCategoriesColumnOrientation;
    std::vector<std::vector<ValueAndFormat>> m_aCategoriesRowOrientation;
    std::vector<std::vector<ValueAndFormat>> m_aLabels;
    std::vector<std::vector<ValueAndFormat>> m_aDataRowVector;

    std::vector<css::chart2::data::PivotTableFieldEntry> m_aColumnFields;
    std::vector<css::chart2::data::PivotTableFieldEntry> m_aRowFields;
    std::vector<css::chart2::data::PivotTableFieldEntry> m_aPageFields;
    std::vector<css::chart2::data::PivotTableFieldEntry> m_aDataFields;

    std::unordered_map<sal_Int32, OUString> m_aFieldOutputDescriptionMap;

    bool m_bNeedsUpdate;

    cpo::uno::Reference<cpo::uno::XComponentContext> m_xContext;

    std::vector<cpo::uno::Reference<css::util::XModifyListener>> m_aValueListeners;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
