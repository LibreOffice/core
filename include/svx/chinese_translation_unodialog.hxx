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

#pragma once

#include <sal/config.h>
#include <svx/svxdllapi.h>

#include <memory>

#include <com/sun/star/awt/XWindow.hpp>
#include <cppuhelper/implbase.hxx>
#include <comphelper/interfacecontainer4.hxx>
#include <mutex>


namespace textconversiondlgs
{


/** This class provides the chinese translation dialog as a uno component.

It can be created via lang::XMultiComponentFactory::createInstanceWithContext
with servicename "com.sun.star.linguistic2.ChineseTranslationDialog"
or implementation name "com.sun.star.comp.linguistic2.ChineseTranslationDialog"

It can be initialized via the XInitialization interface with the following single parameter:
PropertyValue-Parameter: Name="ParentWindow" Type="awt::XWindow".

It can be executed via the ui::dialogs::XExecutableDialog interface.

Made settings can be retrieved via beans::XPropertySet interface.
Following properties are available (read only and not bound):
1) Name="IsDirectionToSimplified" Type="sal_Bool"
2) Name="IsTranslateCommonTerms" Type="sal_Bool"

The dialog gets this information from the registry on execute and writes it back to the registry if ended with OK.
*/

class ChineseTranslationDialog;

class TEXTCONVERSIONDLGS_DLLPUBLIC ChineseTranslation_UnoDialog final : public ::cppu::WeakImplHelper <>
{
public:
    ChineseTranslation_UnoDialog(const css::uno::Reference< css::awt::XWindow >& xParentWindow);
    virtual ~ChineseTranslation_UnoDialog() override;

    sal_Int16 execute();

    bool getIsDirectionToSimplified();
    bool getIsTranslateCommonTerms();

private:

    void impl_DeleteDialog();

private:
    css::uno::Reference< css::awt::XWindow > m_xParentWindow;

    std::unique_ptr<ChineseTranslationDialog> m_xDialog;
};


} //end namespace textconversiondlgs

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
