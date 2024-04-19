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

#include "DAVAuthListener.hxx"
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <utility>


namespace http_dav_ucp
{




    class DAVAuthListener_Impl : public DAVAuthListener
    {
    public:

        DAVAuthListener_Impl(
            css::uno::Reference<css::ucb::XCommandEnvironment> xEnv,
            OUString inURL )
            : m_xEnv(std::move( xEnv )), m_aURL(std::move( inURL ))
        {
        }

        virtual int authenticate( const OUString & inRealm,
                                  const OUString & inHostName,
                                  OUString & inoutUserName,
                                  OUString & outPassWord,
                                  bool bCanUseSystemCredentials ) override;
    private:

        const css::uno::Reference< css::ucb::XCommandEnvironment > m_xEnv;
        const OUString m_aURL;

        OUString m_aPrevPassword;
        OUString m_aPrevUsername;
    };

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
