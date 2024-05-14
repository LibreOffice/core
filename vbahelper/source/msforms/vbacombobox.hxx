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
#ifndef INCLUDED_VBAHELPER_SOURCE_MSFORMS_VBACOMBOBOX_HXX
#define INCLUDED_VBAHELPER_SOURCE_MSFORMS_VBACOMBOBOX_HXX
#include <memory>
#include <optional>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/script/XDefaultProperty.hpp>
#include <ooo/vba/msforms/XComboBox.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

#include "vbacontrol.hxx"
#include "vbalistcontrolhelper.hxx"
#include <vbahelper/vbahelper.hxx>

typedef cppu::ImplInheritanceHelper<ScVbaControl, ov::msforms::XComboBox, css::script::XDefaultProperty > ComboBoxImpl_BASE;
class ScVbaComboBox : public ComboBoxImpl_BASE
{
    ListControlHelper maListHelper;
    OUString sSourceName;

public:
    ScVbaComboBox( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::uno::XInterface >& xControl, const css::uno::Reference< css::frame::XModel >& xModel, std::unique_ptr<ov::AbstractGeometryAttributes> pGeomHelper );

    // Attributes
    virtual css::uno::Any SAL_CALL getListIndex() override;
    virtual ::sal_Int32 SAL_CALL getListCount() override;
    virtual void SAL_CALL setListIndex( const css::uno::Any& _value ) override;
    virtual css::uno::Any SAL_CALL getValue() override;
    virtual void SAL_CALL setValue( const css::uno::Any& _value ) override;
    virtual OUString SAL_CALL getText() override;
    virtual void SAL_CALL setText( const OUString& _text ) override;
    virtual sal_Int32 SAL_CALL getStyle() override;
    virtual void SAL_CALL setStyle( sal_Int32 nStyle ) override;
    virtual sal_Int32 SAL_CALL getDropButtonStyle() override;
    virtual void SAL_CALL setDropButtonStyle( sal_Int32 nDropButtonStyle ) override;
    virtual sal_Int32 SAL_CALL getDragBehavior() override;
    virtual void SAL_CALL setDragBehavior( sal_Int32 nDragBehavior ) override;
    virtual sal_Int32 SAL_CALL getEnterFieldBehavior() override;
    virtual void SAL_CALL setEnterFieldBehavior( sal_Int32 nEnterFieldBehavior ) override;
    virtual sal_Int32 SAL_CALL getListStyle() override;
    virtual void SAL_CALL setListStyle( sal_Int32 nListStyle ) override;
    virtual sal_Int32 SAL_CALL getTextAlign() override;
    virtual void SAL_CALL setTextAlign( sal_Int32 nTextAlign ) override;
    virtual sal_Int32 SAL_CALL getTextLength() override;
    virtual css::uno::Reference< ov::msforms::XNewFont > SAL_CALL getFont() override;
    virtual sal_Int32 SAL_CALL getBackColor() override;
    virtual void SAL_CALL setBackColor( sal_Int32 nBackColor ) override;
    virtual sal_Bool SAL_CALL getAutoSize() override;
    virtual void SAL_CALL setAutoSize( sal_Bool bAutoSize ) override;
    virtual sal_Bool SAL_CALL getLocked() override;
    virtual void SAL_CALL setLocked( sal_Bool bAutoSize ) override;
    virtual OUString SAL_CALL getLinkedCell() override;
    virtual void SAL_CALL setLinkedCell( const OUString& _linkedcell ) override;

    // Methods
    virtual void SAL_CALL AddItem( const css::uno::Any& pvargItem, const css::uno::Any& pvargIndex ) override;
    virtual void SAL_CALL removeItem( const css::uno::Any& index ) override;
    virtual void SAL_CALL Clear(  ) override;
    virtual css::uno::Any SAL_CALL List( const css::uno::Any& pvargIndex, const css::uno::Any& pvarColumn ) override;
    // XControl
    virtual void SAL_CALL setRowSource( const OUString& _rowsource ) override;

    // XDefaultProperty
        OUString SAL_CALL getDefaultPropertyName(  ) override { return u"Value"_ustr; }
    //XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
