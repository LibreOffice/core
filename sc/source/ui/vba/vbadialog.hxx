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
#ifndef SC_VBA_DIALOG_HXX
#define SC_VBA_DIALOG_HXX

#include <cppuhelper/implbase1.hxx>
#include <ooo/vba/excel/XDialog.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <vbahelper/vbadialogbase.hxx>

typedef cppu::ImplInheritanceHelper1< VbaDialogBase, ov::excel::XDialog > ScVbaDialog_BASE;

class ScVbaDialog : public ScVbaDialog_BASE
{
public:
    ScVbaDialog( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > xContext, const css::uno::Reference< css::frame::XModel >& xModel, sal_Int32 nIndex ):ScVbaDialog_BASE( xParent, xContext, xModel, nIndex ) {}
    virtual ~ScVbaDialog() {}

    // Methods
    virtual OUString mapIndexToName( sal_Int32 nIndex );
    // XHelperInterface
    virtual OUString getServiceImplName();
    virtual css::uno::Sequence<OUString> getServiceNames();

};

#endif /* SC_VBA_DIALOG_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
