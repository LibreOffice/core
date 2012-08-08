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

#ifndef _FOLDERPICKER_HXX_
#define _FOLDERPICKER_HXX_

#include <cppuhelper/implbase3.hxx>
#include <osl/mutex.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <com/sun/star/ui/dialogs/XFolderPicker.hpp>
#include <com/sun/star/util/XCancellable.hpp>

#include <memory>

#include "WinFOPImpl.hxx"

//----------------------------------------------------------
// class declaration
//----------------------------------------------------------

class CFolderPicker :
    public  cppu::WeakImplHelper3<
                com::sun::star::ui::dialogs::XFolderPicker,
                com::sun::star::lang::XServiceInfo,
                com::sun::star::util::XCancellable >
{
public:

    // ctor/dtor
    CFolderPicker( const com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory >& xServiceMgr );

    //------------------------------------------------------------------------------------
    // XExecutableDialog
    //------------------------------------------------------------------------------------

    virtual void SAL_CALL setTitle( const rtl::OUString& aTitle )
        throw( com::sun::star::uno::RuntimeException );

    virtual sal_Int16 SAL_CALL execute(  )
        throw( com::sun::star::uno::RuntimeException );

    //------------------------------------------------------------------------------------
    // XFolderPicker functions
    //------------------------------------------------------------------------------------

    virtual void SAL_CALL setDisplayDirectory( const rtl::OUString& aDirectory )
        throw( com::sun::star::lang::IllegalArgumentException, com::sun::star::uno::RuntimeException );

    virtual rtl::OUString SAL_CALL getDisplayDirectory(  )
        throw( com::sun::star::uno::RuntimeException );

    virtual rtl::OUString SAL_CALL getDirectory( )
        throw( com::sun::star::uno::RuntimeException );

    virtual void SAL_CALL setDescription( const rtl::OUString& aDescription )
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
        throw(::com::sun::star::uno::RuntimeException);

    //------------------------------------------------
    // overwrite base class method, which is called
    // by base class dispose function
    //------------------------------------------------

    virtual void SAL_CALL disposing();

private:
    com::sun::star::uno::Reference< com::sun::star::lang::XMultiServiceFactory > m_xServiceMgr;
    std::auto_ptr< CWinFolderPickerImpl >   m_pFolderPickerImpl;
    osl::Mutex  m_aMutex;

// prevent copy and assignment
private:
    CFolderPicker( const CFolderPicker& );
    CFolderPicker& operator=( const CFolderPicker&  );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
