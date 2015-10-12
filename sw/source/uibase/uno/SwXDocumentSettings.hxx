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
        public com::sun::star::lang::XServiceInfo,
        public com::sun::star::lang::XTypeProvider,
        public cppu::OWeakObject
{
protected:
    com::sun::star::uno::Reference< com::sun::star::text::XTextDocument >       mxModel;
    SwXTextDocument*        mpModel;
    SwDocShell*             mpDocSh;
    SwDoc*                  mpDoc;

    /** the printer should be set only once; since there are several
     * printer-related properties, remember the last printer and set it in
     * _postSetValues */
    VclPtr<SfxPrinter>      mpPrinter;

    virtual void _preSetValues ()
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException ) override;
    virtual void _setSingleValue( const comphelper::PropertyInfo & rInfo, const ::com::sun::star::uno::Any &rValue )
        throw (css::beans::UnknownPropertyException, css::beans::PropertyVetoException,
               css::lang::IllegalArgumentException, css::lang::WrappedTargetException,
               css::uno::RuntimeException, std::exception) override;
    virtual void _postSetValues ()
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException ) override;

    virtual void _preGetValues ()
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException ) override;
    virtual void _getSingleValue( const comphelper::PropertyInfo & rInfo, ::com::sun::star::uno::Any & rValue )
        throw (css::beans::UnknownPropertyException,
               css::lang::WrappedTargetException,
               css::uno::RuntimeException) override;
    virtual void _postGetValues ()
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException ) override;
protected:
    virtual ~SwXDocumentSettings()
        throw();
public:
    SwXDocumentSettings( SwXTextDocument* pModel );

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL acquire(  )
        throw () override;
    virtual void SAL_CALL release(  )
        throw () override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  )
        throw(com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
        throw(com::sun::star::uno::RuntimeException, std::exception) override;
    virtual com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  )
        throw(com::sun::star::uno::RuntimeException, std::exception) override;

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
