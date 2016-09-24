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

#ifndef INCLUDED_VBAHELPER_SOURCE_MSFORMS_VBATEXTBOX_HXX
#define INCLUDED_VBAHELPER_SOURCE_MSFORMS_VBATEXTBOX_HXX

#include <cppuhelper/implbase.hxx>
#include <ooo/vba/msforms/XTextBox.hpp>
#include "vbacontrol.hxx"
#include <vbahelper/vbahelper.hxx>

typedef cppu::ImplInheritanceHelper< ScVbaControl, ov::msforms::XTextBox, css::script::XDefaultProperty > TextBoxImpl_BASE;

class ScVbaTextBox : public TextBoxImpl_BASE
{
    bool mbDialog;
public:
    ScVbaTextBox( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::uno::XInterface >& xControl, const css::uno::Reference< css::frame::XModel >& xModel, ov::AbstractGeometryAttributes* pGeomHelper, bool bDialog = false );
   // Attributes
    virtual css::uno::Any SAL_CALL getValue() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setValue( const css::uno::Any& _value ) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getText() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setText( const OUString& _text ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getMaxLength() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setMaxLength( sal_Int32 _maxlength ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getMultiline() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setMultiline( sal_Bool _multiline ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getSpecialEffect() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setSpecialEffect( sal_Int32 nSpecialEffect ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getBorderStyle() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setBorderStyle( sal_Int32 nBorderStyle ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getTextLength() throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< ov::msforms::XNewFont > SAL_CALL getFont() throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getBackColor() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setBackColor( sal_Int32 nBackColor ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getAutoSize() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setAutoSize( sal_Bool bAutoSize ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getLocked() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setLocked( sal_Bool bAutoSize ) throw (css::uno::RuntimeException, std::exception) override;

    // XDefaultProperty
    OUString SAL_CALL getDefaultPropertyName(  ) throw (css::uno::RuntimeException, std::exception) override { return OUString("Value"); }
    //XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};

#endif // INCLUDED_VBAHELPER_SOURCE_MSFORMS_VBATEXTBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
