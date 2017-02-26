/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include "PivotChartDataSequence.hxx"

#include <sal/config.h>

#include "miscuno.hxx"
#include "document.hxx"
#include "docsh.hxx"
#include "hints.hxx"

#include <com/sun/star/chart/ChartDataChangeEvent.hpp>

using namespace css;

namespace sc
{

SC_SIMPLE_SERVICE_INFO( PivotChartDataSequence, "PivotChartDataSequence", "com.sun.star.chart2.data.DataSequence")

const SfxItemPropertyMapEntry* lcl_GetDataSequencePropertyMap()
{
    static const SfxItemPropertyMapEntry aDataSequencePropertyMap_Impl[] =
    {
        { OUString(SC_UNONAME_HIDDENVALUES), 0, cppu::UnoType<uno::Sequence<sal_Int32>>::get(), 0, 0 },
        { OUString(SC_UNONAME_ROLE), 0, cppu::UnoType<css::chart2::data::DataSequenceRole>::get(), 0, 0 },
        { OUString(SC_UNONAME_INCLUDEHIDDENCELLS), 0, cppu::UnoType<bool>::get(), 0, 0 },
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    return aDataSequencePropertyMap_Impl;
}

PivotChartDataSequence::PivotChartDataSequence(ScDocument* pDocument, OUString const & sPivotTableName, OUString const & sID, std::vector<PivotChartItem> const & rColumnData)
    : m_pDocument(pDocument)
    , m_sPivotTableName(sPivotTableName)
    , m_aID(sID)
    , m_aColumnData(rColumnData)
    , m_aShortSideLabels()
    , m_aLongSideLabels()
    , m_aPropSet(lcl_GetDataSequencePropertyMap())
{
    if (m_pDocument)
        m_pDocument->AddUnoObject(*this);
}

PivotChartDataSequence::~PivotChartDataSequence()
{
    SolarMutexGuard g;

    if (m_pDocument)
        m_pDocument->RemoveUnoObject(*this);
}

void PivotChartDataSequence::Notify(SfxBroadcaster& /*rBC*/, const SfxHint& rHint)
{
    if (rHint.GetId() == SfxHintId::Dying)
    {
        m_pDocument = nullptr;
    }
}

uno::Sequence<uno::Any> SAL_CALL PivotChartDataSequence::getData()
{
    SolarMutexGuard aGuard;

    if (!m_pDocument)
        throw uno::RuntimeException();

    uno::Sequence<uno::Any> aSeq(m_aColumnData.size());

    size_t i = 0;
    for (PivotChartItem const & rItem : m_aColumnData)
    {
        if (rItem.m_bIsValue)
            aSeq[i] = uno::makeAny<double>(rItem.m_fValue);
        else
            aSeq[i] = uno::makeAny<OUString>(rItem.m_aString);
        i++;
    }
    return aSeq;
}

// XNumericalDataSequence --------------------------------------------------

uno::Sequence<double> SAL_CALL PivotChartDataSequence::getNumericalData()
{
    SolarMutexGuard aGuard;
    if (!m_pDocument)
        throw uno::RuntimeException();

    uno::Sequence<double> aSeq(m_aColumnData.size());

    size_t i = 0;
    for (PivotChartItem const & rItem : m_aColumnData)
    {
        aSeq[i] = rItem.m_fValue;
        i++;
    }
    return aSeq;
}

// XTextualDataSequence --------------------------------------------------

uno::Sequence<OUString> SAL_CALL PivotChartDataSequence::getTextualData()
{
    SolarMutexGuard aGuard;
    if (!m_pDocument)
        throw uno::RuntimeException();

    uno::Sequence<OUString> aSeq(m_aColumnData.size());

    size_t i = 0;
    for (PivotChartItem const & rItem : m_aColumnData)
    {
        if (!rItem.m_bIsValue)
            aSeq[i] = rItem.m_aString;
        i++;
    }
    return aSeq;
}

OUString SAL_CALL PivotChartDataSequence::getSourceRangeRepresentation()
{
    SolarMutexGuard aGuard;

    return m_aID;
}

uno::Sequence<OUString> SAL_CALL PivotChartDataSequence::generateLabel(chart2::data::LabelOrigin eOrigin)
{
    SolarMutexGuard aGuard;
    if (!m_pDocument)
        throw uno::RuntimeException();

    if (eOrigin == chart2::data::LabelOrigin_SHORT_SIDE)
        return m_aShortSideLabels;
    else if (eOrigin == chart2::data::LabelOrigin_LONG_SIDE)
        return m_aLongSideLabels;

    uno::Sequence<OUString> aSeq;
    return aSeq;
}

sal_Int32 SAL_CALL PivotChartDataSequence::getNumberFormatKeyByIndex(sal_Int32 /*nIndex*/)
{
    SolarMutexGuard aGuard;
    return 0;
}

// XCloneable ================================================================

uno::Reference<util::XCloneable> SAL_CALL PivotChartDataSequence::createClone()
{
    SolarMutexGuard aGuard;

    std::unique_ptr<PivotChartDataSequence> pClone;
    pClone.reset(new PivotChartDataSequence(m_pDocument, m_sPivotTableName, m_aID, m_aColumnData));
    pClone->setRole(m_aRole);
    pClone->setShortSideLabels(m_aShortSideLabels);
    pClone->setLongSideLabels(m_aLongSideLabels);

    uno::Reference<util::XCloneable> xClone(pClone.release());

    return xClone;
}

// XModifyBroadcaster ========================================================

void SAL_CALL PivotChartDataSequence::addModifyListener( const uno::Reference< util::XModifyListener >& aListener )
{
    SolarMutexGuard aGuard;
    m_aValueListeners.push_back(uno::Reference<util::XModifyListener>(aListener));
}

void SAL_CALL PivotChartDataSequence::removeModifyListener( const uno::Reference< util::XModifyListener >& aListener )
{
    SolarMutexGuard aGuard;

    sal_uInt16 nCount = m_aValueListeners.size();
    for (sal_uInt16 n = nCount; n--; )
    {
        uno::Reference<util::XModifyListener>& rObj = m_aValueListeners[n];
        if (rObj == aListener)
        {
            m_aValueListeners.erase(m_aValueListeners.begin() + n);
        }
    }
}

// DataSequence XPropertySet -------------------------------------------------

uno::Reference< beans::XPropertySetInfo> SAL_CALL PivotChartDataSequence::getPropertySetInfo()
{
    SolarMutexGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef = new SfxItemPropertySetInfo( m_aPropSet.getPropertyMap() );
    return aRef;
}

void SAL_CALL PivotChartDataSequence::setPropertyValue(
        const OUString& rPropertyName, const uno::Any& rValue)
{
    if ( rPropertyName == SC_UNONAME_ROLE )
    {
        if ( !(rValue >>= m_aRole))
            throw lang::IllegalArgumentException();
    }
    else if (rPropertyName == SC_UNONAME_INCLUDEHIDDENCELLS
          || rPropertyName == SC_UNONAME_HIDDENVALUES
          || rPropertyName == SC_UNONAME_TIME_BASED
          || rPropertyName == SC_UNONAME_HAS_STRING_LABEL)
    {}
    else
        throw beans::UnknownPropertyException();
}

uno::Any SAL_CALL PivotChartDataSequence::getPropertyValue(const OUString& rPropertyName)
{
    uno::Any aRet;
    if ( rPropertyName == SC_UNONAME_ROLE )
        aRet <<= m_aRole;
    else if ( rPropertyName == SC_UNONAME_INCLUDEHIDDENCELLS )
        aRet <<= false;
    else if ( rPropertyName == SC_UNONAME_HIDDENVALUES )
    {
        css::uno::Sequence<sal_Int32> aHiddenValues;
        aRet <<= aHiddenValues;
    }
    else if (rPropertyName == SC_UNONAME_TIME_BASED)
    {
        aRet <<= false;
    }
    else if (rPropertyName == SC_UNONAME_HAS_STRING_LABEL)
    {
        aRet <<= false;
    }
    else
        throw beans::UnknownPropertyException();
    return aRet;
}

void SAL_CALL PivotChartDataSequence::addPropertyChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener>& /*xListener*/)
{
    OSL_FAIL( "Not yet implemented" );
}

void SAL_CALL PivotChartDataSequence::removePropertyChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener>& /*rListener*/)
{
    OSL_FAIL( "Not yet implemented" );
}

void SAL_CALL PivotChartDataSequence::addVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener>& /*rListener*/)
{
    OSL_FAIL( "Not yet implemented" );
}

void SAL_CALL PivotChartDataSequence::removeVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener>& /*rListener*/)
{
    OSL_FAIL( "Not yet implemented" );
}

} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
