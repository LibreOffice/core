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
#ifndef INCLUDED_VBAHELPER_SOURCE_MSFORMS_VBALISTBOX_HXX
#define INCLUDED_VBAHELPER_SOURCE_MSFORMS_VBALISTBOX_HXX
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
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
    std::unique_ptr< ListControlHelper > mpListHelper;

    sal_Int16 m_nIndex;

public:
    ScVbaListBox( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::uno::XInterface >& xControl, const css::uno::Reference< css::frame::XModel >& xModel, ov::AbstractGeometryAttributes* pGeomHelper );

    // Attributes
    virtual css::uno::Any SAL_CALL getListIndex() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setListIndex( const css::uno::Any& _value ) throw (css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL getListCount() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getValue() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setValue( const css::uno::Any& _value ) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getText() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setText( const OUString& _text ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getMultiSelect() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setMultiSelect( sal_Int32 _multiselect ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< ov::msforms::XNewFont > SAL_CALL getFont() throw (css::uno::RuntimeException, std::exception) override;

    // Methods
    virtual css::uno::Any SAL_CALL Selected( ::sal_Int32 index ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL AddItem( const css::uno::Any& pvargItem, const css::uno::Any& pvargIndex ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeItem( const css::uno::Any& index ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL Clear(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL List( const css::uno::Any& pvargIndex, const css::uno::Any& pvarColumn ) throw (css::uno::RuntimeException, std::exception) override;
    // XControl
    virtual void SAL_CALL setRowSource( const OUString& _rowsource ) throw (css::uno::RuntimeException, std::exception) override;

    // XDefaultProperty
    OUString SAL_CALL getDefaultPropertyName(  ) throw (css::uno::RuntimeException, std::exception) override { return OUString("Value"); }

    //XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;

    //PropListener
    virtual void setValueEvent( const css::uno::Any& value ) override;
    virtual css::uno::Any getValueEvent() override;


};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
