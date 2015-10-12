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
#ifndef INCLUDED_SC_SOURCE_UI_VBA_VBAWORKBOOKS_HXX
#define INCLUDED_SC_SOURCE_UI_VBA_VBAWORKBOOKS_HXX

#include <cppuhelper/implbase.hxx>
#include <vbahelper/vbacollectionimpl.hxx>
#include <ooo/vba/excel/XWorkbooks.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <vbahelper/vbadocumentsbase.hxx>
#include "excelvbahelper.hxx"

typedef cppu::ImplInheritanceHelper< VbaDocumentsBase, ov::excel::XWorkbooks > ScVbaWorkbooks_BASE;

class ScVbaWorkbooks : public ScVbaWorkbooks_BASE
{
private:
    OUString   getFileFilterType( const OUString& rString );
    static bool       isTextFile( const OUString& rString );
    static bool       isSpreadSheetFile( const OUString& rString );
    static sal_Int16& getCurrentDelim(){ static sal_Int16 nDelim = 44; return nDelim; }
public:
    ScVbaWorkbooks( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext );
    virtual ~ScVbaWorkbooks() {}

    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException) override;
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException) override;

    // ScVbaWorkbooks_BASE
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource ) override;
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;

    // XWorkbooks
    virtual css::uno::Any SAL_CALL Add( const css::uno::Any& Template ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL Close(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL Open( const OUString& Filename, const css::uno::Any& UpdateLinks, const css::uno::Any& ReadOnly, const css::uno::Any& Format, const css::uno::Any& Password, const css::uno::Any& WriteResPassword, const css::uno::Any& IgnoreReadOnlyRecommended, const css::uno::Any& Origin, const css::uno::Any& Delimiter, const css::uno::Any& Editable, const css::uno::Any& Notify, const css::uno::Any& Converter, const css::uno::Any& AddToMru ) throw (css::uno::RuntimeException, std::exception) override;
};

#endif // INCLUDED_SC_SOURCE_UI_VBA_VBAWORKBOOKS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
