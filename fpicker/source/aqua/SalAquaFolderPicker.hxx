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

#ifndef INCLUDED_FPICKER_SOURCE_AQUA_SALAQUAFOLDERPICKER_HXX
#define INCLUDED_FPICKER_SOURCE_AQUA_SALAQUAFOLDERPICKER_HXX

#include <cppuhelper/implbase.hxx>
#include <com/sun/star/util/XCancellable.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <com/sun/star/ui/dialogs/XFolderPicker2.hpp>

#include "SalAquaPicker.hxx"

#include <rtl/ustring.hxx>


// class declaration


class SalAquaFolderPicker :
        public SalAquaPicker,
    public cppu::WeakImplHelper<
    css::ui::dialogs::XFolderPicker2,
    css::lang::XServiceInfo,
    css::lang::XEventListener >
{
public:

    // constructor
    SalAquaFolderPicker( const css::uno::Reference< css::lang::XMultiServiceFactory >& xServiceMgr );


    // XExecutableDialog functions


    virtual void SAL_CALL setTitle( const OUString& aTitle )
        SAL_THROW_IfNotObjectiveC( css::uno::RuntimeException ) override;

    virtual sal_Int16 SAL_CALL execute(  )
        SAL_THROW_IfNotObjectiveC( css::uno::RuntimeException ) override;


    // XFolderPicker functions


    virtual void SAL_CALL setDisplayDirectory( const OUString& rDirectory )
        SAL_THROW_IfNotObjectiveC( com::sun::star::lang::IllegalArgumentException, com::sun::star::uno::RuntimeException ) override;

    virtual OUString SAL_CALL getDisplayDirectory(  )
        SAL_THROW_IfNotObjectiveC( com::sun::star::uno::RuntimeException ) override;

    virtual OUString SAL_CALL getDirectory( )
        SAL_THROW_IfNotObjectiveC( com::sun::star::uno::RuntimeException ) override;

    virtual void SAL_CALL setDescription( const OUString& rDescription )
        SAL_THROW_IfNotObjectiveC( com::sun::star::uno::RuntimeException ) override;


    // XServiceInfo


    virtual OUString SAL_CALL getImplementationName(  )
        SAL_THROW_IfNotObjectiveC( css::uno::RuntimeException ) override;

    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName )
        SAL_THROW_IfNotObjectiveC( css::uno::RuntimeException ) override;

    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  )
        SAL_THROW_IfNotObjectiveC( css::uno::RuntimeException ) override;


    // XCancellable


    virtual void SAL_CALL cancel( )
        SAL_THROW_IfNotObjectiveC( css::uno::RuntimeException ) override;


    // XEventListener


    virtual void SAL_CALL disposing( const css::lang::EventObject& aEvent )
        SAL_THROW_IfNotObjectiveC( css::uno::RuntimeException ) override;

private:
    SalAquaFolderPicker( const SalAquaFolderPicker& ) = delete;
    SalAquaFolderPicker& operator=( const SalAquaFolderPicker& ) = delete;

    // to instantiate own services
    css::uno::Reference< css::lang::XMultiServiceFactory > m_xServiceMgr;

};

#endif // INCLUDED_FPICKER_SOURCE_AQUA_SALAQUAFOLDERPICKER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
