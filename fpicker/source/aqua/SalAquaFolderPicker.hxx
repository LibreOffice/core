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

#ifndef _SALAQUAFOLDERPICKER_HXX_
#define _SALAQUAFOLDERPICKER_HXX_

#include <cppuhelper/implbase4.hxx>
#include <com/sun/star/util/XCancellable.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <com/sun/star/ui/dialogs/XFolderPicker.hpp>

#include "SalAquaPicker.hxx"

#include <memory>

#include <rtl/ustring.hxx>

#include <list>

//----------------------------------------------------------
// class declaration
//----------------------------------------------------------

class SalAquaFolderPicker :
        public SalAquaPicker,
    public cppu::WeakImplHelper4<
    ::com::sun::star::ui::dialogs::XFolderPicker,
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::lang::XEventListener,
        ::com::sun::star::util::XCancellable >
{
public:

    // constructor
    SalAquaFolderPicker( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceMgr );

    //------------------------------------------------------------------------------------
    // XExecutableDialog functions
    //------------------------------------------------------------------------------------

    virtual void SAL_CALL setTitle( const ::rtl::OUString& aTitle )
        throw( ::com::sun::star::uno::RuntimeException );

    virtual sal_Int16 SAL_CALL execute(  )
        throw( ::com::sun::star::uno::RuntimeException );

    //------------------------------------------------------------------------------------
    // XFolderPicker functions
    //------------------------------------------------------------------------------------

    virtual void SAL_CALL setDisplayDirectory( const rtl::OUString& rDirectory )
        throw( com::sun::star::lang::IllegalArgumentException, com::sun::star::uno::RuntimeException );

    virtual rtl::OUString SAL_CALL getDisplayDirectory(  )
        throw( com::sun::star::uno::RuntimeException );

    virtual rtl::OUString SAL_CALL getDirectory( )
        throw( com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL setDescription( const rtl::OUString& rDescription )
        throw( com::sun::star::uno::RuntimeException );

    //------------------------------------------------
    // XServiceInfo
    //------------------------------------------------

    virtual ::rtl::OUString SAL_CALL getImplementationName(  )
        throw(::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
        throw(::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  )
        throw(::com::sun::star::uno::RuntimeException);

    //------------------------------------------------
    // XCancellable
    //------------------------------------------------

    virtual void SAL_CALL cancel( )
        throw( ::com::sun::star::uno::RuntimeException );

    //------------------------------------------------
    // XEventListener
    //------------------------------------------------

    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& aEvent )
        throw(::com::sun::star::uno::RuntimeException);

private:
    // prevent copy and assignment
    SalAquaFolderPicker( const SalAquaFolderPicker& );
    SalAquaFolderPicker& operator=( const SalAquaFolderPicker& );

    // to instantiate own services
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xServiceMgr;

};

#endif // _SALAQUAFOLDERPICKER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
