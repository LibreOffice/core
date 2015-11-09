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

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/
#ifndef INCLUDED_UCB_SOURCE_UCP_FTP_FTPCONTENTIDENTIFIER_HXX
#define INCLUDED_UCB_SOURCE_UCP_FTP_FTPCONTENTIDENTIFIER_HXX

#include <vector>
#include "curl.hxx"
#include <curl/easy.h>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <com/sun/star/ucb/XContentIdentifier.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>

#include "ftpdirp.hxx"
#include "ftpurl.hxx"


namespace ftp {

    class FTPContentIdentifier
        : public cppu::OWeakObject,
          public css::lang::XTypeProvider,
          public css::ucb::XContentIdentifier
    {
    public:

        explicit FTPContentIdentifier(const OUString& ident);

        virtual ~FTPContentIdentifier();

        // XInterface

        virtual css::uno::Any SAL_CALL
        queryInterface( const css::uno::Type& rType )
            throw( css::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL acquire() throw() override;

        virtual void SAL_CALL release() throw() override;

        // XTypeProvider

        virtual
        css::uno::Sequence<css::uno::Type> SAL_CALL
        getTypes()
            throw(
                css::uno::RuntimeException, std::exception
            ) override;

        virtual css::uno::Sequence<sal_Int8> SAL_CALL
        getImplementationId()
            throw(
                css::uno::RuntimeException, std::exception
            ) override;


        // XContentIdentifier

        virtual OUString SAL_CALL
        getContentIdentifier()
            throw (
                css::uno::RuntimeException, std::exception
            ) override;

        virtual OUString SAL_CALL
        getContentProviderScheme()
            throw (
                css::uno::RuntimeException, std::exception
            ) override;


    private:

        OUString m_ident;
    };

}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
