/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <sal/config.h>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyVetoException.hpp>
#include <com/sun/star/beans/UnknownPropertyException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/style/VerticalAlignment.hpp>
#include <com/sun/star/text/ColumnSeparatorStyle.hpp>
#include <com/sun/star/text/XTextColumns.hpp>
#include <com/sun/star/uno/Any.h>
#include <com/sun/star/util/Color.hpp>

#include <cppuhelper/supportsservice.hxx>
#include <o3tl/safeint.hxx>
#include <svl/itemprop.hxx>
#include <svx/SvxXTextColumns.hxx>
#include <vcl/svapp.hxx>

#include <numeric>

namespace
{
enum : sal_uInt16
{
    WID_TXTCOL_IS_AUTOMATIC,
    WID_TXTCOL_AUTO_DISTANCE,
    WID_TXTCOL_LINE_WIDTH,
    WID_TXTCOL_LINE_COLOR,
    WID_TXTCOL_LINE_REL_HGT,
    WID_TXTCOL_LINE_ALIGN,
    WID_TXTCOL_LINE_IS_ON,
    WID_TXTCOL_LINE_STYLE,
};

SfxItemPropertyMapEntry const saTextColumns_Impl[] = {
    { u"IsAutomatic"_ustr, WID_TXTCOL_IS_AUTOMATIC, cppu::UnoType<bool>::get(),
      css::beans::PropertyAttribute::READONLY, 0 },
    { u"AutomaticDistance"_ustr, WID_TXTCOL_AUTO_DISTANCE, cppu::UnoType<sal_Int32>::get(), 0, 0 },
    { u"SeparatorLineWidth"_ustr, WID_TXTCOL_LINE_WIDTH, cppu::UnoType<sal_Int32>::get(), 0, 0 },
    { u"SeparatorLineColor"_ustr, WID_TXTCOL_LINE_COLOR,
      cppu::UnoType<com::sun::star::util::Color>::get(), 0, 0 },
    { u"SeparatorLineRelativeHeight"_ustr, WID_TXTCOL_LINE_REL_HGT, cppu::UnoType<sal_Int32>::get(),
      0, 0 },
    { u"SeparatorLineVerticalAlignment"_ustr, WID_TXTCOL_LINE_ALIGN,
      cppu::UnoType<css::style::VerticalAlignment>::get(), 0, 0 },
    { u"SeparatorLineIsOn"_ustr, WID_TXTCOL_LINE_IS_ON, cppu::UnoType<bool>::get(), 0, 0 },
    { u"SeparatorLineStyle"_ustr, WID_TXTCOL_LINE_STYLE, cppu::UnoType<sal_Int16>::get(), 0, 0 },
};

class SvxXTextColumns final
    : public cppu::WeakImplHelper<css::beans::XPropertySet, css::text::XTextColumns,
                                  css::lang::XServiceInfo>
{
public:
    SvxXTextColumns() = default;

    // XTextColumns
    virtual sal_Int32 SAL_CALL getReferenceValue() override;
    virtual sal_Int16 SAL_CALL getColumnCount() override;
    virtual void SAL_CALL setColumnCount(sal_Int16 nColumns) override;
    virtual css::uno::Sequence<css::text::TextColumn> SAL_CALL getColumns() override;
    virtual void SAL_CALL
    setColumns(const css::uno::Sequence<css::text::TextColumn>& Columns) override;

    // XPropertySet
    virtual css::uno::Reference<css::beans::XPropertySetInfo>
        SAL_CALL getPropertySetInfo() override;
    virtual void SAL_CALL setPropertyValue(const OUString& aPropertyName,
                                           const css::uno::Any& aValue) override;
    virtual css::uno::Any SAL_CALL getPropertyValue(const OUString& PropertyName) override;
    virtual void SAL_CALL addPropertyChangeListener(
        const OUString& aPropertyName,
        const css::uno::Reference<css::beans::XPropertyChangeListener>& xListener) override;
    virtual void SAL_CALL removePropertyChangeListener(
        const OUString& aPropertyName,
        const css::uno::Reference<css::beans::XPropertyChangeListener>& aListener) override;
    virtual void SAL_CALL addVetoableChangeListener(
        const OUString& PropertyName,
        const css::uno::Reference<css::beans::XVetoableChangeListener>& aListener) override;
    virtual void SAL_CALL removeVetoableChangeListener(
        const OUString& PropertyName,
        const css::uno::Reference<css::beans::XVetoableChangeListener>& aListener) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override;

private:
    sal_Int32 m_nReference = USHRT_MAX;
    css::uno::Sequence<css::text::TextColumn> m_aTextColumns;
    bool m_bIsAutomaticWidth = true;
    sal_Int32 m_nAutoDistance = 0;

    const SfxItemPropertySet m_aPropSet = { saTextColumns_Impl };

    //separator line
    sal_Int32 m_nSepLineWidth = 0;
    com::sun::star::util::Color m_nSepLineColor = 0; // black
    sal_Int32 m_nSepLineHeightRelative = 100; // full height
    css::style::VerticalAlignment m_nSepLineVertAlign = css::style::VerticalAlignment_MIDDLE;
    bool m_bSepLineIsOn = false;
    sal_Int16 m_nSepLineStyle = css::text::ColumnSeparatorStyle::NONE;
};

OUString SvxXTextColumns::getImplementationName()
{
    return u"com.sun.star.comp.svx.TextColumns"_ustr;
}

sal_Bool SvxXTextColumns::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence<OUString> SvxXTextColumns::getSupportedServiceNames()
{
    return { u"com.sun.star.text.TextColumns"_ustr };
}

sal_Int32 SvxXTextColumns::getReferenceValue()
{
    SolarMutexGuard aGuard;
    return m_nReference;
}

sal_Int16 SvxXTextColumns::getColumnCount()
{
    SolarMutexGuard aGuard;
    return o3tl::narrowing<sal_Int16>(m_aTextColumns.getLength());
}

void SvxXTextColumns::setColumnCount(sal_Int16 nColumns)
{
    SolarMutexGuard aGuard;
    if (nColumns <= 0)
        throw css::uno::RuntimeException();
    m_bIsAutomaticWidth = true;
    m_aTextColumns.realloc(nColumns);
    css::text::TextColumn* pCols = m_aTextColumns.getArray();
    m_nReference = USHRT_MAX;
    sal_Int32 nWidth = m_nReference / nColumns;
    sal_Int32 nDiff = m_nReference - nWidth * nColumns;
    sal_Int32 nDist = m_nAutoDistance / 2;
    for (sal_Int16 i = 0; i < nColumns; i++)
    {
        pCols[i].Width = nWidth;
        pCols[i].LeftMargin = i == 0 ? 0 : nDist;
        pCols[i].RightMargin = i == nColumns - 1 ? 0 : nDist;
    }
    pCols[nColumns - 1].Width += nDiff;
}

css::uno::Sequence<css::text::TextColumn> SvxXTextColumns::getColumns()
{
    SolarMutexGuard aGuard;
    return m_aTextColumns;
}

void SvxXTextColumns::setColumns(const css::uno::Sequence<css::text::TextColumn>& rColumns)
{
    SolarMutexGuard aGuard;
    sal_Int32 nReferenceTemp = std::accumulate(
        rColumns.begin(), rColumns.end(), sal_Int32(0),
        [](const sal_Int32 nSum, const css::text::TextColumn& rCol) { return nSum + rCol.Width; });
    m_bIsAutomaticWidth = false;
    m_nReference = !nReferenceTemp ? USHRT_MAX : nReferenceTemp;
    m_aTextColumns = rColumns;
}

css::uno::Reference<css::beans::XPropertySetInfo> SvxXTextColumns::getPropertySetInfo()
{
    return m_aPropSet.getPropertySetInfo();
}

void SvxXTextColumns::setPropertyValue(const OUString& rPropertyName, const css::uno::Any& aValue)
{
    const SfxItemPropertyMapEntry* pEntry = m_aPropSet.getPropertyMap().getByName(rPropertyName);
    if (!pEntry)
        throw css::beans::UnknownPropertyException("Unknown property: " + rPropertyName,
                                                   getXWeak());
    if (pEntry->nFlags & css::beans::PropertyAttribute::READONLY)
        throw css::beans::PropertyVetoException("Property is read-only: " + rPropertyName,
                                                getXWeak());

    switch (pEntry->nWID)
    {
        case WID_TXTCOL_LINE_WIDTH:
        {
            sal_Int32 nTmp;
            if (!(aValue >>= nTmp) || nTmp < 0)
                throw css::lang::IllegalArgumentException();
            m_nSepLineWidth = nTmp;
            break;
        }
        case WID_TXTCOL_LINE_COLOR:
            if (!(aValue >>= m_nSepLineColor))
                throw css::lang::IllegalArgumentException();
            break;
        case WID_TXTCOL_LINE_STYLE:
            if (!(aValue >>= m_nSepLineStyle))
                throw css::lang::IllegalArgumentException();
            break;
        case WID_TXTCOL_LINE_REL_HGT:
        {
            sal_Int32 nTmp;
            if (!(aValue >>= nTmp) || nTmp < 0)
                throw css::lang::IllegalArgumentException();
            m_nSepLineHeightRelative = nTmp;
            break;
        }
        case WID_TXTCOL_LINE_ALIGN:
            if (css::style::VerticalAlignment eAlign; aValue >>= eAlign)
                m_nSepLineVertAlign = eAlign;
            else if (sal_Int8 nTmp; aValue >>= nTmp)
                m_nSepLineVertAlign = static_cast<css::style::VerticalAlignment>(nTmp);
            else
                throw css::lang::IllegalArgumentException();
            break;
        case WID_TXTCOL_LINE_IS_ON:
            if (!(aValue >>= m_bSepLineIsOn))
                throw css::lang::IllegalArgumentException();
            break;
        case WID_TXTCOL_AUTO_DISTANCE:
        {
            sal_Int32 nTmp;
            if (!(aValue >>= nTmp) || nTmp < 0 || nTmp >= m_nReference)
                throw css::lang::IllegalArgumentException();
            m_nAutoDistance = nTmp;
            sal_Int32 nColumns = m_aTextColumns.getLength();
            css::text::TextColumn* pCols = m_aTextColumns.getArray();
            sal_Int32 nDist = m_nAutoDistance / 2;
            for (sal_Int32 i = 0; i < nColumns; i++)
            {
                pCols[i].LeftMargin = i == 0 ? 0 : nDist;
                pCols[i].RightMargin = i == nColumns - 1 ? 0 : nDist;
            }
            break;
        }
    }
}

css::uno::Any SvxXTextColumns::getPropertyValue(const OUString& rPropertyName)
{
    const SfxItemPropertyMapEntry* pEntry = m_aPropSet.getPropertyMap().getByName(rPropertyName);
    if (!pEntry)
        throw css::beans::UnknownPropertyException("Unknown property: " + rPropertyName,
                                                   getXWeak());

    css::uno::Any aRet;
    switch (pEntry->nWID)
    {
        case WID_TXTCOL_LINE_WIDTH:
            aRet <<= m_nSepLineWidth;
            break;
        case WID_TXTCOL_LINE_COLOR:
            aRet <<= m_nSepLineColor;
            break;
        case WID_TXTCOL_LINE_STYLE:
            aRet <<= m_nSepLineStyle;
            break;
        case WID_TXTCOL_LINE_REL_HGT:
            aRet <<= m_nSepLineHeightRelative;
            break;
        case WID_TXTCOL_LINE_ALIGN:
            aRet <<= m_nSepLineVertAlign;
            break;
        case WID_TXTCOL_LINE_IS_ON:
            aRet <<= m_bSepLineIsOn;
            break;
        case WID_TXTCOL_IS_AUTOMATIC:
            aRet <<= m_bIsAutomaticWidth;
            break;
        case WID_TXTCOL_AUTO_DISTANCE:
            aRet <<= m_nAutoDistance;
            break;
    }
    return aRet;
}

void SvxXTextColumns::addPropertyChangeListener(
    const OUString& /*rPropertyName*/,
    const css::uno::Reference<css::beans::XPropertyChangeListener>& /*xListener*/)
{
}

void SvxXTextColumns::removePropertyChangeListener(
    const OUString& /*rPropertyName*/,
    const css::uno::Reference<css::beans::XPropertyChangeListener>& /*xListener*/)
{
}

void SvxXTextColumns::addVetoableChangeListener(
    const OUString& /*rPropertyName*/,
    const css::uno::Reference<css::beans::XVetoableChangeListener>& /*xListener*/)
{
}

void SvxXTextColumns::removeVetoableChangeListener(
    const OUString& /*rPropertyName*/,
    const css::uno::Reference<css::beans::XVetoableChangeListener>& /*xListener*/)
{
}
}

css::uno::Reference<css::uno::XInterface> SvxXTextColumns_createInstance() noexcept
{
    return getXWeak(new SvxXTextColumns);
}

extern "C" SVXCORE_DLLPUBLIC css::uno::XInterface*
com_sun_star_comp_svx_TextColumns_get_implementation(css::uno::XComponentContext*,
                                                     css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new SvxXTextColumns);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
