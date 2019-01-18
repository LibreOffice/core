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
#ifndef INCLUDED_SW_SOURCE_UI_VBA_VBADOCUMENTS_HXX
#define INCLUDED_SW_SOURCE_UI_VBA_VBADOCUMENTS_HXX

#include <vbahelper/vbacollectionimpl.hxx>
#include <ooo/vba/word/XDocuments.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <vbahelper/vbadocumentsbase.hxx>
#include <cppuhelper/implbase.hxx>
#include "wordvbahelper.hxx"

typedef cppu::ImplInheritanceHelper< VbaDocumentsBase, ov::word::XDocuments > SwVbaDocuments_BASE;

class SwVbaDocuments : public SwVbaDocuments_BASE
{
public:
    SwVbaDocuments( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext );

    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() override;

    // SwVbaDocuments_BASE
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource ) override;
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;

    // Methods
    virtual css::uno::Any SAL_CALL Add( const css::uno::Any& Template, const css::uno::Any& NewTemplate, const css::uno::Any& DocumentType, const css::uno::Any& Visible ) override;
    virtual css::uno::Any SAL_CALL Open( const OUString& Filename, const css::uno::Any& ConfirmConversions, const css::uno::Any& ReadOnly, const css::uno::Any& AddToRecentFiles, const css::uno::Any& PasswordDocument, const css::uno::Any& PasswordTemplate, const css::uno::Any& Revert, const css::uno::Any& WritePasswordDocument, const css::uno::Any& WritePasswordTemplate, const css::uno::Any& Format, const css::uno::Any& Encoding, const css::uno::Any& Visible, const css::uno::Any& OpenAndRepair, const css::uno::Any& DocumentDirection, const css::uno::Any& NoEncodingDialog, const css::uno::Any& XMLTransform ) override;
    virtual css::uno::Any SAL_CALL OpenNoRepairDialog( const OUString& Filename, const css::uno::Any& ConfirmConversions, const css::uno::Any& ReadOnly, const css::uno::Any& AddToRecentFiles, const css::uno::Any& PasswordDocument, const css::uno::Any& PasswordTemplate, const css::uno::Any& Revert, const css::uno::Any& WritePasswordDocument, const css::uno::Any& WritePasswordTemplate, const css::uno::Any& Format, const css::uno::Any& Encoding, const css::uno::Any& Visible, const css::uno::Any& OpenAndRepair, const css::uno::Any& DocumentDirection, const css::uno::Any& NoEncodingDialog, const css::uno::Any& XMLTransform ) override;
    virtual css::uno::Any SAL_CALL OpenOld( const OUString& FileName, const css::uno::Any& ConfirmConversions, const css::uno::Any& ReadOnly, const css::uno::Any& AddToRecentFiles, const css::uno::Any& PasswordDocument, const css::uno::Any& PasswordTemplate, const css::uno::Any& Revert, const css::uno::Any& WritePasswordDocument, const css::uno::Any& WritePasswordTemplate, const css::uno::Any& Format ) override;
    virtual void SAL_CALL Close( const css::uno::Any& SaveChanges, const css::uno::Any& OriginalFormat, const css::uno::Any& RouteDocument ) override;
};

#endif // INCLUDED_SW_SOURCE_UI_VBA_VBADOCUMENTS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
