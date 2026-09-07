/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
#ifndef INCLUDED_VBAHELPER_SOURCE_MSFORMS_VBALISTBOX_HXX
#define INCLUDED_VBAHELPER_SOURCE_MSFORMS_VBALISTBOX_HXX
#include <memory>
#include <optional>
#include <cppuhelper/implbase.hxx>
#include <cpo/uno/XComponentContext.hpp>
#include <com/sun/star/script/XDefaultProperty.hpp>
#include <ooo/vba/msforms/XListBox.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

#include "vbacontrol.hxx"
#include <vbahelper/vbapropvalue.hxx>
#include "vbalistcontrolhelper.hxx"
#include <vbahelper/vbahelper.hxx>

typedef cppu::ImplInheritanceHelper<ScVbaControl, ov::msforms::XListBox, css::script::XDefaultProperty > ListBoxImpl_BASE;
class ScVbaListBox : public ListBoxImpl_BASE
    ,public PropListener
{
    ListControlHelper maListHelper;

    sal_Int16 m_nIndex;

public:
    ScVbaListBox( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< cpo::uno::XComponentContext >& xContext, const css::uno::Reference< cpo::uno::XInterface >& xControl, const css::uno::Reference< css::frame::XModel >& xModel, std::unique_ptr<ov::AbstractGeometryAttributes> pGeomHelper );

    // Attributes
    virtual cpo::uno::Any getListIndex() override;
    virtual void setListIndex( const cpo::uno::Any& _value ) override;
    virtual ::sal_Int32 getListCount() override;
    virtual cpo::uno::Any getValue() override;
    virtual void setValue( const cpo::uno::Any& _value ) override;
    virtual OUString getText() override;
    virtual void setText( const OUString& _text ) override;
    virtual sal_Int32 getMultiSelect() override;
    virtual void setMultiSelect( sal_Int32 _multiselect ) override;
    virtual css::uno::Reference< ov::msforms::XNewFont > getFont() override;

    // Methods
    virtual cpo::uno::Any Selected( ::sal_Int32 index ) override;
    virtual void AddItem( const cpo::uno::Any& pvargItem, const cpo::uno::Any& pvargIndex ) override;
    virtual void removeItem( const cpo::uno::Any& index ) override;
    virtual void Clear(  ) override;
    virtual cpo::uno::Any List( const cpo::uno::Any& pvargIndex, const cpo::uno::Any& pvarColumn ) override;
    // XControl
    virtual void setRowSource( const OUString& _rowsource ) override;

    // XDefaultProperty
    OUString getDefaultPropertyName(  ) override { return u"Value"_ustr; }

    //XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual cpo::uno::Sequence<OUString> getServiceNames() override;

    //PropListener
    virtual void setValueEvent( const cpo::uno::Any& value ) override;
    virtual cpo::uno::Any getValueEvent() override;


};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
