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
#ifndef SW_VBA_DOCUMENTS_HXX
#define SW_VBA_DOCUMENTS_HXX


#include <vbahelper/vbacollectionimpl.hxx>
#include <ooo/vba/word/XDocuments.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <vbahelper/vbadocumentsbase.hxx>
#include "wordvbahelper.hxx"


typedef cppu::ImplInheritanceHelper1< VbaDocumentsBase, ov::word::XDocuments > SwVbaDocuments_BASE;

class SwVbaDocuments : public SwVbaDocuments_BASE
{
public:
    SwVbaDocuments( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext );
    virtual ~SwVbaDocuments() {}

    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException);

    // SwVbaDocuments_BASE
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource );
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();

    // Methods
    virtual css::uno::Any SAL_CALL Add( const css::uno::Any& Template, const css::uno::Any& NewTemplate, const css::uno::Any& DocumentType, const css::uno::Any& Visible ) throw (css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Open( const ::rtl::OUString& Filename, const css::uno::Any& ConfirmConversions, const css::uno::Any& ReadOnly, const css::uno::Any& AddToRecentFiles, const css::uno::Any& PasswordDocument, const css::uno::Any& PasswordTemplate, const css::uno::Any& Revert, const css::uno::Any& WritePasswordDocument, const css::uno::Any& WritePasswordTemplate, const css::uno::Any& Format, const css::uno::Any& Encoding, const css::uno::Any& Visible, const css::uno::Any& OpenAndRepair, const css::uno::Any& DocumentDirection, const css::uno::Any& NoEncodingDialog, const css::uno::Any& XMLTransform ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL Close( const css::uno::Any& SaveChanges, const css::uno::Any& OriginalFormat, const css::uno::Any& RouteDocument ) throw (css::uno::RuntimeException);
};

#endif /* SW_VBA_DOCUMENTS_HXX */
