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

#ifndef _FOLDERPICKER_HXX_
#define _FOLDERPICKER_HXX_

//---------------------------------------------------------
//  includes of other projects
//---------------------------------------------------------

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
