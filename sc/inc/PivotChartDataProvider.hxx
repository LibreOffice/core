/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_SC_INC_PIVOTCHARTDATAPROVIDER_HXX
#define INCLUDED_SC_INC_PIVOTCHARTDATAPROVIDER_HXX

#include "cellsuno.hxx"
#include "externalrefmgr.hxx"
#include "types.hxx"

#include <com/sun/star/chart2/data/XDataProvider.hpp>
#include <com/sun/star/chart2/data/XDataSource.hpp>
#include <com/sun/star/chart2/data/XDataSequence.hpp>
#include <com/sun/star/chart2/data/XLabeledDataSequence.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>

#include <svl/lstner.hxx>
#include <cppuhelper/implbase.hxx>
#include <rtl/ustring.hxx>
#include <svl/itemprop.hxx>

#include "dpsave.hxx"

#include <memory>
#include <vector>

namespace sc
{

class PivotChartItem;

typedef cppu::WeakImplHelper<css::chart2::data::XDataProvider,
                             css::beans::XPropertySet,
                             css::lang::XServiceInfo,
                             css::util::XModifyBroadcaster>
            PivotChartDataProvider_Base;

class PivotChartDataProvider : public PivotChartDataProvider_Base, public SfxListener
{
public:

    explicit PivotChartDataProvider(ScDocument* pDoc, OUString const& sPivotTableName);
    virtual ~PivotChartDataProvider() override;
    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override;

    // XDataProvider
    virtual sal_Bool SAL_CALL createDataSourcePossible(const css::uno::Sequence<css::beans::PropertyValue>& aArguments) override;

    virtual css::uno::Reference<
        css::chart2::data::XDataSource > SAL_CALL createDataSource(
            const css::uno::Sequence< css::beans::PropertyValue >& aArguments ) override;

    virtual css::uno::Sequence<
        css::beans::PropertyValue > SAL_CALL detectArguments(
            const css::uno::Reference< css::chart2::data::XDataSource >& xDataSource ) override;

    virtual sal_Bool SAL_CALL createDataSequenceByRangeRepresentationPossible(
        const OUString& aRangeRepresentation ) override;

    virtual css::uno::Reference<
        css::chart2::data::XDataSequence > SAL_CALL createDataSequenceByRangeRepresentation(
            const OUString& aRangeRepresentation ) override;

    virtual css::uno::Reference<css::chart2::data::XDataSequence> SAL_CALL
        createDataSequenceByValueArray( const OUString& aRole, const OUString& aRangeRepresentation ) override;

    virtual css::uno::Reference< css::sheet::XRangeSelection > SAL_CALL getRangeSelection() override;

    // XPropertySet
    virtual css::uno::Reference<css::beans::XPropertySetInfo> SAL_CALL getPropertySetInfo() override;

    virtual void SAL_CALL setPropertyValue(
            const OUString& rPropertyName,
            const css::uno::Any& rValue) override;

    virtual css::uno::Any SAL_CALL getPropertyValue(
            const OUString& rPropertyName) override;

    virtual void SAL_CALL addPropertyChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference< css::beans::XPropertyChangeListener>& xListener) override;

    virtual void SAL_CALL removePropertyChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference< css::beans::XPropertyChangeListener>& rListener) override;

    virtual void SAL_CALL addVetoableChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference< css::beans::XVetoableChangeListener>& rListener) override;

    virtual void SAL_CALL removeVetoableChangeListener(
            const OUString& rPropertyName,
            const css::uno::Reference< css::beans::XVetoableChangeListener>& rListener) override;

    // XModifyBroadcaster
    virtual void SAL_CALL addModifyListener(
        const css::uno::Reference<css::util::XModifyListener>& aListener) override;

    virtual void SAL_CALL removeModifyListener(
        const css::uno::Reference<css::util::XModifyListener>& aListener) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;

    virtual sal_Bool SAL_CALL supportsService(const OUString& rServiceName) override;

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

private:

    css::uno::Reference<css::chart2::data::XDataSource>
        createPivotChartDataSource(OUString const & aRangeRepresentation);
    css::uno::Reference<css::chart2::data::XDataSource>
        createPivotChartCategoriesDataSource(OUString const & aRangeRepresentation, bool bOrientCol);

    css::uno::Reference<css::chart2::data::XLabeledDataSequence>
        createLabeledDataSequence(css::uno::Reference<css::uno::XComponentContext>& rContext);

    void setLabeledDataSequenceValues(css::uno::Reference<css::chart2::data::XLabeledDataSequence> & xResult,
                                      OUString const & sRoleValues, OUString const & sIdValues,
                                      std::vector<PivotChartItem> const & rValues);

    void setLabeledDataSequence(css::uno::Reference<css::chart2::data::XLabeledDataSequence> & xResult,
                                OUString const & sRoleValues, OUString const & sIdValues,
                                std::vector<PivotChartItem> const & rValues,
                                OUString const & sRoleLabel,  OUString const & sIdLabel,
                                std::vector<PivotChartItem> const & rLabel);
    void createCategories(
        ScDPSaveData* pSaveData, bool bOrientCol,
        css::uno::Reference<css::uno::XComponentContext>& rContext,
        std::vector<css::uno::Reference<css::chart2::data::XLabeledDataSequence>>& rOutLabeledSequences);

    ScDocument*        m_pDocument;
    OUString           m_sPivotTableName;
    SfxItemPropertySet m_aPropSet;
    bool               m_bIncludeHiddenCells;

    std::vector<std::vector<PivotChartItem>> m_aCategoriesColumnOrientation;
    std::vector<std::vector<PivotChartItem>> m_aCategoriesRowOrientation;

    std::vector<css::uno::Reference<css::util::XModifyListener>> m_aValueListeners;
};

}

#endif // INCLUDED_SC_INC_PIVOTCHARTDATAPROVIDER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
