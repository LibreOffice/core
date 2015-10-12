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
#ifndef INCLUDED_VBAHELPER_VBADOCUMENTSBASE_HXX
#define INCLUDED_VBAHELPER_VBADOCUMENTSBASE_HXX


#include <vbahelper/vbacollectionimpl.hxx>
#include <ooo/vba/XDocumentsBase.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>

typedef CollTestImplHelper< ooo::vba::XDocumentsBase > VbaDocumentsBase_BASE;

class VBAHELPER_DLLPUBLIC VbaDocumentsBase : public VbaDocumentsBase_BASE
{
public:
    enum DOCUMENT_TYPE
    {
        WORD_DOCUMENT = 1,
        EXCEL_DOCUMENT
    };

private:
    DOCUMENT_TYPE meDocType;

public:
    VbaDocumentsBase( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, DOCUMENT_TYPE eDocType ) throw (css::uno::RuntimeException);
    virtual ~VbaDocumentsBase() {}

    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException) override = 0;
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException) override = 0;

    // VbaDocumentsBase_BASE
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource ) override = 0;

protected:
    css::uno::Any createDocument() throw (css::uno::RuntimeException, std::exception);
    css::uno::Any openDocument( const OUString& Filename, const css::uno::Any& ReadOnly, const css::uno::Sequence< css::beans::PropertyValue >& rProps ) throw (css::uno::RuntimeException);
};

#endif /* SC_ INCLUDED_VBAHELPER_VBADOCUMENTSBASE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
