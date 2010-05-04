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

#if ! defined INCLUDED_DP_TMPREPOSITORYCOMMANDENVIRONMENT_HXX
#define INCLUDED_DP_TMPREPOSITORYCOMMANDENVIRONMENT_HXX


#include "cppuhelper/compbase3.hxx"
//#include "cppuhelper/implbase2.hxx"
#include "ucbhelper/content.hxx"



namespace css = ::com::sun::star;

namespace dp_manager {

/**
   This command environment is to be used when an extension is temporarily
   stored in the "tmp" repository. It prevents all kind of user interaction.
 */
class TmpRepositoryCommandEnv
    : public ::cppu::WeakImplHelper3< css::ucb::XCommandEnvironment,
                                      css::task::XInteractionHandler,
                                      css::ucb::XProgressHandler >
{
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    css::uno::Reference< css::task::XInteractionHandler> m_forwardHandler;
public:
    virtual ~TmpRepositoryCommandEnv();
    TmpRepositoryCommandEnv();
    TmpRepositoryCommandEnv(
        css::uno::Reference< css::task::XInteractionHandler> const & handler);

    // XCommandEnvironment
    virtual css::uno::Reference<css::task::XInteractionHandler > SAL_CALL
    getInteractionHandler() throw (css::uno::RuntimeException);
    virtual css::uno::Reference<css::ucb::XProgressHandler >
    SAL_CALL getProgressHandler() throw (css::uno::RuntimeException);

    // XInteractionHandler
    virtual void SAL_CALL handle(
        css::uno::Reference<css::task::XInteractionRequest > const & xRequest )
        throw (css::uno::RuntimeException);

    // XProgressHandler
    virtual void SAL_CALL push( css::uno::Any const & Status )
        throw (css::uno::RuntimeException);
    virtual void SAL_CALL update( css::uno::Any const & Status )
        throw (css::uno::RuntimeException);
    virtual void SAL_CALL pop() throw (css::uno::RuntimeException);
};

}




#endif

