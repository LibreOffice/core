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

#pragma once

#include <comphelper/MasterPropertySet.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <cppuhelper/weak.hxx>
#include <vcl/vclptr.hxx>

class SwXTextDocument;
class SwDocShell;
class SwDoc;
class SfxPrinter;

class SwXDocumentSettings final :
        public comphelper::MasterPropertySet,
        public css::lang::XServiceInfo,
        public css::lang::XTypeProvider,
        public cppu::OWeakObject
{
    SwXTextDocument*        mpModel;
    SwDocShell*             mpDocSh;
    SwDoc*                  mpDoc;

    /** the printer should be set only once; since there are several
     * printer-related properties, remember the last printer and set it in
     * _postSetValues */
    VclPtr<SfxPrinter>      mpPrinter;
    bool mbPreferPrinterPapersize;

    virtual void _preSetValues () override;
    virtual void _setSingleValue( const comphelper::PropertyInfo & rInfo, const css::uno::Any &rValue ) override;
    virtual void _postSetValues () override;

    virtual void _preGetValues () override;
    virtual void _getSingleValue( const comphelper::PropertyInfo & rInfo, css::uno::Any & rValue ) override;
    virtual void _postGetValues () override;
    virtual ~SwXDocumentSettings()
        noexcept override;
public:
    SwXDocumentSettings( SwXTextDocument* pModel );

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) override;
    virtual void SAL_CALL acquire(  )
        noexcept override;
    virtual void SAL_CALL release(  )
        noexcept override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
