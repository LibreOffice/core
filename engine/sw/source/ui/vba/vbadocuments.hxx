/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <ooo/vba/word/XDocuments.hpp>
#include <vbahelper/vbadocumentsbase.hxx>
#include <cppuhelper/implbase.hxx>

typedef cppu::ImplInheritanceHelper< VbaDocumentsBase, ov::word::XDocuments > SwVbaDocuments_BASE;

class SwVbaDocuments : public SwVbaDocuments_BASE
{
public:
    SwVbaDocuments( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext );

    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() override;

    // SwVbaDocuments_BASE
    virtual cpo::uno::Any createCollectionObject( const cpo::uno::Any& aSource ) override;
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;

    // Methods
    virtual cpo::uno::Any SAL_CALL Add( const cpo::uno::Any& Template, const cpo::uno::Any& NewTemplate, const cpo::uno::Any& DocumentType, const cpo::uno::Any& Visible ) override;
    virtual cpo::uno::Any SAL_CALL Open( const OUString& Filename, const cpo::uno::Any& ConfirmConversions, const cpo::uno::Any& ReadOnly, const cpo::uno::Any& AddToRecentFiles, const cpo::uno::Any& PasswordDocument, const cpo::uno::Any& PasswordTemplate, const cpo::uno::Any& Revert, const cpo::uno::Any& WritePasswordDocument, const cpo::uno::Any& WritePasswordTemplate, const cpo::uno::Any& Format, const cpo::uno::Any& Encoding, const cpo::uno::Any& Visible, const cpo::uno::Any& OpenAndRepair, const cpo::uno::Any& DocumentDirection, const cpo::uno::Any& NoEncodingDialog, const cpo::uno::Any& XMLTransform ) override;
    virtual cpo::uno::Any SAL_CALL OpenNoRepairDialog( const OUString& Filename, const cpo::uno::Any& ConfirmConversions, const cpo::uno::Any& ReadOnly, const cpo::uno::Any& AddToRecentFiles, const cpo::uno::Any& PasswordDocument, const cpo::uno::Any& PasswordTemplate, const cpo::uno::Any& Revert, const cpo::uno::Any& WritePasswordDocument, const cpo::uno::Any& WritePasswordTemplate, const cpo::uno::Any& Format, const cpo::uno::Any& Encoding, const cpo::uno::Any& Visible, const cpo::uno::Any& OpenAndRepair, const cpo::uno::Any& DocumentDirection, const cpo::uno::Any& NoEncodingDialog, const cpo::uno::Any& XMLTransform ) override;
    virtual cpo::uno::Any SAL_CALL OpenOld( const OUString& FileName, const cpo::uno::Any& ConfirmConversions, const cpo::uno::Any& ReadOnly, const cpo::uno::Any& AddToRecentFiles, const cpo::uno::Any& PasswordDocument, const cpo::uno::Any& PasswordTemplate, const cpo::uno::Any& Revert, const cpo::uno::Any& WritePasswordDocument, const cpo::uno::Any& WritePasswordTemplate, const cpo::uno::Any& Format ) override;
    virtual void SAL_CALL Close( const cpo::uno::Any& SaveChanges, const cpo::uno::Any& OriginalFormat, const cpo::uno::Any& RouteDocument ) override;
};

#endif // INCLUDED_SW_SOURCE_UI_VBA_VBADOCUMENTS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
