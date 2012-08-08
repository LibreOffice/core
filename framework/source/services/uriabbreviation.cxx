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

#include "services/uriabbreviation.hxx"
#include "services.h"

#include "sal/config.h"
#include "cppuhelper/factory.hxx"
#include "cppuhelper/implementationentry.hxx"

#include "tools/urlobj.hxx"

// component helper namespace
namespace css = ::com::sun::star;

// framework namespace
namespace framework
{

namespace css = ::com::sun::star;

//*****************************************************************************************************************
//  XInterface, XTypeProvider, XServiceInfo
//*****************************************************************************************************************
DEFINE_XSERVICEINFO_MULTISERVICE_2 (    UriAbbreviation                                                         ,
                                        ::cppu::OWeakObject                                                     ,
                                        SERVICENAME_STRINGABBREVIATION                                          ,
                                        IMPLEMENTATIONNAME_URIABBREVIATION
                                    )

DEFINE_INIT_SERVICE                 (   UriAbbreviation,
                                        {
                                        }
                                    )

UriAbbreviation::UriAbbreviation(css::uno::Reference< css::uno::XComponentContext > const & context) :
    m_xContext(context)
{
}

// ::com::sun::star::util::XStringAbbreviation:
::rtl::OUString SAL_CALL UriAbbreviation::abbreviateString(const css::uno::Reference< css::util::XStringWidth > & xStringWidth, ::sal_Int32 nWidth, const ::rtl::OUString & aString) throw (css::uno::RuntimeException)
{
    ::rtl::OUString aResult( aString );
    if ( xStringWidth.is() )
    {
        // Use INetURLObject to abbreviate URLs
        INetURLObject aURL( aString );
        aResult = aURL.getAbbreviated( xStringWidth, nWidth, INetURLObject::DECODE_UNAMBIGUOUS );
    }

    return aResult;
}

} // namespace framework





/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
