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

#pragma once
#if 1

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/script/XStorageBasedLibraryContainer.hpp>
#include <com/sun/star/embed/XStorage.hpp>

class BasicManager;

/** helper class which holds and manipulates a BasicManager
*/
class SfxBasicManagerHolder
{
private:
    BasicManager*   mpBasicManager;
    ::com::sun::star::uno::Reference< ::com::sun::star::script::XStorageBasedLibraryContainer >
                    mxBasicContainer;
    ::com::sun::star::uno::Reference< ::com::sun::star::script::XStorageBasedLibraryContainer >
                    mxDialogContainer;

public:
    SfxBasicManagerHolder();

    enum ContainerType
    {
        SCRIPTS, DIALOGS
    };

    /** returns <TRUE/> if and only if the instance is currently bound to a non-<NULL/>
        BasicManager.
    */
    bool    isValid() const { return mpBasicManager != NULL; }

    /** returns the BasicManager which this instance is currently bound to
    */
    BasicManager*
            get() const { return mpBasicManager; }

    /** binds the instance to the given BasicManager
    */
    void    reset( BasicManager* _pBasicManager );

    ::com::sun::star::uno::Reference< ::com::sun::star::script::XLibraryContainer >
            getLibraryContainer( ContainerType _eType );

    /** calls the storeLibraries at both our script and basic library container
    */
    void    storeAllLibraries();

    /** calls the setStorage at all our XStorageBasedLibraryContainer.
    */
    void    setStorage(
                const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& _rxStorage
            );

    /** calls the storeLibrariesToStorage at all our XStorageBasedLibraryContainer.
    */
    void    storeLibrariesToStorage(
                const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& _rxStorage
            );


    /** checks if any modules in the SfxLibraryContainer exceed the binary
        limits.
    */
    sal_Bool LegacyPsswdBinaryLimitExceeded( ::com::sun::star::uno::Sequence< rtl::OUString >& sModules );


private:
    void    impl_releaseContainers();

    bool    impl_getContainer(
                ContainerType _eType,
                ::com::sun::star::uno::Reference< ::com::sun::star::script::XStorageBasedLibraryContainer >& _out_rxContainer );
};

class SfxApplicationScriptLibraryContainer
{
public:
    // Service
    static ::com::sun::star::uno::Sequence< ::rtl::OUString > impl_getStaticSupportedServiceNames();
    static ::rtl::OUString impl_getStaticImplementationName();
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL impl_createInstance
        ( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager )
            throw( ::com::sun::star::uno::Exception );
    static ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory > impl_createFactory
        ( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager );

};

class SfxApplicationDialogLibraryContainer
{
public:
    // Service
    static ::com::sun::star::uno::Sequence< ::rtl::OUString > impl_getStaticSupportedServiceNames();
    static ::rtl::OUString impl_getStaticImplementationName();
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL impl_createInstance
        ( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager )
            throw( ::com::sun::star::uno::Exception );
    static ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory > impl_createFactory
        ( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager );

};

#endif // APPBASLIB_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
