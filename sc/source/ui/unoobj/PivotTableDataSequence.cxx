/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <PivotTableDataSequence.hxx>

#include <sal/config.h>
#include <sal/log.hxx>
#include <o3tl/safeint.hxx>
#include <osl/diagnose.h>
#include <vcl/svapp.hxx>

#include <miscuno.hxx>
#include <document.hxx>
#include <unonames.hxx>

using namespace css;

namespace sc
{

SC_SIMPLE_SERVICE_INFO( PivotTableDataSequence, "PivotTableDataSequence", "com.sun.star.chart2.data.DataSequence")

static const SfxItemPropertyMapEntry* lcl_GetDataSequencePropertyMap()
{
    static const SfxItemPropertyMapEntry aDataSequencePropertyMap_Impl[] =
    {
        { u"" SC_UNONAME_HIDDENVALUES, 0, cppu::UnoType<uno::Sequence<sal_Int32>>::get(), 0, 0 },
        { u"" SC_UNONAME_ROLE, 0, cppu::UnoType<css::chart2::data::DataSequenceRole>::get(), 0, 0 },
        { u"" SC_UNONAME_INCLUDEHIDDENCELLS, 0, cppu::UnoType<bool>::get(), 0, 0 },
        { u"", 0, css::uno::Type(), 0, 0 }
    };
    return aDataSequencePropertyMap_Impl;
}

PivotTableDataSequence::PivotTableDataSequence(ScDocument* pDocument, OUString const & sID,
                                               std::vector<ValueAndFormat> const & rData)
    : m_pDocument(pDocument)
    , m_aID(sID)
    , m_aData(rData)
    , m_aPropSet(lcl_GetDataSequencePropertyMap())
{
    if (m_pDocument)
        m_pDocument->AddUnoObject(*this);
}

PivotTableDataSequence::~PivotTableDataSequence()
{
    SolarMutexGuard g;

    if (m_pDocument)
        m_pDocument->RemoveUnoObject(*this);
}

void PivotTableDataSequence::Notify(SfxBroadcaster& /*rBC*/, const SfxHint& rHint)
{
    if (rHint.GetId() == SfxHintId::Dying)
    {
        m_pDocument = nullptr;
    }
}

uno::Sequence<uno::Any> SAL_CALL PivotTableDataSequence::getData()
{
    SolarMutexGuard aGuard;

    if (!m_pDocument)
        throw uno::RuntimeException();

    uno::Sequence<uno::Any> aSeq(m_aData.size());

    size_t i = 0;
    for (ValueAndFormat const & rItem : m_aData)
    {
        if (rItem.m_eType == ValueType::Numeric)
            aSeq[i] <<= double(rItem.m_fValue);
        else if (rItem.m_eType == ValueType::String)
            aSeq[i] <<= rItem.m_aString;

        i++;
    }
    return aSeq;
}

// XNumericalDataSequence --------------------------------------------------

uno::Sequence<double> SAL_CALL PivotTableDataSequence::getNumericalData()
{
    SolarMutexGuard aGuard;
    if (!m_pDocument)
        throw uno::RuntimeException();

    uno::Sequence<double> aSeq(m_aData.size());

    size_t i = 0;
    for (ValueAndFormat const & rItem : m_aData)
    {
        aSeq[i] = rItem.m_fValue;
        i++;
    }
    return aSeq;
}

// XTextualDataSequence --------------------------------------------------

uno::Sequence<OUString> SAL_CALL PivotTableDataSequence::getTextualData()
{
    SolarMutexGuard aGuard;
    if (!m_pDocument)
        throw uno::RuntimeException();

    uno::Sequence<OUString> aSeq(m_aData.size());

    size_t i = 0;
    for (ValueAndFormat const & rItem : m_aData)
    {
        if (rItem.m_eType == ValueType::String)
            aSeq[i] = rItem.m_aString;
        i++;
    }
    return aSeq;
}

OUString SAL_CALL PivotTableDataSequence::getSourceRangeRepresentation()
{
    SolarMutexGuard aGuard;

    return m_aID;
}

uno::Sequence<OUString> SAL_CALL PivotTableDataSequence::generateLabel(chart2::data::LabelOrigin /*eOrigin*/)
{
    SolarMutexGuard aGuard;
    if (!m_pDocument)
        throw uno::RuntimeException();

    uno::Sequence<OUString> aSeq;
    return aSeq;
}

sal_Int32 SAL_CALL PivotTableDataSequence::getNumberFormatKeyByIndex(sal_Int32 nIndex)
{
    SolarMutexGuard aGuard;
    if (nIndex == -1 && !m_aData.empty())
    {
        return m_aData[0].m_nNumberFormat;
    }
    else if (nIndex < 0 && o3tl::make_unsigned(nIndex) >= m_aData.size())
    {
        SAL_WARN("sc.ui", "Passed invalid index to getNumberFormatKeyByIndex(). Will return default value '0'.");
        return 0;
    }
    return m_aData[size_t(nIndex)].m_nNumberFormat;
}

// XCloneable ================================================================

uno::Reference<util::XCloneable> SAL_CALL PivotTableDataSequence::createClone()
{
    SolarMutexGuard aGuard;

    std::unique_ptr<PivotTableDataSequence> pClone;
    pClone.reset(new PivotTableDataSequence(m_pDocument, m_aID, m_aData));
    pClone->setRole(m_aRole);

    uno::Reference<util::XCloneable> xClone(pClone.release());

    return xClone;
}

// XModifyBroadcaster ========================================================

void SAL_CALL PivotTableDataSequence::addModifyListener(const uno::Reference<util::XModifyListener>& aListener)
{
    SolarMutexGuard aGuard;
    m_aValueListeners.emplace_back(aListener);
}

void SAL_CALL PivotTableDataSequence::removeModifyListener(const uno::Reference<util::XModifyListener>& aListener)
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

uno::Reference< beans::XPropertySetInfo> SAL_CALL PivotTableDataSequence::getPropertySetInfo()
{
    SolarMutexGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef = new SfxItemPropertySetInfo(m_aPropSet.getPropertyMap());
    return aRef;
}

void SAL_CALL PivotTableDataSequence::setPropertyValue(const OUString& rPropertyName, const uno::Any& rValue)
{
    if (rPropertyName == SC_UNONAME_ROLE)
    {
        if (!(rValue >>= m_aRole))
            throw lang::IllegalArgumentException();
    }
    else if (rPropertyName == SC_UNONAME_INCLUDEHIDDENCELLS
          || rPropertyName == SC_UNONAME_HIDDENVALUES
          || rPropertyName == SC_UNONAME_TIME_BASED
          || rPropertyName == SC_UNONAME_HAS_STRING_LABEL)
    {}
    else
        throw beans::UnknownPropertyException(rPropertyName);
}

uno::Any SAL_CALL PivotTableDataSequence::getPropertyValue(const OUString& rPropertyName)
{
    uno::Any aReturn;
    if (rPropertyName == SC_UNONAME_ROLE)
        aReturn <<= m_aRole;
    else if (rPropertyName == SC_UNONAME_INCLUDEHIDDENCELLS)
        aReturn <<= false;
    else if (rPropertyName == SC_UNONAME_HIDDENVALUES)
    {
        css::uno::Sequence<sal_Int32> aHiddenValues;
        aReturn <<= aHiddenValues;
    }
    else if (rPropertyName == SC_UNONAME_TIME_BASED)
    {
        aReturn <<= false;
    }
    else if (rPropertyName == SC_UNONAME_HAS_STRING_LABEL)
    {
        aReturn <<= false;
    }
    else
        throw beans::UnknownPropertyException(rPropertyName);
    return aReturn;
}

void SAL_CALL PivotTableDataSequence::addPropertyChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener>& /*xListener*/)
{
    OSL_FAIL("Not yet implemented");
}

void SAL_CALL PivotTableDataSequence::removePropertyChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XPropertyChangeListener>& /*rListener*/)
{
    OSL_FAIL("Not yet implemented");
}

void SAL_CALL PivotTableDataSequence::addVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener>& /*rListener*/)
{
    OSL_FAIL("Not yet implemented");
}

void SAL_CALL PivotTableDataSequence::removeVetoableChangeListener(
        const OUString& /*rPropertyName*/,
        const uno::Reference< beans::XVetoableChangeListener>& /*rListener*/)
{
    OSL_FAIL("Not yet implemented");
}

} // end sc namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
