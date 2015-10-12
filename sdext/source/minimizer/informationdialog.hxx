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

#ifndef INCLUDED_SDEXT_SOURCE_MINIMIZER_INFORMATIONDIALOG_HXX
#define INCLUDED_SDEXT_SOURCE_MINIMIZER_INFORMATIONDIALOG_HXX

#include "unodialog.hxx"
#include "configurationaccess.hxx"
#include "pppoptimizertoken.hxx"
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/awt/XItemListener.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/drawing/XShapes.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/awt/XItemEventBroadcaster.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/awt/PushButtonType.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <cppuhelper/implbase.hxx>


// - InformationDialog -

class InformationDialog : public UnoDialog, public ConfigurationAccess
{
public:

    InformationDialog( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
            css::uno::Reference< css::frame::XFrame >& rxFrame, const OUString& rSaveAsURL,
                bool& bOpenNewDocument, const sal_Int64& nSourceSize, const sal_Int64& nDestSize, const sal_Int64& nApproxDest );
    ~InformationDialog();

    bool                execute();

private:

    css::uno::Reference< css::frame::XFrame >         mxFrame;
    css::uno::Reference< css::io::XStream >           mxTempFile;

    css::uno::Reference< css::awt::XActionListener >  mxActionListener;

    void InitDialog();

    sal_Int64 mnSourceSize;
    sal_Int64 mnDestSize;
    sal_Int64 mnApproxSize;
    bool& mrbOpenNewDocument;
    const OUString& maSaveAsURL;
};

class OKActionListener : public ::cppu::WeakImplHelper< css::awt::XActionListener >
{
public:
    explicit OKActionListener( InformationDialog& rInformationDialog ) : mrInformationDialog( rInformationDialog ){}

    virtual void SAL_CALL actionPerformed( const css::awt::ActionEvent& Event ) throw ( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw ( css::uno::RuntimeException, std::exception) override;
private:

    InformationDialog& mrInformationDialog;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
