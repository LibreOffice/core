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
#pragma once

#include <sal/config.h>

#include <string_view>

#include <cppuhelper/implbase.hxx>
#include <ooo/vba/excel/XWorkbooks.hpp>
#include <vbahelper/vbadocumentsbase.hxx>

typedef cppu::ImplInheritanceHelper< VbaDocumentsBase, ov::excel::XWorkbooks > ScVbaWorkbooks_BASE;

class ScVbaWorkbooks : public ScVbaWorkbooks_BASE
{
private:
    OUString   getFileFilterType( const OUString& rString );
    static bool       isTextFile( std::u16string_view rString );
    static bool       isSpreadSheetFile( std::u16string_view rString );
    static sal_Int16& getCurrentDelim(){ static sal_Int16 nDelim = 44; return nDelim; }
public:
    ScVbaWorkbooks( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext );

    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() override;

    // ScVbaWorkbooks_BASE
    virtual cpo::uno::Any createCollectionObject( const cpo::uno::Any& aSource ) override;
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;

    // XWorkbooks
    virtual cpo::uno::Any SAL_CALL Add( const cpo::uno::Any& Template ) override;
    virtual void SAL_CALL Close(  ) override;
    virtual cpo::uno::Any SAL_CALL Open( const OUString& Filename, const cpo::uno::Any& UpdateLinks, const cpo::uno::Any& ReadOnly, const cpo::uno::Any& Format, const cpo::uno::Any& Password, const cpo::uno::Any& WriteResPassword, const cpo::uno::Any& IgnoreReadOnlyRecommended, const cpo::uno::Any& Origin, const cpo::uno::Any& Delimiter, const cpo::uno::Any& Editable, const cpo::uno::Any& Notify, const cpo::uno::Any& Converter, const cpo::uno::Any& AddToMru ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
