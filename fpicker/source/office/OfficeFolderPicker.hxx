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
#ifndef INCLUDED_FPICKER_SOURCE_OFFICE_OFFICEFOLDERPICKER_HXX
#define INCLUDED_FPICKER_SOURCE_OFFICE_OFFICEFOLDERPICKER_HXX

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/ui/dialogs/XFolderPicker2.hpp>
#include <com/sun/star/ui/dialogs/XAsynchronousExecutableDialog.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include "commonpicker.hxx"

class Dialog;

typedef
    cppu::ImplInheritanceHelper<
        svt::OCommonPicker, css::ui::dialogs::XFolderPicker2,
        css::ui::dialogs::XAsynchronousExecutableDialog,
        css::lang::XServiceInfo >
    SvtFolderPicker_Base;

class SvtFolderPicker: public SvtFolderPicker_Base
{
private:
    OUString         m_aDescription;

    css::uno::Reference< css::ui::dialogs::XDialogClosedListener >
                            m_xListener;

    void                            prepareExecute( );
    DECL_LINK_TYPED(                DialogClosedHdl, Dialog&, void );

public:
                                    SvtFolderPicker( const css::uno::Reference < css::lang::XMultiServiceFactory >& xFactory );
    virtual                        ~SvtFolderPicker();


    // XFolderPicker2 functions


    virtual void SAL_CALL           setDisplayDirectory( const OUString& aDirectory ) throw( css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception ) override;
    virtual OUString SAL_CALL       getDisplayDirectory() throw( css::uno::RuntimeException, std::exception ) override;
    virtual OUString SAL_CALL       getDirectory() throw( css::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL           setDescription( const OUString& aDescription ) throw ( css::uno::RuntimeException, std::exception ) override;

    virtual void SAL_CALL cancel()
        throw (css::uno::RuntimeException, std::exception) override;


    // XExecutableDialog functions

    virtual void SAL_CALL setTitle( const OUString& _rTitle ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL execute(  ) throw (css::uno::RuntimeException, std::exception) override;


    // XAsynchronousExecutableDialog functions

    virtual void SAL_CALL       setDialogTitle( const OUString& _rTitle ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL       startExecuteModal( const css::uno::Reference< css::ui::dialogs::XDialogClosedListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;


    // XServiceInfo functions


    /* XServiceInfo */
    virtual OUString SAL_CALL    getImplementationName() throw( css::uno::RuntimeException, std::exception ) override;
    virtual sal_Bool SAL_CALL       supportsService( const OUString& sServiceName ) throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL
                                    getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception ) override;

    /* Helper for XServiceInfo */
    static css::uno::Sequence< OUString > impl_getStaticSupportedServiceNames();
    static OUString impl_getStaticImplementationName();

    /* Helper for registry */
    static css::uno::Reference< css::uno::XInterface > SAL_CALL impl_createInstance (
        const css::uno::Reference< css::uno::XComponentContext >& rxContext )
        throw( css::uno::Exception );

protected:

    // OCommonPicker overridables

    virtual VclPtr<SvtFileDialog_Base> implCreateDialog( vcl::Window* _pParent ) override;
    virtual sal_Int16       implExecutePicker( ) override;
};

#endif // INCLUDED_FPICKER_SOURCE_OFFICE_OFFICEFOLDERPICKER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
