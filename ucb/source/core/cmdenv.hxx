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

#ifndef INCLUDED_UCB_SOURCE_CORE_CMDENV_HXX
#define INCLUDED_UCB_SOURCE_CORE_CMDENV_HXX

#include <cppuhelper/implbase.hxx>

#include "com/sun/star/lang/XInitialization.hpp"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/lang/XSingleServiceFactory.hpp"
#include "com/sun/star/ucb/XCommandEnvironment.hpp"

namespace ucb_cmdenv {

class UcbCommandEnvironment :
        public cppu::WeakImplHelper< com::sun::star::lang::XInitialization,
                                      com::sun::star::lang::XServiceInfo,
                                      com::sun::star::ucb::XCommandEnvironment >
{
    com::sun::star::uno::Reference<
        com::sun::star::task::XInteractionHandler > m_xIH;
    com::sun::star::uno::Reference<
        com::sun::star::ucb::XProgressHandler >     m_xPH;

public:
    explicit UcbCommandEnvironment(
        const com::sun::star::uno::Reference<
            com::sun::star::lang::XMultiServiceFactory >& rXSMgr );
    virtual ~UcbCommandEnvironment();

    // XInitialization
    virtual void SAL_CALL
    initialize( const com::sun::star::uno::Sequence<
                        com::sun::star::uno::Any >& aArguments )
        throw( com::sun::star::uno::Exception,
               com::sun::star::uno::RuntimeException, std::exception ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw ( com::sun::star::uno::RuntimeException, std::exception ) override;

    virtual sal_Bool SAL_CALL
    supportsService( const OUString& ServiceName )
        throw ( com::sun::star::uno::RuntimeException, std::exception ) override;

    virtual com::sun::star::uno::Sequence< OUString > SAL_CALL
    getSupportedServiceNames()
        throw ( com::sun::star::uno::RuntimeException, std::exception ) override;

    // XCommandEnvironment
    virtual com::sun::star::uno::Reference<
        com::sun::star::task::XInteractionHandler > SAL_CALL
    getInteractionHandler()
        throw ( com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual com::sun::star::uno::Reference<
        com::sun::star::ucb::XProgressHandler > SAL_CALL
    getProgressHandler()
        throw ( com::sun::star::uno::RuntimeException, std::exception ) override;

    // Non-UNO interfaces
    static OUString
    getImplementationName_Static();
    static com::sun::star::uno::Sequence< OUString >
    getSupportedServiceNames_Static();

    static com::sun::star::uno::Reference<
            com::sun::star::lang::XSingleServiceFactory >
    createServiceFactory( const com::sun::star::uno::Reference<
            com::sun::star::lang::XMultiServiceFactory > & rxServiceMgr );
private:
    //com::sun::star::uno::Reference<
    //    com::sun::star::lang::XMultiServiceFactory > m_xSMgr;
};

} // namespace ucb_cmdenv

#endif // INCLUDED_UCB_SOURCE_CORE_CMDENV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
