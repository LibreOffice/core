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
#ifndef INCLUDED_SW_SOURCE_UI_VBA_VBAFIELD_HXX
#define INCLUDED_SW_SOURCE_UI_VBA_VBAFIELD_HXX
#include <vbahelper/vbahelperinterface.hxx>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <ooo/vba/word/XField.hpp>
#include <ooo/vba/word/XFields.hpp>
#include <vbahelper/vbacollectionimpl.hxx>

typedef InheritedHelperInterfaceWeakImpl< ooo::vba::word::XField > SwVbaField_BASE;

class SwVbaField : public SwVbaField_BASE
{
    css::uno::Reference< css::text::XTextDocument > mxTextDocument;
    css::uno::Reference< css::text::XTextField > mxTextField;
public:
    SwVbaField( const css::uno::Reference< ooo::vba::XHelperInterface >& rParent, const css::uno::Reference< css::uno::XComponentContext >& rContext, const css::uno::Reference< css::text::XTextDocument >& rDocument, const css::uno::Reference< css::text::XTextField >& xTextField) throw ( css::uno::RuntimeException);

    virtual sal_Bool SAL_CALL Update() throw ( css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    // XHelperInterface
    virtual OUString getServiceImplName() SAL_OVERRIDE;
    virtual css::uno::Sequence<OUString> getServiceNames() SAL_OVERRIDE;
};

typedef CollTestImplHelper< ov::word::XFields > SwVbaFields_BASE;

class SwVbaFields : public SwVbaFields_BASE
{
    css::uno::Reference< css::frame::XModel > mxModel;
    css::uno::Reference< css::lang::XMultiServiceFactory > mxMSF;
private:
    css::uno::Reference< css::text::XTextField > Create_Field_FileName(const OUString& rText)
        throw (css::uno::RuntimeException, css::script::BasicErrorException);
    css::uno::Reference< css::text::XTextField > Create_Field_DocProperty( const OUString& _text ) throw (css::uno::RuntimeException);

public:
    SwVbaFields( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext > & xContext, const css::uno::Reference< css::frame::XModel >& xModel );
    // XFields
    virtual css::uno::Reference< ::ooo::vba::word::XField > SAL_CALL Add( const css::uno::Reference< ::ooo::vba::word::XRange >& Range, const css::uno::Any& Type, const css::uno::Any& Text, const css::uno::Any& PreserveFormatting ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual sal_Int32 SAL_CALL Update() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException) SAL_OVERRIDE;
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException) SAL_OVERRIDE;
    // ScVbaCollectionBaseImpl
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource ) SAL_OVERRIDE;

    // XHelperInterface
    virtual OUString getServiceImplName() SAL_OVERRIDE;
    virtual css::uno::Sequence<OUString> getServiceNames() SAL_OVERRIDE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
