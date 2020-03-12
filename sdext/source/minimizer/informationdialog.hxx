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
#include <com/sun/star/awt/XItemListener.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <cppuhelper/implbase.hxx>

OUString InsertFixedText( UnoDialog& rInformationDialog, const OUString& rControlName, const OUString& rLabel,
                                sal_Int32 nXPos, sal_Int32 nYPos, sal_Int32 nWidth, sal_Int32 nHeight, bool bMultiLine, sal_Int16 nTabIndex );

OUString InsertImage( UnoDialog& rInformationDialog, const OUString& rControlName, const OUString& rURL,
    sal_Int32 nPosX, sal_Int32 nPosY, sal_Int32 nWidth, sal_Int32 nHeight, bool bScale );

OUString InsertCheckBox( UnoDialog& rInformationDialog, const OUString& rControlName,
    const css::uno::Reference< css::awt::XItemListener >& rItemListener, const OUString& rLabel,
    sal_Int32 nXPos, sal_Int32 nYPos, sal_Int32 nWidth, sal_Int16 nTabIndex );

OUString InsertButton( UnoDialog& rInformationDialog, const OUString& rControlName,
    css::uno::Reference< css::awt::XActionListener > const & xActionListener, sal_Int32 nXPos, sal_Int32 nYPos,
    sal_Int32 nWidth, sal_Int16 nTabIndex, const OUString& rText );

class InformationDialog : public UnoDialog, public ConfigurationAccess
{
public:

    InformationDialog( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                       css::uno::Reference< css::frame::XFrame > const & rxFrame, const OUString& rSaveAsURL,
                       bool& bOpenNewDocument, sal_Int64 nSourceSize, sal_Int64 nDestSize, sal_Int64 nApproxDest );
    ~InformationDialog();

    void                execute();

private:

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
    explicit OKActionListener( UnoDialog& rDialog ) : mrDialog( rDialog ){}

    virtual void SAL_CALL actionPerformed( const css::awt::ActionEvent& Event ) override;
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;
private:

    UnoDialog& mrDialog;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
