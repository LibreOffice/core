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
#ifndef _DAVREQUESTENVIRONMENT_HXX_
#define _DAVREQUESTENVIRONMENT_HXX_

#include <vector>
#include <rtl/ref.hxx>
#include "DAVAuthListener.hxx"

namespace webdav_ucp
{
    typedef std::pair< rtl::OUString, rtl::OUString > DAVRequestHeader;
    typedef std::vector< DAVRequestHeader > DAVRequestHeaders;

struct DAVRequestEnvironment
{
    rtl::OUString m_aRequestURI;
    rtl::Reference< DAVAuthListener >     m_xAuthListener;
//    rtl::Reference< DAVStatusListener >   m_xStatusListener;
//    rtl::Reference< DAVProgressListener > m_xStatusListener;
    DAVRequestHeaders                     m_aRequestHeaders;
    uno::Reference< ucb::XCommandEnvironment > m_xEnv;

DAVRequestEnvironment( const rtl::OUString & rRequestURI,
                           const rtl::Reference< DAVAuthListener > & xListener,
                           const DAVRequestHeaders & rRequestHeaders,
                           const uno::Reference< ucb::XCommandEnvironment > & xEnv)
    : m_aRequestURI( rRequestURI ),
      m_xAuthListener( xListener ),
      m_aRequestHeaders( rRequestHeaders ),
      m_xEnv( xEnv ){}

    DAVRequestEnvironment() {}
};

} // namespace webdav_ucp

#endif // _DAVREQUESTENVIRONMENT_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
