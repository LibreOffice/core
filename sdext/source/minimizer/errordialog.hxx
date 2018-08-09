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

#ifndef INCLUDED_SDEXT_SOURCE_MINIMIZER_ERRORDIALOG_HXX
#define INCLUDED_SDEXT_SOURCE_MINIMIZER_ERRORDIALOG_HXX

#include "unodialog.hxx"
#include "configurationaccess.hxx"
#include "pppoptimizertoken.hxx"
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/awt/XItemListener.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/awt/XItemEventBroadcaster.hpp>
#include <com/sun/star/awt/PushButtonType.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <cppuhelper/implbase.hxx>

class ErrorDialog : public UnoDialog, public ConfigurationAccess
{
public:
    ErrorDialog(const css::uno::Reference<css::uno::XComponentContext>& rxContext,
                css::uno::Reference<css::frame::XFrame> const& rxFrame, const OUString& rText);
    ~ErrorDialog();

private:
    css::uno::Reference<css::awt::XActionListener> mxActionListener;

    void InitDialog();

    const OUString& maText;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
