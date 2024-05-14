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
#ifndef INCLUDED_VBAHELPER_SOURCE_MSFORMS_VBACHECKBOX_HXX
#define INCLUDED_VBAHELPER_SOURCE_MSFORMS_VBACHECKBOX_HXX
#include <cppuhelper/implbase.hxx>
#include <ooo/vba/msforms/XCheckBox.hpp>
#include <com/sun/star/script/XDefaultProperty.hpp>

#include "vbacontrol.hxx"
#include <vbahelper/vbahelper.hxx>

typedef cppu::ImplInheritanceHelper< ScVbaControl, ov::msforms::XCheckBox, css::script::XDefaultProperty > CheckBoxImpl_BASE;

class ScVbaCheckbox : public CheckBoxImpl_BASE
{
public:
    ScVbaCheckbox(  const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::uno::XInterface >& xControl, const css::uno::Reference< css::frame::XModel >& xModel, std::unique_ptr<ov::AbstractGeometryAttributes> pGeomHelper );
   // Attributes
    virtual OUString SAL_CALL getCaption() override;
    virtual void SAL_CALL setCaption( const OUString& _caption ) override;
    virtual css::uno::Any SAL_CALL getValue() override;
    virtual void SAL_CALL setValue( const css::uno::Any& _value ) override;
    virtual css::uno::Reference< ov::msforms::XNewFont > SAL_CALL getFont() override;
    virtual sal_Int32 SAL_CALL getBackColor() override;
    virtual void SAL_CALL setBackColor( sal_Int32 nBackColor ) override;
    virtual sal_Bool SAL_CALL getAutoSize() override;
    virtual void SAL_CALL setAutoSize( sal_Bool bAutoSize ) override;
    virtual sal_Bool SAL_CALL getLocked() override;
    virtual void SAL_CALL setLocked( sal_Bool bAutoSize ) override;
    // XDefaultProperty
    OUString SAL_CALL getDefaultPropertyName(  ) override { return u"Value"_ustr; }
    //XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};

#endif // INCLUDED_VBAHELPER_SOURCE_MSFORMS_VBACHECKBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
