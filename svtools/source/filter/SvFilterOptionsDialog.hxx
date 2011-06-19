/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _SV_FILTER_OPTIONS_DIALOG_HXX_
#define _SV_FILTER_OPTIONS_DIALOG_HXX_

#include <tools/fldunit.hxx>
#include <cppuhelper/implbase5.hxx>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/document/XExporter.hpp>

class SvFilterOptionsDialog : public cppu::WeakImplHelper5
<
    com::sun::star::document::XExporter,
    com::sun::star::ui::dialogs::XExecutableDialog,
    com::sun::star::beans::XPropertyAccess,
    com::sun::star::lang::XInitialization,
    com::sun::star::lang::XServiceInfo
>
{
    const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >
        mxMgr;
    com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >
        maMediaDescriptor;
    com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >
        maFilterDataSequence;
    com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >
        mxSourceDocument;

    rtl::OUString   maDialogTitle;
    FieldUnit       meFieldUnit;
    sal_Bool        mbExportSelection;

public:

    SvFilterOptionsDialog( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > _rxORB );
    ~SvFilterOptionsDialog();

    // XInterface
    virtual void SAL_CALL acquire() throw();
    virtual void SAL_CALL release() throw();

    // XInitialization
    virtual void SAL_CALL initialize( const com::sun::star::uno::Sequence< com::sun::star::uno::Any > & aArguments )
        throw ( com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException );

    // XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName()
        throw ( com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService( const rtl::OUString& ServiceName )
        throw ( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames()
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
    virtual void SAL_CALL setTitle( const ::rtl::OUString& aTitle )
        throw ( ::com::sun::star::uno::RuntimeException );

    // XExporter
    virtual void SAL_CALL setSourceDocument( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& xDoc )
        throw ( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );

};


#endif // _SV_FILTER_OPTIONS_DIALOG_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
