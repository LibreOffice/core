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

#ifndef INCLUDED_SW_SOURCE_UIBASE_UNO_SWXDOCUMENTSETTINGS_HXX
#define INCLUDED_SW_SOURCE_UIBASE_UNO_SWXDOCUMENTSETTINGS_HXX

#include <comphelper/MasterPropertySet.hxx>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <cppuhelper/weak.hxx>

class SwXTextDocument;
class SwDocShell;
class SwDoc;
class SfxPrinter;

class SwXDocumentSettings :
        public comphelper::MasterPropertySet,
        public css::lang::XServiceInfo,
        public css::lang::XTypeProvider,
        public cppu::OWeakObject
{
protected:
    css::uno::Reference< css::text::XTextDocument >       mxModel;
    SwXTextDocument*        mpModel;
    SwDocShell*             mpDocSh;
    SwDoc*                  mpDoc;

    /** the printer should be set only once; since there are several
     * printer-related properties, remember the last printer and set it in
     * _postSetValues */
    VclPtr<SfxPrinter>      mpPrinter;

    virtual void _preSetValues ()
        throw(css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException ) override;
    virtual void _setSingleValue( const comphelper::PropertyInfo & rInfo, const css::uno::Any &rValue )
        throw (css::beans::UnknownPropertyException, css::beans::PropertyVetoException,
               css::lang::IllegalArgumentException, css::lang::WrappedTargetException,
               css::uno::RuntimeException, std::exception) override;
    virtual void _postSetValues ()
        throw(css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException ) override;

    virtual void _preGetValues ()
        throw(css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException ) override;
    virtual void _getSingleValue( const comphelper::PropertyInfo & rInfo, css::uno::Any & rValue )
        throw (css::beans::UnknownPropertyException,
               css::lang::WrappedTargetException,
               css::uno::RuntimeException) override;
    virtual void _postGetValues ()
        throw(css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException ) override;
protected:
    virtual ~SwXDocumentSettings()
        throw();
public:
    SwXDocumentSettings( SwXTextDocument* pModel );

    // XInterface
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType )
        throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL acquire(  )
        throw () override;
    virtual void SAL_CALL release(  )
        throw () override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  )
        throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
        throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  )
        throw(css::uno::RuntimeException, std::exception) override;

    // XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw (css::uno::RuntimeException, std::exception) override;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
