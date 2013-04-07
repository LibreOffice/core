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
    virtual OUString SAL_CALL
    getOfficeInstallationDirectoryURL()
        throw (::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL
    getOfficeUserDataDirectoryURL()
        throw (::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL
    makeRelocatableURL( const OUString& URL )
        throw (::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL
    makeAbsoluteURL( const OUString& URL )
        throw (::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual OUString SAL_CALL
    getImplementationName()
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL
    supportsService( const OUString& ServiceName )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL
    getSupportedServiceNames()
        throw (::com::sun::star::uno::RuntimeException);

    // XServiceInfo - static versions (used for component registration)
    static OUString SAL_CALL
        getImplementationName_static();
    static ::com::sun::star::uno::Sequence< OUString > SAL_CALL
        getSupportedServiceNames_static();
    static OUString SAL_CALL
        getSingletonName_static();
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
        Create( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& );

private:
    void initDirs();

    OUString                                   m_aOfficeBrandDirMacro;
    OUString                                   m_aOfficeBaseDirMacro;
    OUString                                   m_aUserDirMacro;
    com::sun::star::uno::Reference<
        com::sun::star::uno::XComponentContext >    m_xCtx;
    OUString *                                 m_pOfficeBrandDir;
    OUString *                                 m_pUserDir;
};

} // namespace comphelper

#endif /* !INCLUDED_OFFICEINSTALLATIONDIRECTORIES_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
