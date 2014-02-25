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

#ifndef INCLUDED_FILTER_SOURCE_FLASH_SWFDIALOG_HXX
#define INCLUDED_FILTER_SOURCE_FLASH_SWFDIALOG_HXX

#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/document/XExporter.hpp>

#include <svtools/genericunodialog.hxx>

// -------------
// - SWFDialog -
// -------------

class Window;
class ResMgr;

class SWFDialog : public ::svt::OGenericUnoDialog,
                  public ::comphelper::OPropertyArrayUsageHelper< SWFDialog >,
                  public ::com::sun::star::beans::XPropertyAccess,
                  public ::com::sun::star::document::XExporter
{
private:

    com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >   maMediaDescriptor;
    com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >   maFilterData;
    com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >     mxSrcDoc;

protected:

    // XInterface
    virtual com::sun::star::uno::Any SAL_CALL queryInterface( const com::sun::star::uno::Type& aType ) throw (com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL acquire() throw ();
    virtual void SAL_CALL release() throw ();

    // OGenericUnoDialog
    virtual com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw(com::sun::star::uno::RuntimeException, std::exception);
    virtual OUString SAL_CALL getImplementationName() throw (com::sun::star::uno::RuntimeException, std::exception);
    virtual com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (com::sun::star::uno::RuntimeException, std::exception);
    virtual Dialog* createDialog( Window* pParent );
    virtual void executedDialog( sal_Int16 nExecutionResult );
    virtual com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo>  SAL_CALL getPropertySetInfo() throw(com::sun::star::uno::RuntimeException, std::exception);
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;

    // XPropertyAccess
    using ::cppu::OPropertySetHelper::getPropertyValues;
    using ::cppu::OPropertySetHelper::setPropertyValues;
    virtual com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getPropertyValues(  ) throw (com::sun::star::uno::RuntimeException, std::exception);
    virtual void SAL_CALL setPropertyValues( const com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aProps ) throw (com::sun::star::beans::UnknownPropertyException, com::sun::star::beans::PropertyVetoException, com::sun::star::lang::IllegalArgumentException, com::sun::star::lang::WrappedTargetException, com::sun::star::uno::RuntimeException, std::exception);

       // XExporter
    virtual void SAL_CALL setSourceDocument( const com::sun::star::uno::Reference< com::sun::star::lang::XComponent >& xDoc ) throw(com::sun::star::lang::IllegalArgumentException, com::sun::star::uno::RuntimeException, std::exception);

public:

                SWFDialog( const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxContext );
    virtual     ~SWFDialog();
};

// -----------------------------------------------------------------------------

#endif // SWFDialog_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
