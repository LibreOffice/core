/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef VBA_WORKBOOKS_HXX
#define VBA_WORKBOOKS_HXX


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
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException) = 0;
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException) = 0;

    // VbaDocumentsBase_BASE
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource ) = 0;

protected:
    css::uno::Any createDocument() throw (css::uno::RuntimeException);
    void closeDocuments() throw (css::uno::RuntimeException);
    css::uno::Any openDocument( const ::rtl::OUString& Filename, const css::uno::Any& ReadOnly, const css::uno::Sequence< css::beans::PropertyValue >& rProps ) throw (css::uno::RuntimeException);
};

#endif /* SC_VBA_WORKBOOKS_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
