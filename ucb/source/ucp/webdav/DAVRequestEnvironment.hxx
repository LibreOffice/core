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


#ifndef INCLUDED_UCB_SOURCE_UCP_WEBDAV_DAVREQUESTENVIRONMENT_HXX
#define INCLUDED_UCB_SOURCE_UCP_WEBDAV_DAVREQUESTENVIRONMENT_HXX

#include <vector>
#include <rtl/ref.hxx>
#include "DAVAuthListener.hxx"

namespace http_dav_ucp
{
    typedef std::pair< OUString, OUString > DAVRequestHeader;
    typedef std::vector< DAVRequestHeader > DAVRequestHeaders;

struct DAVRequestEnvironment
{
    OUString m_aRequestURI;
    rtl::Reference< DAVAuthListener >     m_xAuthListener;
    DAVRequestHeaders                     m_aRequestHeaders;
    css::uno::Reference< css::ucb::XCommandEnvironment > m_xEnv;

DAVRequestEnvironment( const OUString & rRequestURI,
                       const rtl::Reference< DAVAuthListener > & xListener,
                       const DAVRequestHeaders & rRequestHeaders,
                       const css::uno::Reference< css::ucb::XCommandEnvironment > & xEnv)
    : m_aRequestURI( rRequestURI ),
      m_xAuthListener( xListener ),
      m_aRequestHeaders( rRequestHeaders ),
      m_xEnv( xEnv ){}

    DAVRequestEnvironment() {}
};

} // namespace http_dav_ucp

#endif // INCLUDED_UCB_SOURCE_UCP_WEBDAV_DAVREQUESTENVIRONMENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
