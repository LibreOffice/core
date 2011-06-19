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

#ifndef INCLUDED_OFFICEINSTALLATIONDIRECTORIES_HXX
#define INCLUDED_OFFICEINSTALLATIONDIRECTORIES_HXX

#include "osl/mutex.hxx"
#include "cppuhelper/implbase2.hxx"

#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/util/XOfficeInstallationDirectories.hpp"

namespace comphelper {

//=========================================================================

typedef cppu::WeakImplHelper2<
            com::sun::star::util::XOfficeInstallationDirectories,
            com::sun::star::lang::XServiceInfo > UnoImplBase;

struct mutex_holder
{
    osl::Mutex m_aMutex;
};

class OfficeInstallationDirectories : public mutex_holder, public UnoImplBase
{
public:
    OfficeInstallationDirectories(
        const com::sun::star::uno::Reference<
            com::sun::star::uno::XComponentContext > & xCtx );
    virtual ~OfficeInstallationDirectories();

    // XOfficeInstallationDirectories
    virtual ::rtl::OUString SAL_CALL
    getOfficeInstallationDirectoryURL()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL
    getOfficeUserDataDirectoryURL()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL
    makeRelocatableURL( const ::rtl::OUString& URL )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL
    makeAbsoluteURL( const ::rtl::OUString& URL )
        throw (::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL
    getImplementationName()
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL
    supportsService( const ::rtl::OUString& ServiceName )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
    getSupportedServiceNames()
        throw (::com::sun::star::uno::RuntimeException);

    // XServiceInfo - static versions (used for component registration)
    static ::rtl::OUString SAL_CALL
        getImplementationName_static();
    static ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
        getSupportedServiceNames_static();
    static ::rtl::OUString SAL_CALL
        getSingletonName_static();
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
        Create( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& );

private:
    void initDirs();

    rtl::OUString                                   m_aOfficeBrandDirMacro;
    rtl::OUString                                   m_aOfficeBaseDirMacro;
    rtl::OUString                                   m_aUserDirMacro;
    com::sun::star::uno::Reference<
        com::sun::star::uno::XComponentContext >    m_xCtx;
    rtl::OUString *                                 m_pOfficeBrandDir;
    rtl::OUString *                                 m_pOfficeBaseDir;
    rtl::OUString *                                 m_pUserDir;
};

} // namespace comphelper

#endif /* !INCLUDED_OFFICEINSTALLATIONDIRECTORIES_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
