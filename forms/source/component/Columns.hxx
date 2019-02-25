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

#ifndef INCLUDED_FORMS_SOURCE_COMPONENT_COLUMNS_HXX
#define INCLUDED_FORMS_SOURCE_COMPONENT_COLUMNS_HXX

#include <cloneable.hxx>
#include <frm_strings.hxx>

#include <com/sun/star/io/XObjectInputStream.hpp>
#include <com/sun/star/io/XObjectOutputStream.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/XCloneable.hpp>

#include <comphelper/propagg.hxx>
#include <comphelper/proparrhlp.hxx>
#include <comphelper/uno3.hxx>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase2.hxx>
#include <cppuhelper/component.hxx>

using namespace comphelper;


namespace frm
{

typedef ::cppu::WeakAggComponentImplHelper2 <   css::lang::XUnoTunnel
                                            ,   css::util::XCloneable > OGridColumn_BASE;
class OGridColumn   :public ::cppu::BaseMutex
                    ,public OGridColumn_BASE
                    ,public OPropertySetAggregationHelper
                    ,public OCloneableAggregation
{
// [properties]
    css::uno::Any  m_aWidth;  // column width
    css::uno::Any  m_aAlign;  // column alignment
    css::uno::Any  m_aHidden; // column hidden?
// [properties]

    OUString       m_aModelName;

// [properties]
    OUString       m_aLabel;      // Column name
// [properties]

public:
    OGridColumn(const css::uno::Reference<css::uno::XComponentContext>& _rContext, const OUString& _sModelName);
    explicit OGridColumn(const OGridColumn* _pOriginal );
    virtual ~OGridColumn() override;

    // UNO binding
    DECLARE_UNO3_AGG_DEFAULTS(OGridControlModel, OGridColumn_BASE)
    virtual css::uno::Any SAL_CALL queryAggregation( const css::uno::Type& _rType ) override;

    static const css::uno::Sequence<sal_Int8>& getUnoTunnelImplementationId();
    // XUnoTunnel
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence<sal_Int8>& _rIdentifier) override;

    // XTypeProvider
    virtual css::uno::Sequence<sal_Int8>           SAL_CALL getImplementationId() override;
    virtual css::uno::Sequence< css::uno::Type>   SAL_CALL getTypes() override;

    // OComponentHelper
    virtual void SAL_CALL disposing() override;

    // XEventListener
    virtual void SAL_CALL disposing(const css::lang::EventObject& _rSource) override;

    // XPersistObject
    void write(const css::uno::Reference< css::io::XObjectOutputStream>& _rxOutStream);
    void read(const css::uno::Reference< css::io::XObjectInputStream>& _rxInStream);

    // XPropertySet
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override = 0;
    virtual void SAL_CALL getFastPropertyValue(css::uno::Any& rValue, sal_Int32 nHandle ) const override;
    virtual sal_Bool SAL_CALL convertFastPropertyValue(css::uno::Any& rConvertedValue, css::uno::Any& rOldValue,
                                          sal_Int32 nHandle, const css::uno::Any& rValue ) override;
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const css::uno::Any& rValue) override;

    using OPropertySetAggregationHelper::getFastPropertyValue;

    // css::beans::XPropertyState
    virtual css::uno::Any getPropertyDefaultByHandle( sal_Int32 nHandle ) const override;

    // XCloneable
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone(  ) override;

    const OUString& getModelName() const { return m_aModelName; }

protected:
    static void clearAggregateProperties(css::uno::Sequence< css::beans::Property>& seqProps, bool bAllowDropDown);
    static void setOwnProperties(css::uno::Sequence< css::beans::Property>& seqProps);

    virtual OGridColumn* createCloneColumn() const = 0;
};

#define DECL_COLUMN(ClassName)                                                                              \
class ClassName                                                                                             \
    :public OGridColumn                                                                                     \
    ,public OAggregationArrayUsageHelper< ClassName >                                                       \
{                                                                                                           \
public:                                                                                                     \
    explicit ClassName(const css::uno::Reference<css::uno::XComponentContext>& _rContext );                                            \
    explicit ClassName(const ClassName* _pCloneFrom);                                                                \
                                                                                                            \
    virtual css::uno::Reference< css::beans::XPropertySetInfo> SAL_CALL getPropertySetInfo() override;  \
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;                                         \
                                                                                                            \
    virtual void fillProperties(                                                                            \
        css::uno::Sequence< css::beans::Property >& /* [out] */ _rProps,          \
        css::uno::Sequence< css::beans::Property >& /* [out] */ _rAggregateProps  \
        ) const override;                                                                                            \
                                                                                                            \
    virtual OGridColumn* createCloneColumn() const override;                                                         \
};


#define IMPL_COLUMN(ClassName, Model, bAllowDropDown)                               \
    ClassName::ClassName( const css::uno::Reference<css::uno::XComponentContext>& _rContext ) \
    :OGridColumn(_rContext, Model) \
{ \
} \
ClassName::ClassName( const ClassName* _pCloneFrom ) \
    :OGridColumn( _pCloneFrom ) \
{ \
} \
css::uno::Reference< css::beans::XPropertySetInfo>  ClassName::getPropertySetInfo() \
{ \
    css::uno::Reference< css::beans::XPropertySetInfo>  xInfo( createPropertySetInfo( getInfoHelper() ) ); \
    return xInfo; \
} \
::cppu::IPropertyArrayHelper& ClassName::getInfoHelper() \
{ \
    return *getArrayHelper(); \
} \
void ClassName::fillProperties( \
    css::uno::Sequence< css::beans::Property >& /* [out] */ _rProps, \
    css::uno::Sequence< css::beans::Property >& /* [out] */ _rAggregateProps \
    ) const \
{ \
    if (m_xAggregateSet.is()) \
    { \
        _rAggregateProps = m_xAggregateSet->getPropertySetInfo()->getProperties(); \
        clearAggregateProperties(_rAggregateProps, bAllowDropDown); \
        setOwnProperties(_rProps); \
    } \
} \
OGridColumn* ClassName::createCloneColumn() const \
{ \
    return new ClassName( this ); \
} \
 \
// column type ids
#define TYPE_CHECKBOX       0
#define TYPE_COMBOBOX       1
#define TYPE_CURRENCYFIELD  2
#define TYPE_DATEFIELD      3
#define TYPE_FORMATTEDFIELD 4
#define TYPE_LISTBOX        5
#define TYPE_NUMERICFIELD   6
#define TYPE_PATTERNFIELD   7
#define TYPE_TEXTFIELD      8
#define TYPE_TIMEFIELD      9

// List of all known columns
const css::uno::Sequence<OUString>& getColumnTypes();
sal_Int32 getColumnTypeByModelName(const OUString& aModelName);

// Columns
DECL_COLUMN(TextFieldColumn)
DECL_COLUMN(PatternFieldColumn)
DECL_COLUMN(DateFieldColumn)
DECL_COLUMN(TimeFieldColumn)
DECL_COLUMN(NumericFieldColumn)
DECL_COLUMN(CurrencyFieldColumn)
DECL_COLUMN(CheckBoxColumn)
DECL_COLUMN(ComboBoxColumn)
DECL_COLUMN(ListBoxColumn)
DECL_COLUMN(FormattedFieldColumn)

}   // namespace frm

#endif // INCLUDED_FORMS_SOURCE_COMPONENT_COLUMNS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
