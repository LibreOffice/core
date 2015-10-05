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
#ifndef INCLUDED_SW_SOURCE_UI_VBA_VBACHECKBOX_HXX
#define INCLUDED_SW_SOURCE_UI_VBA_VBACHECKBOX_HXX

#include <ooo/vba/word/XCheckBox.hpp>
#include <com/sun/star/text/XFormField.hpp>
#include <vbahelper/vbahelperinterface.hxx>

typedef InheritedHelperInterfaceWeakImpl< ooo::vba::word::XCheckBox > SwVbaCheckBox_BASE;

class SwVbaCheckBox : public SwVbaCheckBox_BASE
{
private:
    css::uno::Reference< css::frame::XModel > mxModel;
    css::uno::Reference< css::text::XFormField > mxFormField;

public:
    SwVbaCheckBox( const css::uno::Reference< ooo::vba::XHelperInterface >& rParent, const css::uno::Reference< css::uno::XComponentContext >& rContext, const css::uno::Reference< css::frame::XModel >& rModel, const css::uno::Reference< css::text::XFormField >& xFormField ) throw ( css::uno::RuntimeException );
    virtual ~SwVbaCheckBox();

    // Methods
    sal_Bool SAL_CALL getValue() throw ( css::uno::RuntimeException );
    void SAL_CALL setValue( sal_Bool value ) throw ( css::uno::RuntimeException );

    // XHelperInterface
    virtual OUString getServiceImplName();
    virtual css::uno::Sequence<OUString> getServiceNames();
};
#endif // INCLUDED_SW_SOURCE_UI_VBA_VBACHECKBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
