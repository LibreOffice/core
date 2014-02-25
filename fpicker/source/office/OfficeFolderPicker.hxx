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
#ifndef INCLUDED_SVT_FOLDERPICKER_HXX
#define INCLUDED_SVT_FOLDERPICKER_HXX

#include <cppuhelper/implbase3.hxx>
#include <com/sun/star/ui/dialogs/XFolderPicker2.hpp>
#include <com/sun/star/ui/dialogs/XAsynchronousExecutableDialog.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include "commonpicker.hxx"

class Dialog;

typedef
    cppu::ImplInheritanceHelper3<
        svt::OCommonPicker, com::sun::star::ui::dialogs::XFolderPicker2,
        com::sun::star::ui::dialogs::XAsynchronousExecutableDialog,
        com::sun::star::lang::XServiceInfo >
    SvtFolderPicker_Base;

class SvtFolderPicker: public SvtFolderPicker_Base
{
private:
    OUString         m_aDescription;

    ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XDialogClosedListener >
                            m_xListener;

    void                            prepareExecute( );
    DECL_LINK(                      DialogClosedHdl, Dialog* );

public:
                                    SvtFolderPicker( const ::com::sun::star::uno::Reference < ::com::sun::star::lang::XMultiServiceFactory >& xFactory );
    virtual                        ~SvtFolderPicker();


    // XFolderPicker2 functions


    virtual void SAL_CALL           setDisplayDirectory( const OUString& aDirectory ) throw( ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException );
    virtual OUString SAL_CALL    getDisplayDirectory() throw( ::com::sun::star::uno::RuntimeException );
    virtual OUString SAL_CALL    getDirectory() throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL           setDescription( const OUString& aDescription ) throw ( ::com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL cancel()
        throw (com::sun::star::uno::RuntimeException);


    // XExecutableDialog functions

    virtual void SAL_CALL setTitle( const OUString& _rTitle ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL execute(  ) throw (::com::sun::star::uno::RuntimeException);


    // XAsynchronousExecutableDialog functions

    virtual void SAL_CALL       setDialogTitle( const OUString& _rTitle ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL       startExecuteModal( const ::com::sun::star::uno::Reference< ::com::sun::star::ui::dialogs::XDialogClosedListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);


    // XServiceInfo functions


    /* XServiceInfo */
    virtual OUString SAL_CALL    getImplementationName() throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL       supportsService( const OUString& sServiceName ) throw( ::com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Sequence< OUString > SAL_CALL
                                    getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException );

    /* Helper for XServiceInfo */
    static com::sun::star::uno::Sequence< OUString > impl_getStaticSupportedServiceNames();
    static OUString impl_getStaticImplementationName();

    /* Helper for registry */
    static ::com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL impl_createInstance (
        const ::com::sun::star::uno::Reference< com::sun::star::uno::XComponentContext >& rxContext )
        throw( com::sun::star::uno::Exception );

protected:

    // OCommonPicker overridables

    virtual SvtFileDialog*  implCreateDialog( Window* _pParent );
    virtual sal_Int16       implExecutePicker( );
};

#endif // INCLUDED_SVT_FOLDERPICKER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
