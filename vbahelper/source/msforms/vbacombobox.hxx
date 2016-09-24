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
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/script/XDefaultProperty.hpp>
#include <ooo/vba/msforms/XComboBox.hpp>
#include <comphelper/proparrhlp.hxx>
#include <comphelper/propertycontainer.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>

#include "vbacontrol.hxx"
#include "vbalistcontrolhelper.hxx"
#include <vbahelper/vbahelper.hxx>

typedef cppu::ImplInheritanceHelper<ScVbaControl, ov::msforms::XComboBox, css::script::XDefaultProperty > ComboBoxImpl_BASE;
class ScVbaComboBox : public ComboBoxImpl_BASE
{
    std::unique_ptr< ListControlHelper > mpListHelper;
    OUString sSourceName;

public:
    ScVbaComboBox( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::uno::XInterface >& xControl, const css::uno::Reference< css::frame::XModel >& xModel, ov::AbstractGeometryAttributes* pGeomHelper );

    // Attributes
    virtual css::uno::Any SAL_CALL getListIndex() throw (css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int32 SAL_CALL getListCount() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setListIndex( const css::uno::Any& _value ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getValue() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setValue( const css::uno::Any& _value ) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getText() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setText( const OUString& _text ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getStyle() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setStyle( sal_Int32 nStyle ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getDropButtonStyle() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDropButtonStyle( sal_Int32 nDropButtonStyle ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getDragBehavior() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDragBehavior( sal_Int32 nDragBehavior ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getEnterFieldBehavior() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setEnterFieldBehavior( sal_Int32 nEnterFieldBehavior ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getListStyle() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setListStyle( sal_Int32 nListStyle ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getTextAlign() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setTextAlign( sal_Int32 nTextAlign ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getTextLength() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< ov::msforms::XNewFont > SAL_CALL getFont() throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getBackColor() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setBackColor( sal_Int32 nBackColor ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getAutoSize() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setAutoSize( sal_Bool bAutoSize ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getLocked() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setLocked( sal_Bool bAutoSize ) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getLinkedCell() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setLinkedCell( const OUString& _linkedcell ) throw (css::uno::RuntimeException, std::exception) override;

    // Methods
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
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
