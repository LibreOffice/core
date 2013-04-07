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

#ifndef _SV_FILTER_OPTIONS_DIALOG_HXX_
#define _SV_FILTER_OPTIONS_DIALOG_HXX_

#include <tools/fldunit.hxx>
#include <cppuhelper/implbase5.hxx>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

class SvFilterOptionsDialog : public cppu::WeakImplHelper5
<
    com::sun::star::document::XExporter,
    com::sun::star::ui::dialogs::XExecutableDialog,
    com::sun::star::beans::XPropertyAccess,
    com::sun::star::lang::XInitialization,
    com::sun::star::lang::XServiceInfo
>
{
    const com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >
        mxContext;
    com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >
        maMediaDescriptor;
    com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >
        maFilterDataSequence;
    com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >
        mxSourceDocument;

    OUString   maDialogTitle;
    FieldUnit       meFieldUnit;
    sal_Bool        mbExportSelection;

public:

    SvFilterOptionsDialog( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxORB );
    ~SvFilterOptionsDialog();

    // XInterface
    virtual void SAL_CALL acquire() throw();
    virtual void SAL_CALL release() throw();

    // XInitialization
    virtual void SAL_CALL initialize( const com::sun::star::uno::Sequence< com::sun::star::uno::Any > & aArguments )
        throw ( com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException );

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw ( com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
        throw ( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames()
        throw ( com::sun::star::uno::RuntimeException );

    // XPropertyAccess
    virtual com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getPropertyValues()
        throw ( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setPropertyValues( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > & aProps )
        throw ( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException,
                ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException,
                ::com::sun::star::uno::RuntimeException );

    // XExecuteDialog
    virtual sal_Int16 SAL_CALL execute()
        throw ( com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL setTitle( const OUString& aTitle )
        throw ( ::com::sun::star::uno::RuntimeException );

    // XExporter
    virtual void SAL_CALL setSourceDocument( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& xDoc )
        throw ( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );

};


#endif // _SV_FILTER_OPTIONS_DIALOG_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
