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
#include <com/sun/star/lang/NoSupportException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/style/VerticalAlignment.hpp>
#include <com/sun/star/text/XTextColumns.hpp>
#include <com/sun/star/uno/Any.h>

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <svl/itemprop.hxx>
#include <tools/color.hxx>
#include <vcl/svapp.hxx>

#include "SvxXTextColumns.hxx"

namespace
{
// Modelled after SwXTextColumns
class SvxXTextColumns final
    : public cppu::WeakImplHelper<css::beans::XPropertySet, css::text::XTextColumns,
                                  css::lang::XServiceInfo>
{
public:
    SvxXTextColumns(sal_Int16 nColumns, sal_Int32 nSpacing);

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
    sal_Int32 implGetReference() const
    {
        return (std::numeric_limits<sal_Int32>::max() / mnColumns) * mnColumns;
    }
    sal_Int16 mnColumns;
    sal_Int32 mnSpacing;
};

SvxXTextColumns::SvxXTextColumns(sal_Int16 nColumns, sal_Int32 nSpacing)
    : mnColumns(nColumns)
    , mnSpacing(nSpacing)
{
    if (nColumns <= 0 || nSpacing < 0)
        throw css::lang::IllegalArgumentException();
}

// XTextColumns

sal_Int32 SvxXTextColumns::getReferenceValue()
{
    SolarMutexGuard aGuard;
    return implGetReference();
}

sal_Int16 SvxXTextColumns::getColumnCount()
{
    SolarMutexGuard aGuard;
    return mnColumns;
}

void SvxXTextColumns::setColumnCount(sal_Int16 nColumns)
{
    if (nColumns <= 0)
        throw css::lang::IllegalArgumentException();
    SolarMutexGuard aGuard;
    mnColumns = nColumns;
}

css::uno::Sequence<css::text::TextColumn> SvxXTextColumns::getColumns()
{
    SolarMutexGuard aGuard;
    css::uno::Sequence<css::text::TextColumn> aTextColumns(mnColumns);
    css::text::TextColumn* pCols = aTextColumns.getArray();
    const sal_Int32 nWidth = std::numeric_limits<sal_Int32>::max() / mnColumns;
    const sal_Int32 nReference = nWidth * mnColumns;
    const sal_Int32 nDist = mnSpacing / 2;
    for (sal_Int16 i = 0; i < mnColumns; ++i)
    {
        pCols[i].Width = nWidth;
        pCols[i].LeftMargin = i == 0 ? 0 : nDist;
        pCols[i].RightMargin = i == mnColumns - 1 ? 0 : nDist;
    }
    return aTextColumns;
}

void SvxXTextColumns::setColumns(const css::uno::Sequence<css::text::TextColumn>& /*rColumns*/)
{
    throw css::lang::NoSupportException();
}

// XPropertySet

// The same set of properties that SwXTextColumns supports. Setting most of them is not supported.
enum : sal_uInt16
{
    WID_TXTCOL_LINE_WIDTH,
    WID_TXTCOL_LINE_COLOR,
    WID_TXTCOL_LINE_REL_HGT,
    WID_TXTCOL_LINE_ALIGN,
    WID_TXTCOL_LINE_IS_ON,
    WID_TXTCOL_IS_AUTOMATIC,
    WID_TXTCOL_AUTO_DISTANCE,
    WID_TXTCOL_LINE_STYLE,
};

SfxItemPropertyMapEntry const saTextColumns_Impl[] = {
    { u"SeparatorLineWidth", WID_TXTCOL_LINE_WIDTH, cppu::UnoType<sal_Int32>::get(), 0, 0 },
    { u"SeparatorLineColor", WID_TXTCOL_LINE_COLOR, cppu::UnoType<sal_Int32>::get(), 0, 0 },
    { u"SeparatorLineRelativeHeight", WID_TXTCOL_LINE_REL_HGT, cppu::UnoType<sal_Int32>::get(), 0,
      0 },
    { u"SeparatorLineVerticalAlignment", WID_TXTCOL_LINE_ALIGN,
      cppu::UnoType<css::style::VerticalAlignment>::get(), 0, 0 },
    { u"SeparatorLineIsOn", WID_TXTCOL_LINE_IS_ON, cppu::UnoType<bool>::get(), 0, 0 },
    { u"IsAutomatic", WID_TXTCOL_IS_AUTOMATIC, cppu::UnoType<bool>::get(),
      css::beans::PropertyAttribute::READONLY, 0 },
    { u"AutomaticDistance", WID_TXTCOL_AUTO_DISTANCE, cppu::UnoType<sal_Int32>::get(), 0, 0 },
    { u"SeparatorLineStyle", WID_TXTCOL_LINE_STYLE, cppu::UnoType<sal_Int8>::get(), 0, 0 },
    { u"", 0, css::uno::Type(), 0, 0 },
};

const SfxItemPropertySet saPropSet(saTextColumns_Impl);

css::uno::Reference<css::beans::XPropertySetInfo> SvxXTextColumns::getPropertySetInfo()
{
    static const css::uno::Reference<css::beans::XPropertySetInfo> aRef
        = saPropSet.getPropertySetInfo();
    return aRef;
}

void SvxXTextColumns::setPropertyValue(const OUString& rPropertyName, const css::uno::Any& aValue)
{
    const SfxItemPropertyMapEntry* pEntry = saPropSet.getPropertyMap().getByName(rPropertyName);
    if (!pEntry)
        throw css::beans::UnknownPropertyException("Unknown property: " + rPropertyName,
                                                   static_cast<cppu::OWeakObject*>(this));
    if (pEntry->nFlags & css::beans::PropertyAttribute::READONLY)
        throw css::beans::PropertyVetoException("Property is read-only: " + rPropertyName,
                                                static_cast<cppu::OWeakObject*>(this));

    switch (pEntry->nWID)
    {
        case WID_TXTCOL_LINE_WIDTH:
        case WID_TXTCOL_LINE_COLOR:
        case WID_TXTCOL_LINE_STYLE:
        case WID_TXTCOL_LINE_REL_HGT:
        case WID_TXTCOL_LINE_ALIGN:
        case WID_TXTCOL_LINE_IS_ON:
            throw css::lang::NoSupportException();
        case WID_TXTCOL_AUTO_DISTANCE:
        {
            sal_Int32 nTmp = 0;
            if (!(aValue >>= nTmp) || nTmp < 0 || nTmp >= implGetReference())
                throw css::lang::IllegalArgumentException();
            SolarMutexGuard aGuard;
            mnSpacing = nTmp;
        }
        break;
    }
}

css::uno::Any SvxXTextColumns::getPropertyValue(const OUString& rPropertyName)
{
    const SfxItemPropertyMapEntry* pEntry = saPropSet.getPropertyMap().getByName(rPropertyName);
    if (!pEntry)
        throw css::beans::UnknownPropertyException("Unknown property: " + rPropertyName,
                                                   static_cast<cppu::OWeakObject*>(this));

    css::uno::Any aRet;
    switch (pEntry->nWID)
    {
        case WID_TXTCOL_LINE_WIDTH:
            aRet <<= sal_Int32(0);
            break;
        case WID_TXTCOL_LINE_COLOR:
            aRet <<= COL_BLACK;
            break;
        case WID_TXTCOL_LINE_STYLE:
            aRet <<= sal_Int8(0); // none
            break;
        case WID_TXTCOL_LINE_REL_HGT:
            aRet <<= sal_Int8(100);
            break;
        case WID_TXTCOL_LINE_ALIGN:
            aRet <<= css::style::VerticalAlignment_MIDDLE;
            break;
        case WID_TXTCOL_LINE_IS_ON:
            aRet <<= false;
            break;
        case WID_TXTCOL_IS_AUTOMATIC:
            aRet <<= true;
            break;
        case WID_TXTCOL_AUTO_DISTANCE:
            aRet <<= mnSpacing;
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

// XServiceInfo

OUString SvxXTextColumns::getImplementationName() { return "SvxXTextColumns"; }

sal_Bool SvxXTextColumns::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

css::uno::Sequence<OUString> SvxXTextColumns::getSupportedServiceNames()
{
    return { "com.sun.star.text.TextColumns" };
}
}

css::uno::Reference<css::uno::XInterface>
SvxXTextColumns_createInstance(sal_Int16 nColumns, sal_Int32 nSpacing) noexcept
{
    return static_cast<cppu::OWeakObject*>(new SvxXTextColumns(nColumns, nSpacing));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
