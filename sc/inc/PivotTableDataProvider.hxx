/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <com/sun/star/chart2/data/XDataProvider.hpp>
#include <com/sun/star/chart2/data/XPivotTableDataProvider.hpp>
#include <com/sun/star/chart2/data/PivotTableFieldEntry.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>

#include <svl/lstner.hxx>
#include <cppuhelper/implbase.hxx>
#include <rtl/ustring.hxx>
#include <svl/itemprop.hxx>

#include <vector>
#include <unordered_map>

namespace com::sun::star::chart2::data { class XDataSequence; }
namespace com::sun::star::chart2::data { class XDataSource; }
namespace com::sun::star::chart2::data { class XLabeledDataSequence; }
namespace com::sun::star::uno { class XComponentContext; }

class ScDocument;

namespace sc
{

struct ValueAndFormat;

typedef cppu::WeakImplHelper<css::chart2::data::XDataProvider,
                             css::chart2::data::XPivotTableDataProvider,
                             css::beans::XPropertySet,
                             css::lang::XServiceInfo,
                             css::util::XModifyBroadcaster>
            PivotTableDataProvider_Base;

class PivotTableDataProvider final : public PivotTableDataProvider_Base, public SfxListener
{
public:

    explicit PivotTableDataProvider(ScDocument& rDoc);
    virtual ~PivotTableDataProvider() override;
    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override;

    // XDataProvider
    virtual sal_Bool SAL_CALL
        createDataSourcePossible(const css::uno::Sequence<css::beans::PropertyValue>& aArguments) override;

    virtual css::uno::Reference<css::chart2::data::XDataSource> SAL_CALL
        createDataSource(const css::uno::Sequence<css::beans::PropertyValue>& aArguments) override;

    virtual css::uno::Sequence<css::beans::PropertyValue> SAL_CALL
        detectArguments(const css::uno::Reference<css::chart2::data::XDataSource>& xDataSource) override;

    virtual sal_Bool SAL_CALL
        createDataSequenceByRangeRepresentationPossible(const OUString& aRangeRepresentation) override;

    virtual css::uno::Reference<css::chart2::data::XDataSequence> SAL_CALL
            createDataSequenceByRangeRepresentation(const OUString& aRangeRepresentation) override;

    virtual css::uno::Reference<css::chart2::data::XDataSequence> SAL_CALL
        createDataSequenceByValueArray(const OUString& aRole, const OUString& aRangeRepresentation) override;

    virtual css::uno::Reference<css::sheet::XRangeSelection> SAL_CALL getRangeSelection() override;

    // XPivotTableDataProvider
    virtual css::uno::Sequence<css::chart2::data::PivotTableFieldEntry> SAL_CALL
        getColumnFields() override;
    virtual css::uno::Sequence<css::chart2::data::PivotTableFieldEntry> SAL_CALL
        getRowFields() override;
    virtual css::uno::Sequence<css::chart2::data::PivotTableFieldEntry> SAL_CALL
        getPageFields() override;
    virtual css::uno::Sequence<css::chart2::data::PivotTableFieldEntry> SAL_CALL
        getDataFields() override;

    virtual OUString SAL_CALL getPivotTableName() override;

    virtual void SAL_CALL setPivotTableName(const OUString& sPivotTableName) override;

    virtual sal_Bool SAL_CALL hasPivotTable() override;

    virtual css::uno::Reference<css::chart2::data::XDataSequence> SAL_CALL
        createDataSequenceOfValuesByIndex(sal_Int32 nIndex) override;
    virtual css::uno::Reference<css::chart2::data::XDataSequence> SAL_CALL
        createDataSequenceOfLabelsByIndex(sal_Int32 nIndex) override;
    virtual css::uno::Reference<css::chart2::data::XDataSequence> SAL_CALL
        createDataSequenceOfCategories() override;

    virtual OUString SAL_CALL getFieldOutputDescription(sal_Int32 nPageFieldIndex) override;

    // XPropertySet
    virtual css::uno::Reference<css::beans::XPropertySetInfo> SAL_CALL getPropertySetInfo() override;

    virtual void SAL_CALL
        setPropertyValue(const OUString& rPropertyName, const css::uno::Any& rValue) override;

    virtual css::uno::Any SAL_CALL
        getPropertyValue(const OUString& rPropertyName) override;

    virtual void SAL_CALL addPropertyChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference<css::beans::XPropertyChangeListener>& xListener) override;

    virtual void SAL_CALL removePropertyChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference<css::beans::XPropertyChangeListener>& rListener) override;

    virtual void SAL_CALL addVetoableChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference<css::beans::XVetoableChangeListener>& rListener) override;

    virtual void SAL_CALL removeVetoableChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference<css::beans::XVetoableChangeListener>& rListener) override;

    // XModifyBroadcaster
    virtual void SAL_CALL
        addModifyListener(const css::uno::Reference<css::util::XModifyListener>& aListener) override;

    virtual void SAL_CALL
        removeModifyListener(const css::uno::Reference<css::util::XModifyListener>& aListener) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;

    virtual sal_Bool SAL_CALL supportsService(const OUString& rServiceName) override;

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

private:

    css::uno::Reference<css::chart2::data::XDataSource>
        createValuesDataSource();

    css::uno::Reference<css::chart2::data::XDataSource>
        createCategoriesDataSource(bool bOrientationIsColumn);

    css::uno::Reference<css::chart2::data::XLabeledDataSequence> newLabeledDataSequence();

    css::uno::Reference<css::chart2::data::XDataSequence> assignLabelsToDataSequence(size_t nIndex);

    css::uno::Reference<css::chart2::data::XDataSequence> assignValuesToDataSequence(size_t nIndex);

    css::uno::Reference<css::chart2::data::XDataSequence> assignFirstCategoriesToDataSequence();

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

    css::uno::Reference<css::uno::XComponentContext> m_xContext;

    std::vector<css::uno::Reference<css::util::XModifyListener>> m_aValueListeners;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
